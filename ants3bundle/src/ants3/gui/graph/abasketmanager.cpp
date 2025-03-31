#include "abasketmanager.h"
#include "afiletools.h"
#include "ajsontools.h"
#include "amultidrawrecord.h"

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
    std::vector<double> x, f;
    for (int i=1; i<h->GetXaxis()->GetNbins()-1; i++)
    {
        x.push_back(h->GetBinCenter(i));
        f.push_back(h->GetBinContent(i));
    }
    return new TGraph(x.size(), x.data(), f.data());
}

void ABasketManager::add(const QString & name, const std::vector<ADrawObject> & drawObjects)
{
    if (drawObjects.empty()) return;

    ABasketItem item;
    item.Name = name;

    if (drawObjects.front().Multidraw)
    {
        item.DrawObjects.push_back(drawObjects.front());
        item.Type = "Multidraw";
        Basket.push_back(item);
        return;
    }

    item.Type = drawObjects.front().Pointer->ClassName();

    QMap<TObject*, TObject*> OldToNew;
    TLegend * Legend = nullptr;

    for (size_t i = 0; i < drawObjects.size(); i++)
    {
        const ADrawObject & drObj = drawObjects[i];

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
        item.DrawObjects.push_back(newObj);
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
        TNamed * named = dynamic_cast<TNamed*>(item.DrawObjects.front().Pointer);
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

    Basket.push_back(item);
}

void ABasketManager::update(int index, const std::vector<ADrawObject> & drawObjects)
{
    if (index < 0 || index >= Basket.size()) return;

    add(Basket[index].Name, drawObjects);
    std::swap(Basket[index], Basket.back());
    Basket.pop_back();
}

std::vector<ADrawObject> ABasketManager::getCopy(int index) const
{
    std::vector<ADrawObject> res;

    if (index < 0 || index >= Basket.size()) return res;

    if (!Basket[index].DrawObjects.empty() && Basket[index].DrawObjects.front().Multidraw)
    {
        res.push_back(Basket[index].DrawObjects.front());
        return res;
    }

    QMap<TObject*, TObject*> oldToNew;
    TLegend * Legend = nullptr;

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
                oldToNew[obj.Pointer] = clone;
                //qDebug() << "From basket, old-->cloned" << obj.Pointer << "-->" << clone;
            }

            ADrawObject newObj(clone, obj.Options, obj.bEnabled, obj.bLogScaleX, obj.bLogScaleY);
            newObj.CustomMargins = obj.CustomMargins;
            res.push_back(newObj);
        }

        if (Legend)
        {
            TList * elist = Legend->GetListOfPrimitives();
            int num = elist->GetEntries();
            for (int ie = 0; ie < num; ie++)
            {
                TLegendEntry * en = static_cast<TLegendEntry*>( (*elist).At(ie) );
                QString text = en->GetLabel();
                //qDebug() << "Old entry obj:"<< en->GetObject() << " found?" << oldToNew[ en->GetObject() ];
                en->SetObject( oldToNew[ en->GetObject() ] ); // will override the label
                en->SetLabel(text.toLatin1().data());
            }
        }
    }

    return res;
}

void ABasketManager::clear()
{
    for (ABasketItem & bi : Basket) bi.clearObjects();
    Basket.clear();
}

std::vector<size_t> ABasketManager::getAllMultidrawsUsingIndex(size_t index)
{
    std::vector<size_t> ar;
    for (size_t iM = 0; iM < Basket.size(); iM++)
    {
        if (!isMultidraw(iM)) continue;
        if (isMemberOfSpecificMultidraw(index, iM)) ar.push_back(iM);
    }
    return ar;
}

QString ABasketManager::remove(std::vector<int> indexesToRemove)
{
    // check the possibility to remove the requested basket items
    for (int iToRemove : indexesToRemove)
    {
        std::vector<size_t> arMultisUsingThis = getAllMultidrawsUsingIndex(iToRemove);
        if (arMultisUsingThis.empty()) continue;

        // if all multidraws using that index are also to be removed, do not block removal of the index
        for (int iRem : indexesToRemove)
        {
            for (size_t i = 0; i < arMultisUsingThis.size(); i++)
            {
                const size_t iMult = arMultisUsingThis[i];
                if (iMult == iRem) arMultisUsingThis.erase(arMultisUsingThis.begin() + i); // removing that multidraw, so not blocking
            }
        }
        if (arMultisUsingThis.empty()) continue;

        QString ret = QString("Basket item %0 cannot be removed.\nIt is used by multidraw%1").arg(getName(iToRemove)).arg(arMultisUsingThis.size() == 1 ? " " : "s:\n");
        for (size_t i : arMultisUsingThis) ret += getName(i) + " ";
        return ret;
    }

    std::sort(indexesToRemove.begin(), indexesToRemove.end());

    // making mapping array to shift the entry indexes of the remaining multidraws
    std::vector<size_t> mapping;
    for (size_t i = 0; i < Basket.size(); i++) mapping.push_back(i);
    const size_t oldSize = mapping.size();

    // removing items
    for (int i = indexesToRemove.size() - 1; i >= 0; i--)
    {
        int iToRemove = indexesToRemove[i];
        if (iToRemove < 0 || iToRemove >= Basket.size()) continue;

        Basket[iToRemove].clearObjects();
        Basket.erase(Basket.begin() + iToRemove);
        mapping.erase(mapping.begin() + iToRemove);
    }

    // inverting mapping array (will be newIndex[oldIndex])
    std::vector<size_t> newIndexMap;
    newIndexMap.resize(oldSize);
    for (size_t iNewIndex = 0; iNewIndex < mapping.size(); iNewIndex++)  // [0,1,2,3,4] --> removed 1 and 3 --> mapping [0,2,4] --> old0 is 0; ol2 is 1; old 4 is 2
    {
        size_t oldIndex = mapping[iNewIndex];
        newIndexMap[oldIndex] = iNewIndex;
    }

    // applying the shift in the multidraw indexing
    for (ABasketItem & item : Basket)
    {
        if (item.DrawObjects.empty()) continue;
        if (!item.DrawObjects.front().Multidraw) continue;

        AMultidrawRecord & rec = item.DrawObjects.front().MultidrawSettings;

        for (size_t i = 0; i < rec.BasketItems.size(); i++)
            rec.BasketItems[i] = newIndexMap[rec.BasketItems[i]];
    }

    return "";
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
        TNamed * named = dynamic_cast<TNamed*>(Basket[index].DrawObjects.front().Pointer);
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

bool ABasketManager::isMultidraw(int index) const
{
    if (index < 0 || index >= Basket.size()) return false;
    if (Basket[index].DrawObjects.empty()) return false;
    return Basket[index].DrawObjects.front().Multidraw;
}

bool ABasketManager::isMemberOfSpecificMultidraw(int index, int multidrawIndex)
{
    if (index < 0 || index >= Basket.size()) return false;
    if (!isMultidraw(multidrawIndex)) return false;
    if (Basket[multidrawIndex].DrawObjects.empty())
    {
        qWarning() << "Unexpected empty DrawObjects for a multidraw item";
        return false;
    }

    for (int i : Basket[multidrawIndex].DrawObjects.front().MultidrawSettings.BasketItems)
        if (i == index) return true;

    return false;
}

void ABasketManager::saveBasket(const QString & fileName)
{
    TFile basketFile(fileName.toLocal8Bit().data(), "RECREATE");

    int objectIndex = 0;
    QJsonArray basketJAr;
    for (size_t ib = 0; ib < Basket.size(); ib++)
    {
        QJsonObject itemJson;
        itemJson["ItemName"] = Basket[ib].Name;

        QJsonArray itemArray;
        const std::vector<ADrawObject> & drawObjects = Basket[ib].DrawObjects;
        for (size_t io = 0; io < drawObjects.size(); io++)
        {
            const ADrawObject & obj = drawObjects[io];
            TString keyName = "#";
            keyName += objectIndex;
            objectIndex++;

            if (obj.Pointer) obj.Pointer->Write(keyName);
            else
            {
                TNamed dummy("Dummy", "Dummy");
                dummy.Write(keyName);
            }

            QJsonObject js;
            obj.writeToJson(js);

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
                    links.append(findPointerInDrawObjects(drawObjects, en->GetObject()));
                }
                js["LegendLinks"] = links;
            }
            itemArray.append(js);
        }
        itemJson["ItemObjects"] = itemArray;

        basketJAr.append(itemJson);
    }

    QJsonDocument doc;
    doc.setArray(basketJAr);
    QString descStr(doc.toJson());
    //qDebug() << descStr;

    TNamed desc;
    desc.SetTitle(descStr.toLocal8Bit().data());
    desc.Write("BasketDescription_v2");

    basketFile.Close();
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

    size_t oldBasketSize = Basket.size();

    TNamed * desc = (TNamed*)f.Get("BasketDescription_v2");
    if (!desc)
    {
        f.Close();
        return QString("%1: this is not a valid ANTS3 basket file!").arg(fileName);
    }

    QString text = desc->GetTitle();
    QJsonDocument doc(QJsonDocument::fromJson(text.toLatin1().data()));
    QJsonArray basketArray = doc.array();

    int basketSize = basketArray.size();
    int keyIndex = 0;
    for (int iBasketItem = 0; iBasketItem < basketSize; iBasketItem++ )
    {
        QJsonObject itemJson = basketArray[iBasketItem].toObject();

        QString     itemName  = itemJson["ItemName"].toString();
        QJsonArray  itemArray = itemJson["ItemObjects"].toArray();
        const int   itemSize  = itemArray.size();

        std::vector<ADrawObject> drawObjects;
        int legendIndex = -1;
        QJsonArray legendLinks;
        for (int iDrawObj = 0; iDrawObj < itemSize; iDrawObj++)
        {
            QJsonObject js = itemArray[iDrawObj].toObject();

            QString name = js["Name"].toString();
            TKey * key = (TKey*)f.GetListOfKeys()->At(keyIndex);
            keyIndex++;

            TObject * tObj = key->ReadObj();
            if (tObj)
            {
                ADrawObject drawObj(tObj, "");
                drawObj.readFromJson(js);
                if (drawObj.Multidraw && oldBasketSize != 0) drawObj.shiftMultidrawIndexesBy(oldBasketSize);
                drawObjects.push_back(drawObj);

                TLegend * legend = dynamic_cast<TLegend*>(tObj);
                if (legend)
                {
                    legendIndex = iDrawObj;
                    legendLinks = js["LegendLinks"].toArray();
                }
            }
            else
            {
                TString nm( QString("Corrupted_%1").arg(name).toLatin1().data() );
                tObj = new TNamed(nm, nm);
                qWarning() << "Corrupted TKey in basket file" << fileName << " object:" << name;
            }
        }

        if (legendIndex != -1)
        {
            TLegend * legend = static_cast<TLegend*>(drawObjects[legendIndex].Pointer);
            TList * elist = legend->GetListOfPrimitives();
            const int num = elist->GetEntries();
            for (int ie = 0; ie < num; ie++)
            {
                TLegendEntry * en = static_cast<TLegendEntry*>( (*elist).At(ie) );
                QString text = en->GetLabel();
                TObject * tObj = nullptr;
                int iObj = legendLinks[ie].toInt();
                if (iObj >= 0 && iObj < drawObjects.size())
                    tObj = drawObjects[iObj].Pointer;
                en->SetObject(tObj);                    // will override the label
                en->SetLabel(text.toLatin1().data());   // so restore it
            }
        }

        if (!drawObjects.empty())
        {
            ABasketItem item;
            item.Name = itemName;
            item.DrawObjects = drawObjects;
            item.Type = drawObjects.front().Pointer->ClassName();
            Basket.push_back(item);
            drawObjects.clear();
        }
    }

    f.Close();
    return "";
}

QString ABasketManager::appendTxtAsGraph(const QString & fileName)
{
    std::vector<double> x, y;
    std::vector<std::vector<double> *> V = {&x, &y};
    QString res = ftools::loadDoubleVectorsFromFile(fileName, V);
    if (!res.isEmpty()) return res;

    TGraph* gr = new TGraph(x.size(), x.data(), y.data());
    gr->SetMarkerStyle(20);
    ABasketItem item;
    item.Name = "Graph";
    item.DrawObjects.push_back( ADrawObject(gr, "APL") );
    item.Type = gr->ClassName();
    Basket.push_back(item);

    return "";
}

QString ABasketManager::appendTxtAsGraphErrors(const QString &fileName)
{
    std::vector<double> x, y, err;
    std::vector<std::vector<double> *> V = {&x, &y, &err};
    QString res = ftools::loadDoubleVectorsFromFile(fileName, V);
    if (!res.isEmpty()) return res;

    TGraphErrors* gr = new TGraphErrors(x.size(), x.data(), y.data(), 0, err.data());
    gr->SetMarkerStyle(20);
    ABasketItem item;
    item.Name = "GraphErrors";
    item.DrawObjects.push_back( ADrawObject(gr, "APL") );
    item.Type = gr->ClassName();
    Basket.push_back(item);

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
                    item.DrawObjects.push_back( ADrawObject(p, "") );
                    item.Type = p->ClassName();
                    if (item.Name.isEmpty()) item.Name = QString("%1#%2").arg(item.Type).arg(i);
                Basket.push_back(item);
                //qDebug() << "  appended";
            }
            else qWarning() << "Failed to read object of type" << Type << "from file " << fileName;
        }
        //else qDebug() << "  ignored";
    }

    f->Close();
}

void ABasketManager::reorder(const std::vector<int> & indexes, int to)
{
    std::vector<ABasketItem> ItemsToMove;
    for (size_t i = 0; i < indexes.size(); i++)
    {
        const int index = indexes[i];
        ItemsToMove.push_back( Basket[index] );
        Basket[index]._flag = true;       // mark to be deleted
    }

    for (size_t i = 0; i < ItemsToMove.size(); i++)
    {
        Basket.insert(Basket.begin() + to, ItemsToMove[i]);
        to++;
    }

    for (int i = Basket.size()-1; i >= 0; i--)
        if (Basket[i]._flag)
            Basket.erase(Basket.begin() + i);
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

    std::vector<ADrawObject> drawObjects;
    drawObjects.push_back( ADrawObject(hist, "hist") );
    add(name, drawObjects);

    delete hist;

    return "";
}

int ABasketManager::findPointerInDrawObjects(const std::vector<ADrawObject> & drawObjects, TObject * obj) const
{
    for (size_t i = 0; i < drawObjects.size(); i++)
        if (drawObjects[i].Pointer == obj) return i;
    return -1;
}
