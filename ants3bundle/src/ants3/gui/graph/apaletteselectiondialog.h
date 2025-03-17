#ifndef APALETTESELECTIONDIALOG_H
#define APALETTESELECTIONDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class APaletteSelectionDialog;
}

class APaletteSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit APaletteSelectionDialog(QWidget *parent = nullptr);
    ~APaletteSelectionDialog();

private slots:
    void on_pbClose_clicked();
    void on_cob_activated(int index);

    void on_pbUseDefault_clicked();

private:
    Ui::APaletteSelectionDialog * ui = nullptr;

    std::vector<std::pair<QString,int>> Palettes;
    int DefaultIndex = 0;

signals:
    void requestRedraw();
};

#endif // APALETTESELECTIONDIALOG_H
