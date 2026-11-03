#ifndef AREORDERLISTWIDGET_H
#define AREORDERLISTWIDGET_H

#include <QListWidget>

#include <vector>

class AReorderListWidget : public QListWidget
{
    Q_OBJECT

public:
    AReorderListWidget(QWidget * parent = nullptr);

protected:
    void dropEvent(QDropEvent * event) override;

signals:
    void requestReorder(const std::vector<int> & indexes, int toRow);
};

#endif // AREORDERLISTWIDGET_H
