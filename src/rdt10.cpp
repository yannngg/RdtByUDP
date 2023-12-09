//rdt1.0协议的实现文件
#include <string>
#include <cstring>
#include "../include/rdt10.h"

Rdt10::Rdt10(): UDPsocket(AF_INET, SOCK_DGRAM, 0) {}
Rdt10::~Rdt10(){ UDPsocket::~UDPsocket();}

//@brief 【原始】
//@param buf  发送消息缓冲区指针
//@param n    发送消息字节数
//@param flags    可选的标志位
//@param addr 目标地址，sockaddr结构体的指针，包含发送数据的远程主机IP地址和端口号信息
//@param addr_len 目标地址字节长度
//@return 发送成功的字节数，否则返回值小于消息字节数
ssize_t Rdt10::send(const void *buf, size_t n, int flags, const sockaddr *addr, socklen_t addr_len){
    return UDPsocket::send(buf, n, flags, addr, addr_len);
}

//@brief 【原始】
//@param buf  存储接收数据的缓冲区
//@param n    接收数据的最大长度
//@param flags    可选的标志位
//@param addr 源地址，sockaddr结构体的指针，包含发送数据的远程主机IP地址和端口号信息
//@param addr_len 存储源地址长度的指针
//@return 实际接收到的数据字节数
ssize_t Rdt10::receive(void *buf, size_t n, int flags, sockaddr *addr, socklen_t *addr_len){
    return UDPsocket::receive(buf, n, flags, addr, addr_len);
}

//@brief 【原始】
//@param addr sockaddr结构体的指针，该结构体包含了要绑定的IP地址和端口号信息
//@param len  addr指向结构体的大小
//@return   成功返回0，否则返回-1
int Rdt10::bindaddr(const sockaddr *addr, socklen_t len){
    return UDPsocket::bindaddr(addr, len);
}

//@brief 【简化】
//@param buf  发送消息缓冲区【string重载版本】
//@param flags    可选的标志位
//@param addr 目标地址，sockaddr结构体的指针，包含发送数据的远程主机IP地址和端口号信息
//@return 发送成功的字节数，否则返回值小于消息字节数
ssize_t Rdt10::send(const std::string &buf, int flags, const sockaddr *addr){
    return UDPsocket::send(buf, flags, addr);
}

//@brief 【简化】
//@param buf  存储接收数据的缓冲区【string重载版本】
//@param flags    可选的标志位
//@return 实际接收到的数据字节数
ssize_t Rdt10::receive(std::string &buf, int flags){
    return UDPsocket::receive(buf, flags);
}

//@brief 【简化】
//@param addr sockaddr结构体的指针，该结构体包含了要绑定的IP地址和端口号信息
//@return   成功返回0，否则返回-1
int Rdt10::bindaddr(const sockaddr *addr){
    return UDPsocket::bindaddr(addr);
}
