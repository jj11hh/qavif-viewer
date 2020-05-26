#include "jpegsegreader.h"
#include <QtEndian>

void JpegSegReader::readMarker(){
    if (atEnd())
        return;

    io->read(reinterpret_cast<char *>(&currentMarker), sizeof(quint16));

    currentMarker = qFromBigEndian(currentMarker);
    if (currentMarker == M_SOI || currentMarker == M_SOS || !valid()){
        currentSize = 0;
    }
    else {
        io->read(reinterpret_cast<char *>(&currentSize), sizeof(quint16));
        currentSize = qFromBigEndian(currentSize);
    }
}

JpegSegReader::JpegSegReader(QIODevice *iodevice)
{
    Q_CHECK_PTR(iodevice);
    io = iodevice;

    io->read(reinterpret_cast<char *>(&currentMarker), sizeof(quint16));

    currentMarker = qFromBigEndian(currentMarker);
    if (currentMarker == M_SOI || currentMarker == M_SOS || !valid()){
        currentSize = 0;
    }
    else {
        io->read(reinterpret_cast<char *>(&currentSize), sizeof(quint16));
        currentSize = qFromBigEndian(currentSize);
    }
}

int JpegSegReader::read(QByteArray &byteArray) {
    if (currentSize){
        quint16 size = qToBigEndian(currentSize);
        byteArray.append(reinterpret_cast<char *>(&size), 2);
        byteArray.append(io->read(currentSize - 2));

        readMarker();
        return byteArray.length();
    }

    readMarker();
    return 0;
}

QByteArray JpegSegReader::read(){
    QByteArray byteArray;
    this->read(byteArray);
    return byteArray;
}

void JpegSegReader::skip() {
    if (currentSize){
        io->skip(currentSize - 2);
    }
    readMarker();
}

quint16 JpegSegReader::current(){
    return currentMarker;
}

quint16 JpegSegReader::size(){
    return currentSize;
}

bool JpegSegReader::atEnd(){
    return io->atEnd() || currentMarker == M_SOS || !valid();
}

bool JpegSegReader::valid(){
    return (currentMarker & 0xFFC0) == 0xFFC0;
}
