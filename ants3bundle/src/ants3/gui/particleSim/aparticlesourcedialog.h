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
    void on_pbAccept_clicked();
    void on_pbReject_clicked();
    void on_pbGunTest_clicked();   // !!!***
    void on_cobGunSourceType_currentIndexChanged(int index);
    void on_pbGunAddNew_clicked();
    void on_pbGunRemove_clicked();
    void on_lwGunParticles_currentRowChanged(int currentRow);
    void on_cobUnits_activated(int index);
    void on_cbLinkedParticle_toggled(bool checked);
    void on_pbUpdateRecord_clicked(); // !!!***
    void on_cbLinkedParticle_clicked(bool checked);
    void on_sbLinkedTo_editingFinished();
    void on_ledLinkingProbability_editingFinished();
    void on_pbGunShowSpectrum_clicked(); // !!!***
    void on_pbGunLoadSpectrum_clicked(); // !!!***
    void on_pbDeleteSpectrum_clicked();

    void on_leGunParticle_editingFinished();

    void on_pbShowSource_clicked(bool checked);

    void on_pbHelpParticle_clicked();

signals:
    void delayClose();
    void requestTestParticleGun(AParticleGun * gun, int num);
    void requestShowSource();
    void requestDraw(TObject * obj, QString options, bool transferOwnership, bool focusWindow);

private:
    AParticleSourceRecord         LocalRec;
    const AParticleSourceRecord & OriginalRec;

    Ui::AParticleSourceDialog * ui;

    bool bUpdateInProgress = false;

private:
    void UpdateListWidget();
    void updateParticleInfo();
    void updateColorLimitingMat();
};

#endif // APARTICLESOURCEDIALOG_H
