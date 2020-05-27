#include "jpegheaderreader.h"
#include <QtEndian>

void JpegHeaderReader::readMark(){
    if (atEnd())
        return;

    io->read(reinterpret_cast<char *>(&currentMark), sizeof(quint16));

    currentMark = qFromBigEndian(currentMark);
    if (currentMark == M_SOI || currentMark == M_SOS || !valid()){
        currentSize = 0;
    }
    else {
        io->read(reinterpret_cast<char *>(&currentSize), sizeof(quint16));
        currentSize = qFromBigEndian(currentSize);
    }
}

JpegHeaderReader::JpegHeaderReader(QIODevice *iodevice)
{
    Q_CHECK_PTR(iodevice);
    io = iodevice;

    io->read(reinterpret_cast<char *>(&currentMark), sizeof(quint16));

    currentMark = qFromBigEndian(currentMark);
    if (currentMark == M_SOI || currentMark == M_SOS || !valid()){
        currentSize = 0;
    }
    else {
        io->read(reinterpret_cast<char *>(&currentSize), sizeof(quint16));
        currentSize = qFromBigEndian(currentSize);
    }
}

int JpegHeaderReader::read(QByteArray &byteArray) {
    if (currentSize){
        quint16 size = qToBigEndian(currentSize);
        byteArray.append(reinterpret_cast<char *>(&size), 2);
        byteArray.append(io->read(currentSize - 2));

        readMark();
        return byteArray.length();
    }

    readMark();
    return 0;
}

QByteArray JpegHeaderReader::read(){
    QByteArray byteArray;
    this->read(byteArray);
    return byteArray;
}

void JpegHeaderReader::skip() {
    if (currentSize){
        io->skip(currentSize - 2);
    }
    readMark();
}

quint16 JpegHeaderReader::current(){
    return currentMark;
}

quint16 JpegHeaderReader::size(){
    return currentSize;
}

bool JpegHeaderReader::atEnd(){
    return io->atEnd() || currentMark == M_SOS || !valid();
}

bool JpegHeaderReader::valid(){
    return (currentMark & 0xFFC0) == 0xFFC0;
}
