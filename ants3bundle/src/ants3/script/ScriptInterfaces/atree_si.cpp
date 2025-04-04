#include "atree_si.h"
#include "aroottreerecord.h"
#include "ascriptobjstore.h"

#include <QDebug>

//#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
//#include "TLeaf.h"

ATree_SI::ATree_SI() : TmpHub(AScriptObjStore::getInstance())
{
    Description = "Interface to CERN ROOT trees";

    Help["loadTree"] = "If the third argument is not provided, the first tree found in the file is loaded";
    Help["newTree"] = "HeadersOfBranches:  array of [Name,BranchType] values\n"
                      "Avaiable BranchTypes:\n"
                      "C - string\n"
                      "I - int\n"
                      "F - float\n"
                      "D - double\n"
                      "O - bool\n"
                      "AC - vector of strings\n"
                      "AI - vector of ints\n"
                      "AF - vector of floats\n"
                      "AD - vector of doubles\n"
                      "AO - vector of bools";
}

void ATree_SI::loadTree(QString TreeName, QString FileName, QString TreeNameInFile)
{
    if (!bGuiThread)
    {
        abort("Can load TTree only in main thread!");
        return;
    }

    ARootTreeRecord * rec = new ARootTreeRecord(0, TreeName);
    const QString ErrorString = rec->loadTree(FileName, TreeNameInFile);
    if (ErrorString.isEmpty())
    {
        bool bOK = TmpHub.Trees.append(TreeName, rec, bAbortIfExists);
        if (!bOK)
        {
            delete rec;
            abort("Tree " + TreeName + " already exists!");
        }
    }
    else abort("Failed to create tree "+ TreeName + ": " + ErrorString);
}

void ATree_SI::newTree(QString TreeName, QVariantList HeadersOfBranches,
                       QString StoreInFileName, int AutosaveAfterEntriesAdded)
{
    if (!bGuiThread)
    {
        abort("Can load TTree only in main thread!");
        return;
    }

    if ( isTreeExists(TreeName) )
    {
        if (bAbortIfExists)
        {
            abort("Tree " + TreeName + " already exists!");
            return;
        }
        deleteTree(TreeName); // need to delete first -> cannot just replace because of save to file root mechanism
    }

    if (HeadersOfBranches.size() < 1)
    {
        abort("CreateTree() requires array of arrays as the second argument");
        return;
    }

    std::vector<std::pair<QString, QString>> h;
    for (int ibranch = 0; ibranch < HeadersOfBranches.size(); ibranch++)
    {
        QVariantList th = HeadersOfBranches.at(ibranch).toList();
        if (th.size() != 2)
        {
            abort("CreateTree() headers should be array of [Name,Type] values");
            return;
        }

        QString Bname = th.at(0).toString();
        QString Btype = th.at(1).toString();
        if (!ABranchBuffer::isValidType(Btype))
        {
            abort("CreateTree() header contain unknown branch type: " + Btype );
            return;
        }

        h.push_back( std::pair<QString, QString>(Bname, Btype) );
    }

    ARootTreeRecord* rec = new ARootTreeRecord(nullptr, TreeName);
    bool bOK = rec->createTree(TreeName, h, StoreInFileName, AutosaveAfterEntriesAdded);
    if (bOK)
    {
        bOK = TmpHub.Trees.append(TreeName, rec, bAbortIfExists);
        if (!bOK) // paranoic - should not happen
        {
            delete rec;
            abort("Tree " + TreeName + " already exists!");
        }
    }
    else abort("Failed to create tree: " + TreeName);
}

void ATree_SI::fill(QString TreeName, QVariantList Array)
{
    ARootTreeRecord * r = dynamic_cast<ARootTreeRecord*>(TmpHub.Trees.getRecord(TreeName));
    if (!r) abort("Tree " + TreeName + " not found!");
    else
    {
        bool bOK = r->fillSingle(Array);
        if (!bOK) abort("Fill failed");
    }
}

int ATree_SI::getNumEntries(QString TreeName)
{
    ARootTreeRecord* r = dynamic_cast<ARootTreeRecord*>(TmpHub.Trees.getRecord(TreeName));
    if (!r)
        return 0;
    else
        return r->countEntries();
}

QVariantList ATree_SI::getBranchNames(QString TreeName)
{
    QVariantList vl;
    ARootTreeRecord* r = dynamic_cast<ARootTreeRecord*>(TmpHub.Trees.getRecord(TreeName));
    if (r)
    {
        const QStringList sl = r->getBranchNames();
        for (const QString & s : sl) vl << s;
    }
    return vl;
}

QVariantList ATree_SI::getBranchTypes(QString TreeName)
{
    QVariantList vl;
    ARootTreeRecord* r = dynamic_cast<ARootTreeRecord*>(TmpHub.Trees.getRecord(TreeName));
    if (r)
    {
        const QStringList sl = r->getBranchTypes();
        for (const QString & s : sl) vl << s;
    }
    return vl;
}

QVariantList ATree_SI::getAllTreeNames()
{
    const QStringList sl = TmpHub.Trees.getAllRecordNames();

    QVariantList vl;
    for (const QString & s : sl) vl << s;
    return vl;
}

bool ATree_SI::isTreeExists(QString TreeName)
{
    ARootTreeRecord* r = dynamic_cast<ARootTreeRecord*>(TmpHub.Trees.getRecord(TreeName));
    return r;
}

void ATree_SI::resetTreeAddresses(QString TreeName)
{
    ARootTreeRecord* r = dynamic_cast<ARootTreeRecord*>(TmpHub.Trees.getRecord(TreeName));
    if (!r)
        abort("Tree "+TreeName+" not found!");
    else
        r->resetTreeRecords();
}

/*
void AInterfaceToTTree::Scan(const QString& TreeName, const QString& arg1, const QString& arg2, const QString& arg3)
{
    ARootTreeRecord* r = dynamic_cast<ARootTreeRecord*>(TmpHub.Trees.getRecord(TreeName));
    if (!r)
        abort("Tree "+TreeName+" not found!");
    else
        r->scan(arg1, arg2, arg3);
}
*/

/*
const QString AInterfaceToTTree::GetTreeStructure(const QString& TreeName)
{
    ARootObjBase* r = TmpHub.Trees.getRecord(TreeName);
    if (!r)
    {
        abort("Tree " + TreeName + " not found!");
        return "";
    }
    TTree *t = static_cast<TTree*>(r->GetObject());

    QString s = "Thee ";
    s += TreeName;
    s += " has the following branches (-> data_type):<br>";
    for (int i=0; i<t->GetNbranches(); i++)
    {
        TObjArray* lb = t->GetListOfBranches();
        const TBranch* b = (const TBranch*)(lb->At(i));
        QString name = b->GetName();
        s += name;
        s += " -> ";
        QString type = b->GetClassName();
        if (type.isEmpty())
        {
            QString title = b->GetTitle();
            title.remove(name);
            title.remove("/");
            s += title;
        }
        else
        {
            type.replace("<", "(");
            type.replace(">", ")");
            s += type;
        }
        s += "<br>";
    }
    return s;
}
*/

QVariantList ATree_SI::getBranch(QString TreeName, QString BranchName)
{
    QVariantList res;
    ARootTreeRecord* r = dynamic_cast<ARootTreeRecord*>(TmpHub.Trees.getRecord(TreeName));
    if (!r)
        abort("Tree " + TreeName + " not found!");
    else
    {
        if (!r->isBranchExist(BranchName))
            abort("Tree " + TreeName + " does not have branch " + BranchName);
        else
            res = r->getBranch(BranchName);
    }
    return res;
}

QVariant ATree_SI::getBranch(QString TreeName, QString BranchName, int EntryIndex)
{
    ARootTreeRecord* r = dynamic_cast<ARootTreeRecord*>(TmpHub.Trees.getRecord(TreeName));
    if (!r)
        abort("Tree " + TreeName + " not found!");
    else
    {
        if (!r->isBranchExist(BranchName))
            abort("Tree " + TreeName + " does not have branch " + BranchName);
        else
            return r->getBranch(BranchName, EntryIndex);
    }
    return QVariantList();
}

QVariantList ATree_SI::getEntry(QString TreeName, int EntryIndex)
{
    ARootTreeRecord* r = dynamic_cast<ARootTreeRecord*>(TmpHub.Trees.getRecord(TreeName));
    if (!r)
    {
        abort("Tree " + TreeName + " not found!");
        return QVariantList();
    }
    else
        return r->getEntry(EntryIndex);
}

const QVariantList assertBinsAndRanges(const QVariant& in)
{
    QVariantList out;
    bool bOK;

    QVariantList inVL = in.toList();
    if (inVL.size() == 3)
    {
        int bins = inVL.at(0).toInt(&bOK);
        if (!bOK) bins = 100;
        double from = inVL.at(1).toDouble(&bOK); if (!bOK) from = 0;
        double to   = inVL.at(2).toDouble(&bOK); if (!bOK) to   = 0;
        out << bins << from << to;
    }
    else out << 100 << 0.0 << 0.0;
    return out;
}

const QVariantList assertMarkerLine(const QVariant& in)
{
    QVariantList out;
    bool bOK;

    QVariantList inVL = in.toList();
    if (inVL.size() == 3)
    {
        int color = inVL.at(0).toInt(&bOK);
        if (!bOK) color = 602;
        double style = inVL.at(1).toInt(&bOK);    if (!bOK) style = 1;
        double size  = inVL.at(2).toDouble(&bOK); if (!bOK) size  = 1.0;
        out << color << style << size;
    }
    else out << 602 << 1 << 1.0;
    return out;
}

QString ATree_SI::draw(QString TreeName, QString what, QString cuts, QString options,
                       QVariantList binsAndRanges, QVariantList markerAndLineAttributes, bool AbortIfFailedToDraw)
{
    if (!bGuiThread)
    {
        abort("Threads cannot draw!");
        return "";
    }

    ARootTreeRecord* r = dynamic_cast<ARootTreeRecord*>(TmpHub.Trees.getRecord(TreeName));
    if (!r)
    {
        abort("Tree "+TreeName+" not found!");
        return "";
    }
    else
    {
        QVariantList outBR;
        for (int i = 0; i < 3; i++)
        {
            QVariantList el = assertBinsAndRanges( i < binsAndRanges.size() ? binsAndRanges.at(i) : 0 );
            outBR.push_back( el );
        }
        QVariantList outML;
        for (int i = 0; i < 2; i++)
        {
            QVariantList el = assertMarkerLine( i < markerAndLineAttributes.size() ? markerAndLineAttributes.at(i) : 0 );
            outML.push_back( el );
        }

        QString error;
        r->externalLock();
        TTree* t = static_cast<TTree*>(r->GetObject());
        emit requestTreeDraw(t, what, cuts, options, outBR, outML, &error);
        r->externalUnlock();

        //r->resetTreeRecords();

        if (AbortIfFailedToDraw && !error.isEmpty())
            abort("Tree Draw error -> " + error);

        return error;
    }
}

void ATree_SI::save(QString TreeName, QString FileName)
{
    if (!bGuiThread)
    {
        abort("Threads cannot save tree!");
        return;
    }

    ARootTreeRecord* r = dynamic_cast<ARootTreeRecord*>(TmpHub.Trees.getRecord(TreeName));
    if (!r)
        abort("Tree "+TreeName+" not found!");
    else
    {
        r->save(FileName);
        const QString err = r->resetTreeRecords();
        if (!err.isEmpty())
            abort("Could not recover tree after save:\n" + err);
    }
}

void ATree_SI::flushToFile(QString TreeName)
{
    if (!bGuiThread)
    {
        abort("Threads cannot close trees!");
        return;
    }

    ARootTreeRecord* r = dynamic_cast<ARootTreeRecord*>(TmpHub.Trees.getRecord(TreeName));
    if (!r)
        abort("Tree "+TreeName+" not found!");
    else
    {
        bool bOK = r->autoSave();
        if (bOK) ;
        else abort("Tree " + TreeName + " does not support autosave to file.\nThis feature requires the tree to be created with non-empty file name argument.");
    }
}

bool ATree_SI::deleteTree(QString TreeName)
{
    return TmpHub.Trees.remove(TreeName);
}

void ATree_SI::deleteAllTrees()
{
    TmpHub.Trees.clear();
}

void ATree_SI::setAbortIfAlreadyExists(bool flag)
{
    bAbortIfExists = flag;
}
