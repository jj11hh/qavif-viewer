#ifndef IMGCONVSETTINGS_H
#define IMGCONVSETTINGS_H

#include <QObject>

class ImgConvSettings
{
public:
    ImgConvSettings(int minQ=10, int maxQ=10, int encodeSpeed = 8, int jpegQ=90, bool saveExif=true, bool restoreExif=true);
    ImgConvSettings(const ImgConvSettings &other);
    ~ImgConvSettings();
    int minQuantizer = 0;
    int maxQuantizer = 0;
    int encodeSpeed = 0;
    int jpegQuality = 0;
    bool isSaveAvifExif = true;
    bool isSaveJpegExif = true;
};

Q_DECLARE_METATYPE(ImgConvSettings);

#endif // IMGCONVSETTINGS_H
