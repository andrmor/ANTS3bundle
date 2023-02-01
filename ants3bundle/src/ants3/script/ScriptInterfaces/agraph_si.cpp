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
    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else    r->addPoint(x, y);
}

void AGraph_SI::addPoint(QString graphName, double x, double y, double errorX, double errorY)
{
    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else    r->addPoint(x, y, errorX, errorY);
}

void AGraph_SI::addPoint(QString graphName, double x, double y, double z)
{
    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r) abort("Graph " + graphName + " not found!");
    else    r->AddPoint2D(x, y, z);
}

void AGraph_SI::addPoints(QString graphName, QVariantList xArray, QVariantList yArray)
{
    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r)
    {
        abort("Graph " + graphName + " not found!");
        return;
    }

    const int num = xArray.size();
    if (num != yArray.size())
    {
        abort("addPoints: mismatch in array sizes for graph " + graphName);
        return;
    }
    if (num == 0) return;

    std::vector<double> xArr(num), yArr(num);
    bool ok1, ok2;
    for (int i = 0; i < num; i++)
    {
        xArr[i] = xArray[i].toDouble(&ok1);
        yArr[i] = yArray[i].toDouble(&ok2);
        if (!ok1 || !ok2)
        {
            if (!ok1) abort("addPoints: bad format of xArray for graph " + graphName);
            else      abort("addPoints: bad format of yArray for graph " + graphName);
            return;
        }
    }
    r->addPoints(xArr, yArr);
}

void AGraph_SI::addPoints(QString graphName, QVariantList xArray, QVariantList yArray, QVariantList xErrArray, QVariantList yErrArray)
{
    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r)
    {
        abort("Graph " + graphName + " not found!");
        return;
    }

    const int num = xArray.size();
    if (num != yArray.size() || num != xErrArray.size() || num != yErrArray.size())
    {
        abort("addPoints: mismatch in array sizes for graph " + graphName);
        return;
    }
    if (num == 0) return;

    std::vector<double> xArr(num), yArr(num), xErArr(num), yErArr(num);
    bool ok1, ok2, ok3, ok4;

    for (int i = 0; i < num; i++)
    {
        xArr[i] = xArray[i].toDouble(&ok1);
        yArr[i] = yArray[i].toDouble(&ok2);
        xErArr[i] = xErrArray[i].toDouble(&ok3);
        yErArr[i] = yErrArray[i].toDouble(&ok4);
        if (!ok1 || !ok2 || !ok3 || !ok4)
        {
            if      (!ok1) abort("addPoints: bad format of xArray for graph " + graphName);
            else if (!ok2) abort("addPoints: bad format of yArray for graph " + graphName);
            else if (!ok3) abort("addPoints: bad format of xErrArray for graph " + graphName);
            else           abort("addPoints: bad format of yErrArray for graph " + graphName);
            return;
        }
    }
    ARootObjBase::EStatus res = r->addPoints(xArr, yArr, xErArr, yErArr);
    if (res != ARootObjBase::OK)
        abort("addPoints with four array arguments is applicable only to graphs with errors");
}

void AGraph_SI::addPoints(QString graphName, QVariantList xArray, QVariantList yArray, QVariantList zArray)
{
    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r)
    {
        abort("Graph " + graphName + " not found!");
        return;
    }

    const int num = xArray.size();
    if (num != yArray.size() || num != zArray.size())
    {
        abort("addPoints: mismatch in array sizes for graph " + graphName);
        return;
    }
    if (num == 0) return;

    std::vector<double> xArr(num), yArr(num), zArr(num);
    bool ok1, ok2, ok3;

    for (int i = 0; i < num; i++)
    {
        xArr[i] = xArray[i].toDouble(&ok1);
        yArr[i] = yArray[i].toDouble(&ok2);
        zArr[i] = zArray[i].toDouble(&ok3);
        if (!ok1 || !ok2 || !ok3)
        {
            if      (!ok1) abort("addPoints: bad format of xArray for graph " + graphName);
            else if (!ok2) abort("addPoints: bad format of yArray for graph " + graphName);
            else           abort("addPoints: bad format of zArray for graph " + graphName);
            return;
        }
    }
    ARootObjBase::EStatus res = r->addPoints(xArr, yArr, zArr);
    if (res != ARootObjBase::OK)
        abort("addPoints with three array arguments is applicable only to 2D graphs");
}

void AGraph_SI::addPoints(QString graphName, QVariantList array)
{
    if (array.isEmpty()) return;

    const int size = array.size();

    bool ok;
    array.front().toDouble(&ok);
    if (ok)
    {
        QVariantList vl;
        for (int i = 0; i < size; i++) vl << i;
        addPoints(graphName, vl, array);
        return;
    }

    const int numDim = array.front().toList().size();
    if (numDim < 2 || numDim > 4)
    {
        abort("addPoints with one array argument: each element should be Y or [X,Y] or [X,Y,Xerr,Yerr] or [x,y,z]" + graphName);
        return;
    }

    std::vector<double> xArr(size), yArr(size), zArr, xErrArr, yErrArr;
    if (numDim == 3) zArr.resize(size);
    if (numDim == 4)
    {
        xErrArr.resize(size);
        yErrArr.resize(size);
    }

    bool ok1, ok2, ok3 = true, ok4 = true;
    bool bError = false;
    for (int i = 0; i < size; i++)
    {
        const QVariantList vl = array[i].toList();
        if (vl.size() != numDim)
        {
            bError = true;
            break;
        }
        xArr[i] = vl[0].toDouble(&ok1);
        yArr[i] = vl[1].toDouble(&ok2);
        if (!ok1 || !ok2)
        {
            bError = true;
            break;
        }
        if (numDim == 3)
        {
            zArr[i] = vl[2].toDouble(&ok3);
            if (!ok3)
            {
                bError = true;
                break;
            }
        }
        if (numDim == 4)
        {
            xErrArr[i] = vl[2].toDouble(&ok3);
            yErrArr[i] = vl[3].toDouble(&ok4);
            {
                if (!ok3 || !ok4)
                {
                    bError = true;
                    break;
                }
            }
        }
    }
    if (bError)
    {
        abort("AddPoints with one array argument: invalid format for graph " + graphName);
        return;
    }

    ARootGraphRecord* r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (r)
    {
        ARootObjBase::EStatus res;
        switch (numDim)
        {
        case 2 :
            res = r->addPoints(xArr, yArr);
            if (res != ARootObjBase::OK) abort("addPoints with one array argument [x,y] is applicable only to 1D graphs");
            break;
        case 3 :
            res = r->addPoints(xArr, yArr, yArr);
            if (res != ARootObjBase::OK) abort("addPoints with one array argument [x,y,z] is applicable only to 2D graphs");
            break;
        case 4 :
            res = r->addPoints(xArr, yArr, xErrArr, yErrArr);
            if (res != ARootObjBase::OK) abort("addPoints with one array argument [x,y,xErr,yErr] is applicable only to 1D error graphs");
            break;
        }
    }
    else abort("Graph " + graphName + " not found!");
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
