//udp客户端
#include <iostream>
#include <fstream>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../include/rdt22.h"
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
    std::unique_ptr<Rdt22> socket(new Rdt22());
    if (socket->failed()) {
        cerr << "create socket failed" << endl;
        exit(1);
    }

    std::unique_ptr<FileReader> fileR(new FileReader("/mnt/d/my_vscode/code_repository/projects/RdtByUDP/send.txt"));

    string message;

    while (!fileR->isEnd()){
        message = fileR->readNextBlock(64);
        socket->rdt22Send(message, 0, (struct sockaddr *)&server_addr);
    }
}