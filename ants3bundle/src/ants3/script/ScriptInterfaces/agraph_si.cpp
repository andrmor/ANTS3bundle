#include "agraph_si.h"
#include "ascriptobjstore.h"
#include "arootobjcollection.h"
#include "arootgraphrecord.h"

#include <QDebug>

#include "TObject.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraph2D.h"
#include "TFile.h"
#include "TKey.h"

AGraph_SI::AGraph_SI()
    : Graphs(AScriptObjStore::getInstance().Graphs)
{
    Description = "CERN ROOT graphs";

    Help["new1D"] = "Creates a new 1D graph (Root TGraph object)";
    Help["new1DErr"] = "Creates a new 1D graph with errors (Root TGraphErrors object)";
    Help["new2D"] = "Creates a new 2D graph (Root TGraph2D object)";

    //Help["configureAbortIfAlreadyExists"] = "If set to true, an attempt to create a graph with already existent name will casuse abort. Default is false";

    Help["setMarkerProperties"] = "Default marker properties are 1, 20, 1";
    Help["setLineProperties"] = "Default line properties are 1, 1, 2";
    Help["draw"] = "Draws the graph. Refer to https://root.cern.ch/doc/master/classTGraphPainter.html for draw options";

    Help["addPoint"] = "Add a point to 1D or 1DErr graph: (X, Y) or (X, Y, errY) or (X, Y, errX, errY)";
    Help["addPoint2D"] = "Add a point to 2D graph: (X, Y, Z)";

    Help["addPoints"] = "Add array(s) of point using the following argument options:\n"
                        "1. (xArray, yArray), (xArray, yArray, yErrorArray) or (xArray, yArray, xErrorArray, yErrorArray);\n"
                        "2. A single array of Y coordinates. In this case X scale will be 0, 1, 2, etc;\n"
                        "3. An arrays of [x,y] arrays of points;";

    Help["saveRoot"] = "Save graph as a Root object";

    Help["remove"] = "Removes the graph";
    Help["removeAllGraph"] = "Removes all graphs";
}

//AGraph_SI::AGraph_SI(const AGraph_SI &other) :
//    AScriptInterface(other),
//    TmpHub(other.TmpHub) {}

void AGraph_SI::new1D(QString graphName)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw graphs!");
        return;
    }

    TGraph* gr = new TGraph();
    ARootGraphRecord* rec = new ARootGraphRecord(gr, graphName, "TGraph");
    bool bOK = Graphs.append(graphName, rec, AbortIfExists);
    if (!bOK)
    {
        delete gr;
        delete rec;
        abort("Graph "+graphName+" already exists!");
    }
    else
    {
        gr->SetFillColor(0);
        gr->SetFillStyle(0);
    }
}

void AGraph_SI::new1DErr(QString graphName)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw graphs!");
        return;
    }

    TGraphErrors * gr = new TGraphErrors();
    ARootGraphRecord* rec = new ARootGraphRecord(gr, graphName, "TGraphErrors");
    bool bOK = Graphs.append(graphName, rec, AbortIfExists);
    if (!bOK)
    {
        delete gr;
        delete rec;
        abort("Graph "+graphName+" already exists!");
    }
    else
    {
        gr->SetFillColor(0);
        gr->SetFillStyle(0);
    }
}

void AGraph_SI::new2D(QString graphName)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw graphs!");
        return;
    }

    TGraph2D * gr = new TGraph2D();
    ARootGraphRecord* rec = new ARootGraphRecord(gr, graphName, "TGraph2D");
    bool bOK = Graphs.append(graphName, rec, AbortIfExists);
    if (!bOK)
    {
        delete gr;
        delete rec;
        abort("Graph "+graphName+" already exists!");
    }
    else
    {
        gr->SetFillColor(0);
        gr->SetFillStyle(0);
    }
}

void AGraph_SI::setMarkerProperties(QString graphName, int color, int style, double size)
{
    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else    r->setMarkerProperties(color, style, size);
}

void AGraph_SI::setLineProperties(QString graphName, int color, int style, int width)
{
    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else    r->setLineProperties(color, style, width);
}

void AGraph_SI::setTitle(QString graphName, QString graphTitle)
{
    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else    r->setTitle(graphTitle);
}

void AGraph_SI::setAxisTitles(QString graphName, QString x_Title, QString y_Title)
{
    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else    r->setAxisTitles(x_Title, y_Title);
}

void AGraph_SI::addPoint(QString graphName, double x, double y)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else    r->addPoint(x, y);
}

void AGraph_SI::addPoint(QString graphName, double x, double y, double errorY)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else    r->addPoint(x, y, 0, errorY);
}

void AGraph_SI::addPoint(QString graphName, double x, double y, double errorX, double errorY)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else    r->addPoint(x, y, errorX, errorY);
}

void AGraph_SI::addPoints(QString GraphName, QVariantList vx, QVariantList vy)
{
    if (vx.isEmpty() || vx.size() != vy.size())
    {
        abort("Empty array or mismatch in array sizes in AddPoints for graph " + GraphName);
        return;
    }

    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(GraphName));
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

void AGraph_SI::addPoints(QString GraphName, QVariantList vx, QVariantList vy, QVariantList vEy)
{
    if (vx.isEmpty() || vx.size() != vy.size() || vx.size() != vEy.size())
    {
        abort("Empty array or mismatch in array sizes in AddPoints for graph " + GraphName);
        return;
    }

    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(GraphName));
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

void AGraph_SI::addPoints(QString GraphName, QVariantList vx, QVariantList vy, QVariantList vEx, QVariantList vEy)
{
    if (vx.isEmpty() || vx.size() != vy.size() || vx.size() != vEx.size() || vx.size() != vEy.size())
    {
        abort("Empty array or mismatch in array sizes in AddPoints for graph " + GraphName);
        return;
    }

    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(GraphName));
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

void AGraph_SI::addPoints(QString GraphName, QVariantList v)
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
        addPoints(GraphName, vl, v);
        return;
    }

    bool bError = false;
    bool bValidX, bValidY, bValidErrX, bValidErrY;
    QVector<double> xArr(v.size()), yArr(v.size()), xErrArr(v.size()), yErrArr(v.size());

    const QVariantList vFirst = v.at(0).toList();
    const int length = vFirst.size();
    if (length < 2 || length > 4)
    {
        abort("Invalid array in addPoints() for graph: each entry should be X,Y  [or X,Y,Xerr or X,Y,Xerr,Yerr for TGraphError]" + GraphName);
        return;
    }

    for (int i=0; i<v.size(); i++)
    {
        const QVariantList vxy = v.at(i).toList();
        if (vxy.size() < length)
        {
            bError = true;
            break;
        }
        double x = vxy.at(0).toDouble(&bValidX);
        double y = vxy.at(1).toDouble(&bValidY);
        double xErr = 0; bValidErrX = true;
        double yErr = 0; bValidErrY = true;
        if (length == 3) yErr = vxy.at(2).toDouble(&bValidErrY);
        if (length == 4)
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

    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
        r->AddPoints(xArr, yArr, xErrArr, yErrArr);
}

void AGraph_SI::addPoint2D(QString GraphName, double x, double y, double z)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
        r->AddPoint2D(x, y, z);
}

void AGraph_SI::setYRange(QString graphName, double min, double max)
{
    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else    r->setYRange(min, max);
}

void AGraph_SI::setMinimum(QString graphName, double min)
{
    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else    r->setMinimum(min);
}

void AGraph_SI::setMaximum(QString graphName, double max)
{
    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else    r->setMaximum(max);
}

void AGraph_SI::setXRange(QString graphName, double min, double max)
{
    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else    r->setXRange(min, max);
}

void AGraph_SI::setXDivisions(QString graphName, int numDiv)
{
    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else    r->setXDivisions(numDiv);
}

void AGraph_SI::setYDivisions(QString graphName, int numDiv)
{
    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else    r->setYDivisions(numDiv);
}

void AGraph_SI::sort(QString graphName)
{
    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else    r->sort();
}

void AGraph_SI::draw(QString graphName, QString options)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw graphs!");
        return;
    }

    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else
    {
        TObject * copy = r->GetObject()->Clone(r->GetObject()->GetName());
        emit requestDraw(copy, options, true);
        r->LastDrawOption = options;
    }
}

void AGraph_SI::load(QString NewGraphName, QString FileName)
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
        bool bOK = Graphs.append(NewGraphName, rec, AbortIfExists);
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

void AGraph_SI::save(QString graphName, QString fileName)
{
    if (!bGuiThread)
    {
        abort("Threads cannot save graphs!");
        return;
    }

    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else    r->exportRoot(fileName);
}

QVariantList AGraph_SI::getPoints(QString GraphName)
{
    QVariantList res;

    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(GraphName));
    if (!r)
        abort("Graph "+GraphName+" not found!");
    else
    {
        const std::vector<std::pair<double, double>> vec = r->GetPoints();
        for (const auto & pair : vec)
        {
            QVariantList el;
            el << pair.first << pair.second;
            res.push_back(el); // creates nested array!
        }
    }

    return res;
}

bool AGraph_SI::remove(QString graphName)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw graphs!");
        return false;
    }

    return Graphs.remove(graphName);
}

void AGraph_SI::removeAll()
{
    if (!bGuiThread) abort("Threads cannot create/delete/draw graphs!");
    else             Graphs.clear();
}
