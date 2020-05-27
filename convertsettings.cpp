#include "convertsettings.h"

ConvertSettings::ConvertSettings(int minQ, int maxQ, int _encodeSpeed, int jpegQ, bool saveExif, bool restoreExif)
    : minQuantizer(minQ), maxQuantizer(maxQ), encodeSpeed(_encodeSpeed), jpegQuality(jpegQ), isSaveAvifExif(saveExif), isSaveJpegExif(restoreExif)
{}

ConvertSettings::~ConvertSettings(){}
ConvertSettings::ConvertSettings(const ConvertSettings &other){
    minQuantizer = other.minQuantizer;
    maxQuantizer = other.maxQuantizer;
    encodeSpeed = other.encodeSpeed;
    jpegQuality = other.jpegQuality;
    isSaveAvifExif = other.isSaveAvifExif;
    isSaveJpegExif = other.isSaveJpegExif;
}
