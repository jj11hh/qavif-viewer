#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QCommandLineParser commandLineParser;
    commandLineParser.addHelpOption();
    commandLineParser.addPositionalArgument(MainWindow::tr("[file]"), MainWindow::tr("Image file to open."));
    commandLineParser.process(QCoreApplication::arguments());
    if (!commandLineParser.positionalArguments().isEmpty()
        && !w.loadImage(commandLineParser.positionalArguments().front())) {
        return -1;
    }

    w.show();
    return a.exec();
}
