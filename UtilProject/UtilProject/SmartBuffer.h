#ifndef _SMART_BUFFER_H_
#define _SMART_BUFFER_H_

#include <string>

class SmartBuffer{
public:
	SmartBuffer() = default;
	SmartBuffer(std::size_t size){
		_buffer = (char*)malloc(size*sizeof(char));
		if (_buffer){
			_capacity = size;
			_length = 0;
			_data = _buffer;
		}
	}

	~SmartBuffer(){
		if (_buffer) free(_buffer);
	}

	std::size_t reserve(std::size_t size){
		_buffer = (char*)realloc(_buffer, size);
		_data = _buffer;
		_length = 0;
		_capacity = size;
		return _capacity;
	}

    //append data to the buffer tail. if buffer capacity is not enough
    //and if increase is true ,it will realloc the property buffer size
    //otherwist, return zero
    //Return if failed, return zero, or return size append
	std::size_t append(const char* data, std::size_t size, bool increase = true){
		std::size_t tail_size = _capacity - _length - (_data - _buffer);
		std::size_t left_capacity = _capacity - _length;
		if (size <= tail_size){ 
            //If the buffer tail space is large enough, just copy data to buffer
			memcpy(_data + _length, data, size);
			_length += size;
		}
		else{
			if (size <= left_capacity){
                //If the tail space is not enough, but left capacity is enough,
                //first memmove the data, than copy data
				memmove(_buffer, _data, _length);
				_data = _buffer;
				memcpy(_data + _length, data, size);
			}
			else{
				if (increase){
					int isize = size - left_capacity;
					isize = isize >= 1024 ? isize : 1024;
					memmove(_buffer, _data, _length);
					_buffer = (char*)realloc(_buffer, _capacity + isize);
					memcpy(_buffer + _length, data, size);
					_data = _buffer; 
                    _length += size;
                    _capacity += isize;
				}
				else{
                    //If not auto increase buffer, return zero
					return 0;
				}
			}
		}
		return size;
	}

    std::size_t append(const char* data){
        return append(data, strlen(data));
    }

    void append_string_end(){
        char end_of_string[1] = { 0 };
        append(end_of_string, 1);
    }

    void set_data_size(std::size_t size){
        if (_capacity >= size){
            _data = _buffer;
            _length = size;
        }
        else{
            reserve(size);
            _length = size;
        }
    }

	//Reset the old data with new data
	void reset(char* data, int length){
		reset(); append(data, length);
	}

	//pop data from back of the buffer queue
    //Return: the size pop from back
	std::size_t pop_back(std::size_t size){
		if (size >= _length){
			reset(); //if size if bigger then actual size, reset the buf
			return _length;
		}
		else{
			_length -= size;
			return _length;
		}
	}

	//pop data from front of the buffer queue
    //Return: the size pop from front
	std::size_t pop_front(std::size_t size){
		if (size >= _length){
			reset(); //if size is bigger then actual size, reset the buf
			return _length;
		}
		else{
			_data += size;
			_length -= size;
		}
		return size;
	}

	//reset the buffer, clear all data
	void reset(){
		_data = _buffer;
		_length = 0;
	}

	std::size_t size(){ return _length; }
	char* buff(){ return _data; }

	void swap_buf(SmartBuffer& other){
        std::swap(_buffer, other._buffer);
        std::swap(_data, other._data);
        std::swap(_capacity, other._capacity);
        std::swap(_length, other._length);
	}

	SmartBuffer& operator=(const SmartBuffer& other) = delete;
	SmartBuffer(SmartBuffer& other) = delete;
	SmartBuffer(SmartBuffer&& other){
        _buffer = other._buffer;
        _data = other._data;
        _capacity = other._capacity;
        _length = other._length;

        other._buffer = nullptr;
        other._data = nullptr;
        other._capacity = 0;
        other._length = 0;
	}

	SmartBuffer& SmartBuffer::operator=(SmartBuffer&& other){
        swap_buf(other);
	}
private:
	char*		    _buffer{ nullptr }; //point to begin of the buffer
	char*		    _data{ nullptr }; //point to begin of data
	std::size_t	    _capacity{ 0 }; //the buffer capacity
	std::size_t		_length{ 0 }; //actual data length
};

#endif /*_SMART_BUFFER_H_*/

