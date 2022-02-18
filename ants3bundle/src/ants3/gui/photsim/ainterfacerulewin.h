#ifndef AINTERFACERULEWIN_H
#define AINTERFACERULEWIN_H

#include "aguiwindow.h"

namespace Ui {
class AInterfaceRuleWin;
}

class AMaterialHub;
class AInterfaceRuleHub;

class AInterfaceRuleWin : public AGuiWindow
{
    Q_OBJECT

public:
    explicit AInterfaceRuleWin(QWidget * parent = nullptr);
    ~AInterfaceRuleWin();

    void updateGui();

private:
    const AMaterialHub & MatHub;
    AInterfaceRuleHub  & RuleHub;

    Ui::AInterfaceRuleWin * ui = nullptr;

    int NumMatRules = 0;
    bool BulkUpdate = false;

    void updateMatGui();
    void updateVolGui();

private slots:
    void onMatCellDoubleClicked();
    void onVolCellDoubleClicked();
    void onVolCellChanged();

    void on_pbAddNewVolumeRule_clicked();
};

#endif // AINTERFACERULEWIN_H
