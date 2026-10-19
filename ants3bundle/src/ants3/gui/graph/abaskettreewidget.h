#ifndef ABASKETTREEWIDGET_H
#define ABASKETTREEWIDGET_H

#include <QTreeWidget>

class ABasketManager;

class ABasketTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit ABasketTreeWidget(QWidget * parent = nullptr);

    void fill(ABasketManager * basket);

    int basketIndex(int row) const;
    bool isSelectedGroup(QTreeWidgetItem * item);

protected:
    void dropEvent(QDropEvent * event) override;

signals:
    void requestReorder(const std::vector<int> & indexes, int toRow);

private:
    enum { GroupRole = Qt::UserRole };
    int DefaultIndent = 10;

};

#endif // ABASKETTREEWIDGET_H
