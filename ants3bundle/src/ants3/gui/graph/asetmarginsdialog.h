#ifndef ASETMARGINSDIALOG_H
#define ASETMARGINSDIALOG_H

#include "adrawmarginsrecord.h"
#include <QDialog>

namespace Ui {
class ASetMarginsDialog;
}

class ASetMarginsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ASetMarginsDialog(const ADrawMarginsRecord & rec, const ADrawMarginsRecord & defaultRec, QWidget * parent = nullptr);
    ~ASetMarginsDialog();

    bool isDefaultSelected() {return UseDefault;}
    ADrawMarginsRecord getResult() const;

private slots:
    void on_pbAccept_clicked();
    void on_pbCancel_clicked();
    void on_pbReset_clicked();

private:
    const ADrawMarginsRecord DefaultRec;

    Ui::ASetMarginsDialog * ui = nullptr;

    bool UseDefault = false;

    void updateGui(const ADrawMarginsRecord & rec);
};

#endif // ASETMARGINSDIALOG_H
