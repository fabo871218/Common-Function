#pragma once

class CircleBuffer{
public:
	CircleBuffer(int size = 1024){
		m_pBuffer = new BYTE[1024];
		m_pData = m_pBuffer;
		m_capacity = 1024;
		m_dataLen = 0;
	}

	~CircleBuffer(){
		if(m_pBuffer){
			delete []m_pBuffer;
		}
	}

	void increase_size(int size){
		int nsize = (size >= 1024 ? size : 1024);
		BYTE* nbuf = new BYTE[m_capacity + nsize];
		if(m_dataLen > 0)
			memcpy(nbuf, m_pData, m_dataLen);
		delete []m_pBuffer;
		m_pBuffer = nbuf;
		m_pData = nbuf;
		m_capacity += nsize;
	}

	void append(unsigned char* data, int length){
		int tail_size = m_capacity - m_dataLen - (m_pData - m_pBuffer);
		int left_capacity = m_capacity - m_dataLen;
		if(length <= tail_size){
			memcpy(m_pData + m_dataLen, data, length);
		}else if(length > tail_size && length <= left_capacity){
			memmove(m_pBuffer, m_pData, m_dataLen);
			m_pData = m_pBuffer;
			memcpy(m_pData + m_dataLen, data, length);
		}else{
			increase_size(length - left_capacity);
			memcpy(m_pData + m_dataLen, data, length);
		}
		m_dataLen += length;
	}

	void input(unsigned char* data, int length){
		reset();append(data, length);
	}

	//
	void	popout(int size){
		if(size >= m_dataLen){
			m_pData = m_pBuffer;
			m_dataLen = 0;
		}else{
			m_pData += size;
			m_dataLen -= size;
		}
	}

	//重置buffer，清除所有的数据
	void reset(){
		m_pData = m_pBuffer;
		m_dataLen = 0;
	}
	int size(){ return m_dataLen;}
	BYTE* buff(){ return m_pData; }
private:
	CircleBuffer::CircleBuffer(const CircleBuffer& other){}
	CircleBuffer& CircleBuffer::operator=(const CircleBuffer& other){}
private:
	BYTE*		m_pBuffer;
	BYTE*		m_pData;
	int			m_capacity;
	int			m_dataLen;
};
