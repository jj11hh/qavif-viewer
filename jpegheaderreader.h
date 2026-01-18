#ifndef JPEGHEADERREADER_H
#define JPEGHEADERREADER_H

#include <vector>
#include <cstdint>
#include <cstddef>

class JpegHeaderReader
{
public:
    // Work with memory buffer
    JpegHeaderReader(const uint8_t* data, size_t size);
    
    uint16_t current();
    uint16_t size();
    std::vector<uint8_t> read();
    void skip();
    bool valid();
    bool atEnd();

    static const uint16_t M_SOF0  = 0xffc0;
    static const uint16_t M_SOF1  = 0xffc1;
    static const uint16_t M_SOF2  = 0xffc2;
    static const uint16_t M_SOF3  = 0xffc3;

    static const uint16_t M_SOF5  = 0xffc5;
    static const uint16_t M_SOF6  = 0xffc6;
    static const uint16_t M_SOF7  = 0xffc7;

    static const uint16_t M_JPG   = 0xffc8;
    static const uint16_t M_SOF9  = 0xffc9;
    static const uint16_t M_SOF10 = 0xffca;
    static const uint16_t M_SOF11 = 0xffcb;

    static const uint16_t M_SOF13 = 0xffcd;
    static const uint16_t M_SOF14 = 0xffce;
    static const uint16_t M_SOF15 = 0xffcf;

    static const uint16_t M_DHT   = 0xffc4;

    static const uint16_t M_DAC   = 0xffcc;

    static const uint16_t M_RST0  = 0xffd0;
    static const uint16_t M_RST1  = 0xffd1;
    static const uint16_t M_RST2  = 0xffd2;
    static const uint16_t M_RST3  = 0xffd3;
    static const uint16_t M_RST4  = 0xffd4;
    static const uint16_t M_RST5  = 0xffd5;
    static const uint16_t M_RST6  = 0xffd6;
    static const uint16_t M_RST7  = 0xffd7;

    static const uint16_t M_SOI   = 0xffd8;
    static const uint16_t M_EOI   = 0xffd9;
    static const uint16_t M_SOS   = 0xffda;
    static const uint16_t M_DQT   = 0xffdb;
    static const uint16_t M_DNL   = 0xffdc;
    static const uint16_t M_DRI   = 0xffdd;
    static const uint16_t M_DHP   = 0xffde;
    static const uint16_t M_EXP   = 0xffdf;

    static const uint16_t M_APP0  = 0xffe0;
    static const uint16_t M_APP1  = 0xffe1;
    static const uint16_t M_APP2  = 0xffe2;
    static const uint16_t M_APP3  = 0xffe3;
    static const uint16_t M_APP4  = 0xffe4;
    static const uint16_t M_APP5  = 0xffe5;
    static const uint16_t M_APP6  = 0xffe6;
    static const uint16_t M_APP7  = 0xffe7;
    static const uint16_t M_APP8  = 0xffe8;
    static const uint16_t M_APP9  = 0xffe9;
    static const uint16_t M_APP10 = 0xffea;
    static const uint16_t M_APP11 = 0xffeb;
    static const uint16_t M_APP12 = 0xffec;
    static const uint16_t M_APP13 = 0xffed;
    static const uint16_t M_APP14 = 0xffee;
    static const uint16_t M_APP15 = 0xffef;

    static const uint16_t M_JPG0  = 0xfff0;
    static const uint16_t M_JPG13 = 0xfffd;
    static const uint16_t M_COM   = 0xfffe;

    static const uint16_t M_TEM   = 0xff01;

private:
    uint16_t currentMark = 0;
    uint16_t currentSize = 0;
    const uint8_t* m_data = nullptr;
    size_t m_size = 0;
    size_t m_pos = 0;

    void readMark();
};

#endif // JPEGHEADERREADER_H