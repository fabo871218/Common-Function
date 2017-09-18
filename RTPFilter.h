#ifndef RTP_FILTER_H_
#define RTP_FILTER_H_

#include "CircleBuffer.h"

#define RTP_FILTER

#define MAX_RTPFILTER_LEN	64
#define MAX_RTPFILTER_PACKET	32
/*
@brief 主要用来处理RTP包乱序的问题
*/
class RTPFilter{
public:
	RTPFilter(){
		memset(&m_seqArray, 0, sizeof(m_seqArray));
		m_requiredSeq = 0;
	}

	void InputPacket(BYTE* data, int size){
		if(size < sizeof(RTPHDR)) return;
		memcpy(&m_lastSeqNum, data+2, sizeof(unsigned short));
		m_lastSeqNum = ntohs(m_lastSeqNum);
		m_bufArray[m_lastSeqNum % MAX_RTPFILTER_LEN].input(data, size);
		m_seqArray[m_lastSeqNum % MAX_RTPFILTER_LEN] = m_lastSeqNum;
	}

	BOOL IsForward(unsigned short required, unsigned short nextseq){
		unsigned short tmp = nextseq - required;
		if(tmp < 0xFFFF - 1000){
			return TRUE;
		}
		return FALSE;
	}
	
	BOOL GetStreamData(CircleBuffer& buf){
		while(true){
			unsigned short index = m_requiredSeq%MAX_RTPFILTER_LEN; //读取下一个buf
			if((m_requiredSeq == m_seqArray[index] || IsForward(m_requiredSeq, m_seqArray[index])) &&
				m_bufArray[index].size() > 0){ 
			//buf有数据
				BYTE* data = m_bufArray[index].buff();
				int size = m_bufArray[index].size();

				RTPHDR rtp; memcpy(&rtp, data, sizeof(RTPHDR));
				buf.append(data + sizeof(RTPHDR), size - sizeof(RTPHDR));
				m_bufArray[index].reset();
				if(m_requiredSeq != m_seqArray[index]){
					TRACE("RTP seqnum skip from %d to %d\n", m_requiredSeq, m_seqArray[index]);
					m_requiredSeq = m_seqArray[index];
				}
				m_requiredSeq++;
				if(rtp.marker == 1) return TRUE;
			}else{
			//buf 没有数据
				if(IsForward(m_requiredSeq, m_lastSeqNum) && m_lastSeqNum - m_requiredSeq >= MAX_RTPFILTER_PACKET){
				//最新的序号已经大于期望的包序号超过20，则认为当前这个包已经丢失
					TRACE("Lost packet %d\n", m_requiredSeq);
					m_requiredSeq++;
				}else{
					return FALSE;
				}
			}
		}
	}
private:
	CircleBuffer	m_bufArray[MAX_RTPFILTER_LEN]; //RTP buf数组
	unsigned short	m_seqArray[MAX_RTPFILTER_LEN]; //RTP 序列号数组
	unsigned short	m_requiredSeq;	//期望额包序号
	unsigned short	m_lastSeqNum; //最后收到的RTP包的序列号
};

#endif /*RTP_FILTER_H_*/
