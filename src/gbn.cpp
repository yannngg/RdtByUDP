//gbn协议实现文件
#include <string>
#include <cstring>
#include <cmath>
#include <ctime>
#include "../include/gbn.h"

#define DEBUG

const double GBN::probability_bit_error = 0;
const double GBN::probability_drop_package = 0;
const uint8_t GBN::encode_size = 255;//实际值+1
const uint8_t GBN::window_size = 255;
const uint64_t GBN::max_fragment_size = 1024;
const double GBN::time_out = 1e-4;
const double GBN::stop_delay_time = 1e-3;


GBN::GBN() : UDPsocket(AF_INET, SOCK_DGRAM, 0),send_base(1), next_seqnum(1), expected_seqnum(1){}
GBN::~GBN(){ UDPsocket::~UDPsocket();}


int GBN::bindaddr(const sockaddr *addr, socklen_t len){
    return UDPsocket::bindaddr(addr, len);
}

int GBN::bindaddr(const sockaddr *addr){
    return UDPsocket::bindaddr(addr);
}

//阻塞式发送，确保接收方接收到发送的消息后才返回，返回发送数据长度
ssize_t GBN::gbnSend(const std::string& buf, const sockaddr* addr){
    bool begin = true;

    unsigned packet_num = (buf.size() + max_fragment_size - 1)/max_fragment_size;
    uint64_t sent_packet = 0;
    clock_t send_start_time = 0;
    while(true){
        //超时重发
        if(send_base != next_seqnum){
            if(1.0*(clock() - send_start_time)/CLOCKS_PER_SEC >= time_out){
                size_t queue_size = send_buffer.size();
                for(size_t i=0; i<queue_size; i++){
                    std::string tmp_packet = send_buffer.front();
                    send_buffer.pop();
                    send_buffer.push(tmp_packet);

                    uint16_t tmpChkSum = twoByteStringToUint16(tmp_packet.substr(0, 2));
                    //模拟bit_error
                    modify_uint16_t_with_probability(tmpChkSum, probability_bit_error);
                    tmp_packet = uint16ToTwoByteString(tmpChkSum) + tmp_packet.substr(2);

                    //模拟drop_package
                    ssize_t res = send_package_with_probability_drop(tmp_packet, 0, addr, probability_drop_package);
                    #ifdef DEBUG
                    if (res < 0) std::cout << "【resend】message: " << (int)((uint8_t)tmp_packet[3]) << " has dropped" << std::endl;
                    else std::cout << "【resend】send message: " << (int)((uint8_t)tmp_packet[3]) << " successfully" << std::endl;
                    #endif
                    
                }
                send_start_time = clock();
            }  
        }

        //发送
        if(sent_packet <= packet_num && (send_base + window_size)%(encode_size + 1) != next_seqnum){
            std::string send_packet, send_flag, send_seq, fragment;
            uint16_t sendChkSum;
            
            //数据分块
            size_t lower,upper;
            lower = sent_packet * max_fragment_size;
            upper = ((sent_packet + 1) * max_fragment_size < buf.size()) ? (sent_packet + 1) * max_fragment_size :buf.size();
            
            if(sent_packet < packet_num){
                send_flag = std::string(1, 0);
                fragment = buf.substr(lower, upper - lower);
            }
            else{
                //发送结束报文
                send_flag = std::string(1, 1);
                fragment = std::string();
            }
                

            send_seq = std::string(1, next_seqnum);
            send_packet = send_flag + send_seq + fragment;

            sendChkSum = calculateChecksum(send_packet);
            send_packet = uint16ToTwoByteString(sendChkSum) + send_packet;

            //更新缓冲区
            send_buffer.push(send_packet);

            //模拟bit_error
            modify_uint16_t_with_probability(sendChkSum, probability_bit_error);
            send_packet = uint16ToTwoByteString(sendChkSum) + send_packet.substr(2);

            //模拟drop_package
            ssize_t res = send_package_with_probability_drop(send_packet, 0, addr, probability_drop_package);
            #ifdef DEBUG
            if (res < 0) std::cout << "【send】message: " << (int)next_seqnum << " has dropped" << std::endl;
            else std::cout << "【send】send message: " << (int)next_seqnum << " successfully" << std::endl;
            #endif

            
            //更新参数
            if(send_base == next_seqnum) send_start_time = clock();
            next_seqnum = (next_seqnum + 1)%(encode_size + 1);
            sent_packet++;
        }

        //接收
        std::string ACK_message = std::string(4, 0);
        ssize_t res = UDPsocket::receive(ACK_message, MSG_DONTWAIT);
        
        if(res > 0){
            std::string recv_chksum, recv_flag, recv_seq;
            recv_chksum = ACK_message.substr(0, 2);
            recv_flag = ACK_message.substr(2, 1);
            recv_seq = ACK_message.substr(3, 1);

            //等待接收端旧的socket关闭
            if(begin && (uint8_t)recv_flag[0])
                continue;
            else
                begin = false;

            uint16_t recvChkSum = twoByteStringToUint16(recv_chksum);
            if (recvChkSum == calculateChecksum(recv_seq)){
                uint8_t ACK_seq = recv_seq[0];

                //更新缓冲区
                size_t pop_num;
                if(ACK_seq - send_base > -2)
                    pop_num = (ACK_seq - send_base + 1)%(encode_size + 1);
                else
                    pop_num = ACK_seq + (encode_size + 1) - send_base + 1;
                for(size_t i = 0; i < pop_num; i++) send_buffer.pop();

                //更新参数
                send_base = (send_base + pop_num)%(encode_size + 1);

                //退出
                if(sent_packet == packet_num + 1 && send_base == next_seqnum) break;
            }
        }

    }

    return buf.size();
}

//接收全部消息后，等待一段时间然后退出，确保发送方能正常退出
ssize_t GBN::gbnRecv(std::string& buf){
    size_t write_pos = 0;
    std::string send_seq = std::string(1, (expected_seqnum + encode_size)%(encode_size + 1));

    bool is_stopped = false;
    clock_t stop_start = 0;
    while (true)
    {   
        //停止倒计时
        if(is_stopped && 1.0*(clock() - stop_start)/CLOCKS_PER_SEC >= stop_delay_time) break;

        //接收
        std::string recv_packet(4 + max_fragment_size, 0);
        ssize_t recv_size = UDPsocket::receive(recv_packet, MSG_DONTWAIT);

        if(recv_size > 0 ){
            std::string recv_chksum = recv_packet.substr(0, 2);
            std::string recv_flag = recv_packet.substr(2, 1);
            std::string recv_seq = recv_packet.substr(3, 1);
            std::string recv_fragment = recv_packet.substr(4);

            uint16_t recvChkSum = twoByteStringToUint16(recv_chksum);
            if(!is_stopped && recvChkSum == calculateChecksum(recv_flag + recv_seq + recv_fragment) && (uint8_t)recv_seq[0] == expected_seqnum){
                if(!is_stopped && (uint8_t)recv_flag[0]){
                    is_stopped = true;
                    stop_start = clock();
                }

                if(!is_stopped){
                    //std::cout << recv_fragment.data() << std::endl;
                    //std::cout << recv_size << std::endl;
                    for(size_t i=0; i<recv_size - 4; i++) buf[write_pos + i] = recv_fragment[i];    
                    write_pos += recv_size - 4; 
                }
                    
                //参数更新
                send_seq = recv_seq;
                expected_seqnum = (expected_seqnum + 1)%(encode_size + 1);
            }
            
            
            //发送
            std::string send_packet;
            std::string send_flag;
            
            if(is_stopped)
                send_flag = std::string(1, 1); //发送结束报文
            else
                send_flag = std::string(1, 0);

            //模拟bit_error
            uint16_t sendChkSum = calculateChecksum(send_seq);
            modify_uint16_t_with_probability(sendChkSum, probability_bit_error);
            send_packet = uint16ToTwoByteString(sendChkSum) + send_flag + send_seq;


            //模拟drop_package
            ssize_t res = send_package_with_probability_drop(send_packet, 0, (sockaddr*)&addrrecv, probability_drop_package);
            #ifdef DEBUG
            if (res < 0) std::cout << "【send】ACK message: " << (int)((uint8_t)send_seq[0]) << " has dropped" << std::endl;
            else std::cout << "【send】send ACK message: " << (int)((uint8_t)(send_seq[0])) << " successfully" << std::endl;
            #endif  
        }   
    }

    return write_pos;
    
}

ssize_t GBN::send_package_with_probability_drop(const std::string &buf, int flags, const sockaddr *addr, double probability){
    if (static_cast<double>(rand()) / RAND_MAX < probability)
        return -1;
    return UDPsocket::send(buf, flags, addr);
}