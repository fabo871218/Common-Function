#include "RespParser.h"

#define MAX_PROPERTY_LENGTH 256

#define GOTO_BEGIN() goto parse_begin;

#define GOTO_END() goto parse_end;

#define GOTO_END_STEP_ONE() \
    p++;           \
    goto parse_end;

enum state{
    s_resp_dead = 1,
    s_resp_array_size, //��ʼ�������鳤��
    s_resp_array_size_end_lf,
    s_resp_property_size, //������һ�������ֶ�
    s_resp_property_size_end_lf, //һ���������ݵ�\r\n�ָ���,����$14\r\nabcdefghijklmn\r\n ��һ��\r\n����s_resp_property_sperate������һ������s_resp_property_end
    s_resp_property_content, //�����ֶν������
    s_resp_property_end_cr, // \r
    s_resp_property_end_lf, // \n
    s_resp_picture_size, //������ͼƬ
    s_resp_picture_size_end_lf,
    s_resp_picture_content, //����ͼƬ���
    s_resp_picture_end_cr,
    s_resp_picture_end_lf,
    s_resp_element_start,
};

void RespParser::parser_init(resp_cb parser_begin,
    resp_data_cb parser_property,
    resp_cb parser_complete,
    resp_data_cb parser_picture){
    state = s_resp_dead;
    arrsize = 0;
    content_length = 0;
    circle_buf.reset();
    error = RESP_OK;

    on_parser_begin = parser_begin;
    on_parser_complete = parser_complete;
    on_parser_property = parser_property;
    on_parser_picture = parser_picture;
}

void RespParser::parser_reset(){
    state = s_resp_dead;
    arrsize = 0;
    content_length = 0;
    error = RESP_OK;
    circle_buf.reset();
}

int RespParser::parser_excute(const char* data, size_t length){
    if (error != RESP_OK || length == 0)
        return 0;

    const char* p = data;
    const char* end = data + length;
    char ch;
    for (p = data; p < data + length; p++){
    parse_begin:
        ch = *p;
        switch (state){
        case s_resp_dead:{
            if (ch != '*'){
                error = RESP_ERR_INVALID_START;
                GOTO_END_STEP_ONE();
            }
            else{
                on_parser_begin(); //������ʼ
                state = s_resp_array_size;
            }
        }break;
        case s_resp_array_size:{
            if (ch == '\r'){
                circle_buf.append_string_end();
                arrsize = atoi(circle_buf.buff());
                if (arrsize == 0){
                    error = RESP_ERR_INVALID_ARR_SIZE;
                    GOTO_END_STEP_ONE();
                }
                circle_buf.reset();
                state = s_resp_array_size_end_lf;
            }
            else{
                if (ch < '0' || ch > '9'){
                    error = RESP_ERR_INVALID_NUMBER;
                    GOTO_END_STEP_ONE();
                }
                circle_buf.append(p, 1);
            }
        }break;
        case s_resp_array_size_end_lf:{
            if (ch == '\n'){
                state = s_resp_element_start;
            }
            else{
                error = RESP_ERR_INVALID_ARR_END;
                GOTO_END_STEP_ONE();
            }
        }break;
        case s_resp_property_end_cr:{
            if (ch == '\r'){
                state = s_resp_property_end_lf;
            }
            else{
                error = RESP_ERR_INVALID_PROPERTY_END;
                GOTO_END_STEP_ONE();
            }
        }break;
        case s_resp_property_end_lf:{
            if (ch == '\n'){
                arrsize--;
                if (arrsize == 0){
                    on_parser_complete(); //�ص��������
                    state = s_resp_dead;
                    GOTO_END_STEP_ONE();
                }
                else{
                    state = s_resp_element_start;
                }
            }
            else{
                error = RESP_ERR_INVALID_PROPERTY_END;
                GOTO_END_STEP_ONE();
            }
        }break;
        case s_resp_element_start:{
            if (ch == '$'){
                state = s_resp_property_size;
            }
            else if (ch == '#'){
                state = s_resp_picture_size;
            }
            else{
                error = RESP_ERR_INVALID_ELEMENT_START;
                GOTO_END_STEP_ONE();
            }
        }break;
        case s_resp_property_size:{
            if (ch == '\r'){
                circle_buf.append_string_end();
                content_length = atoi(circle_buf.buff());
                if (content_length == 0){
                    error = RESP_ERR_INVALID_PROPERTY_SIZE;
                    GOTO_END_STEP_ONE();
                }
                circle_buf.reset();
                state = s_resp_property_size_end_lf;
            }
            else{
                if (ch < '0' || ch > '9'){
                    error = RESP_ERR_INVALID_NUMBER;
                    GOTO_END_STEP_ONE();
                }
                circle_buf.append(p, 1);
            }
        }break;
        case s_resp_property_size_end_lf:{
            if (ch == '\n'){
                state = s_resp_property_content;
            }
        }break;
        case s_resp_property_content:{
            if (p + content_length > end){
                int nread = end - p;
                circle_buf.append(p, nread);
                content_length -= nread;
                p += nread;
            }
            else{
                circle_buf.append(p, content_length);
                circle_buf.append_string_end();
                p += content_length;
                on_parser_property(circle_buf.buff(), circle_buf.size()); //���Իص�
                circle_buf.reset();
                state = s_resp_property_end_cr;
                content_length = 0;
            }
            if (p >= end){
                GOTO_END();
            }
            else{
                GOTO_BEGIN();
            }
        }break;
        case s_resp_picture_size:{
            if (ch == '\r'){
                circle_buf.append_string_end();
                content_length = atoi(circle_buf.buff());
                if (content_length < 0){
                    error = RESP_ERR_INVALID_PICTURE_SIZE;
                    GOTO_END_STEP_ONE();
                }
                circle_buf.reset();
                state = s_resp_picture_size_end_lf;
            }
            else{
                if (ch < '0' || ch > '9'){
                    error = RESP_ERR_INVALID_NUMBER;
                    GOTO_END_STEP_ONE();
                }
                circle_buf.append(p, 1);
            }
        }break;
        case s_resp_picture_size_end_lf:{
            if (ch == '\n'){
                state = s_resp_picture_content;
            }
        }break;
        case s_resp_picture_content:{
            if (p + content_length > end){
                int nread = end - p;
                on_parser_picture(p, nread); //�ص�ͼƬ����
                content_length -= nread;
                p += nread;
            }
            else{
                on_parser_picture(p, content_length); //�ص�ͼƬ����
                p += content_length;
                state = s_resp_picture_end_cr;
                content_length = 0;
            }

            if (p >= end){
                GOTO_END();
            }
            else{
                GOTO_BEGIN();
            }
        }break;
        case s_resp_picture_end_cr:{
            if (ch == '\r'){
                state = s_resp_picture_end_lf;
            }
            else{
                error = RESP_ERR_INVALID_PICTURE_END;
                GOTO_END_STEP_ONE();
            }
        }break;
        case s_resp_picture_end_lf:{
            if (ch == '\n'){
                arrsize--;
                if (arrsize == 0){
                    on_parser_complete(); //�ص��������
                    state = s_resp_dead;
                    GOTO_END_STEP_ONE();
                }
                else{
                    state = s_resp_element_start;
                }
            }
            else{
                error = RESP_ERR_INVALID_PICTURE_END;
                GOTO_END_STEP_ONE();
            }
        }break;
        default:
            error = RESP_FAILED;
            GOTO_END_STEP_ONE();
        }
    }

parse_end:
    return p - data;
}