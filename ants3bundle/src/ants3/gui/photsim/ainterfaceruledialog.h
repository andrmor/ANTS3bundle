#ifndef AINTERFACERULEDIALOG_H
#define AINTERFACERULEDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QSet>

namespace Ui {
class AInterfaceRuleDialog;
}

class AInterfaceRule;
class AOpticalOverrideTester;
class AMaterialHub;
class AInterfaceRuleHub;

class AInterfaceRuleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AInterfaceRuleDialog(AInterfaceRule * rule, int matFrom, int matTo, QWidget * parent);
    ~AInterfaceRuleDialog();

    AInterfaceRule * getRule();

private slots:
    void on_pbAccept_clicked();
    void on_pbCancel_clicked();
    void on_cobType_activated(int index);
    void on_pbTestOverride_clicked();

protected:
    void closeEvent(QCloseEvent * e);

private:
    AMaterialHub      & MatHub;
    AInterfaceRuleHub & RuleHub;

    int MatFrom;
    int MatTo;

    Ui::AInterfaceRuleDialog * ui           = nullptr;
    AInterfaceRule           * LocalRule    = nullptr;
    AOpticalOverrideTester   * TesterWindow = nullptr;

    int customWidgetPositionInLayout = 5;
    QWidget * customWidget = nullptr;

    QSet<AInterfaceRule*> TmpRules;

    void updateGui();
    AInterfaceRule * findInOpended(const QString & ovType);
    void clearTmpRules();
};

#endif // AINTERFACERULEDIALOG_H
