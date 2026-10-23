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

QString ABasketManager::addGroup(const QString & name, const QString & parentGroup, int iBasketIndex)
{
    if (iBasketIndex != -1)
    {
        if (iBasketIndex < 0 || iBasketIndex >= Basket.size())
            return "Invalid insert position";
    }

    int parentIndex = -1;
    if (!parentGroup.isEmpty())
    {
        parentIndex = findGroup(parentGroup);
        if (parentIndex == -1) return "Parent group not found";
    }

    if (iBasketIndex != -1)
    {
        // check insert position is compatible with the parentGroup
        bool valid = (Basket[iBasketIndex].ParentGroup == parentGroup);
        if (!valid)
        {
            // another possibility: it is the group item itself
            if (Basket[iBasketIndex].isGroup)
            {
                QString name = Basket[iBasketIndex].ParentGroup;
                if (!name.isEmpty()) name += "/";
                name += Basket[iBasketIndex].Name;
                if (name == parentGroup) valid = true;
            }
        }
        if (!valid) return "Insertion position is not compatibe with the requested parent group";
    }

    ABasketItem item;
    item.Name = name;
    item.ParentGroup = parentGroup;
    item.isGroup = true;
    item.Collapsed = false;

    Basket.push_back(item);

    std::vector<int> indexes;
    indexes.push_back(Basket.size() - 1);

    if (iBasketIndex == -1) // move to the end of the corresponding group --> script uses this approach
    {
        if (parentIndex != -1)
        {
            // find the first item not belonging to the parent group, otherwise keep it last
            for (int iTo = parentIndex + 1; iTo < Basket.size(); iTo++)
            {
                if (Basket[iTo].ParentGroup != parentGroup)
                {
                    reorder(indexes, iTo);
                    return "";
                }
            }
        }
        // if reach this point, no need to move
    }
    else
    {
        if (Basket[iBasketIndex].isGroup) iBasketIndex++; // normally adding above, but for a group it has to be the next item
        if (iBasketIndex < Basket.size()-1) reorder(indexes, iBasketIndex); // else already there
    }

    return "";
}

QStringList ABasketManager::getGroupNames() const
{
    QStringList sl = {QString()};

    for (const ABasketItem & item : Basket)
    {
        if (!item.isGroup) continue;

        QString name;
        if (!item.ParentGroup.isEmpty()) name = item.ParentGroup + "/";
        name += item.Name;
        sl << name;
    }
    return sl;
}

int ABasketManager::findGroup(const QString & name)
{
    for (int i = 0; i < Basket.size(); ++i)
    {
        if (!Basket[i].isGroup) continue;

        QString thisName = Basket[i].ParentGroup;
        if (!thisName.isEmpty()) thisName += "/";
        thisName += Basket[i].Name;
        if (thisName == name) return i;
    }
    return -1;
}



QString ABasketManager::joinPath(const QString & parentGroup, const QString & thisGroupName)
{
    return parentGroup.isEmpty() ? thisGroupName : parentGroup + '/' + thisGroupName;
}

bool ABasketManager::groupExists(const QString & fullPath) const
{
    for (const ABasketItem & item : Basket)
    {
        if (!item.isGroup) continue;
        if (item.getGroupFullPath() == fullPath) return true;
    }
    return false;
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

QString ABasketManager::removeGroupAndContents(int index)
{
    if (index < 0 || index >= Basket.size()) return "Invalid index";
    if (!Basket[index].isGroup) return "This is not a group";

    std::vector<int> itemIndexes;
    getIndexes_ItemsInGroupRecursive(index, itemIndexes);
    return remove(itemIndexes);
}

QString ABasketManager::removeGroupButKeepContents(int index)
{
    if (index < 0 || index >= (int)Basket.size()) return "Invalid index";
    if (!Basket[index].isGroup) return "This is not a group";

    std::vector<int> indexes;
    getIndexes_ItemsInGroupRecursive(index, indexes);
    // indexes[0] is the group itself, the rest are all its descendants (groups and items)
    Q_ASSERT(!indexes.empty() && indexes[0] == index);

    const QString from = Basket[index].getGroupFullPath(); // e.g. "A1/A2/A3"
    const QString to   = Basket[index].ParentGroup;        // e.g. "A1/A2" ("" at root)

    // Every descendant's ParentGroup is either 'from' or starts with 'from/'
    // Its new parent is the same path with the removed group's segment stripped out.
    auto newParentOf = [&](const QString & oldParent) -> QString
    {
        const QString suffix = oldParent.mid(from.size()); // "" or "/X/Y"
        return to.isEmpty() ? suffix.mid(1) : to + suffix;
    };

    // true if path is the removed group or one of its descendants
    auto inRemovedSubtree = [&](const QString & path)
    {
        return path == from || path.startsWith(from + '/');
    };

    // Check for conflicting group names
    // Groups inside the removed subtree all move, so they can't block each other;
    // only groups outside the subtree (which stay in place) can cause a conflict.
    for (size_t i = 1; i < indexes.size(); i++)
    {
        const auto & it = Basket[indexes[i]];
        if (!it.isGroup) continue;

        const QString newFull = joinPath(newParentOf(it.ParentGroup), it.Name);
        if (!inRemovedSubtree(newFull) && groupExists(newFull))
            return QString("Removing this group would result in conflicting group names: see group %1").arg(newFull);
    }

    // Save the descendants' indices, translated to their positions after erase()
    std::vector<int> descendants;
    descendants.reserve(indexes.size() - 1);
    for (size_t i = 1; i < indexes.size(); i++)
        descendants.push_back(indexes[i] > index ? indexes[i] - 1 : indexes[i]); // paranoid :) they should be >

    // Attempt to remove the group itself (assuming it can fail, in the current implementation should not happen, as it tests only for multiplots items)
    const QString error = remove(std::vector<int>{index});
    if (!error.isEmpty()) return error; // nothing has been modified yet

    // Re-parent groups and items
    for (int d : descendants)
        Basket[d].ParentGroup = newParentOf(Basket[d].ParentGroup);

    return "";
}

void ABasketManager::getIndexes_ItemsInGroupRecursive(int indexOfGroup, std::vector<int> & indexes) const
{
    if (indexOfGroup < 0 || indexOfGroup >= Basket.size()) return;
    if (!Basket[indexOfGroup].isGroup) return;

    indexes.push_back(indexOfGroup); // the group item itself
    QString fullPath_GroupToRemove = Basket[indexOfGroup].getGroupFullPath();
    for (int i = 0; i < (int)Basket.size(); i++)
    {
        if (Basket[i].ParentGroup == fullPath_GroupToRemove)
        {
            indexes.push_back(i);
            if (Basket[i].isGroup) getIndexes_ItemsInGroupRecursive(i, indexes);
        }
    }
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

/*
QStringList ABasketManager::getItemNames() const
{
    QStringList res;
    for (const ABasketItem & item : Basket)
        res << item.Name;
    return res;
}
*/

QString ABasketManager::getGroupFullPath(int index)
{
    if (index < 0 || index >= Basket.size()) return "";
    if (!Basket[index].isGroup) return "";

    return Basket[index].ParentGroup.isEmpty() ? Basket[index].Name : Basket[index].ParentGroup + '/' + Basket[index].Name;
}

void ABasketManager::setCollapsedFlag(int index, bool flag)
{
    if (index < 0 || index >= Basket.size()) return;
    Basket[index].Collapsed = flag;
    qDebug() << "BM-->" << Basket[index].Name << Basket[index].Collapsed;
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

bool ABasketManager::isGroup(int index) const
{
    if (index < 0 || index >= (int)Basket.size()) return false;
    return Basket[index].isGroup;
}

QString ABasketManager::getParentGroup(int index) const
{
    if (index < 0 || index >= (int)Basket.size()) return "";
    return Basket[index].ParentGroup;
}

/*
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

        // Group-related
        itemJson["ParentGroup"] = Basket[ib].ParentGroup;
        itemJson["IsGroup"] = Basket[ib].isGroup;
        itemJson["Collapsed"] = Basket[ib].Collapsed;

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

    TNamed desc;
    desc.SetTitle(descStr.toLocal8Bit().data());
    desc.Write("BasketDescription_v2");

    basketFile.Close();
}
*/

#include <numeric>
void ABasketManager::saveBasket(const QString & fileName)
{
    std::vector<int> indexes(Basket.size());
    std::iota(indexes.begin(), indexes.end(), 0);

    writeBasketFile(fileName, indexes);
}

QString ABasketManager::saveGroup(int index, const QString & fileName)
{
    if (index < 0 || index >= (int)Basket.size()) return "Invalid index";
    if (!Basket[index].isGroup) return "This is not a group";

    // indexes[0] is the group itself, the rest are all its descendants (groups and items)
    std::vector<int> indexes;
    getIndexes_ItemsInGroupRecursive(index, indexes);
    Q_ASSERT(!indexes.empty() && indexes[0] == index);

    const QString error = checkMultidrawsAreComplete(indexes);
    if (!error.isEmpty()) return error;

    // keep the original relative order of the entries, as in a full save
    std::sort(indexes.begin(), indexes.end());

    return writeBasketFile(fileName, indexes, index);
}

QString ABasketManager::writeBasketFile(const QString & fileName, const std::vector<int> & indexes, int rootGroupIndex)
{
    TFile basketFile(fileName.toLocal8Bit().data(), "RECREATE");
    if (basketFile.IsZombie()) return "Cannot open file for writing: " + fileName;

    // Rebasing for single-group export:
    // the exported group gets an empty ParentGroup; for the descendants the parent path
    // of the exported group (and its trailing '/') is stripped from the beginning.
    const bool rebase = (rootGroupIndex >= 0);
    const QString exportedParent = rebase ? Basket[rootGroupIndex].ParentGroup : QString();
    const int stripLength = exportedParent.isEmpty() ? 0 : exportedParent.size() + 1;

    int objectIndex = 0;
    QJsonArray basketJAr;
    for (int ib : indexes)
    {
        QJsonObject itemJson;
        itemJson["ItemName"] = Basket[ib].Name;

        // Group-related
        QString parentGroup = Basket[ib].ParentGroup;
        if (rebase)
        {
            if (ib == rootGroupIndex) parentGroup.clear();
            else                      parentGroup = parentGroup.mid(stripLength);
        }
        itemJson["ParentGroup"] = parentGroup;
        itemJson["IsGroup"] = Basket[ib].isGroup;
        itemJson["Collapsed"] = Basket[ib].Collapsed;

        QJsonArray itemArray;
        const std::vector<ADrawObject> & drawObjects = Basket[ib].DrawObjects;
        for (size_t io = 0; io < drawObjects.size(); io++)
        {
            const ADrawObject & obj = drawObjects[io];

            QJsonObject js;
            obj.writeToJson(js, (rebase ? rootGroupIndex : 0)); // entrire bakset: normal save, for multidar, indexes are adjusted, check that all members are present ar in th ecaller

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

    TNamed desc;
    desc.SetTitle(descStr.toLocal8Bit().data());
    desc.Write("BasketDescription_v2");

    basketFile.Close();
    return "";
}

QString ABasketManager::checkMultidrawsAreComplete(const std::vector<int> & indexes)
{
    std::vector<bool> inSubset(Basket.size(), false);
    for (int i : indexes) inSubset[i] = true;

    for (int m : indexes)
    {
        if (!isMultidraw(m)) continue;

        // Every member of this multidraw must be in the subset.
        // It is enough to scan the entries outside of it: a member found there is a violation.
        for (int j = 0; j < (int)Basket.size(); j++)
        {
            if (inSubset[j]) continue;
            if (isMemberOfSpecificMultidraw(j, m))
                return QString("Cannot save the group: multidraw item '%1' has a member '%2' which is not in the group")
                    .arg(Basket[m].Name).arg(joinPath(Basket[j].ParentGroup, Basket[j].Name));
        }
    }
    return "";
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

        //if (!drawObjects.empty())
        {
            ABasketItem item;
            item.Name = itemJson["ItemName"].toString();

            // Group-related
            jstools::parseJson(itemJson, "ParentGroup", item.ParentGroup);
            jstools::parseJson(itemJson, "IsGroup", item.isGroup);
            jstools::parseJson(itemJson, "Collapsed", item.Collapsed);

            item.DrawObjects = drawObjects;
            if (!drawObjects.empty())
                item.Type = drawObjects.front().Pointer->ClassName(); // "TList" if the first draw object is a list

            Basket.push_back(item);
            drawObjects.clear();
        }
    }

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

QString ABasketManager::checkGroupCanBeMovedToIndex(int indexOfGroup, int indexTo)
{
    if (indexOfGroup == indexTo) return ""; // nothing to do

    if (indexOfGroup < 0 || indexOfGroup >= (int)Basket.size()) return "Bad group index";
    if (indexTo < 0 || indexTo >= (int)Basket.size()) return "Bad target index"; // !!!*** maybe need to allow last!

    // !!!*** not ready to handle group reorder!
    const QString originalPath = Basket[indexOfGroup].getGroupFullPath();
    QString newPath = joinPath(Basket[indexTo].getGroupFullPath(), Basket[indexOfGroup].Name);
    if (originalPath == newPath) return ""; // remains in the same parent group (or root)

    if (groupExists(newPath)) return "Moving the group would result in group name conflict";

    return "";
}

void ABasketManager::addGroupMembersRecursive(int itemIndex, std::vector<int> & indexes)
{
    if (!Basket[itemIndex].isGroup) return;

    QString groupPath = Basket[itemIndex].getGroupFullPath();

    // Find all direct children in Basket, preserving their order
    std::vector<int> children;
    for (int i = 0; i < (int)Basket.size(); i++)
        if (Basket[i].ParentGroup == groupPath)
            children.push_back(i);

    // Insert children immediately after the parent group
    auto pos = std::find(indexes.begin(), indexes.end(), itemIndex);
    if (pos == indexes.end()) return;

    pos = indexes.insert(pos + 1, children.begin(), children.end());

    // Recursively expand child groups
    for (int childIndex : children)
    {
        if (Basket[childIndex].isGroup)
            addGroupMembersRecursive(childIndex, indexes);
    }
}

#include <QString>
#include <QHash>
#include <QSet>
#include <QVector>
#include <algorithm>

void ABasketManager::updateParentGroup(int indexTo, std::vector<int> & indexes)
{
    // 1. Determine the destination parent path.
    const QString newPath = Basket[indexTo].isGroup ? Basket[indexTo].getGroupFullPath() : Basket[indexTo].ParentGroup;

    // 2. Snapshot original parent paths and full group paths.
    QHash<int, QString> originalParents;
    QHash<int, QString> originalGroupPaths;

    for (int i = 0; i < (int)Basket.size(); ++i)
    {
        originalParents.insert(i, Basket[i].ParentGroup);

        if (Basket[i].isGroup)
        {
            const QString fullPath = Basket[i].ParentGroup.isEmpty() ? Basket[i].Name : Basket[i].ParentGroup + "/" + Basket[i].Name;
            originalGroupPaths.insert(i, fullPath);
        }
    }

    // 3. Identify the top-level entries being moved.
    // Descendants of another explicitly moved group are not moved independently.
    QSet<int> movedIndexes;
    for (int index : indexes) movedIndexes.insert(index);

    QVector<int> topLevelIndexes;

    for (int index : indexes)
    {
        const QString parentPath = originalParents[index];

        bool hasMovedAncestor = false;

        for (int otherIndex : indexes)
        {
            if (otherIndex == index || !Basket[otherIndex].isGroup) continue;
            const QString ancestorPath = originalGroupPaths[otherIndex];
            if (parentPath == ancestorPath || parentPath.startsWith(ancestorPath + "/"))
            {
                hasMovedAncestor = true;
                break;
            }
        }

        if (!hasMovedAncestor) topLevelIndexes.push_back(index);
    }

    // 4. Build path transformations for top-level moved groups.
    struct PathMove
    {
        QString oldPath;
        QString newPath;
    };

    QVector<PathMove> pathMoves;

    for (int index : topLevelIndexes)
    {
        if (!Basket[index].isGroup) continue;
        const QString oldPath = originalGroupPaths[index];
        const QString newGroupPath = newPath.isEmpty() ? Basket[index].Name : newPath + "/" + Basket[index].Name;
        pathMoves.push_back({oldPath, newGroupPath});
    }

    // 5. Update all items exactly once using their original paths.
    for (int i = 0; i < (int)Basket.size(); ++i)
    {
        const QString oldParent = originalParents[i];

        // Check whether this item belongs to a moved group.
        int bestMatch = -1;

        for (int j = 0; j < pathMoves.size(); ++j)
        {
            const QString& oldGroupPath = pathMoves[j].oldPath;

            if (oldParent == oldGroupPath || oldParent.startsWith(oldGroupPath + "/"))
            {
                if (bestMatch == -1 || oldGroupPath.size() > pathMoves[bestMatch].oldPath.size())
                    bestMatch = j;
            }
        }

        if (bestMatch != -1)
        {
            const PathMove& move = pathMoves[bestMatch];
            QString suffix = oldParent.mid(move.oldPath.size());
            if (suffix.startsWith('/')) suffix.remove(0, 1);
            Basket[i].ParentGroup = move.newPath;
            if (!suffix.isEmpty()) Basket[i].ParentGroup += "/" + suffix;
        }
    }

    // 6. Update explicitly moved top-level items and groups.
    // Top-level groups were handled above; their parent path is the destination.
    for (int index : topLevelIndexes)
        Basket[index].ParentGroup = newPath;
}

QString ABasketManager::reorder(std::vector<int> & indexes, int indexTo)
{
    // currently a limited implementation: do not allow:
    // 1. mix of items and groups
    // 2. more than one group

    int numGroups = 0;
    int numItems = 0;
    for (size_t i = 0; i < indexes.size(); i++)
    {
        const int index = indexes[i];
        if (Basket[index].isGroup) numGroups++;
        else numItems++;
    }
    if (numGroups > 0 && numItems > 0) return "Cannot move a mixture of groups and items";
    if (numGroups > 1) return "Cannot move more than one group";

    // check group can be moved to the index
    if (numGroups == 1) // current limit!
    {
        QString err = checkGroupCanBeMovedToIndex(indexes.front(), indexTo);
        if (!err.isEmpty()) return err;
    }

    // adding indexes of all members of the group
    if (numGroups == 1)
        addGroupMembersRecursive(indexes.front(), indexes);

    // updating ParentGroup info
    updateParentGroup(indexTo, indexes);
    /*
    const QString originalPath = Basket[indexes.front()].ParentGroup;
    QString newPath = (Basket[indexTo].isGroup ? Basket[indexTo].getGroupFullPath() : Basket[indexTo].ParentGroup);
    for (int index : indexes)
    {
        qDebug() << "item"<< Basket[index].Name << "had " << Basket[index].ParentGroup;
        Basket[index].ParentGroup = newPath + Basket[index].ParentGroup.mid(originalPath.size());
        qDebug() << "--> now" << Basket[index].ParentGroup;
    }
    */

    std::vector<ABasketItem> itemsToMove;
    for (size_t i = 0; i < indexes.size(); i++)
    {
        const int index = indexes[i];
        itemsToMove.push_back( Basket[index] );
        Basket[index]._toErase = true;
    }

    int indexTo_Copy = indexTo;
    for (size_t i = 0; i < itemsToMove.size(); i++)
    {
        Basket.insert(Basket.begin() + indexTo, itemsToMove[i]);
        indexTo++;
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
        mapping.insert(mapping.begin() + indexTo_Copy, indexMoved);
        indexTo_Copy++;
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

    return "";
}

void ABasketManager::moveLastBasketItemToGroup(const QString & parentGroup, bool toBottom)
{
    if (Basket.empty()) return;

    std::vector<int> indexes;
    indexes.push_back(Basket.size() - 1);

    if (parentGroup.isEmpty())
    {
        Basket.back().ParentGroup.clear(); // paranoid
        if (toBottom) return; // already at the right place
        reorder(indexes, 0);
        return;
    }

    int parentIndex = findGroup(parentGroup);
    if (parentIndex == -1)
    {
        qWarning() << "Group" << parentGroup << "not found, basket move canceled!";
        return;
    }

    Basket.back().ParentGroup = parentGroup;

    if (!toBottom)
    {
        reorder(indexes, parentIndex + 1);
        return;
    }

    // find the first item not belonging to the parent group, otherwise keep it last
    for (int iTo = parentIndex + 1; iTo < Basket.size(); iTo++)
    {
        if (Basket[iTo].ParentGroup != parentGroup)
        {
            reorder(indexes, iTo);
            return;
        }
    }
    // else already the last
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

QString ABasketManager::checkGroupName(const QString & name)
{
    if (name.trimmed().isEmpty()) return "Group name cannot be empty";
    if (name.contains('/'))       return "Group name cannot contain '/'";
    return "";
}

QString ABasketManager::renameGroup(int index, const QString & newName)
{
    if (index < 0 || index >= Basket.size()) return "Invalid basket index";
    if (!Basket[index].isGroup) return "Selected item is not a group";
    if (newName == Basket[index].Name) return "New name is the same as old";
    const QString err = checkGroupName(newName);
    if (!err.isEmpty()) return err;

    const QString newPath = joinPath(Basket[index].ParentGroup, newName);
    if (groupExists(newPath)) return "A group with this name conflicts with existing ones";

    const QString oldPath = Basket[index].getGroupFullPath();
    Basket[index].Name = newName;
    replaceGroupPrefix(oldPath, newPath);
    return "";
}

void ABasketManager::replaceGroupPrefix(const QString & oldPath, const QString & newPath)
{
    //qDebug() << "--------->" << oldPath << "to" << newPath;
    for (ABasketItem & it : Basket)
    {
        //qDebug() << "item"<< it.Name << "had " << it.ParentGroup;
        if (isInsideGroup(it.ParentGroup, oldPath))
            it.ParentGroup = newPath + it.ParentGroup.mid(oldPath.size());
        //qDebug() << "--> now" << it.ParentGroup;
    }
}

bool ABasketManager::isInsideGroup(const QString & path, const QString & group) const
{
    if (group.isEmpty()) return true;
    return path == group || path.startsWith(group + '/');
}
