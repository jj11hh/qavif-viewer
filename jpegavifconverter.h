#ifndef JPEGAVIFCONVERTER_H
#define JPEGAVIFCONVERTER_H

#include "convertsettings.h"

#include <QIODevice>
#include <QImage>

class JpegAvifConverter
{
public:
    JpegAvifConverter(const ConvertSettings &);
    bool ConvertJpegToAvif(const QString &, const QString &) const;
    bool ConvertAvifToJpeg(const QString &, const QString &) const;
    bool ImageToAvif(QImage &, const QString &) const;

private:
    ConvertSettings settings;
};

#endif // JPEGAVIFCONVERTER_H
