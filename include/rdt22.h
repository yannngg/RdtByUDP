//rdt2.2协议头文件
//数据字段前两个字节为chksum字段，接着两个字节分别为flag、seq字段
#pragma once

#include "UDPsocket.h"

class Rdt22: public UDPsocket{
    static const double probability;
    uint8_t state;
public:
    Rdt22();
    ~Rdt22();

    int bindaddr(const sockaddr* addr, socklen_t len);
    int bindaddr(const sockaddr* addr); 

    ssize_t ackSend(const std::string& buf, int flags, const sockaddr* addr);
    ssize_t rdt22Receive(std::string& buf, int flags);
    ssize_t rdt22Send(const std::string& buf, int flags, const sockaddr* addr);
    ssize_t ackReceive(std::string& buf, int flags);
};