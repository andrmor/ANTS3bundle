#ifndef AINTERFACERULEDIALOG_H
#define AINTERFACERULEDIALOG_H

#include <set>

#include <QDialog>
#include <QString>
#include <QStringList>

namespace Ui {
class AInterfaceRuleDialog;
}

class AInterfaceRule;
class AInterfaceRuleWidget;
class AInterfaceRuleTester;
class AMaterialHub;
class AInterfaceRuleHub;
class TObject;

class AInterfaceRuleDialog : public QDialog
{
    Q_OBJECT

public:
    AInterfaceRuleDialog(AInterfaceRule * rule, int matFrom, int matTo, QWidget * parent, QString volFrom = "", QString volTo = ""); // !!!*** load
    ~AInterfaceRuleDialog();

    AInterfaceRule * getRule();
    bool isSetSymmetric() const;

    int MatFrom;
    int MatTo;

    QString VolumeFrom;
    QString VolumeTo;

private slots:
    void on_cobType_currentIndexChanged(int index);
    void on_cobSurfaceModel_currentIndexChanged(int index);

    void on_pbAccept_clicked();
    void on_pbCancel_clicked();
    void on_pbInfo_clicked();
    void on_cobType_activated(int index);
    void on_pbTestOverride_clicked();
    void on_cobSurfaceModel_activated(int index);
    void on_lePolishGlisur_editingFinished();
    void on_leSigmaAlphaUnified_editingFinished();
    void on_pbLoadCustomNormalDistribution_clicked();
    void on_pbShowCustomNormalDistribution_clicked();
    void on_pbRemoveCustomNormalDistribution_clicked();
    void on_cbCustNorm_CorrectForOrientation_clicked(bool checked);
    void on_pbShowCustomNormalDistribution_customContextMenuRequested(const QPoint &pos);
    void on_cbKillBackRefracted_clicked(bool checked);

    void on_cbSymmetric_clicked(bool checked);

protected:
    void closeEvent(QCloseEvent * e);

private:
    AMaterialHub      & MatHub;
    AInterfaceRuleHub & RuleHub;

    Ui::AInterfaceRuleDialog * ui           = nullptr;
    AInterfaceRule           * Rule    = nullptr;
    AInterfaceRuleTester     * TesterWindow = nullptr;

    int customWidgetPositionInLayout = 4;
    AInterfaceRuleWidget * CustomWidget = nullptr;

    std::set<AInterfaceRule*> TmpRules;

    void updateGui();
    AInterfaceRule * findInOpended(const QString & ovType);
    void clearTmpRules();
    void updateCustomNormalButtons();
    void updateSymmetricVisuals();

signals:
    // signal retranslators from AInterfaceRuleTester and AInterfaceRuleWidget
    void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);
    void requestDrawLegend(double x1, double y1, double x2, double y2, QString title);
    void requestClearGeometryViewer(); // also has to set current canvas to geometry view window!
    void requestShowTracks(bool activateWindow = false);

    void closed(bool);

};

#endif // AINTERFACERULEDIALOG_H
