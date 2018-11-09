#ifndef _RESP_BUILDER_H_
#define _RESP_BUILDER_H_

#include "SmartBuffer.h"

class RespBuilder{
public:
    RespBuilder() : _buf(1024){
        _buf.set_data_size(6);//预分配5个字节长度，留给*xxx\r\n,xx代表后面数组长度 
        _crlf[0] = '\r'; _crlf[1] = '\n'; _separate[0] = ',';
    }

    RespBuilder(size_t size) : _buf(size){
        _buf.set_data_size(6);//预分配5个字节长度，留给*xx\r\n,xx代表后面数组长度 
        _crlf[0] = '\r'; _crlf[1] = '\n'; _separate[0] = ',';
    }

    ~RespBuilder(){}

    void add_property_string(const char* keyName, const char* value){
        if (keyName == nullptr || value == nullptr) return;
        int keyLen = strlen(keyName);
        int valueLen = strlen(value);
        if (keyLen > 128) return; //属性的key大小不能超过128个字节
        char buf[32] = { 0 };
        sprintf(buf, "$%d", keyLen + valueLen + 1);
        _buf.append(buf, strlen(buf));
        _buf.append(_crlf, 2);
        _buf.append(keyName, keyLen);
        _buf.append(_separate, 1);
        _buf.append(value, valueLen);
        _buf.append(_crlf, 2);
        _property_num++;
    }
    void add_property_string(const char* keyName, const char* value, std::size_t size){
        if (keyName == nullptr || value == nullptr) return;
        int keyLen = strlen(keyName);
        if (keyLen > 128) return;
        char buf[32] = { 0 };
        sprintf(buf, "$%d", keyLen + size + 1);
        _buf.append(buf, strlen(buf));
        _buf.append(_crlf, 2);
        _buf.append(keyName, keyLen);
        _buf.append(_separate, 1);
        _buf.append(value, size);
        _buf.append(_crlf, 2);
        _property_num++;
    }

    void add_property_int(const char* keyName, int value){
        if (keyName == nullptr || strlen(keyName) > 128) return;
        char szValue[32] = { 0 }; sprintf(szValue, "%d", value);
        char tmp[160] = { 0 };
        sprintf(tmp, "$%d\r\n%s,%s\r\n", strlen(keyName) + strlen(szValue) + 1, keyName, szValue);
        _buf.append(tmp, strlen(tmp));
        _property_num++;
    }

    void add_property_binary(const char* data, int length){
        char buf[32] = { 0 };
        sprintf(buf, "#%d\r\n", length);
        _buf.append(buf, strlen(buf));
        _buf.append(data, length);
        _buf.append(_crlf, 2);
        _property_num++;
    }

    void add_property_binary_ex(const char* keyName, const char* data, int length){
        char buf[128] = { 0 };
        sprintf(buf, "#%d\r\n%s,", strlen(keyName) + length + 1, keyName);
        _buf.append(buf, strlen(buf));
        _buf.append(data, length);
        _buf.append(_crlf, 2);
        _property_num++;
    }

    const char* resp_print(){
        if (_property_num > 0){
            char buf[32] = { 0 };
            sprintf(buf, "*%03d\r\n", _property_num);
            if (strlen(buf) == 6){
                memcpy((void*)_buf.buff(), buf, 6);
                return _buf.buff();
            }
        }
        _buf.reset();
        return _buf.buff();
    }

    size_t resp_size(){ return _buf.size(); };
private:
    char            _crlf[2];
    char            _separate[1];
    SmartBuffer     _buf;
    int             _property_num{ 0 };
    int             _binary_len{ 0 };
};

#endif /*RESP_BUILDER_H_*/
