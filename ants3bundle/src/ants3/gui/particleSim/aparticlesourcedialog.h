#ifndef APARTICLESOURCEDIALOG_H
#define APARTICLESOURCEDIALOG_H

#include "aparticlesourcedialogbase.h"
#include "asourceparticlegenerator.h"
#include "aparticlesourcerecord.h"

#include <QDialog>

namespace Ui {
class AParticleSourceDialog;
}

class TObject;
class QLineEdit;
class QComboBox;

class AParticleSourceDialog : public AParticleSourceDialogBase
{
    Q_OBJECT

public:
    explicit AParticleSourceDialog(const AParticleSourceRecord_Standard & Rec, QWidget * parent);
    ~AParticleSourceDialog();

    AParticleSourceRecordBase * getResult() override;

protected:
    virtual void closeEvent(QCloseEvent * e) override;

private slots:
    void on_pbAccept_clicked();
    void on_pbReject_clicked();

    void on_pbUpdateRecord_clicked();

    void on_pbGunTest_clicked();
    void on_cobGunSourceType_currentIndexChanged(int index);
    void on_pbGunAddNew_clicked();
    void on_pbGunRemove_clicked();
    void on_lwGunParticles_currentRowChanged(int currentRow);
    void on_cobUnits_activated(int index);
    void on_cobEnergySigmaUnits_activated(int index);
    void on_sbLinkedTo_editingFinished();
    void on_ledLinkingProbability_editingFinished();
    void on_ledHalfLife_editingFinished();

    void on_pbGunShowSpectrum_clicked();
    void on_pbGunLoadSpectrum_clicked();
    void on_pbDeleteSpectrum_clicked();

    void on_pbShowSource_clicked(bool checked);
    void on_pbHelpParticle_clicked();

    void on_cobAngularMode_currentIndexChanged(int index);
    void on_cbAngularCutoff_toggled(bool checked);

    void on_pbShowAngular_clicked();
    void on_pbLoadAngular_clicked();
    void on_pbDeleteAngular_clicked();

    void on_leSourceLimitMaterial_textEdited(const QString &arg1);

    void on_pbTimeCustomShow_clicked();
    void on_pbTimeCustomLoad_clicked();
    void on_pbTimeCustomDelete_clicked();

    void on_pbAxialDistributionShow_clicked();

    void on_pbAxialDistributionLoad_clicked();

    void on_pbAxialDistributionRemove_clicked();

    void on_cobEnergy_currentIndexChanged(int index);

    void on_cbEnergyGaussBlur_toggled(bool checked);

    void on_leGunParticle_editingFinished();

private:
    AParticleSourceRecord_Standard         LocalRec;
    const AParticleSourceRecord_Standard & OriginalRec;

    Ui::AParticleSourceDialog * ui;

    bool bUpdateInProgress = false;

private:
    void updateListWidget();
    void updateParticleInfo();
    void updateColorLimitingMat();
    void updateCustomAngularButtons();
    void updateAxialButtons();
    void updateTimeButtons();
    void updateDirectionVisibility();
    void storePersistentSettings();
    void restorePersistentSettings();
    void updateFixedEnergyIndication(const AGunParticle & gRec);
    void updateFixedEnergy();
    void updateTimeWithUnitsIndication(double time_ns, AParticleSourceRecord_Standard::ETimeUnits prefUnits, QLineEdit * led, QComboBox * cob);
    void readTimeWithUnits(QLineEdit * led, QComboBox * cob, double & time_ns, AParticleSourceRecord_Standard::ETimeUnits &prefUnits);
};

#endif // APARTICLESOURCEDIALOG_H
