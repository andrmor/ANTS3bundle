#ifndef ASENSORWINDOW_H
#define ASENSORWINDOW_H

#include "aguiwindow.h"

namespace Ui {
class ASensorWindow;
}

class ASensorHub;
class TObject;

class ASensorWindow : public AGuiWindow
{
    Q_OBJECT

public:
    explicit ASensorWindow(QWidget * parent = nullptr);
    ~ASensorWindow();

    void updateGui();

private slots:
    //automatic
    void on_cobSensorType_currentIndexChanged(int index);

    //user actions
    void on_cobModel_activated(int index);
    void on_pbAddNewModel_clicked();
    void on_pbCloneModel_clicked();
    void on_pbRemoveModel_clicked();
    void on_leModelName_editingFinished();
    void on_ledEffectivePDE_editingFinished();
    void on_cobSensorType_activated(int index);
    void on_sbPixelsX_editingFinished();
    void on_sbPixelsY_editingFinished();

    void on_sbModelIndex_editingFinished();

    void on_cobAssignmentMode_activated(int index); // !!!*** consider more "soft" method instead of global rebuild

    void on_pbShowSensorsOfThisModel_clicked();

    void on_pbLoadPDE_clicked();
    void on_pbRemovePDE_clicked();
    void on_pbShowPDE_clicked();
    void on_pbShowBinnedPDE_clicked();

private:
    ASensorHub & SensHub;
    Ui::ASensorWindow * ui = nullptr;

    void updateNumPixels();
    void onModelIndexChanged();
    void updateHeader();
    void updatePdeButtons();

signals:
    void requestShowSensorModels(int iModel);
    void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);
};

#endif // ASENSORWINDOW_H
