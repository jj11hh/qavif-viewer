#pragma once

#include <QGraphicsView>
#include <QWheelEvent>
#include <QKeyEvent>

class ImageView : public QGraphicsView
{
	Q_OBJECT

public:
    ImageView(QWidget *parent = nullptr);
    ~ImageView() override;
	void viewFit();
    void zoomIn();
    void zoomOut();

signals:
    void nextImage();
    void prevImage();

private:
    void scaleView(qreal scaleFactor);
    bool isResized;
	bool isLandscape;

protected:
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
};
