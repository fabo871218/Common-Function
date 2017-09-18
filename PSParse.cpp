#include "stdafx.h"
#include "PSParse.h"

unsigned char packet_start_code_prefix[3] = {0x00, 0x00, 0x01};
unsigned char end_code[4]				  = {0x00, 0x00, 0x01, 0xB9};
unsigned char pack_start_code [4]		  = {0x00, 0x00, 0x01, 0xBA};
unsigned char system_header_start_code[4] = {0x00, 0x00, 0x01, 0xBB};
unsigned char program_stream_map_start_code[4] = {0x00, 0x00, 0x01, 0xBC};
unsigned char* parse_system_header(unsigned char* p){
	int header_length;
	p += 4; /*skip start code*/
	header_length = (*(p++)) << 8;
	header_length += *(p++);
	p += header_length;
	return p;
}

LONGLONG parse_time_stamp(unsigned char* p){
	int b;
	LONGLONG val;

	b = *p++;
	val = (b & 0xE0) << 29;

	b = (*(p++)) << 8;
	b += *(p++);
	val += ((b & 0xFFFE) << 14);

	b = (*(p++)) << 8;
	b += *(p++);
	val += ((b & 0xFFFE) >> 1);

	return val;
}

LONGLONG parse_scr(unsigned char* p){
	int			tmp;   
	LONGLONG	ltmp;   

	tmp = *p++; //get first byte
	//get part of SCR (30..32)
	ltmp = (tmp & 0x38) << 26; //

	//get part of SCR (15..29)
	tmp <<= 8;   
	tmp += *(p++); //get second byte
	ltmp += (tmp & 0x03FF) << 19;   

	tmp <<= 8;
	tmp = *(p++);  //get third byte
	ltmp += (tmp & 0xF8) << 11; 

	tmp <<= 8;   
	tmp += *(p++);  //get fourth byte 
	ltmp += (tmp & 0x03FF) << 4; 

	tmp = *p; // 5th byte
	ltmp += (tmp & 0xf8);   
	return (ltmp);   
}

LONGLONG parse_scre(unsigned char *p){   
	int tmp;   
	LONGLONG ltmp;   

	tmp = *p++;   
	tmp <<= 8;   
	tmp += *(p++);   
	ltmp = (tmp & 0x03FE) >> 1;   
	return (ltmp);   
} 

LONGLONG parse_muxrate(unsigned char *p){   
	int			tmp;   
	LONGLONG	ltmp;   

	tmp = *p++;   
	tmp <<= 8;   
	tmp += *(p++);   
	tmp <<= 8;   
	tmp += *(p++);   
	ltmp = (tmp & 0xFFFFFC) >> 2;   
	return (ltmp);   
}

int parse_stuffing_length(unsigned char* p){
	return (*p++) & 0x07;
}

unsigned char* parse_program_stream_map(unsigned char* p){
	int head_length = 0;
	p += 4; //skip the start code
	head_length += *p++;
	head_length <<= 8;
	head_length += *p++;
	p += head_length;
	return p;
}

unsigned char* parse_packet(int stream, unsigned char *p, CircleBuffer& esbuf){     
	unsigned char	*end;   
	unsigned char	*data_start;   
	int				count;   
	int				tmp;                //32bit temperatory   
	int				flags; 
	PS_PACKET		psp;   
	memset(&psp,sizeof(PS_PACKET),0);

	p += 3;  /* skip start code*/   
	psp.id = *(p++);   
	psp.length = (*(p++)) << 8;   
	psp.length += *(p++);   

	end = p + psp.length;   

	bool isVideo = false;
	//store audio data   
	if ((stream >= FIRST_VIDEO_STREAM) && (stream <= LAST_VIDEO_STREAM)){   
		isVideo = true;  
	}
	if(!isVideo){
		goto done;
	}

	if (psp.id == PADDING_STREAM){
		goto done;  /* ignore */   
	}else{   
		flags = (*(p++)) << 8;   
		flags += *(p++);   

		psp.flag = (flags >> 14) & 0x03;     

		if (psp.flag != 2){
			goto done;
		}
		psp.hlength = *(p++);   
		//get the data start address   
		data_start = p + psp.hlength;   
		//get pts & dts   
		psp.ptsdts = (flags >> 6) & 0x03;   
		switch (psp.ptsdts){   
		case 0x10://有PTS  
			psp.pts = parse_time_stamp(p);
			p += 5;   
			break;   
		case 0x11://有PTS DTS
			psp.pts = parse_time_stamp(p);  //获取PTS
			p += 5;  
			psp.dts = parse_time_stamp (p); //获取DTS  
			p += 5;   
			break;
		case 0x00://没有DTS PTS
			break;
		default:   
			break; 
		}   

		psp.escr = (flags >> 5 ) & 1;   
		if(psp.escr == TRUE){   

			psp.escrf.escr = parse_scr(p);   
			p += 4;   
			psp.escrf.extension = (short)parse_scre(p);   
			p +=2;   
		}   

		psp.rate = (flags >> 4 ) & 1;   
		if(psp.rate == TRUE){   
			tmp = *(p++);   
			tmp <<= 8;   
			tmp += *(p++);   
			tmp <<= 8;   
			tmp += *(p++);   
			psp.ratef.rate = (tmp & 0x7ffffe) >> 1;   
		}   

		psp.trick = (flags >> 3) & 1;   
		if(psp.trick == TRUE){   
			tmp = *(p++);   
			psp.trickf.mode = (tmp & 0xe0) >> 5;   
			psp.trickf.content = tmp & 0x1f;   
		}   

		psp.add = (flags >> 2 ) & 1;   
		if(psp.add == TRUE){   
			psp.addf.info = *(p++) >> 1;   
		}   

		psp.crc = (flags >> 1 ) & 1;   
		if(psp.crc == TRUE){   
			tmp = *(p++);   
			tmp <<= 8;   
			tmp += *(p++);   
			psp.crcf.crc = tmp;   
		}   

		psp.extension = flags & 1;   
		if(psp.extension == TRUE){   
			tmp = (*(p++));   
			psp.extf.pridata = (tmp >> 7) & 1;   
			psp.extf.header = (tmp >> 6) & 1;   
			psp.extf.counter = (tmp >> 5) & 1;   
			psp.extf.buffer = (tmp >> 4) & 1;   
			psp.extf.extension = tmp & 1;   
			if(psp.extf.pridata == TRUE){   
				for(int i=0;i<16;i++){   
					psp.extf.prif.pridata[i] = *p++;   
				}   
			}   
			if(psp.extf.header == TRUE){   
				psp.extf.headerf.length = *p++;   

			}   
			if(psp.extf.counter == TRUE){   
				psp.extf.countf.counter = *p++ & 0x7f;   

			}   
			if(psp.extf.buffer == TRUE){   
				tmp = *p++;   
				tmp += *p++;   
				psp.extf.buff.scale = (tmp & 0x2000) >> 13;   
				psp.extf.buff.size = tmp & 0x1fff;   

			}   
			if(psp.extf.extension == TRUE){   
				psp.extf.extf.length = *(p++) & 0x7f;   
				p += psp.extf.extf.length;   
			}   
		}
		p = data_start;   
		count = (end - p);
		esbuf.append(p, count);
	}   
done:    
	return (end);   
} 

void parse_pack(unsigned char *buf, int size, CircleBuffer& esbuf){   
	int				stream;   
	unsigned char	*p = buf;		//   
	unsigned char	*pstart = buf;   
	int				tmp;			//32bit temperatory     
	PS_PACK			pp;   

	if(size <= PS_HEADER_LEN) return;
	
	p += 4;   //skip start code   
	//if next 2bit is not '01', an error occurs.   
	tmp = *p;
	if( ((tmp >> 6) & 0x03) != 1) return; 
	//get scr;   
	pp.scr = parse_scr(p); 

	//get scr ext;
	p += 4;   
	pp.scre = (short)parse_scre(p); 

	//get mux rate;   
	p += 2;   
	pp.rate = (int)parse_muxrate(p);

	//get stuffing length
	p += 3;
	pp.stufflength = parse_stuffing_length(p++);
	p += pp.stufflength;
 
	// parse system header;   
	if (memcmp(p, system_header_start_code, sizeof (system_header_start_code)) == 0){  
		p = parse_system_header(p);
		if(p >= buf + size) return;
	}

	if(memcmp(p, program_stream_map_start_code, sizeof(program_stream_map_start_code)) == 0){
		p = parse_program_stream_map(p);
		if(p >= buf + size) return;
	}

	// parse pes packet;   
	while (p < (buf + size)){   
		if (memcmp (p, packet_start_code_prefix, sizeof (packet_start_code_prefix)) != 0){
			return;
		} 
		stream = *(p+3);   
		p = parse_packet (stream, p, esbuf);   
	}   
	//verify    
	if (p != (buf + size)){
		//包数据不全
	}   
}  