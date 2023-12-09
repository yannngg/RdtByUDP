//udp客户端
#include <iostream>
#include <memory>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../include/Rdt30.h"
#include "../include/iofile.h"

using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::cerr;

static void server();

int main(){
    server();
    return 0;
}

void server(){
    struct sockaddr_in server_addr{0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(atoi("6000"));

    std::unique_ptr<Rdt30> sev_socket(new Rdt30());
    if (sev_socket->failed()) {
        cerr << "create socket failed" << endl;
        exit(1);
    }

    std::unique_ptr<FileWriter> fileW(new FileWriter("file.txt"));

    if (sev_socket->bindaddr((sockaddr*)&server_addr) == -1){
        cerr << "bind addr failed!" << endl;
        exit(1);
    }

    string recv_message(UDPsocket::BUFSIZE, 0);
    ssize_t recv_len = 0;

    cout <<"server is listening" << endl;

    while(true){
        if ((recv_len = sev_socket->rdt30Receive(recv_message, 0)) != -1){
            fileW->writeNextBlock(recv_message, recv_len);
        }
    }
    
}
