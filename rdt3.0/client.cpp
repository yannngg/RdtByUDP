//udp客户端
#include <iostream>
#include <fstream>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../include/rdt30.h"
#include "../include/iofile.h"

using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::cerr;

void client();

int main(){
    srand(time(0));
    client();
    return 0;
}

void client(){
    struct sockaddr_in server_addr{0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(atoi("6000"));

    //分配网络套接字资源
    std::unique_ptr<Rdt30> socket(new Rdt30());
    if (socket->failed()) {
        cerr << "create socket failed" << endl;
        exit(1);
    }

    //设置超时
    timeval timeout;
    timeout.tv_sec = socket->timer; // 超时时间为5秒
    timeout.tv_usec = 0;

    if (!socket->init(SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)))
        std::cout << "set timer successfully" << std::endl;
    else std::cout << "set timer failed" << std::endl;

    std::unique_ptr<FileReader> fileR(new FileReader("/mnt/d/my_vscode/code_repository/projects/RdtByUDP/send.txt"));
    string message;

    while (!fileR->isEnd()){
        message = fileR->readNextBlock(64);
        socket->rdt30Send(message, 0, (struct sockaddr *)&server_addr);
    }
}