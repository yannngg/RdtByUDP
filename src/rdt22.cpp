//rdt2.2协议实现文件
#include <string>
#include <cstring>
#include "../include/rdt22.h"

//不可靠信道bit_error概率
const double Rdt22::probability = 0.25;

Rdt22::Rdt22(): UDPsocket(AF_INET, SOCK_DGRAM, 0) { state = 0; }
Rdt22::~Rdt22(){ UDPsocket::~UDPsocket();}

//@brief 数据发送方发送数据
ssize_t Rdt22::rdt22Send(const std::string &buf, int flags, const sockaddr *addr){
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
    
    //模拟bit_error场景
    modify_uint16_t_with_probability(chksum, probability);
    message = uint16ToTwoByteString(chksum)+ message;

    res = UDPsocket::send(message, flags, addr);
    
    std::cout << "【send】send message: " << (int)state << " successfully" << std::endl;

    //停等
    std::string recv_message(BUFSIZE, 0);
    //接收ACK
    ssize_t recv_len = ackReceive(recv_message, 0);

    //若消息被篡改或不是目标ACK，重发数据
    while (recv_len == -1 || recv_message[1] != char(state)){
        if (recv_len == -1) 
            std::cout << "【recv】ACK has been modified" << std::endl;
        else if (recv_message[1] != char(state))
            std::cout << "【recv】received redundant ACK:" << (int)recv_message[1] << std::endl;

        //重新计算
        message = flagByte + seq + buf;
        chksum = calculateChecksum(message);
        //模拟不可靠信道
        modify_uint16_t_with_probability(chksum, probability);
        message = uint16ToTwoByteString(chksum)+ message;

        res = UDPsocket::send(message, flags, addr);
        std::cout << "【send】send message: "<< (int)state << " again"<< std::endl;
        recv_len = ackReceive(recv_message, 0);
    }
    std::cout << "【recv】received ACK: " << (int)state << " enter state: " << (state+1) % 2<< std::endl;
    //更新状态
    state = (state+1) % 2;
    return res;
}

//@brief 数据发送方接收ACK
ssize_t Rdt22::ackReceive(std::string& buf, int flags){
    std::string recv_message(BUFSIZE, 0);

    size_t res = UDPsocket::receive(recv_message, flags);
    uint16_t recvChkSum = twoByteStringToUint16(recv_message);
    recv_message = recv_message.substr(2);
    //比较检验和
    if (recvChkSum != calculateChecksum(recv_message))
        return -1;

    buf = recv_message;
    return res;
}

//@brief 数据接收方发送ACK
ssize_t Rdt22::ackSend(const std::string& buf, int flags, const sockaddr* addr){
    std::string message, seq, flagByte;
    flagByte = std::string(1, 1);
    seq = std::string(1, buf[0]);
    message = flagByte+ seq;
    uint16_t chksum = calculateChecksum(message);

    //模拟不可靠信道
    modify_uint16_t_with_probability(chksum, probability);
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
ssize_t Rdt22::rdt22Receive(std::string &buf, int flags){
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
int Rdt22::bindaddr(const sockaddr *addr, socklen_t len){
    return UDPsocket::bindaddr(addr, len);
}

//@brief 【简化】
//@param addr sockaddr结构体的指针，该结构体包含了要绑定的IP地址和端口号信息
//@return   成功返回0，否则返回-1
int Rdt22::bindaddr(const sockaddr *addr){
    return UDPsocket::bindaddr(addr);
}