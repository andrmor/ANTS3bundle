#include "aparticlesimwin.h"
#include "ui_aparticlesimwin.h"
#include "aparticlesimhub.h"
#include "ag4simulationsettings.h"
#include "guitools.h"

#include <QListWidget>
#include <QDebug>
#include <QCheckBox>
#include <QLineEdit>

AParticleSimWin::AParticleSimWin(QWidget *parent) :
    QMainWindow(parent),
    G4SimSet(AParticleSimHub::getInstance().getInstance().Settings.G4Set),
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

