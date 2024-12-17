#include "agraphwin_si.h"
#include "graphwindowclass.h"

#include <QApplication>
#include <QDebug>

AGraphWin_SI::AGraphWin_SI(GraphWindowClass * graphWin) :
    AWindowInterfaceBase(graphWin), GraphWindow(graphWin)
{
    Description = "Access to the Graph window";

    Help["setLog"] = "Switch between log and linear scale for X and Y scales";

    Help["setStatPanelVisible"] = "Toggles visibility of the panel with statistical information of the shown histograms";

    {
        AScriptHelpEntry se;
        QString txt = "Add legend box for the currently drawn graphs/histograms. The arguments define the box position:\n"
                      "the 1st and 2nd give x,y of the lower left corner of the box and the 3rd and 4th give x,y of the upper right corner. "
                      "The values should be in the range from 0.0 to 1.0: 0 corresponds to the left(lower) border of the canvas, and 1 is the right(upper) one";
        se.addRecord(4, txt);
        txt += ".\nThe fifth optional argument defines the text shown in the top line of the legend box";
        se.addRecord(5, txt);
        Help["addLegend"] = se;
    }
    Help["setLegendBorder"] = "Define the color, size and style of the border line of the legend box";

    QString genTxt = "Add a box with the provided text to the current draw. if 'showframe' argument is true, a border line is drawn around the text box. "
                     "'alignment' defines the text alignment inside the box: 0 is left, 1 is center and 2 is right";
    Help["addText"]         = genTxt;
    Help["addTextAtScreenXY"] = genTxt +
                              ".\n The last four arguments define the screen position of the bottom-left (x1,y1) and top-right (x2,y2) corner of the box. "
                              "The x and y values should range from 0 (bottom or left border of the canvas) to 1.0 (top or right border of the canvas))";
    Help["addTextAtXY"]   = genTxt +
                            ".\n The last four arguments define the position of the bottom-left (x1,y1) and top-right (x2,y2) corner of the box in the coordinates of the draw.";

    Help["addLine"]  = "Add a line from (x1,y1) to (x2, y2) coordinats. The last three arguments define CERN ROOT's line color, width and style";
    Help["addArrow"] = "Add an arrow from (x1,y1) to (x2, y2) coordinats. The last three arguments define CERN ROOT's line color, width and style";

    Help["addToBasket"] = "Add the currently drawn graphs/histograms to the basket as a new record and name it acording to the provided argument";
    Help["clearBasket"] = "Remove all records from the basket. Cannot be undone!";

    Help["getAxisRanges"] = "Return an array with the range of X and Y axis: [[Xfrom, Xto], [Yfrom, Yto]]";

    Help["saveImage"] = "Save image currently shown on the graph window to an image file.\nTip: use .png suffix";

    Help["show3D"] = "Start the 3D image viewer and loads the PET reconstructed image file generated by CASToR library (see 'pet' script unit)";

    Help["getContent"] = "Return the data content of the first drawn 1D histogram or 1D graph. It is an array with sub-arrays of coordinate and value pairs";

    connect(this, &AGraphWin_SI::requestShow3D, GraphWindow, &GraphWindowClass::show3D, Qt::QueuedConnection);

    // the rest of the methods to look like that:
    connect(this, &AGraphWin_SI::requestAddToBasket,         graphWin, &GraphWindowClass::addCurrentToBasket,  Qt::QueuedConnection);
    connect(this, &AGraphWin_SI::requestClearBasket,         graphWin, &GraphWindowClass::ClearBasket,         Qt::QueuedConnection);
    connect(this, &AGraphWin_SI::requestSetLog,              graphWin, &GraphWindowClass::SetLog,              Qt::QueuedConnection);
    connect(this, &AGraphWin_SI::requestSetStatPanelVisible, graphWin, &GraphWindowClass::SetStatPanelVisible, Qt::QueuedConnection);
    connect(this, &AGraphWin_SI::requestAddLegend,           graphWin, &GraphWindowClass::drawLegend,          Qt::QueuedConnection);
    connect(this, &AGraphWin_SI::requestSetLegendBorder,     graphWin, &GraphWindowClass::SetLegendBorder,     Qt::QueuedConnection);

}

void AGraphWin_SI::setLog(bool Xaxis, bool Yaxis)
{
    QApplication::processEvents();
    emit requestSetLog(Xaxis, Yaxis);
    QApplication::processEvents();
}

void AGraphWin_SI::setStatPanelVisible(bool flag)
{
    QApplication::processEvents();
    emit requestSetStatPanelVisible(flag);
    QApplication::processEvents();
}

void AGraphWin_SI::addLegend(double x1, double y1, double x2, double y2, QString title)
{
    QApplication::processEvents();
    emit requestAddLegend(x1, y1, x2, y2, title);
    QApplication::processEvents();
}

void AGraphWin_SI::setLegendBorder(int color, int style, int size)
{
    QApplication::processEvents();
    emit requestSetLegendBorder(color, style, size);
    QApplication::processEvents();
}

void AGraphWin_SI::addText(QString text, bool showframe, int alignment_0Left1Center2Right)
{
    GraphWindow->ShowTextPanel(text, showframe, alignment_0Left1Center2Right);
}

void AGraphWin_SI::addTextAtScreenXY(QString text, bool Showframe, int Alignment_0Left1Center2Right, double x1, double y1, double x2, double y2)
{
    GraphWindow->ShowTextPanel(text, Showframe, Alignment_0Left1Center2Right, x1, y1, x2, y2, "NDC");
}

void AGraphWin_SI::addTextAtXY(QString text, bool Showframe, int Alignment_0Left1Center2Right, double x1, double y1, double x2, double y2)
{
    GraphWindow->ShowTextPanel(text, Showframe, Alignment_0Left1Center2Right, x1, y1, x2, y2, "BR");
}

void AGraphWin_SI::addLine(double x1, double y1, double x2, double y2, int color, int width, int style)
{
    GraphWindow->AddLine(x1, y1, x2, y2, color, width, style);
}

void AGraphWin_SI::addArrow(double x1, double y1, double x2, double y2, int color, int width, int style)
{
    GraphWindow->AddArrow(x1, y1, x2, y2, color, width, style);
}

void AGraphWin_SI::addToBasket(QString title)
{
    QApplication::processEvents();
    emit requestAddToBasket(title);
    QApplication::processEvents();
}

void AGraphWin_SI::clearBasket()
{
    QApplication::processEvents();
    emit requestClearBasket();
    QApplication::processEvents();
}

void AGraphWin_SI::saveImage(QString fileName)
{
    GraphWindow->SaveGraph(fileName);
}

/*
void AGraphWin_SI::exportTH2AsText(QString fileName)
{
    GraphWindow->ExportTH2AsText(fileName);
}
*/

#include <QTimer>
void AGraphWin_SI::show3D(QString castorFileName)
{
    emit requestShow3D(castorFileName);
}

/*
QVariant AGraphWin_SI::GetProjection()
{
    QVector<double> vec = GraphWindow->Get2DArray();
    QJsonArray arr;
    for (auto v : vec) arr << v;
    QJsonValue jv = arr;
    QVariant res = jv.toVariant();
    return res;
}

void AGraphWin_SI::UseProjectionTool(QString option)
{
    QString res = GraphWindow->UseProjectionTool(option);
    if (!res.isEmpty()) abort(res);
}

void AGraphWin_SI::ConfigureProjectionTool(double x0, double y0, double dx, double dy, double angle)
{
    GraphWindow->ConfigureProjectionTool(x0, y0, dx, dy, angle);
}
*/

QVariantList AGraphWin_SI::getAxisRanges()
{
    QVariantList vl;

    bool ok1, ok2;
    double min = 0;
    double max = 0;

    min = GraphWindow->getMinX(&ok1);
    max = GraphWindow->getMaxX(&ok2);
    if (ok1 && ok2) vl.push_back( QVariantList{min, max} );

    min = GraphWindow->getMinY(&ok1);
    max = GraphWindow->getMaxY(&ok2);
    if (ok1 && ok2) vl.push_back( QVariantList{min, max} );

    min = GraphWindow->getMinZ(&ok1);
    max = GraphWindow->getMaxZ(&ok2);
    if (ok1 && ok2) vl.push_back( QVariantList{min, max} );

    return vl;
}

#include "TObject.h"
#include "TGraphErrors.h"
#include "TH1.h"
QVariantList AGraphWin_SI::getContent()
{
    QVariantList vl;

    TObject * obj = GraphWindow->GetMainPlottedObject();
    if (obj)
    {
        QString ClName = obj->ClassName();
        if (ClName == "TH1D")
        {
            TH1 * h = dynamic_cast<TH1*>(obj);
            if (h)
            {
                int bins = h->GetNbinsX();
                for (int i=1; i<=bins; i++)
                {
                    QVariantList el;
                    el << h->GetBinLowEdge(i) << h->GetBinContent(i);
                    vl.push_back(el);
                }
            }
        }
        else if (ClName == "TGraph" || ClName == "TGraphErrors")
        {
            TGraph * g = dynamic_cast<TGraph*>(obj);
            if (g)
            {
                int bins = g->GetN();
                for (int i=0; i<bins; i++)
                {
                    QVariantList el;
                    double x, y;
                    g->GetPoint(i, x, y);
                    el << x << y;
                    vl.push_back(el);
                }
            }
        }
    }

    return vl;
}
