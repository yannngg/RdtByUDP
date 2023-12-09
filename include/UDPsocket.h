//自定义套接字资源管理类
#pragma once

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class UDPsocket{
protected:
    //套接字句柄
    int m_socket_fd;
public:
    //发送数据的远程主机IP地址和端口号信息
    sockaddr_in addrrecv;
    socklen_t addrrecv_len;

    //定义缓冲区大小
    static const int BUFSIZE = 1024;
    UDPsocket(int _domain = AF_INET, int _type = SOCK_DGRAM, int _protocol = 0);
    virtual ~UDPsocket();
    //@brief 判断是否成功获取套接字资源
    bool failed(){ return m_socket_fd == -1; };
    //@brief 获取套接字句柄接口
    int getSocket_fd() const{ return m_socket_fd;}
    int init(int level, int optname, const void* optval, socklen_t optlen);

    
    virtual ssize_t send(const void* buf, size_t n, int flags, const sockaddr* addr, socklen_t addr_len);
    virtual ssize_t receive(void* buf, size_t n, int flags, sockaddr* addr, socklen_t* addr_len);
    virtual int bindaddr(const sockaddr* addr, socklen_t len);
    
    virtual ssize_t send(const std::string& buf, int flags, const sockaddr* addr);
    virtual ssize_t receive(std::string& buf, int flags);
    virtual int bindaddr(const sockaddr* addr);
};

std::string uint32_t_to_string(uint32_t addr);
uint16_t calculateChecksum(const std::string& data);
std::string uint16ToTwoByteString(uint16_t value);
uint16_t twoByteStringToUint16(const std::string& byteString);
void modify_uint16_t_with_probability(uint16_t &value, double probability);