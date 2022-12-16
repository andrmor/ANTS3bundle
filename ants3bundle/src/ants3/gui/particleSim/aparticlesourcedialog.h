#ifndef APARTICLESOURCEDIALOG_H
#define APARTICLESOURCEDIALOG_H

#include "asourceparticlegenerator.h"
#include "aparticlesourcerecord.h"

#include <QDialog>

namespace Ui {
class AParticleSourceDialog;
}

class MainWindow;
class TObject;

class AParticleSourceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AParticleSourceDialog(const AParticleSourceRecord & Rec, QWidget * parent);
    ~AParticleSourceDialog();

    AParticleSourceRecord & getResult(); //transfers ownership

protected:
    virtual void closeEvent(QCloseEvent * e) override;

private slots:
    void on_pbUpdateRecord_clicked();
    void on_pbAccept_clicked();
    void on_pbReject_clicked();
    void on_pbGunTest_clicked();
    void on_cobGunSourceType_currentIndexChanged(int index);
    void on_pbGunAddNew_clicked();
    void on_pbGunRemove_clicked();
    void on_lwGunParticles_currentRowChanged(int currentRow);
    void on_cobUnits_activated(int index);
    void on_sbLinkedTo_editingFinished();
    void on_ledLinkingProbability_editingFinished();
    void on_pbGunShowSpectrum_clicked();
    void on_pbGunLoadSpectrum_clicked();
    void on_pbDeleteSpectrum_clicked();
    void on_cbRangeBaseEnergyData_clicked(); // !!!*** dublicate code (on Load Spectrum uses the same check-up!)
    void on_pbShowSource_clicked(bool checked);
    void on_pbHelpParticle_clicked();
    void on_cobAngularMode_currentIndexChanged(int index);
    void on_cbAngularCutoff_toggled(bool checked);
    void on_pbShowAngular_clicked();
    void on_pbLoadAngular_clicked();
    void on_pbDeleteAngular_clicked();

signals:
    void delayClose();
    void requestTestParticleGun(AParticleGun * gun, int num, bool fillStatistics);
    void requestShowSource();
    void requestDraw(TObject * obj, QString options, bool transferOwnership, bool focusWindow);

private:
    AParticleSourceRecord         LocalRec;
    const AParticleSourceRecord & OriginalRec;

    Ui::AParticleSourceDialog * ui;

    bool bUpdateInProgress = false;

private:
    void updateListWidget();
    void updateParticleInfo();
    void updateColorLimitingMat();
    void updateCustomAngularButtons();
    void updateDirectionVisibility();
    void storePersistentSettings();
    void restorePersistentSettings();
};

#endif // APARTICLESOURCEDIALOG_H
