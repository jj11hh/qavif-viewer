#include "jpegavifconverter.h"
#include "jpegheaderreader.h"
#include "avif/avif.h"
#include <cstdio>
#include <cstring>
#include <QFile>
#include <QDebug>
#include <QBuffer>
#include <QThread>
#include <QApplication>
#include <QtEndian>

extern "C" {
#include "turbojpeg.h"
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

bool JpegAvifConverter::ConvertJpegToAvif(const QString &jpegpath, const QString &avifpath) const{
    tjhandle handle = nullptr;
    int width, height, subsample, colorspace;
    int ret = 0;
    int depth = 8;
    avifPixelFormat yuv_format = AVIF_PIXEL_FORMAT_YUV420;

    QFile jpegFile(jpegpath);
    QFile avifFile(avifpath);

    if (!jpegFile.open(QIODevice::ReadOnly)){
        qCritical("Can't open file: %s", jpegpath.toUtf8().constData());
        return false;
    }
    if (!avifFile.open(QIODevice::WriteOnly)){
        qCritical("Can't open file: %s", avifpath.toUtf8().constData());
        return false;
    }

    auto jpegBytes = jpegFile.readAll();

    QByteArray icc, exif;
    QBuffer jpeg_io(&jpegBytes);
    jpeg_io.open(QBuffer::ReadOnly);
    JpegHeaderReader jpegReader(&jpeg_io);
    auto * jpeg_buf = reinterpret_cast<const uint8_t *>(jpegBytes.constData());
    unsigned long jpeg_size = static_cast<std::make_unsigned<int>::type>(jpegBytes.length());

    jpegFile.close();

    handle = tjInitDecompress();
    Q_CHECK_PTR(handle);

    ret = tjDecompressHeader3(handle, jpeg_buf, jpeg_size, &width, &height, &subsample, &colorspace);
    if (ret < 0){
        tjDestroy(handle);
        qCritical("Can't read a valid jpeg head: %s", jpegpath.toUtf8().constData());
        return false;
    }

    qDebug() << "Loaded JPEG: w=" << width << ", h=" << height;

    // Dump ICC and EXIF here
    while (! jpegReader.atEnd()){
        qDebug("Marker 0x%4x got", jpegReader.current());
        if (jpegReader.current() == JpegHeaderReader::M_APP1){ // EXIF in APP1 Segment
            exif += jpegReader.read();
            qDebug() << "EXIF read, " << exif.length() << "bytes";
        }
        else if (jpegReader.current() == JpegHeaderReader::M_APP2){ // ICC Profile was stored in APP2 Segment
            icc += jpegReader.read();
            qDebug() << "ICC read, " << icc.length() << "bytes";
        }
        else {
            jpegReader.skip();
        }
    }

    yuv_format = format_j2a(subsample);

    if (yuv_format == AVIF_PIXEL_FORMAT_NONE){
        tjDestroy(handle);
        qCritical("Unsupported format");
        return false;
    }

    auto avifImage = avifImageCreate(width, height, depth, yuv_format);

    // Copy ICC and EXIF
    if (icc.length()){
        avifImageSetProfileICC(avifImage,
            reinterpret_cast<uint8_t *>(icc.data()),
            static_cast<std::make_unsigned<int>::type>(icc.length()));
    }

    if (exif.length() && settings.isSaveAvifExif){
        avifImageSetMetadataExif(avifImage,
            reinterpret_cast<uint8_t*>(exif.data()),
            static_cast<std::make_unsigned<int>::type>(exif.length()));
    }

    // SRGB color space
    avifImage->colorPrimaries = (avifColorPrimaries)1;
    avifImage->transferCharacteristics = (avifTransferCharacteristics)13;
    avifImage->matrixCoefficients = (avifMatrixCoefficients)5;
    // Let's try to decode/encode it in YUV PLANES WAY

    unsigned long y_size, u_size, v_size;
    y_size = tjPlaneSizeYUV(0, width, 0, height, subsample);
    u_size = tjPlaneSizeYUV(1, width, 0, height, subsample);
    v_size = tjPlaneSizeYUV(2, width, 0, height, subsample);

    int y_w, u_w, v_w;
    y_w = tjPlaneWidth(0, width, subsample);
    u_w = tjPlaneWidth(1, width, subsample);
    v_w = tjPlaneWidth(2, width, subsample);

    qDebug("as TurboJpeg needed, size of [y, u, v] == [%lu, %lu, %lu]", y_size, u_size, v_size);
    qDebug("width of [y, u, v] == [%d, %d, %d]", y_w, u_w, v_w);

    // avifImageAllocatePlanes(avifImage, AVIF_PLANES_YUV);
    // turbojpeg will pad memory, but libavif don't
    // let's hack it

    int channelSize = avifImageUsesU16(avifImage) ? 2 : 1;
    int fullRowBytes = channelSize * (int)avifImage->width;
    avifPixelFormatInfo info;
    avifGetPixelFormatInfo(avifImage->yuvFormat, &info);
    int shiftedW = ((int)avifImage->width + info.chromaShiftX) >> info.chromaShiftX;
    // int shiftedH = ((int)avifImage->height + info.chromaShiftY) >> info.chromaShiftY;

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

    // Make libavif manage planes to avoid mem leak
    avifImage->imageOwnsYUVPlanes = AVIF_TRUE;

    tjDecompressToYUVPlanes(
                handle,
                jpeg_buf,
                jpeg_size,
                avifImage->yuvPlanes,
                width,
                nullptr,
                height,
                0
    );

    rearrage(plane_y, plane_y, (size_t)y_w, (size_t)fullRowBytes, height);

    tjDestroy(handle);
    auto encoder = avifEncoderCreate();
    if (! encoder) {
        qCritical("can't create avif encoder");
        avifImageDestroy(avifImage);
        return false;
    }
    encoder->maxThreads = QThread::idealThreadCount();
    encoder->minQuantizer = settings.minQuantizer;
    encoder->maxQuantizer = settings.maxQuantizer;
    encoder->codecChoice = AVIF_CODEC_CHOICE_RAV1E;
    encoder->speed = settings.encodeSpeed;

    qDebug("starting encode: mt=%d, minQ=%d, maxQ=%d, speed=%d",
           encoder->maxThreads,
           encoder->minQuantizer,
           encoder->maxQuantizer,
           encoder->speed);

    avifRWData raw = AVIF_DATA_EMPTY;
    auto&& encodeResult = avifEncoderWrite(encoder, avifImage, &raw);
    if (encodeResult != AVIF_RESULT_OK){
        qCritical("avif encode failed");
        avifImageDestroy(avifImage);
        avifRWDataFree(&raw);
        avifEncoderDestroy(encoder);
        return false;
    }

    qint64&& write_size = avifFile.write(reinterpret_cast<const char*>(raw.data), static_cast<qint64>(raw.size));
    qint64&& should_write = static_cast<qint64>(raw.size);

    avifImageDestroy(avifImage);
    avifRWDataFree(&raw);
    avifEncoderDestroy(encoder);

    if (write_size != should_write) {
        qCritical("wrote size don't match file size");
        qCritical("should write %lld, wrote %lld", should_write, write_size);
        return false;
    }

    return true;
}

bool JpegAvifConverter::ConvertAvifToJpeg(const QString &avifpath, const QString &jpegpath) const{
    bool ret = true;

    avifROData raw;
    QFile jpegFile(jpegpath);
    QFile avifFile(avifpath);

    if (!jpegFile.open(QIODevice::WriteOnly)){                        // [open]  jpegFile
        qCritical("Can't open file: %s", jpegpath.toUtf8().constData());
        return false;
    }
    if (!avifFile.open(QIODevice::ReadOnly)){                      // [open]  avifFile
        qCritical("Can't open file: %s", avifpath.toUtf8().constData());
        return false;                                               // !EXIT!
    }

    qDebug("Encode Jpeg to file: %s", jpegpath.toUtf8().constData());

    auto avifBytes = avifFile.readAll();

    raw.data = reinterpret_cast<uint8_t*>(avifBytes.data());
    raw.size = static_cast<std::make_unsigned<int>::type>(avifBytes.length());

    avifImage *image = avifImageCreateEmpty();
    avifDecoder *decoder = avifDecoderCreate();
    avifResult decodeResult = avifDecoderRead(decoder, image, &raw);
    int encodeResult;
    tjhandle handle = nullptr;
    int subsample;
    unsigned char *jpegBuf = nullptr;
    unsigned long jpegSize = 0;

    if (decodeResult == AVIF_RESULT_OK){
        qDebug("avif decode ok");
        subsample = format_a2j(image->yuvFormat);

        int width = (int)image->width;
        int height = (int)image->height;
        int channelSize = avifImageUsesU16(image) ? 2 : 1;
        int fullRowBytes = channelSize * (int)image->width;
        avifPixelFormatInfo info;
        avifGetPixelFormatInfo(image->yuvFormat, &info);

        size_t y_size = tjPlaneSizeYUV(0, width, 0, height, subsample);

        int y_w = tjPlaneWidth(0, width, subsample);

        uint8_t * y_buffer = nullptr;
        uint8_t * yuvPlanes[3];

        if (y_w != fullRowBytes){
            y_buffer = new uint8_t[(size_t)y_size];
            rearrage(image->yuvPlanes[0], y_buffer, (size_t)fullRowBytes, (size_t)y_w, height);
            yuvPlanes[0] = y_buffer;
            yuvPlanes[1] = image->yuvPlanes[1];
            yuvPlanes[2] = image->yuvPlanes[2];
        }
        else {
            yuvPlanes[0] = image->yuvPlanes[0];
            yuvPlanes[1] = image->yuvPlanes[1];
            yuvPlanes[2] = image->yuvPlanes[2];
        }

        if (subsample != -1){
            handle = tjInitCompress();
            if (handle != nullptr){
                encodeResult = tjCompressFromYUVPlanes(
                            handle,
                            const_cast<const uint8_t**>(reinterpret_cast<uint8_t**>(yuvPlanes)),
                            static_cast<std::make_signed<unsigned int>::type>(image->width),
                            nullptr,
                            static_cast<std::make_signed<unsigned int>::type>(image->height),
                            subsample,
                            &jpegBuf,
                            &jpegSize,
                            settings.jpegQuality,
                            0
                            );
                if (encodeResult != -1){
                    qDebug("turbojpeg encode ok");
                    // Here, We got the metadatas from avif
                    // and JPEG from turbojpeg

                    if (settings.isSaveJpegExif){
                        // build a header
                        quint16 m;
                        m = qToBigEndian(JpegHeaderReader::M_SOI);
                        jpegFile.write( reinterpret_cast<char *>(&m), 2);

                        qDebug() << "EXIF size: "<< image->exif.size;
                        if (image->exif.size >= 2){
                            m = qToBigEndian(JpegHeaderReader::M_APP1);
                            jpegFile.write( reinterpret_cast<char *>(&m), 2);
                            jpegFile.write(
                                reinterpret_cast<char*>(image->exif.data),
                                static_cast<std::make_signed<size_t>::type>(image->exif.size)
                            );
                        }

                        qDebug() << "ICC size: "<< image->icc.size;
                        if (image->icc.size >= 2){
                            m = qToBigEndian(JpegHeaderReader::M_APP2);
                            jpegFile.write( reinterpret_cast<char *>(&m), 2);
                            jpegFile.write(
                                reinterpret_cast<char*>(image->icc.data),
                                static_cast<std::make_signed<size_t>::type>(image->icc.size)
                            );
                        }


                        int i = 0;
                        // It's quite straightforward, is it ?
#define get_word(x)  (qFromBigEndian(*reinterpret_cast<quint16 *>(&jpegBuf[x])))

                        Q_ASSERT( get_word(i) == JpegHeaderReader::M_SOI );
                        i += 2; // Skip SOI

                        while ( (m = get_word(i)) != JpegHeaderReader::M_SOS){
                            if ( (m & 0xFFE0) == 0xFFE0 ){ // APP0 to APP15 is 0xFFE0 to 0xFFEF
                                // get_word(i + 2) is the next word, the length of segment
                                // But the marker itself is not included, so plus 2 to skip it too
                                i += get_word(i + 2) + 2;
                            }
                            else {
                                jpegFile.write(
                                    reinterpret_cast<char*>(&jpegBuf[i]),
                                    get_word(i + 2) + 2);

                                i += get_word(i + 2) + 2;
                            }
                        }
#undef get_word

                        // m == M_SOS here
                        Q_ASSERT( static_cast<std::make_unsigned<int>::type>(i) < jpegSize );

                        jpegFile.write( reinterpret_cast<char*>(&jpegBuf[i]),
                                        static_cast<std::make_signed<size_t>::type>(jpegSize) - i);
                    }
                    else {
                        // TODO: NO ICC Write
                        jpegFile.write( reinterpret_cast<char*>(jpegBuf),
                                        static_cast<std::make_signed<size_t>::type>(jpegSize));
                    }
                }
                else {
                    qCritical("jpeg encode failed");
                    ret = false;
                }
            }
            else {
                qCritical("turbojpeg init failed");
                ret = false;
            }
            tjDestroy(handle);
        }
        else {
            qCritical("unsupported format");
            ret = false;
        }

        // those points may not set, but it's safe to delete them
        delete y_buffer;
    }
    else {
        qCritical("avif decode failed");
        qCritical("ERROR: Failed to decode: %s\n", avifResultToString(decodeResult));
        ret = false;
    }
    avifDecoderDestroy(decoder);
    avifImageDestroy(image);
    return ret;
}

bool JpegAvifConverter::ImageToAvif(QImage &image, const QString &path) const {
    if (image.isNull())
        return false;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    bool ret = true;
    int depth = 8;
    avifPixelFormat format = AVIF_PIXEL_FORMAT_YUV420;
    avifImage *dimage = avifImageCreate(image.width(), image.height(), depth, format);

    avifRGBImage rgb;
    avifRGBImageSetDefaults(&rgb, dimage);
    rgb.depth = 8;
    rgb.format = AVIF_RGB_FORMAT_RGBA;

    if (image.format() != QImage::Format_RGBA8888){
        image.convertTo(QImage::Format_RGBA8888);
    }

    rgb.pixels = image.bits();
    rgb.rowBytes = image.bytesPerLine();

    // SRGB color space
    dimage->colorPrimaries = AVIF_COLOR_PRIMARIES_IEC61966_2_4;
    dimage->transferCharacteristics
            = AVIF_TRANSFER_CHARACTERISTICS_SRGB;

    avifImageRGBToYUV(dimage, &rgb);

    avifRWData output = AVIF_DATA_EMPTY;
    avifEncoder *encoder = avifEncoderCreate();
    encoder->maxThreads = QThread::idealThreadCount();
    encoder->maxQuantizer = settings.maxQuantizer;
    encoder->minQuantizer = settings.minQuantizer;
    encoder->maxQuantizerAlpha = settings.maxQuantizer;
    encoder->minQuantizerAlpha = settings.minQuantizer;
    avifResult encoderResult = avifEncoderWrite(encoder, dimage, &output);
    if (encoderResult == AVIF_RESULT_OK){
        file.write((char *)output.data, output.size);
        ret = true;
    }
    else {
        ret = false;
    }

    avifImageDestroy(dimage);
    avifRWDataFree(&output);
    avifEncoderDestroy(encoder);

    return ret;
}
