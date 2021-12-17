#ifndef ABASKETMANAGER_H
#define ABASKETMANAGER_H

#include "abasketitem.h"

#include <QString>
#include <QVector>
#include <QStringList>

class ABasketManager
{
public:
    ABasketManager();
    ~ABasketManager();

    void                add(const QString & name, const QVector<ADrawObject> & drawObjects); //makes deep copy
    void                update(int index, const QVector<ADrawObject> & drawObjects);         //makes deep copy

    const QVector<ADrawObject> getCopy(int index) const;  //returns deep copy

    void                clear();
    void                remove(int index);

    QString             getType(int index) const;

    int                 size() const;

    QString             getName(int index) const;
    void                rename(int index, const QString & newName);
    const QStringList   getItemNames() const;

    void                saveAll(const QString & fileName);

    QString             appendBasket(const QString & fileName);

    QString             appendTxtAsGraph(const QString & fileName);
    QString             appendTxtAsGraphErrors(const QString & fileName);
    void                appendRootHistGraphs(const QString & fileName);

    void                reorder(const QVector<int> &indexes, int to);

private:
    QVector<ABasketItem> Basket;
    //QVector<ADrawObject> NotValidItem; // to return on wrong index

private:
    int                 findPointerInDrawObjects(const QVector<ADrawObject> & DrawObjects, TObject * obj) const;
};

#endif // ABASKETMANAGER_H
