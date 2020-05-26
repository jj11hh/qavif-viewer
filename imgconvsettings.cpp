#include "imgconvsettings.h"

ImgConvSettings::ImgConvSettings(int minQ, int maxQ, int _encodeSpeed, int jpegQ, bool saveExif, bool restoreExif)
    : minQuantizer(minQ), maxQuantizer(maxQ), encodeSpeed(_encodeSpeed), jpegQuality(jpegQ), isSaveAvifExif(saveExif), isSaveJpegExif(restoreExif)
{}

ImgConvSettings::~ImgConvSettings(){}
ImgConvSettings::ImgConvSettings(const ImgConvSettings &other){
    minQuantizer = other.minQuantizer;
    maxQuantizer = other.maxQuantizer;
    encodeSpeed = other.encodeSpeed;
    jpegQuality = other.jpegQuality;
    isSaveAvifExif = other.isSaveAvifExif;
    isSaveJpegExif = other.isSaveJpegExif;
}
