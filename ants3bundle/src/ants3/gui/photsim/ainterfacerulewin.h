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

    void updateMatGui();

private slots:
    void onMatCellDoubleClicked();
    void onMatDialogAccepted();
};

#endif // AINTERFACERULEWIN_H
