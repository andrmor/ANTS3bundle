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

    bool isGroup(const QTreeWidgetItem * item) const;
    int getBasketIndex(const QTreeWidgetItem * item) const;

protected:
    void dropEvent(QDropEvent * event) override;  // !!!*** check item drop to last position! overflow index?

signals:
    void requestReorder(std::vector<int> & indexes, int toIndex, int on0above1below2viewport3);

private:
    enum { GroupRole = Qt::UserRole, IndexRole = Qt::UserRole + 1 };
    int DefaultIndent = 10;

    int dropIndicatorToFixedInt(DropIndicatorPosition indicator);
};

#endif // ABASKETTREEWIDGET_H
