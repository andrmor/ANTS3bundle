#include "abasketmanager.h"
#include "afiletools.h"
#include "ajsontools.h"
#include "amultidrawrecord.h"

#include <QJsonDocument>
#include <QDebug>

#include <algorithm>
#include <functional>

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
        else clone = ABasketManager::makeCloneOfTObject(tobj);

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

TObject * ABasketManager::makeCloneOfTObject(TObject * tobj)
{
    TObject * clone = nullptr;
    if (!tobj) return clone;

    const QString type = tobj->ClassName();
    if (type.startsWith("TH2"))
    {
        // maybe obsolete now! old comment: bug in root? causes crash if options were changed. Going copy constructor way
        if      (type == "TH2D") clone = new TH2D(*static_cast<TH2D*>(tobj));
        else if (type == "TH2F") clone = new TH2F(*static_cast<TH2F*>(tobj));
        else if (type == "TH2I") clone = new TH2I(*static_cast<TH2I*>(tobj));
        else if (type == "TH2S") clone = new TH2S(*static_cast<TH2S*>(tobj));
        else if (type == "TH2C") clone = new TH2C(*static_cast<TH2C*>(tobj));
        else clone = tobj->Clone();
    }
    else if (type == "TGraph2D")
    {
        clone = tobj->Clone();
        // seems obsolete with the current root:
        //clone = new TGraph2D(*static_cast<TGraph2D*>(tobj));  // tobj->Clone() unzooms to full xy range, also forgets axis settings
        // old comment: // blanc screen on basket redraw (last ROOT checked: 6.36.04) without the next line
        //if (obj.Options.contains("tri", Qt::CaseInsensitive) || obj.Options.contains("p", Qt::CaseInsensitive))
        //    ((TGraph2D*)clone)->SetMargin(0); // col and lego work fine. ROOT :)
    }
    else if (type == "TList")
    {
        TList * oldList = static_cast<TList*>(tobj);
        TList * newList = new TList();
        for (TObject * elObj : *oldList)
        {
            TObject * cloned = ABasketManager::makeCloneOfTObject(elObj);
            newList->Add(cloned);
        }
        clone = newList;
    }
    else
        clone = tobj->Clone();

    return clone;
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
                /*
                TGraph2D * g2 = dynamic_cast<TGraph2D*>(obj.Pointer);
                if (g2)
                {
                    clone = new TGraph2D(*g2); // obj->Clone() unzooms to full xy range
                    // blanc screen on basket redraw (last ROOT checked: 6.36.04) without the next line
                    if (obj.Options.contains("tri", Qt::CaseInsensitive) || obj.Options.contains("p", Qt::CaseInsensitive))
                        ((TGraph2D*)clone)->SetMargin(0); // col and lego work fine. ROOT :)
                }
                else    clone = obj.Pointer->Clone();
                */
                clone = ABasketManager::makeCloneOfTObject(obj.Pointer);

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
    Groups.clear();
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

/*
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
*/

// ---------------------------------------------------------------------------
// Updated ABasketManager::saveBasket / ABasketManager::appendBasket
//
// Change: obj.Pointer can now be either
//   (a) a single TObject*  (existing behavior), or
//   (b) a TList* containing only individual TObject* entries (no recursion:
//       a TList inside the TList is NOT expected/handled).
//
// Approach: each ADrawObject still gets exactly one JSON entry ("js"), but
// that entry now records whether it represents a list ("IsList" + "ListSize").
// The number of TKeys written/read for that entry is 1 for a plain object,
// or ListSize for a TList (one key per list element, in list order).
// The key-numbering / key-reading index ("objectIndex" / "keyIndex") advances
// accordingly, so everything downstream (other draw objects, other basket
// items) stays correctly aligned.
// ---------------------------------------------------------------------------

void ABasketManager::saveBasket(const QString & fileName)
{
    TFile basketFile(fileName.toLocal8Bit().data(), "RECREATE");

    int objectIndex = 0;
    QJsonArray basketJAr;
    for (size_t ib = 0; ib < Basket.size(); ib++)
    {
        QJsonObject itemJson;
        itemJson["ItemName"] = Basket[ib].Name;
        if (!Basket[ib].Group.isEmpty()) itemJson["Group"] = Basket[ib].Group;   // ignored by old readers

        QJsonArray itemArray;
        const std::vector<ADrawObject> & drawObjects = Basket[ib].DrawObjects;
        for (size_t io = 0; io < drawObjects.size(); io++)
        {
            const ADrawObject & obj = drawObjects[io];

            QJsonObject js;
            obj.writeToJson(js);

            TList * list = dynamic_cast<TList*>(obj.Pointer);
            if (list)
            {
                // obj.Pointer is a flat TList of individual TObjects (no recursion)
                js["IsList"] = true;

                int listSize = 0;
                TIter next(list);
                TObject * elObj;
                while ( (elObj = next()) )
                {
                    TString keyName = "#";
                    keyName += objectIndex;
                    objectIndex++;

                    if (elObj) elObj->Write(keyName);
                    else
                    {
                        TNamed dummy("Dummy", "Dummy");
                        dummy.Write(keyName);
                    }
                    listSize++;
                }
                js["ListSize"] = listSize;
            }
            else
            {
                js["IsList"] = false;

                TString keyName = "#";
                keyName += objectIndex;
                objectIndex++;

                if (obj.Pointer) obj.Pointer->Write(keyName);
                else
                {
                    TNamed dummy("Dummy", "Dummy");
                    dummy.Write(keyName);
                }
            }

            // Legend linking is unrelated to whether obj.Pointer is a list;
            // a TLegend is never stored as a TList, so this dynamic_cast simply
            // returns nullptr in the list case and behaves as before otherwise.
            TLegend * Legend = dynamic_cast<TLegend*>(obj.Pointer);
            if (Legend)
            {
                QJsonArray links;
                TList * elist = Legend->GetListOfPrimitives();
                int num = elist->GetEntries();
                for (int ie = 0; ie < num; ie++)
                {
                    TLegendEntry * en = static_cast<TLegendEntry*>( (*elist).At(ie));
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

    // Group structure. Written AFTER all object keys: old readers address keys by position, so they are not affected
    QJsonArray groupsJAr;
    for (const QString & g : Groups) groupsJAr.append(g);
    QJsonDocument gdoc; gdoc.setArray(groupsJAr);
    TNamed gdesc;
    gdesc.SetTitle(QString(gdoc.toJson(QJsonDocument::Compact)).toLocal8Bit().data());
    gdesc.Write("BasketGroups");

    TNamed desc;
    desc.SetTitle(descStr.toLocal8Bit().data());
    desc.Write("BasketDescription_v2");

    basketFile.Close();
}

QString ABasketManager::appendBasket(const QString & fileName)
{
    TFile f(fileName.toLocal8Bit().data());

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
        QString itemName = itemJson["ItemName"].toString();
        QJsonArray itemArray = itemJson["ItemObjects"].toArray();
        const int itemSize = itemArray.size();

        std::vector<ADrawObject> drawObjects;
        int legendIndex = -1;
        QJsonArray legendLinks;

        for (int iDrawObj = 0; iDrawObj < itemSize; iDrawObj++)
        {
            QJsonObject js = itemArray[iDrawObj].toObject();
            QString name = js["Name"].toString();

            const bool isList = js.value("IsList").toBool(false);

            TObject * tObj = nullptr;

            if (isList)
            {
                // Rebuild a flat TList of individual TObjects (no recursion)
                const int listSize = js["ListSize"].toInt();
                TList * newList = new TList();

                for (int iEl = 0; iEl < listSize; iEl++)
                {
                    TKey * key = (TKey*)f.GetListOfKeys()->At(keyIndex);
                    keyIndex++;

                    TObject * elObj = key ? key->ReadObj() : nullptr;
                    if (!elObj)
                    {
                        TString nm( QString("Corrupted_%1_elem%2").arg(name).arg(iEl).toLatin1().data() );
                        elObj = new TNamed(nm, nm);
                        qWarning() << "Corrupted TKey in basket file" << fileName << " list object:" << name << "element" << iEl;
                    }
                    newList->Add(elObj);
                }
                tObj = newList; // never null: an empty/degenerate list is still a valid TList
            }
            else
            {
                TKey * key = (TKey*)f.GetListOfKeys()->At(keyIndex);
                keyIndex++;

                tObj = key->ReadObj();
            }

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
                en->SetObject(tObj); // will override the label
                en->SetLabel(text.toLatin1().data()); // so restore it
            }
        }

        if (!drawObjects.empty())
        {
            ABasketItem item;
            item.Name = itemName;

            item.Group = normalizePath(itemJson["Group"].toString());   // if missing in old files -> root
            ensureGroup(item.Group);

            item.DrawObjects = drawObjects;
            item.Type = drawObjects.front().Pointer->ClassName(); // "TList" if the first draw object is a list
            Basket.push_back(item);
            drawObjects.clear();
        }
    }

    if (TNamed * gdesc = dynamic_cast<TNamed*>(f.Get("BasketGroups")))   // absent in old files
    {
        const QJsonDocument gdoc = QJsonDocument::fromJson(QString(gdesc->GetTitle()).toLatin1());
        for (const QJsonValue & v : gdoc.array()) ensureGroup(normalizePath(v.toString()));
    }

    arrangeByTree();    // appended items may belong to existing groups

    f.Close();
    return "";
}

 // ----

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

/*
void ABasketManager::reorder(const std::vector<int> & indexes, int to)
{
    std::vector<ABasketItem> itemsToMove;
    for (size_t i = 0; i < indexes.size(); i++)
    {
        const int index = indexes[i];
        itemsToMove.push_back( Basket[index] );
        Basket[index]._toErase = true;
    }

    int toCopy = to;
    for (size_t i = 0; i < itemsToMove.size(); i++)
    {
        Basket.insert(Basket.begin() + to, itemsToMove[i]);
        to++;
    }

    for (int i = Basket.size()-1; i >= 0; i--)
        if (Basket[i]._toErase)
            Basket.erase(Basket.begin() + i);

    // creating mapping to update indexes of multidraw items
    std::vector<int> mapping;
    for (size_t i = 0; i < Basket.size(); i++) mapping.push_back(i);
    for (size_t iMoved = 0; iMoved < indexes.size(); iMoved++)
    {
        int indexMoved = indexes[iMoved];
        for (size_t i = 0; i < mapping.size(); i++)
        {
            if (mapping[i] == indexMoved)
            {
                mapping[i] = -1; // marker to be removed after insertion
                break;
            }
        }
    }
    for (size_t iMoved = 0; iMoved < indexes.size(); iMoved++)
    {
        int indexMoved = indexes[iMoved];
        mapping.insert(mapping.begin() + toCopy, indexMoved);
        toCopy++;
    }
    for (int i = mapping.size()-1; i >= 0; i--)
        if (mapping[i] == -1) mapping.erase(mapping.begin() + i);

    // inverting mapping array (will be newIndex[oldIndex])
    std::vector<size_t> newIndexMap;  // new[old], but mapping is the list of old indexes
    newIndexMap.resize(mapping.size());
    for (size_t iNewIndex = 0; iNewIndex < mapping.size(); iNewIndex++)  // similar to remove: [0,1,2,3,4] --> removed 1 and 3 --> mapping [0,2,4] --> old0 is 0; ol2 is 1; old 4 is 2
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
}
*/

void ABasketManager::reorder(const std::vector<int> & indexes, int to)
{
    const int n = static_cast<int>(Basket.size());
    if (indexes.empty() || n == 0) return;

    // Validate the insertion position.
    if (to < 0 || to > n) return;

    // Sort and validate the selected indices.
    // Sorting also guarantees that moved items retain their original order.
    std::vector<int> moved = indexes;
    std::sort(moved.begin(), moved.end());

    if (moved.front() < 0 || moved.back() >= n) return;

    if (std::adjacent_find(moved.begin(), moved.end()) != moved.end())
        return; // Duplicate indices are not allowed.

    // Mark the items to be moved.
    std::vector<bool> selected(n, false);
    for (int index : moved)
        selected[index] = true;

    // Construct the order of all items that remain in the basket.
    // order[newPosition] = oldIndex.
    std::vector<int> order;
    order.reserve(n);

    for (int i = 0; i < n; ++i)
        if (!selected[i])
            order.push_back(i);

    // The original insertion position must be adjusted because
    // selected items before 'to' will have been removed.
    int adjustedTo = to;
    for (int index : moved)
    {
        if (index < to)
            --adjustedTo;
        else
            break;
    }

    // Insert the moved items at the adjusted position.
    order.insert(order.begin() + adjustedTo, moved.begin(), moved.end());

    // Reorder the basket and update all multidraw references.
    applyOrder(order);
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

// ------------ Groups -------------

QString ABasketManager::joinPath(const QString & parent, const QString & name)
{
    return parent.isEmpty() ? name : parent + '/' + name;
}

QString ABasketManager::parentPath(const QString & path)
{
    const int i = path.lastIndexOf('/');
    return (i < 0) ? QString() : path.left(i);
}

QString ABasketManager::leafName(const QString & path)
{
    return path.mid(path.lastIndexOf('/') + 1);
}

QString ABasketManager::normalizePath(const QString & path)
{
    return path.split('/', Qt::SkipEmptyParts).join('/');
}

bool ABasketManager::isInsideGroup(const QString & path, const QString & group)
{
    if (group.isEmpty()) return true;
    return path == group || path.startsWith(group + '/');
}

QString ABasketManager::checkGroupName(const QString & name)
{
    if (name.trimmed().isEmpty()) return "Group name cannot be empty";
    if (name.contains('/'))       return "Group name cannot contain '/'";
    return "";
}

QString ABasketManager::getGroup(int index) const
{
    if (index < 0 || index >= (int)Basket.size()) return "";
    return Basket[index].Group;
}

bool ABasketManager::groupExists(const QString & path) const
{
    if (path.isEmpty()) return true;
    return std::find(Groups.begin(), Groups.end(), path) != Groups.end();
}

std::vector<int> ABasketManager::getItemsInGroup(const QString & path, bool recursive) const
{
    std::vector<int> res;
    for (int i = 0; i < (int)Basket.size(); i++)
        if (recursive ? isInsideGroup(Basket[i].Group, path) : Basket[i].Group == path)
            res.push_back(i);
    return res;
}

bool ABasketManager::countGroups() const
{
    return Groups.size();
}

void ABasketManager::ensureGroup(const QString & path)
{
    QString cur;
    for (const QString & seg : path.split('/', Qt::SkipEmptyParts))
    {
        cur = joinPath(cur, seg);
        if (!groupExists(cur)) Groups.push_back(cur);
    }
}

QString ABasketManager::addGroup(const QString & path)
{
    const QString err = checkGroupName(leafName(path));
    if (!err.isEmpty()) return err;
    if (!groupExists(parentPath(path))) return "Parent group does not exist";
    if (groupExists(path)) return "Group already exists";
    Groups.push_back(path);
    return "";
}

void ABasketManager::replaceGroupPrefix(const QString & oldPath, const QString & newPath)
{
    auto fix = [&](QString & g) { if (isInsideGroup(g, oldPath)) g = newPath + g.mid(oldPath.size()); };
    for (QString & g : Groups) fix(g);
    for (ABasketItem & it : Basket) fix(it.Group);

    normalizeGroups();
}

QString ABasketManager::renameGroup(const QString & path, const QString & newName)
{
    if (path.isEmpty() || !groupExists(path)) return "Group does not exist";
    const QString err = checkGroupName(newName);
    if (!err.isEmpty()) return err;
    const QString newPath = joinPath(parentPath(path), newName);
    if (newPath == path) return "";
    if (groupExists(newPath)) return "A group with this name already exists here";
    replaceGroupPrefix(path, newPath);
    return "";
}

QString ABasketManager::moveGroup(const QString & path, const QString & newParentPath)
{
    if (path.isEmpty() || !groupExists(path)) return "Group does not exist";
    if (!groupExists(newParentPath))          return "Target group does not exist";
    if (isInsideGroup(newParentPath, path))   return "Cannot move a group into itself";
    const QString newPath = joinPath(newParentPath, leafName(path));
    if (newPath == path) return "";
    if (groupExists(newPath)) return "Target already contains a group with this name";

    //replaceGroupPrefix(path, newPath);
    //return "";

    replaceGroupPrefix(path, newPath);
    arrangeByTree();      // flat indexes change
    return "";
}

QString ABasketManager::removeGroup(const QString & path)
{
    if (path.isEmpty() || !groupExists(path)) return "Group does not exist";
    const QString parent = parentPath(path);

    auto fix = [&](QString & g)
    {
        if (g == path) g = parent;
        else if (g.startsWith(path + '/')) g = joinPath(parent, g.mid(path.size() + 1));
    };
    for (ABasketItem & it : Basket) fix(it.Group);
    for (QString & g : Groups) fix(g);

    std::vector<QString> cleaned;   // drop root and duplicates (possible if subgroup names collide with the parent's)
    for (const QString & g : Groups)
        if (!g.isEmpty() && std::find(cleaned.begin(), cleaned.end(), g) == cleaned.end())
            cleaned.push_back(g);
    Groups = cleaned;

    arrangeByTree();

    return "";
}

QString ABasketManager::removeGroupAndContents(const QString & path)
{
    if (path.isEmpty() || !groupExists(path)) return "Group does not exist";

    const std::vector<int> indexes = getItemsInGroup(path, true);
    if (!indexes.empty())
    {
        const QString err = remove(indexes);
        if (!err.isEmpty()) return err;
    }
    Groups.erase(std::remove_if(Groups.begin(), Groups.end(),
                                [&](const QString & g){ return isInsideGroup(g, path); }),
                 Groups.end());
    return "";
}

/*
QString ABasketManager::moveItems(const std::vector<int> & indexes, const QString & targetGroup, int toIndex)
{
    if (!groupExists(targetGroup)) return "Target group does not exist";
    if (indexes.empty()) return "";
    for (int i : indexes)
        if (i < 0 || i >= (int)Basket.size()) return "Invalid basket index";

    for (int i : indexes) Basket[i].Group = targetGroup;

    int to = toIndex;
    if (to < 0 || to > (int)Basket.size())   // append after the last item of the target group
    {
        to = 0;
        for (int i = 0; i < (int)Basket.size(); i++)
            if (Basket[i].Group == targetGroup) to = i + 1;
    }
    reorder(indexes, to);    // also fixes multidraw indexes
    return "";
}
*/

QString ABasketManager::moveItems(const std::vector<int> & indexes, const QString & targetGroup, int toIndex)
{
    if (!groupExists(targetGroup)) return "Target group does not exist";
    if (indexes.empty()) return "";
    for (int i : indexes)
        if (i < 0 || i >= (int)Basket.size()) return "Invalid basket index";

    int to = toIndex;
    if (to < 0 || to > (int)Basket.size())
    {
        // "end of the group": after the last item that is ALREADY in the target group.
        // Must be computed before reassigning groups and must ignore the items being moved.
        to = -1;
        for (int i = 0; i < (int)Basket.size(); i++)
        {
            if (std::binary_search(indexes.begin(), indexes.end(), i)) continue;   // indexes are ascending
            if (Basket[i].Group == targetGroup) to = i + 1;
        }
        if (to < 0) to = indexes.front();    // empty group: no neighbors, keep the position
    }

    for (int i : indexes) Basket[i].Group = targetGroup;

    reorder(indexes, to);     // puts the items right next to the requested neighbor, fixes multidraw indexes
    arrangeByTree();          // groups the flat list: every group becomes a contiguous block
    return "";
}

void ABasketManager::normalizeGroups()
{
    std::vector<QString> res;
    for (const QString & g : Groups)
    {
        QString cur;
        for (const QString & seg : g.split('/', Qt::SkipEmptyParts))
        {
            cur = joinPath(cur, seg);
            if (std::find(res.begin(), res.end(), cur) == res.end()) res.push_back(cur);
        }
    }
    Groups = res;
}

void ABasketManager::applyOrder(const std::vector<int> & order)
{
    std::vector<ABasketItem> sorted;
    sorted.reserve(order.size());
    std::vector<int> newIndexOf(order.size());
    for (int n = 0; n < (int)order.size(); n++)
    {
        sorted.push_back(Basket[order[n]]);   // shallow copy; ABasketItem's destructor does not delete objects
        newIndexOf[order[n]] = n;
    }
    Basket.swap(sorted);

    for (ABasketItem & item : Basket)
    {
        if (item.DrawObjects.empty() || !item.DrawObjects.front().Multidraw) continue;
        for (int & i : item.DrawObjects.front().MultidrawSettings.BasketItems)
            i = newIndexOf[i];
    }
}

void ABasketManager::arrangeByTree()
{
    std::vector<int>  order;
    std::vector<bool> used(Basket.size(), false);

    std::function<void(const QString &)> visit = [&](const QString & group)
    {
        for (const QString & g : Groups)                 // subgroups first, in creation order (as in the widget)
            if (parentPath(g) == group) visit(g);
        for (int i = 0; i < (int)Basket.size(); i++)     // then own items, in current relative order
            if (!used[i] && Basket[i].Group == group) { order.push_back(i); used[i] = true; }
    };
    visit("");

    for (int i = 0; i < (int)Basket.size(); i++)         // safety net: items with an unregistered group
        if (!used[i]) order.push_back(i);

    bool changed = false;
    for (int n = 0; n < (int)order.size(); n++) if (order[n] != n) { changed = true; break; }
    if (changed) applyOrder(order);
}


