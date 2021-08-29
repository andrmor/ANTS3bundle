#include "aparticlesimwin.h"
#include "ui_aparticlesimwin.h"
#include "aparticlesimhub.h"
#include "aparticlesimsettings.h"
#include "ag4simulationsettings.h"
#include "guitools.h"

#include <QListWidget>
#include <QDebug>
#include <QCheckBox>
#include <QLineEdit>

AParticleSimWin::AParticleSimWin(QWidget *parent) :
    QMainWindow(parent),
    SimSet(AParticleSimHub::getInstance().Settings),
    G4SimSet(SimSet.G4Set),
    ui(new Ui::AParticleSimWin)
{
    ui->setupUi(this);
}

AParticleSimWin::~AParticleSimWin()
{
    delete ui;
}

void AParticleSimWin::updateGui()
{
    ui->lePhysicsList->setText(G4SimSet.PhysicsList);
    ui->cobRefPhysLists->setCurrentIndex(-1);

    for (auto& s : G4SimSet.Commands)
        ui->pteCommands->appendPlainText(s);

    for (auto& s : G4SimSet.SensitiveVolumes)
        ui->pteSensitiveVolumes->appendPlainText(s);

    ui->pteStepLimits->clear();   // !!!*** redo
    for (auto & key : G4SimSet.StepLimits.keys())
        ui->pteStepLimits->appendPlainText( QString("%1 %2").arg(key).arg(G4SimSet.StepLimits.value(key)) );

    ui->cbBinaryOutput->setChecked(G4SimSet.BinaryOutput);
    ui->sbPrecision->setValue(G4SimSet.Precision);

    ui->cbUseTSphys->setChecked(G4SimSet.UseTSphys);
}

// --- input ---

void AParticleSimWin::on_lePhysicsList_editingFinished()
{
    G4SimSet.PhysicsList = ui->lePhysicsList->text();
}
void AParticleSimWin::on_cobRefPhysLists_activated(int index)
{
     ui->lePhysicsList->setText( ui->cobRefPhysLists->itemText(index) );
     on_lePhysicsList_editingFinished();
}
void AParticleSimWin::on_cbUseTSphys_clicked(bool checked)
{
    G4SimSet.UseTSphys = checked;
}
void AParticleSimWin::on_pteCommands_textChanged()
{
    const QString t = ui->pteCommands->document()->toPlainText();
    G4SimSet.Commands = t.split('\n', Qt::SkipEmptyParts);
}
void AParticleSimWin::on_cbBinaryOutput_clicked(bool checked)
{
    G4SimSet.BinaryOutput = checked;
}
void AParticleSimWin::on_sbPrecision_editingFinished()
{
    G4SimSet.Precision = ui->sbPrecision->value();
}
void AParticleSimWin::on_pteSensitiveVolumes_textChanged()
{
    const QRegularExpression rx = QRegularExpression("(\\ |\\,|\\n|\\t)"); //separators: ' ' or ',' or 'n' or '\t'
    QString t = ui->pteSensitiveVolumes->document()->toPlainText();
    G4SimSet.SensitiveVolumes = t.split(rx, Qt::SkipEmptyParts);
}
void AParticleSimWin::on_pteStepLimits_textChanged()
{
    const QRegularExpression rx2 = QRegularExpression("(\\ |\\t)"); //separators: ' ' or '\t'

    G4SimSet.StepLimits.clear();
    const QString t = ui->pteStepLimits->document()->toPlainText();
    const QStringList sl = t.split('\n', Qt::SkipEmptyParts);
    for (const QString & str : sl)
    {
        QStringList f = str.split(rx2, Qt::SkipEmptyParts);
        if (f.size() != 2)
        {
            guitools::message("Bad format of step limits, it should be (new line for each):\nVolume_name Step_Limit");
            return;
        }
        QString vol = f[0];
        bool bOK;
        double step = f[1].toDouble(&bOK);
        if (!bOK)
        {
            guitools::message("Bad format of step limits: failed to convert to double value: " + f[1]);
            return;
        }
        G4SimSet.StepLimits[vol] = step;
    }
}

#include "aparticlesourcedialog.h"
#include "aparticlesourcerecord.h"
void AParticleSimWin::on_pbEditParticleSource_clicked()
{
    int isource = ui->lwDefinedParticleSources->currentRow();
    if (isource == -1)
    {
        guitools::message("Select a source to edit", this);
        return;
    }
    ASourceGenSettings & SourceGenSettings = SimSet.SourceGenSettings;
    const int numSources = SourceGenSettings.getNumSources();
    if (isource >= numSources)
    {
        guitools::message("Error - bad source index!", this);
        return;
    }

    AParticleSourceDialog * ParticleSourceDialog = new AParticleSourceDialog(SourceGenSettings.getSourceRecord(isource), this);

    int res = ParticleSourceDialog->exec(); // if detector is rebuild (this->readSimSettingsFromJson() is triggered), ParticleSourceDialog is signal-blocked and rejected
    if (res == QDialog::Rejected)
    {
        delete ParticleSourceDialog; ParticleSourceDialog = nullptr;
        return;
    }

    SourceGenSettings.replace(isource, ParticleSourceDialog->getResult());
    delete ParticleSourceDialog; ParticleSourceDialog = nullptr;

    AParticleSourceRecord * ps = SourceGenSettings.getSourceRecord(isource);
    ps->updateLimitedToMat();

//    on_pbUpdateSimConfig_clicked();

//    if (Detector->isGDMLempty())   !!!*** no need?
//    {
//        double XYm = 0;
//        double  Zm = 0;
//        for (int isource = 0; isource < numSources; isource++)
//        {
//            double msize =   ps->size1;
//            UpdateMax(msize, ps->size2);
//            UpdateMax(msize, ps->size3);

//            UpdateMax(XYm, fabs(ps->X0)+msize);
//            UpdateMax(XYm, fabs(ps->Y0)+msize);
//            UpdateMax(Zm,  fabs(ps->Z0)+msize);
//        }

//        double currXYm = Detector->Sandwich->getWorldSizeXY();
//        double  currZm = Detector->Sandwich->getWorldSizeZ();
//        if (XYm > currXYm || Zm > currZm)
//        {
//            //need to override
//            Detector->Sandwich->setWorldSizeFixed(true);
//            Detector->Sandwich->setWorldSizeXY( std::max(1.05*XYm, currXYm) );
//            Detector->Sandwich->setWorldSizeZ ( std::max(1.05*Zm,  currZm) );
//            ReconstructDetector();
//        }
//    }

//    if (ui->pbGunShowSource->isChecked()) ShowParticleSource_noFocus();
}

void AParticleSimWin::on_pbAddSource_clicked()
{
    AParticleSourceRecord* s = new AParticleSourceRecord();
    s->GunParticles.push_back(new GunParticleStruct());
    SimSet.SourceGenSettings.append(s);

//    on_pbUpdateSimConfig_clicked();
    updateListSources();
    ui->lwDefinedParticleSources->setCurrentRow(SimSet.SourceGenSettings.getNumSources() - 1);
}

void AParticleSimWin::on_pbCloneSource_clicked()
{
    int index = ui->lwDefinedParticleSources->currentRow();
    if (index == -1)
    {
        guitools::message("Select a source to clone", this);
        return;
    }

    bool ok = SimSet.SourceGenSettings.clone(index);
    if (!ok) return;

//    on_pbUpdateSimConfig_clicked();
    updateListSources();
    ui->lwDefinedParticleSources->setCurrentRow(index+1);
}

void AParticleSimWin::on_pbRemoveSource_clicked()
{
    int isource = ui->lwDefinedParticleSources->currentRow();
    if (isource == -1)
    {
        guitools::message("Select a source to remove", this);
        return;
    }

    const int numSources = SimSet.SourceGenSettings.getNumSources();
    if (isource >= numSources)
    {
        guitools::message("Error - bad source index!", this);
        return;
    }

    const QString SourceName = SimSet.SourceGenSettings.getSourceRecord(isource)->name;
    bool ok = guitools::confirm(QString("Remove source %0?").arg(SourceName), this);
    if (!ok) return;

    SimSet.SourceGenSettings.remove(isource);
    updateListSources();

//    on_pbUpdateSimConfig_clicked();
//    if (ui->pbGunShowSource->isChecked())
//    {
//        if (numSources == 0)
//        {
//            Detector->GeoManager->ClearTracks();
//            GeometryWindow->ShowGeometry(false);
//        }
//        else ShowParticleSource_noFocus();
//    }
}

void AParticleSimWin::updateListSources()
{
    ASourceGenSettings & SourceGenSettings = SimSet.SourceGenSettings;
    const int numSources = SourceGenSettings.getNumSources();

    int curRow = ui->lwDefinedParticleSources->currentRow();
    ui->lwDefinedParticleSources->clear();

    for (int i = 0; i < numSources; i++)
    {
        AParticleSourceRecord * pr = SourceGenSettings.getSourceRecord(i);
        QListWidgetItem * item = new QListWidgetItem();
        ui->lwDefinedParticleSources->addItem(item);

        QFrame* fr = new QFrame();
        fr->setFrameShape(QFrame::Box);
        QHBoxLayout* l = new QHBoxLayout();
        l->setContentsMargins(3, 2, 3, 2);
            QLabel* lab = new QLabel(pr->name);
            lab->setMinimumWidth(110);
            QFont f = lab->font();
            f.setBold(true);
            lab->setFont(f);
        l->addWidget(lab);
        l->addWidget(new QLabel(pr->getShapeString() + ','));
        l->addWidget(new QLabel( QString("%1 particle%2").arg(pr->GunParticles.size()).arg( pr->GunParticles.size()>1 ? "s" : "" ) ) );
        l->addStretch();

        l->addWidget(new QLabel("Fraction:"));
            QLineEdit* e = new QLineEdit(QString::number(pr->Activity));
            e->setMaximumWidth(50);
            e->setMinimumWidth(50);
            QDoubleValidator* val = new QDoubleValidator(this);
            val->setBottom(0);
            e->setValidator(val);
            QObject::connect(e, &QLineEdit::editingFinished, [pr, e, this]()
            {
                double newVal = e->text().toDouble();
                if (pr->Activity == newVal) return;
                pr->Activity = newVal;
                e->clearFocus();
//                emit this->RequestUpdateSimConfig();  // !!!*** need?
            });
        l->addWidget(e);

            double totAct = SourceGenSettings.getTotalActivity();
            double per = ( totAct == 0 ? 0 : 100.0 * pr->Activity / totAct );
            QString t = (per == 0 ? "-Off-" : QString("%1%").arg(per, 3, 'g', 3) );
            lab = new QLabel(t);
            lab->setMinimumWidth(45);
        l->addWidget(lab);

        fr->setLayout(l);
        item->setSizeHint(fr->sizeHint());
        ui->lwDefinedParticleSources->setItemWidget(item, fr);
        item->setSizeHint(fr->sizeHint());

        bool bVis = (numSources > 1);
        if (!bVis && pr->Activity == 0) bVis = true;
        e->setVisible(bVis);
    }

    if (curRow < 0 || curRow >= ui->lwDefinedParticleSources->count())
        curRow = 0;
    ui->lwDefinedParticleSources->setCurrentRow(curRow);
}

void AParticleSimWin::on_lwDefinedParticleSources_itemDoubleClicked(QListWidgetItem *)
{
    on_pbEditParticleSource_clicked();
}

