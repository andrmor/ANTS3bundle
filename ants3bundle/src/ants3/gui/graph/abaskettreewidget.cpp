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

    connect(this, &QTreeWidget::itemSelectionChanged, this, &ABasketTreeWidget::updateMovePreview);
}

/*
void ABasketTreeWidget::fill(ABasketManager * basket)
{
    const QSignalBlocker blocker(this);
    clear();

    const Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    const auto & items = basket->getItems();
    const int n = items.size();

    auto fullPath = [](const ABasketItem & item)
    {
        return item.ParentGroup.isEmpty() ? item.Name : item.ParentGroup + "/" + item.Name;
    };

    // Pass 1: one tree item per basket entry, stored at the same index as in the basket
    QVector<QTreeWidgetItem*> treeItems(n, nullptr);
    bool hasGroups = false;
    for (int i = 0; i < n; i++)
    {
        const ABasketItem & item = items[i];

        QTreeWidgetItem * treeItem = new QTreeWidgetItem();
        treeItem->setText(0, item.Name);
        treeItem->setData(0, GroupRole, item.isGroup);
        treeItem->setData(0, IndexRole, i);
        treeItem->setFlags(flags);
        if (item.isGroup)
        {
            treeItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
            hasGroups = true;
        }
        treeItems[i] = treeItem;
    }

    setIndentation(hasGroups ? DefaultIndent : 0);

    // Finds the tree item of the group with the given full path (first match wins)
    auto findGroup = [&](const QString & path) -> QTreeWidgetItem *
    {
        for (int i = 0; i < n; i++)
            if (items[i].isGroup && fullPath(items[i]) == path) return treeItems[i];
        return nullptr;
    };

    // Pass 2: attach items to their parents (parents need not be attached yet)
    for (int i = 0; i < n; i++)
    {
        QTreeWidgetItem * parent = items[i].ParentGroup.isEmpty() ? nullptr : findGroup(items[i].ParentGroup);

        if (parent && parent != treeItems[i]) parent->addChild(treeItems[i]);
        else
        {
            if (!items[i].ParentGroup.isEmpty())
                qWarning() << "ABasketTreeWidget::fill: missing parent group" << items[i].ParentGroup << "for" << items[i].Name;
            addTopLevelItem(treeItems[i]);
        }
    }

    // Pass 3: expansion state, after the whole tree is built
    for (int i = 0; i < n; i++)
        if (items[i].isGroup) treeItems[i]->setExpanded(!items[i].Collapsed);
}
*/

void ABasketTreeWidget::fill(ABasketManager * basket)
{
    // Don't let clear()/setExpanded() trigger itemExpanded/itemCollapsed slots
    const QSignalBlocker blocker(this);
    clear();

    const Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    const int n = basket->size();

    // I assume max ~200 items and ~25 groups
    QVector<QTreeWidgetItem*> treeItems(n, nullptr); // same indexing as the basket
    QVector<QString>          groupPaths(n);         // full path, filled for groups only
    QVector<int>              groupIndexes;          // basket indexes of the groups (few)

    // Pass 1: one tree item per basket entry
    for (int i = 0; i < n; i++)
    {
        //const ABasketItem & item = items[i];

        QTreeWidgetItem * treeItem = new QTreeWidgetItem();
        treeItem->setText(0, basket->getName(i));
        treeItem->setData(0, GroupRole, basket->isGroup(i));
        treeItem->setData(0, IndexRole, i);
        treeItem->setFlags(flags);
        if (basket->isGroup(i))
        {
            treeItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
            groupPaths[i] = basket->joinPath(basket->getParentGroup(i), basket->getName(i));
            groupIndexes << i;
        }
        treeItems[i] = treeItem;
    }

    setIndentation(groupIndexes.isEmpty() ? 0 : DefaultIndent); // maybe still 0 if all groups are empty? !!!***

    // Finds the group with the given full path (first match wins)
    auto findGroup = [&](const QString & path) -> QTreeWidgetItem *
    {
        for (int g : groupIndexes)
            if (groupPaths[g] == path) return treeItems[g];
        return nullptr;
    };

    // Pass 2: attach items to their parents (parents need not be attached yet)
    for (int i = 0; i < n; i++)
    {
        //const ABasketItem & item = items[i];

        QTreeWidgetItem * parent = basket->getParentGroup(i).isEmpty() ? nullptr : findGroup(basket->getParentGroup(i));

        if (parent && parent != treeItems[i]) parent->addChild(treeItems[i]);
        else
        {
            if (!basket->getParentGroup(i).isEmpty())
                qWarning() << "ABasketTreeWidget::fill: missing or invalid parent group" << basket->getParentGroup(i) << "for" << basket->getName(i);
            addTopLevelItem(treeItems[i]);
        }
    }

    // Paranoid now, but might help later if the design idea changes: Rescue items not reachable from the tree (parent cycles in corrupted data)
    for (int i = 0; i < n; i++)
    {
        QTreeWidgetItem * t = treeItems[i];
        if (t->treeWidget()) continue; // reachable

        qWarning() << "ABasketTreeWidget::fill: unreachable item (parent cycle)" << basket->getName(i);
        if (QTreeWidgetItem * p = t->parent()) p->removeChild(t); // break the cycle
        addTopLevelItem(t);
    }

    // Pass 3: expansion state, once the whole tree is built
    for (int g : groupIndexes)
        treeItems[g]->setExpanded(!basket->isCollapsedGroup(g));
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

int ABasketTreeWidget::dropIndicatorToFixedInt(DropIndicatorPosition indicator)
{
    switch (indicator)
    {
    case OnItem     : return 0;
    case AboveItem  : return 1;
    case BelowItem  : return 2;
    case OnViewport : return 3;
    }
    return 3;
}

void ABasketTreeWidget::updateMovePreview()
{
    // Collect all explicitly selected items.
    const QList<QTreeWidgetItem *> selected = selectedItems();

    // Record all items that will be moved, including descendants.
    QSet<QTreeWidgetItem *> moveItems;
    QSet<QTreeWidgetItem *> explicitlySelected;

    for (QTreeWidgetItem * item : selected)
    {
        explicitlySelected.insert(item);
        moveItems.insert(item);

        // Individual items have no descendants to include.
        if (!item->data(0, GroupRole).toBool())
            continue;

        // Include all descendants, regardless of expansion state.
        QVector<QTreeWidgetItem *> stack;

        for (int i = 0; i < item->childCount(); ++i)
            stack.append(item->child(i));

        while (!stack.isEmpty())
        {
            QTreeWidgetItem * child = stack.takeLast();
            moveItems.insert(child);

            for (int i = 0; i < child->childCount(); ++i)
                stack.append(child->child(i));
        }
    }

    // Refresh the preview for every item in the tree.
    QVector<QTreeWidgetItem *> stack;

    for (int i = 0; i < topLevelItemCount(); ++i)
        stack.append(topLevelItem(i));

    //const QBrush previewBrush(palette().color(QPalette::AlternateBase));
    //const QBrush previewBrush(QColor(255, 220, 150)); // light orange
    /*
    QColor selectionColor = palette().color(QPalette::Highlight);
    QColor previewColor = selectionColor.lighter(170);
    */
    QColor selectionColor = palette().color(QPalette::Highlight);
    QColor backgroundColor = palette().color(QPalette::Base);
    QColor previewColor(
        (selectionColor.red()   + 2 * backgroundColor.red())   / 3,
        (selectionColor.green() + 2 * backgroundColor.green()) / 3,
        (selectionColor.blue()  + 2 * backgroundColor.blue())  / 3
        );

    while (!stack.isEmpty())
    {
        QTreeWidgetItem * item = stack.takeLast();

        const bool preview = moveItems.contains(item);
        const bool explicitSelection = explicitlySelected.contains(item);

        // Store the preview state independently of the visual appearance.
        item->setData(0, MovePreviewRole, preview && !explicitSelection);

        // Preserve Qt's standard selection appearance for explicitly
        // selected items. Tint only implicitly included descendants.
        /*
        if (!explicitSelection && preview)
            item->setBackground(0, previewBrush);
        else
            item->setBackground(0, QBrush());
        */
        /*
        if (!explicitSelection && preview)
        {
            item->setBackground(0, QBrush(QColor(255, 220, 150)));
            item->setForeground(0, QBrush(Qt::black));
        }
        else
        {
            item->setBackground(0, QBrush());
            item->setForeground(0, QBrush());
        }
        */
        if (!explicitSelection && preview)
            item->setBackground(0, QBrush(previewColor));
        else
            item->setBackground(0, QBrush());

        for (int i = 0; i < item->childCount(); ++i)
            stack.append(item->child(i));
    }
}

void ABasketTreeWidget::dropEvent(QDropEvent * event)
{
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
    const DropIndicatorPosition dropIndicator = dropIndicatorPosition();
    int on0above1below2viewport3 = dropIndicatorToFixedInt(dropIndicator);

    event->ignore();

    emit requestReorder(indexes, indexTo, on0above1below2viewport3);
}

