#include "abaskettreewidget.h"
#include "abasketmanager.h"

#include <QDropEvent>

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

    //connect(this, &QTreeWidget::itemCollapsed, this, [this](QTreeWidgetItem * it){ if (isGroup(it)) CollapsedGroups.insert(groupPath(it)); });
    //connect(this, &QTreeWidget::itemExpanded,  this, [this](QTreeWidgetItem * it){ if (isGroup(it)) CollapsedGroups.remove(groupPath(it)); });

    DefaultIndent = indentation();
}

void ABasketTreeWidget::fill(ABasketManager * basket)
{
    clear();

    //setIndentation( basket.countGroups() == 0 ? 0 : DefaultIndent );

    // Stack of groups representing the current hierarchy.
    const Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    std::vector<QTreeWidgetItem*> groupStack;

    for (const ABasketItem & record : basket->getItems())
        {
            if (!record.Group.isEmpty())
            {
                // Create a group item.
                auto * groupItem = new QTreeWidgetItem();
                groupItem->setText(0, record.Group);
                groupItem->setData(0, GroupRole, true);
                groupItem->setFlags(flags);

                // A group is a child of the preceding group, if any.
                if (groupStack.empty())
                    addTopLevelItem(groupItem);
                else
                    groupStack.back()->addChild(groupItem);

                groupItem->setExpanded(!record.CollapsedGroup);
                groupStack.push_back(groupItem);
            }
            else
            {
                // Create an individual item.
                auto * item = new QTreeWidgetItem();
                item->setText(0, record.Name);
                item->setData(0, GroupRole, false);
                item->setFlags(flags);

                // Individual items belong to the innermost group
                if (groupStack.empty())
                    addTopLevelItem(item);
                else
                    groupStack.back()->addChild(item);
            }
        }

/*
    QHash<QString, QTreeWidgetItem*> groupItems;

    std::function<QTreeWidgetItem*(const QString &)> groupItem = [&](const QString & path) -> QTreeWidgetItem *
    {
        if (path.isEmpty()) return nullptr;
        auto it = groupItems.find(path);
        if (it != groupItems.end()) return it.value();

        QTreeWidgetItem * parent = groupItem(ABasketManager::parentPath(path));
        QTreeWidgetItem * gi = parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(this);
        gi->setText(0, ABasketManager::leafName(path));
        gi->setData(0, ItemRole, -1);
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
        ti->setData(0, ItemRole, i);
        ti->setData(0, GroupRole, basket.getGroup(i));
        ti->setFlags(flags);
    }
*/
}

int ABasketTreeWidget::basketIndex(int row) const
{
    /// ***
    qWarning() << "basketIndex not implemented yet, returning 0";
    // compute based on number of groups and collapse status
    return 0;
}

bool ABasketTreeWidget::isSelectedGroup(QTreeWidgetItem * item)
{
    if (!item) return false;
    return item->data(0, Qt::UserRole).toBool();
}

void ABasketTreeWidget::dropEvent(QDropEvent * event)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const QPoint pos = event->pos();
#else
    const QPoint pos = event->position().toPoint();
#endif

    /*
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
*/
}

