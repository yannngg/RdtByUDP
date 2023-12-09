#include <iostream>
#include <fstream>
#include <string>
#include "../include/iofile.h"

FileReader::FileReader(const std::string& filename) : file(filename, std::ios::binary| std::ios::in) {
    if (!file.is_open()) {
        std::cerr << "无法打开文件： " << filename << std::endl;
        exit(1);
    }
}

FileReader::~FileReader() {
    if (file.is_open()) {
        file.close();
    }
}

std::string FileReader::readNextBlock(size_t blockSize) {  
    if (!file.is_open()) {
        return "";
    }

    char* buffer = new char[blockSize];
    file.read(buffer, blockSize);
    size_t bytesRead = file.gcount();

    std::string result(buffer, bytesRead);
    delete[] buffer;

    return result;
}

FileWriter::FileWriter(const std::string &filename):file(filename, std::ios::binary) {
    if (!file.is_open()) {
        std::cerr << "无法打开文件： " << filename << std::endl;
        exit(1);
    }
}

FileWriter::~FileWriter(){
    if (file.is_open()){
        file.close();
    }
}

void FileWriter::writeNextBlock(const std::string &block, unsigned int len){
    if (!file.is_open()) return;
    file.flush();
    file.write(block.data(), len);
    //刷新缓冲区，避免写入过快
    file.flush();
}
