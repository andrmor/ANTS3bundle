#ifndef APARTICLESOURCEDIALOG_ECOMUG_H
#define APARTICLESOURCEDIALOG_ECOMUG_H

#include "aparticlesourcedialogbase.h"
#include "asourceparticlegenerator.h"
#include "aparticlesourcerecord.h"

#include <QDialog>

namespace Ui {
class AParticleSourceDialog_EcoMug;
}

class TObject;
class QLineEdit;
class QComboBox;

class AParticleSourceDialog_EcoMug : public AParticleSourceDialogBase
{
    Q_OBJECT

public:
    explicit AParticleSourceDialog_EcoMug(const AParticleSourceRecord_EcoMug & Rec, QWidget * parent);
    ~AParticleSourceDialog_EcoMug();

    AParticleSourceRecordBase * getResult() override;

protected:
    virtual void closeEvent(QCloseEvent * e) override;

private slots:
    void on_pbAccept_clicked();
    void on_pbReject_clicked();

    void on_pbUpdateRecord_clicked();

    void on_pbGunTest_clicked();

    void on_pbShowSource_clicked(bool checked);

    void on_cobGeneratorShape_currentIndexChanged(int index);

private:
    AParticleSourceRecord_EcoMug         LocalRec;
    const AParticleSourceRecord_EcoMug & OriginalRec;

    Ui::AParticleSourceDialog_EcoMug * ui;

private:
    void storePersistentSettings();
    void restorePersistentSettings();
};

#endif // APARTICLESOURCEDIALOG_ECOMUG_H
