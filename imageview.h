#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

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
    void setScale(qreal);

signals:
    void nextImage();
    void prevImage();
    void resized(qreal);

private:
    void scaleView(qreal scaleFactor);
    void updateScale();
    bool isResized;
	bool isLandscape;

protected:
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
};

#endif //IMAGEVIEW_H
