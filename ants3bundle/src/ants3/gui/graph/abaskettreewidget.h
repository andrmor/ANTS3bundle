#ifndef ABASKETTREEWIDGET_H
#define ABASKETTREEWIDGET_H

#include <QTreeWidget>
#include <QSet>
#include <QStringList>

#include <vector>

class ABasketManager;

class ABasketTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit ABasketTreeWidget(QWidget * parent = nullptr);

    void fill(const ABasketManager & basket);                 // rebuilds the tree
    QTreeWidgetItem * itemForBasketIndex(int index) const;

    bool    isGroup(const QTreeWidgetItem * item) const;
    int     basketIndex(const QTreeWidgetItem * item) const; // -1 for groups / nullptr
    QString groupPath(const QTreeWidgetItem * item) const;   // groups only

    std::vector<int> selectedBasketIndexes() const;           // items only, ascending
    QStringList      selectedGroupPaths() const;              // topmost selected groups only
    QString          groupAt(const QPoint & pos) const;       // clicked group, group of clicked item, or "" (root)

protected:
    void dropEvent(QDropEvent * event) override;

signals:
    void requestMove(const std::vector<int> & items, const QStringList & groups,
                     const QString & targetGroup, int toIndex);

private:
    enum { IndexRole = Qt::UserRole, GroupRole = Qt::UserRole + 1 };

    QSet<QString> CollapsedGroups;
    std::vector<QTreeWidgetItem*> IndexToItem;

    int DefaultIndent = 10;
};

#endif // ABASKETTREEWIDGET_H
