#ifndef JPEGAVIFCONVERTER_H
#define JPEGAVIFCONVERTER_H

#include "convertsettings.h"
#include "Image.h"
#include <string>

class JpegAvifConverter
{
public:
    JpegAvifConverter(const ConvertSettings &);
    bool ConvertJpegToAvif(const std::string &, const std::string &) const;
    bool ConvertAvifToJpeg(const std::string &, const std::string &) const;
    bool ImageToAvif(const Image &, const std::string &) const;

private:
    ConvertSettings settings;
};

#endif // JPEGAVIFCONVERTER_H