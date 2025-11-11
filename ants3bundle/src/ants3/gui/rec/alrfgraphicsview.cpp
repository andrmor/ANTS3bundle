#include "alrfgraphicsview.h"

#include <QLineEdit>
#include <QWheelEvent>

ALrfGraphicsView::ALrfGraphicsView(QWidget *parent) : QGraphicsView(parent)
{
    CursorMode = 0;
}

ALrfGraphicsView::ALrfGraphicsView(QGraphicsScene *scene, QWidget *parent) : QGraphicsView(scene,parent)
{
    CursorMode = 0;
}

void ALrfGraphicsView::wheelEvent(QWheelEvent * event)
{
    const int delta = event->angleDelta().y();
    if (delta > 0) scale(1.1, 1.1);
    else           scale(1.0/1.1, 1.0/1.1);
}

void ALrfGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    MousePosition = this->mapToScene(event->pos());
    //MousePosition = event->pos();
    //  qDebug() << MousePosition << this->mapToScene(event->pos());
    emit MouseMovedSignal(&MousePosition);
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
