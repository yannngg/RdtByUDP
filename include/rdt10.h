//rdt1.0协议头文件
//数据字段前两个字节为chksum字段
#pragma once

#include "UDPsocket.h"

class Rdt10: public UDPsocket{
public:
    Rdt10();
    ~Rdt10();

    ssize_t send(const void* buf, size_t n, int flags, const sockaddr* addr, socklen_t addr_len);
    ssize_t receive(void* buf, size_t n, int flags, sockaddr* addr, socklen_t* addr_len);
    int bindaddr(const sockaddr* addr, socklen_t len);

    ssize_t send(const std::string& buf, int flags, const sockaddr* addr);
    ssize_t receive(std::string& buf, int flags);
    int bindaddr(const sockaddr* addr); 
};