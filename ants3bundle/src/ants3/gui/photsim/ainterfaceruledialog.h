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
class TObject;

class AInterfaceRuleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AInterfaceRuleDialog(AInterfaceRule * rule, int matFrom, int matTo, QWidget * parent); // !!!*** load
    ~AInterfaceRuleDialog();

    AInterfaceRule * getRule();

    int MatFrom;
    int MatTo;

private slots:
    void on_pbAccept_clicked();
    void on_pbCancel_clicked();
    void on_cobType_activated(int index);
    void on_pbTestOverride_clicked();

    void on_cobSurfaceModel_currentIndexChanged(int index);

    void on_cobSurfaceModel_activated(int index);

protected:
    void closeEvent(QCloseEvent * e);  // !!!*** saqve/load settings!

private:
    AMaterialHub      & MatHub;
    AInterfaceRuleHub & RuleHub;


    Ui::AInterfaceRuleDialog * ui           = nullptr;
    AInterfaceRule           * LocalRule    = nullptr;
    AOpticalOverrideTester   * TesterWindow = nullptr;

    int customWidgetPositionInLayout = 5;
    QWidget * customWidget = nullptr;

    QSet<AInterfaceRule*> TmpRules;

    void updateGui();
    AInterfaceRule * findInOpended(const QString & ovType);
    void clearTmpRules();

signals:
    // next four are retranslators from aopticaloverridetester
    void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);
    void requestDrawLegend(double x1, double y1, double x2, double y2, QString title);
    void requestClearGeometryViewer(); // also has to set current canvas to geometry view window!
    void requestShowTracks(); // also focuses the geo view window
    void closed(bool);

};

#endif // AINTERFACERULEDIALOG_H
