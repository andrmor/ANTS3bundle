#ifndef ALRFVIEWEROBJECT_H
#define ALRFVIEWEROBJECT_H

#include <QObject>
#include <QColor>

#include <vector>

class ASensorHub;
class ALrfGraphicsView;
class QGraphicsScene;
class QGraphicsItem;

class PMpropsClass
{
public:
    QColor pen;
    QColor brush;
    QString text;
    QColor textColor;
    bool visible;

    PMpropsClass() {pen = Qt::black; brush = Qt::white; text = ""; textColor = Qt::black; visible = true;}
};

class ALrfViewerObject : public QObject
{
    Q_OBJECT
public:
    explicit ALrfViewerObject(ALrfGraphicsView * GV);
    ~ALrfViewerObject();

    void DrawAll();
    void ResetViewport();

    void ClearColors();
    void SetPenColor(int ipm, QColor color);
    void SetBrushColor(int ipm, QColor color);
    void SetText(int ipm, QString text);
    void SetTextColor(int ipm, QColor color);
    void SetVisible(int ipm, bool fFlag);
    void SetCursorMode(int option); //0-normal (hands), 1-cross only

//signals:  not needed?
//    void PMselectionChanged(QVector<int>);

public slots:
    void forceResize();

private slots:
    void sceneSelectionChanged();

private:
    ASensorHub & SensHub;

    std::vector<QGraphicsItem*> SensIcons;
    std::vector<PMpropsClass>   SensProps;

    ALrfGraphicsView * GrView = nullptr;
    QGraphicsScene   * Scene;
    double GVscale;
    int    CursorMode;

};

#endif // ALRFVIEWEROBJECT_H
