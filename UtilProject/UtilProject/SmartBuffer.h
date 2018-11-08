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

	std::size_t append(char* data, std::size_t size, bool increase = true){
		std::size_t tail_size = _capacity - _length - (_data - _buffer);
		std::size_t left_capacity = _capacity - _length;
		if (size <= tail_size){
			memcpy(_data + _length, data, size);
			_length += size;
		}
		else{
			if (size <= left_capacity){
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
					_data = _buffer; _length += size;
				}
				else{
					return 0;
				}
			}
		}
		return size;
	}

	//Reset the old data with new data
	void reset(char* data, int length){
		reset(); 
		append(data, length);
	}

	//pop data from back of the buffer queue
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

	}

	SmartBuffer& operator=(const SmartBuffer& other) = delete;
	SmartBuffer(SmartBuffer& other) = delete;
	SmartBuffer(SmartBuffer&& other){

	}

	SmartBuffer& SmartBuffer::operator=(SmartBuffer&& other){

	}
private:
	SmartBuffer::SmartBuffer(const SmartBuffer& other){}
	SmartBuffer& SmartBuffer::operator=(const SmartBuffer& other) = delete;
private:
	char*		_buffer{ nullptr }; //point to begin of the buffer
	char*		_data{ nullptr }; //point to begin of data
	std::size_t			_capacity{ 0 }; //the buffer capacity
	std::size_t			_length{ 0 }; //actual data length
};

#endif /*_SMART_BUFFER_H_*/

