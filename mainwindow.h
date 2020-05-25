#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool loadImage(const QString &path);

private:
    Ui::MainWindow *ui;
    QGraphicsScene* m_graphicsScene;
    void saveImage();
    void openImage();
    void about();
    void fitWindow();
    QString lastOpenPath;

private slots:
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionAbout_triggered();
};

#endif // MAINWINDOW_H
