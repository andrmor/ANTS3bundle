#include "aphotsimwin.h"
#include "ageometryhub.h"
#include "aphotonsimhub.h"
#include "amonitorhub.h"
#include "amonitor.h"
#include "arandomhub.h"
#include "aphotonsimmanager.h"
#include "aphotonsimsettings.h"
#include "ui_aphotsimwin.h"
#include "aconfig.h"
#include "ajsontools.h"
#include "guitools.h"
#include "agraphbuilder.h"
#include "adispatcherinterface.h"
#include "aerrorhub.h"
#include "adepositionfilehandler.h"
#include "asensordrawwidget.h"
#include "asensorhub.h"
#include "aphotonbombfilehandler.h"
#include "anoderecord.h"

#include <QDebug>
#include <QLabel>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>
#include <QTabWidget>

#include "TH1D.h"
#include "TH2D.h"
#include "TObject.h"
#include "TGeoManager.h"
#include "TVirtualGeoTrack.h"
#include "TGeoTrack.h"

APhotSimWin::APhotSimWin(QWidget * parent) :
    AGuiWindow("PhotSim", parent),
    SimSet(APhotonSimHub::getInstance().Settings),
    MonitorHub(AMonitorHub::getConstInstance()),
    ui(new Ui::APhotSimWin)
{
    ui->setupUi(this);

    SignalsFileSettings = new AFileSettingsBase();
    SignalsFileHandler  = new AFileHandlerBase(*SignalsFileSettings);

    BombFileSettings = new ABombFileSettings();
    BombFileHandler  = new APhotonBombFileHandler(*BombFileSettings);

    ADispatcherInterface & Dispatcher = ADispatcherInterface::getInstance();
    connect(&Dispatcher, &ADispatcherInterface::updateProgress, this, &APhotSimWin::onProgressReceived);

    APhotonSimManager & SimMan = APhotonSimManager::getInstance(); // make a class ref here !!!***
    connect(&SimMan, &APhotonSimManager::requestUpdateResultsGUI, this, &APhotSimWin::showSimulationResults);

    QList<QPushButton*> listDummyButtons = findChildren<QPushButton*>();
    for (QPushButton * pb : qAsConst(listDummyButtons))
        if (pb->objectName().startsWith("pbd"))
            pb->setVisible(false);

    ui->cbSensorsAll->setChecked(true);

    YellowCircle = guitools::createColorCirclePixmap({15,15}, Qt::yellow);
    ui->labAdvancedBombOn->setPixmap(YellowCircle);
    ui->labSkipTracingON->setPixmap(YellowCircle);

    gvSensors = new ASensorDrawWidget(this);
    QVBoxLayout * lV = new QVBoxLayout(ui->frSensorDraw);
    lV->setContentsMargins(2,2,2,2);
    lV->addWidget(gvSensors);

    updateGui();
}

APhotSimWin::~APhotSimWin()
{
    delete ui;

    // !!!*** delete dynamic members!
}

void APhotSimWin::updateGui()
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
    updateDepoGui();
    updateBombFileGui();
    updatePhotonFileGui();

    updateGeneralSettingsGui();
}

void APhotSimWin::updatePhotBombGui()
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
        }
        ui->cobNodeGenerationMode->setCurrentIndex(index);
    }

    //Single
    const ASingleSettings & sset = SimSet.BombSet.SingleSettings;
    ui->ledSingleX->setText(QString::number(sset.Position[0]));
    ui->ledSingleY->setText(QString::number(sset.Position[1]));
    ui->ledSingleZ->setText(QString::number(sset.Position[2]));

    //Grid
    const AGridSettings & g = SimSet.BombSet.GridSettings;
    ui->ledOriginX->setText(QString::number(g.X0));
    ui->ledOriginY->setText(QString::number(g.Y0));
    ui->ledOriginZ->setText(QString::number(g.Z0));
    ui->cbSecondAxis->setChecked(g.ScanRecords[1].bEnabled);
    ui->cbThirdAxis-> setChecked(g.ScanRecords[2].bEnabled);
    for (int i = 0; i < 3; i++)
    {
        QLineEdit *leDX, *leDY, *leDZ;
        QSpinBox  *sbNodes;
        QComboBox *cobBiDir;
        switch (i)
        {
        case 0: leDX = ui->led0X; leDY = ui->led0Y; leDZ = ui->led0Z; sbNodes = ui->sb0nodes; cobBiDir = ui->cob0dir; break;
        case 1: leDX = ui->led1X; leDY = ui->led1Y; leDZ = ui->led1Z; sbNodes = ui->sb1nodes; cobBiDir = ui->cob1dir; break;
        case 2: leDX = ui->led2X; leDY = ui->led2Y; leDZ = ui->led2Z; sbNodes = ui->sb2nodes; cobBiDir = ui->cob2dir; break;
        }
        const APhScanRecord & r = g.ScanRecords[i];
        leDX->setText(QString::number(r.DX));
        leDY->setText(QString::number(r.DY));
        leDZ->setText(QString::number(r.DZ));
        sbNodes->setValue(r.Nodes);
        cobBiDir->setCurrentIndex(r.bBiDirect ? 1 : 0);
    }

    //Flood
    const AFloodSettings & fset = SimSet.BombSet.FloodSettings;
    ui->sbFloodNumber->setValue(fset.Number);
    ui->cobFloodShape->setCurrentIndex(fset.Shape == AFloodSettings::Rectangular ? 0 : 1);
    ui->ledFloodXfrom->setText(QString::number(fset.Xfrom));
    ui->ledFloodXto->setText(QString::number(fset.Xto));
    ui->ledFloodYfrom->setText(QString::number(fset.Yfrom));
    ui->ledFloodYto->setText(QString::number(fset.Yto));
    ui->ledFloodCenterX->setText(QString::number(fset.X0));
    ui->ledFloodCenterY->setText(QString::number(fset.Y0));
    ui->ledFloodOuterDiameter->setText(QString::number(fset.OuterDiameter));
    ui->ledFloodInnerDiameter->setText(QString::number(fset.InnerDiameter));
    ui->cobFloodZmode->setCurrentIndex(fset.Zmode == AFloodSettings::Fixed ? 0 : 1);
    ui->ledFloodZ->setText(QString::number(fset.Zfixed));
    ui->ledFloodZfrom->setText(QString::number(fset.Zfrom));
    ui->ledFloodZto->setText(QString::number(fset.Zto));

    updateAdvancedBombIndicator();
}

void APhotSimWin::updateDepoGui()
{
    ui->leDepositionFile->setText(SimSet.DepoSet.FileName);
    ui->labDepositionFileFormat->setText(SimSet.DepoSet.getFormatName());

    QString strEvents = "--";
    if (SimSet.DepoSet.isValidated()) strEvents = QString::number(SimSet.DepoSet.NumEvents);
    ui->labDepositionEvents->setText(strEvents);

    ui->cbPrimaryScint->setChecked(SimSet.DepoSet.Primary);
    ui->cbSecondaryScint->setChecked(SimSet.DepoSet.Secondary);
}

void APhotSimWin::updateBombFileGui()
{
    const ABombFileSettings & s = SimSet.BombSet.BombFileSettings;

    ui->leNodeFileName->setText(s.FileName);
    ui->labNodeFileFormat->setText(s.getFormatName());

    QString strEvents = "--";
    if (s.isValidated()) strEvents = QString::number(s.NumEvents);
    ui->labNodeFileEvents->setText(strEvents);
}

void APhotSimWin::updatePhotonFileGui()
{
    const APhotonFileSettings & s = SimSet.PhotFileSet;

    ui->leSinglePhotonsFile->setText(s.FileName);
    ui->labSinglePhotonsFileFormat->setText(s.getFormatName());

    QString strEvents = "--";
    if (s.isValidated()) strEvents = QString::number(s.NumEvents);
    ui->labSinglePhotonsEvents->setText(strEvents);
}

void APhotSimWin::updateGeneralSettingsGui()
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

void APhotSimWin::on_pbdWave_clicked()
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

void APhotSimWin::storeGeneralSettings()
{
    SimSet.WaveSet.Enabled = ui->cbWaveResolved->isChecked();
    SimSet.WaveSet.From    = ui->ledWaveFrom->text().toDouble();
    SimSet.WaveSet.To      = ui->ledWaveTo  ->text().toDouble();
    SimSet.WaveSet.Step    = ui->ledWaveStep->text().toDouble();
}

void APhotSimWin::disableInterface(bool flag)
{
    ui->pbConfigureOutput->setDisabled(flag);
    ui->pbSimulate->setDisabled(flag);

    ui->progbSim->setEnabled(flag);
    ui->pbAbort->setEnabled(flag);

    qApp->processEvents();
}

void APhotSimWin::onProgressReceived(double progress)
{
    if (!ui->progbSim->isEnabled()) return; // simulation is not running

    ui->progbSim->setValue(progress * 100.0);
}

void APhotSimWin::on_sbMaxNumbPhTransitions_editingFinished()
{
    SimSet.OptSet.MaxPhotonTransitions  = ui->sbMaxNumbPhTransitions->value();
}

void APhotSimWin::on_cbRndCheckBeforeTrack_clicked()
{
    SimSet.OptSet.CheckQeBeforeTracking = ui->cbRndCheckBeforeTrack->isChecked();
}

void APhotSimWin::on_pbQEacceleratorHelp_clicked()
{
    QString str;
    str += "In this mode first the maximum detection efficiency over all sensors is calculated. "
           "Before tracing each photon, a random number is generated "
           "and the max det.eff. is checked against it. If the generated number is larger, "
           "there is no chance that the photon will be detected, so tracing is skipped.\n\n"
           "WARNING: do NOT use this mode if you are interested in statistics of traced photons "
           "as it will be distorted!";
    guitools::message(str, this);
}

void APhotSimWin::on_cobSimType_activated(int index)
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

void APhotSimWin::on_cobNumPhotonsMode_activated(int index)
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

void APhotSimWin::on_cobNodeGenerationMode_activated(int index)
{
    switch (index)
    {
    default:
    case 0 : SimSet.BombSet.GenerationMode = EBombGen::Single; break;
    case 1 : SimSet.BombSet.GenerationMode = EBombGen::Grid;   break;
    case 2 : SimSet.BombSet.GenerationMode = EBombGen::Flood;  break;
    case 3 : SimSet.BombSet.GenerationMode = EBombGen::File;   break;
    }
}

void APhotSimWin::on_ledSingleX_editingFinished()
{
    SimSet.BombSet.SingleSettings.Position[0] = ui->ledSingleX->text().toDouble();
}

void APhotSimWin::on_ledSingleY_editingFinished()
{
    SimSet.BombSet.SingleSettings.Position[1] = ui->ledSingleY->text().toDouble();
}

void APhotSimWin::on_ledSingleZ_editingFinished()
{
    SimSet.BombSet.SingleSettings.Position[2] = ui->ledSingleZ->text().toDouble();
}

void APhotSimWin::on_pbSimulate_clicked()
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
        guitools::message("Simulation error:\n" + AErrorHub::getQError(), this);
        if ( AErrorHub::getQError().contains("Exchange directory") ) emit requestConfigureExchangeDir();
    }
    else
    {
        ui->progbSim->setValue(100);
    }

    disableInterface(false);

    TGeoManager * GeoManager = AGeometryHub::getInstance().GeoManager;
    GeoManager->ClearTracks();
    emit requestClearGeoMarkers(0);
    emit requestShowGeometry(false);

    if (ok) showSimulationResults();

    updateDepoGui(); // file was force-checked in this mode
}

void APhotSimWin::showSimulationResults()
{
    if (ui->cbAutoLoadResults->isChecked())
    {
        ui->leResultsWorkingDir->setText(SimSet.RunSet.OutputDirectory);

        on_pbLoadAllResults_clicked();

        /*
        if (SimSet.RunSet.SaveTracks)
        {
            ui->leTracksFile->setText(SimSet.RunSet.FileNameTracks);
            on_pbLoadAndShowTracks_clicked();
        }

        if (SimSet.RunSet.SaveMonitors)
        {
            ui->leMonitorsFileName->setText(SimSet.RunSet.FileNameMonitors);
            on_pbLoadMonitorsData_clicked();
        }

        if (SimSet.RunSet.SaveStatistics)
        {
            ui->leStatisticsFile->setText(SimSet.RunSet.FileNameStatistics);
            on_pbLoadAndShowStatistics_clicked();
        }

        if (SimSet.RunSet.SavePhotonBombs)
        {
            ui->leBombsFile->setText(SimSet.RunSet.FileNamePhotonBombs);
            on_pbShowBombsMultiple_clicked();
        }
        */
    }
}

void APhotSimWin::on_pbLoadAllResults_clicked()
{
    // !!!*** check logic - load only what was configured in sim!

    ui->sbEvent->setValue(0);

    APhotSimRunSettings Set;

    ui->leStatisticsFile->setText(Set.FileNameStatistics);
    loadStatistics(true);

    ui->leMonitorsFileName->setText(Set.FileNameMonitors);
    loadMonitorsData(true);

    ui->leSensorSigFileName->setText(Set.FileNameSensorSignals);
    reshapeSensorSignalTable();
    showSensorSignals(true);

    ui->leBombsFile->setText(Set.FileNamePhotonBombs);
    on_pbShowBombsMultiple_clicked();

    ui->leTracksFile->setText(Set.FileNameTracks);
    loadTracks(true);
}

void APhotSimWin::reshapeSensorSignalTable()
{
    ui->twSensorTable->clear();

    const int numColumns = ui->sbSensorTableColumns->value();
    ui->twSensorTable->setColumnCount(numColumns);

    const int numSensors = ASensorHub::getConstInstance().countSensors();
    int numRows = numSensors / numColumns;
    if (numRows * numColumns < numSensors) numRows++;
    ui->twSensorTable->setRowCount(numRows);

    ui->twSensorTable->verticalHeader()->setVisible(false);
    ui->twSensorTable->horizontalHeader()->setVisible(false);
}

void APhotSimWin::on_sbSensorTableColumns_editingFinished()
{
    reshapeSensorSignalTable();
    showSensorSignals(false);
}

void APhotSimWin::on_sbFloodNumber_editingFinished()
{
    SimSet.BombSet.FloodSettings.Number = ui->sbFloodNumber->value();
}
void APhotSimWin::on_cobFloodShape_activated(int index)
{
    SimSet.BombSet.FloodSettings.Shape = (index == 0 ? AFloodSettings::Rectangular : AFloodSettings::Ring);
}
void APhotSimWin::on_ledFloodXfrom_editingFinished()
{
    SimSet.BombSet.FloodSettings.Xfrom = ui->ledFloodXfrom->text().toDouble();
}
void APhotSimWin::on_ledFloodXto_editingFinished()
{
    SimSet.BombSet.FloodSettings.Xto   = ui->ledFloodXto->text().toDouble();
}
void APhotSimWin::on_ledFloodYfrom_editingFinished()
{
    SimSet.BombSet.FloodSettings.Yfrom = ui->ledFloodYfrom->text().toDouble();
}
void APhotSimWin::on_ledFloodYto_editingFinished()
{
    SimSet.BombSet.FloodSettings.Yto   = ui->ledFloodYto->text().toDouble();
}
void APhotSimWin::on_ledFloodCenterX_editingFinished()
{
    SimSet.BombSet.FloodSettings.X0 = ui->ledFloodCenterX->text().toDouble();
}
void APhotSimWin::on_ledFloodCenterY_editingFinished()
{
    SimSet.BombSet.FloodSettings.Y0 = ui->ledFloodCenterY->text().toDouble();
}
void APhotSimWin::on_ledFloodOuterDiameter_editingFinished()
{
    SimSet.BombSet.FloodSettings.OuterDiameter = ui->ledFloodOuterDiameter->text().toDouble();
}
void APhotSimWin::on_ledFloodInnerDiameter_editingFinished()
{
    SimSet.BombSet.FloodSettings.InnerDiameter = ui->ledFloodInnerDiameter->text().toDouble();
}
void APhotSimWin::on_cobFloodZmode_activated(int index)
{
    SimSet.BombSet.FloodSettings.Zmode = (index == 0 ? AFloodSettings::Fixed : AFloodSettings::Range);
}
void APhotSimWin::on_ledFloodZ_editingFinished()
{
    SimSet.BombSet.FloodSettings.Zfixed = ui->ledFloodZ->text().toDouble();
}
void APhotSimWin::on_ledFloodZfrom_editingFinished()
{
    SimSet.BombSet.FloodSettings.Zfrom = ui->ledFloodZfrom->text().toDouble();
}
void APhotSimWin::on_ledFloodZto_editingFinished()
{
    SimSet.BombSet.FloodSettings.Zto = ui->ledFloodZto->text().toDouble();
}

void APhotSimWin::on_sbNumPhotons_editingFinished()
{
    SimSet.BombSet.PhotonsPerBomb.FixedNumber = ui->sbNumPhotons->value();
}
void APhotSimWin::on_ledPoissonMean_editingFinished()
{
    SimSet.BombSet.PhotonsPerBomb.PoissonMean = ui->ledPoissonMean->text().toDouble();
}
void APhotSimWin::on_sbNumMin_editingFinished()
{
    SimSet.BombSet.PhotonsPerBomb.UniformMin = ui->sbNumMin->value();
}
void APhotSimWin::on_sbNumMax_editingFinished()
{
    SimSet.BombSet.PhotonsPerBomb.UniformMax = ui->sbNumMax->value();
}
void APhotSimWin::on_ledGaussSigma_editingFinished()
{
    SimSet.BombSet.PhotonsPerBomb.NormalSigma = ui->ledGaussSigma->text().toDouble();
}
void APhotSimWin::on_ledGaussMean_editingFinished()
{
    SimSet.BombSet.PhotonsPerBomb.NormalMean = ui->ledGaussMean->text().toDouble();
}
void APhotSimWin::on_pbNumDistShow_clicked()
{

}
void APhotSimWin::on_pbNumDistLoad_clicked()
{

}
void APhotSimWin::on_pbNumDistDelete_clicked()
{

}

#include "aphotonsimoutputdialog.h"
void APhotSimWin::on_pbConfigureOutput_clicked()
{
    APhotonSimOutputDialog dialog(this);
    dialog.exec();
}

// ========================= RESULTS ========================

void APhotSimWin::on_pbSelectTracksFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file with track data", SimSet.RunSet.OutputDirectory);
    if (!fileName.isEmpty()) ui->leTracksFile->setText(fileName);
}

void APhotSimWin::on_pbLoadAndShowTracks_clicked()
{
    loadTracks(false);
}

void APhotSimWin::loadTracks(bool suppressMessage)
{
    QString FileName = ui->leTracksFile->text();
    if (!FileName.contains('/')) FileName = ui->leResultsWorkingDir->text() + '/' + FileName;

    QFile file(FileName);
    if(!file.open(QIODevice::ReadOnly | QFile::Text))
    {
        if (!suppressMessage) guitools::message("Could not open: " + FileName, this);
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
void APhotSimWin::on_pbSelectStatisticsFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file with photon statistics", SimSet.RunSet.OutputDirectory);
    if (!fileName.isEmpty()) ui->leStatisticsFile->setText(fileName);
}

void APhotSimWin::on_pbLoadAndShowStatistics_clicked()
{
    loadStatistics(false);
}

void APhotSimWin::loadStatistics(bool suppressMessage)
{
    QString FileName = ui->leStatisticsFile->text();
    if (!FileName.contains('/')) FileName = ui->leResultsWorkingDir->text() + '/' + FileName;

    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, FileName);
    if (!ok)
    {
        if (!suppressMessage) guitools::message("Could not open: " + FileName, this);
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
void APhotSimWin::on_pbShowTransitionDistr_clicked()
{
    APhotonStatistics & Stat = AStatisticsHub::getInstance().SimStat;
    if (Stat.TransitionDistr)
        emit requestDraw(Stat.TransitionDistr, "hist", false, true);
}
void APhotSimWin::on_pbShowWaveDistr_clicked()
{
    APhotonStatistics & Stat = AStatisticsHub::getInstance().SimStat;
    if (Stat.WaveDistr)
        emit requestDraw(Stat.WaveDistr, "hist", false, true);
}
void APhotSimWin::on_pbShowTimeDistr_clicked()
{
    APhotonStatistics & Stat = AStatisticsHub::getInstance().SimStat;
    if (Stat.TimeDistr)
        emit requestDraw(Stat.TimeDistr, "hist", false, true);
}
void APhotSimWin::on_pbShowAngleDistr_clicked()
{
    APhotonStatistics & Stat = AStatisticsHub::getInstance().SimStat;
    if (Stat.AngularDistr)
        emit requestDraw(Stat.AngularDistr, "hist", false, true);
}

// --- monitors ---

#include "amonitorhub.h"
#include "amonitor.h"

void APhotSimWin::on_pbChooseMonitorsFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file with data recorded by monitors", SimSet.RunSet.OutputDirectory);
    if (!fileName.isEmpty()) ui->leMonitorsFileName->setText(fileName);
}

void APhotSimWin::on_pbLoadMonitorsData_clicked()
{
    loadMonitorsData(false);
}

void APhotSimWin::loadMonitorsData(bool suppressMessage)
{
    AMonitorHub & MonitorHub = AMonitorHub::getInstance();
    MonitorHub.clearData(AMonitorHub::Photon);

    QString FileName = ui->leMonitorsFileName->text();
    if (!FileName.contains('/')) FileName = ui->leResultsWorkingDir->text() + '/' + FileName;

    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, FileName);
    if(!ok)
    {
        if (!suppressMessage) guitools::message("Could not open: " + FileName, this);
        return;
    }

    QString err = MonitorHub.appendDataFromJson(json, AMonitorHub::Photon);
    if (!err.isEmpty()) guitools::message(err, this);
    updateMonitorGui();
}

void APhotSimWin::updateMonitorGui()
{
    const int numMonitors = MonitorHub.countMonitors(AMonitorHub::Photon);
    ui->labNumMonitors->setText(QString::number(numMonitors));
    const int numWithHits = MonitorHub.countMonitorsWithHits(AMonitorHub::Photon);
    ui->labMonitorsWithHits->setText(QString::number(numWithHits));

    ui->frMonitors->setVisible(numMonitors != 0);

    if (numMonitors > 0)
    {
        const int oldNum = ui->cobMonitor->currentIndex();

        ui->cobMonitor->clear();
        for (int i = 0; i < numMonitors; i++)
            //ui->cobMonitor->addItem( QString("%1   index=%2").arg(MonitorHub.Monitors[i].Name).arg(i));
            ui->cobMonitor->addItem(MonitorHub.PhotonMonitors[i].Name);

        if (oldNum >-1 && oldNum < numMonitors)
        {
            ui->cobMonitor->setCurrentIndex(oldNum);
            ui->sbMonitorIndex->setValue(oldNum);
        }
        else ui->sbMonitorIndex->setValue(0);

        const int imon = ui->cobMonitor->currentIndex();
        const AMonitor & Mon = *MonitorHub.PhotonMonitors[imon].Monitor;
        ui->leDetections->setText( QString::number(Mon.getHits()) );

        ui->pbMonitorShowXY->setEnabled(Mon.xy);
        ui->pbMonitorShowTime->setEnabled(Mon.time);
        ui->pbMonitorShowAngle->setEnabled(Mon.angle);
        ui->pbMonitorShowWaveIndex->setEnabled(Mon.wave);
        ui->pbMonitorShowWavelength->setEnabled(Mon.wave);
    }
}

void APhotSimWin::on_cobMonitor_activated(int)
{
    updateMonitorGui();
}

void APhotSimWin::on_sbMonitorIndex_editingFinished()
{
    int mon = ui->sbMonitorIndex->value();
    if (mon >= ui->cobMonitor->count()) mon = 0;
    ui->sbMonitorIndex->setValue(mon);
    if (mon < ui->cobMonitor->count()) ui->cobMonitor->setCurrentIndex(mon); //protection: can be empty
    updateMonitorGui();
}

void APhotSimWin::on_pbNextMonitor_clicked()
{
    int numMon = MonitorHub.countMonitors(AMonitorHub::Photon);
    if (numMon == 0) return;

    int iMon = ui->cobMonitor->currentIndex();
    int iMonStart = iMon;
    int hits;
    do
    {
        iMon++;
        if (iMon >= numMon) iMon = 0;
        if (iMon == iMonStart) return;
        hits = MonitorHub.PhotonMonitors[iMon].Monitor->getHits();
    }
    while (hits == 0);

    if (iMon < ui->cobMonitor->count()) ui->cobMonitor->setCurrentIndex(iMon);
    updateMonitorGui();
}

void APhotSimWin::on_pbMonitorShowAngle_clicked()
{
    const int numMonitors = MonitorHub.countMonitors(AMonitorHub::Photon);
    const int iMon = ui->cobMonitor->currentIndex();
    if (iMon >=0 && iMon < numMonitors)
    {
        TH1D * h = MonitorHub.PhotonMonitors[iMon].Monitor->angle;
        h->GetXaxis()->SetTitle("Angle of incidence, deg");
        emit requestDraw(h, "hist", false, true);
    }
}

void APhotSimWin::on_pbMonitorShowXY_clicked()
{
    const int numMonitors = MonitorHub.countMonitors(AMonitorHub::Photon);
    const int iMon = ui->cobMonitor->currentIndex();
    if (iMon >=0 && iMon < numMonitors)
    {
        TH2D * h = MonitorHub.PhotonMonitors[iMon].Monitor->xy;
        h->GetXaxis()->SetTitle("Local X, mm");
        h->GetYaxis()->SetTitle("Local Y, mm");
        emit requestDraw(h, "colz", false, true);
    }
}

void APhotSimWin::on_pbMonitorShowTime_clicked()
{
    const int numMonitors = MonitorHub.countMonitors(AMonitorHub::Photon);
    const int iMon = ui->cobMonitor->currentIndex();
    if (iMon >=0 && iMon < numMonitors)
    {
        TH1D * h = MonitorHub.PhotonMonitors[iMon].Monitor->time;
        TString title = "Time, ";
        title += MonitorHub.PhotonMonitors[iMon].Monitor->config.timeUnits.toLatin1().data();
        h->GetXaxis()->SetTitle(title);
        emit requestDraw(h, "hist", false, true);
    }
}

void APhotSimWin::on_pbMonitorShowWaveIndex_clicked()
{
    const int numMonitors = MonitorHub.countMonitors(AMonitorHub::Photon);
    const int iMon = ui->cobMonitor->currentIndex();
    if (iMon >=0 && iMon < numMonitors)
    {
        TH1D * h = MonitorHub.PhotonMonitors[iMon].Monitor->wave;
        h->GetXaxis()->SetTitle("Index");
        emit requestDraw(h, "hist", false, true);
    }
}

void APhotSimWin::on_pbMonitorShowWavelength_clicked()
{
    if (!SimSet.WaveSet.Enabled)
    {
        on_pbMonitorShowWaveIndex_clicked();
        return;
    }

    const int numMonitors = MonitorHub.countMonitors(AMonitorHub::Photon);
    const int iMon = ui->cobMonitor->currentIndex();
    if (iMon >=0 && iMon < numMonitors)
    {
        TH1D * h = MonitorHub.PhotonMonitors[iMon].Monitor->wave;
        int nbins = h->GetXaxis()->GetNbins();

        double gsWaveFrom = SimSet.WaveSet.From;
        double gsWaveTo = SimSet.WaveSet.To;
        double gsWaveBins = SimSet.WaveSet.countNodes();
        if (gsWaveBins > 1) gsWaveBins--;
        double wavePerBin = (gsWaveTo - gsWaveFrom) / gsWaveBins;

        double binFrom = h->GetBinLowEdge(1);
        double waveFrom = gsWaveFrom + (binFrom - 0.5) * wavePerBin;
        double binTo = h->GetBinLowEdge(nbins+1);
        double waveTo = gsWaveFrom + (binTo - 0.5) * wavePerBin;

        TH1D * hnew = new TH1D("", "", nbins, waveFrom, waveTo);
        for (int i=1; i <= nbins; i++)
        {
            double y = h->GetBinContent(i);
            hnew->SetBinContent(i, y);
        }
        hnew->SetXTitle("Wavelength, nm");
        emit requestDraw(hnew, "hist", true, true);
    }
}

void APhotSimWin::on_pbShowMonitorHitDistribution_clicked()
{
    const int numMon = MonitorHub.countMonitors(AMonitorHub::Photon);
    if (numMon == 0) return;

    TH1D * h = new TH1D("", "Monitor hits", numMon, 0, numMon);
    int sumHits = 0;
    for (int iMon = 0; iMon < numMon; iMon++)
    {
        int hits = MonitorHub.PhotonMonitors[iMon].Monitor->getHits();
        sumHits += hits;
        if (hits > 0) h->Fill(iMon, hits);
    }

    if (sumHits == 0) return;
    h->SetEntries(sumHits);
    h->GetXaxis()->SetTitle("Monitor index");
    h->GetYaxis()->SetTitle("Hits");
    emit requestDraw(h, "hist", true, true);
}

void APhotSimWin::on_pbShowMonitorTimeOverall_clicked()
{
    const int numMon = MonitorHub.countMonitors(AMonitorHub::Photon);
    if (numMon == 0) return;

    int    bins = 1000;
    double from = -1e30;
    double to   = +1e30;

    for (int iM = 0; iM < numMon; iM++)
    {
        TH1D * hh = MonitorHub.PhotonMonitors[iM].Monitor->time;
        if (!hh) continue;

        int thisBins = hh->GetXaxis()->GetNbins();
        int thisFrom = hh->GetBinLowEdge(1);
        int thisTo   = hh->GetBinLowEdge(bins+1);

        if (thisBins < bins) bins = thisBins;
        if (thisFrom > from) from = thisFrom;
        if (thisTo   < to  ) to   = thisTo;
    }

    TH1D * time = new TH1D("", "Time of hits", bins, from, to);

    int sumHits = 0;
    for (int iMon = 0; iMon < numMon; iMon++)
    {
        TH1D * h = MonitorHub.PhotonMonitors[iMon].Monitor->time;
        int hits = h->GetEntries();
        if (hits == 0) continue;

        sumHits += hits;
        for (int iBin = 1; iBin <= h->GetNbinsX(); iBin++)
            time->Fill(h->GetBinCenter(iBin), h->GetBinContent(iBin));
    }

    // TODO under / overflow  !!!***

    time->BufferEmpty(1);
    time->SetEntries(sumHits);
    time->GetXaxis()->SetTitle("Time, ns");
    time->GetYaxis()->SetTitle("Hits");
    emit requestDraw(time, "hist", true, true);
}

// ---

void APhotSimWin::on_pbChangeDepositionFile_clicked()
{
    QString fileName = guitools::dialogLoadFile(this, "Select file with energy deposition data", "");
    if (fileName.isEmpty()) return;
    ui->leDepositionFile->setText(fileName);
    on_leDepositionFile_editingFinished();
}
void APhotSimWin::on_leDepositionFile_editingFinished()
{
    QString NewFileName = ui->leDepositionFile->text();
    if (NewFileName != SimSet.DepoSet.FileName)
    {
        SimSet.DepoSet.clear();
        SimSet.DepoSet.FileName = ui->leDepositionFile->text();
        updateDepoGui();
    }
}
void APhotSimWin::on_cbPrimaryScint_clicked(bool checked)
{
    SimSet.DepoSet.Primary = checked;
}
void APhotSimWin::on_cbSecondaryScint_clicked(bool checked)
{
    SimSet.DepoSet.Secondary = checked;
}

void APhotSimWin::on_pbAnalyzeDepositionFile_clicked()
{
    ADepositionFileHandler fh(SimSet.DepoSet);

    //if (SimSet.DepoSet.isValidated()) return; // already up to date

    fh.determineFormat();

    if (SimSet.DepoSet.FileFormat == APhotonDepoSettings::Invalid)
    {
        guitools::message("Cannot open file!", this);
        updateDepoGui();
        return;
    }
    if (SimSet.DepoSet.FileFormat == APhotonDepoSettings::Undefined)
    {
        guitools::message("Unknown format of the depo file!", this);
        updateDepoGui();
        return;
    }

    AErrorHub::clear();
    bool ok = fh.init();
    if (!ok)
    {
        guitools::message(AErrorHub::getQError(), this);
        SimSet.DepoSet.FileFormat = APhotonDepoSettings::Invalid;
        updateDepoGui();
        return;
    }

    fh.checkFile();
    if (SimSet.DepoSet.NumEvents == -1)
    {
        guitools::message("Deposition file is invalid", this);
        SimSet.DepoSet.FileFormat = APhotonDepoSettings::Invalid;
    }
    updateDepoGui();
}

void APhotSimWin::on_pbCollectDepoFileStatistics_clicked()
{
    ADepositionFileHandler fh(SimSet.DepoSet);

    if (!SimSet.DepoSet.isValidated())
    {
        fh.determineFormat();

        if (SimSet.DepoSet.FileFormat == APhotonDepoSettings::Invalid)
        {
            guitools::message("Cannot open file!", this);
            updateDepoGui();
            return;
        }
        if (SimSet.DepoSet.FileFormat == APhotonDepoSettings::Undefined)
        {
            guitools::message("Unknown format of the depo file!", this);
            updateDepoGui();
            return;
        }
    }

    AErrorHub::clear();

    fh.collectStatistics();
    updateDepoGui();
    if (SimSet.DepoSet.NumEvents == -1)
    {
        guitools::message("Deposition file is invalid", this);
        SimSet.DepoSet.FileFormat = APhotonDepoSettings::Invalid;
    }
    else
    {
        QString txt = fh.formReportString();
        guitools::message1(txt, "Deposition file info", this);
    }
}

#include "adeporecord.h"
void APhotSimWin::on_pbViewDepositionFile_clicked()
{
    ADepositionFileHandler fh(SimSet.DepoSet);
    ADepoRecord record;
    QString text = fh.preview(record, 100);
    guitools::message1(text, "", this);
}
void APhotSimWin::on_pbHelpDepositionFile_clicked()
{
    QString txt = ""
    "ASCII file format\n"
    "-----------------\n"
    "#Event_index\n"
    "ParticleName indexMat Energy[keV] X Y Z[mm] Time[ns] indexVolume";

    guitools::message1(txt, "Deposition file format", this);
}

void APhotSimWin::on_pbdUpdateScanSettings_clicked()
{
    AGridSettings & g = SimSet.BombSet.GridSettings;

    g.X0 = ui->ledOriginX->text().toDouble();
    g.Y0 = ui->ledOriginY->text().toDouble();
    g.Z0 = ui->ledOriginZ->text().toDouble();

    g.ScanRecords[1].bEnabled = ui->cbSecondAxis->isChecked();
    g.ScanRecords[2].bEnabled = ui->cbThirdAxis->isChecked();

    for (int i = 0; i < 3; i++)
    {
        QLineEdit *leDX, *leDY, *leDZ;
        QSpinBox  *sbNodes;
        QComboBox *cobBiDir;
        switch (i)
        {
        case 0: leDX = ui->led0X; leDY = ui->led0Y; leDZ = ui->led0Z; sbNodes = ui->sb0nodes; cobBiDir = ui->cob0dir; break;
        case 1: leDX = ui->led1X; leDY = ui->led1Y; leDZ = ui->led1Z; sbNodes = ui->sb1nodes; cobBiDir = ui->cob1dir; break;
        case 2: leDX = ui->led2X; leDY = ui->led2Y; leDZ = ui->led2Z; sbNodes = ui->sb2nodes; cobBiDir = ui->cob2dir; break;
        }

        APhScanRecord & r = g.ScanRecords[i];

        r.DX        = leDX->text().toDouble();
        r.DY        = leDY->text().toDouble();
        r.DZ        = leDZ->text().toDouble();
        r.Nodes     = sbNodes->value();
        r.bBiDirect = (cobBiDir->currentIndex() == 1);
    }
}

#include "abombadvanceddialog.h"
void APhotSimWin::on_pbAdvancedBombSettings_clicked()
{
    ABombAdvancedDialog dia(this);
    dia.exec();
    updateAdvancedBombIndicator();
}

void APhotSimWin::updateAdvancedBombIndicator()
{
    const APhotonAdvancedSettings & s = SimSet.BombSet.AdvancedSettings;

    bool on = (s.DirectionMode != APhotonAdvancedSettings::Isotropic || s.bFixWave || s.bFixDecay || s.bOnlyVolume || s.bOnlyMaterial);
    ui->labAdvancedBombOn->setVisible(on);
}

// --- BombFile ---

void APhotSimWin::on_leNodeFileName_editingFinished()
{
    ABombFileSettings & s = SimSet.BombSet.BombFileSettings;
    QString NewFileName = ui->leNodeFileName->text();
    if (NewFileName != s.FileName)
    {
        s.clear();
        s.FileName = ui->leNodeFileName->text();
        updateBombFileGui();
    }
}
void APhotSimWin::on_pbNodeFileChange_clicked()
{
    QString fileName = guitools::dialogLoadFile(this, "Select file with photon bombs", "");
    if (fileName.isEmpty()) return;
    ui->leNodeFileName->setText(fileName);
    on_leNodeFileName_editingFinished();
}
#include "aphotonbombfilehandler.h"
void APhotSimWin::on_pbNodeFileAnalyze_clicked()
{
    ABombFileSettings & bset = SimSet.BombSet.BombFileSettings;

    APhotonBombFileHandler fh(bset);

    const bool CollectStats = ui->cbNodeFileCollectStatistics->isChecked();
    if (bset.isValidated() && !CollectStats) return; // already up to date

    if (!bset.isValidated())
    {
        fh.determineFormat();

        if (bset.FileFormat == ABombFileSettings::Invalid)
        {
            guitools::message("Cannot open file!", this);
            updateBombFileGui();
            return;
        }
        if (bset.FileFormat == ABombFileSettings::Undefined)
        {
            guitools::message("Unknown format of the file with photon bombs!", this);
            updateBombFileGui();
            return;
        }
    }

    AErrorHub::clear();
    bool ok = fh.init();
    if (!ok)
    {
        guitools::message(AErrorHub::getQError(), this);
        bset.FileFormat = ABombFileSettings::Invalid;
        updateBombFileGui();
        return;
    }

    fh.checkFile();
    if (bset.NumEvents == -1)
    {
        guitools::message("Photon bomb file is invalid", this);
        bset.FileFormat = ABombFileSettings::Invalid;
    }
    updateBombFileGui();
}
#include "anoderecord.h"
void APhotSimWin::on_pbNodeFilePreview_clicked()
{
    APhotonBombFileHandler fh(SimSet.BombSet.BombFileSettings);
    ANodeRecord rec;
    QString text = fh.preview(rec, 100);
    guitools::message1(text, "", this);
}
void APhotSimWin::on_pbNodeFileHelp_clicked()
{

}

// ---

void APhotSimWin::on_cobNodeGenerationMode_currentIndexChanged(int index)
{
    bool bFromFile = (index == 3);
    ui->cobNumPhotonsMode->setDisabled(bFromFile);
    ui->swNumPhotons->setDisabled(bFromFile);
}

// ---

void APhotSimWin::on_leSinglePhotonsFile_editingFinished()
{
    APhotonFileSettings & s = SimSet.PhotFileSet;
    const QString NewFileName = ui->leSinglePhotonsFile->text();
    if (NewFileName != s.FileName)
    {
        s.clear();
        s.FileName = NewFileName;
        updatePhotonFileGui();
    }
}

void APhotSimWin::on_pbChangeSinglePhotonsFile_clicked()
{
    QString fileName = guitools::dialogLoadFile(this, "Select file with individual photon records", "");
    if (fileName.isEmpty()) return;
    ui->leSinglePhotonsFile->setText(fileName);
    on_leSinglePhotonsFile_editingFinished();
}

#include "aphotonfilehandler.h"
void APhotSimWin::on_pbAnalyzeSinglePhotonsFile_clicked()
{
    APhotonFileSettings & bset = SimSet.PhotFileSet;

    APhotonFileHandler fh(bset);

    if (bset.isValidated()) return; // already up to date

    fh.determineFormat();

    if (bset.FileFormat == AFileSettingsBase::Invalid)
    {
        guitools::message("Cannot open file!", this);
        updatePhotonFileGui();
        return;
    }
    if (bset.FileFormat == AFileSettingsBase::Undefined)
    {
        guitools::message("Unknown format of the file with individual photon records!", this);
        updatePhotonFileGui();
        return;
    }

    AErrorHub::clear();
    bool ok = fh.init();
    if (!ok)
    {
        guitools::message(AErrorHub::getQError(), this);
        bset.FileFormat = AFileSettingsBase::Invalid;
        updatePhotonFileGui();
        return;
    }

    ok = fh.checkFile();
    if (!ok)
    {
        guitools::message("File with individual photon records: invalid format!", this);
        bset.FileFormat = AFileSettingsBase::Invalid;
    }
    updatePhotonFileGui();
}

#include "aphoton.h"
void APhotSimWin::on_pbViewSinglePhotFile_clicked()
{
    APhotonFileHandler fh(SimSet.PhotFileSet);
    APhoton phot;
    QString text = fh.preview(phot, 100);
    guitools::message1(text, "", this);
}
void APhotSimWin::on_pbSinglePhotonsHelp_clicked()
{

}

// --- Show Photon bombs ---

void APhotSimWin::on_pbSelectBombsFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file with photon bombs", SimSet.RunSet.OutputDirectory);
    if (!fileName.isEmpty()) ui->leBombsFile->setText(fileName);
}

void APhotSimWin::on_pbChangeWorkingDir_clicked()
{
    QString dir = guitools::dialogDirectory(this, "Select directory with photon data", SimSet.RunSet.OutputDirectory, true, false);
    if (!dir.isEmpty()) ui->leResultsWorkingDir->setText(dir);
}

void APhotSimWin::on_tbwResults_currentChanged(int index)
{
    ui->frEventNumber->setVisible(index > 1);
}

void APhotSimWin::on_pbShowEvent_clicked()
{
    disableGui(true);
        doShowEvent();
    disableGui(false);
}

void APhotSimWin::doShowEvent()
{
    switch (ui->tbwResults->currentIndex())
    {
    case 2 : showSensorSignals(true);  break;
    case 3 : showBombSingleEvent();   break;
    case 4 : showTracksSingleEvent(); break;
    default :;
    }
}

#include "asensorsignalarray.h"
void APhotSimWin::showSensorSignals(bool suppressMessage)
{
    QString name = ui->leSensorSigFileName->text();
    if (!name.contains('/')) name = ui->leResultsWorkingDir->text() + '/' + name;

    if (name.isEmpty())
    {
        if (!suppressMessage) guitools::message("File name is empty!", this);
        return;
    }

    if (SignalsFileSettings->FileName != name || !SignalsFileHandler->isInitialized())
    {
        SignalsFileSettings->clear();

        SignalsFileSettings->FileName = name;
        AErrorHub::clear();
        bool ok = SignalsFileHandler->init();
        if (!ok)
        {
            if (!suppressMessage) guitools::message(AErrorHub::getQError(), this);
            return;
        }
    }

    ASensorSignalArray ar;
    const int numSensors = ASensorHub::getConstInstance().countSensors();
    ar.Signals.resize(numSensors);

    AErrorHub::clear();
    bool ok = SignalsFileHandler->gotoEvent(ui->sbEvent->value());
    if (!ok)
    {
        guitools::message(AErrorHub::getQError(), this); // check: silence error if suppressMessage?
        return;
    }

    ok = SignalsFileHandler->readNextRecordSameEvent(ar);
    if (!ok)
    {
        guitools::message(AErrorHub::getQError(), this); // check: silence error if suppressMessage?
        return;
    }

    // !!!*** int --> size_t?
    std::vector<int> enabledSensors;
    fillListEnabledSensors(enabledSensors);

    showSensorSignalDraw(ar.Signals, enabledSensors);
    showSensorSignalTable(ar.Signals, enabledSensors);
}

void APhotSimWin::fillListEnabledSensors(std::vector<int> & enabledSensors)
{
    const int numSensors = ASensorHub::getConstInstance().countSensors();
    if (ui->cbSensorsAll->isChecked())
    {
        for (int i = 0; i < numSensors; i++)
            enabledSensors.push_back(i);
    }
    else
    {
        QString txt;
        if      (ui->cbSensorsG1->isChecked()) txt = ui->leSensorsG1->text();
        else if (ui->cbSensorsG2->isChecked()) txt = ui->leSensorsG2->text();
        else if (ui->cbSensorsG3->isChecked()) txt = ui->leSensorsG3->text();
        guitools::extractNumbersFromQString(txt, enabledSensors);
    }
}

void APhotSimWin::showSensorSignalDraw(const std::vector<float> & signalArray, const std::vector<int> & enabledSensors)
{
    gvSensors->updateGui(signalArray, enabledSensors);
}

void APhotSimWin::showSensorSignalTable(const std::vector<float> & signalArray, const std::vector<int> & enabledSensors)
{
    ui->twSensorTable->clearContents();

    const size_t numSensors = signalArray.size();
    const int numColumns = ui->sbSensorTableColumns->value();

    int currentRow = 0;
    int currentColumn = 0;
    for (int iSensorIndex : enabledSensors)
        if (iSensorIndex < numSensors)
        {
            QString txt = QString("#%0: %1").arg(iSensorIndex).arg(signalArray[iSensorIndex]);
            QTableWidgetItem * item = ui->twSensorTable->item(currentRow, currentColumn);
            if (!item)
            {
                item = new QTableWidgetItem();
                ui->twSensorTable->setItem(currentRow, currentColumn, item);
            }
            item->setText(txt);
            currentColumn++;
            if (currentColumn >= numColumns)
            {
                currentColumn = 0;
                currentRow++;
            }
        }
}

// ------

void APhotSimWin::showBombSingleEvent()
{
    emit requestClearGeoMarkers(0);

    bool ok = updateBombHandler();
    if (!ok) return;

    int iEvent = ui->sbEvent->value();
    if (iEvent < 0)
    {
        ui->sbEvent->setValue(0);
        iEvent = 0;
    }

    /*
    if (iEvent >= BombFileSettings->NumEvents)
    {
        iEvent = BombFileSettings->NumEvents - 1;
        if (iEvent < 0) iEvent = 0;
        ui->sbEvent->setValue(iEvent);
    }
    */

    ok = BombFileHandler->gotoEvent(iEvent);
    if (!ok)
    {
        guitools::message("Cannot go to this event!", this);
        return;
    }

    ANodeRecord node(0,0,0);
    while (BombFileHandler->readNextRecordSameEvent(node))
        emit requestAddPhotonNodeGeoMarker(node);

    emit requestShowGeoMarkers();
}

void APhotSimWin::showTracksSingleEvent()
{
    QString FileName = ui->leTracksFile->text();
    if (!FileName.contains('/')) FileName = ui->leResultsWorkingDir->text() + '/' + FileName;

    QFile file(FileName);
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) return;

    TGeoManager * GeoManager = AGeometryHub::getInstance().GeoManager;
    GeoManager->ClearTracks();

    const int iShowEvent = ui->sbEvent->value();

    QTextStream in(&file);
    int iCurrentEvent = 0;
    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (line.startsWith('#'))
        {
            line.remove('#');
            iCurrentEvent = line.toInt();
            continue;
        }

        if (iShowEvent != iCurrentEvent) continue;

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

bool APhotSimWin::updateBombHandler()
{
    QString name = ui->leBombsFile->text();
    if (!name.contains('/')) name = ui->leResultsWorkingDir->text() + '/' + name;

    if (name.isEmpty())
    {
        //guitools::message("File name is empty!", this);
        return false;
    }

    if (BombFileSettings->FileName != name || !BombFileHandler->isInitialized())
    {
        BombFileSettings->clear();

        BombFileSettings->FileName = name;
        AErrorHub::clear();
        bool ok = BombFileHandler->init();
        if (!ok)
        {
            //guitools::message(AErrorHub::getQError(), this);
            return false;
        }
    }

    return true;
}

void APhotSimWin::disableGui(bool flag)
{
    setDisabled(flag);
}

void APhotSimWin::on_pbEventNumberLess_clicked()
{
    int iEvent = ui->sbEvent->value();
    if (iEvent == 0) return;

    ui->sbEvent->setValue(iEvent - 1);
    on_pbShowEvent_clicked();
}

void APhotSimWin::on_pbEventNumberMore_clicked()
{
    ui->sbEvent->setValue(ui->sbEvent->value() + 1);
    on_pbShowEvent_clicked();
}

void APhotSimWin::on_pbChooseSensorSigFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file with sensor data", SimSet.RunSet.OutputDirectory);
    if (!fileName.isEmpty()) ui->leSensorSigFileName->setText(fileName);
}

void APhotSimWin::on_pbShowBombsMultiple_clicked()
{
    emit requestShowGeometry(true);
//    emit requestShowTracks(); // !!!***

    emit requestClearGeoMarkers(0);

    bool ok = updateBombHandler();
    if (!ok) return;

    BombFileHandler->init();

    ANodeRecord node(0,0,0);
    while (!BombFileHandler->atEnd())
    {
        while (BombFileHandler->readNextRecordSameEvent(node))
            emit requestAddPhotonNodeGeoMarker(node);
        BombFileHandler->acknowledgeNextEvent();
    }
    emit requestShowGeoMarkers();
}

void APhotSimWin::on_pbSingleSourceShow_clicked()
{
    double pos[3];
    pos[0] = ui->ledSingleX->text().toDouble();
    pos[1] = ui->ledSingleY->text().toDouble();
    pos[2] = ui->ledSingleZ->text().toDouble();
    emit requestShowPosition(pos, false);
}

void APhotSimWin::on_cbRndCheckBeforeTrack_toggled(bool checked)
{
    ui->labSkipTracingON->setVisible(checked);
    ui->twGeneralOption->setTabIcon(1, (checked ? YellowCircle : QIcon()));
}

void APhotSimWin::on_sbEvent_editingFinished()
{
    on_pbShowEvent_clicked();
}
