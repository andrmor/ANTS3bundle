#include "alrfgraphicsview.h"

#include <QLineEdit>
#include <QWheelEvent>

ALrfGraphicsView::ALrfGraphicsView(QWidget *parent) :
    QGraphicsView(parent) {}

ALrfGraphicsView::ALrfGraphicsView(QGraphicsScene * scene, QWidget * parent) :
    QGraphicsView(scene, parent) {}

void ALrfGraphicsView::wheelEvent(QWheelEvent * event)
{
    const int delta = event->angleDelta().y();
    if (delta > 0) scale(1.1, 1.1);
    else           scale(1.0/1.1, 1.0/1.1);
}

void ALrfGraphicsView::mouseMoveEvent(QMouseEvent * event)
{
    MousePosition = mapToScene(event->pos());
    emit mouseMovedSignal(&MousePosition);

    QGraphicsView::mouseMoveEvent(event);
}

void ALrfGraphicsView::enterEvent(QEnterEvent * event)
{
    QGraphicsView::enterEvent(event);
    if (CursorMode == 1)
        viewport()->setCursor(Qt::CrossCursor);
}

void ALrfGraphicsView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    if (CursorMode == 1)
        viewport()->setCursor(Qt::CrossCursor);
}

void ALrfGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    if (CursorMode == 1)
        viewport()->setCursor(Qt::CrossCursor);
}
