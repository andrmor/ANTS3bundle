#ifndef AROOTLINECONFIGURATOR_H
#define AROOTLINECONFIGURATOR_H

#include <QDialog>

class QPainter;
class QSpinBox;
class QComboBox;
class QFrame;

class ARootLineConfigurator : public QDialog
{
    Q_OBJECT
public:
    ARootLineConfigurator(int & color, int & width, int & style, QWidget * parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);

private:
    int SquareSize = 30;
    QList<int> BaseColors;

    int & ReturnColor;
    int & ReturnWidth;
    int & ReturnStyle;

    QSpinBox* sbWidth;
    QComboBox* comStyle;
    QSpinBox* sbColor;
    QFrame* frCol;

    void PaintColorRow(QPainter *p, int row, int colorBase);

private slots:
    void updateColorFrame();
    void previewColor();
    void onAccept();
    void onUserAction();

signals:
    void propertiesChanged(int color, int width, int style);

};

#endif // AROOTLINECONFIGURATOR_H
