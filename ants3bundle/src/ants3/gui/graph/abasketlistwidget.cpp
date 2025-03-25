#include "abasketlistwidget.h"

#include <QDropEvent>
#include <QDebug>

ABasketListWidget::ABasketListWidget(QWidget * parent) :
    QListWidget(parent)
{
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAcceptDrops(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDropIndicatorShown(true);
}

#include <QtGlobal>
void ABasketListWidget::dropEvent(QDropEvent * event)
{
    //qDebug() << "Basket: Drop detected";

    const QList<QListWidgetItem*> sel = selectedItems();
    std::vector<int> indexes;
    for (const QListWidgetItem * item : sel)
        indexes.push_back( row(item) );
    //qDebug() << "   Indexes to be moved:" << indexes;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QListWidgetItem * itemTo = itemAt(event->pos());
#else
    QListWidgetItem * itemTo = itemAt(event->position().toPoint());
#endif

    int rowTo = count();
    if (itemTo)
    {
        rowTo = row(itemTo);
        if (dropIndicatorPosition() == QAbstractItemView::BelowItem) rowTo++;
    }
    //qDebug() << "   Move to position:" << rowTo << itemTo;

    event->ignore();

    emit requestReorder(indexes, rowTo);
}
