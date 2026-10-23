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

    setExpandsOnDoubleClick(false);

    DefaultIndent = indentation();
}

void ABasketTreeWidget::fill(ABasketManager * basket)
{
    clear();

    const Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    QMap<QString, QTreeWidgetItem*> GroupItems; // Map full group paths to QTreeWidgetItem objects

    // First pass: create all group items
    for (const auto & item : basket->getItems())
    {
        if (!item.isGroup) continue;

        QString path = item.ParentGroup.isEmpty() ? item.Name : item.ParentGroup + "/" + item.Name;

        QTreeWidgetItem * group = new QTreeWidgetItem();
        group->setText(0, item.Name);
        group->setData(0, GroupRole, true);
        group->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        group->setFlags(flags);

        GroupItems[path] = group;
    }

    setIndentation( GroupItems.isEmpty() ? 0 : DefaultIndent ); // check at least one group is not empty

    // Second pass: insert items and groups into the tree
    for (int iBasketInbdex = 0; iBasketInbdex < basket->size(); iBasketInbdex++)
    {
        const ABasketItem & item = basket->getItems()[iBasketInbdex];

        QTreeWidgetItem * treeItem = nullptr;

        if (item.isGroup)
        {
            QString path = item.ParentGroup.isEmpty() ? item.Name : item.ParentGroup + "/" + item.Name;
            treeItem = GroupItems[path];
        }
        else
        {
            treeItem = new QTreeWidgetItem();
            treeItem->setText(0, item.Name);
            treeItem->setData(0, GroupRole, false);
            treeItem->setFlags(flags);
        }

        treeItem->setData(0, IndexRole, iBasketInbdex);
        //if (item.isGroup) treeItem->setExpanded(!item.Collapsed);

        if (item.ParentGroup.isEmpty()) addTopLevelItem(treeItem);
        else
        {
            auto it = GroupItems.find(item.ParentGroup);
            if (it != GroupItems.end()) it.value()->addChild(treeItem);
            else                        addTopLevelItem(treeItem); // Missing parent
        }

    }
}

bool ABasketTreeWidget::isGroup(const QTreeWidgetItem * item) const
{
    if (!item) return true; // true is a safer option
    return item->data(0, GroupRole).toBool();
}

int ABasketTreeWidget::getBasketIndex(const QTreeWidgetItem * item) const
{
    if (!item) return -1;
    return item->data(0, IndexRole).toInt();
}

void ABasketTreeWidget::dropEvent(QDropEvent * event)
{
    qDebug() << "Basket: Drop detected";

    const QList<QTreeWidgetItem*> sel = selectedItems();

    std::vector<int> indexes;
    for (const QTreeWidgetItem * item : sel)
        indexes.push_back( getBasketIndex(item) );

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTreeWidgetItem * itemTo = itemAt(event->pos());
#else
    QTreeWidgetItem * itemTo = itemAt(event->position().toPoint());
#endif

    int indexTo = getBasketIndex(itemTo);
    if (indexTo >= 0)
    {
        if (dropIndicatorPosition() == QAbstractItemView::BelowItem) indexTo++;
    }
    qDebug() << "   Move to item:" << indexTo << itemTo;

    event->ignore();

    emit requestReorder(indexes, indexTo);


    /*
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
    */
}

