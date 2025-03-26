#ifndef AINTERFACETOTTREE_H
#define AINTERFACETOTTREE_H

#include "ascriptinterface.h"

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantList>

class AScriptObjStore;
class TTree;

class ATree_SI : public AScriptInterface
{
    Q_OBJECT

public:
    ATree_SI();

    AScriptInterface * cloneBase() const {return new ATree_SI();}

public slots:
    void         newTree(QString TreeName, QVariantList HeadersOfBranches,
                         QString StoreInFileName = "", int AutosaveAfterEntriesAdded = 10000);
    void         loadTree(QString TreeName, QString FileName, QString TreeNameInFile = ""); //one leaf per branch!

    void         fill(QString TreeName, QVariantList Array);

    int          getNumEntries(QString TreeName);
    QVariantList getBranchNames(QString TreeName);
    QVariantList getBranchTypes(QString TreeName);

    QVariantList getBranch(QString TreeName, QString BranchName);
    QVariant     getBranch(QString TreeName, QString BranchName, int EntryIndex);
    QVariantList getEntry(QString TreeName, int EntryIndex);

    QString      draw(QString TreeName, QString what, QString cuts, QString options,
                      QVariantList binsAndRanges = QVariantList(), QVariantList markerAndLineAttributes = QVariantList(),
                      bool AbortIfFailedToDraw = true);

    void         save(QString TreeName, QString FileName);

    void         flushToFile(QString TreeName);

    QVariantList getAllTreeNames();
    bool         isTreeExists(QString TreeName);

    void         resetTreeAddresses(QString TreeName);
    //void     Scan(const QString& TreeName, const QString &arg1, const QString &arg2, const QString &arg3);

    bool         deleteTree(QString TreeName);
    void         deleteAllTrees();

    void         setAbortIfAlreadyExists(bool flag);

signals:
    void         requestTreeDraw(TTree * tree, QString what, QString cond, QString how,
                                 QVariantList binsAndRanges, QVariantList markersAndLine, QString * result = nullptr);

private:
    AScriptObjStore & TmpHub;

    bool bAbortIfExists = false;
};

#endif // AINTERFACETOTTREE_H
