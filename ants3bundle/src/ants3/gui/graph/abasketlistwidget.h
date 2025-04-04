#ifndef ABASKETLISTWIDGET_H
#define ABASKETLISTWIDGET_H

#include <QListWidget>

#include <vector>

class ABasketListWidget : public QListWidget
{
    Q_OBJECT

public:
    ABasketListWidget(QWidget * parent = nullptr);

protected:
    void dropEvent(QDropEvent * event) override;

signals:
    void requestReorder(const std::vector<int> & indexes, int toRow);
};

#endif // ABASKETLISTWIDGET_H
