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

    const Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    // Map full group paths to QTreeWidgetItem objects
    QMap<QString, QTreeWidgetItem*> GroupItems;

    // First pass: create all groups
    for (const auto& item : basket->getItems())
    {
        if (!item.isGroup) continue;

        // Full path of this group
        QString path = item.ParentGroup.isEmpty() ? item.Name : item.ParentGroup + "/" + item.Name;

        QTreeWidgetItem* group = new QTreeWidgetItem();
        group->setText(0, item.Name);
        group->setData(0, Qt::UserRole, true);
        group->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        group->setFlags(flags);

        GroupItems[path] = group;
    }

    setIndentation( GroupItems.isEmpty() ? 0 : DefaultIndent );

    // Second pass: insert items and groups into the tree
    for (const auto& item : basket->getItems())
    {
        QTreeWidgetItem* treeItem = nullptr;

        // Get the corresponding tree item
        if (item.isGroup)
        {
            QString path = item.ParentGroup.isEmpty() ? item.Name : item.ParentGroup + "/" + item.Name;
            treeItem = GroupItems[path];
        }
        else
        {
            treeItem = new QTreeWidgetItem();
            treeItem->setText(0, item.Name);
            treeItem->setData(0, Qt::UserRole, false);
            treeItem->setFlags(flags);
        }

        // Find the parent group
        if (item.ParentGroup.isEmpty()) addTopLevelItem(treeItem);
        else
        {
            auto it = GroupItems.find(item.ParentGroup);
            if (it != GroupItems.end()) it.value()->addChild(treeItem);
            else                        addTopLevelItem(treeItem); // Missing parent
        }

        if (item.isGroup) treeItem->setExpanded(!item.Collapsed);
    }

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

