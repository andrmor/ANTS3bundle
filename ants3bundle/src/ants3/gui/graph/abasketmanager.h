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

    QString             addGroup(const QString & name, const QString & parentGroup, int iBasketIndex = -1); // parent group: "aaa/bbb/ccc" - full path! -1 -> as last item of this group, otherwise at this position !!!*** todo insert/check
    QStringList         getGroupNames() const;

    void                clear();
    QString             remove(std::vector<int> indexesToRemove); // returns error if it was not possible to remove at least one item (part of a multidraw)

    QString             getType(int index) const;

    int                 size() const;

    QString             getName(int index) const;
    void                rename(int index, const QString & newName);
    QStringList         getItemNames() const; // not needed?

    std::vector<ABasketItem> & getItems() {return Basket;}  // !!!*** to make safe later
    void                setCollapsedFlag(int index, bool flag);

    bool                isMultidraw(int index) const;
    bool                isMemberOfSpecificMultidraw(int index, int multidrawIndex);

    void                saveBasket(const QString & fileName);
    QString             appendBasket(const QString & fileName);

    QString             appendTxtAsGraph(const QString & fileName);
    QString             appendTxtAsGraphErrors(const QString & fileName);
    void                appendRootHistGraphs(const QString & fileName);

    void                reorder(const std::vector<int> & indexes, int to); // assume uniqueness of indexes
    void                moveLastBasketItemToGroup(const QString & parentGroup, bool toBottom); // triggers reorder

    QString             mergeHistograms(const std::vector<int> & indexes);

    // groups
    static QString      checkGroupName(const QString & name);  // returns error text or ""

private:
    std::vector<ABasketItem> Basket;

private:
    int                 findPointerInDrawObjects(const std::vector<ADrawObject> & drawObjects, TObject * obj) const;
    std::vector<size_t> getAllMultidrawsUsingIndex(size_t index);
    int                 findGroup(const QString & name); // returns -1 if not found

public:
    static TObject * makeCloneOfTObject(TObject * tobj);
};

#endif // ABASKETMANAGER_H
