#ifndef CONVERTSETTINGS_H
#define CONVERTSETTINGS_H

#include <QObject>

class ConvertSettings
{
public:
    ConvertSettings(int minQ=10, int maxQ=10, int encodeSpeed = 8, int jpegQ=90, bool saveExif=true, bool restoreExif=true);
    ConvertSettings(const ConvertSettings &other);
    ~ConvertSettings();
    int minQuantizer = 0;
    int maxQuantizer = 0;
    int encodeSpeed = 0;
    int jpegQuality = 0;
    bool isSaveAvifExif = true;
    bool isSaveJpegExif = true;
};

Q_DECLARE_METATYPE(ConvertSettings);

#endif // CONVERTSETTINGS_H
