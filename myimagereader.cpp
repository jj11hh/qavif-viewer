#include "myimagereader.h"
#include "avif.h"
#include <QFile>

MyImageReader::MyImageReader(const QString &fileName)
{
    reader = nullptr;
    if (fileName.endsWith(".avif")){
        format = "avif";
        path = fileName;
    }
    else {
        reader = new QImageReader(fileName);
        format = reader->format();
    }
}

MyImageReader::~MyImageReader(){
    delete reader;
}

QImage MyImageReader::read(){
    if (reader){
        return reader->read();
    }

    QFile file(path);

    if (! file.open(QIODevice::ReadOnly)){
        return QImage();
    }

    auto datas = file.readAll();
    avifROData rawdata;
    rawdata.data = reinterpret_cast<uint8_t*>(datas.data());
    rawdata.size = static_cast<std::make_unsigned<int>::type>(datas.length());

    QImage image;
    avifImage *aimage = avifImageCreateEmpty();
    avifDecoder *decoder = avifDecoderCreate();
    avifResult decodeResult = avifDecoderRead(decoder, aimage, &rawdata);
    int width;
    int height;

    if (decodeResult == AVIF_RESULT_OK){
        width = (int) aimage->width;
        height = (int) aimage->height;
        avifRGBImage rgb;
        avifRGBImageSetDefaults(&rgb, aimage);
        rgb.format = AVIF_RGB_FORMAT_RGB;
        rgb.depth = 8;

        avifRGBImageAllocatePixels(&rgb);
        avifImageYUVToRGB(aimage, &rgb);

        image = QImage(rgb.pixels, width, height,
                     (int)rgb.rowBytes,
                     QImage::Format_RGB888, avifFree, rgb.pixels);
    }

    avifImageDestroy(aimage);
    avifDecoderDestroy(decoder);

    return image;
}
