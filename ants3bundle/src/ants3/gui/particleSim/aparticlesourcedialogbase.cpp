#include "aparticlesourcedialogbase.h"

#include "aparticlesourcerecord.h"

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

AParticleSourceDialogBase::AParticleSourceDialogBase(QWidget *parent) :
    QDialog{parent}
{}

