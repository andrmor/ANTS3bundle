#include "aparticlesourcedialogbase.h"
#include "aparticlesourcerecord.h"
#include "ageobasedelegate.h"
#include "aonelinetextedit.h"
#include "aparticlesourcedialog.h"
#include "aparticlesourcedialog_ecomug.h"

AParticleSourceDialogBase * AParticleSourceDialogBase::factory(AParticleSourceRecordBase * source, QWidget * parent)
{
    AParticleSourceRecord_Standard * stSource = dynamic_cast<AParticleSourceRecord_Standard*>(source);
    if (stSource) return new AParticleSourceDialog(*stSource, parent);

    AParticleSourceRecord_EcoMug * muSource = dynamic_cast<AParticleSourceRecord_EcoMug*>(source);
    if (muSource) return new AParticleSourceDialog_EcoMug(*muSource, parent);

    return nullptr;
}

void AParticleSourceDialogBase::processGeoConstAwareEditFinished(AOneLineTextEdit *edit, QString &str, double &val, const QString &name, QWidget *parent, bool bForbidZero, bool bForbidNegative, bool bMakeHalf)
{
    double doubleVal = 0;
    QString stringVal;
    AGeoBaseDelegate::processEditBox(name, edit, doubleVal, stringVal, parent, bForbidZero, bForbidNegative, bMakeHalf);
    edit->updateTooltip();
    val = doubleVal;
    str = stringVal;
}

AParticleSourceDialogBase::AParticleSourceDialogBase(QWidget *parent) :
    QDialog{parent}
{}

