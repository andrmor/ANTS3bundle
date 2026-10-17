#ifndef ABASKETMANAGER_H
#define ABASKETMANAGER_H

#include "abasketitem.h"

#include <QString>
#include <QStringList>

#include <vector>

class ABasketManager
{
public:
    ~ABasketManager();

    void                add(const QString & name, const std::vector<ADrawObject> & drawObjects); //makes deep copy
    void                update(int index, const std::vector<ADrawObject> & drawObjects);         //makes deep copy

    std::vector<ADrawObject> getCopy(int index) const;  //returns deep copy

    void                clear();
    QString             remove(std::vector<int> indexesToRemove); // returns error if it was not possible to remove at least one item (part of a multidraw)

    QString             getType(int index) const;

    int                 size() const;

    QString             getName(int index) const;
    void                rename(int index, const QString & newName);
    QStringList         getItemNames() const;

    bool                isMultidraw(int index) const;
    bool                isMemberOfSpecificMultidraw(int index, int multidrawIndex);

    void                saveBasket(const QString & fileName);
    QString             appendBasket(const QString & fileName);

    QString             appendTxtAsGraph(const QString & fileName);
    QString             appendTxtAsGraphErrors(const QString & fileName);
    void                appendRootHistGraphs(const QString & fileName);

    void                reorder(const std::vector<int> & indexes, int to); // assume uniqueness of indexes

    QString             mergeHistograms(const std::vector<int> & indexes);

    // ---- groups (tree view support) ----
    static QString      joinPath(const QString & parent, const QString & name);
    static QString      parentPath(const QString & path);
    static QString      leafName(const QString & path);
    static QString      normalizePath(const QString & path);
    static bool         isInsideGroup(const QString & path, const QString & group); // path==group or descendant; root contains everything
    static QString      checkGroupName(const QString & name);                       // returns error text or ""

    QString             getGroup(int index) const;
    const std::vector<QString> & getGroups() const {return Groups;}
    bool                groupExists(const QString & path) const;                    // "" (root) always exists
    std::vector<int>    getItemsInGroup(const QString & path, bool recursive) const;

    // GROUPS
    bool                countGroups() const;
    void                ensureGroup(const QString & path);                          // creates missing ancestors too
    QString             addGroup(const QString & path);
    QString             renameGroup(const QString & path, const QString & newName);
    QString             moveGroup(const QString & path, const QString & newParentPath);
    QString             removeGroup(const QString & path);                          // items/subgroups move to the parent
    QString             removeGroupAndContents(const QString & path);               // uses remove(), so multidraw protection applies
    // indexes: unique, ascending. toIndex: flat position (same meaning as in reorder()), or -1 = end of the group
    QString             moveItems(const std::vector<int> & indexes, const QString & targetGroup, int toIndex);

private:
    std::vector<ABasketItem> Basket;
    std::vector<QString>     Groups;   // all existing groups (full paths); order = display order among siblings

private:
    int                 findPointerInDrawObjects(const std::vector<ADrawObject> & drawObjects, TObject * obj) const;
    std::vector<size_t> getAllMultidrawsUsingIndex(size_t index);

    // GROUPS
    void                replaceGroupPrefix(const QString & oldPath, const QString & newPath);
    void                normalizeGroups();                              // ancestors always precede descendants in Groups
    void                arrangeByTree();                                // makes flat order == tree order (remaps multidraw indexes)
    void                applyOrder(const std::vector<int> & order);     // order[newPos] = oldIndex

public:
    static TObject * makeCloneOfTObject(TObject * tobj);
};

#endif // ABASKETMANAGER_H
