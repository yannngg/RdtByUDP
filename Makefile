# 编译器
CC = g++

# 编译选项
CFLAGS = -Iinclude


all: client server

client: src/others/client.cpp include/UDPsocket.cpp include/iofile.cpp
	$(CC) $(CFLAGS) -o output/client src/others/client.cpp include/UDPsocket.cpp include/iofile.cpp

server: src/others/server.cpp include/UDPsocket.cpp include/iofile.cpp
	$(CC) $(CFLAGS) -o output/server src/others/server.cpp include/UDPsocket.cpp include/iofile.cpp

test: src/others/test.cpp include/UDPsocket.cpp include/UDPsocket.cpp include/iofile.cpp
	$(CC) $(CFLAGS) -o output/test src/others/test.cpp  include/UDPsocket.cpp include/iofile.cpp

run_client: output/client
	@./output/client

run_server: output/server
	@./output/server

run_test: output/test
	@./output/test

