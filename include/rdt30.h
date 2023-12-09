//rdt3.0协议头文件
//数据字段前两个字节为chksum字段，接着两个字节分别为flag、seq字段
#pragma once

#include "UDPsocket.h"

class Rdt30: public UDPsocket{
    static const double probability_bit_error;
    static const double probability_drop_package;
    uint8_t state;
public:
    static const time_t timer;

    Rdt30();
    ~Rdt30();

    int bindaddr(const sockaddr* addr, socklen_t len);
    int bindaddr(const sockaddr* addr); 

    ssize_t ackSend(const std::string& buf, int flags, const sockaddr* addr);
    ssize_t rdt30Receive(std::string& buf, int flags);
    ssize_t rdt30Send(const std::string& buf, int flags, const sockaddr* addr);
    ssize_t ackReceive(std::string& buf, int flags);

    ssize_t send_package_with_probability_drop(const std::string &buf, int flags, const sockaddr *addr, double probability);
};

