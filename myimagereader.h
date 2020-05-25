#ifndef MYIMAGEREADER_H
#define MYIMAGEREADER_H

#include <QImageReader>


class MyImageReader
{
public:
    MyImageReader(const QString &fileName);
    ~MyImageReader();
    QImage read();

private:
    QImageReader *reader;
    QString path;
    QByteArray format;
};

#endif // MYIMAGEREADER_H
