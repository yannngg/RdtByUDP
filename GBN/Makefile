# 指定编译器和编译选项
CC = g++
CFLAGS = -Wall -std=c++11

# 指定源文件和目标文件
SRCS = ../src/iofile.cpp ../src/UDPsocket.cpp ../src/gbn.cpp
SRC_CLIENT = client.cpp
SRC_SERVER = server.cpp

OBJS = $(SRCS:.cpp=.o)
OBJS_CLIENT = $(SRCS:.cpp=.o) $(SRC_CLIENT:.cpp=.o)
OBJS_SERVER = $(SRCS:.cpp=.o) $(SRC_SERVER:.cpp=.o)

OUT_PUT_DIR = output

# 指定头文件路径
INCLUDES = -Iinclude

all: client server clean

# 链接可执行文件
client: $(OBJS_CLIENT)
	$(CC) $(CFLAGS) $(OBJS_CLIENT) -o $(OUT_PUT_DIR)/$@

server: $(OBJS_SERVER)
	$(CC) $(CFLAGS) $(OBJS_SERVER) -o $(OUT_PUT_DIR)/$@

run_client:
	./$(OUT_PUT_DIR)/client

run_server:
	./$(OUT_PUT_DIR)/server

# 清理目标文件和可执行文件
clean:
	@rm -f $(OBJS) $(OBJS_CLIENT) $(OBJS_SERVER)
