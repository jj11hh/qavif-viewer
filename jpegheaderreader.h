#ifndef JPEGHEADERREADER_H
#define JPEGHEADERREADER_H

#include <QIODevice>

class JpegHeaderReader
{
public:
    JpegHeaderReader(QIODevice *);
    quint16 current();
    quint16 size();
    QByteArray read();
    int read(QByteArray &);
    void skip();
    bool valid();
    bool atEnd();

    static const quint16 M_SOF0  = 0xffc0;
    static const quint16 M_SOF1  = 0xffc1;
    static const quint16 M_SOF2  = 0xffc2;
    static const quint16 M_SOF3  = 0xffc3;

    static const quint16 M_SOF5  = 0xffc5;
    static const quint16 M_SOF6  = 0xffc6;
    static const quint16 M_SOF7  = 0xffc7;

    static const quint16 M_JPG   = 0xffc8;
    static const quint16 M_SOF9  = 0xffc9;
    static const quint16 M_SOF10 = 0xffca;
    static const quint16 M_SOF11 = 0xffcb;

    static const quint16 M_SOF13 = 0xffcd;
    static const quint16 M_SOF14 = 0xffce;
    static const quint16 M_SOF15 = 0xffcf;

    static const quint16 M_DHT   = 0xffc4;

    static const quint16 M_DAC   = 0xffcc;

    static const quint16 M_RST0  = 0xffd0;
    static const quint16 M_RST1  = 0xffd1;
    static const quint16 M_RST2  = 0xffd2;
    static const quint16 M_RST3  = 0xffd3;
    static const quint16 M_RST4  = 0xffd4;
    static const quint16 M_RST5  = 0xffd5;
    static const quint16 M_RST6  = 0xffd6;
    static const quint16 M_RST7  = 0xffd7;

    static const quint16 M_SOI   = 0xffd8;
    static const quint16 M_EOI   = 0xffd9;
    static const quint16 M_SOS   = 0xffda;
    static const quint16 M_DQT   = 0xffdb;
    static const quint16 M_DNL   = 0xffdc;
    static const quint16 M_DRI   = 0xffdd;
    static const quint16 M_DHP   = 0xffde;
    static const quint16 M_EXP   = 0xffdf;

    static const quint16 M_APP0  = 0xffe0;
    static const quint16 M_APP1  = 0xffe1;
    static const quint16 M_APP2  = 0xffe2;
    static const quint16 M_APP3  = 0xffe3;
    static const quint16 M_APP4  = 0xffe4;
    static const quint16 M_APP5  = 0xffe5;
    static const quint16 M_APP6  = 0xffe6;
    static const quint16 M_APP7  = 0xffe7;
    static const quint16 M_APP8  = 0xffe8;
    static const quint16 M_APP9  = 0xffe9;
    static const quint16 M_APP10 = 0xffea;
    static const quint16 M_APP11 = 0xffeb;
    static const quint16 M_APP12 = 0xffec;
    static const quint16 M_APP13 = 0xffed;
    static const quint16 M_APP14 = 0xffee;
    static const quint16 M_APP15 = 0xffef;

    static const quint16 M_JPG0  = 0xfff0;
    static const quint16 M_JPG13 = 0xfffd;
    static const quint16 M_COM   = 0xfffe;

    static const quint16 M_TEM   = 0xff01;

private:
    quint16 currentMark = 0;
    quint16 currentSize = 0;
    QIODevice *io = nullptr;
    void readMark();
};

#endif // JPEGHEADERREADER_H
