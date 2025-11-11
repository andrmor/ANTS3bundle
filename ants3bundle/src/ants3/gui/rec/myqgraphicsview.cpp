#include "myqgraphicsview.h"

#include <QLineEdit>
#include <QWheelEvent>

myQGraphicsView::myQGraphicsView(QWidget *parent) : QGraphicsView(parent)
{
    CursorMode = 0;
}

myQGraphicsView::myQGraphicsView(QGraphicsScene *scene, QWidget *parent) : QGraphicsView(scene,parent)
{
    CursorMode = 0;
}

void myQGraphicsView::wheelEvent(QWheelEvent * event)
{
    const int delta = event->angleDelta().y();
    if (delta > 0) scale(1.1, 1.1);
    else           scale(1.0/1.1, 1.0/1.1);
}

void myQGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    MousePosition = this->mapToScene(event->pos());
    //MousePosition = event->pos();
    //  qDebug() << MousePosition << this->mapToScene(event->pos());
    emit MouseMovedSignal(&MousePosition);
    QGraphicsView::mouseMoveEvent(event);
}

void myQGraphicsView::enterEvent(QEnterEvent * event)
{
    QGraphicsView::enterEvent(event);
    if (CursorMode == 1)
        viewport()->setCursor(Qt::CrossCursor);
}

void myQGraphicsView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    if (CursorMode == 1)
        viewport()->setCursor(Qt::CrossCursor);
}

void myQGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    if (CursorMode == 1)
        viewport()->setCursor(Qt::CrossCursor);
}
