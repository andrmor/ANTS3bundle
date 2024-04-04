#ifndef AGRAPHWIN_SI_H
#define AGRAPHWIN_SI_H

#include "awindowinterfacebase.h"

#include <QString>
#include <QVariant>
#include <QVariantList>

class GraphWindowClass;

class AGraphWin_SI : public AWindowInterfaceBase
{
    Q_OBJECT

public:
    AGraphWin_SI(GraphWindowClass * graphWin);

    AScriptInterface * cloneBase() const {return new AGraphWin_SI(GraphWindow);}

public slots:
    void setLog(bool Xaxis, bool Yaxis);

    void setStatPanelVisible(bool flag);

    void addLegend(double x1, double y1, double x2, double y2, QString title);
    void setLegendBorder(int color, int style, int size);

    void addText(QString text, bool Showframe, int Alignment_0Left1Center2Right);
    void addTextScreenXY(QString text, bool Showframe, int Alignment_0Left1Center2Right, double x1, double y1, double x2, double y2);
    void addTextTrueXY(QString text, bool Showframe, int Alignment_0Left1Center2Right, double x1, double y1, double x2, double y2);

    void addLine(double x1, double y1, double x2, double y2, int color, int width, int style);
    void addArrow(double x1, double y1, double x2, double y2, int color, int width, int style);

    void addToBasket(QString Title);
    void clearBasket();

    void saveImage(QString fileName);

    void exportTH2AsText(QString fileName); // !!!*** need?

    void show3D(QString castorFileName);

    /*
    QVariant GetProjection();
    void ConfigureProjectionTool(double x0, double y0, double dx, double dy, double angle);
    void UseProjectionTool(QString option);
    */

    QVariantList getAxes();

    QVariantList getContent();

signals:
    void requestShow3D(QString fileName);

private:
    GraphWindowClass * GraphWindow = nullptr;
};

#endif // AGRAPHWIN_SI_H
