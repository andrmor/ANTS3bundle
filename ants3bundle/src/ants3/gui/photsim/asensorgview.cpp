#include "asensorgview.h"

#include <QMouseEvent>

void ASensorGView::wheelEvent(QWheelEvent * event)
{
    if (event->angleDelta().y() > 0) scale(1.1, 1.1);
    else                             scale(1.0/1.1, 1.0/1.1);
}

void ASensorGView::mouseMoveEvent(QMouseEvent *event)
{
    MousePosition = this->mapToScene(event->pos());
    emit mouseMoved(&MousePosition);
    QGraphicsView::mouseMoveEvent(event);
}

void ASensorGView::enterEvent(QEvent *event)
{
    QGraphicsView::enterEvent(event);
    if (CursorMode == 1)
        viewport()->setCursor(Qt::CrossCursor);
}

void ASensorGView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    if (CursorMode == 1)
        viewport()->setCursor(Qt::CrossCursor);
}

void ASensorGView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    if (CursorMode == 1)
        viewport()->setCursor(Qt::CrossCursor);
}
