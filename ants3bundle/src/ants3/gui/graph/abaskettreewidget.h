#ifndef ABASKETTREEWIDGET_H
#define ABASKETTREEWIDGET_H

#include <QTreeWidget>

class ABasketTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit ABasketTreeWidget(QWidget * parent = nullptr);

protected:
    void dropEvent(QDropEvent * event) override;

signals:
    void requestReorder(const std::vector<int> & indexes, int toRow);

private:
    int DefaultIndent = 10;

};

#endif // ABASKETTREEWIDGET_H
