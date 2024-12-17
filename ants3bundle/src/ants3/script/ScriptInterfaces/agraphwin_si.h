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
    void addToBasket(QString title);
    void clearBasket();

    void setLog(bool Xaxis, bool Yaxis);
    void setStatPanelVisible(bool flag);

    void addLegend(double x1, double y1, double x2, double y2, QString title = "");
    void setLegendBorder(int color, int style, int size);

    void addText(QString text, bool showframe, int alignment_0Left1Center2Right);
    void addTextAtScreenXY(QString text, bool Showframe, int Alignment_0Left1Center2Right, double x1, double y1, double x2, double y2);
    void addTextAtXY(QString text, bool Showframe, int Alignment_0Left1Center2Right, double x1, double y1, double x2, double y2);

    void addLine(double x1, double y1, double x2, double y2, int color, int width, int style);
    void addArrow(double x1, double y1, double x2, double y2, int color, int width, int style);

    QVariantList getAxisRanges();
    QVariantList getContent();

    void saveImage(QString fileName);

    void show3D(QString castorFileName);

    /*
    QVariant GetProjection();
    void ConfigureProjectionTool(double x0, double y0, double dx, double dy, double angle);
    void UseProjectionTool(QString option);
    */

signals:
    void requestShow3D(QString fileName, bool keepSettings = false);

    void requestAddToBasket(QString title);
    void requestClearBasket();
    void requestSetLog(bool Xaxis, bool Yaxis);
    void requestSetStatPanelVisible(bool flag);
    void requestAddLegend(double x1, double y1, double x2, double y2, QString title);
    void requestSetLegendBorder(int color, int style, int size);
    void requestAddText(QString text, bool Showframe, int Alignment_0Left1Center2Right, double x1, double y1, double x2, double y2, QString opt);
    void requestAddLine(double x1, double y1, double x2, double y2, int color, int width, int style);
    void requestAddArrow(double x1, double y1, double x2, double y2, int color, int width, int style);

private:
    GraphWindowClass * GraphWindow = nullptr;
};

#endif // AGRAPHWIN_SI_H
