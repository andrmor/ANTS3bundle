#ifndef AROOTTREERECORD_H
#define AROOTTREERECORD_H

#include "arootobjbase.h"

#include <QObject>
#include <QVector>
#include <QMap>
#include <QString>
#include <QMutex>

#include "TString.h"

#include <vector>
#include <string>

class TTree;
class TBranch;

class ABranchBuffer
{
public:
    ABranchBuffer(const QString& branchName, const QString& branchType, TTree* tree);     // creates a new branch!
    ABranchBuffer(const QString& branchName, const QString& branchType, TBranch* branch); // creates buffer for a branch loaded from TTree
    ABranchBuffer(){}

    bool           isValid() const {return branchPtr;}

    void           write(const QVariant& val);
    const QVariant read();

    bool           canFill() const {return bCanFill;}

    QString        name;
    TString        tName;
    QString        type;
    TTree*         treePtr;
    char           cType = '-';
    bool           bVector = false;
    TBranch*       branchPtr = 0;      // if it remains 0 -> branch is invalid

    //TString C;
    char    C[300];
    int     I;
    float   F;
    double  D;
    bool    O;
    std::vector<std::string> AC, *pAC; //need pointers in TTree::setBranchAddress()
    std::vector<int>     AI, *pAI;
    std::vector<float>   AF, *pAF;
    std::vector<double>  AD, *pAD;
    std::vector<bool>    AO, *pAO;
    QVector<bool>        Ao;           // to read std::vector<bool>

private:
    bool bCanFill = true;              //fixed array type can be loaded from TFile but new entries cannot be added to these trees

public:
    static bool isValidType(const QString & codeName); // {"C","I","F","D","O","AC","AI","AF","AD","AO"};

};

class TFile;

class ARootTreeRecord : public ARootObjBase
{
public:
    ARootTreeRecord(TObject* tree, const QString & name);
    ~ARootTreeRecord();

    // Protected by Mutex
    bool               createTree(const QString & name, const std::vector<std::pair<QString, QString>> & branches,
                                  const QString fileName = "", int autosaveNum = 10000);
    QString            loadTree(const QString& fileName, const QString treeNameInFile = ""); //report error ("" if fine)
    QString            resetTreeRecords(); // need to call it after save!

    int                countBranches() const;
    bool               isBranchExist(const QString& branchName) const;
    QStringList        getBranchNames() const;
    QStringList        getBranchTypes() const;
    int                countEntries() const;

    bool               fillSingle(const QVariantList& vl);

    QVariantList       getBranch(const QString & branchName);
    QVariant           getBranch(const QString & branchName, int entry);
    QVariantList       getEntry(int entry);

    void               save(const QString &FileName);

    bool               autoSave();
    //void               setAutoSave(int autosaveAfterEntriesWritten);

    //void               scan(const QString& arg1, const QString& arg2, const QString& arg3);

private:
    std::vector<ABranchBuffer*> Branches;
    QMap<QString, ABranchBuffer*> MapOfBranches;

    TFile * file = nullptr;

    bool  canAddEntries = true;  //some trees can be loaded, but due to conversion fo data, cannot be filled with new entries

};

#endif // AROOTTREERECORD_H
