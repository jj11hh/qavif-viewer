#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "myimagereader.h"
#include "dialogsettings.h"
#include "convertsettings.h"
#include "jpegavifconverter.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QFileInfo>
#include <QProgressDialog>
#include <QtConcurrent/QtConcurrentRun>

#include <QGraphicsPixmapItem>
#include <QImageReader>
#include <QImageWriter>
#include <QDebug>
#include <QRegularExpression>
#include <QThread>
#include <QtMath>

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    lastOpenPath("")
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/Images/icon.png"));
    m_graphicsScene = new QGraphicsScene();
    m_graphicsScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    QImage bground(50, 50, QImage::Format_RGB888);
    for (int y = 0; y < 25; y++)
    {
        for (int x = 0; x < 25; x++)
        {
            bground.setPixel(x, y, qRgb(0xCA, 0xCA, 0xCA));
            bground.setPixel(x + 25, y, qRgb(0xFF, 0xFF, 0xFF));
            bground.setPixel(x, y + 25, qRgb(0xFF, 0xFF, 0xFF));
            bground.setPixel(x + 25, y + 25, qRgb(0xCA, 0xCA, 0xCA));
        }
    }
    m_graphicsScene->setBackgroundBrush(QPixmap::fromImage(bground));

    ui->m_graphicsView->setScene(m_graphicsScene);

    ui->m_graphicsView->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    ui->m_graphicsView->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );

    ui->statusBar->showMessage(tr("ready"), 0);
    connect(ui->m_graphicsView, &ImageView::nextImage, this, &MainWindow::nextImage);
    connect(ui->m_graphicsView, &ImageView::prevImage, this, &MainWindow::prevImage);
    connect(ui->m_graphicsView, &ImageView::resized, this, &MainWindow::onResized);

    ui->m_graphicsView->viewFit();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openImage()
{
    if (! lastOpenPath.has_value()){
        lastOpenPath =
            QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    }

    QString qStrFilePath = QFileDialog::getOpenFileName(this,
        tr("Open Image"),
        lastOpenPath.value(),
        tr("Image Files (*.png *.jpg *.bmp *.avif)"));

    if (qStrFilePath.isEmpty())
        return;

    if (!loadImage(qStrFilePath)){
        QMessageBox msgBox;
        msgBox.setText(tr("Cannot read file"));
        msgBox.exec();
        return;
    }
}

bool MainWindow::loadImage(const QString &path){
    qDebug() << "loading " << path;

    currentPath = path;
    currentDir = QFileInfo(path).absoluteDir();
    MyImageReader reader(path);
    QImage qimg = reader.read();
    if (qimg.isNull())
    {
        ui->statusBar->showMessage(tr("Load failed: %1").arg(path), 0);
        if (!m_graphicsScene->sceneRect().isEmpty())
        {
            m_graphicsScene->clear();
        }
        return false;
    }
    if (!m_graphicsScene->sceneRect().isEmpty())
    {
        m_graphicsScene->clear();
    }
    m_graphicsScene->setSceneRect(qimg.rect());

    auto item = new QGraphicsPixmapItem(QPixmap::fromImage(qimg));
    item->setTransformationMode(Qt::SmoothTransformation);
    m_graphicsScene->addItem(item);

    ui->m_graphicsView->viewFit();

    ui->statusBar->showMessage(tr("Image loaded: %1").arg(path), 0);

    return true;
}

void MainWindow::nextImage(){
    if (! currentDir.has_value())
        return;
    QStringList filter = {"*.jpe", "*.jpeg", "*.jpg", "*.bmp", "*.png", "*.avif"};
    QStringList images = currentDir.value().entryList(filter, QDir::Files, QDir::Name);
    qDebug() << images.join(", ");
    if (images.length() == 0)
        return;

    QString currentFileName = QFileInfo(currentPath.value()).fileName();
    int found = images.indexOf(currentFileName);
    if (found == -1)
        found = 0;
    else {
        found++;
        if (found >= images.length())
            found = 0;
    }

    loadImage(currentDir.value().path() + '/' + images[found]);
}
void MainWindow::prevImage(){
    if (! currentDir.has_value())
        return;
    QStringList filter = {"*.jpe", "*.jpeg", "*.jpg", "*.bmp", "*.png", "*.avif"};
    QStringList images = currentDir.value().entryList(filter, QDir::Files, QDir::Name);
    if (images.length() == 0)
        return;

    QString currentFileName = QFileInfo(currentPath.value()).fileName();
    int found = images.indexOf(currentFileName);
    if (found == -1)
        found = 0;
    else {
        found--;
        if (found < 0)
            found = images.length() - 1;
    }

    loadImage(currentDir.value().path() + '/' + images[found]);
}
void MainWindow::saveImage()
{
    if (!currentPath.has_value())
        return;

    QString qStrFilePath = QFileDialog::getSaveFileName(this,
            tr("Save Image"),
            QStandardPaths::writableLocation(QStandardPaths::PicturesLocation).replace("cache", "newfile.jpg"),
            tr("JPG file (*.jpg);;PNG file (*.png);;BMP file (*.bmp);;AVIF file (*.avif)"));

    if (qStrFilePath.isEmpty())
        return;

    assert(currentPath.has_value());
    if (qStrFilePath.endsWith(".avif")){
        auto dialogSettings = DialogSettings(this, ConvertSettings());
        dialogSettings.exec();

        if (!dialogSettings.getAccepted()){
            return;
        }
        auto settings = dialogSettings.getSettings();
        auto convertor = JpegAvifConverter(settings);

        // It will be slow, show a progress bar and run in another thread

        QProgressDialog progress(tr("Converting files..."), tr("Abort Convert"), 0, 100, this);

        progress.setCancelButton(nullptr);
        progress.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
        progress.setWindowModality(Qt::WindowModal);
        progress.setRange(0, 100);
        progress.setValue(1);
        progress.show();
        QFuture<bool> future;
        if (currentPath.value().contains(QRegularExpression("\\.jp([eg]|eg)$", QRegularExpression::CaseInsensitiveOption))){
            future = QtConcurrent::run([=]() -> bool {
                return convertor.ConvertJpegToAvif(currentPath.value(), qStrFilePath);
            });
        }
        else {
            future = QtConcurrent::run([=]() -> bool {
                auto img = MyImageReader(currentPath.value()).read();
                return convertor.ImageToAvif(img, qStrFilePath);
            });
        }

        // It's a fake progress bar, not actual progress
        float i = 0;
        while(1){
            QThread::msleep(10);
            QApplication::processEvents();
            float d = i + 1;
            progress.setValue(100 - (100.0 / d));
            if (future.isFinished())
                break;
            if (i < 10000)
                i += 0.05f;
        }
        progress.setValue(100);
        progress.close();

        if (future.result()){
            ui->statusBar->showMessage(tr("image saved"), 0);
        }
        else {
            QMessageBox msgBox;
            msgBox.setText(tr("Cannot write file"));
            msgBox.exec();
            return;
        }
    }
    else { // Non-avif file

        auto img = MyImageReader(currentPath.value()).read();
        QImageWriter writer(qStrFilePath);
        if(!writer.canWrite())
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Cannot write file"));
            msgBox.exec();
            return;
        }
        writer.write(img);
        ui->statusBar->showMessage(tr("image saved"), 0);
    }
}

void MainWindow::fitWindow()
{
    ui->m_graphicsView->viewFit();
}

void MainWindow::on_actionOpen_triggered()
{
    openImage();
}

void MainWindow::on_actionSave_triggered()
{
    saveImage();
}

void MainWindow::onResized(qreal factor){
    ui->LbZoom->setText(QString::number(int(factor * 100)) + "%");
    ui->SlZoom->setValue(int(factor * 100));
}

void MainWindow::about(){
    QMessageBox::about(this, tr("About Qavif Viewer"),
            tr("<p>The <b>Qavif Viewer</b> is a simple image viewer "
               "that supports avif</p>"
               "<p>Author: Jiang Yiheng, https://github.com/jj11hh/qavif-viewer</p>"
               "<p>Copyright 2020 Jiang Yiheng</p>"

               "<p>Permission is hereby granted, free of charge, "
               "to any person obtaining a copy of this software and "
               "associated documentation files (the \"Software\"), to "
               "deal in the Software without restriction, including "
               "without limitation the rights to use, copy, modify, "
               "merge, publish, distribute, sublicense, and/or sell "
               "copies of the Software, and to permit persons to whom "
               "the Software is furnished to do so, subject to the "
               "following conditions:</p>"

               "<p>The above copyright notice and this permission "
               "notice shall be included in all copies or substantial "
               "portions of the Software.</p>"

               "<p>THE SOFTWARE IS PROVIDED \"AS IS\", "
               "WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, "
               "INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF "
               "MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE "
               "AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS "
               "OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES "
               "OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, "
               "TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN "
               "CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER "
               "DEALINGS IN THE SOFTWARE.</p>"
               ));
}

void MainWindow::on_actionAbout_triggered()
{
    about();
}

void MainWindow::on_BtnFitWindow_clicked()
{
    ui->m_graphicsView->viewFit();
}

void MainWindow::on_BtnOriginalSize_clicked()
{
    ui->m_graphicsView->setScale(1.0);
}

void MainWindow::on_SlZoom_valueChanged(int value)
{
    ui->m_graphicsView->setScale(value / 100.0);
}
