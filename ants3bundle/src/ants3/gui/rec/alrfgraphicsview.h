#ifndef ALRFGRAPHICSVIEW_H
#define ALRFGRAPHICSVIEW_H

#include <QObject>
#include <QGraphicsView>

class ALrfGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    ALrfGraphicsView( QWidget * parent = 0 );
    ALrfGraphicsView( QGraphicsScene * scene, QWidget * parent = 0 );
    void setCursorMode(int mode) {CursorMode = mode;}

protected:
    void wheelEvent ( QWheelEvent * event ) override;
    void mouseMoveEvent(QMouseEvent * event) override;
    void enterEvent(QEnterEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;

private:
    QPointF MousePosition;
    int CursorMode;

signals:
    void MouseMovedSignal(QPointF *Pos);
};

#endif // ALRFGRAPHICSVIEW_H
