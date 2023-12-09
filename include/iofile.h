#include <iostream>
#include <fstream>
#include <string>

class FileReader {
    std::ifstream file;
public:
    FileReader(const std::string& filename);
    ~FileReader();
    bool isEnd() const { return file.eof();}
    std::string readNextBlock(size_t blockSize);
};

class FileWriter{
    std::ofstream file;
public:
    FileWriter(const std::string& filename);
    ~FileWriter();
    void flush(){file.flush();}
    void writeNextBlock(const std::string& block, unsigned int len);
};