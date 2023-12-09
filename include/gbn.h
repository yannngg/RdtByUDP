//gbn.h协议头文件
//数据字段前两个字节为chksum字段，接着两个字节为flag字段、seq字段
//flag=0正常报文 flag=1结束报文
//窗口大小为 2**8 - 1 = 255
#pragma once

#include <queue>
#include "UDPsocket.h"

class GBN: public UDPsocket{

    static const double probability_bit_error;
    static const double probability_drop_package;
    static const double time_out;//以秒为单位
    static const double stop_delay_time;//以秒为单位

    static const uint64_t max_fragment_size;
    
    static const uint8_t encode_size;
    static const uint8_t window_size;

    uint8_t send_base;
    uint8_t next_seqnum;
    uint8_t expected_seqnum;

    std::queue<std::string> send_buffer;

    ssize_t send_package_with_probability_drop(const std::string &buf, int flags, const sockaddr *addr, double probability);
public:

    GBN();
    ~GBN();

    int bindaddr(const sockaddr* addr, socklen_t len);
    int bindaddr(const sockaddr* addr); 

    ssize_t gbnSend(const std::string& buf, const sockaddr* addr);
    ssize_t gbnRecv(std::string& buf);

    
};
