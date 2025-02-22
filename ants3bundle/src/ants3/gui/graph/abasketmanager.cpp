#include "abasketmanager.h"
#include "afiletools.h"
#include "ajsontools.h"

#include <QJsonDocument>
#include <QDebug>

#include "TObject.h"
#include "TNamed.h"
#include "TF1.h"
#include "TF2.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraph2D.h"
#include "TAxis.h"
#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TKey.h"
#include "TLegend.h"
#include "TLegendEntry.h"

ABasketManager::~ABasketManager()
{
    clear();
}

TGraph * HistToGraph(TH1 * h)
{
    if (!h) return nullptr;
    QVector<double> x, f;
    for (int i=1; i<h->GetXaxis()->GetNbins()-1; i++)
    {
        x.append(h->GetBinCenter(i));
        f.append(h->GetBinContent(i));
    }
    return new TGraph(x.size(), x.data(), f.data());
}

void ABasketManager::add(const QString & name, const QVector<ADrawObject> & drawObjects)
{
    ABasketItem item;
    item.Name = name;
    item.Type = drawObjects.first().Pointer->ClassName();

    QMap<TObject*, TObject*> OldToNew;
    TLegend * Legend = nullptr;

    for (int i = 0; i < drawObjects.size(); i++)
    {
        const ADrawObject & drObj = drawObjects.at(i);

        QString type = drObj.Pointer->ClassName();
        QString options = drObj.Options;

        TObject * tobj = nullptr;
        //preparing to clone: process special cases
        if (type == "TF1")
        {
            //does not work normal way - recalculated LRFs will replace old ones even in the copied object
            TF1* f = (TF1*)drObj.Pointer;
            TGraph* g = HistToGraph( f->GetHistogram() );
            g->GetXaxis()->SetTitle( f->GetHistogram()->GetXaxis()->GetTitle() );
            g->GetYaxis()->SetTitle( f->GetHistogram()->GetYaxis()->GetTitle() );
            g->SetLineStyle( f->GetLineStyle());
            g->SetLineColor(f->GetLineColor());
            g->SetLineWidth(f->GetLineWidth());
            g->SetFillColor(0);
            g->SetFillStyle(0);
            tobj = g;
            if (!options.contains("same", Qt::CaseInsensitive))
                if (!options.contains("AL", Qt::CaseInsensitive))
                    options += "AL";
            type = g->ClassName();
            if (i == 0) item.Type = type;
        }
        else if (type == "TF2")
        {
            //does not work normal way - recalculated LRFs will replace old ones even in the copied object
            TF2* f = (TF2*)drObj.Pointer;
            TH1* h = f->CreateHistogram();
            tobj = h;
            type = h->ClassName();
            if (i == 0) item.Type = type;
        }
        else
        {
            //general case
            tobj = drObj.Pointer;
        }

        TObject * clone;
        TLegend * OldLegend = dynamic_cast<TLegend*>(tobj);
        if (OldLegend)
        {
            Legend = new TLegend(*OldLegend); // after cloning Legend has invalid object pointers, so have to use copy constructor
            clone = Legend;
        }
        else if (type.startsWith("TH2"))
        {
            // bug in root? causes crash if options were changed. Going copy constructor way
            if      (type == "TH2D") clone = new TH2D(*static_cast<TH2D*>(tobj));
            else if (type == "TH2F") clone = new TH2F(*static_cast<TH2F*>(tobj));
            else if (type == "TH2I") clone = new TH2I(*static_cast<TH2I*>(tobj));
            else if (type == "TH2S") clone = new TH2S(*static_cast<TH2S*>(tobj));
            else if (type == "TH2C") clone = new TH2C(*static_cast<TH2C*>(tobj));
            else clone = tobj->Clone(); //paranoic
        }
        else if (type == "TGraph2D")
        {
            clone = new TGraph2D(*static_cast<TGraph2D*>(tobj));  // clone unzooms to full range
        }
        else
        {
            clone = tobj->Clone();
            //qDebug() << "to Basket, old-->cloned" << drObj.Pointer << "-->" << clone;
        }
        OldToNew[drObj.Pointer] = clone;

        ADrawObject newObj = ADrawObject(clone, options, drObj.bEnabled, drObj.bLogScaleX, drObj.bLogScaleY);
        newObj.CustomMargins = drObj.CustomMargins;
        item.DrawObjects.append(newObj);
    }

    if (Legend)
    {
        TList * elist = Legend->GetListOfPrimitives();
        int num = elist->GetEntries();
        for (int ie = 0; ie < num; ie++)
        {
            TLegendEntry * en = static_cast<TLegendEntry*>( (*elist).At(ie) );
            const QString text = en->GetLabel();
            //qDebug() << "Old entry obj:"<< en->GetObject() << " found?" << OldToNew[ en->GetObject() ];
            en->SetObject( OldToNew[ en->GetObject() ] ); // will override the label
            en->SetLabel(text.toLatin1().data());
        }
    }

    // it is convinient if title-less objects will be named the same as their basket entry - less problems with a TLegend
    if (item.DrawObjects.size() > 0)
    {
        TNamed * named = dynamic_cast<TNamed*>(item.DrawObjects.first().Pointer);
        if (named)
        {
            const TString title = named->GetTitle();
            if (title == "")
            {
                named->SetTitle(name.toLatin1().data());
                if (item.Name.isEmpty()) item.Name = name;
            }
        }
    }

    Basket << item;
}

void ABasketManager::update(int index, const QVector<ADrawObject> & drawObjects)
{
    if (index < 0 || index >= Basket.size()) return;

    add(Basket[index].Name, drawObjects);
    std::swap(Basket[index], Basket.last());

    Basket.remove(Basket.size()-1);
}

QVector<ADrawObject> ABasketManager::getCopy(int index) const
{
    QVector<ADrawObject> res;

    QMap<TObject*, TObject*> OldToNew;
    TLegend * Legend = nullptr;

    if (index >= 0 && index < Basket.size())
    {
        for (const ADrawObject & obj : Basket.at(index).DrawObjects)
        {
            TObject * clone = nullptr;
            TLegend * OldLegend = dynamic_cast<TLegend*>(obj.Pointer);
            if (OldLegend)
            {
                Legend = new TLegend(*OldLegend); // if cloned, Legend has invalid object pointers
                clone = Legend;
            }
            else
            {
                TGraph2D * g2 = dynamic_cast<TGraph2D*>(obj.Pointer);
                if (g2) clone = new TGraph2D(*g2); //clone unzooms to full range
                else    clone = obj.Pointer->Clone();
                OldToNew[obj.Pointer] = clone;
                //qDebug() << "From basket, old-->cloned" << obj.Pointer << "-->" << clone;
            }

            ADrawObject newObj(clone, obj.Options, obj.bEnabled, obj.bLogScaleX, obj.bLogScaleY);
            newObj.CustomMargins = obj.CustomMargins;
            res << newObj;
        }

        if (Legend)
        {
            TList * elist = Legend->GetListOfPrimitives();
            int num = elist->GetEntries();
            for (int ie = 0; ie < num; ie++)
            {
                TLegendEntry * en = static_cast<TLegendEntry*>( (*elist).At(ie) );
                QString text = en->GetLabel();
                //qDebug() << "Old entry obj:"<< en->GetObject() << " found?" << OldToNew[ en->GetObject() ];
                en->SetObject( OldToNew[ en->GetObject() ] ); // will override the label
                en->SetLabel(text.toLatin1().data());
            }
        }
    }

    return res;
}

void ABasketManager::clear()
{
    for (int ib=0; ib<Basket.size(); ib++)
        Basket[ib].clearObjects();
    Basket.clear();
}

void ABasketManager::remove(int index)
{
    if (index < 0 || index >= Basket.size()) return;
    Basket[index].clearObjects();
    Basket.remove(index);
}

QString ABasketManager::getType(int index) const
{
    if (index < 0 || index >= Basket.size()) return "";
    return Basket[index].Type;
}

int ABasketManager::size() const
{
    return Basket.size();
}

QString ABasketManager::getName(int index) const
{
    if (index < 0 || index >= Basket.size()) return "";
    return Basket.at(index).Name;
}

void ABasketManager::rename(int index, const QString & newName)
{
    if (index < 0 || index >= Basket.size()) return;

    // it is convinient if title-less objects are named the same as their basket entry - less problems with a TLegend
    if (Basket[index].DrawObjects.size() > 0)
    {
        TNamed * named = dynamic_cast<TNamed*>(Basket[index].DrawObjects.first().Pointer);
        if (named)
        {
            const TString title = named->GetTitle();
            if (title == Basket[index].Name)
                named->SetTitle(newName.toLatin1().data());
        }
    }

    Basket[index].Name = newName;
}

QStringList ABasketManager::getItemNames() const
{
    QStringList res;
    for (const ABasketItem & item : Basket)
        res << item.Name;
    return res;
}

void ABasketManager::saveAll(const QString & fileName)
{
    TFile f(fileName.toLocal8Bit(), "RECREATE");

    int objectIndex = 0;
    QJsonArray BasketArray;
    for (int ib = 0; ib < Basket.size(); ib++)
    {
        QJsonObject ItemJson;
        ItemJson["ItemName"] = Basket.at(ib).Name;

        QJsonArray ItemArray;
        const QVector<ADrawObject> & DrawObjects = Basket.at(ib).DrawObjects;
        for (int io = 0; io < DrawObjects.size(); io++)
        {
            const ADrawObject & obj = DrawObjects.at(io);
            TString KeyName = "#";
            KeyName += objectIndex;
            objectIndex++;
            obj.Pointer->Write(KeyName);

            QJsonObject js;
            js["Name"] = obj.Name;
            js["Options"] = obj.Options;
            js["Enabled"] = obj.bEnabled;
            js["LogX"] = obj.bLogScaleX;
            js["LogY"] = obj.bLogScaleY;
            TLegend * Legend = dynamic_cast<TLegend*>(obj.Pointer);
            if (Legend)
            {
                QJsonArray links;
                TList * elist = Legend->GetListOfPrimitives();
                int num = elist->GetEntries();
                for (int ie = 0; ie < num; ie++)
                {
                    TLegendEntry * en = static_cast<TLegendEntry*>( (*elist).At(ie));
                    //qDebug() << "Entry obj:"<< en->GetObject();
                    links.append(findPointerInDrawObjects(DrawObjects, en->GetObject()));
                }
                js["LegendLinks"] = links;
            }
            ItemArray.append(js);
        }
        ItemJson["ItemObjects"] = ItemArray;

        BasketArray.append(ItemJson);
    }

    QJsonDocument doc;
    doc.setArray(BasketArray);
    QString descStr(doc.toJson());
    qDebug() << descStr;

    TNamed desc;
    desc.SetTitle(descStr.toLocal8Bit().data());
    desc.Write("BasketDescription_v2");

    f.Close();
}

QString ABasketManager::appendBasket(const QString & fileName)
{
    TFile f(fileName.toLocal8Bit().data());

    // log block
//    int numKeys = f.GetListOfKeys()->GetEntries();
//    qDebug() << "Keys:"<<numKeys;
//    for (int i=0; i<numKeys; i++)
//    {
//        TKey * key = (TKey*)f.GetListOfKeys()->At(i);
//        QString type = key->GetClassName();
//        QString ObjName = key->GetName();
//        QString title = key->GetTitle();
//        qDebug() << title << ObjName << type;
//    }

    bool ok = true;
    TNamed* desc = (TNamed*)f.Get("BasketDescription_v2");
    if (desc)
    {
        //new system
        QString text = desc->GetTitle();
        QJsonDocument doc(QJsonDocument::fromJson(text.toLatin1().data()));
        QJsonArray BasketArray = doc.array();
        //qDebug() << BasketArray;

        int basketSize = BasketArray.size();
        int KeyIndex = 0;
        for (int iBasketItem = 0; iBasketItem < basketSize; iBasketItem++ )
        {
            QJsonObject ItemJson = BasketArray[iBasketItem].toObject();
            //qDebug() << "Item"<<iBasketItem << ItemJson;

            QString    ItemName  = ItemJson["ItemName"].toString();
            QJsonArray ItemArray = ItemJson["ItemObjects"].toArray();
            int        ItemSize  = ItemArray.size();

            QVector<ADrawObject> drawObjects;
            int LegendIndex = -1;
            QJsonArray LegendLinks;
            for (int iDrawObj = 0; iDrawObj < ItemSize; iDrawObj++)
            {
                QJsonObject js = ItemArray[iDrawObj].toObject();
                QString Name     = js["Name"].toString();
                QString Options  = js["Options"].toString();
                bool    bEnabled = js["Enabled"].toBool();
                bool    bLogX    = false; jstools::parseJson(js, "LogX", bLogX);
                bool    bLogY    = false; jstools::parseJson(js, "LogY", bLogY);

                TKey *key = (TKey*)f.GetListOfKeys()->At(KeyIndex);
                KeyIndex++;

                TObject * p = key->ReadObj();
                if (p)
                {
                    ADrawObject Obj(p, Options);
                    Obj.Name = Name;
                    Obj.bEnabled = bEnabled;
                    Obj.bLogScaleX = bLogX;
                    Obj.bLogScaleY = bLogY;
                    drawObjects << Obj;

                    TLegend * Legend = dynamic_cast<TLegend*>(p);
                    if (Legend)
                    {
                        LegendIndex = iDrawObj;
                        LegendLinks = js["LegendLinks"].toArray();
                    }
                }
                else
                {
                    TString nm(QString("Corrupted_%1").arg(Name).toLatin1().data());
                    p = new TNamed(nm , nm);
                    qWarning() << "Corrupted TKey in basket file" << fileName << " object:" << Name;
                }
            }

            if (LegendIndex != -1)
            {
                TLegend * Legend = static_cast<TLegend*>(drawObjects[LegendIndex].Pointer);
                TList * elist = Legend->GetListOfPrimitives();
                int num = elist->GetEntries();
                for (int ie = 0; ie < num; ie++)
                {
                    TLegendEntry * en = static_cast<TLegendEntry*>( (*elist).At(ie) );
                    QString text = en->GetLabel();
                    TObject * p = nullptr;
                    int iObj = LegendLinks[ie].toInt();
                    if (iObj >= 0 && iObj < drawObjects.size())
                        p = drawObjects[iObj].Pointer;
                    en->SetObject(p);                       // will override the label
                    en->SetLabel(text.toLatin1().data());   // so restore it
                }
            }

            if (!drawObjects.isEmpty())
            {
                ABasketItem item;
                item.Name = ItemName;
                item.DrawObjects = drawObjects;
                item.Type = drawObjects.first().Pointer->ClassName();
                Basket << item;
                drawObjects.clear();
            }
        }
    }
    else
    {
        desc = (TNamed*)f.Get("BasketDescription");
        if (desc)
        {
            //old system
            QString text = desc->GetTitle();
            //qDebug() << "Basket description:"<<text;
            //qDebug() << "Number of keys:"<<f->GetListOfKeys()->GetEntries();

            QStringList sl = text.split('\n', Qt::SkipEmptyParts);

            int numLines = sl.size();
            int basketSize =  numLines/2;
            //qDebug() << "Description lists" << basketSize << "objects in the basket";


            int indexFileObject = 0;
            if (numLines % 2 == 0 ) // should be even number of lines
            {
                for (int iDrawObject = 0; iDrawObject < basketSize; iDrawObject++ )
                {
                    //qDebug() << ">>>>Object #"<< iDrawObject;
                    QString name = sl[iDrawObject*2];
                    bool ok;
                    QStringList fields = sl[iDrawObject*2+1].split('|');
                    if (fields.size()<2)
                    {
                        qWarning()<<"Too short descr line";
                        ok=false;
                        break;
                    }

                    const QString sNumber = fields[0];

                    int numObj = sNumber.toInt(&ok);
                    if (!ok)
                    {
                        qWarning() << "Num obj convertion error!";
                        ok=false;
                        break;
                    }
                    if (numObj != fields.size()-1)
                    {
                        qWarning()<<"Number of objects vs option strings mismatch:"<<numObj<<fields.size()-1;
                        ok=false;
                        break;
                    }

                    //qDebug() << "Name:"<< name << "objects:"<< numObj;

                    QVector<ADrawObject> drawObjects;
                    for (int iDrawObj = 0; iDrawObj < numObj; iDrawObj++)
                    {
                        TKey *key = (TKey*)f.GetListOfKeys()->At(indexFileObject++);
                        //key->SetMotherDir(0);
                        //QString type = key->GetClassName();
                        //TString objName = key->GetName();
                        //qDebug() << "-->"<< iDrawObj <<"   "<<objName<<"  "<<type<<"   "<<fields[iDrawObj+1];

                        TObject * p = key->ReadObj();
                        if (p) drawObjects << ADrawObject(p, fields[iDrawObj+1]);
                        //qDebug() << p;

                        /*
                    TLegend * Legend = dynamic_cast<TLegend*>(p);
                    if (Legend)
                    {
                        TList * elist = Legend->GetListOfPrimitives();
                        int num = elist->GetEntries();
                        for (int ie = 0; ie < num; ie++)
                        {
                            TLegendEntry * en = static_cast<TLegendEntry*>( (*elist).At(ie));
                            qDebug() << "Entry obj:"<< en->GetObject();
                        }
                    }
                    */
                    }

                    if (!drawObjects.isEmpty())
                    {
                        ABasketItem item;
                        item.Name = name;
                        item.DrawObjects = drawObjects;
                        item.Type = drawObjects.first().Pointer->ClassName();
                        Basket << item;
                        drawObjects.clear();
                    }
                }
            }
            else ok = false;
        }
        else
        {
            f.Close();
            return QString("%1: this is not a valid ANTS2 basket file!").arg(fileName);
        }
    }

    f.Close();
    if (!ok) return QString("%1: corrupted basket file").arg(fileName);
    return "";
}

QString ABasketManager::appendTxtAsGraph(const QString & fileName)
{
    QVector<double> x, y;
    QVector<QVector<double> *> V = {&x, &y};
    const QString res = ftools::loadDoubleVectorsFromFile(fileName, V);
    if (!res.isEmpty()) return res;

    TGraph* gr = new TGraph(x.size(), x.data(), y.data());
    gr->SetMarkerStyle(20);
    ABasketItem item;
    item.Name = "Graph";
    item.DrawObjects << ADrawObject(gr, "APL");
    item.Type = gr->ClassName();
    Basket << item;

    return "";
}

QString ABasketManager::appendTxtAsGraphErrors(const QString &fileName)
{
    QVector<double> x, y, err;
    QVector<QVector<double> *> V = {&x, &y, &err};
    const QString res = ftools::loadDoubleVectorsFromFile(fileName, V);
    if (!res.isEmpty()) return res;

    TGraphErrors* gr = new TGraphErrors(x.size(), x.data(), y.data(), 0, err.data());
    gr->SetMarkerStyle(20);
    ABasketItem item;
    item.Name = "GraphErrors";
    item.DrawObjects << ADrawObject(gr, "APL");
    item.Type = gr->ClassName();
    Basket << item;

    return "";
}

void ABasketManager::appendRootHistGraphs(const QString & fileName)
{
    QByteArray ba = fileName.toLocal8Bit();
    const char *c_str = ba.data();
    TFile * f = new TFile(c_str);

    const int numKeys = f->GetListOfKeys()->GetEntries();
    //qDebug() << "File contains" << numKeys << "TKeys";

    for (int i = 0; i < numKeys; i++)
    {
        TKey * key = (TKey*)f->GetListOfKeys()->At(i);
        QString Type = key->GetClassName();
        QString Name = key->GetName();
        //qDebug() << i << Type << Name;

        if (Type.startsWith("TH") || Type.startsWith("TProfile") || Type.startsWith("TGraph"))
        {
            TObject * p = key->ReadObj();
            if (p)
            {
                ABasketItem item;
                    item.Name = Name;
                    item.DrawObjects << ADrawObject(p, "");
                    item.Type = p->ClassName();
                    if (item.Name.isEmpty()) item.Name = QString("%1#%2").arg(item.Type).arg(i);
                Basket << item;
                //qDebug() << "  appended";
            }
            else qWarning() << "Failed to read object of type" << Type << "from file " << fileName;
        }
        //else qDebug() << "  ignored";
    }

    f->Close();
}

void ABasketManager::reorder(const QVector<int> &indexes, int to)
{
    QVector< ABasketItem > ItemsToMove;
    for (int i = 0; i < indexes.size(); i++)
    {
        const int index = indexes.at(i);
        ItemsToMove << Basket.at(index);
        Basket[index]._flag = true;       // mark to be deleted
    }

    for (int i = 0; i < ItemsToMove.size(); i++)
    {
        Basket.insert(to, ItemsToMove.at(i));
        to++;
    }

    for (int i = Basket.size()-1; i >= 0; i--)
        if (Basket.at(i)._flag)
            Basket.remove(i);
}

#include "aroothistappenders.h"
QString ABasketManager::mergeHistograms(const std::vector<int> & indexes)
{
    int foundHistos = 0;
    int bins;
    double from, to;
    for (int index : indexes)
    {
        const ABasketItem & item = Basket[index];
        if (item.DrawObjects.empty()) continue;

        const ADrawObject & obj = item.DrawObjects.front();
        const TH1D * h = dynamic_cast<const TH1D*>(obj.Pointer);
        if (!h) return "At least one of the selected items is not a 1D histogram";

        int thisBins = h->GetNbinsX();
        double thisFrom = h->GetXaxis()->GetXmin();
        double thisTo   = h->GetXaxis()->GetXmax();
        if (foundHistos == 0)
        {
            // this is the first
            bins = thisBins;
            from = thisFrom;
            to   = thisTo;
        }
        else
        {
            if (bins != thisBins || from != thisFrom || to != thisTo)
                return "Found mismatch in histogram binning";
        }
        foundHistos++;
    }

    if (foundHistos < 2) return "Select at least two 1D histograms";

    TH1D * hist = nullptr;
    QString name;
    for (int index : indexes)
    {
        const ABasketItem & item = Basket[index];
        if (item.DrawObjects.empty()) continue;

        const ADrawObject & obj = item.DrawObjects.front();
        const TH1D * h = dynamic_cast<const TH1D*>(obj.Pointer);

        appendTH1DwithStat(hist, h);
        name += item.Name + "+";
    }
    name.chop(1);

    QVector<ADrawObject> drawObjects;
    drawObjects << ADrawObject(hist, "hist");
    add(name, drawObjects);

    delete hist;

    return "";
}

int ABasketManager::findPointerInDrawObjects(const QVector<ADrawObject> &DrawObjects, TObject *obj) const
{
    for (int i=0; i<DrawObjects.size(); i++)
        if (DrawObjects.at(i).Pointer == obj) return i;
    return -1;
}
