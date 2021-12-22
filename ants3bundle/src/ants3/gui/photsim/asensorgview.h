#ifndef ASENSORGVIEW_H
#define ASENSORGVIEW_H

#include <QGraphicsView>
#include <QPointF>

class ASensorGView : public QGraphicsView
{
    Q_OBJECT

public:
    ASensorGView(QWidget * parent) : QGraphicsView(parent) {}
    ASensorGView(QGraphicsScene * scene, QWidget * parent) : QGraphicsView(scene, parent) {}

    int     CursorMode = 0;

signals:
    void mouseMoved(QPointF * Pos);

protected:
    void wheelEvent (QWheelEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;
    void enterEvent(QEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;

private:
    QPointF MousePosition;
};

#endif // ASENSORGVIEW_H
