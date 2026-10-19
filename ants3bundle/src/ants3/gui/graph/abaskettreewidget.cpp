#include "abaskettreewidget.h"

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

