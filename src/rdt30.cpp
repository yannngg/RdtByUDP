//rdt2.2协议实现文件
#include <string>
#include <cstring>
#include <cmath>
#include <ctime>
#include "../include/rdt30.h"

const double Rdt30::probability_bit_error = 0.25;
const double Rdt30::probability_drop_package = 0.25;
const time_t Rdt30::timer = 2;

Rdt30::Rdt30() : UDPsocket(AF_INET, SOCK_DGRAM, 0) { state = 0;}
Rdt30::~Rdt30(){ UDPsocket::~UDPsocket();}

//@brief 数据发送方发送数据，以一定概率发送失败模拟丢包
ssize_t Rdt30::rdt30Send(const std::string &buf, int flags, const sockaddr *addr){
    std::string message, seq, flagByte;
    uint16_t chksum = 0;
    ssize_t res = 0;

    std::cout << "state: " << (int)state << std::endl;

    //标志位置0
    flagByte = std::string(1, 0);
    //报文序号
    seq = std::string(1, state);
    message = flagByte + seq + buf;
    chksum = calculateChecksum(message);
    
    //模拟bit_error
    modify_uint16_t_with_probability(chksum, probability_bit_error);
    message = uint16ToTwoByteString(chksum)+ message;

    //模拟drop_package
    res = send_package_with_probability_drop(message, 0, addr, probability_drop_package);
    if (res < 0) std::cout << "【send】send message: " << (int)state << " but has dropped" << std::endl;
    else std::cout << "【send】send message: " << (int)state << " successfully" << std::endl;

    //停等
    std::string recv_message(BUFSIZE, 0);
    ssize_t recv_len = 0;
    //接收ACK
    recv_len = ackReceive(recv_message, 0);
    //std::cout << "recv_len: "<< recv_len << std::endl;

    //若消息超时、被篡改或不是目标ACK，重发数据
    while (recv_len == -1 || recv_len == -2 || recv_message[1] != char(state)){
        if (recv_len == -1)
            std::cout << "【time】time out, send message: " << (int)state << " again" << std::endl;
        else if (recv_len == -2)
            std::cout << "【recv】ACK has been modified" << std::endl;
        else if ( recv_message[1] != char(state))
            std::cout << "【recv】received redundant ACK: " << (int)recv_message[1] << std::endl;

        //重新计算
        message = flagByte + seq + buf;
        chksum = calculateChecksum(message);
        //模拟不可靠信道
        modify_uint16_t_with_probability(chksum, probability_bit_error);
        message = uint16ToTwoByteString(chksum)+ message;

        //模拟drop_package
        res = send_package_with_probability_drop(message, 0, addr, probability_drop_package);
        if (res < 0) std::cout << "【send】send message: " << (int)state << " again, but has dropped" << std::endl;
        else std::cout << "【send】send message: "<< (int)state << " again"<< std::endl;

        recv_len = ackReceive(recv_message, 0);
    }
    std::cout << "【recv】received ACK: " << (int)state << " enter state: " << (state+1) % 2<< std::endl;
    //更新状态
    state = (state+1) % 2;
    return res;
}

//@brief 数据发送方接收ACK
ssize_t Rdt30::ackReceive(std::string& buf, int flags){
    std::string recv_message(BUFSIZE, 0);

    size_t res = UDPsocket::receive(recv_message, flags);
    //超时情况
    if (res == -1) return -1;
    uint16_t recvChkSum = twoByteStringToUint16(recv_message);
    recv_message = recv_message.substr(2);
    //比较检验和
    if (recvChkSum != calculateChecksum(recv_message))
        return -2;

    buf = recv_message;
    return res;
}

//@brief 数据接收方发送ACK
ssize_t Rdt30::ackSend(const std::string& buf, int flags, const sockaddr* addr){
    std::string message, seq, flagByte;
    flagByte = std::string(1, 1);
    seq = std::string(1, buf[0]);
    message = flagByte+ seq;
    uint16_t chksum = calculateChecksum(message);

    //模拟不可靠信道
    modify_uint16_t_with_probability(chksum, probability_bit_error);
    message = uint16ToTwoByteString(chksum)+ message;

    ssize_t res = 0;
    res = UDPsocket::send(message, flags, addr) >= 0;

    if (res < 0) {
        std::cerr << "【error】send ACK error" << std::endl;
        return -1;
    }
    //不需要停等
    return res;
}

//@brief 数据接收方接收数据
ssize_t Rdt30::rdt30Receive(std::string &buf, int flags){
    std::string recv_message(BUFSIZE, 0);
    ssize_t ack_res = 0;
    size_t res = UDPsocket::receive(recv_message, flags);
    if (res < 0) return -1;

    std::cout << "state: " << (int)state << std::endl;

    uint16_t recvChkSum = twoByteStringToUint16(recv_message);
    recv_message = recv_message.substr(2);

    //数据被篡改或非目标报文
    if (recvChkSum != calculateChecksum(recv_message) || recv_message[1] != char(state)){
        //若数据被篡改
        if (recvChkSum != calculateChecksum(recv_message)){
            std::cout << "【recv】message has been modified" << std::endl;
            ack_res = ackSend(std::string(char((state+1)%2), 1), 0, (sockaddr*)&addrrecv);
            if (ack_res != -1)
                std::cout << "【send】send ACK: " << ((int)state+1)%2 << " successfully"<< std::endl;
        }
        else{
            std::cout << "【recv】received redundant message: "<< ((int)state+1)%2 << std::endl;
            ack_res = ackSend(std::string(char((state+1)%2), 1), 0, (sockaddr*)&addrrecv);
            if (ack_res != -1)
                std::cout << "【send】send ACK: " << ((int)state+1)%2 << " successfully"<< std::endl;
        }
        return -1;
    }

    std::cout << "【recv】received message: "<< (int)state << " enter state: "<< ((int)state+1)%2<< std::endl;
    ackSend(std::string(char(state), 1), 0, (sockaddr*)&addrrecv);
    std::cout << "【send】send ACK: " << (int)state << " successfully"<< std::endl;

    //更新状态
    state = (state+1)%2;
    //否则，获取向上传出数据
    buf = recv_message.substr(2);

    return res -4;
}

//@brief 【原始】
//@param addr sockaddr结构体的指针，该结构体包含了要绑定的IP地址和端口号信息
//@param len  addr指向结构体的大小
//@return   成功返回0，否则返回-1
int Rdt30::bindaddr(const sockaddr *addr, socklen_t len){
    return UDPsocket::bindaddr(addr, len);
}

//@brief 【简化】
//@param addr sockaddr结构体的指针，该结构体包含了要绑定的IP地址和端口号信息
//@return   成功返回0，否则返回-1
int Rdt30::bindaddr(const sockaddr *addr){
    return UDPsocket::bindaddr(addr);
}

//@brief 模拟丢包场景，返回-1表示丢包
ssize_t Rdt30::send_package_with_probability_drop(const std::string &buf, int flags, const sockaddr *addr, double probability){
    if (static_cast<double>(rand()) / RAND_MAX < probability)
        return -1;
    return UDPsocket::send(buf, flags, addr);
}