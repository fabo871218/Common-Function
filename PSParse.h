#pragma once

#include "CircleBuffer.h"

#define PS_HEADER_LEN					14 //PS包头部长度

typedef struct _PS_ESCR{
	LONGLONG	escr;
	short		extension;
}PS_ESCR;

typedef struct _PS_ESRATE{
	int			rate;
}PS_ESRATE;

typedef struct _PS_TRICK{
	short		mode;
	short		content;
}PS_TRICK;

typedef struct _PS_ADD{
	char		info;
}PS_ADD;

typedef struct _PS_CRC{
	short		crc;
}PS_CRC;

typedef struct _PS_EXT_PRIDATA{   
	char pridata[16];   //PES_private_data   
}PS_EXT_PRIDATA;  

typedef struct _PS_EXT_HEADER{   
	short length;       //pack_field_length   
}PS_EXT_HEADER;   

typedef struct _PS_EXT_CONTER{   
	short counter;      //program_packet_sequence_counter   
	bool identifier;    //MPEG1_MPEG2_identifier   
	short length;       //original_stuff_length   
}PS_EXT_CONTER;   

typedef struct _PS_EXT_BUFFER{   
	int		scale;         //P-STD_buffer_scale   
	short	size;         //P-STD_buffer_size   
}PS_EXT_BUFFER;  

typedef struct _PS_EXT_EXT{   
	short length;       //PES_extension_field_length   
}PS_EXT_EXT; 

typedef struct _PS_EXT{   
	bool pridata;       //PES_private_data_flag   
	bool header;        //pack_header_field_flag   
	bool counter;       //program_packet_sequence_counter_flag   
	bool buffer;        //P-STD_buffer_flag   
	bool extension;     //PES_extension_flag_2   
	PS_EXT_PRIDATA prif;        //PES_private_data_field   
	PS_EXT_HEADER headerf;      //pack_header_field_field   
	PS_EXT_CONTER countf;      //program_packet_sequence_counter_field   
	PS_EXT_BUFFER buff;         //P-STD_buffer_field   
	PS_EXT_EXT extf;            //PES_extension_field   

}PS_EXT;   
//PES packet   
typedef struct _PS_PACKET{   
	unsigned char id;  //stream_id   
	short length;      //PES_packet_length   
	short flag;        //'10'   
	short scramble;    //PES_scrambling_control   
	bool priority;     //PES_priority   
	bool align;        //data_alignment_indicator   
	bool copyright;    //copyright   
	bool origin;       //original_or_copy   
	short ptsdts;      //PTS_DTS_flags   
	bool escr;         //ESCR_flag   
	bool rate;         //ES_rate_flag   
	bool trick;        //DSM_trick_mode_flag   
	bool add;          //additional_copy_info_flag   
	bool crc;          //PES_CRC_flag   
	bool extension;    //PES_extension_flag   
	short hlength;     //PES_header_data_length   
	LONGLONG pts;          
	LONGLONG dts;   
	PS_ESCR escrf;     //ESCR_field   
	PS_ESRATE ratef;   //ES_rate_field   
	PS_TRICK trickf;   //DSM_trick_mode_field   
	PS_ADD addf;       //additional_copy_info_field    
	PS_CRC crcf;       //PES_CRC_field   
	PS_EXT extf;       //PES_extension_field       

}PS_PACKET;   
//////////////////////////////////////////////////////////////////////////////   

////////////////////////////////////////////////////////////////////////////////   
//Pack structure   
typedef struct _PS_PACK{   
	LONGLONG scr;       //system_clock_reference_base   
	short scre;         //system_clock_reference_extension   
	int rate;           //program_mux_rate   
	short stufflength;  //pack_stuffing_length   
}PS_PACK;   

#define STREAM_MAPPING     0xbc   
#define PRIVATE_STREAM_1   0xbd   
#define PADDING_STREAM     0xbe   
#define PRIVATE_STREAM_2   0xbf   

#define FIRST_AUDIO_STREAM 0xc0   
#define LAST_AUDIO_STREAM  0xdf   

#define FIRST_VIDEO_STREAM 0xe0   
#define LAST_VIDEO_STREAM  0xef   

#define STREAM_DIRECTORY   0xff 

void parse_pack(unsigned char *buf, int size, CircleBuffer& esbuf);