#ifndef JPEGAVIFCONVERTER_H
#define JPEGAVIFCONVERTER_H

#include "imgconvsettings.h"

#include <QIODevice>
#include <QImage>

class JpegAvifConverter
{
public:
    JpegAvifConverter(const ImgConvSettings &);
    bool ConvertJpegToAvif(const QString &, const QString &) const;
    bool ConvertAvifToJpeg(const QString &, const QString &) const;
    bool ImageToAvif(QImage &, const QString &) const;

private:
    ImgConvSettings settings;
};

#endif // JPEGAVIFCONVERTER_H
