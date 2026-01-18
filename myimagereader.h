#ifndef MYIMAGEREADER_H
#define MYIMAGEREADER_H

#include "Image.h"
#include <string>

class MyImageReader
{
public:
    MyImageReader(const std::string &fileName);
    ~MyImageReader();
    Image read();

private:
    std::string path;
    std::string format;
};

#endif // MYIMAGEREADER_H