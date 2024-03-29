#ifndef AGEOBASETREEWIDGET_H
#define AGEOBASETREEWIDGET_H

#include <QTreeWidget>

class AGeoObject;

class AGeoBaseTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    AGeoBaseTreeWidget(AGeoObject * World);

protected:
    void dropEvent(QDropEvent * event);
    void dragEnterEvent(QDragEnterEvent * event);
    void dragMoveEvent(QDragMoveEvent * event);
    void dragLeaveEvent(QDragLeaveEvent *);

private:
    AGeoObject * World;

    QTreeWidgetItem * previousHoverItem = nullptr;
    const QTreeWidgetItem * movingItem  = nullptr;  // used only to prevent highlight of item under the moving one if it is the same as target

private:
    //void configureStyle();  // hard to maintain with continuos changes in Qt

signals:
    void RequestRebuildDetector();
};

#endif // AGEOBASETREEWIDGET_H
