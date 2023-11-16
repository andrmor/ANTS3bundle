#include "aparticlesimwin.h"
#include "ui_aparticlesimwin.h"
#include "a3global.h"
#include "ageometryhub.h"
#include "aparticlesimhub.h"
#include "aparticlesimsettings.h"
#include "ag4simulationsettings.h"
#include "aparticlesimmanager.h"
#include "guitools.h"
#include "aerrorhub.h"
#include "amaterialhub.h"
#include "amonitorhub.h"
#include "amonitor.h"
#include "acalorimeterhub.h"
#include "acalorimeter.h"
#include "ajsontools.h"
#include "aparticletrackvisuals.h"
#include "aparticlesourceplotter.h"
#include "adispatcherinterface.h"
#include "ageoobject.h"
#include "ath.h"
#include "atrackingdataimporter.h"
#include "aeventsdonedialog.h"
#include "atrackingdataexplorer.h"
#include "aeventtrackingrecord.h"

#include <QListWidget>
#include <QDialog>
#include <QDebug>
#include <QCheckBox>
#include <QLineEdit>
#include <QFile>
#include <QRegularExpression>
#include <QDoubleValidator>

#include <string>

#include "TVirtualGeoTrack.h"
#include "TH1D.h"

AParticleSimWin::AParticleSimWin(QWidget * parent) :
    AGuiWindow("PartSim", parent),
    SimSet(AParticleSimHub::getInstance().Settings),
    G4SimSet(SimSet.G4Set),
    SimManager(AParticleSimManager::getInstance()),
    MonitorHub(AMonitorHub::getInstance()),
    CalHub(ACalorimeterHub::getInstance()),
    ui(new Ui::AParticleSimWin)
{
    CurrentEventRecord = AEventTrackingRecord::create();

    ui->setupUi(this);

    ui->frEventFilters->setVisible(false);

    on_cobPTHistVolRequestWhat_currentIndexChanged(ui->cobPTHistVolRequestWhat->currentIndex());

    ui->cobEVkin->setCurrentIndex(1);
    ui->cobEVdepo->setCurrentIndex(1);
    ui->pbShowEventTree->setVisible(false);
    ui->pbShowGeometry->setVisible(false);
    ui->pbUpdateIcon->setVisible(false);
    ui->pbUpdateCaloRange->setVisible(false);

    QDoubleValidator * dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = this->findChildren<QLineEdit*>();
    foreach(QLineEdit *w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);

    updateGui();

    ADispatcherInterface & Dispatcher = ADispatcherInterface::getInstance();
    connect(&Dispatcher, &ADispatcherInterface::updateProgress, this, &AParticleSimWin::onProgressReceived);

    connect(&SimManager, &AParticleSimManager::requestUpdateResultsGUI, this, &AParticleSimWin::updateResultsGui);

    QPixmap pm = guitools::createColorCirclePixmap({15,15}, Qt::yellow);
    ui->labFilterActivated->setPixmap(pm);
    on_pbUpdateIcon_clicked();
    ui->labRangeWarningHor->setPixmap(pm);
    ui->labRangeWarningVert->setPixmap(pm);
    updateRangeWarning();

    ui->twMain->setCurrentIndex(1);
}

AParticleSimWin::~AParticleSimWin()
{
    delete ui;
}

void AParticleSimWin::updateGui()
{
    bGuiUpdateInProgress = true;  // -->

    updateG4Gui();
    updateSimGui();
    onMaterialsChanged();

    updateGeneralControlInResults();

    emit killSourceDialog();

    bGuiUpdateInProgress = false; // <--
}

void AParticleSimWin::updateSimGui()
{
    ui->ledEvents->setText(QString::number(SimSet.Events));

    int iMode = 0;
    if      (SimSet.GenerationMode == AParticleSimSettings::Sources) iMode = 0;
    else if (SimSet.GenerationMode == AParticleSimSettings::File)    iMode = 1;
    else if (SimSet.GenerationMode == AParticleSimSettings::Script)  iMode = 2;
    else qWarning() << "Unknown particle generation mode!";
    ui->cobParticleGenerationMode->setCurrentIndex(iMode);

    updateSourceList();

    ui->cbGunAllowMultipleEvents->setChecked(SimSet.SourceGenSettings.MultiEnabled);
    ui->cobPartPerEvent->setCurrentIndex(SimSet.SourceGenSettings.MultiMode == ASourceGeneratorSettings::Constant ? 0 : 1);
    ui->ledGunAverageNumPartperEvent->setText(QString::number(SimSet.SourceGenSettings.MultiNumber));

    //ui->leGenerateFromFile_FileName->setText(SimSet.FileGenSettings.FileName.data());
    updateFileParticleGeneratorGui();
}

void AParticleSimWin::updateG4Gui()
{
    ui->lePhysicsList->setText(G4SimSet.PhysicsList.data());
    ui->cobRefPhysLists->setCurrentIndex(-1);

    ui->pteCommands->clear();
    for (const auto & s : G4SimSet.Commands)
        ui->pteCommands->appendPlainText(s.data());

    QStringList svl;
    ui->pteSensitiveVolumes->clear();
    for (const auto & s : G4SimSet.SensitiveVolumes) svl << s.data();
    ui->pteSensitiveVolumes->appendPlainText(svl.join("\n"));
    ui->cbIncludeScintillators->setChecked(G4SimSet.AddScintillatorsToSensitiveVolumes);

    ui->lwStepLimits->clear();
    for (const auto & it : G4SimSet.StepLimits)
        ui->lwStepLimits->addItem( QString("%0 -> %1").arg(it.first.data(), QString::number(it.second)) );

    ui->cbUseTSphys->setChecked(G4SimSet.UseTSphys);
}

// --- input ---

void AParticleSimWin::on_lePhysicsList_editingFinished()
{
    G4SimSet.PhysicsList = ui->lePhysicsList->text().toLatin1().data();
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
    if (bGuiUpdateInProgress) return;

    const QString t = ui->pteCommands->document()->toPlainText();
    const QStringList sl = t.split('\n', Qt::SkipEmptyParts);
    G4SimSet.Commands.clear();
    for (auto & s : sl) G4SimSet.Commands.push_back(s.toLatin1().data());
}
void AParticleSimWin::on_pteSensitiveVolumes_textChanged()
{
    if (bGuiUpdateInProgress) return;

    const QRegularExpression rx = QRegularExpression("(\\ |\\,|\\n|\\t)"); //separators: ' ' or ',' or 'n' or '\t'
    QString t = ui->pteSensitiveVolumes->document()->toPlainText();
    const QStringList sl = t.split(rx, Qt::SkipEmptyParts);
    G4SimSet.SensitiveVolumes.clear();
    for (auto & s : sl) G4SimSet.SensitiveVolumes.push_back(s.toLatin1().data());
}

void AParticleSimWin::on_pbAddNewStepLimit_clicked()
{
    showStepLimitDialog("", 0);
}

void AParticleSimWin::showStepLimitDialog(const QString & volName, double limit)
{
    QDialog d(this);

    QVBoxLayout * vl = new QVBoxLayout(&d);

    QHBoxLayout * hl = new QHBoxLayout();
        hl->addWidget(new QLabel("Volume:"));
        QLineEdit * leVol = new QLineEdit(); hl->addWidget(leVol);
        if (!volName.isEmpty()) leVol->setText(volName);
        hl->addWidget(new QLabel("Step limit:"));
        QLineEdit * leStep = new QLineEdit(); hl->addWidget(leStep);
        if (limit > 0) leStep->setText(QString::number(limit));
        QDoubleValidator * dv = new QDoubleValidator(&d);
        leStep->setValidator(dv);
    vl->addLayout(hl);

    QPushButton * pbAccept = new QPushButton("Add this limit");
    QObject::connect(pbAccept, &QPushButton::clicked, &d,
    [leVol, leStep, &d]()
    {
        const QString vol = leVol->text();
        if (vol.isEmpty())
        {
            guitools::message("Volume is empty!", &d);
            return;
        }
        if (vol.contains('*') && (vol.count('*') > 1 || vol.indexOf('*') != vol.length()-1) )
        {
            guitools::message("Only one '*' can be used in the volume name,\nand it should be at the end!", &d);
            return;
        }
        double step = leStep->text().toDouble();
        if (step <= 0)
        {
            guitools::message("Step limit should have a positive numeric value", &d);
            return;
        }
        d.accept();
    }
    );

    vl->addWidget(pbAccept);

    int res = d.exec();
    if (res == QDialog::Rejected) return;


    G4SimSet.StepLimits[leVol->text().toLatin1().data()] = leStep->text().toDouble();
    updateG4Gui();
}
void AParticleSimWin::on_lwStepLimits_itemDoubleClicked(QListWidgetItem * item)
{
    const QString line = item->text();
    QStringList sl = line.split(" -> ");
    if (sl.size() != 2)
    {
        guitools::message("Something went wrong!", this);
        return;
    }

    QString vol = sl.first();
    double limit = sl[1].toDouble();

    G4SimSet.StepLimits.erase(vol.toLatin1().data());

    showStepLimitDialog(vol, limit);
}
void AParticleSimWin::on_pbRemoveStepLimit_clicked()
{
     QListWidgetItem * item = ui->lwStepLimits->currentItem();
     if (!item)
     {
         guitools::message("Select a record to remove", this);
         return;
     }
     const QString line = item->text();
     QStringList sl = line.split(" -> ");
     if (sl.size() != 2)
     {
         guitools::message("Something went wrong!", this);
         return;
     }
     QString vol = sl.first();
     G4SimSet.StepLimits.erase(vol.toLatin1().data());
     updateG4Gui();
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
    ASourceGeneratorSettings & SourceGenSettings = SimSet.SourceGenSettings;
    const int numSources = SourceGenSettings.getNumSources();
    if (isource >= numSources)
    {
        guitools::message("Error - bad source index!", this);
        return;
    }

    AParticleSourceDialog ParticleSourceDialog(SourceGenSettings.SourceData.at(isource), this);
    connect(&ParticleSourceDialog, &AParticleSourceDialog::requestTestParticleGun, this, &AParticleSimWin::testParticleGun);
    connect(&ParticleSourceDialog, &AParticleSourceDialog::requestShowSource,      this, &AParticleSimWin::onRequestShowSource);
    connect(&ParticleSourceDialog, &AParticleSourceDialog::requestDraw,            this, &AParticleSimWin::requestDraw);
    connect(this,                  &AParticleSimWin::killSourceDialog,             &ParticleSourceDialog, &AParticleSourceDialog::reject);

    int res = ParticleSourceDialog.exec();
    if (res == QDialog::Rejected) return;

    AParticleSourceRecord & ps = ParticleSourceDialog.getResult();
    SourceGenSettings.replace(isource, ps);

    updateSourceList();
    if (ui->pbGunShowSource->isChecked()) on_pbGunShowSource_toggled(true);

    checkWorldSize(ps);
}

#include "aworldsizewarningdialog.h"
void AParticleSimWin::checkWorldSize(AParticleSourceRecord & ps)
{
    if (IgnoreWorldSizeWarning) return;

    AGeometryHub & GeoHub = AGeometryHub::getInstance();
    const double wXY = GeoHub.getWorldSizeXY();
    const double wZ  = GeoHub.getWorldSizeZ();
    double maxXY = std::max(abs(ps.X0), abs(ps.Y0));
    double maxZ  = abs(ps.Z0);
    switch (ps.Shape)
    {
    case AParticleSourceRecord::Point :
        break;
    case AParticleSourceRecord::Line :
        maxXY += 0.5 * ps.Size1;
        maxZ  += 0.5 * ps.Size1;
        break;
    case AParticleSourceRecord::Rectangle :
        maxXY += 0.5 * std::max(ps.Size1, ps.Size2);
        maxZ  += 0.5 * std::max(ps.Size1, ps.Size2);
        break;
    case AParticleSourceRecord::Round :
        maxXY += 0.5 * ps.Size1;
        maxZ  += 0.5 * ps.Size1;
        break;
    case AParticleSourceRecord::Box :
        maxXY += 0.5 * std::max(ps.Size1, std::max(ps.Size2, ps.Size3));
        maxZ  += 0.5 * std::max(ps.Size1, std::max(ps.Size2, ps.Size3));
        break;
    case AParticleSourceRecord::Cylinder :
        maxXY += 0.5 * std::max(ps.Size1, ps.Size2);
        maxZ  += 0.5 * std::max(ps.Size1, ps.Size2);
        break;
    }

    maxXY *= 1.1;
    maxZ  *= 1.1;

    if (maxXY > wXY || maxZ > wZ)
    {
        AWorldSizeWarningDialog d( std::max(maxXY, wXY), std::max(maxZ, wZ), this);
        d.exec();
        switch (d.Result)
        {
        case AWorldSizeWarningDialog::OK :
            break;
        case AWorldSizeWarningDialog::Goto :
            emit requestShowGeoObjectDelegate("World", true);
            break;
        case AWorldSizeWarningDialog::DontBother :
            IgnoreWorldSizeWarning = true;
            break;
        }
    }
}

void AParticleSimWin::on_pbAddSource_clicked()
{
    AParticleSourceRecord s;
    s.Particles.push_back(AGunParticle());
    SimSet.SourceGenSettings.SourceData.push_back(s);

    updateSourceList();
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

    updateSourceList();
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

    const QString SourceName = SimSet.SourceGenSettings.SourceData.at(isource).Name.data();
    bool ok = guitools::confirm(QString("Remove source %0?").arg(SourceName), this);
    if (!ok) return;

    SimSet.SourceGenSettings.remove(isource);
    updateSourceList();

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

void AParticleSimWin::updateSourceList()
{
    ASourceGeneratorSettings & SourceGenSettings = SimSet.SourceGenSettings;
    const int numSources = SourceGenSettings.getNumSources();

    int curRow = ui->lwDefinedParticleSources->currentRow();
    ui->lwDefinedParticleSources->clear();

    const double TotalActivity = SourceGenSettings.calculateTotalActivity();

    for (int i = 0; i < numSources; i++)
    {
        AParticleSourceRecord & pr = SourceGenSettings.SourceData[i];
        QListWidgetItem * item = new QListWidgetItem();
        ui->lwDefinedParticleSources->addItem(item);

        QFrame* fr = new QFrame();
        fr->setFrameShape(QFrame::Box);
        QHBoxLayout* l = new QHBoxLayout();
        l->setContentsMargins(3, 2, 3, 2);
            QLabel* lab = new QLabel(pr.Name.data());
            lab->setMinimumWidth(110);
            QFont f = lab->font();
            f.setBold(true);
            lab->setFont(f);
        l->addWidget(lab);
        l->addWidget(new QLabel( QString(pr.getShapeString().data()) + ','));
            QString SPart;
            if (pr.Particles.size() == 1) SPart = pr.Particles.front().Particle.data();
            else SPart = QString("%1 particles").arg(pr.Particles.size());
        l->addWidget(new QLabel(SPart));
        l->addStretch();

        l->addWidget(new QLabel("Fraction:"));
            QLineEdit* e = new QLineEdit(QString::number(pr.Activity));
            e->setMaximumWidth(50);
            e->setMinimumWidth(50);
            QDoubleValidator* val = new QDoubleValidator(this);
            val->setBottom(0);
            e->setValidator(val);
            QObject::connect(e, &QLineEdit::editingFinished, [&pr, e, this]()
            {
                double newVal = e->text().toDouble();
                if (pr.Activity == newVal) return;
                pr.Activity = newVal;
                e->clearFocus();
                updateSourceList();
            });
        l->addWidget(e);

            double per = ( TotalActivity == 0 ? 0 : 100.0 * pr.Activity / TotalActivity );
            QString t = (per == 0 ? "-Off-" : QString("%1%").arg(per, 3, 'g', 3) );
            lab = new QLabel(t);
            lab->setMinimumWidth(45);
        l->addWidget(lab);

        fr->setLayout(l);
        item->setSizeHint(fr->sizeHint());
        ui->lwDefinedParticleSources->setItemWidget(item, fr);
        item->setSizeHint(fr->sizeHint());

        bool bVis = (numSources > 1);
        if (!bVis && pr.Activity == 0) bVis = true;
        e->setVisible(bVis);
    }

    if (curRow < 0 || curRow >= ui->lwDefinedParticleSources->count())
        curRow = 0;
    ui->lwDefinedParticleSources->setCurrentRow(curRow);
}

void AParticleSimWin::updateGeneralControlInResults()
{
    updateMonitorGui();
    updateCalorimeterGui();
}

void AParticleSimWin::on_lwDefinedParticleSources_itemDoubleClicked(QListWidgetItem *)
{
    on_pbEditParticleSource_clicked();
}

#include "afileparticlegenerator.h"
void AParticleSimWin::on_pbGunTest_clicked()
{
    AParticleGun * pg = nullptr;
    switch (ui->cobParticleGenerationMode->currentIndex())
    {
    case 0:
        pg = SimManager.Generator_Sources;
        break;
    case 1:
        pg = SimManager.Generator_File;
        SimManager.Generator_File->checkFile(false);  // !!!*** error?
        break;
    default:
        guitools::message("This generation mode is not implemented!", this);
        return;
    }

    AParticleSourcePlotter::clearTracks();
    if (ui->cobParticleGenerationMode->currentIndex() == 0)
    {
        if (ui->pbGunShowSource->isChecked())
            for (const AParticleSourceRecord & s : SimSet.SourceGenSettings.SourceData)
                AParticleSourcePlotter::plotSource(s);
    }

    emit requestBusyStatus(true);   // -->   !!!***
onBusyStatusChange(true);

    ui->pbAbort->setEnabled(true);
    QFont font = ui->pbAbort->font();
    font.setBold(true);
    ui->pbAbort->setFont(font);
    QApplication::processEvents();

    testParticleGun(pg, ui->sbGunTestEvents->value(), ui->cbShowStatistics->isChecked());

    ui->pbAbort->setEnabled(false);
    font.setBold(false);
    ui->pbAbort->setFont(font);
    emit requestBusyStatus(false);  // <--  !!!***
onBusyStatusChange(false);

    if (ui->cobParticleGenerationMode->currentIndex() == 1) updateFileParticleGeneratorGui();
}

void AParticleSimWin::configureAngleStat(AParticleGun * gun)
{
    CollectAngle = false;
    const ASourceParticleGenerator * ps = dynamic_cast<const ASourceParticleGenerator*>(gun);
    if (!ps) return;

    if (ps->Settings.getNumSources() > 1) return;

    const AParticleSourceRecord & sr = ps->Settings.SourceData.front();
    if (!sr.isDirectional()) return;

    CollectAngle = true;
    SourceStatDirection = ps->getCollimationDirection(0);
}

#include "TGeoManager.h"
void AParticleSimWin::testParticleGun(AParticleGun * gun, int numParticles, bool fillStatistics)
{
    AErrorHub::clear();

    if (!gun)
    {
        guitools::message("Particle gun is not defined", this);
        return;
    }

    bool bOK = gun->init();
    if (!bOK)
    {
        guitools::message( QString("Failed to initialize particle gun!\n%0").arg(AErrorHub::getError().data()), this);
        return;
    }
    gun->setStartEvent(0);

    const double WorldSizeXY = AGeometryHub::getInstance().getWorldSizeXY();
    const double WorldSizeZ  = AGeometryHub::getInstance().getWorldSizeZ();
    double Length = std::max(WorldSizeXY, WorldSizeZ)*0.4;

    int numTracks = 0;
    if (fillStatistics)
    {
        delete histTime;   histTime   = new TH1D("", "Time", 100,0,0);   histTime->GetXaxis()->SetTitle("Time, ns");
        delete histEnergy; histEnergy = new TH1D("", "Energy", 100,0,0); histEnergy->GetXaxis()->SetTitle("Energy, keV");
        delete histAngle;  histAngle  = new TH1D("", "Angle", 181,0,181);  histAngle->GetXaxis()->SetTitle("Angle, degrees");
        configureAngleStat(gun);
        SeenParticles.clear();
    }

    auto handler = [&numTracks, Length, fillStatistics, this](const AParticleRecord & particle)
    {
        //qDebug() << particle.particle.data();
        if (fillStatistics) addStatistics(particle);

        if (numTracks > 1000) return;
        int track_index = gGeoManager->AddTrack(1, 22);
        TVirtualGeoTrack * track = gGeoManager->GetTrack(track_index);
        AParticleTrackVisuals::getInstance().applyToParticleTrack(track, particle.particle.data());
        track->AddPoint(particle.r[0], particle.r[1], particle.r[2], 0);
        track->AddPoint(particle.r[0] + particle.v[0]*Length, particle.r[1] + particle.v[1]*Length, particle.r[2] + particle.v[2]*Length, 0);
        numTracks++;
        emit requestAddMarker(particle.r);
    };

    emit requestClearMarkers(0);

    for (int iRun = 0; iRun < numParticles; iRun++)
    {
        bool bOK = gun->generateEvent(handler, iRun);
        //if (!bOK || numTracks > 10000) break;
        if (!bOK) break;
    }

    if (gun->AbortRequested) return;

    emit requestShowGeometry(true, true, false);
    emit requestShowTracks();

    if (fillStatistics)
    {
        const size_t numParticles = SeenParticles.size();
        TH1D * histSeen = new TH1D("", "Seen particles", numParticles, 0, numParticles);
        TAxis * axis = histSeen->GetXaxis();
        int iBin = 1;
        for (auto const & pair : SeenParticles)
        {
            histSeen->SetBinContent(iBin, pair.second);
            axis->SetBinLabel(iBin, pair.first.data());
            iBin++;
        }
        histSeen->SetMinimum(0);
        emit requestDraw(histSeen,   "hist", true,  true); emit requestAddToBasket("Seen particles");
        emit requestDraw(histTime,   "hist", false, true); emit requestAddToBasket("Time");
        if (CollectAngle)
        {
            emit requestDraw(histAngle, "hist", false, true); emit requestAddToBasket("Angle");
        }
        emit requestDraw(histEnergy, "hist", false, true); emit requestAddToBasket("Energy");
    }
}

void AParticleSimWin::disableGui(bool flag)
{
    //setDisabled(flag);
    ui->ledEvents->setDisabled(flag);
    ui->pbConfigureOutput->setDisabled(flag);
    ui->pbSimulate->setDisabled(flag);

    ui->progbSim->setEnabled(flag);
    ui->pbAbort->setEnabled(flag);
    qApp->processEvents();
}

void AParticleSimWin::on_pbGunShowSource_toggled(bool checked)
{
    AParticleSourcePlotter::clearTracks();
    emit requestClearMarkers(0);

    if (checked)
    {
        emit requestShowGeometry(true, true, true);
        for (const AParticleSourceRecord & s : SimSet.SourceGenSettings.SourceData)
            AParticleSourcePlotter::plotSource(s);
        emit requestShowTracks();
    }
    else
    {
        emit requestShowGeometry(false, true, true);
    }
}

void AParticleSimWin::on_cobParticleGenerationMode_activated(int index)
{
    if      (index == 0) SimSet.GenerationMode = AParticleSimSettings::Sources;
    else if (index == 1) SimSet.GenerationMode = AParticleSimSettings::File;
    else                 SimSet.GenerationMode = AParticleSimSettings::Script;
}

void AParticleSimWin::on_ledEvents_editingFinished()
{
    SimSet.Events = ui->ledEvents->text().toDouble();
}

#include "aparticlesimoutputdialog.h"
void AParticleSimWin::on_pbConfigureOutput_clicked()
{
    AParticleSimOutputDialog d(this);
    d.exec();
}

#include "aconfig.h"
void AParticleSimWin::on_pbSimulate_clicked()
{
    A3Global::getInstance().saveConfig();
    AConfig::getInstance().save(A3Global::getInstance().QuicksaveDir + "/QuickSave0.json");

    clearResultsGui();

    disableGui(true);
    SimManager.simulate();
    disableGui(false);

    if (SimManager.isAborted())
    {
        ui->progbSim->setValue(0);
        return;
    }

    if (AErrorHub::isError())
    {
        guitools::message(AErrorHub::getQError(), this);
        if ( AErrorHub::getQError().contains("Exchange directory") ) emit requestConfigureExchangeDir();
    }
    else
    {
        updateResultsGui();
    }

    emit requestShowGeoObjectDelegate("", false);
}

void AParticleSimWin::updateResultsGui()
{
    ui->progbSim->setValue(100);

    ui->leWorkingDirectory->setText(SimSet.RunSet.OutputDirectory.data());
    LastDir_Working = ui->leWorkingDirectory->text();

    if (ui->cbAutoLoadResults->isChecked())
    {
        if (SimSet.RunSet.SaveTrackingHistory)
        {
            ui->leTrackingDataFile->setText(SimSet.RunSet.FileNameTrackingHistory.data());
            LastFile_Tracking = ui->leTrackingDataFile->text();

            on_pbShowTracks_clicked();
            EV_showTree();
        }

        if (SimSet.RunSet.MonitorSettings.Enabled)
        {
            ui->leMonitorsFileName->setText(SimSet.RunSet.MonitorSettings.FileName.data());
            LastFile_Monitors = ui->leMonitorsFileName->text();
            updateMonitorGui(); // data will be already loaded for merging
        }

        if (SimSet.RunSet.CalorimeterSettings.Enabled)
        {
            ui->leCalorimetersFileName->setText(SimSet.RunSet.CalorimeterSettings.FileName.data());
            LastFile_Calorimeters = ui->leCalorimetersFileName->text();
            updateCalorimeterGui(); // data will be already loaded for merging
        }

        if (SimSet.RunSet.SaveDeposition)
            ui->leDepositionFileName->setText(SimSet.RunSet.FileNameDeposition.data());
    }
}

void AParticleSimWin::onBusyStatusChange(bool busy)
{
    bool bEnable = !busy;

    ui->tabGeant4->setEnabled(bEnable);
    ui->tabResults->setEnabled(bEnable);

    ui->swParticleGenerationMode->setEnabled(bEnable);

    ui->cobParticleGenerationMode->setEnabled(bEnable);
    ui->pbGunTest->setEnabled(bEnable);
    ui->sbGunTestEvents->setEnabled(bEnable);
    ui->cbShowStatistics->setEnabled(bEnable);

    ui->ledEvents->setEnabled(bEnable);
    ui->pbConfigureOutput->setEnabled(bEnable);
    ui->pbSimulate->setEnabled(bEnable);
    ui->cbAutoLoadResults->setEnabled(bEnable);
}

void AParticleSimWin::on_pbLoadAllResults_clicked()
{
    QString dir = ui->leWorkingDirectory->text();
    AParticleRunSettings defaultSettings;

    //tracking data
    QString fileName(defaultSettings.FileNameTrackingHistory.data());
    ui->leTrackingDataFile->setText(fileName);
    if (QFile(dir + '/' + fileName).exists())
    {
        EV_showTree();
        on_pbShowTracks_clicked();
    }

    //monitors
    fileName = QString(defaultSettings.MonitorSettings.FileName.data());
    ui->leMonitorsFileName->setText(fileName);
    if (QFile(dir + '/' + fileName).exists())
        on_pbLoadMonitorsData_clicked();

    //calorimeters
    fileName = QString(defaultSettings.CalorimeterSettings.FileName.data());
    ui->leCalorimetersFileName->setText(fileName);
    if (QFile(dir + '/' + fileName).exists())
        on_pbLoadCalorimetersData_clicked();
}

void AParticleSimWin::onMaterialsChanged()
{
    QStringList mats = AMaterialHub::getInstance().getListOfMaterialNames();

    ui->cobPTHistVolMatFrom->clear();
    ui->cobPTHistVolMatFrom->addItems(mats);

    ui->cobPTHistVolMatTo->clear();
    ui->cobPTHistVolMatTo->addItems(mats);

    ui->cobPTHistVolMat->clear();
    ui->cobPTHistVolMat->addItems(mats);
}

void AParticleSimWin::onRequestShowSource()
{
    emit requestShowGeometry(false, true, true);
    emit requestShowTracks();
}

void AParticleSimWin::on_pbShowTracks_clicked()
{
    QString fileName = ui->leTrackingDataFile->text();
    LastFile_Tracking = fileName;
    if (!fileName.contains('/')) fileName = ui->leWorkingDirectory->text() + '/' + fileName;

    QStringList LimitTo;
    if (ui->cbLimitToParticleTracks->isChecked())
        LimitTo = ui->leLimitToParticleTracks->text().split(QRegularExpression("\\s|,"), Qt::SkipEmptyParts);
    QStringList Exclude;
    if (ui->cbExcludeParticleTracks->isChecked() && ui->leExcludeParticleTracks->isEnabled())
        Exclude = ui->leExcludeParticleTracks->text().split(QRegularExpression("\\s|,"), Qt::SkipEmptyParts);

    const int MaxTracks = ui->sbMaxTracks->value();

    const bool SkipPrimaries   = ui->cbSkipPrimaryTracks->isChecked();
    const bool SkipPrimNoInter = ui->cbSkipPrimaryTracksNoInteraction->isChecked();
    const bool SkipSecondaries = ui->cbSkipSecondaryTracks->isChecked();

    ATrackingDataExplorer explorer;
    AEventsDoneDialog dialog(this);
    connect(&explorer, &ATrackingDataExplorer::reportEventsProcessed, &dialog, &AEventsDoneDialog::onProgressReported);
    connect(&dialog, &AEventsDoneDialog::rejected, &explorer, &ATrackingDataExplorer::abortEventProcessing);
    dialog.setModal(true);
    dialog.show();
    QApplication::processEvents();

    QString err = explorer.buildTracks(fileName, LimitTo, Exclude,
                                       SkipPrimaries, SkipPrimNoInter, SkipSecondaries,
                                       MaxTracks, -1);

    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    emit requestShowGeometry(true, true, true);
    emit requestShowTracks();
}

#include <QFileDialog>
void AParticleSimWin::on_pbChooseWorkingDirectory_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select data directory",
                                                    SimSet.RunSet.OutputDirectory.data(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty())
    {
        ui->leWorkingDirectory->setText(dir);
        on_leWorkingDirectory_editingFinished();
    }
}

void AParticleSimWin::on_leWorkingDirectory_editingFinished()
{
    const QString str = ui->leWorkingDirectory->text();
    if (str == LastDir_Working) return;
    LastDir_Working = str;
    clearResults();
    clearResultsGui();
}

void AParticleSimWin::on_pbChooseFileTrackingData_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file with tracking data", ui->leWorkingDirectory->text());
    if (fileName.isEmpty()) return;

    //fileName = QFileInfo(fileName).completeBaseName() + "." + QFileInfo(fileName).suffix();
    ui->leTrackingDataFile->setText(fileName);
    on_leTrackingDataFile_editingFinished();
}

void AParticleSimWin::on_leTrackingDataFile_editingFinished()
{
    const QString str = ui->leTrackingDataFile->text();
    if (str == LastFile_Tracking) return;
    LastFile_Tracking = str;
    clearResultsTracking();
    clearResultsGuiTrackig();
}

void AParticleSimWin::on_cbGunAllowMultipleEvents_clicked(bool checked)
{
    SimSet.SourceGenSettings.MultiEnabled = checked;
}
void AParticleSimWin::on_cobPartPerEvent_activated(int index)
{
    SimSet.SourceGenSettings.MultiMode = (index == 0 ? ASourceGeneratorSettings::Constant : ASourceGeneratorSettings::Poisson);
}
void AParticleSimWin::on_ledGunAverageNumPartperEvent_editingFinished()
{
    SimSet.SourceGenSettings.MultiNumber = ui->ledGunAverageNumPartperEvent->text().toDouble();
}
void AParticleSimWin::on_cbGunAllowMultipleEvents_toggled(bool checked)
{
    ui->cobPartPerEvent->setVisible(checked);
    ui->labPartPerEvent->setVisible(checked);
    ui->ledGunAverageNumPartperEvent->setVisible(checked);
}
void AParticleSimWin::on_cobPartPerEvent_currentIndexChanged(int index)
{
    ui->labPartPerEvent->setText(index == 0 ? "per event" : "with mean");
}

// --- Event viewer ---

#include <QTreeWidget>
#include "aeventtrackingrecord.h"
#include "amaterialhub.h"
//#include "amaterial.h"

void AParticleSimWin::fillEvTabViewRecord(QTreeWidgetItem * item, const AParticleTrackingRecord * pr, int ExpansionLevel) const
{
    const AMaterialHub & MatHub = AMaterialHub::getConstInstance();

    item->setText(0, pr->ParticleName);
    qlonglong poi = reinterpret_cast<qlonglong>(pr);
    item->setText(1, QString("%1").arg(poi));
    //item->setFlags(w->flags() & ~Qt::ItemIsDragEnabled);// & ~Qt::ItemIsSelectable);

    if (ExpansionLevel > 0) ui->trwEventView->expandItem(item);
    ExpansionLevel--;

    int precision = ui->sbEVprecision->value();
    bool bHideTransp = ui->cbEVhideTrans->isChecked();
    bool bHideTranspPrim = ui->cbEVhideTransPrim->isChecked();
    bool bHideStepLim = ui->cbEVhideStepLim->isChecked();
    bool bHideStepLimPrim = ui->cbEVhideStepLimPrim->isChecked();
    bool bHideIoni = ui->cbEVhideIoni->isChecked();
    bool bHideIoniPrim = ui->cbEVhideIoniPrim->isChecked();

    bool bPos = ui->cbEVpos->isChecked();
    bool bStep = ui->cbEVstep->isChecked();
    bool bTime = ui->cbEVtime->isChecked();
    double timeUnits = 1.0;
    switch (ui->cobEVtime->currentIndex())
    {
    case 0: break;
    case 1: timeUnits *= 0.001; break;
    case 2: timeUnits *= 1.0e-6; break;
    case 3: timeUnits *= 1.0e-9; break;
    case 4: timeUnits *= 1.666666666666666e-11; break;
    }
    bool bVolume = ui->cbEVvol->isChecked();
    bool bKin = ui->cbEVkin->isChecked();
    bool bDepo = ui->cbEVdepo->isChecked();
    double kinUnits = 1.0;
    switch (ui->cobEVkin->currentIndex())
    {
    case 0: kinUnits *= 1.0e6;
    case 1: break;
    case 2: kinUnits *= 1.0e-3; break;
    }
    double depoUnits = 1.0;
    switch (ui->cobEVdepo->currentIndex())
    {
    case 0: depoUnits *= 1.0e6;
    case 1: break;
    case 2: depoUnits *= 1.0e-3; break;
    }
    bool bIndex = ui->cbEVvi->isChecked();
    bool bMat = ui->cbEVmat->isChecked();

    QString curVolume;
    int     curVolIndex;
    int     curMat;

    for (size_t iStep = 0; iStep < pr->getSteps().size(); iStep++)
    {
        ATrackingStepData * step = pr->getSteps().at(iStep);

        QString s = step->Process;

        if (step->Process == "C")
        {
            ATransportationStepData * trStep = static_cast<ATransportationStepData*>(step);
            curVolume = trStep->VolName;
            curVolIndex = trStep->VolIndex;
            curMat = trStep->iMaterial;
        }
        else if (step->Process == "T")
        {
            ATransportationStepData * trStep = dynamic_cast<ATransportationStepData*>(step);
            if (bHideTransp || (bHideTranspPrim && pr->isPrimary()) )
            {
                curVolume   = trStep->VolName;
                curVolIndex = trStep->VolIndex;
                curMat      = trStep->iMaterial;
                continue;
            }

            s += QString("  %1 (#%2, %3) -> %4 (#%5, %6)").arg(curVolume)
                                                          .arg(curVolIndex)
                                                          .arg(MatHub.getMaterialName(curMat))
                                                          .arg(trStep->VolName)
                                                          .arg(trStep->VolIndex)
                                                          .arg(MatHub.getMaterialName(trStep->iMaterial));
            //cannot set currents yet - the indication should still show the "from" values - remember about energy deposition during "T" step!
        }
        else if (!step->TargetIsotope.isEmpty()) s += QString(" (%0)").arg(step->TargetIsotope);

        if (bHideStepLim || (bHideStepLimPrim && pr->isPrimary()))
            if (step->Process == "StepLimiter") continue;

        if (bHideIoni || (bHideIoniPrim && pr->isPrimary()))
            if (step->Process == "hIoni" || step->Process == "ionIoni")
                if (step->Energy > 0) continue;

        if (bPos) s += QString("  (%1, %2, %3)").arg(step->Position[0], 0, 'g', precision).arg(step->Position[1], 0, 'g', precision).arg(step->Position[2], 0, 'g', precision);
        if (bStep)
        {
            double delta = 0;
            if (iStep != 0)
            {
                ATrackingStepData * prev = pr->getSteps().at(iStep-1);
                for (int i=0; i<3; i++)
                    delta += (step->Position[i] - prev->Position[i]) * (step->Position[i] - prev->Position[i]);
                delta = sqrt(delta);
            }
            s += QString("  %1mm").arg(delta, 0, 'g', precision);
        }

        if (step->Process != "O" && step->Process != "T")
        {
            if (bVolume) s += QString("  %1").arg(curVolume);
            if (bIndex)  s += QString("  %1").arg(curVolIndex);
            if (bMat)    s += QString("  %1").arg(MatHub.getMaterialName(curMat));
        }

        if (bTime)   s += QString("  t=%1").arg(step->Time * timeUnits, 0, 'g', precision);
        if (bDepo)   s += QString("  depo=%1").arg(step->DepositedEnergy * depoUnits, 0, 'g', precision);
        if (bKin)    s += QString("  E=%1").arg(step->Energy * kinUnits, 0, 'g', precision);

        QTreeWidgetItem * it = new QTreeWidgetItem(item);
        it->setText(0, s);
        qlonglong poi = reinterpret_cast<qlonglong>(pr);
        it->setText(1, QString("%1").arg(poi));
        poi = reinterpret_cast<qlonglong>(step);
        it->setText(2, QString("%1").arg(poi));

        if (ExpansionLevel > 0) ui->trwEventView->expandItem(it);

        for (int iSec : step->Secondaries)
        {
            QTreeWidgetItem * subItem = new QTreeWidgetItem(it);
            fillEvTabViewRecord(subItem, pr->getSecondaries().at(iSec), ExpansionLevel-1);
        }

        if (step->Process == "T")
        {
            ATransportationStepData * trStep = dynamic_cast<ATransportationStepData*>(step);
            curVolume = trStep->VolName;
            curVolIndex = trStep->VolIndex;
            curMat = trStep->iMaterial;
        }
    }
}

void AParticleSimWin::EV_showTree()
{
    ui->trwEventView->clear();

    QString fileName = ui->leTrackingDataFile->text();
    if (!fileName.contains('/')) fileName = ui->leWorkingDirectory->text() + '/' + fileName;

    ATrackingDataImporter tdi(fileName); // !!!*** make it persistent
    if (tdi.ErrorString.isEmpty()) tdi.extractEvent(ui->sbShowEvent->value(), CurrentEventRecord);
    if (!tdi.ErrorString.isEmpty())
    {
        guitools::message(tdi.ErrorString, this);
        CurrentEventRecord->clear();
        return;
    }
    // !!!*** add error processing, separetely process bad event index

    const int ExpLevel = ui->sbEVexpansionLevel->value();

    for (AParticleTrackingRecord * pr : CurrentEventRecord->getPrimaryParticleRecords())
    {
        QTreeWidgetItem * item = new QTreeWidgetItem(ui->trwEventView);
        fillEvTabViewRecord(item, pr, ExpLevel);
    }
}

void AParticleSimWin::on_pbShowEventTree_clicked()
{
    ExpandedItems.clear();
    int counter = 0;
    for (int i=0; i<ui->trwEventView->topLevelItemCount(); i++)
    {
        QTreeWidgetItem * item = ui->trwEventView->topLevelItem(i);
        doProcessExpandedStatus(item, counter, true);
    }

    EV_showTree();

    for (int i=0; i<ui->trwEventView->topLevelItemCount(); i++)
    {
        QTreeWidgetItem * item = ui->trwEventView->topLevelItem(i);
        doProcessExpandedStatus(item, counter, false);
    }
}

void AParticleSimWin::doProcessExpandedStatus(QTreeWidgetItem * item, int & counter, bool bStore)
{
    if (bStore)
    {
        ExpandedItems.push_back(item->isExpanded());
        for (int i=0; i<item->childCount(); i++)
            doProcessExpandedStatus(item->child(i), counter, bStore);
    }
    else
    {
        if (counter >= ExpandedItems.size()) return; // not expected
        if (ExpandedItems.at(counter)) ui->trwEventView->expandItem(item);
        else ui->trwEventView->collapseItem(item);
        counter++;
        for (int i=0; i<item->childCount(); i++)
            doProcessExpandedStatus(item->child(i), counter, bStore);
    }
}

void AParticleSimWin::on_sbEVexpansionLevel_valueChanged(int)
{
    EV_showTree();
}

void AParticleSimWin::on_cbEVhideTrans_clicked()
{
    EV_showTree();
}

void AParticleSimWin::on_cbEVhideTransPrim_clicked()
{
    EV_showTree();
}

void AParticleSimWin::on_cbEVhideStepLim_clicked()
{
    EV_showTree();
}

void AParticleSimWin::on_cbEVhideStepLimPrim_clicked()
{
    EV_showTree();
}

void AParticleSimWin::on_cbEVhideIoni_clicked()
{
    EV_showTree();
}

void AParticleSimWin::on_cbEVhideIoniPrim_clicked()
{
    EV_showTree();
}

bool AParticleSimWin::isTrackingDataFileExists()
{
    QString fileName = ui->leTrackingDataFile->text();
    if (!fileName.contains('/')) fileName = ui->leWorkingDirectory->text() + '/' + fileName;
    if (fileName.isEmpty())
    {
        guitools::message("File name with tracking data is not configured!", this);
        return false;
    }
    if (!QFileInfo::exists(fileName))
    {
        guitools::message("Configured file with tracking data not found!", this);
        return false;
    }
    return true;
}

void AParticleSimWin::on_pbEventView_clicked()
{
    if (!isTrackingDataFileExists()) return;

    EV_showTree();

    if (ui->cbEVtracks->isChecked())
    {
        QString fileName = ui->leTrackingDataFile->text();
        if (!fileName.contains('/')) fileName = ui->leWorkingDirectory->text() + '/' + fileName;

        ATrackingDataExplorer explorer;
        explorer.buildTracksForEventRecord(CurrentEventRecord, ui->cbEVsupressSec->isChecked());

        emit requestShowGeometry(true, true, true);
        emit requestShowTracks();
    }
}

// --- Statistics ---

#include "atrackinghistorycrawler.h"
#include "TH1D.h"
#include "TH2D.h"

void AParticleSimWin::on_pbPTHistRequest_clicked()
{
    if (!isTrackingDataFileExists()) return;

    setEnabled(false);

    ATrackingHistoryCrawler crawler(ui->leWorkingDirectory->text() + "/" + ui->leTrackingDataFile->text());

    AEventsDoneDialog dialog(this);
    connect(&crawler, &ATrackingHistoryCrawler::reportProgress, &dialog, &AEventsDoneDialog::onProgressReported);
    connect(&dialog, &AEventsDoneDialog::rejected, &crawler, &ATrackingHistoryCrawler::abort);
    dialog.setModal(true);
    dialog.show();
    QApplication::processEvents();

    AFindRecordSelector options;
    options.bParticle = ui->cbPTHistParticle->isChecked();
    options.Particle = ui->lePTHistParticle->text();
    options.bPrimary = ui->cbPTHistOnlyPrim->isChecked();
    options.bSecondary = ui->cbPTHistOnlySec->isChecked();
    options.bLimitToFirstInteractionOfPrimary = ui->cbPTHistLimitToFirst->isChecked();
    options.bTime = ui->cbPTHistTime->isChecked();
    options.TimeFrom = ui->lePTHistTimeFrom->text().toDouble();
    options.TimeTo = ui->lePTHistTimeTo->text().toDouble();

    int numThreads = ui->sbNumThreadsStatistics->value();
    int numEventsPerThread = ui->ledEventsPerThread->text().toDouble();
    if (numEventsPerThread < 1.0) numEventsPerThread = 1.0;

    int Selector = ui->twPTHistType->currentIndex();
    if (Selector == 0) findInBulk(crawler, options, numThreads, numEventsPerThread);
    else               findInTransitions(crawler, options, numThreads, numEventsPerThread);

    QTextCursor txtCursor = ui->ptePTHist->textCursor();
    txtCursor.setPosition(0);
    ui->ptePTHist->setTextCursor(txtCursor);

    setEnabled(true);
}

void AParticleSimWin::findInBulk(ATrackingHistoryCrawler & crawler, AFindRecordSelector & options, int numThreads, int numEventsPerThread)
{
    int    bins = ui->sbPTHistBinsX->value();
    double from = ui->ledPTHistFromX->text().toDouble();
    double to   = ui->ledPTHistToX  ->text().toDouble();

    int    bins2 = ui->sbPTHistBinsY->value();
    double from2 = ui->ledPTHistFromY->text().toDouble();
    double to2   = ui->ledPTHistToY  ->text().toDouble();

    options.bMaterial = ui->cbPTHistVolMat->isChecked();
    options.Material = ui->cobPTHistVolMat->currentIndex();
    options.bVolume = ui->cbPTHistVolVolume->isChecked();
    options.Volume = ui->lePTHistVolVolume->text().toLocal8Bit().data();
    options.bVolumeIndex = ui->cbPTHistVolIndex->isChecked();
    options.VolumeIndex = ui->sbPTHistVolIndex->value();

    int What = ui->cobPTHistVolRequestWhat->currentIndex();
    switch (What)
    {
    case 0:
      {
        AHistorySearchProcessor_findParticles p;
        crawler.find(options, p, numThreads, numEventsPerThread);
        ui->ptePTHist->clear();
        ui->ptePTHist->appendPlainText("Particle and number of times:\n");
        std::vector<std::pair<QString,int>> vec;
        p.getResults(vec);
        for (const auto & pair : vec)
            ui->ptePTHist->appendPlainText(QString("%1\t : %2").arg(pair.first).arg(pair.second));
        break;
      }
    case 1:
      {
        int mode = ui->cobPTHistVolPlus->currentIndex();
        if (mode < 0 || mode > 2)
        {
            guitools::message("Unknown process selection mode", this);
            setEnabled(true);
            return;
        }

        AHistorySearchProcessor_findProcesses::SelectionMode sm = static_cast<AHistorySearchProcessor_findProcesses::SelectionMode>(mode);
        AHistorySearchProcessor_findProcesses p(sm, ui->cbLimitToHadronic->isChecked(), ui->leLimitHadronicTarget->text());
        crawler.find(options, p, numThreads, numEventsPerThread);

        ui->ptePTHist->clear();
        ui->ptePTHist->appendPlainText("Process and number of times:\n");
        std::vector<std::pair<QString,int>> vec;
        p.getResults(vec);
        for (const auto & pair : vec)
            ui->ptePTHist->appendPlainText(QString("%1\t : %2").arg(pair.first).arg(pair.second));

        selectedModeForProcess = mode;
        break;
      }
    case 2:
      {
        AHistorySearchProcessor_findTravelledDistances p(bins, from, to);
        crawler.find(options, p, numThreads, numEventsPerThread);

        if (p.Hist->GetEntries() == 0)
            guitools::message("No trajectories found", this);
        else
        {
            emit requestDraw(p.Hist, "hist", true, true);
            p.Hist = nullptr;
        }
        binsDistance = bins;
        fromDistance = from;
        toDistance = to;

        break;
      }
    case 3:
      {
        int mode = ui->cobPTHistVolPlus->currentIndex();
        if (mode < 0 || mode > 2)
        {
            guitools::message("Unknown energy deposition collection mode", this);
            setEnabled(true);
            return;
        }
        AHistorySearchProcessor_findDepositedEnergy::CollectionMode edm = static_cast<AHistorySearchProcessor_findDepositedEnergy::CollectionMode>(mode);

        if (ui->cbPTHistVolVsTime->isChecked())
        {
            AHistorySearchProcessor_findDepositedEnergyTimed p(edm, bins, from, to, bins2, from2, to2);
            crawler.find(options, p, numThreads, numEventsPerThread);

            if (p.Hist2D->GetEntries() == 0)
                guitools::message("No deposition detected", this);
            else
            {
                emit requestDraw(p.Hist2D, "colz", true, true);
                p.Hist2D = nullptr;
            }
            binsTime = bins2;
            fromTime = from2;
            toTime   = to2;
        }
        else
        {
            AHistorySearchProcessor_findDepositedEnergy p(edm, bins, from, to);
            crawler.find(options, p, numThreads, numEventsPerThread);

            if (p.Hist->GetEntries() == 0)
                guitools::message("No deposition detected", this);
            else
            {
                emit requestDraw(p.Hist, "hist", true, true);
                p.Hist = nullptr;
            }
        }
        selectedModeForEnergyDepo = mode;
        binsEnergy = bins;
        fromEnergy = from;
        toEnergy = to;
        break;
      }
    case 4:
     {
        AHistorySearchProcessor_getDepositionStats * p = nullptr;
        if (ui->cbLimitTimeWindow->isChecked())
        {
            p = new AHistorySearchProcessor_getDepositionStatsTimeAware(ui->ledTimeFrom->text().toDouble(), ui->ledTimeTo->text().toDouble());
            crawler.find(options, *p, numThreads, numEventsPerThread);
        }
        else
        {
            p = new AHistorySearchProcessor_getDepositionStats();
            crawler.find(options, *p, numThreads, numEventsPerThread);
        }

        ui->ptePTHist->clear();
        ui->ptePTHist->appendPlainText("Deposition statistics:\n");

        std::vector< std::tuple<QString,double,double,int,double,double> > data;
        const double sum = p->getResults(data);

        for (const auto & el : data)
        {
            QString str = QString("%1\t%2 keV (%3%)\t#: %4")
                          .arg(std::get<0>(el))
                          .arg(std::get<1>(el))
                          .arg( QString::number(std::get<2>(el), 'g', 4) )
                          .arg(std::get<3>(el));

            if (std::get<4>(el) != 0) str += QString("\tmean: %1 keV").arg(std::get<4>(el));
            if (std::get<5>(el) != 0) str += QString("\tsigma: %1 keV").arg(std::get<5>(el));

            ui->ptePTHist->appendPlainText(str);
        }
        ui->ptePTHist->appendPlainText("\n---------\n");
        ui->ptePTHist->appendPlainText(QString("sum of all listed depositions: %1 keV").arg(sum));
        delete p;
        break;
     }
    case 5:
        {
            AHistorySearchProcessor_findHadronicChannels p;
            crawler.find(options, p, numThreads, numEventsPerThread);
            ui->ptePTHist->clear();
            ui->ptePTHist->appendPlainText("Hadronic channel and number of times:\n");
            std::vector<std::pair<QString,int>> vec;
            p.getResults(vec);
            for (const auto & pair : vec)
                ui->ptePTHist->appendPlainText(QString("%1\t : %2").arg(pair.first).arg(pair.second));
            break;
        }
    default:
        qWarning() << "Unknown type of volume request";
    }
}

void AParticleSimWin::findInTransitions(ATrackingHistoryCrawler & crawler, AFindRecordSelector & options, int numThreads, int numEventsPerThread)
{
    options.bFromMat = ui->cbPTHistVolMatFrom->isChecked();
    options.FromMat = ui->cobPTHistVolMatFrom->currentIndex();
    options.bFromVolume = ui->cbPTHistVolVolumeFrom->isChecked();
    options.bEscaping = ui->cbPTHistEscaping->isChecked();
    options.FromVolume = ui->lePTHistVolVolumeFrom->text().toLocal8Bit().data();
    options.bFromVolIndex = ui->cbPTHistVolIndexFrom->isChecked();
    options.FromVolIndex = ui->sbPTHistVolIndexFrom->value();

    options.bToMat = ui->cbPTHistVolMatTo->isChecked();
    options.ToMat = ui->cobPTHistVolMatTo->currentIndex();
    options.bToVolume = ui->cbPTHistVolVolumeTo->isChecked();
    options.bCreated = ui->cbPTHistCreated->isChecked();
    options.ToVolume = ui->lePTHistVolVolumeTo->text().toLocal8Bit().data();
    options.bToVolIndex = ui->cbPTHistVolIndexTo->isChecked();
    options.ToVolIndex = ui->sbPTHistVolIndexTo->value();

    int    bins = ui->sbPTHistBinsX->value();
    double from = ui->ledPTHistFromX->text().toDouble();
    double to   = ui->ledPTHistToX  ->text().toDouble();

    int    bins2 = ui->sbPTHistBinsY->value();
    double from2 = ui->ledPTHistFromY->text().toDouble();
    double to2   = ui->ledPTHistToY  ->text().toDouble();

    QString what = ui->lePTHistBordWhat->text();
    QString vsWhat = ui->lePTHistBordVsWhat->text();
    QString andVsWhat = ui->lePTHistBordAndVsWhat->text();
    QString cuts = ui->lePTHistBordCuts->text();

    bool bVs = ui->cbPTHistBordVs->isChecked();
    bool bVsVs = ui->cbPTHistBordAndVs->isChecked();
    bool bAveraged = ui->cbPTHistBordAsStat->isChecked();

    if (!bVs)
    {
        //1D stat
        AHistorySearchProcessor_Border p(what, cuts, bins, from, to);
        if (!p.ErrorString.isEmpty()) guitools::message(p.ErrorString, this);
        else
        {
            crawler.find(options, p, numThreads, numEventsPerThread);
            if (p.Hist1D->GetEntries() == 0) guitools::message("No data", this);
            else
            {
                emit requestDraw(p.Hist1D, "hist", true, true);
                p.Hist1D = nullptr;
            }
        }
    }
    else
    {
        // "vs" is activated
        if (!bVsVs && bAveraged)
        {
            //1D vs
            AHistorySearchProcessor_Border p(what, vsWhat, cuts, bins, from, to);
            if (!p.ErrorString.isEmpty()) guitools::message(p.ErrorString, this);
            else
            {
                crawler.find(options, p, numThreads, numEventsPerThread);
                if (p.Hist1D->GetEntries() == 0) guitools::message("No data", this);
                else
                {
                    emit requestDraw(p.Hist1D, "hist", true, true);
                    p.Hist1D = nullptr;
                }
            }
        }
        else if (!bVsVs && !bAveraged)
        {
            //2D stat
            AHistorySearchProcessor_Border p(what, vsWhat, cuts, bins, from, to, bins2, from2, to2);
            if (!p.ErrorString.isEmpty()) guitools::message(p.ErrorString, this);
            else
            {
                crawler.find(options, p, numThreads, numEventsPerThread);
                if (p.Hist2D->GetEntries() == 0) guitools::message("No data", this);
                else
                {
                    emit requestDraw(p.Hist2D, "colz", true, true);
                    p.Hist2D = nullptr;
                }
            }
            binsB2 = bins2;
            fromB2 = from2;
            toB2 = to2;
        }
        else if (bVsVs)
        {
            //2D vsvs
            AHistorySearchProcessor_Border p(what, vsWhat, andVsWhat, cuts, bins, from, to, bins2, from2, to2);
            if (!p.ErrorString.isEmpty()) guitools::message(p.ErrorString, this);
            else
            {
                crawler.find(options, p, numThreads, numEventsPerThread);
                if (p.Hist2D->GetEntries() == 0) guitools::message("No data", this);
                else
                {
                    emit requestDraw(p.Hist2D, "colz", true, true);
                    p.Hist2D = nullptr;
                }
            }
            binsB2 = bins2;
            fromB2 = from2;
            toB2 = to2;
        }
        else guitools::message("Unexpected mode!", this);
    }
    binsB1 = bins;
    fromB1 = from;
    toB1 = to;
}

void AParticleSimWin::on_cbPTHistOnlyPrim_clicked(bool checked)
{
    if (checked) ui->cbPTHistOnlySec->setChecked(false);
}

void AParticleSimWin::on_cbPTHistOnlySec_clicked(bool checked)
{
    if (checked) ui->cbPTHistOnlyPrim->setChecked(false);
}

void AParticleSimWin::on_cobPTHistVolRequestWhat_currentIndexChanged(int index)
{
    updatePTHistoryBinControl();

    if (index == 1)
    {
        ui->cobPTHistVolPlus->clear();
        ui->cobPTHistVolPlus->addItems(QStringList() << "All"<<"With energy deposition"<<"Track end");
        ui->cobPTHistVolPlus->setCurrentIndex(selectedModeForProcess);
    }
    else if (index == 2)
    {
        ui->sbPTHistBinsX->setValue(binsDistance);
        ui->ledPTHistFromX->setText(QString::number(fromDistance));
        ui->ledPTHistToX->setText(QString::number(toDistance));
    }
    else if (index == 3)
    {
        ui->sbPTHistBinsX->setValue(binsEnergy);
        ui->ledPTHistFromX->setText(QString::number(fromEnergy));
        ui->ledPTHistToX->setText(QString::number(toEnergy));

        ui->sbPTHistBinsY->setValue(binsTime);
        ui->ledPTHistFromY->setText(QString::number(fromTime));
        ui->ledPTHistToY->setText(QString::number(toTime));

        ui->cobPTHistVolPlus->clear();
        ui->cobPTHistVolPlus->addItems(QStringList() << "Individual"<<"With secondaries"<<"Over event");
        ui->cobPTHistVolPlus->setCurrentIndex(selectedModeForEnergyDepo);
    }
    ui->cobPTHistVolPlus->setVisible(index == 1 || index == 3);

    ui->frLimitHadronicTarget->setVisible(index == 1);
    ui->frTimeAware->setVisible(index == 4);

    ui->cbPTHistVolVsTime->setVisible(index == 3);
}

void AParticleSimWin::on_twPTHistType_currentChanged(int index)
{
    if (index == 0)
        on_cobPTHistVolRequestWhat_currentIndexChanged(ui->cobPTHistVolRequestWhat->currentIndex());
    else
    {
        ui->sbPTHistBinsX->setValue(binsB1);
        ui->ledPTHistFromX->setText(QString::number(fromB1));
        ui->ledPTHistToX->setText(QString::number(toB1));
        ui->sbPTHistBinsY->setValue(binsB2);
        ui->ledPTHistFromY->setText(QString::number(fromB2));
        ui->ledPTHistToY->setText(QString::number(toB2));
    }

    updatePTHistoryBinControl();
}

void AParticleSimWin::updatePTHistoryBinControl()
{
    if (ui->twPTHistType->currentIndex() == 0)
    {
        //Volume
        const int sel = ui->cobPTHistVolRequestWhat->currentIndex();
        ui->frPTHistX->setVisible( sel == 2 || sel == 3);
        ui->frPTHistY->setVisible( sel == 3 && ui->cbPTHistVolVsTime->isChecked() );
    }
    else
    {
        //Border
        bool bVs = ui->cbPTHistBordVs->isChecked();
        ui->lePTHistBordVsWhat->setEnabled(bVs);
        ui->cbPTHistBordAndVs->setEnabled(bVs);
        if (!bVs) ui->cbPTHistBordAndVs->setChecked(false);

        bool bVsVs = ui->cbPTHistBordAndVs->isChecked();
        if (bVsVs) ui->cbPTHistBordAsStat->setChecked(true);
        ui->lePTHistBordAndVsWhat->setEnabled(bVs && bVsVs);
        ui->cbPTHistBordAsStat->setEnabled(bVs && !bVsVs);
        bool bAveraged = ui->cbPTHistBordAsStat->isChecked();

        ui->frPTHistX->setVisible(true);
        ui->frPTHistY->setVisible(bVsVs || (bVs && !bAveraged));
    }
}

void AParticleSimWin::on_cbPTHistBordVs_toggled(bool)
{
    updatePTHistoryBinControl();
}

void AParticleSimWin::on_cbPTHistBordAndVs_toggled(bool)
{
    updatePTHistoryBinControl();
}

void AParticleSimWin::on_cbPTHistBordAsStat_toggled(bool)
{
    updatePTHistoryBinControl();
}

// ---- from file ----

void AParticleSimWin::on_pbGenerateFromFile_Help_clicked()
{
    QString s = "Currently two formats are supported:\n"
                "\n"
                "1. G4ants ascii file\n"
                "Each event is marked with a special line,\n"
                "starting with '#' symbol immediately followed by the event index.\n"
                "The first event should have index of 0: #0\n"
                "Each record of a particle occupies one line, the format is:\n"
                "ParticleName Energy X Y Z DirX DirY DirZ Time\n"
                "\n"
                "2. G4ants binary file\n"
                "Each new entry starts either with a (char)EE or a (char)FF\n"
                "0xEE is followed by the event number (int),\n"
                "0xFF is followed by the particle record, which is\n"
                "ParticleName Energy X Y Z DirX DirY DirZ Time\n"
                "where ParticleName is 0-terminated string and the rest are doubles\n"
                "\n"
                "Energy is in keV, Position is in mm,\n"
                "Direction is unitary vector and Time is in ns.\n";

    guitools::message(s, this);
}

void AParticleSimWin::on_pbGenerateFromFile_Change_clicked()
{
    QString fileName = guitools::dialogLoadFile(this, "Select a file with particle generation data", "All files (*)");
    if (fileName.isEmpty()) return;
    ui->leGenerateFromFile_FileName->setText(fileName);
    on_leGenerateFromFile_FileName_editingFinished();
}

void AParticleSimWin::on_leGenerateFromFile_FileName_editingFinished()
{
    const std::string newName = ui->leGenerateFromFile_FileName->text().toLatin1().data();
    if (newName == SimSet.FileGenSettings.FileName) return;

    SimSet.FileGenSettings.clear();
    SimSet.FileGenSettings.FileName = newName;
    updateFileParticleGeneratorGui();
}

void AParticleSimWin::updateFileParticleGeneratorGui()
{
    const QString FileName(SimSet.FileGenSettings.FileName.data());
    ui->leGenerateFromFile_FileName->setText(FileName);

    ui->labFileFormat->setText("Undefined");
    ui->labNumberEventsInFile->setText("--");
    ui->lwFileStatistics->clear();

    QFileInfo fi(FileName);
    if (!fi.exists())
    {
        ui->labFileFormat->setText("File not found");
        return;
    }

    if (SimSet.FileGenSettings.isValidated())
    {
        ui->labFileFormat->setText( QString(SimSet.FileGenSettings.getFormatName().data()) );

        QString numStr = QString::number(SimSet.FileGenSettings.NumEvents);
        if (SimSet.FileGenSettings.statNumEmptyEventsInFile > 0)
            numStr += QString(", empty %1").arg(SimSet.FileGenSettings.statNumEmptyEventsInFile);
        if (SimSet.FileGenSettings.statNumMultipleEvents    > 0)
            numStr += QString(", multiple %1").arg(SimSet.FileGenSettings.statNumMultipleEvents);
        ui->labNumberEventsInFile->setText(numStr);

        for (const AParticleInFileStatRecord & rec : SimSet.FileGenSettings.ParticleStat)
        {
            ui->lwFileStatistics->addItem( QString("%1 \t# %2 \t <E>: %4 keV")
                                           .arg(rec.Name.data())
                                           .arg(rec.Entries)
                                           .arg( QString::number(rec.Energy / rec.Entries, 'g', 6) ) );
        }
    }
    else ui->lwFileStatistics->addItem(" Click 'Analyse file' to see statistics");
}

void AParticleSimWin::on_pbAnalyzeFile_clicked()
{
    AErrorHub::clear();

    AFileParticleGenerator * pg = SimManager.Generator_File;
//    WindowNavigator->BusyOn();  // -->
    pg->checkFile(ui->cbFileCollectStatistics->isChecked());
//    WindowNavigator->BusyOff(); // <--

    if (AErrorHub::isError()) guitools::message(AErrorHub::getError().data(), this);
    updateFileParticleGeneratorGui();
}

void AParticleSimWin::on_pbFilePreview_clicked()
{
    AErrorHub::clear();

    bool ok = SimManager.Generator_File->checkFile(false);
    if (!ok)
        guitools::message(AErrorHub::getError().data(), this);
    else
    {
        QString out(SimManager.Generator_File->getPreview(100).data());
        guitools::message1(out, SimSet.FileGenSettings.FileName.data(), this);
    }
}

// ------------------ monitors -----------------------

void AParticleSimWin::on_pbChooseMonitorsFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file with data recorded by particle monitors", SimSet.RunSet.OutputDirectory.data());
    if (fileName.isEmpty()) return;

    ui->leMonitorsFileName->setText(fileName);
    on_leMonitorsFileName_editingFinished();
}

void AParticleSimWin::on_leMonitorsFileName_editingFinished()
{
    const QString str = ui->leMonitorsFileName->text();
    if (str == LastFile_Monitors) return;
    LastFile_Monitors = str;
    clearResultsMonitors();
    clearResultsGuiMonitors();
}

void AParticleSimWin::on_pbLoadMonitorsData_clicked()
{
    QString FileName = ui->leMonitorsFileName->text();
    LastFile_Monitors = FileName;
    if (!FileName.contains('/'))
        FileName = ui->leWorkingDirectory->text() + '/' + FileName;

    MonitorHub.clearData(AMonitorHub::Particle);

    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, FileName);
    if(!ok)
    {
        guitools::message("Could not open: " + FileName, this);
        return;
    }

    QString err = MonitorHub.appendDataFromJson(json, AMonitorHub::Particle);
    if (!err.isEmpty()) guitools::message(err, this);
    updateMonitorGui();
}

void AParticleSimWin::updateMonitorGui()
{
    const int numMonitors = MonitorHub.countMonitors(AMonitorHub::Particle);
    ui->labNumMonitors->setText(QString::number(numMonitors));
    const int numWithHits = MonitorHub.countMonitorsWithHits(AMonitorHub::Particle);
    ui->labMonitorsWithHits->setText(QString::number(numWithHits));

    ui->frMonitors->setVisible(numMonitors != 0);

    if (numMonitors > 0)
    {
        const int oldNum = ui->cobMonitor->currentIndex();

        ui->cobMonitor->clear();
        for (int i = 0; i < numMonitors; i++)
            //ui->cobMonitor->addItem( QString("%1   index=%2").arg(MonitorHub.Monitors[i].Name).arg(i));
            ui->cobMonitor->addItem(MonitorHub.ParticleMonitors[i].Name);

        if (oldNum >-1 && oldNum < numMonitors)
        {
            ui->cobMonitor->setCurrentIndex(oldNum);
            ui->sbMonitorIndex->setValue(oldNum);
        }
        else ui->sbMonitorIndex->setValue(0);

        const int imon = ui->cobMonitor->currentIndex();
        const AMonitor & Mon = *MonitorHub.ParticleMonitors[imon].Monitor;
        ui->leDetections->setText( QString::number(Mon.getHits()) );

        ui->pbMonitorShowXY->setEnabled(Mon.xy);
        ui->pbMonitorShowTime->setEnabled(Mon.time);
        ui->pbMonitorShowAngle->setEnabled(Mon.angle);
        ui->pbMonitorShowEnergy->setEnabled(Mon.energy);
    }
}

void AParticleSimWin::on_cobMonitor_activated(int)
{
    updateMonitorGui();
}

void AParticleSimWin::on_sbMonitorIndex_editingFinished()
{
    int mon = ui->sbMonitorIndex->value();
    if (mon >= ui->cobMonitor->count()) mon = 0;
    ui->sbMonitorIndex->setValue(mon);
    if (mon < ui->cobMonitor->count()) ui->cobMonitor->setCurrentIndex(mon); //protection: can be empty
    updateMonitorGui();
}

void AParticleSimWin::on_pbNextMonitor_clicked()
{
    int numMon = MonitorHub.countMonitors(AMonitorHub::Particle);
    if (numMon == 0) return;

    int iMon = ui->cobMonitor->currentIndex();
    int iMonStart = iMon;
    int hits;
    do
    {
        iMon++;
        if (iMon >= numMon) iMon = 0;
        if (iMon == iMonStart) return;
        hits = MonitorHub.ParticleMonitors[iMon].Monitor->getHits();
    }
    while (hits == 0);

    if (iMon < ui->cobMonitor->count()) ui->cobMonitor->setCurrentIndex(iMon);
    updateMonitorGui();
}

void AParticleSimWin::on_pbShowMonitorProperties_clicked()
{
    const int iMon = ui->cobMonitor->currentIndex();
    const int numMon = MonitorHub.countMonitors(AMonitorHub::Particle);
    if (iMon < 0 || iMon >= numMon) return;

    AGeoObject * obj = MonitorHub.ParticleMonitors[iMon].GeoObj;
    if (obj) emit requestShowGeoObjectDelegate(obj->Name, true);
}

void AParticleSimWin::on_pbMonitorShowAngle_clicked()
{
    const int numMonitors = MonitorHub.countMonitors(AMonitorHub::Particle);
    const int iMon = ui->cobMonitor->currentIndex();
    if (iMon >=0 && iMon < numMonitors)
    {
        TH1D * h = MonitorHub.ParticleMonitors[iMon].Monitor->angle;
        h->GetXaxis()->SetTitle("Angle of incidence, deg");
        emit requestDraw(h, "hist", false, true);
    }
}

void AParticleSimWin::on_pbMonitorShowXY_clicked()
{
    const int numMonitors = MonitorHub.countMonitors(AMonitorHub::Particle);
    const int iMon = ui->cobMonitor->currentIndex();
    if (iMon >=0 && iMon < numMonitors)
    {
        TH2D * h = MonitorHub.ParticleMonitors[iMon].Monitor->xy;
        h->GetXaxis()->SetTitle("Local X, mm");
        h->GetYaxis()->SetTitle("Local Y, mm");
        emit requestDraw(h, "colz", false, true);
    }
}

void AParticleSimWin::on_pbMonitorShowTime_clicked()
{
    const int numMonitors = MonitorHub.countMonitors(AMonitorHub::Particle);
    const int iMon = ui->cobMonitor->currentIndex();
    if (iMon >=0 && iMon < numMonitors)
    {
        TH1D * time = MonitorHub.ParticleMonitors[iMon].Monitor->time;
        TString title = "Time, ";
        title += MonitorHub.ParticleMonitors[iMon].Monitor->config.timeUnits.toLatin1().data();
        time->GetXaxis()->SetTitle(title);
        emit requestDraw(time, "hist", false, true);
    }
}

void AParticleSimWin::on_pbMonitorShowEnergy_clicked()
{
    const int numMonitors = MonitorHub.countMonitors(AMonitorHub::Particle);
    const int iMon = ui->cobMonitor->currentIndex();
    if (iMon >=0 && iMon < numMonitors)
    {
        TH1D * energy = MonitorHub.ParticleMonitors[iMon].Monitor->energy;
        TString title = "Energy, ";
        title += MonitorHub.ParticleMonitors[iMon].Monitor->config.energyUnits.toLatin1().data();
        energy->GetXaxis()->SetTitle(title);
        emit requestDraw(energy, "hist", false, true);
    }
}

void AParticleSimWin::on_pbShowMonitorHitDistribution_clicked()
{
    const int numMon = MonitorHub.countMonitors(AMonitorHub::Particle);
    if (numMon == 0) return;

    TH1D * h = new TH1D("", "Monitor hits", numMon, 0, numMon);
    int sumHits = 0;
    for (int iMon = 0; iMon < numMon; iMon++)
    {
        int hits = MonitorHub.ParticleMonitors[iMon].Monitor->getHits();
        sumHits += hits;
        if (hits > 0) h->Fill(iMon, hits);
    }

    if (sumHits == 0) return;
    h->SetEntries(sumHits);
    h->GetXaxis()->SetTitle("Monitor index");
    h->GetYaxis()->SetTitle("Hits");
    emit requestDraw(h, "hist", true, true);
}

void AParticleSimWin::on_pbShowMonitorTimeOverall_clicked()
{
    const int numMon = MonitorHub.countMonitors(AMonitorHub::Particle);
    if (numMon == 0) return;

    int    bins = 1000;
    double from = -1e30;
    double to   = +1e30;

    for (int iM = 0; iM < numMon; iM++)
    {
        TH1D * hh = MonitorHub.ParticleMonitors[iM].Monitor->time;
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
        TH1D * h = MonitorHub.ParticleMonitors[iMon].Monitor->time;
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

void AParticleSimWin::on_sbShowEvent_editingFinished()
{
    on_pbEventView_clicked();
}

#include "atrackdrawdialog.h"
void AParticleSimWin::on_pbConfigureTrackStyles_clicked()
{
    ATrackDrawDialog D(this);
    int res = D.exec();

    A3Global & GlobSet = A3Global::getInstance();
    AParticleTrackVisuals & vis = AParticleTrackVisuals::getInstance();

    if (res == QDialog::Accepted)
    {
        vis.writeToJson(GlobSet.TrackVisAttributes);
        GlobSet.saveConfig();
    }
    else
    {
        vis.readFromJson(GlobSet.TrackVisAttributes);
    }
}

void AParticleSimWin::on_cbLimitToParticleTracks_toggled(bool checked)
{
    ui->leLimitToParticleTracks->setEnabled(checked);

    ui->cbExcludeParticleTracks->setEnabled(!checked);
    ui->leExcludeParticleTracks->setEnabled(!checked && ui->cbExcludeParticleTracks->isChecked());
}

void AParticleSimWin::on_cbExcludeParticleTracks_toggled(bool checked)
{
    ui->leExcludeParticleTracks->setEnabled(checked && !ui->cbLimitToParticleTracks->isChecked());
}

#include <QMenu>
#include "TGraph.h"
#include "agraphbuilder.h"
void AParticleSimWin::on_trwEventView_customContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem * item = ui->trwEventView->currentItem();
    if (!item) return;

    AParticleTrackingRecord * pr = nullptr;
    QString s = item->text(1);
    if (!s.isEmpty())
    {
        qlonglong sp = s.toLongLong();
        pr = reinterpret_cast<AParticleTrackingRecord*>(sp);
    }
    ATrackingStepData * st = nullptr;
    s = item->text(2);
    if (!s.isEmpty())
    {
        qlonglong sp = s.toLongLong();
        st = reinterpret_cast<ATrackingStepData*>(sp);
    }

    if (!pr) return;

    QMenu Menu;
    QAction * showPosition = nullptr; if (st) showPosition = Menu.addAction("Show position");
    QAction * centerA      = nullptr; if (st) centerA = Menu.addAction("Center view at this position");
    Menu.addSeparator();
    QAction * showDepo     = Menu.addAction("Show energy deposition graph");

    QAction* selectedItem = Menu.exec(ui->trwEventView->mapToGlobal(pos));
    if (!selectedItem) return; //nothing was selected

    if (selectedItem == showDepo)
    {
        std::vector<std::pair<double,double>> dist;
        pr->fillDepositionData(dist);
        if (!dist.empty())
        {
            TGraph * g = AGraphBuilder::graph(dist);
            AGraphBuilder::configure(g, "Deposited energy by step", "Travelled distance, mm", "Deposited energy over step, keV", 4, 20, 1, 4);
            emit requestDraw(g, "APL", true, true);
        }
    }
    else if (selectedItem == showPosition)
    {
        double pos[3];
        for (int i=0; i<3; i++) pos[i] = st->Position[i];
        emit requestShowPosition(pos, true);
    }
    else if (selectedItem == centerA)
    {
        double pos[3];
        for (int i=0; i<3; i++) pos[i] = st->Position[i];
        emit requestCenterView(pos);
    }
}

void AParticleSimWin::on_pbPreviousEvent_clicked()
{
    if (!isTrackingDataFileExists()) return;

    int curEv = ui->sbShowEvent->value();
    if (curEv == 0) return;
    int ev = findEventWithFilters(curEv, false);
    if (ev == -1)
        guitools::message("Cannot find events according to the selected criteria", this);
    else if (ev >= 0)
    {
        ui->sbShowEvent->setValue(ev);
        on_pbEventView_clicked();
    }
}

void AParticleSimWin::on_pbNextEvent_clicked()
{
    if (!isTrackingDataFileExists()) return;

    int curEv = ui->sbShowEvent->value();
    int ev = findEventWithFilters(curEv, true);
    if (ev == -1)
    {
        guitools::message("Cannot find events according to the selected criteria", this);
    }
    else if (ev >= 0)
    {
        ui->sbShowEvent->setValue(ev);
        on_pbEventView_clicked();
    }
}

void AParticleSimWin::abortFind()
{
    bFindEventAbortRequested = true;
}

int AParticleSimWin::findEventWithFilters(int currentEv, bool bUp)
{
    bFindEventAbortRequested = false;

    QDialog dReport(this);
    dReport.setWindowTitle("Event finder");
        QHBoxLayout * l = new QHBoxLayout();
        dReport.setLayout(l);
        l->addWidget(new QLabel("Event #"));
            QLabel * labEvent = new QLabel("");
        l->addWidget(labEvent);
            QPushButton * pbAbort = new QPushButton("Abort");
            QObject::connect(pbAbort, &QPushButton::clicked, this, &AParticleSimWin::abortFind);
            QObject::connect(&dReport, &QDialog::rejected, this, &AParticleSimWin::abortFind);
        l->addWidget(pbAbort);
    dReport.show();

    const QRegularExpression rx = QRegularExpression("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'

    const bool bLimProc = ui->cbEVlimToProc->isChecked();
    const bool bLimProc_prim = ui->cbEVlimitToProcPrim->isChecked();

    const bool bExclProc = ui->cbEVexcludeProc->isChecked();
    const bool bExclProc_prim = ui->cbEVexcludeProcPrim->isChecked();

    const bool bLimVols = ui->cbLimitToVolumes->isChecked();

    const bool bLimParticles = ui->cbLimitToParticles->isChecked();
    const bool bExcludeParticles = ui->cbExcludeParticles->isChecked();

    const QStringList LimProc = ui->leEVlimitToProc->text().split(rx, Qt::SkipEmptyParts);
    const QStringList ExclProc = ui->leEVexcludeProc->text().split(rx, Qt::SkipEmptyParts);
    QStringList LimVols = ui->leLimitToVolumes->text().split(rx, Qt::SkipEmptyParts);

    const QStringList MustContainParticles = ui->leLimitToParticles->text().split(rx, Qt::SkipEmptyParts);
    const QStringList ExcludeParticles = ui->leExcludeParticles->text().split(rx, Qt::SkipEmptyParts);

    AEventTrackingRecord * er = AEventTrackingRecord::create();
    QString fileName = ui->leTrackingDataFile->text();
    if (!fileName.contains('/')) fileName = ui->leWorkingDirectory->text() + '/' + fileName;
    ATrackingDataImporter tdi(fileName);
    if (!tdi.ErrorString.isEmpty())
    {
        guitools::message(tdi.ErrorString, this);
        return -1;
    }

    bUp ? currentEv++ : currentEv--;
    while (currentEv >= 0)
    {
        bool ok = tdi.extractEvent(currentEv, er);
        if (!ok)
        {
            guitools::message(tdi.ErrorString, this);
            return -1;
        }

        bool bGood = true;
        if (bLimProc)           bGood = er->isHaveProcesses(LimProc, bLimProc_prim);
        if (bGood && bExclProc) bGood = !er->isHaveProcesses(ExclProc, bExclProc_prim);
        if (bGood && bLimVols)
        {
            QStringList LimVolStartWith;
            for (int i=LimVols.size()-1; i >= 0; i--)
            {
                const QString & s = LimVols.at(i);
                if (s.endsWith('*'))
                {
                    LimVolStartWith << s.mid(0, s.size()-1);
                    LimVols.removeAt(i);
                }
            }
            bGood = er->isTouchedVolumes(LimVols, LimVolStartWith);
        }
        if (bGood && bLimParticles)     bGood = er->isContainParticle(MustContainParticles);
        if (bGood && bExcludeParticles) bGood = !er->isContainParticle(ExcludeParticles);

        if (bGood) return currentEv;

        bUp ? currentEv++ : currentEv--;

        QApplication::processEvents();
        if (bFindEventAbortRequested) return -2;
        labEvent->setText(QString::number(currentEv));
    };
    return -1;
}

void AParticleSimWin::onProgressReceived(double progress)
{
    if (!ui->progbSim->isEnabled()) return; // simulation is not running
    ui->progbSim->setValue(progress * 100.0);
}

void AParticleSimWin::on_cbPTHistVolVsTime_toggled(bool)
{
    updatePTHistoryBinControl();
}

void AParticleSimWin::on_pbUpdateIcon_clicked()
{
    std::vector<QCheckBox*> vec = {ui->cbEVlimToProc, ui->cbEVexcludeProc, ui->cbLimitToVolumes, ui->cbLimitToParticles, ui->cbExcludeParticles};

    bool vis = false;
    for (const auto cb : vec)
        if (cb->isChecked())
        {
            vis = true;
            break;
        }

    ui->labFilterActivated->setVisible(vis);
}

// ------------------ calorimeters -----------------------

void AParticleSimWin::on_pbChooseCalorimetersFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file with data recorded by calorimeters", SimSet.RunSet.OutputDirectory.data());
    if (fileName.isEmpty()) return;

    ui->leCalorimetersFileName->setText(fileName);
    on_leCalorimetersFileName_editingFinished();
}

void AParticleSimWin::on_leCalorimetersFileName_editingFinished()
{
    const QString str = ui->leCalorimetersFileName->text();
    if (str == LastFile_Calorimeters) return;
    LastFile_Calorimeters = str;
    clearResultsCalorimeters();
    clearResultsGuiCalorimeters();
}

void AParticleSimWin::on_pbLoadCalorimetersData_clicked()
{
    QString FileName = ui->leCalorimetersFileName->text();
    LastFile_Calorimeters = FileName;
    if (!FileName.contains('/'))
        FileName = ui->leWorkingDirectory->text() + '/' + FileName;

    CalHub.clearData();

    QJsonArray jsar;
    bool ok = jstools::loadJsonArrayFromFile(jsar, FileName);
    if (!ok)
    {
        guitools::message("Could not open: " + FileName, this);
        return;
    }

    QString err = CalHub.appendDataFromJson(jsar);
    if (!err.isEmpty()) guitools::message(err, this);
    updateCalorimeterGui();
}

void AParticleSimWin::updateCalorimeterGui()
{
    const int numCal = CalHub.countCalorimeters();
    ui->labNumCalorimeters->setText(QString::number(numCal));
    const int numWithData = CalHub.countCalorimetersWithData();
    ui->labNumCalWithData->setText(QString::number(numWithData));

    ui->frCaloShow->setVisible(numCal != 0);

    if (numCal > 0)
    {
        const int oldNum = ui->cobCalorimeter->currentIndex();

        ui->cobCalorimeter->clear();
        ui->cobCalorimeter->addItems(CalHub.getCalorimeterNames());

        if (oldNum >-1 && oldNum < numCal)
        {
            ui->cobCalorimeter->setCurrentIndex(oldNum);
            ui->sbCalorimeterIndex->setValue(oldNum);
        }
        else ui->sbCalorimeterIndex->setValue(0);

        const int iCal = ui->cobCalorimeter->currentIndex();
        const ACalorimeter * Cal = CalHub.Calorimeters[iCal].Calorimeter;

        if (Cal)
        {
            ui->leCalorimetersEntries->setText( QString::number(Cal->Entries) );
            ui->pbCaloShow->setEnabled(Cal->DataHistogram);
            ui->frCaloShowDepoOverEvent->setVisible(Cal->EventDepoData);
            updateShowCalorimeterGui();
        }
        else
        {
            ui->leCalorimetersEntries->setText("--");
            ui->pbCaloShow->setEnabled(false);
        }
    }
}

void AParticleSimWin::addStatistics(const AParticleRecord & p)
{
    histTime->Fill(p.time, 1);
    histEnergy->Fill(p.energy, 1);
    if (CollectAngle)
    {
        AVector3 particleDir(p.v);
        histAngle->Fill(particleDir.angle(SourceStatDirection)*180.0/3.1415926535, 1);
    }
    SeenParticles[p.particle]++;
}

void AParticleSimWin::on_pbNextCalorimeter_clicked()
{
    int numCal = CalHub.countCalorimeters();
    if (numCal == 0) return;

    int iCal = ui->cobCalorimeter->currentIndex();
    //int iCalStart = iCal;

    double depo = 0;
    do
    {
        iCal++;
        if (iCal >= numCal) return;
            /*iCal = 0;
        if (iCal == iCalStart) return;
        */
        depo = CalHub.Calorimeters[iCal].Calorimeter->Stats[0];
    }
    while (depo == 0);

    if (iCal < ui->cobCalorimeter->count()) ui->cobCalorimeter->setCurrentIndex(iCal);
    updateCalorimeterGui();
}

void AParticleSimWin::on_cobCalorimeter_activated(int)
{
    updateCalorimeterGui();
}

void AParticleSimWin::on_sbCalorimeterIndex_editingFinished()
{
    int iCal = ui->sbCalorimeterIndex->value();
    if (iCal >= ui->cobCalorimeter->count()) iCal = 0;
    ui->sbCalorimeterIndex->setValue(iCal);
    if (iCal < ui->cobCalorimeter->count()) ui->cobCalorimeter->setCurrentIndex(iCal); //protection: can be empty
    updateCalorimeterGui();
}



void AParticleSimWin::on_pbShowCalorimeterSettings_clicked()
{
    const int iCal = ui->cobCalorimeter->currentIndex();

    int numCal = CalHub.countCalorimeters();
    if (iCal < 0 || iCal >= numCal) return;

    AGeoObject * obj = CalHub.Calorimeters[iCal].GeoObj;
    if (obj) emit requestShowGeoObjectDelegate(obj->Name, true);
}

void AParticleSimWin::clearResults()
{
    clearResultsTracking();
    clearResultsMonitors();
    clearResultsCalorimeters();
}

void AParticleSimWin::clearResultsTracking()
{
    AGeometryHub::getInstance().GeoManager->ClearTracks();
}

void AParticleSimWin::clearResultsMonitors()
{
    MonitorHub.clearData(AMonitorHub::Particle);
}

void AParticleSimWin::clearResultsCalorimeters()
{
    CalHub.clearData();
}

void AParticleSimWin::clearResultsGui()
{
    clearResultsGuiTrackig();
    clearResultsGuiMonitors();
    clearResultsGuiCalorimeters();
}

void AParticleSimWin::clearResultsGuiTrackig()
{
    emit requestShowGeometry(false, true, true);
    ui->trwEventView->clear();
    ui->ptePTHist->clear();
}

void AParticleSimWin::clearResultsGuiMonitors()
{
    updateMonitorGui();
}

void AParticleSimWin::clearResultsGuiCalorimeters()
{
    updateCalorimeterGui();
}

void AParticleSimWin::on_pbSaveParticleSource_clicked()
{
    const int iSource = ui->lwDefinedParticleSources->currentRow();
    if (iSource == -1)
    {
        guitools::message("Select a source to export", this);
        return;
    }
    ASourceGeneratorSettings & SourceGenSettings = SimSet.SourceGenSettings;
    const int numSources = SourceGenSettings.getNumSources();
    if (iSource >= numSources)
    {
        guitools::message("Error - bad source index!", this);
        return;
    }

    const AParticleSourceRecord & source = SimSet.SourceGenSettings.SourceData[iSource];

    QString fileName = guitools::dialogSaveFile(this, "Save source " + QString(source.Name.data()) + " to file", "Json files (*.json)");
    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".json")) fileName += ".json";
    QJsonObject json;
    source.writeToJson(json);
    bool ok = jstools::saveJsonToFile(json, fileName);
    if (!ok) guitools::message("Failed to open file for saving: " + fileName);
}

void AParticleSimWin::on_pbLoadParticleSource_clicked()
{
    QString fileName = guitools::dialogLoadFile(this, "Import particle source file", "Json files (*.json);;All files (*)");
    if (fileName.isEmpty()) return;

    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, fileName);
    if (!ok)
    {
        guitools::message("Cannot open file: " + fileName, this);
        return;
    }

    AParticleSourceRecord s;
    ok = s.readFromJson(json);
    if (!ok)
    {
        guitools::message("Error in imported source", this);
        return;
    }

    QString err(s.check().data());
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    SimSet.SourceGenSettings.SourceData.push_back(s);
    updateSourceList();
    ui->lwDefinedParticleSources->setCurrentRow(SimSet.SourceGenSettings.getNumSources() - 1);
}

void AParticleSimWin::on_pbAbort_clicked()
{
    SimManager.Generator_Sources->AbortRequested = true;
    SimManager.Generator_File->AbortRequested = true;
    SimManager.abort();
}

void AParticleSimWin::on_ledEventsPerThread_editingFinished()
{
    bool ok;
    double numEv = ui->ledEventsPerThread->text().toDouble(&ok);
    if (!ok || numEv < 1.0)
    {
        ui->ledEventsPerThread->setText("1");
        guitools::message("Number of events per chunk should be at least one", this);
    }
}

void AParticleSimWin::on_cobCaloShowType_currentIndexChanged(int)
{
    updateShowCalorimeterGui();
}
void AParticleSimWin::on_cobCaloAxes_currentIndexChanged(int)
{
    updateShowCalorimeterGui();
}
void AParticleSimWin::on_cbCaloAverage_clicked()
{
    updateShowCalorimeterGui();
}

void AParticleSimWin::updateShowCalorimeterGui()
{
    const int iCal = ui->cobCalorimeter->currentIndex();
    int numCal = CalHub.countCalorimeters(); if (iCal < 0 || iCal >= numCal) return;
    const ACalorimeterProperties & p = CalHub.Calorimeters[iCal].Calorimeter->Properties;
    bool bEnergy = (p.DataType == ACalorimeterProperties::Energy);
    QString buttonText = (bEnergy ? "Show deposited energy" : "Show dose");
    ui->pbCaloShow->setText(buttonText);

    bool bProjection = (ui->cobCaloShowType->currentIndex() == 0);

    if (!bEnergy && bProjection)
    {
        ui->cobCaloShowType->setCurrentIndex(1);
        return; // comes back on update
    }

    ui->cobCaloShowType->setVisible(bEnergy);

    bool b1D = (ui->cobCaloAxes->currentIndex() < 3);
    bool b2D = (ui->cobCaloAxes->currentIndex() > 2 && ui->cobCaloAxes->currentIndex() != 6);
    bool b3D = (ui->cobCaloAxes->currentIndex() == 6);
    bool bAverage = ui->cbCaloAverage->isChecked();

    ui->cbCaloAverage->setVisible(!bProjection && !b3D);
    ui->cbCalorimeterSwapAxes->setVisible(b2D);

    if (bProjection)
    {
        ui->frCaloX->setVisible(false);
        ui->frCaloY->setVisible(false);
        ui->frCaloZ->setVisible(false);
    }
    else switch (ui->cobCaloAxes->currentIndex())
    {
    case 0: // X
        ui->frCaloX->setVisible(false);
        ui->frCaloY->setVisible(true);
        ui->frCaloZ->setVisible(true);
        break;
    case 1: // Y
        ui->frCaloX->setVisible(true);
        ui->frCaloY->setVisible(false);
        ui->frCaloZ->setVisible(true);
        break;
    case 2: // Z
        ui->frCaloX->setVisible(true);
        ui->frCaloY->setVisible(true);
        ui->frCaloZ->setVisible(false);
        break;
    case 3: // XY
        ui->frCaloX->setVisible(false);
        ui->frCaloY->setVisible(false);
        ui->frCaloZ->setVisible(true);
        break;
    case 4: // XZ
        ui->frCaloX->setVisible(false);
        ui->frCaloY->setVisible(true);
        ui->frCaloZ->setVisible(false);
        break;
    case 5: // YZ
        ui->frCaloX->setVisible(true);
        ui->frCaloY->setVisible(false);
        ui->frCaloZ->setVisible(false);
        break;
    case 6: // XYZ
        ui->frCaloX->setVisible(false);
        ui->frCaloY->setVisible(false);
        ui->frCaloZ->setVisible(false);
        break;
    }

    if (!bProjection && !bAverage)
    {
        ui->labCaloX0->setText("iX");
        ui->labCaloY0->setText("iY");
        ui->labCaloZ0->setText("iZ");
        ui->labCaloX1->setVisible(false);
        ui->labCaloY1->setVisible(false);
        ui->labCaloZ1->setVisible(false);
        ui->sbCaloX1->setVisible(false);
        ui->sbCaloY1->setVisible(false);
        ui->sbCaloZ1->setVisible(false);
    }
    else
    {
        ui->labCaloX0->setText("iXfrom");
        ui->labCaloY0->setText("iYfrom");
        ui->labCaloZ0->setText("iZfrom");
        ui->labCaloX1->setText("iXto");
        ui->labCaloY1->setText("iYto");
        ui->labCaloZ1->setText("iZto");
        ui->labCaloX1->setVisible(true);
        ui->labCaloY1->setVisible(true);
        ui->labCaloZ1->setVisible(true);
        ui->sbCaloX1->setVisible(true);
        ui->sbCaloY1->setVisible(true);
        ui->sbCaloZ1->setVisible(true);
    }

    int maxX = p.Bins[0] - 1; ui->sbCaloX0->setMaximum(maxX); ui->sbCaloX1->setMaximum(maxX);
    int maxY = p.Bins[1] - 1; ui->sbCaloY0->setMaximum(maxX); ui->sbCaloY1->setMaximum(maxX);
    int maxZ = p.Bins[2] - 1; ui->sbCaloZ0->setMaximum(maxX); ui->sbCaloZ1->setMaximum(maxX);

    if (ui->sbCaloX0->value() == 0 && ui->sbCaloX1->value() == 0)
    {
        ui->sbCaloX0->setValue(maxX/2);
        ui->sbCaloX1->setValue(maxX/2);
    }
    if (ui->sbCaloY0->value() == 0 && ui->sbCaloY1->value() == 0)
    {
        ui->sbCaloY0->setValue(maxY/2);
        ui->sbCaloY1->setValue(maxY/2);
    }
    if (ui->sbCaloZ0->value() == 0 && ui->sbCaloZ1->value() == 0)
    {
        ui->sbCaloZ0->setValue(maxZ/2);
        ui->sbCaloZ1->setValue(maxZ/2);
    }

    updateCaloRange();
}

void AParticleSimWin::on_pbCaloShow_clicked()
{
    const int iCal = ui->cobCalorimeter->currentIndex();

    int numCal = CalHub.countCalorimeters();
    if (iCal < 0 || iCal >= numCal) return;

    ATH3D * Data = CalHub.Calorimeters[iCal].Calorimeter->DataHistogram;
    if (!Data) return;

    const ACalorimeterProperties & p = CalHub.Calorimeters[iCal].Calorimeter->Properties;

    bool bProjection = (ui->cobCaloShowType->currentIndex() == 0);
    int  axisIndex = ui->cobCaloAxes->currentIndex();
    bool b1D = (axisIndex >= 0 && axisIndex < 3);
    bool b2D = (axisIndex > 2 && axisIndex < 6);
    bool b3D = (axisIndex == 6);
    bool bAverage = ui->cbCaloAverage->isChecked();
    bool bSwapAxes = ui->cbCalorimeterSwapAxes->isChecked();

    if (bProjection)
    {
        if (b1D)
        {
            TString opt(ui->cobCaloAxes->currentText().toLatin1().data());
            opt.ToLower();

            TH1D * h = (TH1D*)(Data->Project3D(opt)->Clone());
            if (!h) return;

            h->SetTitle(TString(CalHub.Calorimeters[iCal].Name.toLatin1().data()) + "-" + opt);
            h->GetXaxis()->SetTitle( TString(opt) + ", mm" );
            h->GetYaxis()->SetTitle("Deposited energy, MeV");

            emit requestDraw(h, "hist", true, true);
        }
        else if (b2D)
        {
            TString opt(ui->cobCaloAxes->currentText().toLatin1().data());
            opt.ToLower();
            if (bSwapAxes) std::swap(opt[0], opt[1]);

            TH2D * h = (TH2D*)(Data->Project3D(opt)->Clone());
            if (!h) return;

            h->SetTitle(TString(CalHub.Calorimeters[iCal].Name.toLatin1().data()) + "-" + opt);
            h->GetXaxis()->SetTitle(TString(opt[1]) + ", mm");
            h->GetYaxis()->SetTitle(TString(opt[0]) + ", mm");
            h->GetZaxis()->SetTitle("Deposited energy, MeV");

            emit requestDraw(h, "colz", true, true);
        }
        else if (b3D)
        {
            TH3D * h = (TH3D*)(Data->Clone());

            h->SetTitle(TString(CalHub.Calorimeters[iCal].Name.toLatin1().data()) + "-3D");
            h->GetXaxis()->SetTitle("x, mm");
            h->GetYaxis()->SetTitle("y, mm");
            h->GetZaxis()->SetTitle("z, mm");

            emit requestDraw(h, "box2", true, true);
        }
        return;
    }

    // slices
    if (b1D)
    {
        TH1D * hist = new TH1D("", "", p.Bins[axisIndex], p.Origin[axisIndex], p.Origin[axisIndex]+p.Step[axisIndex]*p.Bins[axisIndex]);

        TString axisTitle;
        switch (axisIndex)
        {
        case 0:
        {
            axisTitle = "x";
            int fromYIndex = ui->sbCaloY0->value(); int toYIndex = (bAverage ? ui->sbCaloY1->value() : fromYIndex+1);
            if (fromYIndex > toYIndex) std::swap(fromYIndex, toYIndex);
            int fromZIndex = ui->sbCaloZ0->value(); int toZIndex = (bAverage ? ui->sbCaloZ1->value() : fromZIndex+1);
            if (fromZIndex > toZIndex) std::swap(fromZIndex, toZIndex);
            double factor = (1.0+toYIndex-fromYIndex) * (1.0+toZIndex-fromZIndex);
            for (int iBin = 0; iBin < p.Bins[axisIndex]; iBin++)
                for (int iY = fromYIndex; iY < toYIndex; iY++)
                    for (int iZ = fromZIndex; iZ < toZIndex; iZ++)
                        hist->AddBinContent(iBin+1, Data->GetBinContent(iBin+1, iY+1, iZ+1)/factor);
            break;
        }
        case 1:
        {
            axisTitle = "y";
            int fromXIndex = ui->sbCaloX0->value(); int toXIndex = (bAverage ? ui->sbCaloX1->value() : fromXIndex+1);
            if (fromXIndex > toXIndex) std::swap(fromXIndex, toXIndex);
            int fromZIndex = ui->sbCaloZ0->value(); int toZIndex = (bAverage ? ui->sbCaloZ1->value() : fromZIndex+1);
            if (fromZIndex > toZIndex) std::swap(fromZIndex, toZIndex);
            double factor = (1.0+toXIndex-fromXIndex) * (1.0+toZIndex-fromZIndex);
            for (int iBin = 0; iBin < p.Bins[axisIndex]; iBin++)
                for (int iX = fromXIndex; iX < toXIndex; iX++)
                    for (int iZ = fromZIndex; iZ < toZIndex; iZ++)
                        hist->AddBinContent(iBin+1, Data->GetBinContent(iX+1, iBin+1, iZ+1)/factor);
            break;
        }
        case 2:
        {
            axisTitle = "z";
            int fromXIndex = ui->sbCaloX0->value(); int toXIndex = (bAverage ? ui->sbCaloX1->value() : fromXIndex+1);
            if (fromXIndex > toXIndex) std::swap(fromXIndex, toXIndex);
            int fromYIndex = ui->sbCaloY0->value(); int toYIndex = (bAverage ? ui->sbCaloY1->value() : fromYIndex+1);
            if (fromYIndex > toYIndex) std::swap(fromYIndex, toYIndex);
            double factor = (1.0+toYIndex-fromYIndex) * (1.0+toXIndex-fromXIndex);
            for (int iBin = 0; iBin < p.Bins[axisIndex]; iBin++)
                for (int iX = fromXIndex; iX < toXIndex; iX++)
                    for (int iY = fromYIndex; iY < toYIndex; iY++)
                        hist->AddBinContent(iBin+1, Data->GetBinContent(iX+1, iY+1, iBin+1)/factor);
            break;
        }
        }

        hist->SetTitle(TString(CalHub.Calorimeters[iCal].Name.toLatin1().data()) + "-" + axisTitle);
        hist->GetXaxis()->SetTitle(axisTitle + ", mm");
        hist->GetYaxis()->SetTitle("Dose, Gy");

        emit requestDraw(hist, "hist", true, true);
    }
    else if (b2D)
    {
        TH2D * hist = nullptr;
        TString verAxisTitle, horAxisTitle;

        switch (axisIndex)
        {
        case 3: // xy
            {
            verAxisTitle = "x";
            horAxisTitle = "y";
            if (bSwapAxes)
            {
                hist = new TH2D("", "",
                                p.Bins[0], p.Origin[0], p.Origin[0]+p.Step[0]*p.Bins[0],
                                p.Bins[1], p.Origin[1], p.Origin[1]+p.Step[1]*p.Bins[1]);
                std::swap(verAxisTitle, horAxisTitle);
            }
            else
            {
                hist = new TH2D("", "",
                                p.Bins[1], p.Origin[1], p.Origin[1]+p.Step[1]*p.Bins[1],
                                p.Bins[0], p.Origin[0], p.Origin[0]+p.Step[0]*p.Bins[0]);
            }
            int fromZIndex = ui->sbCaloZ0->value(); int toZIndex = (bAverage ? ui->sbCaloZ1->value() : fromZIndex+1);
            if (fromZIndex > toZIndex) std::swap(fromZIndex, toZIndex);
            double factor = 1.0 + toZIndex - fromZIndex;
            for (int iBinX = 0; iBinX < p.Bins[0]; iBinX++)
                for (int iBinY = 0; iBinY < p.Bins[1]; iBinY++)
                    for (int iZ = fromZIndex; iZ < toZIndex; iZ++)
                    {
                        int bin = (bSwapAxes ? hist->GetBin(iBinX+1,iBinY+1) : hist->GetBin(iBinY+1, iBinX+1));
                        hist->AddBinContent(bin, Data->GetBinContent(iBinX+1, iBinY+1, iZ+1)/factor);
                    }
            break;
            }
        case 4: // xz
            {
            verAxisTitle = "x";
            horAxisTitle = "z";
            if (bSwapAxes)
            {
                hist = new TH2D("", "",
                                p.Bins[0], p.Origin[0], p.Origin[0]+p.Step[0]*p.Bins[0],
                                p.Bins[2], p.Origin[2], p.Origin[2]+p.Step[2]*p.Bins[2]);
                std::swap(verAxisTitle, horAxisTitle);
            }
            else
            {
                hist = new TH2D("", "",
                                p.Bins[2], p.Origin[2], p.Origin[2]+p.Step[2]*p.Bins[2],
                                p.Bins[0], p.Origin[0], p.Origin[0]+p.Step[0]*p.Bins[0]);
            }

            int fromYIndex = ui->sbCaloY0->value(); int toYIndex = (bAverage ? ui->sbCaloY1->value() : fromYIndex+1);
            if (fromYIndex > toYIndex) std::swap(fromYIndex, toYIndex);
            double factor = 1.0 + toYIndex - fromYIndex;
            for (int iBinX = 0; iBinX < p.Bins[0]; iBinX++)
                for (int iY = fromYIndex; iY < toYIndex; iY++)
                    for (int iBinZ = 0; iBinZ < p.Bins[2]; iBinZ++)
                    {
                        int bin = (bSwapAxes ? hist->GetBin(iBinX+1, iBinZ+1) : hist->GetBin(iBinZ+1, iBinX+1));
                        hist->AddBinContent(bin, Data->GetBinContent(iBinX+1, iY+1, iBinZ+1)/factor);
                    }
            break;
            }
        case 5: // yz
            {
            verAxisTitle = "y";
            horAxisTitle = "z";
            if (bSwapAxes)
            {
                hist = new TH2D("", "",
                                p.Bins[1], p.Origin[1], p.Origin[1]+p.Step[1]*p.Bins[1],
                                p.Bins[2], p.Origin[2], p.Origin[2]+p.Step[2]*p.Bins[2]);
                std::swap(verAxisTitle, horAxisTitle);
            }
            else
            {
                hist = new TH2D("", "",
                                p.Bins[2], p.Origin[2], p.Origin[2]+p.Step[2]*p.Bins[2],
                                p.Bins[1], p.Origin[1], p.Origin[1]+p.Step[1]*p.Bins[1]);
            }
            int fromXIndex = ui->sbCaloX0->value(); int toXIndex = (bAverage ? ui->sbCaloX1->value() : fromXIndex+1);
            if (fromXIndex > toXIndex) std::swap(fromXIndex, toXIndex);
            double factor = 1.0 + toXIndex - fromXIndex;
            for (int iX = fromXIndex; iX < toXIndex; iX++)
                for (int iBinY = 0; iBinY < p.Bins[1]; iBinY++)
                    for (int iBinZ = 0; iBinZ < p.Bins[2]; iBinZ++)
                    {
                        int bin = (bSwapAxes ? hist->GetBin(iBinY+1, iBinZ+1) : hist->GetBin(iBinZ+1, iBinY+1));
                        hist->AddBinContent(bin, Data->GetBinContent(iX+1, iBinY+1, iBinZ+1)/factor);
                    }
            break;
            }
        }

        if (!hist) return;
        hist->SetTitle(TString(CalHub.Calorimeters[iCal].Name.toLatin1().data()) + "-" + verAxisTitle + horAxisTitle);
        hist->GetXaxis()->SetTitle(horAxisTitle + ", mm");
        hist->GetYaxis()->SetTitle(verAxisTitle + ", mm");
        hist->GetZaxis()->SetTitle("Dose, Gy");
        hist->SetEntries(Data->GetEntries());

        emit requestDraw(hist, "colz", true, true);
    }
    else if (b3D)
    {
        TH3D * h = (TH3D*)(Data->Clone());

        h->SetTitle(TString(CalHub.Calorimeters[iCal].Name.toLatin1().data()) + "-Dose-3D");
        h->GetXaxis()->SetTitle("x, mm");
        h->GetYaxis()->SetTitle("y, mm");
        h->GetZaxis()->SetTitle("z, mm");

        emit requestDraw(h, "box2", true, true);
    }
}

void AParticleSimWin::on_pbUpdateCaloRange_clicked()
{
    updateCaloRange();
}

void AParticleSimWin::updateCaloRange()
{
    const int iCal = ui->cobCalorimeter->currentIndex();

    int numCal = CalHub.countCalorimeters();
    if (iCal < 0 || iCal >= numCal)
    {
        ui->labCaloRangeX->clear();
        ui->labCaloRangeY->clear();
        ui->labCaloRangeZ->clear();
        return;
    }

    const ACalorimeterProperties & p = CalHub.Calorimeters[iCal].Calorimeter->Properties;

    bool bProjection = (ui->cobCaloShowType->currentIndex() == 0);
    if (bProjection) return;

    bool bAverage = ui->cbCaloAverage->isChecked();

    int fromXIndex = ui->sbCaloX0->value(); int toXIndex = (bAverage ? ui->sbCaloX1->value() : fromXIndex+1);
    if (fromXIndex > toXIndex) std::swap(fromXIndex, toXIndex);
    int fromYIndex = ui->sbCaloY0->value(); int toYIndex = (bAverage ? ui->sbCaloY1->value() : fromYIndex+1);
    if (fromYIndex > toYIndex) std::swap(fromYIndex, toYIndex);
    int fromZIndex = ui->sbCaloZ0->value(); int toZIndex = (bAverage ? ui->sbCaloZ1->value() : fromZIndex+1);
    if (fromZIndex > toZIndex) std::swap(fromZIndex, toZIndex);

    double rx0 = p.Origin[0] + p.Step[0] * fromXIndex;
    double rx1 = p.Origin[0] + p.Step[0] * toXIndex;
    ui->labCaloRangeX->setText(QString("  (from %0 to %1 mm)").arg(rx0, 5).arg(rx1, 5));
    double ry0 = p.Origin[1] + p.Step[1] * fromYIndex;
    double ry1 = p.Origin[1] + p.Step[1] * toYIndex;
    ui->labCaloRangeY->setText(QString("  (from %0 to %1 mm)").arg(ry0, 5).arg(ry1, 5));
    double rz0 = p.Origin[2] + p.Step[2] * fromZIndex;
    double rz1 = p.Origin[2] + p.Step[2] * toZIndex;
    ui->labCaloRangeZ->setText(QString("  (from %0 to %1 mm)").arg(rz0, 5).arg(rz1, 5));
}

#include <QDesktopServices>
void AParticleSimWin::on_pbChooseWorkingDirectory_customContextMenuRequested(const QPoint &)
{
    if (ui->leWorkingDirectory->text().isEmpty()) return;
    QDesktopServices::openUrl( QUrl::fromLocalFile(ui->leWorkingDirectory->text()) );
}

void AParticleSimWin::on_cbIncludeScintillators_clicked(bool checked)
{
    G4SimSet.AddScintillatorsToSensitiveVolumes = checked;
}

void AParticleSimWin::updateRangeWarning()
{
    bool bMultithread = (ui->sbNumThreadsStatistics->value() > 0);

    double HorFrom = ui->ledPTHistFromX->text().toDouble();
    double HorTo = ui->ledPTHistToX->text().toDouble();
    ui->labRangeWarningHor->setVisible(bMultithread && (HorFrom >= HorTo));

    double VertFrom = ui->ledPTHistFromY->text().toDouble();
    double VertTo = ui->ledPTHistToY->text().toDouble();
    ui->labRangeWarningVert->setVisible(bMultithread && (VertFrom >= VertTo));
}

void AParticleSimWin::on_sbNumThreadsStatistics_valueChanged(int)
{
    updateRangeWarning();
}
void AParticleSimWin::on_ledPTHistFromX_editingFinished()
{
    updateRangeWarning();
}
void AParticleSimWin::on_ledPTHistToX_editingFinished()
{
    updateRangeWarning();
}
void AParticleSimWin::on_ledPTHistFromY_editingFinished()
{
    updateRangeWarning();
}
void AParticleSimWin::on_ledPTHistToY_editingFinished()
{
    updateRangeWarning();
}

void AParticleSimWin::on_pbCaloShowDepoOverEvent_clicked()
{
    const int iCal = ui->cobCalorimeter->currentIndex();

    int numCal = CalHub.countCalorimeters();
    if (iCal < 0 || iCal >= numCal) return;

    ATH1D * Data = CalHub.Calorimeters[iCal].Calorimeter->EventDepoData;
    if (!Data) return;

    Data->GetXaxis()->SetTitle("Deposited energy over event, MeV");

    emit requestDraw(Data, "hist", false, true);
}

// -----------------

void AParticleSimWin::on_pbChooseDepositionFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select a file with energy deposition data", SimSet.RunSet.OutputDirectory.data());
    if (fileName.isEmpty()) return;

    ui->leDepositionFileName->setText(fileName);
}

#include "adepositionfilehandler.h"
void AParticleSimWin::on_pbAnalyzeDepositionFile_clicked()
{
    APhotonDepoSettings DepoSet;
    DepoSet.FileName = ui->leDepositionFileName->text();
    if (!DepoSet.FileName.contains('/'))
        DepoSet.FileName = ui->leWorkingDirectory->text() + '/' + DepoSet.FileName;

    ADepositionFileHandler fh(DepoSet);

    //if (!DepoSet.isValidated())
    {
        fh.determineFormat();

        if (DepoSet.FileFormat == APhotonDepoSettings::Invalid)
        {
            guitools::message("Cannot open file!", this);
            return;
        }
        if (DepoSet.FileFormat == APhotonDepoSettings::Undefined)
        {
            guitools::message("Unknown format of the depo file!", this);
            return;
        }
    }

    AErrorHub::clear();

    fh.collectStatistics();
    if (DepoSet.NumEvents == -1)
    {
        guitools::message("Deposition file is invalid", this);
        DepoSet.FileFormat = APhotonDepoSettings::Invalid;
    }
    else
    {
        QString txt = fh.formReportString();
        guitools::message1(txt, "Deposition file info", this);
    }
}

void AParticleSimWin::on_pbHelpOnDepositionDataFormat_clicked()
{
    QString txt = ""
                  "ASCII file format\n"
                  "-----------------\n"
                  "#Event_index\n"
                  "ParticleName indexMat Energy[keV] X[mm] Y[mm] Z[mm] Time[ns] indexVolume\n"
                  "\n"
                  "Data can be loaded using the following script:\n"
                  "fn = \"/pathToFile/Deposition.dat\"\n"
                  "data = core.load3DArray(fn, '#', ['s', 'i', 'd',   'd','d','d',  'd', 'i'], 0, 1e10, true)\n"
                  "\n"
                  "data is a 3D array: data[eventIndex][depositionIndex][depositionData]\n"
                  "For example, data[0][0][2] gives the deposition energy in the first deposition of the first event.\n"
                  "\n"
                  "\n"
                  "Binary file format\n"
                  "-----------------\n"
                  "Event record:\n"
                  "0xee EventIndex(int)\n"
                  "Data record:\n"
                  "ParticleName(0-terminated) indexMat(int) Energy[keV](double) X[mm](double) Y[mm](double) Z[mm](double) Time[ns](double) indexVolume(int)\n"
                  "\n"
                  "Data can be loaded using the following script:\n"
                  "fn = \"/pathToFile/Deposition.dat\"\n"
                  "data = core.load3DBinaryArray(fn, 0xff, ['s', 'i', 'd',   'd','d','d',  'd', 'i'], 0xee, ['i'], 0, 1e10, true)\n"
                  "";

    guitools::message1(txt, "Info for deposition data", this);
}
