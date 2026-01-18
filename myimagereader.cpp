#include "myimagereader.h"
#include "avif/avif.h"
#include <cstdio>
#include <vector>
#include <fstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

extern "C" {
#include "turbojpeg.h"
}

MyImageReader::MyImageReader(const std::string &fileName) : path(fileName)
{
    // Simple extension check
    size_t dot = fileName.find_last_of(".");
    if (dot != std::string::npos) {
        std::string ext = fileName.substr(dot + 1);
        // to lower
        for (auto &c : ext) c = tolower(c);
        
        if (ext == "avif") format = "avif";
        else if (ext == "jpg" || ext == "jpeg") format = "jpeg";
        else format = "other";
    } else {
        format = "other";
    }
}

MyImageReader::~MyImageReader(){
}

Image MyImageReader::read(){
    Image img;
    img.valid = false;

    // Read file content
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return img;

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read((char*)buffer.data(), size)) return img;

    if (format == "avif") {
        avifROData rawdata;
        rawdata.data = buffer.data();
        rawdata.size = buffer.size();

        avifImage *aimage = avifImageCreateEmpty();
        avifDecoder *decoder = avifDecoderCreate();
        avifResult result = avifDecoderSetIOMemory(decoder, rawdata.data, rawdata.size);
        
        if (result == AVIF_RESULT_OK) {
            result = avifDecoderRead(decoder, aimage);
            if (result == AVIF_RESULT_OK) {
                img.width = aimage->width;
                img.height = aimage->height;
                img.channels = 4; // Force RGBA
                
                avifRGBImage rgb;
                avifRGBImageSetDefaults(&rgb, aimage);
                rgb.format = AVIF_RGB_FORMAT_RGBA;
                rgb.depth = 8;
                
                // Allocate destination buffer
                img.data.resize(rgb.width * rgb.height * 4);
                rgb.pixels = img.data.data();
                rgb.rowBytes = rgb.width * 4;
                
                if (avifImageYUVToRGB(aimage, &rgb) == AVIF_RESULT_OK) {
                    img.valid = true;
                }
            }
        }
        avifImageDestroy(aimage);
        avifDecoderDestroy(decoder);
    } 
    else if (format == "jpeg") {
        tjhandle handle = tjInitDecompress();
        if (handle) {
            int width, height, subsamp, colorspace;
            if (tjDecompressHeader3(handle, buffer.data(), buffer.size(), &width, &height, &subsamp, &colorspace) == 0) {
                img.width = width;
                img.height = height;
                img.channels = 4; // Force RGBA
                img.data.resize(width * height * 4);
                
                if (tjDecompress2(handle, buffer.data(), buffer.size(), img.data.data(), width, 0, height, TJPF_RGBA, 0) == 0) {
                    img.valid = true;
                }
            }
            tjDestroy(handle);
        }
    } 
    
    // Fallback to stb_image if not valid yet (or if format was other)
    if (!img.valid) {
        int w, h, c;
        // Force 4 channels (RGBA)
        unsigned char *data = stbi_load_from_memory(buffer.data(), (int)buffer.size(), &w, &h, &c, 4);
        if (data) {
            img.width = w;
            img.height = h;
            img.channels = 4;
            img.data.assign(data, data + (w * h * 4));
            img.valid = true;
            stbi_image_free(data);
        }
    }

    return img;
}