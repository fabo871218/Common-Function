#ifndef RESP_PARSER_H_
#define RESP_PARSER_H_

#define RESP_OK     0
#define RESP_FAILED    1
#define RESP_ERR_INVALID_START 2
#define RESP_ERR_INVALID_ARR_SIZE 3
#define RESP_ERR_INVALID_NUMBER 4
#define RESP_ERR_INVALID_ARR_END 5
#define RESP_ERR_INVALID_PROPERTY_END 6
#define RESP_ERR_INVALID_ELEMENT_START 7
#define RESP_ERR_INVALID_PROPERTY_SIZE 8
#define RESP_ERR_INVALID_PICTURE_SIZE 9
#define RESP_ERR_INVALID_PICTURE_END 10

#include <functional>
#include "SmartBuffer.h"


typedef std::function<int()> resp_cb;
typedef std::function<int(const char* data, int length)> resp_data_cb;

class RespParser{
public:
    void parser_init(resp_cb parser_begin,
        resp_data_cb parser_property,
        resp_cb parser_complete,
        resp_data_cb parser_picture);
    void parser_reset();
    int parser_excute(const char* data, size_t length);
    int parser_error(){ return error; }

private:
    int             state;
    int             arrsize;
    size_t          content_length;
    int             error;
    SmartBuffer     circle_buf;
    int             property_length;

    resp_cb         on_parser_begin;
    resp_data_cb    on_parser_property;
    resp_cb         on_parser_complete;
    resp_data_cb    on_parser_picture;
};

#endif /*RESP_PARSER_H_*/