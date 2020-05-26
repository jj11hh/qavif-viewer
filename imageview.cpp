#include "imageview.h"
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QtMath>

ImageView::ImageView(QWidget *parent)
	: QGraphicsView(parent)
{
	setCacheMode(CacheBackground);
	setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
	setTransformationAnchor(AnchorUnderMouse);

	isResized = false;
	isLandscape = false;

    setDragMode(QGraphicsView::ScrollHandDrag);
}

ImageView::~ImageView()
{
}

void ImageView::zoomIn()
{
	scaleView(qreal(1.2));
}

void ImageView::zoomOut()
{
	scaleView(1 / qreal(1.2));
}

void ImageView::viewFit()
{
	fitInView(sceneRect(), Qt::KeepAspectRatio);
    updateScale();
    isResized = true;

	if (sceneRect().width() > sceneRect().height())
		isLandscape = true;
	else
		isLandscape = false;
}

void ImageView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() == Qt::NoModifier)
	{
		if (event->delta() > 0) zoomIn();
		else zoomOut();
	}
	else if (event->modifiers() == Qt::ShiftModifier)
	{
		QWheelEvent fakeEvent(event->pos(), event->delta(), event->buttons(), Qt::NoModifier, Qt::Horizontal);
		QGraphicsView::wheelEvent(&fakeEvent);
	}
    else if (event->modifiers() == Qt::ControlModifier)
	{
		QGraphicsView::wheelEvent(event);
	}
}

void ImageView::scaleView(qreal scaleFactor)
{
    if(sceneRect().isEmpty())
        return;

	QRectF expectedRect = transform().scale(scaleFactor, scaleFactor).mapRect(sceneRect());
	qreal expRectLength;
	int viewportLength;
	int imgLength;

	if (isLandscape)
	{
		expRectLength = expectedRect.width();
		viewportLength = viewport()->rect().width();
        imgLength = int(sceneRect().width());
	}
	else
	{
		expRectLength = expectedRect.height();
		viewportLength = viewport()->rect().height();
        imgLength = int(sceneRect().height());
	}

    if (expRectLength < viewportLength / 2) // minimum zoom : half of viewport
	{
        if (!isResized || scaleFactor < 1)
            scaleFactor = scaleFactor * (viewportLength/2 / expRectLength);
	}
    else if (expRectLength > imgLength * 3) // maximum zoom : x3
	{
		if (!isResized || scaleFactor > 1)
            scaleFactor = scaleFactor * (imgLength*3 / expRectLength);
	}
	else
	{
		isResized = false;
	}


	scale(scaleFactor, scaleFactor);
    updateScale();
}

void ImageView::setScale(qreal factor){
    resetMatrix();

    scaleView(factor);
    updateScale();
}

void ImageView::updateScale(){
    auto trans = transform();
    emit resized(qreal(sqrt(trans.m11() * trans.m11() + trans.m12() * trans.m12())));
}

void ImageView::resizeEvent(QResizeEvent *event)
{
	isResized = true;
	QGraphicsView::resizeEvent(event);
}

void ImageView::keyPressEvent(QKeyEvent *event){
    if (event->key() == Qt::Key_Left){
        emit prevImage();
    }
    else if (event->key() == Qt::Key_Right){
        emit nextImage();
    }
}
