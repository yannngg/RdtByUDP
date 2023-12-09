//gbn服务端
#include <iostream>
#include <memory>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../include/gbn.h"
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

static void server(){
    struct sockaddr_in server_addr{0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(atoi("6000"));

    std::unique_ptr<GBN> sev_socket(new GBN());
    if (sev_socket->failed()) {
        cerr << "create socket failed" << endl;
        exit(1);
    }

    std::unique_ptr<FileWriter> fileW(new FileWriter("file.txt"));

    if (sev_socket->bindaddr((sockaddr*)&server_addr) == -1){
        cerr << "bind addr failed!" << endl;
        exit(1);
    }

    string recv_message(BUFSIZ, 0);
    ssize_t recv_len = 0;

    cout <<"server is listening" << endl;

    while(true){
        recv_len = sev_socket->gbnRecv(recv_message);

        if(recv_len == 2 && !recv_message[0] && !recv_message[1]) break;
        fileW->flush();
        fileW->writeNextBlock(recv_message, recv_len);
    }
}
