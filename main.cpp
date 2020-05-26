#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QTranslator>
#include <QDebug>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString locale = QLocale::system().name();
    QTranslator translator;
    if (translator.load("translations/app_" +locale +".qm", ":/")){
        qDebug() << "Translate file app_" +locale + " loaded" ;
        a.installTranslator(&translator);
    }
    else {
        qDebug() << "No translate file loaded" ;
    }

    MainWindow w;

    QCommandLineParser commandLineParser;
    commandLineParser.addHelpOption();
    commandLineParser.addPositionalArgument(MainWindow::tr("[file]"), MainWindow::tr("Image file to open."));
    commandLineParser.process(QCoreApplication::arguments());
    if (!commandLineParser.positionalArguments().isEmpty()){
        auto path = commandLineParser.positionalArguments().front();
        QTimer::singleShot(100, &w, [path, &w](){w.loadImage(path);});
    }


    w.show();
    return a.exec();
}
