#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <cstdint>
#include <string>

struct Image {
    int width = 0;
    int height = 0;
    int channels = 0; // 3 for RGB, 4 for RGBA
    std::vector<uint8_t> data;
    bool valid = false;
};

#endif // IMAGE_H
