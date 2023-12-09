//UDPsocket.hpp的实现文件
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include "../include/UDPsocket.h"


//@brief UDPsocket类构造函数，内部实现分配套接字资源
//@param domain 协议族
//@param type   数据传输类型
//@param protocol   协议信息
//@return 返回自定义套接字对象
UDPsocket::UDPsocket(int domain, int type, int protocol){
    //创建套接字
    m_socket_fd = socket(domain, type, protocol);
    //初始化
    addrrecv = {0};
    addrrecv_len = sizeof(sockaddr);
}

//@brief UDPsocket类析构函数，内部实现关闭套接字资源
UDPsocket::~UDPsocket(){
    //关闭套接字
    close(m_socket_fd);
}

//@brief 设置套接字属性
int UDPsocket::init(int level, int optname, const void* optval, socklen_t optlen){
    return setsockopt(m_socket_fd, level, optname, optval, optlen);
}

//@brief 【原始版本】发送报文，内部实现调用sendto接口
//@param buf  发送消息缓冲区指针
//@param n    发送消息字节数
//@param flags    可选的标志位
//@param addr 目标地址，sockaddr结构体的指针，包含发送数据的远程主机IP地址和端口号信息
//@param addr_len 目标地址字节长度
//@return 发送成功的字节数，否则返回值小于消息字节数
ssize_t UDPsocket::send(const void* buf, size_t n, int flags, const sockaddr* addr, socklen_t addr_len){
    return sendto(m_socket_fd, buf, n, flags, addr, addr_len);
}

//@brief 【原始版本】接收报文，内部实现调用recvfrom接口
//@param buf  存储接收数据的缓冲区
//@param n    接收数据的最大长度
//@param flags    可选的标志位
//@param addr 源地址，sockaddr结构体的指针，包含发送数据的远程主机IP地址和端口号信息
//@param addr_len 存储源地址长度的指针
//@return 实际接收到的数据字节数
ssize_t UDPsocket::receive(void* buf, size_t n, int flags, sockaddr* addr, socklen_t* addr_len)
{
    return recvfrom(m_socket_fd, buf, n, flags, addr, addr_len);
}

//@brief 【原始版本】将套接字与指定IP和端口号进行绑定，内部实现调用bind方法
//@param addr sockaddr结构体的指针，该结构体包含了要绑定的IP地址和端口号信息
//@param len  addr指向结构体的大小
//@return   成功返回0，否则返回-1
int UDPsocket::bindaddr(const sockaddr* addr, socklen_t len){
    return bind(m_socket_fd, addr, len);
}

//@brief 【简化版本】发送报文，内部实现调用sendto接口
//@param buf  发送消息缓冲区【string重载版本】
//@param flags    可选的标志位
//@param addr 目标地址，sockaddr结构体的指针，包含发送数据的远程主机IP地址和端口号信息
//@return 发送成功的字节数，否则返回值小于消息字节数
ssize_t UDPsocket::send(const std::string& buf, int flags, const sockaddr* addr){
    return sendto(m_socket_fd, buf.data(), buf.size(), flags, addr, sizeof(*addr));
}


//@brief 【简化版本】接收报文，内部实现调用recvfrom接口
//@param buf  存储接收数据的缓冲区【string重载版本】
//@param flags    可选的标志位
//@return 实际接收到的数据字节数
ssize_t UDPsocket::receive(std::string& buf, int flags){
    return recvfrom(m_socket_fd, buf.data(), buf.size(), flags,
    (sockaddr*)&addrrecv, &addrrecv_len);
}

//@brief 【简化版本】将套接字与指定IP和端口号进行绑定，内部实现调用bind方法
//@param addr sockaddr结构体的指针，该结构体包含了要绑定的IP地址和端口号信息
//@return   成功返回0，否则返回-1
int UDPsocket::bindaddr(const sockaddr* addr){
    return bind(m_socket_fd, addr, sizeof(*addr));
}

//@brief 将32位IP地址转string
//@param addr: 32位ip地址
//@return string形式的IP地址
std::string uint32_t_to_string(uint32_t addr){
    uint8_t* ptr = (uint8_t*)&addr;
    return std::to_string(*ptr)+"."+std::to_string(*(ptr+1))
        +"."+std::to_string(*(ptr+2))+"."+std::to_string(*(ptr+3));
}

//@brief 计算校验和
uint16_t calculateChecksum(const std::string& data) {
    uint32_t sum = 0;
    for (char c : data) {
        sum += static_cast<uint8_t>(c);
    }
    return static_cast<uint16_t>(sum);
}

//@brief 将uint16_t转换为2字节的字符串
std::string uint16ToTwoByteString(uint16_t value) {
    std::stringstream ss;
    ss.write(reinterpret_cast<const char*>(&value), sizeof(value));
    return ss.str();
}
//@brief 将字符串前两个字节转uint16_t
uint16_t twoByteStringToUint16(const std::string& byteString) {
    std::stringstream ss;
    ss.write(byteString.data(), 2);
    uint16_t value;
    ss.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}

//@brief 将字符串第一个字节转uint8_t
uint8_t firstByteStringToUint8(const std::string& byteString) {
    std::stringstream ss;
    ss.write(byteString.data(), 1);
    uint8_t value;
    ss.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}

//@brief 以给定概率修改uint16_t变量的值，概率取0~1
void modify_uint16_t_with_probability(uint16_t &value, double probability) {
    if (static_cast<double>(rand()) / RAND_MAX < probability) {
        value = static_cast<uint16_t>(rand());
    }
}