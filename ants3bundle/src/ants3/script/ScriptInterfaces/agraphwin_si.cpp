#include "agraphwin_si.h"
#include "graphwindowclass.h"

#include <QDebug>

AGraphWin_SI::AGraphWin_SI(GraphWindowClass * graphWin) :
    AWindowInterfaceBase(graphWin), GraphWindow(graphWin)
{
    Description = "Access to the Graph window of GUI";

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

    Help["addToBasket"] = "Add the currently drawn graphs/histograms as an entry in the basket with the provided name.";
    Help["clearBasket"] = "Completely clears the basket. Cannot be undone!";

    Help["saveImage"] = "Save image currently shown on the graph window to an image file.\nTip: use .png extension";

    connect(this, &AGraphWin_SI::requestShow3D, GraphWindow, &GraphWindowClass::show3D, Qt::QueuedConnection);
}

void AGraphWin_SI::setLog(bool Xaxis, bool Yaxis)
{
    GraphWindow->SetLog(Xaxis, Yaxis);
}

void AGraphWin_SI::setStatPanelVisible(bool flag)
{
    GraphWindow->SetStatPanelVisible(flag);
}

void AGraphWin_SI::addLegend(double x1, double y1, double x2, double y2, QString title)
{
    GraphWindow->drawLegend(x1, y1, x2, y2, title);
}

void AGraphWin_SI::setLegendBorder(int color, int style, int size)
{
    GraphWindow->SetLegendBorder(color, style, size);
}

void AGraphWin_SI::addText(QString text, bool Showframe, int Alignment_0Left1Center2Right)
{
    GraphWindow->ShowTextPanel(text, Showframe, Alignment_0Left1Center2Right);
}

void AGraphWin_SI::addTextScreenXY(QString text, bool Showframe, int Alignment_0Left1Center2Right, double x1, double y1, double x2, double y2)
{
    GraphWindow->ShowTextPanel(text, Showframe, Alignment_0Left1Center2Right, x1, y1, x2, y2, "NDC");
}

void AGraphWin_SI::addTextTrueXY(QString text, bool Showframe, int Alignment_0Left1Center2Right, double x1, double y1, double x2, double y2)
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

void AGraphWin_SI::addToBasket(QString Title)
{
    GraphWindow->addCurrentToBasket(Title);
}

void AGraphWin_SI::clearBasket()
{
    GraphWindow->ClearBasket();
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

QVariantList AGraphWin_SI::getAxes()
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
