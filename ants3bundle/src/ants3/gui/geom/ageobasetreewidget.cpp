#include "ageobasetreewidget.h"
#include "ageoobject.h"
#include "ageotype.h"

#include <QDropEvent>
#include <QMessageBox>
#include <QList>
#include <QDebug>
#include <QTreeView>
#include <QtGlobal>
//#include <QApplication>
//#include <QStyle>
#include <QPalette>

AGeoBaseTreeWidget::AGeoBaseTreeWidget(AGeoObject * World) :
    QTreeWidget(), World(World)
{
    setHeaderHidden(true);
    setAcceptDrops(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    //setDropIndicatorShown(false);
    setDropIndicatorShown(true);
    //setIndentation(50);
    setContentsMargins(0, 0, 0, 0);
    setFrameStyle(QFrame::NoFrame);
    setIconSize(QSize(20, 20));
    setContextMenuPolicy(Qt::CustomContextMenu);

    //configureStyle();
}

/*
void AGeoBaseTreeWidget::configureStyle()
{
    QString style = "QTreeView {alternate-background-color: yellow;}"
            "QTreeView::branch:has-siblings:!adjoins-item {"
                "border-image: url(:/images/gui/images/tw-vline.png) 0;"
            "}"
            "QTreeView::branch:has-siblings:adjoins-item {"
                "border-image: url(:/images/gui/images/tw-branch-more.png) 0;"
            "}"
            "QTreeView::branch:!has-children:!has-siblings:adjoins-item {"
                "border-image: url(:/images/gui/images/tw-branch-end.png) 0;"
            "}"
            "QTreeView::branch:has-children:!has-siblings:closed,"
            "QTreeView::branch:closed:has-children:has-siblings {"
                "border-image: none;"
                "image: url(:/images/gui/images/tw-branch-closed.png);"
            "}"
            "QTreeView::branch:open:has-children:!has-siblings,"
            "QTreeView::branch:open:has-children:has-siblings {"
                "border-image: none;"
                "image: url(:/images/gui/images/tw-branch-open.png);"
            "}";
    setStyleSheet(style);
}
*/

void AGeoBaseTreeWidget::dropEvent(QDropEvent * event)
{
    if (previousHoverItem)
    {
        //previousHoverItem->setBackground(0, Qt::white);
        //previousHoverItem->setBackground(0, QApplication::style()->standardPalette().color(QPalette::Window));
        previousHoverItem->setBackground(0, QPalette().color(QPalette::Base));
        previousHoverItem = nullptr;
    }

    QList<QTreeWidgetItem*> selected = selectedItems();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTreeWidgetItem * itemTo = this->itemAt(event->pos());
#else
    QTreeWidgetItem * itemTo = this->itemAt(event->position().toPoint());
#endif
    if (!itemTo)
    {
        qDebug() << "No item on drop position - rejected!";
        event->ignore();
        return;
    }
    QString DraggedTo = itemTo->text(0);
    AGeoObject * objTo = World->findObjectByName(DraggedTo);
    if (!objTo)
    {
        qWarning() << "Error: objTo not found!";
        event->ignore();
        return;
    }

    if (objTo->fLocked || objTo->Type->isInstance())
    {
        qWarning() << "Cannot drop to a locked object or a prototype instance";
        event->ignore();
        return;
    }

    QStringList selNames;

    AGeoObject * ContainerTo = nullptr;
    //qDebug() <<"start of drop";
    if (!showDropIndicator())
    {
        ContainerTo = objTo;
        //qDebug() <<"ON ITEM";
    }
    else
    {
        if (objTo->Container)
             ContainerTo = objTo->Container;
        else ContainerTo = objTo;
        //qDebug() <<"NOT ON ITEM";
    }
    QString containerErrorStr;
    bool containerOk = true;
    if (!ContainerTo->isWorld()) containerOk = ContainerTo->isContainerValidForDrop(containerErrorStr);

    for (int i=0; i<selected.size(); i++) //error catching
    {
        QTreeWidgetItem* DraggedItem = this->selectedItems().at(i);
        if (!DraggedItem)
        {
            //qDebug() << "Drag source item invalid, ignore";
            event->ignore();
            return;
        }

        QString DraggedName = DraggedItem->text(0);
        AGeoObject* obj = World->findObjectByName(DraggedName);
        if (!obj)
        {
            qWarning() << "Error: obj not found!";
            event->ignore();
            return;
        }

        if (ContainerTo != obj->Container)
        {
            if (!containerOk)
            {
                event->ignore();
                QMessageBox::information(this, "", containerErrorStr);
                return;
            }

            if (obj->isInUseByComposite())
            {
                event->ignore();
                QMessageBox::information(this, "", "Cannot move objects: Contains object(s) used in a composite object generation");
                return;
            }
            if (ContainerTo->Type->isCompositeContainer() && !obj->Type->isSingle())
            {
                event->ignore();
                QMessageBox::information(this, "", "Can insert only elementary objects to the list of composite members!");
                return;
            }
            if (!ContainerTo->Type->isPrototype() &&
                ContainerTo->Type->isHandlingSet() &&
                !obj->Type->isSingle())
            {
                event->ignore();
                QMessageBox::information(this, "", "Can insert only elementary objects to sets!");
                return;
            }
        }
    }

    for (int i=selected.size()-1; i>-1; i--) //actual move and reorder
    {
        QTreeWidgetItem* DraggedItem = this->selectedItems().at(i);

        QString DraggedName = DraggedItem->text(0);
        AGeoObject * obj = World->findObjectByName(DraggedName);
        obj->TrueRot = nullptr;

        if (ContainerTo != obj->Container)
        {
            //AGeoObject* objFormerContainer = obj->Container;

            bool ok = obj->migrateTo(ContainerTo);
            if (!ok)
            {
                qWarning() << "Object migration failed: cannot migrate down the chain (or to itself)!";
                event->ignore();
                return;
            }

            /*
            if (ContainerTo->Type->isStack())
            {
                //qDebug() << "updating stack of this object";
                obj->updateStack();
            }
            if (objFormerContainer && objFormerContainer->Type->isStack())
            {
                //qDebug() << "updating stack of the former container";
                if (objFormerContainer->HostedObjects.size()>0)
                    objFormerContainer->HostedObjects.front()->updateStack();
            }
            */
        }

        if (ContainerTo != objTo)
        {
            bool fAfter = (dropIndicatorPosition() == QAbstractItemView::BelowItem);
            obj->repositionInHosted(objTo, fAfter);

            //if (obj && obj->Container && obj->Container->Type->isStack()) obj->updateStack();
        }

    }
    //qDebug() << "Drag completed, updating gui";
    //UpdateGui();
    emit RequestRebuildDetector();

    // Restore selection
    for (int i=0; i<selNames.size(); i++)
    {
        QList<QTreeWidgetItem*> list = findItems(selNames.at(i), Qt::MatchExactly | Qt::MatchRecursive);
        if (!list.isEmpty()) list.first()->setSelected(true);
    }

    event->ignore();
    return;
}

void AGeoBaseTreeWidget::dragEnterEvent(QDragEnterEvent *event)
{
    previousHoverItem = nullptr;
    //qDebug() << "Drag enter. Selection size:"<< selectedItems().size();
    //attempt to drag items contaning locked objects should be canceled!

    const int numItems = selectedItems().size();
    for (int iItem = 0; iItem < numItems; iItem++)
    {
        QTreeWidgetItem * DraggedItem = selectedItems().at(iItem);
        QString DraggedName = DraggedItem->text(0);
        //qDebug() << "Draggin item:"<< DraggedName;
        AGeoObject * obj = World->findObjectByName(DraggedName);
        if (!obj)
        {
            qDebug() << "Drag item not found!!";
            event->ignore();
            return;
        }
        //if (obj->fLocked || obj->isContainsLocked() || obj->Type->isGridElement() || obj->Type->isCompositeContainer())
        if (obj->fLocked || obj->Type->isGridElement() || obj->Type->isCompositeContainer())
        {
            qDebug() << "Drag forbidden for one of the items!";
            event->ignore();
            return;
        }

    }

    // Drop and mouseRelease are not fired if drop on the same item as started -> teal highlight is not removed
    // Clumsy fix - do not show teal highlight if the item is the same
    movingItem = ( numItems > 0 ? selectedItems().at(0)
                                : nullptr);

    event->accept();
}

void AGeoBaseTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->source() != this)
    {
        event->ignore();
        return;
    }

    QTreeWidget::dragMoveEvent(event);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const Qt::KeyboardModifiers mod = event->keyboardModifiers();
#else
    const Qt::KeyboardModifiers mod = event->modifiers();
#endif
    bool bRearrange = (mod == Qt::ALT || mod == Qt::CTRL || mod == Qt::SHIFT);

    setDropIndicatorShown(bRearrange);

    if (previousHoverItem)
    {
        //previousHoverItem->setBackground(0, Qt::white);
        //previousHoverItem->setBackground(0, QApplication::style()->standardPalette().color(QPalette::Window));
        previousHoverItem->setBackground(0, QPalette().color(QPalette::Base));
        previousHoverItem = nullptr;
    }
    if (!bRearrange)
    {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QTreeWidgetItem * itemOver = this->itemAt(event->pos());
#else
        QTreeWidgetItem * itemOver = this->itemAt(event->position().toPoint());
#endif
        if (itemOver && itemOver != movingItem)
        {
            //itemOver->setBackground(0, Qt::cyan);
            //itemOver->setBackground(0, QApplication::style()->standardPalette().color(QPalette::Highlight));
            itemOver->setBackground(0, QPalette().color(QPalette::Highlight));
            previousHoverItem = itemOver;
        }
    }
}

void AGeoBaseTreeWidget::dragLeaveEvent(QDragLeaveEvent *)
{
    if (previousHoverItem)
    {
        //previousHoverItem->setBackground(0, Qt::white);
        //previousHoverItem->setBackground(0, QApplication::style()->standardPalette().color(QPalette::Window));
        previousHoverItem->setBackground(0, QPalette().color(QPalette::Base));
        previousHoverItem = nullptr;
    }
}

