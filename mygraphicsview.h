#pragma once

#include <QGraphicsView>
#include <QWheelEvent>
#include <QKeyEvent>

class MyGraphicsView : public QGraphicsView
{
	Q_OBJECT

public:
    MyGraphicsView(QWidget *parent = nullptr);
    ~MyGraphicsView() override;
	void viewFit();
    void zoomIn();
    void zoomOut();

private:
    void scaleView(qreal scaleFactor);
    bool isResized;
	bool isLandscape;

protected:
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
};
