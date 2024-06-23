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

    Help["new1D"] = "Create a new 1D graph (TGraph object of CERN Root)";
    Help["new1DErr"] = "Create a new 1D graph with errors (TGraphErrors object of CERN Root)";
    Help["new2D"] = "Create a new 2D graph (TGraph2D object of CERN Root)";

    Help["addPoint"] = {{3, "Add a point to 1D graph by providing X and Y coordinates"},
                        {4, "Add a point to 2D graph by providing X, Y and Z coordinates"},
                        {5, "Add a point to 1D graph with error bars by providing X, Y, errorX and errorY"}};

    Help["addPoints"] = {{2, "Add array of points to the selected graph. The elements of the array should be also arrays and can be the following:\n"
                             "[X,Y] for 1D graph;\n[X,Y,Xerr,Yerr] for 1D graph with errors;\n[X,Y,Z] for 2D graph"},
                         {4, "Add array of points to the selected graph. The provided 'array' argument should be a 2d array,\n"
                             "and indexX and indexY should identify the column indexes to be used for X and Y coordinates"},
                         {3, "Add array of points to the 1D graph;\n xArray and yArray are 1D arrays of X and Y coordinates"},
                         {5, "Add array of points to the 1DError graph;\n xArray and yArray are 1D arrays of X and Y coordinates\n"
                             "and xErrArray and yErrArray are the corresponding errors"}};

    Help["draw"] = {{1, "Draw the graph using the default 'APL' options string.\n"
                        "Refer to https://root.cern.ch/doc/master/classTGraphPainter.html for the list of the available options"},
                    {2, "Draw the graph using the provided options string;\n"
                        "Refer to https://root.cern.ch/doc/master/classTGraphPainter.html for the list of the available options"}};

    Help["setMarkerProperties"] = "Configure color, style, and size of the markers for the selected graph. Default values are 1, 20, 1.0";
    Help["setLineProperties"] = "Configure color, style and width of the line for the selected graph. Default line properties are 1, 1, 2";

    Help["setTitle"] = "Sets title of the slected graph";
    Help["setAxisTitles"] = "Sets X and Y axis titles for the selected graph";

    Help["setXRange"] = "Set shown range for X axis";
    Help["setYRange"] = "Set shown range for Y axis";
    Help["setMinimum"] = "Set shown minimum for Y axis";
    Help["setMaximum"] = "Set shown maximum for X axis";

    QString divHelp = "Argument is: ndiv = N1 + 100*N2 + 10000*N3, where\n"
                      "N1 = number of 1st divisions, N2 = number of 2nd divisions and N3 = number of 3rd divisions.\n"
                      "e.g.: ndiv = 0 --> no tick marks; ndiv = 2 --> 2 divisions, one tick mark in the middle of the axis.";
    Help["setXDivisions"] = "Configures ticks for X axis\n" + divHelp;
    Help["setYDivisions"] = "Configures ticks for Y axis\n" + divHelp;

    Help["sort"] = "Sorts points of 1D graph to have continuously increasing X";

    Help["getData"] = "Get an array with the data points of the graph";

    Help["save"] = "Save graph as a Root object file (.root or .c)";
    Help["load"] = "Load graph from a file containing Root objects. If there are several graphs in the file, provide the graph name as third argument";

    Help["remove"] = "Remove this graph";
    Help["removeAll"] = "Remove all graphs";

    Help["configureAbortIfAlreadyExists"] = "If set to true, an attempt to create a graph with already existent name will cause abort. Default is false";
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
    else
    {
        if      (r->getType() == "TGraph2D")     r->addPoint2D(x, y, z);
        else if (r->getType() == "TGraphErrors") r->addPoint(x, y, 0, z);
        else abort(graphName + ": Cannot use addPoint with three doubles for that type of graphs!");
    }
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

/*
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
*/

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
            res = r->addPoints(xArr, yArr, zArr);
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

void AGraph_SI::addPoints(QString graphName, QVariantList array, int indexX, int indexY)
{
    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (!r)
    {
        abort("Graph " + graphName + " not found!");
        return;
    }

    const int num = array.size();
    if (num == 0) return;

    const int minSize = std::max(indexX, indexY) + 1;
    bool bAutoX = (indexX == -1);

    std::vector<double> xArr(num), yArr(num);
    bool ok1, ok2;
    for (int i = 0; i < num; i++)
    {
        const QVariantList vl = array[i].toList();
        if (vl.size() < minSize)
        {
            abort("Graph " + graphName + " cannot be filled using the provided array and column indexes for X and Y");
            return;
        }
        xArr[i] = ( bAutoX ? i : vl[indexX].toDouble(&ok1) );
        yArr[i] = vl[indexY].toDouble(&ok2);
        if (!ok1 || !ok2)
        {
            abort("Graph " + graphName + ": converion to double problem!");
            return;
        }
    }
    r->addPoints(xArr, yArr);
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

void AGraph_SI::load(QString graphName, QString fileName, QString graphNameInFile)
{
    if (!bGuiThread)
    {
        abort("Threads cannot load graphs!");
        return;
    }

    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(graphName));
    if (r && AbortIfExists)
    {
        abort("Graph " + graphName + " already exists!");
        return;
    }

    TFile * f = new TFile(fileName.toLocal8Bit().data());
    if (!f)
    {
        abort("Cannot open file " + fileName);
        return;
    }

    const int numKeys = f->GetListOfKeys()->GetEntries();

    ARootGraphRecord * rec = nullptr;
    bool bFound = false;
    for (int i = 0; i < numKeys; i++)
    {
        TKey * key = (TKey*)f->GetListOfKeys()->At(i);
        QString Type = key->GetClassName();
        QString Name = key->GetName();
        qDebug() << i << Type << Name;

        if (!graphNameInFile.isEmpty() && Name != graphNameInFile) continue;
        bFound = true;

        if (Type == "TGraph")
        {
            TGraph * g = (TGraph*)key->ReadObj();
            rec = new ARootGraphRecord(g, graphName, "TGraph");
            break;
        }
        else if (Type == "TGraphErrors")
        {
            TGraphErrors * ge = (TGraphErrors*)key->ReadObj();
            rec = new ARootGraphRecord(ge, graphName, "TGraphErrors");
            break;
        }
        else if (Type == "TGraph2D")
        {
            TGraph2D * hist = (TGraph2D*)key->ReadObj();
            rec = new ARootGraphRecord(hist, graphName, "TGraph2D");
            break;
        }
    }
    f->Close();
    delete f;

    if (!rec)
    {
        if (!graphNameInFile.isEmpty() && !bFound)
            abort("Graph with name " + graphNameInFile + " not found in file " + fileName);
        else
            abort("Error loading graph.\nCurrently supported graph types are TGraph, TGraphError and TGraph2D");
    }
    else
    {
        bool bOK = Graphs.append(graphName, rec, false);
        if (!bOK)
        {
            delete rec;
            abort("Load graph from file " + fileName + " failed!");
        }
    }
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

QVariantList AGraph_SI::getData(QString GraphName)
{
    QVariantList res;

    ARootGraphRecord * r = dynamic_cast<ARootGraphRecord*>(Graphs.getRecord(GraphName));
    if (!r)
    {
        abort("Graph " + GraphName + " not found!");
        return res;
    }

    std::vector<double> x, y, z, errx, erry;
    r->getData(x, y, z, errx, erry);
    const bool bHaveZ   = !z.empty();
    const bool bHaveErr = !errx.empty();
    const size_t size = x.size();
    for (size_t i = 0; i < size; i++)
    {
        QVariantList el;
        el << x[i] << y[i];
        if (bHaveZ) el << z[i];
        if (bHaveErr) el << errx[i] << erry[i];
        res.push_back(el);
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
