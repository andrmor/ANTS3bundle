#ifndef AINTERFACERULEWIN_H
#define AINTERFACERULEWIN_H

#include <QMainWindow>

namespace Ui {
class AInterfaceRuleWin;
}

class AMaterialHub;
class AInterfaceRuleHub;

class AInterfaceRuleWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit AInterfaceRuleWin(QWidget *parent = nullptr);
    ~AInterfaceRuleWin();

    void updateGui();

private:
    const AMaterialHub & MatHub;
    AInterfaceRuleHub  & RuleHub;

    Ui::AInterfaceRuleWin * ui = nullptr;

    int NumMatRules = 0;

    void updateMatGui();
    void updateVolGui();

private slots:
    void onMatCellDoubleClicked();
    void onMatDialogAccepted();

    void on_pbAddNewVolumeRule_clicked();
    void onVolCellDoubleClicked();
};

#endif // AINTERFACERULEWIN_H
