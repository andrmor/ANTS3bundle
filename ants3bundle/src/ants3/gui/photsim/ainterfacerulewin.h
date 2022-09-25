#ifndef AINTERFACERULEWIN_H
#define AINTERFACERULEWIN_H

#include "aguiwindow.h"

#include <QString>

#include "TGString.h"

namespace Ui {
class AInterfaceRuleWin;
}

class AMaterialHub;
class AInterfaceRuleHub;
class AInterfaceRuleDialog;
class TObject;
class QTableWidgetItem;

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

    AInterfaceRuleDialog * RuleDialog = nullptr;
    QTableWidgetItem * itemDoubleClicked = nullptr;

    int NumMatRules = 0;
    bool BulkUpdate = false;

    TString LastFrom;
    TString LastTo;

    void updateMatGui();
    void updateVolGui();

    void configureInterfaceDialog();

private slots:
    void onMatCellDoubleClicked();
    void onVolCellDoubleClicked();
    void onVolCellChanged();
    void OnRuleDialogAccepted_Mat();
    void OnRuleDialogAccepted_Vol();

    void on_pbAddNewVolumeRule_clicked();

signals:
    // retranslated from AInterfaceRuleTester
    void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);
    void requestDrawLegend(double x1, double y1, double x2, double y2, QString title);
    void requestClearGeometryViewer(); // also has to set current canvas to geometry view window!
    void requestShowTracks();

};

#endif // AINTERFACERULEWIN_H
