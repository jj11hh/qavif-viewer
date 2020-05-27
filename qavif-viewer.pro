#-------------------------------------------------
#
# Project created by QtCreator 2018-02-10T15:54:43
#
#-------------------------------------------------

QT       += core gui
CONFIG   += c++17

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qavif-viewer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
    dialogsettings.cpp \
    imageview.cpp \
    convertsettings.cpp\
    jpegavifconverter.cpp \
    jpegheaderreader.cpp \
    mainwindow.cpp \
    myimagereader.cpp

HEADERS  += mainwindow.h \
    dialogsettings.h \
    imageview.h \
    convertsettings.h \
    jpegavifconverter.h \
    jpegheaderreader.h \
    myimagereader.h

LIBS += -L"$$_PRO_FILE_PWD_/thirdparty/lib" -lavif -laom -ldav1d -lrav1e -lturbojpeg
INCLUDEPATH += $$_PRO_FILE_PWD_/thirdparty/include

FORMS    += mainwindow.ui \
    dialogsettings.ui
TRANSLATIONS = app_zh_CN.ts app_en_US.ts

win32-msvc* {
    QMAKE_CXXFLAGS += /utf-8
    LIBS += -lws2_32 -ladvapi32 -lpsapi -lUserenv
    RC_ICONS = Images/icon.ico
}

unix {
    LIBS += -ldl
}

DISTFILES += \
    app_zh_CN.qm app_en_US.qm

RESOURCES += \
    resouce.qrc


