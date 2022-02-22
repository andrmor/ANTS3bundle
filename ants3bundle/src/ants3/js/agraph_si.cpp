#include "agraph_si.h"
#include "ascriptobjstore.h"
#include "arootobjcollection.h"
#include "arootgraphrecord.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>

#include "TObject.h"
#include "TF2.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraph2D.h"
#include "TF1.h"
#include "TFile.h"
#include "TKey.h"

AGraph_SI::AGraph_SI()
    : TmpHub(AScriptObjStore::getInstance())
{
    Description = "CERN ROOT graphs - TGraph";

    Help["NewGraph"] = "Creates a new graph (Root TGraph object)";
    Help["SetMarkerProperties"] = "Default marker properties are 1, 20, 1";
    Help["SetLineProperties"] = "Default line properties are 1, 1, 2";
    Help["Draw"] = "Draws the graph (use \"APL\" options if in doubt)";

    Help["AddPoints"] = "Can add arrays of point using two options:\n"
                        "1. Provide two arrays - one for X and one for Y coordinates of the points;\n"
                        "2. Provide an array of [x,y] arrays of points;\n"
                        "3. Provide an array of Y coordinates. In this case X scale will be 0, 1, 2, etc.";
}

//AGraph_SI::AGraph_SI(const AGraph_SI &other) :
//    AScriptInterface(other),
//    TmpHub(other.TmpHub) {}

void AGraph_SI::create1D(QString GraphName)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw graphs!");
        return;
    }

    TGraph* gr = new TGraph();
    ARootGraphRecord* rec = new ARootGraphRecord(gr, GraphName, "TGraph");
    bool bOK = TmpHub.Graphs.append(GraphName, rec, bAbortIfExists);
    if (!bOK)
    {
        delete gr;
        delete rec;
        abort("Graph "+GraphName+" already exists!");
    }
    else
    {
        gr->SetFillColor(0);
        gr->SetFillStyle(0);
    }
}

void AGraph_SI::create1DErr(QString GraphName)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw graphs!");
        return;
    }

    TGraphErrors * gr = new TGraphErrors();
    ARootGraphRecord* rec = new ARootGraphRecord(gr, GraphName, "TGraphErrors");
    bool bOK = TmpHub.Graphs.append(GraphName, rec, bAbortIfExists);
    if (!bOK)
    {
        delete gr;
        delete rec;
        abort("Graph "+GraphName+" already exists!");
    }
    else
    {
        gr->SetFillColor(0);
        gr->SetFillStyle(0);
    }
}

void AGraph_SI::create2D(QString GraphName)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw graphs!");
        return;
    }

    TGraph2D * gr = new TGraph2D();
    ARootGraphRecord* rec = new ARootGraphRecord(gr, GraphName, "TGraph2D");
    bool bOK = TmpHub.Graphs.append(GraphName, rec, bAbortIfExists);
    if (!bOK)
    {
        delete gr;
        delete rec;
        abort("Graph "+GraphName+" already exists!");
    }
    else
    {
        gr->SetFillColor(0);
        gr->SetFillStyle(0);
    }
}

void AGraph_SI::SetMarkerProperties(QString GraphName, int MarkerColor, int MarkerStyle, double MarkerSize)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
        r->SetMarkerProperties(MarkerColor, MarkerStyle, MarkerSize);
}

void AGraph_SI::SetLineProperties(QString GraphName, int LineColor, int LineStyle, int LineWidth)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
        r->SetLineProperties(LineColor, LineStyle, LineWidth);
}

void AGraph_SI::SetTitles(QString GraphName, QString X_Title, QString Y_Title, QString GraphTitle)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
        r->SetTitles(X_Title, Y_Title, GraphTitle);
}

void AGraph_SI::AddPoint(QString GraphName, double x, double y)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
        r->AddPoint(x, y);
}

void AGraph_SI::AddPoint(QString GraphName, double x, double y, double errorY)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
        r->AddPoint(x, y, 0, errorY);
}

void AGraph_SI::AddPoint(QString GraphName, double x, double y, double errorX, double errorY)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
        r->AddPoint(x, y, errorX, errorY);
}

void AGraph_SI::AddPoints(QString GraphName, QVariantList vx, QVariantList vy)
{
    if (vx.isEmpty() || vx.size() != vy.size())
    {
        abort("Empty array or mismatch in array sizes in AddPoints for graph " + GraphName);
        return;
    }

    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
    {
        QVector<double> xArr(vx.size());
        QVector<double> yArr(vx.size());
        bool bValidX, bValidY;

        for (int i=0; i<vx.size(); i++)
        {
            double x = vx.at(i).toDouble(&bValidX);
            double y = vy.at(i).toDouble(&bValidY);
            if (bValidX && bValidY)
            {
                //  qDebug() << i << x << y;
                xArr[i] = x;
                yArr[i] = y;
            }
            else
            {
                abort("Not numeric value found in AddPoints() for " + GraphName);
                return;
            }
        }
        r->AddPoints(xArr, yArr);
    }
}

void AGraph_SI::AddPoints(QString GraphName, QVariantList vx, QVariantList vy, QVariantList vEy)
{
    if (vx.isEmpty() || vx.size() != vy.size() || vx.size() != vEy.size())
    {
        abort("Empty array or mismatch in array sizes in AddPoints for graph " + GraphName);
        return;
    }

    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
    {
        QVector<double> xArr(vx.size());
        QVector<double> yArr(vx.size());
        QVector<double> xErrArr(vx.size());
        QVector<double> yErrArr(vx.size());

        bool bValidX, bValidY, bValidYerr;

        for (int i=0; i<vx.size(); i++)
        {
            double x    = vx.at(i).toDouble(&bValidX);
            double y    = vy.at(i).toDouble(&bValidY);
            double yerr = vEy.at(i).toDouble(&bValidYerr);
            if (bValidX && bValidY && bValidYerr)
            {
                xArr[i] = x;
                yArr[i] = y;
                xErrArr[i] = 0;
                yErrArr[i] = yerr;
            }
            else
            {
                abort("Not numeric value found in AddPoints() for " + GraphName);
                return;
            }
        }
        r->AddPoints(xArr, yArr, xErrArr, yErrArr);
    }
}

void AGraph_SI::AddPoints(QString GraphName, QVariantList vx, QVariantList vy, QVariantList vEx, QVariantList vEy)
{
    if (vx.isEmpty() || vx.size() != vy.size() || vx.size() != vEx.size() || vx.size() != vEy.size())
    {
        abort("Empty array or mismatch in array sizes in AddPoints for graph " + GraphName);
        return;
    }

    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
    {
        QVector<double> xArr(vx.size());
        QVector<double> yArr(vx.size());
        QVector<double> xErrArr(vx.size());
        QVector<double> yErrArr(vx.size());

        bool bValidX, bValidY, bValidXerr, bValidYerr;

        for (int i=0; i<vx.size(); i++)
        {
            double x    = vx.at(i).toDouble(&bValidX);
            double y    = vy.at(i).toDouble(&bValidY);
            double xerr = vEx.at(i).toDouble(&bValidXerr);
            double yerr = vEy.at(i).toDouble(&bValidYerr);
            if (bValidX && bValidY && bValidXerr && bValidYerr)
            {
                xArr[i] = x;
                yArr[i] = y;
                xErrArr[i] = xerr;
                yErrArr[i] = yerr;
            }
            else
            {
                abort("Not numeric value found in AddPoints() for " + GraphName);
                return;
            }
        }
        r->AddPoints(xArr, yArr, xErrArr, yErrArr);
    }
}

void AGraph_SI::AddPoints(QString GraphName, QVariantList v)
{
    if (v.isEmpty())
    {
        abort("Empty array in AddPoints for graph " + GraphName);
        return;
    }

    bool bOK = false;
    v.at(0).toDouble(&bOK);
    if (bOK)
    {
        QVariantList vl;
        for (int i=0; i<v.size(); i++) vl << i;
        AddPoints(GraphName, vl, v);
        return;
    }

    bool bError = false;
    bool bValidX, bValidY, bValidErrX, bValidErrY;
    QVector<double> xArr(v.size()), yArr(v.size()), xErrArr(v.size()), yErrArr(v.size());

    const QVariantList vFirst = v.at(0).toList();
    int Length = vFirst.size();
    if (Length < 2 || Length > 4)
    {
        abort("Invalid array in AddPoints() for graph: each entry should be X,Y  [or X,Y,Xerr or X,Y,Xerr,Yerr for TGraphError]" + GraphName);
        return;
    }

    for (int i=0; i<v.size(); i++)
    {
        const QVariantList vxy = v.at(i).toList();
        if (vxy.size() < Length)
        {
            bError = true;
            break;
        }
        double x = vxy.at(0).toDouble(&bValidX);
        double y = vxy.at(1).toDouble(&bValidY);
        double xErr = 0; bValidErrX = true;
        double yErr = 0; bValidErrY = true;
        if (Length == 3) yErr = vxy.at(2).toDouble(&bValidErrY);
        if (Length == 4)
        {
            xErr = vxy.at(2).toDouble(&bValidErrX);
            yErr = vxy.at(3).toDouble(&bValidErrY);
        }

        if (bValidX && bValidY && bValidErrX && bValidErrY)
        {
            xArr[i] = x;
            yArr[i] = y;
            xErrArr[i] = xErr;
            yErrArr[i] = yErr;
        }
        else
        {
            bError = true;
            break;
        }
    }
    if (bError)
    {
        abort("Invalid array in AddPoints() for graph " + GraphName);
        return;
    }

    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
        r->AddPoints(xArr, yArr, xErrArr, yErrArr);
}

void AGraph_SI::AddPoint2D(QString GraphName, double x, double y, double z)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
        r->AddPoint2D(x, y, z);
}

void AGraph_SI::SetYRange(const QString &GraphName, double min, double max)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
        r->SetYRange(min, max);
}

void AGraph_SI::SetMinimum(const QString &GraphName, double min)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
        r->SetMinimum(min);
}

void AGraph_SI::SetMaximum(const QString &GraphName, double max)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
        r->SetMaximum(max);
}

void AGraph_SI::SetXRange(const QString &GraphName, double min, double max)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
        r->SetXRange(min, max);
}

void AGraph_SI::SetXDivisions(const QString &GraphName, int numDiv)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
        r->SetXDivisions(numDiv);
}

void AGraph_SI::SetYDivisions(const QString &GraphName, int numDiv)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
        r->SetYDivisions(numDiv);
}

void AGraph_SI::Sort(const QString &GraphName)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
        r->Sort();
}

void AGraph_SI::Draw(QString GraphName, QString options)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw graphs!");
        return;
    }

    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
    {
        TObject * copy = r->GetObject()->Clone(r->GetObject()->GetName());
        emit RequestDraw(copy, options, true);
        r->LastDrawOption = options;
    }
}

void AGraph_SI::LoadTGraph(const QString &NewGraphName, const QString &FileName)
{
    if (!bGuiThread)
    {
        abort("Threads cannot load graphs!");
        return;
    }

    TFile* f = new TFile(FileName.toLocal8Bit().data());
    if (!f)
    {
        abort("Cannot open file " + FileName);
        return;
    }

    const int numKeys = f->GetListOfKeys()->GetEntries();

    TGraph* g = 0;
    for (int ikey = 0; ikey < numKeys; ikey++)
    {
        TKey *key = (TKey*)f->GetListOfKeys()->At(ikey);
        qDebug() << "Key->  name:" << key->GetName() << " class:" << key->GetClassName() <<" title:"<< key->GetTitle();

        const QString Type = key->GetClassName();
        if (Type != "TGraph") continue;
        g = dynamic_cast<TGraph*>(key->ReadObj());
        if (g) break;
    }
    if (!g) abort(FileName + " does not contain TGraphs");
    else
    {
        ARootGraphRecord* rec = new ARootGraphRecord(g, NewGraphName, "TGraph");
        bool bOK = TmpHub.Graphs.append(NewGraphName, rec, bAbortIfExists);
        if (!bOK)
        {
            delete g;
            delete rec;
            abort("Graph "+NewGraphName+" already exists!");
        }
        else
        {
            qDebug() << "Draw opt:"<<g->GetDrawOption() << g->GetOption();
            //g->Dump();
            g->SetFillColor(0);
            g->SetFillStyle(0);
        }
    }

    f->Close();
    delete f;
}

void AGraph_SI::Save(const QString &GraphName, const QString &FileName)
{
    if (!bGuiThread)
    {
        abort("Threads cannot save graphs!");
        return;
    }

    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
        r->Save(FileName);
}

const QVariant AGraph_SI::GetPoints(const QString &GraphName)
{
    QVariantList res;

    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(TmpHub.Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
    {
        const QVector<QPair<double, double> > vec = r->GetPoints();
        for (const QPair<double, double>& pair : vec)
        {
            QVariantList el;
            el << pair.first << pair.second;
            res.push_back(el); // creates nested array!
        }
    }

    return res;
}

bool AGraph_SI::Delete(QString GraphName)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw graphs!");
        return false;
    }

    return TmpHub.Graphs.remove(GraphName);
}

void AGraph_SI::DeleteAllGraph()
{
    if (!bGuiThread)
        abort("Threads cannot create/delete/draw graphs!");
    else
        TmpHub.Graphs.clear();
}
