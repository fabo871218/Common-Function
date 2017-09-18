#ifndef __SZ_RESOURCE_H__
#define __SZ_RESOURCE_H__

template<typename _Ty, typename _Lock = CRITICAL_SECTION>
class scresuorce{
public:
	typedef _Ty						value_type;
	typedef _Lock					lock_type;
	typedef scresuorce<_Ty, _Lock>	my_type;
private:
	scresuorce(lock_type* _lock = NULL, value_type* _value = NULL, int* _state = NULL)
		: lock_(NULL)
		, value_(NULL){
		// needn't to lock if no resource input
		if (NULL != _value && NULL != _lock){
			EnterCriticalSection(_lock);
		}else{
			return;
		}
		lock_	= _lock;
		value_	= _value;
	}

	scresuorce(const my_type& _ref){
		my_type& r 		= const_cast<my_type&>(_ref);
		this->value_	= r.value_;
		this->lock_		= r.lock_;
		r.value_		= NULL;
		r.lock_			= NULL;
	}

public:
	my_type& operator = (const my_type& _ref){
		if (this != &_ref){
			this->release();
			my_type& r 		= const_cast<my_type&>(_ref);
			this->value_	= r.value_;
			this->lock_		= r.lock_;
			r.value_		= NULL;
			r.lock_			= NULL;
		}
		return *this;
	}

	~scresuorce(){
		this->release();
	}

	value_type*	operator->(){
		return this->value_;
	}

	value_type& operator *(){
		return *this->value_;
	}

	void release(){
		if (NULL != this->value_ && NULL != this->lock_){
			this->value_	= NULL;
			LeaveCriticalSection(lock_);
		}
		this->value_	= NULL;
		this->lock_		= NULL;
	}

	bool empty() const{
		return NULL == this->lock_ || NULL == this->value_;
	}

	operator bool() const{
		return ! this->empty();
	}

private:
	value_type*		value_;
	lock_type*		lock_;
};

template<typename _Ty, typename _Lock = CRITICAL_SECTION>
class scresuorce_manager{
public:
	typedef scresuorce<_Ty, _Lock>				resource_type;
	typedef scresuorce_manager<_Ty, _Lock>		my_type;
	typedef typename resource_type::value_type	value_type;
	typedef typename resource_type::lock_type	lock_type;

public:
	scresuorce_manager()
		: empty_(true){
		InitializeCriticalSection(&lock_);
	}

	scresuorce_manager(const value_type& _val)
		: lock_(false, true)
		, value_(_val)
		, empty_(false){
	}

	~scresuorce_manager(){
		// protect rc that has been got by user
		if (! this->empty()){
			resource_type rc = this->get();
			rc.release();
		}
		DeleteCriticalSection(&lock_);
	}

	//SCI_DISABLE_COPY(scresuorce_manager);

public:
	resource_type get(int* _state = NULL){
		if (this->empty()){
			return resource_type(NULL, NULL);
		}
		return resource_type(&this->lock_, &this->value_, _state);
	}

	bool set(const value_type& _value){
		bool result = false;
		EnterCriticalSection(&this->lock_);
		if (empty_){
			value_ 	= _value;
			empty_	= false;
			result 	= true;
		}
		LeaveCriticalSection(&this->lock_);
		return result;
	}

	void clear(){
		EnterCriticalSection(&this->lock_);
		this->empty_ = true;
		LeaveCriticalSection(&this->lock_);
	}

	bool empty(){
		if (! this->empty_)
		{
			return false;
		}
		EnterCriticalSection(&this->lock_);
		bool ret = this->empty_;
		LeaveCriticalSection(&this->lock_);
		return ret;
	}

private:
	bool			empty_;
	value_type		value_;
	lock_type		lock_;
};

#endif // __SZ_RESOURCE_H__

