#include "abaskettreewidget.h"
#include "abasketmanager.h"

#include <QDropEvent>
#include <QHash>
#include <QStyle>
#include <functional>
#include <algorithm>

ABasketTreeWidget::ABasketTreeWidget(QWidget * parent) :
    QTreeWidget(parent)
{
    setColumnCount(1);
    setHeaderHidden(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAcceptDrops(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDropIndicatorShown(true);

    connect(this, &QTreeWidget::itemCollapsed, this, [this](QTreeWidgetItem * it){ if (isGroup(it)) CollapsedGroups.insert(groupPath(it)); });
    connect(this, &QTreeWidget::itemExpanded,  this, [this](QTreeWidgetItem * it){ if (isGroup(it)) CollapsedGroups.remove(groupPath(it)); });

    DefaultIndent = indentation();
}

bool ABasketTreeWidget::isGroup(const QTreeWidgetItem * item) const
{
    return item && item->data(0, IndexRole).toInt() < 0;
}

int ABasketTreeWidget::basketIndex(const QTreeWidgetItem * item) const
{
    return item ? item->data(0, IndexRole).toInt() : -1;
}

QString ABasketTreeWidget::groupPath(const QTreeWidgetItem * item) const
{
    return item ? item->data(0, GroupRole).toString() : QString();
}

QTreeWidgetItem * ABasketTreeWidget::itemForBasketIndex(int index) const
{
    if (index < 0 || index >= (int)IndexToItem.size()) return nullptr;
    return IndexToItem[index];
}

void ABasketTreeWidget::fill(const ABasketManager & basket)
{
    clear();
    IndexToItem.assign(basket.size(), nullptr);

    setIndentation( basket.countGroups() == 0 ? 0 : DefaultIndent );

    const Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    QHash<QString, QTreeWidgetItem*> groupItems;

    std::function<QTreeWidgetItem*(const QString &)> groupItem = [&](const QString & path) -> QTreeWidgetItem *
    {
        if (path.isEmpty()) return nullptr;
        auto it = groupItems.find(path);
        if (it != groupItems.end()) return it.value();

        QTreeWidgetItem * parent = groupItem(ABasketManager::parentPath(path));
        QTreeWidgetItem * gi = parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(this);
        gi->setText(0, ABasketManager::leafName(path));
        gi->setData(0, IndexRole, -1);
        gi->setData(0, GroupRole, path);
        gi->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        gi->setFlags(flags);
        gi->setExpanded(!CollapsedGroups.contains(path));
        groupItems.insert(path, gi);
        return gi;
    };

    // groups are listed above the items of the same level
    for (const QString & g : basket.getGroups()) groupItem(g);
    for (int i = 0; i < basket.size(); i++) groupItem(basket.getGroup(i));   // safety net

    for (int i = 0; i < basket.size(); i++)
    {
        QTreeWidgetItem * parent = groupItem(basket.getGroup(i));
        QTreeWidgetItem * ti = parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(this);
        ti->setText(0, basket.getName(i));
        ti->setData(0, IndexRole, i);
        ti->setData(0, GroupRole, basket.getGroup(i));
        ti->setFlags(flags);
        IndexToItem[i] = ti;
    }
}

std::vector<int> ABasketTreeWidget::selectedBasketIndexes() const
{
    std::vector<int> res;
    for (const QTreeWidgetItem * item : selectedItems())
        if (!isGroup(item)) res.push_back(basketIndex(item));
    std::sort(res.begin(), res.end());
    return res;
}

QStringList ABasketTreeWidget::selectedGroupPaths() const
{
    QStringList all;
    for (const QTreeWidgetItem * item : selectedItems())
        if (isGroup(item)) all << groupPath(item);

    QStringList top;
    for (const QString & g : all)
    {
        bool nested = false;
        for (const QString & other : all)
            if (other != g && ABasketManager::isInsideGroup(g, other)) {nested = true; break;}
        if (!nested) top << g;
    }
    return top;
}

QString ABasketTreeWidget::groupAt(const QPoint & pos) const
{
    const QTreeWidgetItem * item = itemAt(pos);
    return item ? groupPath(item) : QString();   // GroupRole holds the path for groups and the owning group for items
}

void ABasketTreeWidget::dropEvent(QDropEvent * event)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const QPoint pos = event->pos();
#else
    const QPoint pos = event->position().toPoint();
#endif
    QTreeWidgetItem * target = itemAt(pos);
    const DropIndicatorPosition dip = dropIndicatorPosition();

    QString targetGroup;     // root by default (drop on empty area)
    int toIndex = -1;        // -1: end of the target group
    if (target)
    {
        if (isGroup(target))
            targetGroup = (dip == OnItem) ? groupPath(target) : ABasketManager::parentPath(groupPath(target));
        else
        {
            targetGroup = groupPath(target);              // owning group of the item
            const int idx = basketIndex(target);
            toIndex = (dip == AboveItem) ? idx : idx + 1;
        }
    }

    const QStringList groups = selectedGroupPaths();
    std::vector<int> items = selectedBasketIndexes();

    // items inside a group that is moved too travel with that group
    items.erase(std::remove_if(items.begin(), items.end(), [&](int i)
                               {
                                   const QString g = groupPath(IndexToItem[i]);
                                   for (const QString & mg : groups) if (ABasketManager::isInsideGroup(g, mg)) return true;
                                   return false;
                               }), items.end());

    event->ignore();   // the tree is rebuilt from the manager, never modified by Qt itself

    for (const QString & g : groups)
        if (ABasketManager::isInsideGroup(targetGroup, g)) return;   // cannot drop a group into itself

    emit requestMove(items, groups, targetGroup, toIndex);
}
