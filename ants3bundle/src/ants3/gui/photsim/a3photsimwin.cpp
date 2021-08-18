#include "a3photsimwin.h"
#include "aphotonsimhub.h"
#include "arandomhub.h"
#include "aphotonsimmanager.h"
#include "aphotonsimsettings.h"
#include "ui_a3photsimwin.h"
#include "a3config.h"
#include "guitools.h"
#include "agraphbuilder.h"

#include <QDebug>

#include "TH1D.h"
#include "TObject.h"

A3PhotSimWin::A3PhotSimWin(QWidget *parent) :
    QMainWindow(parent),
    SimSet(APhotonSimHub::getInstance().Settings),
    ui(new Ui::A3PhotSimWin)
{
    ui->setupUi(this);

    connect(&A3Config::getInstance(), &A3Config::requestUpdatePhotSimGui, this, &A3PhotSimWin::updateGui);

    QList<QPushButton*> listDummyButtons = this->findChildren<QPushButton*>();
    for (QPushButton * pb : qAsConst(listDummyButtons))
        if (pb->objectName().startsWith("pbd"))
            pb->setVisible(false);

    updateGui();
}

A3PhotSimWin::~A3PhotSimWin()
{
    delete ui;
}

void A3PhotSimWin::updateGui()
{
    int index;
    switch (SimSet.SimType)
    {
    default:
    case EPhotSimType::PhotonBombs       : index = 0; break;
    case EPhotSimType::FromEnergyDepo    : index = 1; break;
    case EPhotSimType::IndividualPhotons : index = 2; break;
    case EPhotSimType::FromLRFs          : index = 3; break;
    }
    ui->cobSimType->setCurrentIndex(index);

    updatePhotBombGui();

    updateGeneralSettingsGui();
}

void A3PhotSimWin::updatePhotBombGui()
{
    const APhotonsPerBombSettings & PS = SimSet.BombSet.PhotonsPerBomb;
    {
        int index;
        switch (PS.Mode)
        {
        default:
        case APhotonsPerBombSettings::Constant : index = 0; break;
        case APhotonsPerBombSettings::Poisson  : index = 1; break;
        case APhotonsPerBombSettings::Uniform  : index = 2; break;
        case APhotonsPerBombSettings::Normal   : index = 3; break;
        case APhotonsPerBombSettings::Custom   : index = 4; break;
        }
        ui->cobNumPhotonsMode->setCurrentIndex(index);

        ui->sbNumPhotons->setValue(PS.FixedNumber);
        ui->ledPoissonMean->setText( QString::number(PS.PoissonMean) );
        ui->sbNumMin->setValue(PS.UniformMin);
        ui->sbNumMax->setValue(PS.UniformMax);
        ui->ledGaussMean->setText( QString::number(PS.NormalMean) );
        ui->ledGaussSigma->setText( QString::number(PS.NormalSigma) );
    }

    {
        int index;
        switch (SimSet.BombSet.GenerationMode)
        {
        default:
        case EBombGen::Single : index = 0; break;
        case EBombGen::Grid   : index = 1; break;
        case EBombGen::Flood  : index = 2; break;
        case EBombGen::File   : index = 3; break;
        case EBombGen::Script : index = 4; break;
        }
        ui->cobNodeGenerationMode->setCurrentIndex(index);
    }

    //Single
    ui->ledSingleX->setText(QString::number(SimSet.BombSet.SingleSettings.Position[0]));
    ui->ledSingleY->setText(QString::number(SimSet.BombSet.SingleSettings.Position[1]));
    ui->ledSingleZ->setText(QString::number(SimSet.BombSet.SingleSettings.Position[2]));

    //Flood
    ui->sbFloodNumber->setValue(SimSet.BombSet.FloodSettings.Number);
    ui->cobFloodShape->setCurrentIndex(SimSet.BombSet.FloodSettings.Shape == AFloodSettings::Rectangular ? 0 : 1);
    ui->ledFloodXfrom->setText(QString::number(SimSet.BombSet.FloodSettings.Xfrom));
    ui->ledFloodXto->setText(QString::number(SimSet.BombSet.FloodSettings.Xto));
    ui->ledFloodYfrom->setText(QString::number(SimSet.BombSet.FloodSettings.Yfrom));
    ui->ledFloodYto->setText(QString::number(SimSet.BombSet.FloodSettings.Yto));
    ui->ledFloodCenterX->setText(QString::number(SimSet.BombSet.FloodSettings.X0));
    ui->ledFloodCenterY->setText(QString::number(SimSet.BombSet.FloodSettings.Y0));
    ui->ledFloodOuterDiameter->setText(QString::number(SimSet.BombSet.FloodSettings.OuterDiameter));
    ui->ledFloodInnerDiameter->setText(QString::number(SimSet.BombSet.FloodSettings.InnerDiameter));
    ui->cobFloodZmode->setCurrentIndex(SimSet.BombSet.FloodSettings.Zmode == AFloodSettings::Fixed ? 0 : 1);
    ui->ledFloodZ->setText(QString::number(SimSet.BombSet.FloodSettings.Zfixed));
    ui->ledFloodZfrom->setText(QString::number(SimSet.BombSet.FloodSettings.Zfrom));
    ui->ledFloodZto->setText(QString::number(SimSet.BombSet.FloodSettings.Zto));
}

void A3PhotSimWin::updateGeneralSettingsGui()
{
    ui->twGeneralOption->setEnabled(SimSet.SimType != EPhotSimType::FromLRFs);

    ui->cbWaveResolved->setChecked(SimSet.WaveSet.Enabled);
    ui->fWaveOptions->setEnabled(SimSet.WaveSet.Enabled);
    ui->ledWaveFrom->setText(QString::number(SimSet.WaveSet.From));
    ui->ledWaveTo  ->setText(QString::number(SimSet.WaveSet.To));
    ui->ledWaveStep->setText(QString::number(SimSet.WaveSet.Step));
    ui->labWaveNodes->setText(QString::number(SimSet.WaveSet.countNodes()));

    ui->sbMaxNumbPhTransitions->setValue(SimSet.OptSet.MaxPhotonTransitions);
    ui->cbRndCheckBeforeTrack->setChecked(SimSet.OptSet.CheckQeBeforeTracking);
}

void A3PhotSimWin::on_pbdWave_clicked()
{
    if (ui->ledWaveStep->text().toDouble() <= 0)
    {
        updateGeneralSettingsGui();
        guitools::message("Step should be positive!", this);
        return;
    }

    if (ui->ledWaveFrom->text().toDouble() > ui->ledWaveTo  ->text().toDouble())
    {
        updateGeneralSettingsGui();
        guitools::message("'From' should not be larger than 'To'!", this);
        return;
    }

    storeGeneralSettings();
    updateGeneralSettingsGui();
}

void A3PhotSimWin::storeGeneralSettings()
{
    SimSet.WaveSet.Enabled = ui->cbWaveResolved->isChecked();
    SimSet.WaveSet.From    = ui->ledWaveFrom->text().toDouble();
    SimSet.WaveSet.To      = ui->ledWaveTo  ->text().toDouble();
    SimSet.WaveSet.Step    = ui->ledWaveStep->text().toDouble();
}

void A3PhotSimWin::disableInterface(bool flag)
{
    ui->pbConfigureOutput->setDisabled(flag);
    ui->pbSimulate->setDisabled(flag);

    ui->progbSim->setEnabled(flag);
    ui->pbAbort->setEnabled(flag);
}

void A3PhotSimWin::on_sbMaxNumbPhTransitions_editingFinished()
{
    SimSet.OptSet.MaxPhotonTransitions  = ui->sbMaxNumbPhTransitions->value();
}

void A3PhotSimWin::on_cbRndCheckBeforeTrack_clicked()
{
    SimSet.OptSet.CheckQeBeforeTracking = ui->cbRndCheckBeforeTrack->isChecked();
}

void A3PhotSimWin::on_pbQEacceleratorHelp_clicked()
{
    guitools::message("TODO", this);
}

void A3PhotSimWin::on_cobSimType_activated(int index)
{
    switch (index)
    {
    default:
    case 0 : SimSet.SimType = EPhotSimType::PhotonBombs;       break;
    case 1 : SimSet.SimType = EPhotSimType::FromEnergyDepo;    break;
    case 2 : SimSet.SimType = EPhotSimType::IndividualPhotons; break;
    case 3 : SimSet.SimType = EPhotSimType::FromLRFs;          break;
    }
}

void A3PhotSimWin::on_cobNumPhotonsMode_activated(int index)
{
    APhotonsPerBombSettings & PS = SimSet.BombSet.PhotonsPerBomb;

    switch (index)
    {
    default:
    case 0: PS.Mode = APhotonsPerBombSettings::Constant; break;
    case 1: PS.Mode = APhotonsPerBombSettings::Poisson;  break;
    case 2: PS.Mode = APhotonsPerBombSettings::Uniform;  break;
    case 3: PS.Mode = APhotonsPerBombSettings::Normal;   break;
    case 4: PS.Mode = APhotonsPerBombSettings::Custom;   break;
    }
}

void A3PhotSimWin::on_cobNodeGenerationMode_activated(int index)
{
    switch (index)
    {
    default:
    case 0 : SimSet.BombSet.GenerationMode = EBombGen::Single; break;
    case 1 : SimSet.BombSet.GenerationMode = EBombGen::Grid;   break;
    case 2 : SimSet.BombSet.GenerationMode = EBombGen::Flood;  break;
    case 3 : SimSet.BombSet.GenerationMode = EBombGen::File;   break;
    case 4 : SimSet.BombSet.GenerationMode = EBombGen::Script; break;
    }
}

void A3PhotSimWin::on_ledSingleX_editingFinished()
{
    SimSet.BombSet.SingleSettings.Position[0] = ui->ledSingleX->text().toDouble();
}

void A3PhotSimWin::on_ledSingleY_editingFinished()
{
    SimSet.BombSet.SingleSettings.Position[1] = ui->ledSingleY->text().toDouble();
}

void A3PhotSimWin::on_ledSingleZ_editingFinished()
{
    SimSet.BombSet.SingleSettings.Position[2] = ui->ledSingleZ->text().toDouble();
}

void A3PhotSimWin::on_pbSimulate_clicked()
{
    SimSet.RunSet.Seed = INT_MAX * ARandomHub::getInstance().uniform();

    ui->progbSim->setValue(0);
    disableInterface(true);
    qApp->processEvents();

    APhotonSimManager & SimMan = APhotonSimManager::getInstance();
    bool ok = SimMan.simulate();
    if (!ok)
    {
        ui->progbSim->setValue(0);
        guitools::message("Simulation error:\n" + SimMan.ErrorString, this);
    }
    else
    {
        ui->progbSim->setValue(100);
    }

    disableInterface(false);

    if (ok && ui->cbAutoLoadResults->isChecked())
    {
        if (SimSet.RunSet.SaveTracks)
        {
            ui->leTracksFile->setText(SimSet.RunSet.OutputDirectory + '/' + SimSet.RunSet.FileNameTracks);
            on_pbLoadAndShowTracks_clicked();
        }

        if (SimSet.RunSet.SaveStatistics)
        {
            ui->leStatisticsFile->setText(SimSet.RunSet.OutputDirectory + '/' + SimSet.RunSet.FileNameStatistics);
            on_pbLoadAndShowStatistics_clicked();
        }

    }
}

void A3PhotSimWin::on_sbFloodNumber_editingFinished()
{
    SimSet.BombSet.FloodSettings.Number = ui->sbFloodNumber->value();
}
void A3PhotSimWin::on_cobFloodShape_activated(int index)
{
    SimSet.BombSet.FloodSettings.Shape = (index == 0 ? AFloodSettings::Rectangular : AFloodSettings::Ring);
}
void A3PhotSimWin::on_ledFloodXfrom_editingFinished()
{
    SimSet.BombSet.FloodSettings.Xfrom = ui->ledFloodXfrom->text().toDouble();
}
void A3PhotSimWin::on_ledFloodXto_editingFinished()
{
    SimSet.BombSet.FloodSettings.Xto   = ui->ledFloodXto->text().toDouble();
}
void A3PhotSimWin::on_ledFloodYfrom_editingFinished()
{
    SimSet.BombSet.FloodSettings.Yfrom = ui->ledFloodYfrom->text().toDouble();
}
void A3PhotSimWin::on_ledFloodYto_editingFinished()
{
    SimSet.BombSet.FloodSettings.Yto   = ui->ledFloodYto->text().toDouble();
}
void A3PhotSimWin::on_ledFloodCenterX_editingFinished()
{
    SimSet.BombSet.FloodSettings.X0 = ui->ledFloodCenterX->text().toDouble();
}
void A3PhotSimWin::on_ledFloodCenterY_editingFinished()
{
    SimSet.BombSet.FloodSettings.Y0 = ui->ledFloodCenterY->text().toDouble();
}
void A3PhotSimWin::on_ledFloodOuterDiameter_editingFinished()
{
    SimSet.BombSet.FloodSettings.OuterDiameter = ui->ledFloodOuterDiameter->text().toDouble();
}
void A3PhotSimWin::on_ledFloodInnerDiameter_editingFinished()
{
    SimSet.BombSet.FloodSettings.InnerDiameter = ui->ledFloodInnerDiameter->text().toDouble();
}
void A3PhotSimWin::on_cobFloodZmode_activated(int index)
{
    SimSet.BombSet.FloodSettings.Zmode = (index == 0 ? AFloodSettings::Fixed : AFloodSettings::Range);
}
void A3PhotSimWin::on_ledFloodZ_editingFinished()
{
    SimSet.BombSet.FloodSettings.Zfixed = ui->ledFloodZ->text().toDouble();
}
void A3PhotSimWin::on_ledFloodZfrom_editingFinished()
{
    SimSet.BombSet.FloodSettings.Zfrom = ui->ledFloodZfrom->text().toDouble();
}
void A3PhotSimWin::on_ledFloodZto_editingFinished()
{
    SimSet.BombSet.FloodSettings.Zto = ui->ledFloodZto->text().toDouble();
}


void A3PhotSimWin::on_sbNumPhotons_editingFinished()
{
    SimSet.BombSet.PhotonsPerBomb.FixedNumber = ui->sbNumPhotons->value();
}
void A3PhotSimWin::on_ledPoissonMean_editingFinished()
{
    SimSet.BombSet.PhotonsPerBomb.PoissonMean = ui->ledPoissonMean->text().toDouble();
}
void A3PhotSimWin::on_sbNumMin_editingFinished()
{
    SimSet.BombSet.PhotonsPerBomb.UniformMin = ui->sbNumMin->value();
}
void A3PhotSimWin::on_sbNumMax_editingFinished()
{
    SimSet.BombSet.PhotonsPerBomb.UniformMax = ui->sbNumMax->value();
}
void A3PhotSimWin::on_ledGaussSigma_editingFinished()
{
    SimSet.BombSet.PhotonsPerBomb.NormalSigma = ui->ledGaussSigma->text().toDouble();
}
void A3PhotSimWin::on_ledGaussMean_editingFinished()
{
    SimSet.BombSet.PhotonsPerBomb.NormalMean = ui->ledGaussMean->text().toDouble();
}
void A3PhotSimWin::on_pbNumDistShow_clicked()
{

}
void A3PhotSimWin::on_pbNumDistLoad_clicked()
{

}
void A3PhotSimWin::on_pbNumDistDelete_clicked()
{

}

#include "aphotonsimoutputdialog.h"
void A3PhotSimWin::on_pbConfigureOutput_clicked()
{
    APhotonSimOutputDialog dialog(this);
    dialog.exec();
}


// ========================= RESULTS ========================
#include "ajsontools.h"
#include "ageometryhub.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include "TGeoManager.h"
#include "TVirtualGeoTrack.h"
#include "TGeoTrack.h"

void A3PhotSimWin::on_pbSelectTracksFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file with track data", SimSet.RunSet.OutputDirectory);
    if (!fileName.isEmpty()) ui->leTracksFile->setText(fileName);
}

void A3PhotSimWin::on_pbLoadAndShowTracks_clicked()
{
    const QString FileName = ui->leTracksFile->text();
    QFile file(FileName);
    if(!file.open(QIODevice::ReadOnly | QFile::Text))
    {
        guitools::message("Could not open: " + FileName, this);
        return;
    }

    TGeoManager * GeoManager = AGeometryHub::getInstance().GeoManager;
    GeoManager->ClearTracks();

    QTextStream in(&file);
    while(!in.atEnd())
    {
        const QString line = in.readLine();
        if (line.startsWith('#')) continue;

        QJsonObject json = jstools::strToJson(line);
        QJsonArray ar;
        bool ok = jstools::parseJson(json, "P", ar);
        if (!ok)
        {
            guitools::message("Unknown file format!", this);
            return;
        }
        const bool bHit = (json.contains("h") ? true : false);
        const bool bSec = (json.contains("s") ? true : false);

        TGeoTrack * track = new TGeoTrack(1, 22);
        int Color = 7;
        if (bSec) Color = kMagenta;
        if (bHit) Color = 2;
        track->SetLineColor(Color);
        //track->SetLineWidth(th->Width);
        //track->SetLineStyle(th->Style);

        for (int iNode = 0; iNode < ar.size(); iNode++)
        {
            QJsonArray el = ar[iNode].toArray();
            if (el.size() < 3) continue; // !!!***
            track->AddPoint(el[0].toDouble(), el[1].toDouble(), el[2].toDouble(), 0);
        }
        if (track->GetNpoints() > 1) GeoManager->AddTrack(track);
        else delete track;
    }

    emit requestShowGeometry(); // !!!***
    emit requestShowTracks();
}

#include "astatisticshub.h"
void A3PhotSimWin::on_pbSelectStatisticsFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file with photon statistics", SimSet.RunSet.OutputDirectory);
    if (!fileName.isEmpty()) ui->leStatisticsFile->setText(fileName);
}
void A3PhotSimWin::on_pbLoadAndShowStatistics_clicked()
{
    const QString FileName = ui->leStatisticsFile->text();
    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, FileName);
    if(!ok)
    {
        guitools::message("Could not open: " + FileName, this);
        return;
    }

    APhotonStatistics & Stat = AStatisticsHub::getInstance().SimStat;
    Stat.readFromJson(json);

    const int sum = Stat.Absorbed + Stat.InterfaceRuleLoss + Stat.HitSensor + Stat.Escaped + Stat.LossOnGrid + Stat.TracingSkipped +
                    Stat.MaxTransitions + Stat.GeneratedOutside + Stat.MonitorKill;

    QString s;
    s = "Absorption (bulk): "      + QString::number(Stat.BulkAbsorption)       + "\n" +
        "Fresnel transmission: "   + QString::number(Stat.FresnelTransmitted)   + "\n" +
        "Fresnel reflection: "     + QString::number(Stat.FresnelReflected)     + "\n" +
        "InterfaceRule, loss: "    + QString::number(Stat.InterfaceRuleLoss)    + "\n" +
        "InterfaceRule, back: "    + QString::number(Stat.InterfaceRuleBack)    + "\n" +
        "InterfaceRule, forward: " + QString::number(Stat.InterfaceRuleForward) + "\n" +
        "Rayleigh: "               + QString::number(Stat.Rayleigh)             + "\n" +
        "Reemission: "             + QString::number(Stat.Reemission)           + "\n";

    ui->ptePhotonProcesses->clear();
    ui->ptePhotonProcesses->appendPlainText(s);

    s = "Absorbed: "               + QString::number(Stat.Absorbed)             + "\n" +
        "Escaped world: "          + QString::number(Stat.Escaped)              + "\n" +
        "Generated outside world: "+ QString::number(Stat.GeneratedOutside)     + "\n" +
        "Hit sensor: "             + QString::number(Stat.HitSensor)            + "\n" +
        "InterfaceRule loss: "     + QString::number(Stat.InterfaceRuleLoss)    + "\n" +
        "Max transitions reached: "+ QString::number(Stat.MaxTransitions)       + "\n" +
        "Monitor kill: "           + QString::number(Stat.MonitorKill)          + "\n" +
        "Optical grid loss: "      + QString::number(Stat.LossOnGrid)           + "\n" +
        "Tracing skipped: "        + QString::number(Stat.TracingSkipped)       + "\n" +
        "---\n"
        "Sum of all above: "       + QString::number(sum);

    ui->pteEndOfLife->clear();
    ui->pteEndOfLife->appendPlainText(s);
}
void A3PhotSimWin::on_pbShowTransitionDistr_clicked()
{
    APhotonStatistics & Stat = AStatisticsHub::getInstance().SimStat;
    if (Stat.TransitionDistr)
        requestDraw(Stat.TransitionDistr, "hist", false, true);
}
void A3PhotSimWin::on_pbShowWaveDistr_clicked()
{
    APhotonStatistics & Stat = AStatisticsHub::getInstance().SimStat;
    if (Stat.WaveDistr)
        requestDraw(Stat.WaveDistr, "hist", false, true);
}
void A3PhotSimWin::on_pbShowTimeDistr_clicked()
{
    APhotonStatistics & Stat = AStatisticsHub::getInstance().SimStat;
    if (Stat.TimeDistr)
        requestDraw(Stat.TimeDistr, "hist", false, true);
}
void A3PhotSimWin::on_pbShowAngleDistr_clicked()
{
    APhotonStatistics & Stat = AStatisticsHub::getInstance().SimStat;
    if (Stat.AngularDistr)
        requestDraw(Stat.AngularDistr, "hist", false, true);
}

// --- monitors ---

#include "amonitorhub.h"
#include "amonitor.h"

void A3PhotSimWin::on_pbChooseMonitorsFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file with data recorded by monitors", SimSet.RunSet.OutputDirectory);
    if (!fileName.isEmpty()) ui->leMonitorsFileName->setText(fileName);
}

void A3PhotSimWin::on_pbLoadMonitorsData_clicked()
{
    AMonitorHub & MonitorHub = AMonitorHub::getInstance();
    MonitorHub.clearData();

    const QString FileName = ui->leMonitorsFileName->text();
    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, FileName);
    if(!ok)
    {
        guitools::message("Could not open: " + FileName, this);
        return;
    }

    MonitorHub.appendDataFromJson(json);
    updateMonitorGui();
}

void A3PhotSimWin::updateMonitorGui()
{
    const AMonitorHub & MonitorHub = AMonitorHub::getConstInstance();
    const int numMonitors = MonitorHub.countMonitors();
    ui->labNumMonitors->setText(QString::number(numMonitors));
    const int numWithHits = MonitorHub.countMonitorsWithHits();
    ui->labMonitorsWithHits->setText(QString::number(numWithHits));

    ui->frMonitors->setVisible(numMonitors != 0);

    if (numMonitors > 0)
    {
        const int oldNum = ui->cobMonitor->currentIndex();

        ui->cobMonitor->clear();
        for (int i = 0; i < numMonitors; i++)
            ui->cobMonitor->addItem( QString("%1   index=%2").arg(MonitorHub.Monitors[i].Name).arg(i));

        if (oldNum >-1 && oldNum < numMonitors)
        {
            ui->cobMonitor->setCurrentIndex(oldNum);
            ui->sbMonitorIndex->setValue(oldNum);
        }
        else ui->sbMonitorIndex->setValue(0);

        const int imon = ui->cobMonitor->currentIndex();
        const AMonitor & Mon = *MonitorHub.Monitors[imon].Monitor;
        ui->leDetections->setText( QString::number(Mon.getHits()) );

        const bool bPhotonMode = (Mon.config.PhotonOrParticle == 0);
        ui->pbMonitorShowWave->setVisible(bPhotonMode);
        ui->pbShowWavelength->setVisible(bPhotonMode);
        ui->pbMonitorShowEnergy->setVisible(!bPhotonMode);
    }
}

void A3PhotSimWin::on_cobMonitor_activated(int)
{
    updateMonitorGui();
}

void A3PhotSimWin::on_sbMonitorIndex_editingFinished()
{
    int mon = ui->sbMonitorIndex->value();
    if (mon >= ui->cobMonitor->count()) mon = 0;
    ui->sbMonitorIndex->setValue(mon);
    if (mon < ui->cobMonitor->count()) ui->cobMonitor->setCurrentIndex(mon); //protection: can be empty
    updateMonitorGui();
}

void A3PhotSimWin::on_pbNextMonitor_clicked()
{
    int numMon = AMonitorHub::getConstInstance().countMonitors();
    if (numMon == 0) return;

    int iMon = ui->cobMonitor->currentIndex();
    int iMonStart = iMon;
    int hits;
    do
    {
        iMon++;
        if (iMon >= numMon) iMon = 0;
        if (iMon == iMonStart) return;
        hits = AMonitorHub::getConstInstance().Monitors[iMon].Monitor->getHits();
    }
    while (hits == 0);

    if (iMon < ui->cobMonitor->count()) ui->cobMonitor->setCurrentIndex(iMon);
    updateMonitorGui();
}

