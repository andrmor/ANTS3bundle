#ifndef AADDTOBASKETDIALOG_H
#define AADDTOBASKETDIALOG_H

#include <QDialog>
#include <QStringList>

namespace Ui {
class AAddToBasketDialog;
}

class AAddToBasketDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AAddToBasketDialog(const QStringList & groups, QWidget * parent = nullptr);
    ~AAddToBasketDialog();

    QString getPath() const;
    bool    isBottom() const;
    QString getTitle() const;

private slots:
    void on_pbAccept_clicked();

    void on_pcCancel_clicked();

private:
    Ui::AAddToBasketDialog *ui;
};

#endif // AADDTOBASKETDIALOG_H
