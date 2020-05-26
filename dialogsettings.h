#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include "imgconvsettings.h"
#include <QDialog>

namespace Ui {
class DialogSettings;
}

class DialogSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSettings(QWidget *parent, const ImgConvSettings &settings);
    ~DialogSettings();

    ImgConvSettings getSettings() const;
    void setSettings(const ImgConvSettings &value);
    bool getAccepted() const;

private slots:
    void on_sliderMaxQuantizer_valueChanged(int value);
    void on_sliderMinQuantizer_valueChanged(int value);
    void on_DialogSettings_accepted();
    void on_sliderJpegQuality_valueChanged(int value);
    void on_sliderEncodeSpeed_valueChanged(int value);

private:
    Ui::DialogSettings *ui;
    ImgConvSettings settings;
    bool accepted;
};

#endif // DIALOGSETTINGS_H
