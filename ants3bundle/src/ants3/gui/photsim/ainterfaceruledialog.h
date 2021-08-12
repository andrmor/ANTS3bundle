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
    explicit AInterfaceRuleDialog(int matFrom, int matTo, QWidget * parent);
    ~AInterfaceRuleDialog();

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

    Ui::AInterfaceRuleDialog * ui           = nullptr;
    AInterfaceRule           * ovLocal      = nullptr;
    AOpticalOverrideTester   * TesterWindow = nullptr;
    int matFrom;
    int matTo;

    int customWidgetPositionInLayout = 5;
    QWidget * customWidget = nullptr;

    QSet<AInterfaceRule*> openedOVs;

    void updateGui();
    AInterfaceRule * findInOpended(const QString & ovType);
    void clearOpenedExcept(AInterfaceRule * keepOV);
};

#endif // AINTERFACERULEDIALOG_H
