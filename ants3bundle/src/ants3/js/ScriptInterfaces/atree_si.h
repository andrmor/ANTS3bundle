#ifndef AINTERFACETOTTREE_H
#define AINTERFACETOTTREE_H

#include "ascriptinterface.h"

#include <QObject>
#include <QVector>
#include <QString>
#include <QVariant>
#include <QVariantList>

#include "TString.h"
#include "TTree.h"

#include <vector>

class AScriptObjStore;

class ATree_SI : public AScriptInterface
{
    Q_OBJECT

public:
    ATree_SI();

public slots:

    void     newTree(QString TreeName, QVariantList HeadersOfBranches,
                     QString StoreInFileName = "", int AutosaveAfterEntriesAdded = 10000); // !!!*** to std::vector inside
    void     loadTree(QString TreeName, QString FileName, QString TreeNameInFile = ""); //one leaf per branch!

    void     fill(QString TreeName, QVariantList Array);

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
    void         RequestTreeDraw(TTree* tree, const QString& what, const QString& cond, const QString& how,
                                 const QVariantList binsAndRanges, const QVariantList markersAndLine, QString* result = 0);

private:
    AScriptObjStore & TmpHub;

    bool            bAbortIfExists = false;
};

#endif // AINTERFACETOTTREE_H
