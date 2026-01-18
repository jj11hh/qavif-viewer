#include "jpegavifconverter.h"
#include "jpegheaderreader.h"
#include "avif/avif.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <memory>

extern "C" {
#include "turbojpeg.h"
}

// Helper endian functions
static uint16_t to_be16(uint16_t x) {
    return ((x & 0xFF) << 8) | ((x >> 8) & 0xFF);
}

static void rearrage(uint8_t *src_buffer, uint8_t *dst_buffer, size_t src_size, size_t dst_size, int height){
    if (src_size == dst_size){
        if (src_buffer != dst_buffer)
            memmove(dst_buffer, src_buffer, src_size * (size_t)height);
    }

    if (src_size > dst_size){
        for (int i = 0; i < height; i ++){
            memmove(dst_buffer + (dst_size * (size_t)i), src_buffer + (src_size * (size_t)i), dst_size);
        }
    }
    else {
        for (int i = height - 1; i >= 0; i --) {
            memmove(dst_buffer + (dst_size * (size_t)i), src_buffer + (src_size * (size_t)i), src_size);
        }
    }
}

static inline avifPixelFormat format_j2a(int subsample){
    switch (subsample){
    case TJSAMP_420:
        return AVIF_PIXEL_FORMAT_YUV420;
    case TJSAMP_422:
        return AVIF_PIXEL_FORMAT_YUV422;
    case TJSAMP_444:
        return AVIF_PIXEL_FORMAT_YUV444;
    default:
        return AVIF_PIXEL_FORMAT_NONE;
    }
}

static inline int format_a2j(avifPixelFormat format){
    switch (format){
    case AVIF_PIXEL_FORMAT_YUV420:
        return TJSAMP_420;
    case AVIF_PIXEL_FORMAT_YUV422:
        return TJSAMP_422;
    case AVIF_PIXEL_FORMAT_YUV444:
        return TJSAMP_444;
    default:
        return -1;
    }
}

JpegAvifConverter::JpegAvifConverter(const ConvertSettings &convSettings): settings(convSettings) {}

bool JpegAvifConverter::ConvertJpegToAvif(const std::string &jpegpath, const std::string &avifpath) const{
    tjhandle handle = nullptr;
    int width, height, subsample, colorspace;
    int ret = 0;
    int depth = 8;
    avifPixelFormat yuv_format = AVIF_PIXEL_FORMAT_YUV420;

    std::ifstream jpegFile(jpegpath, std::ios::binary | std::ios::ate);
    if (!jpegFile.is_open()) return false;
    
    size_t jpegSize = jpegFile.tellg();
    jpegFile.seekg(0, std::ios::beg);
    std::vector<uint8_t> jpegBytes(jpegSize);
    if (!jpegFile.read((char*)jpegBytes.data(), jpegSize)) return false;
    jpegFile.close();

    // Scan headers
    JpegHeaderReader jpegReader(jpegBytes.data(), jpegBytes.size());
    std::vector<uint8_t> icc;
    std::vector<uint8_t> exif;

    handle = tjInitDecompress();
    if (!handle) return false;

    ret = tjDecompressHeader3(handle, jpegBytes.data(), jpegBytes.size(), &width, &height, &subsample, &colorspace);
    if (ret < 0){
        tjDestroy(handle);
        return false;
    }

    // Dump ICC and EXIF
    while (!jpegReader.atEnd()){
        uint16_t m = jpegReader.current();
        if (m == JpegHeaderReader::M_APP1){ // EXIF in APP1 Segment
            auto chunk = jpegReader.read();
            exif.insert(exif.end(), chunk.begin(), chunk.end());
        }
        else if (m == JpegHeaderReader::M_APP2){ // ICC Profile
            auto chunk = jpegReader.read();
            icc.insert(icc.end(), chunk.begin(), chunk.end());
        }
        else {
            jpegReader.skip();
        }
    }

    yuv_format = format_j2a(subsample);
    if (yuv_format == AVIF_PIXEL_FORMAT_NONE){
        tjDestroy(handle);
        return false;
    }

    auto avifImage = avifImageCreate(width, height, depth, yuv_format);

    if (!icc.empty()){
        avifImageSetProfileICC(avifImage, icc.data(), icc.size());
    }

    if (!exif.empty() && settings.isSaveAvifExif){
        avifImageSetMetadataExif(avifImage, exif.data(), exif.size());
    }

    avifImage->colorPrimaries = (avifColorPrimaries)1;
    avifImage->transferCharacteristics = (avifTransferCharacteristics)13;
    avifImage->matrixCoefficients = (avifMatrixCoefficients)5;

    unsigned long y_size = tjPlaneSizeYUV(0, width, 0, height, subsample);
    unsigned long u_size = tjPlaneSizeYUV(1, width, 0, height, subsample);
    unsigned long v_size = tjPlaneSizeYUV(2, width, 0, height, subsample);

    int y_w = tjPlaneWidth(0, width, subsample);
    // int u_w = tjPlaneWidth(1, width, subsample);
    // int v_w = tjPlaneWidth(2, width, subsample);

    int channelSize = 1; // 8 bit
    int fullRowBytes = channelSize * (int)avifImage->width;
    avifPixelFormatInfo info;
    avifGetPixelFormatInfo(avifImage->yuvFormat, &info);
    int shiftedW = ((int)avifImage->width + info.chromaShiftX) >> info.chromaShiftX;
    int uvRowBytes = channelSize * shiftedW;

    uint8_t * plane_y = (uint8_t *) avifAlloc(y_size);
    uint8_t * plane_u = (uint8_t *) avifAlloc(u_size);
    uint8_t * plane_v = (uint8_t *) avifAlloc(v_size);

    avifImage->yuvRowBytes[AVIF_CHAN_Y] = (uint32_t)fullRowBytes;
    avifImage->yuvPlanes[AVIF_CHAN_Y] = plane_y;
    avifImage->yuvRowBytes[AVIF_CHAN_U] = (uint32_t)uvRowBytes;
    avifImage->yuvPlanes[AVIF_CHAN_U] = plane_u;
    avifImage->yuvRowBytes[AVIF_CHAN_V] = (uint32_t)uvRowBytes;
    avifImage->yuvPlanes[AVIF_CHAN_V] = plane_v;
    avifImage->imageOwnsYUVPlanes = AVIF_TRUE;

    // Use pointers to planes
    unsigned char* planes[3] = { plane_y, plane_u, plane_v };
    
    tjDecompressToYUVPlanes(
                handle,
                jpegBytes.data(),
                jpegBytes.size(),
                planes,
                width,
                nullptr,
                height,
                0
    );

    rearrage(plane_y, plane_y, (size_t)y_w, (size_t)fullRowBytes, height);

    tjDestroy(handle);

    auto encoder = avifEncoderCreate();
    if (!encoder) {
        avifImageDestroy(avifImage);
        return false;
    }
    // encoder->maxThreads = ...; // Use default
    encoder->minQuantizer = settings.minQuantizer;
    encoder->maxQuantizer = settings.maxQuantizer;
    encoder->codecChoice = AVIF_CODEC_CHOICE_AUTO;
    encoder->speed = settings.encodeSpeed;

    avifRWData raw = AVIF_DATA_EMPTY;
    auto encodeResult = avifEncoderWrite(encoder, avifImage, &raw);
    
    bool success = false;
    if (encodeResult == AVIF_RESULT_OK){
        std::ofstream outFile(avifpath, std::ios::binary);
        if (outFile.is_open()) {
            outFile.write((const char*)raw.data, raw.size);
            success = true;
        }
    }

    avifImageDestroy(avifImage);
    avifRWDataFree(&raw);
    avifEncoderDestroy(encoder);

    return success;
}

bool JpegAvifConverter::ConvertAvifToJpeg(const std::string &avifpath, const std::string &jpegpath) const{
    std::ifstream avifFile(avifpath, std::ios::binary | std::ios::ate);
    if (!avifFile.is_open()) return false;
    size_t size = avifFile.tellg();
    avifFile.seekg(0, std::ios::beg);
    std::vector<uint8_t> avifBytes(size);
    if (!avifFile.read((char*)avifBytes.data(), size)) return false;

    avifROData raw;
    raw.data = avifBytes.data();
    raw.size = avifBytes.size();

    avifImage *image = avifImageCreateEmpty();
    avifDecoder *decoder = avifDecoderCreate();
    
    if (avifDecoderReadMemory(decoder, image, raw.data, raw.size) != AVIF_RESULT_OK) {
        avifDecoderDestroy(decoder);
        avifImageDestroy(image);
        return false;
    }

    bool ret = true;
    int subsample = format_a2j(image->yuvFormat);

    if (subsample != -1) {
        tjhandle handle = tjInitCompress();
        if (handle) {
            unsigned char *jpegBuf = nullptr;
            unsigned long jpegSize = 0;
            
            // Similar logic for handling strides as in existing code
             int channelSize = 1;
             int fullRowBytes = channelSize * (int)image->width;
             // avifPixelFormatInfo info;
             // avifGetPixelFormatInfo(image->yuvFormat, &info);
             
             size_t y_size = tjPlaneSizeYUV(0, image->width, 0, image->height, subsample);
             int y_w = tjPlaneWidth(0, image->width, subsample);
             
             uint8_t * y_buffer = nullptr;
             const uint8_t * yuvPlanes[3];
             
             if (y_w != fullRowBytes) {
                 y_buffer = new uint8_t[y_size];
                 rearrage(image->yuvPlanes[0], y_buffer, (size_t)fullRowBytes, (size_t)y_w, image->height);
                 yuvPlanes[0] = y_buffer;
             } else {
                 yuvPlanes[0] = image->yuvPlanes[0];
             }
             yuvPlanes[1] = image->yuvPlanes[1];
             yuvPlanes[2] = image->yuvPlanes[2];
             
             if (tjCompressFromYUVPlanes(
                 handle,
                 yuvPlanes,
                 image->width,
                 nullptr,
                 image->height,
                 subsample,
                 &jpegBuf,
                 &jpegSize,
                 settings.jpegQuality,
                 0
             ) == 0) {
                 
                 std::ofstream jpegFile(jpegpath, std::ios::binary);
                 if (jpegFile.is_open()) {
                      if (settings.isSaveJpegExif) {
                          // Write SOI
                          uint16_t soi = to_be16(JpegHeaderReader::M_SOI);
                          jpegFile.write((char*)&soi, 2);
                          
                          // Write EXIF
                          if (image->exif.size >= 2) {
                              uint16_t m = to_be16(JpegHeaderReader::M_APP1);
                              jpegFile.write((char*)&m, 2);
                              jpegFile.write((char*)image->exif.data, image->exif.size);
                          }
                          
                          // Write ICC
                          if (image->icc.size >= 2) {
                              uint16_t m = to_be16(JpegHeaderReader::M_APP2);
                              jpegFile.write((char*)&m, 2);
                              jpegFile.write((char*)image->icc.data, image->icc.size);
                          }
                          
                          // Write rest of JPEG
                          // Skip SOI in buffer
                          size_t i = 2;
                          // Helper to get word
                          auto get_word = [&](size_t pos) -> uint16_t {
                              return (uint16_t)((jpegBuf[pos] << 8) | jpegBuf[pos+1]);
                          };
                          
                          while (i < jpegSize) {
                              uint16_t m = get_word(i);
                              if (m == JpegHeaderReader::M_SOS) {
                                  // Write everything from here
                                  jpegFile.write((char*)(jpegBuf + i), jpegSize - i);
                                  break;
                              }
                              
                              if ((m & 0xFFE0) == 0xFFE0) {
                                  // Skip app segments in original if we are writing our own? 
                                  // But turbojpeg doesn't preserve EXIF/ICC usually, 
                                  // so the jpegBuf shouldn't have them?
                                  // Turbojpeg generated buffer is clean image data.
                                  // Wait, turbojpeg compress generates a VALID JPEG with headers (JFIF).
                                  // It usually inserts APP0 (JFIF).
                                  // We might be duplicating or messing up if we just blindly insert APP1/APP2 after SOI.
                                  
                                  // The original code assumes we insert our APP1/APP2 *after* SOI and *before* other segments 
                                  // but it filtered out existing APP segments?
                                  // "if ( (m & 0xFFE0) == 0xFFE0 ) ... i += len + 2" -> Skips APP segments from buffer.
                                  
                                  size_t len = get_word(i+2);
                                  i += len + 2;
                              } else {
                                  size_t len = get_word(i+2);
                                  jpegFile.write((char*)(jpegBuf + i), len + 2);
                                  i += len + 2;
                              }
                          }
                      } else {
                          jpegFile.write((char*)jpegBuf, jpegSize);
                      }
                 } else {
                     ret = false;
                 }
                 tjFree(jpegBuf);
             } else {
                 ret = false;
             }
             
             if (y_buffer) delete[] y_buffer;
             tjDestroy(handle);

        }
    } else {
        ret = false;
    }

    avifDecoderDestroy(decoder);
    avifImageDestroy(image);
    return ret;
}

bool JpegAvifConverter::ImageToAvif(const Image &image, const std::string &path) const {
    if (!image.valid) return false;

    avifImage *dimage = avifImageCreate(image.width, image.height, 8, AVIF_PIXEL_FORMAT_YUV420);

    avifRGBImage rgb;
    avifRGBImageSetDefaults(&rgb, dimage);
    rgb.format = AVIF_RGB_FORMAT_RGBA;
    rgb.depth = 8;
    rgb.pixels = const_cast<uint8_t*>(image.data.data());
    rgb.rowBytes = image.width * 4;

    dimage->colorPrimaries = AVIF_COLOR_PRIMARIES_IEC61966_2_4;
    dimage->transferCharacteristics = AVIF_TRANSFER_CHARACTERISTICS_SRGB;

    avifImageRGBToYUV(dimage, &rgb);

    avifRWData output = AVIF_DATA_EMPTY;
    avifEncoder *encoder = avifEncoderCreate();
    // Threads? 
    encoder->minQuantizer = settings.minQuantizer;
    encoder->maxQuantizer = settings.maxQuantizer;
    encoder->maxQuantizerAlpha = settings.maxQuantizer;
    encoder->minQuantizerAlpha = settings.minQuantizer;
    encoder->speed = settings.encodeSpeed;

    bool ret = false;
    if (avifEncoderWrite(encoder, dimage, &output) == AVIF_RESULT_OK) {
        std::ofstream file(path, std::ios::binary);
        if (file.is_open()) {
            file.write((char*)output.data, output.size);
            ret = true;
        }
    }

    avifImageDestroy(dimage);
    avifRWDataFree(&output);
    avifEncoderDestroy(encoder);
    
    return ret;
}