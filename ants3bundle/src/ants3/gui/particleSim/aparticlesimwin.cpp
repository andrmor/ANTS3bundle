#include "aparticlesimwin.h"
#include "ui_aparticlesimwin.h"
#include "aparticlesimhub.h"
#include "aparticlesimsettings.h"
#include "ag4simulationsettings.h"
#include "aparticlesimmanager.h"
#include "guitools.h"
#include "aerrorhub.h"

#include <QListWidget>
#include <QDebug>
#include <QCheckBox>
#include <QLineEdit>

AParticleSimWin::AParticleSimWin(QWidget *parent) :
    QMainWindow(parent),
    SimSet(AParticleSimHub::getInstance().Settings),
    G4SimSet(SimSet.G4Set),
    SimManager(AParticleSimManager::getInstance()),
    ui(new Ui::AParticleSimWin)
{
    ui->setupUi(this);

    ui->frEventFilters->setVisible(false);

    on_cobPTHistVolRequestWhat_currentIndexChanged(ui->cobPTHistVolRequestWhat->currentIndex());
}

AParticleSimWin::~AParticleSimWin()
{
    delete ui;
}

void AParticleSimWin::updateGui()
{
    updateG4Gui();
    updateSimGui();
}

void AParticleSimWin::updateSimGui()
{
    ui->sbEvents->setValue(SimSet.Events);

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
}

void AParticleSimWin::updateG4Gui()
{
    ui->lePhysicsList->setText(G4SimSet.PhysicsList.data());
    ui->cobRefPhysLists->setCurrentIndex(-1);

    ui->pteCommands->clear();
    for (const auto & s : G4SimSet.Commands)
        ui->pteCommands->appendPlainText(s.data());

    ui->pteSensitiveVolumes->clear();
    for (const auto & s : G4SimSet.SensitiveVolumes)
        ui->pteSensitiveVolumes->appendPlainText(s.data());

    ui->pteStepLimits->clear();
    for (const auto & it : G4SimSet.StepLimits)
        ui->pteStepLimits->appendPlainText( QString("%1 %2").arg(it.first.data(), it.second) );

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
    const QString t = ui->pteCommands->document()->toPlainText();
    const QStringList sl = t.split('\n', Qt::SkipEmptyParts);
    G4SimSet.Commands.clear();
    for (auto & s : sl) G4SimSet.Commands.push_back(s.toLatin1().data());
}
void AParticleSimWin::on_pteSensitiveVolumes_textChanged()
{
    const QRegularExpression rx = QRegularExpression("(\\ |\\,|\\n|\\t)"); //separators: ' ' or ',' or 'n' or '\t'
    QString t = ui->pteSensitiveVolumes->document()->toPlainText();
    const QStringList sl = t.split(rx, Qt::SkipEmptyParts);
    G4SimSet.SensitiveVolumes.clear();
    for (auto & s : sl) G4SimSet.SensitiveVolumes.push_back(s.toLatin1().data());
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
        G4SimSet.StepLimits[vol.toLatin1().data()] = step;
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
    ASourceGeneratorSettings & SourceGenSettings = SimSet.SourceGenSettings;
    const int numSources = SourceGenSettings.getNumSources();
    if (isource >= numSources)
    {
        guitools::message("Error - bad source index!", this);
        return;
    }

    AParticleSourceDialog ParticleSourceDialog(SourceGenSettings.SourceData.at(isource), this);

    int res = ParticleSourceDialog.exec(); // !!!*** check: if detector is rebuild (this->readSimSettingsFromJson() is triggered), ParticleSourceDialog is signal-blocked and rejected
    if (res == QDialog::Rejected) return;

    AParticleSourceRecord & ps = ParticleSourceDialog.getResult();
    SourceGenSettings.replace(isource, ps);

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
    AParticleSourceRecord s;
    s.Particles.push_back(AGunParticle());
    SimSet.SourceGenSettings.SourceData.push_back(s);

//    on_pbUpdateSimConfig_clicked();
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

//    on_pbUpdateSimConfig_clicked();
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
        l->addWidget(new QLabel( QString("%1 particle%2").arg(pr.Particles.size()).arg( pr.Particles.size()>1 ? "s" : "" ) ) );
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
//                emit this->RequestUpdateSimConfig();  // !!!*** update gui!
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

void AParticleSimWin::on_lwDefinedParticleSources_itemDoubleClicked(QListWidgetItem *)
{
    on_pbEditParticleSource_clicked();
}

#include "TGeoManager.h"
#include "afileparticlegenerator.h"
void AParticleSimWin::on_pbGunTest_clicked()
{
//    WindowNavigator->BusyOn();   // -->

    gGeoManager->ClearTracks();
    emit requestShowGeometry(true, true, false);

    if (ui->cobParticleGenerationMode->currentIndex() == 0)
    {
        if (ui->pbGunShowSource->isChecked())
            for (int i = 0; i < SimSet.SourceGenSettings.getNumSources(); i++)
                drawSource(i);
    }

    AParticleGun * pg = nullptr;
    switch (ui->cobParticleGenerationMode->currentIndex())
    {
    case 0:
        pg = SimManager.Generator_Sources;
        break;
    case 1:
        pg = SimManager.Generator_File;
        SimManager.Generator_File->initWithCheck(false);
        break;
    case 2:
//        pg = SimManager->ScriptParticleGenerator;
        break;
    default:
        guitools::message("This generation mode is not implemented!", this);
//        WindowNavigator->BusyOff(); // <--
        return;
    }

    ui->pbAbort->setEnabled(true);
    QFont font = ui->pbAbort->font();
    font.setBold(true);
    ui->pbAbort->setFont(font);

    testParticleGun(pg, ui->sbGunTestEvents->value()); //script generator is aborted on click of the stop button!

    ui->pbAbort->setEnabled(false);
    ui->pbAbort->setText("stop");
    font.setBold(false);
    ui->pbAbort->setFont(font);


    emit requestShowTracks();
//    emit requestShowMarkers();

//    WindowNavigator->BusyOff();  // <--
}

#include "TVirtualGeoTrack.h"
#include "ageometryhub.h"
#include "aparticlerecord.h"
#include "TVector3.h"
void AParticleSimWin::drawSource(int iSource)
{
    //check iSource is correct  !!!***
    const AParticleSourceRecord & p = SimSet.SourceGenSettings.SourceData.at(iSource);

    int index = p.Shape;
    double X0 = p.X0;
    double Y0 = p.Y0;
    double Z0 = p.Z0;
    double Phi = p.Phi*3.1415926535/180.0;
    double Theta = p.Theta*3.1415926535/180.0;
    double Psi = p.Psi*3.1415926535/180.0;
    double size1 = p.Size1;
    double size2 = p.Size2;
    double size3 = p.Size3;
    double CollPhi = p.CollPhi*3.1415926535/180.0;
    double CollTheta = p.CollTheta*3.1415926535/180.0;
    double Spread = p.Spread*3.1415926535/180.0;

    //calculating unit vector along 1D direction
    TVector3 VV(sin(Theta)*sin(Phi), sin(Theta)*cos(Phi), cos(Theta));
    //qDebug()<<VV[0]<<VV[1]<<VV[2];

    TVector3 V[3];
    V[0].SetXYZ(size1, 0, 0);
    V[1].SetXYZ(0, size2, 0);
    V[2].SetXYZ(0, 0, size3);
    for (int i=0; i<3; i++)
    {
        V[i].RotateX(Phi);
        V[i].RotateY(Theta);
        V[i].RotateZ(Psi);
    }
    switch (index)
    {
    case 0:
    {
        gGeoManager->SetCurrentPoint(X0,Y0,Z0);
        gGeoManager->DrawCurrentPoint(9);
//        GeometryWindow->ClearGeoMarkers();
//        GeoMarkerClass* marks = new GeoMarkerClass("Source", 3, 10, kBlack);
//        marks->SetNextPoint(X0, Y0, Z0);
//        GeometryWindow->GeoMarkers.append(marks);
//        GeoMarkerClass* marks1 = new GeoMarkerClass("Source", 4, 3, kBlack);
//        marks1->SetNextPoint(X0, Y0, Z0);
//        GeometryWindow->GeoMarkers.append(marks1);
//        GeometryWindow->ShowGeometry(false);
        break;
    }
    case (1):
    { //linear source
        Int_t track_index = gGeoManager->AddTrack(1,22);
        TVirtualGeoTrack *track = gGeoManager->GetTrack(track_index);
        track->AddPoint(X0+VV[0]*size1, Y0+VV[1]*size1, Z0+VV[2]*size1, 0);
        track->AddPoint(X0-VV[0]*size1, Y0-VV[1]*size1, Z0-VV[2]*size1, 0);
        track->SetLineWidth(3);
        track->SetLineColor(9);
        break;
    }
    case (2):
    { //area source - square
        Int_t track_index = gGeoManager->AddTrack(1,22);
        TVirtualGeoTrack *track = gGeoManager->GetTrack(track_index);
        track->AddPoint(X0-V[0][0]-V[1][0], Y0-V[0][1]-V[1][1], Z0-V[0][2]-V[1][2], 0);
        track->AddPoint(X0+V[0][0]-V[1][0], Y0+V[0][1]-V[1][1], Z0+V[0][2]-V[1][2], 0);
        track->AddPoint(X0+V[0][0]+V[1][0], Y0+V[0][1]+V[1][1], Z0+V[0][2]+V[1][2], 0);
        track->AddPoint(X0-V[0][0]+V[1][0], Y0-V[0][1]+V[1][1], Z0-V[0][2]+V[1][2], 0);
        track->AddPoint(X0-V[0][0]-V[1][0], Y0-V[0][1]-V[1][1], Z0-V[0][2]-V[1][2], 0);
        track->SetLineWidth(3);
        track->SetLineColor(9);
        break;
    }
    case (3):
    { //area source - round
        Int_t track_index = gGeoManager->AddTrack(1,22);
        TVirtualGeoTrack *track = gGeoManager->GetTrack(track_index);
        TVector3 Circ;
        for (int i=0; i<51; i++)
        {
            double x = size1*cos(3.1415926535/25.0*i);
            double y = size1*sin(3.1415926535/25.0*i);
            Circ.SetXYZ(x,y,0);
            Circ.RotateX(Phi);
            Circ.RotateY(Theta);
            Circ.RotateZ(Psi);
            track->AddPoint(X0+Circ[0], Y0+Circ[1], Z0+Circ[2], 0);
        }
        track->SetLineWidth(3);
        track->SetLineColor(9);
        break;
    }

    case (4):
    { //volume source - box
        for (int i=0; i<3; i++)
            for (int j=0; j<3; j++)
            {
                if (j==i) continue;
                //third k
                int k = 0;
                for (; k<2; k++)
                    if (k!=i && k!=j) break;
                for (int s=-1; s<2; s+=2)
                {
                    //  qDebug()<<"i j k shift"<<i<<j<<k<<s;
                    Int_t track_index = gGeoManager->AddTrack(1,22);
                    TVirtualGeoTrack *track = gGeoManager->GetTrack(track_index);
                    track->AddPoint(X0-V[i][0]-V[j][0]+V[k][0]*s, Y0-V[i][1]-V[j][1]+V[k][1]*s, Z0-V[i][2]-V[j][2]+V[k][2]*s, 0);
                    track->AddPoint(X0+V[i][0]-V[j][0]+V[k][0]*s, Y0+V[i][1]-V[j][1]+V[k][1]*s, Z0+V[i][2]-V[j][2]+V[k][2]*s, 0);
                    track->AddPoint(X0+V[i][0]+V[j][0]+V[k][0]*s, Y0+V[i][1]+V[j][1]+V[k][1]*s, Z0+V[i][2]+V[j][2]+V[k][2]*s, 0);
                    track->AddPoint(X0-V[i][0]+V[j][0]+V[k][0]*s, Y0-V[i][1]+V[j][1]+V[k][1]*s, Z0-V[i][2]+V[j][2]+V[k][2]*s, 0);
                    track->AddPoint(X0-V[i][0]-V[j][0]+V[k][0]*s, Y0-V[i][1]-V[j][1]+V[k][1]*s, Z0-V[i][2]-V[j][2]+V[k][2]*s, 0);
                    track->SetLineWidth(3);
                    track->SetLineColor(9);
                }
            }
        break;
    }
    case(5):
    { //volume source - cylinder
        TVector3 Circ;
        Int_t track_index = gGeoManager->AddTrack(1,22);
        TVirtualGeoTrack *track = gGeoManager->GetTrack(track_index);
        double z = size3;
        for (int i=0; i<51; i++)
        {
            double x = size1*cos(3.1415926535/25.0*i);
            double y = size1*sin(3.1415926535/25.0*i);
            Circ.SetXYZ(x,y,z);
            Circ.RotateX(Phi);
            Circ.RotateY(Theta);
            Circ.RotateZ(Psi);
            track->AddPoint(X0+Circ[0], Y0+Circ[1], Z0+Circ[2], 0);
        }
        track->SetLineWidth(3);
        track->SetLineColor(9);
        track_index = gGeoManager->AddTrack(1,22);
        track = gGeoManager->GetTrack(track_index);
        z = -z;
        for (int i=0; i<51; i++)
        {
            double x = size1*cos(3.1415926535/25.0*i);
            double y = size1*sin(3.1415926535/25.0*i);
            Circ.SetXYZ(x,y,z);
            Circ.RotateX(Phi);
            Circ.RotateY(Theta);
            Circ.RotateZ(Psi);
            track->AddPoint(X0+Circ[0], Y0+Circ[1], Z0+Circ[2], 0);
        }
        track->SetLineWidth(3);
        track->SetLineColor(9);
        break;
    }
    }

    TVector3 K(sin(CollTheta)*sin(CollPhi), sin(CollTheta)*cos(CollPhi), cos(CollTheta)); //collimation direction
    Int_t track_index = gGeoManager->AddTrack(1,22);
    TVirtualGeoTrack *track = gGeoManager->GetTrack(track_index);
    const double WorldSizeXY = AGeometryHub::getInstance().getWorldSizeXY();
    const double WorldSizeZ  = AGeometryHub::getInstance().getWorldSizeZ();
    double Klength = std::max(WorldSizeXY, WorldSizeZ)*0.5;

    track->AddPoint(X0, Y0, Z0, 0);
    track->AddPoint(X0+K[0]*Klength, Y0+K[1]*Klength, Z0+K[2]*Klength, 0);
    track->SetLineWidth(2);
    track->SetLineColor(9);

    TVector3 Knorm = K.Orthogonal();
    TVector3 K1(K);
    K1.Rotate(Spread, Knorm);
    for (int i=0; i<8; i++)  //drawing spread
    {
        Int_t track_index = gGeoManager->AddTrack(1,22);
        TVirtualGeoTrack *track = gGeoManager->GetTrack(track_index);

        track->AddPoint(X0, Y0, Z0, 0);
        track->AddPoint(X0+K1[0]*Klength, Y0+K1[1]*Klength, Z0+K1[2]*Klength, 0);
        K1.Rotate(3.1415926535/4.0, K);

        track->SetLineWidth(1);
        track->SetLineColor(9);
    }
}

void AParticleSimWin::testParticleGun(AParticleGun * Gun, int numParticles)
{
    AErrorHub::clear();

    if (!Gun)
    {
        guitools::message("Particle gun is not defined", this);
        return;
    }

    bool bOK = Gun->init();
    if (!bOK)
    {
        guitools::message( QString("Failed to initialize particle gun!\n%0").arg(AErrorHub::getError().data()), this);
        return;
    }
    Gun->setStartEvent(0);
    if (ui->cobParticleGenerationMode->currentIndex() == 1) updateFileParticleGeneratorGui();

    const double WorldSizeXY = AGeometryHub::getInstance().getWorldSizeXY();
    const double WorldSizeZ  = AGeometryHub::getInstance().getWorldSizeZ();
    double Length = std::max(WorldSizeXY, WorldSizeZ)*0.4;

    int numTracks = 0;

    auto handler = [&numTracks, Length](const AParticleRecord & particle)
    {
        if (numTracks > 10000) return;
        int track_index = gGeoManager->AddTrack(1, 22);
        TVirtualGeoTrack *track = gGeoManager->GetTrack(track_index);
        track->AddPoint(particle.r[0], particle.r[1], particle.r[2], 0);
        track->AddPoint(particle.r[0] + particle.v[0]*Length, particle.r[1] + particle.v[1]*Length, particle.r[2] + particle.v[2]*Length, 0);
        numTracks++;
    };

    for (int iRun=0; iRun<numParticles; iRun++)
    {
        bool bOK = Gun->generateEvent(handler, iRun);
        if (!bOK || numTracks > 10000) break;
    }

    /*
    double R[3], K[3];
    std::vector<AParticleRecord> GP;
    for (int iRun=0; iRun<numParticles; iRun++)
    {
        bool bOK = Gun->generateEvent(GP, iRun);
        if (bOK && numTracks < 1000)
        {
            for (const AParticleRecord & p : GP)
            {
                R[0] = p.r[0];
                R[1] = p.r[1];
                R[2] = p.r[2];

                K[0] = p.v[0];
                K[1] = p.v[1];
                K[2] = p.v[2];

                int track_index = gGeoManager->AddTrack(1, 22);
                TVirtualGeoTrack *track = gGeoManager->GetTrack(track_index);
                track->AddPoint(R[0], R[1], R[2], 0);
                track->AddPoint(R[0] + K[0]*Length, R[1] + K[1]*Length, R[2] + K[2]*Length, 0);
//                SimulationManager->TrackBuildOptions.applyToParticleTrack(track, p->Id);

//                GeoMarkerClass* marks = new GeoMarkerClass("t", 7, 1, SimulationManager->TrackBuildOptions.getParticleColor(p->Id));
//                marks->SetNextPoint(R[0], R[1], R[2]);
//                GeometryWindow->GeoMarkers.append(marks);

                ++numTracks;
                if (numTracks > 1000) break;
            }
        }

        GP.clear();

        if (!bOK) break;
    }
    */
}

void AParticleSimWin::clearResultsGui()
{
    ui->trwEventView->clear();
}

void AParticleSimWin::disableGui(bool flag)
{
    setDisabled(flag);
}

void AParticleSimWin::on_pbGunShowSource_toggled(bool checked)
{
    if (checked)
    {
        emit requestShowGeometry(true, true, true);
        for (int i = 0; i < SimSet.SourceGenSettings.getNumSources(); i++)
            drawSource(i);
        emit requestShowTracks();
    }
    else
    {
//        GeometryWindow->ClearGeoMarkers();
        gGeoManager->ClearTracks();
        emit requestShowGeometry(false, true, true);
    }
}

void AParticleSimWin::on_cobParticleGenerationMode_activated(int index)
{
    if      (index == 0) SimSet.GenerationMode = AParticleSimSettings::Sources;
    else if (index == 1) SimSet.GenerationMode = AParticleSimSettings::File;
    else                 SimSet.GenerationMode = AParticleSimSettings::Script;
}

void AParticleSimWin::on_sbEvents_editingFinished()
{
    SimSet.Events = ui->sbEvents->value();
}

#include "aparticlesimoutputdialog.h"
void AParticleSimWin::on_pbConfigureOutput_clicked()
{
    AParticleSimOutputDialog d(this);
    d.exec();
}

void AParticleSimWin::on_pbSimulate_clicked()
{
    clearResultsGui();

    disableGui(true);
    SimManager.simulate();
    disableGui(false);

    if (AErrorHub::isError()) guitools::message(AErrorHub::getError().data(), this);
    else if (ui->cbAutoLoadResults->isChecked())
    {
        ui->leWorkingDirectory->setText(SimSet.RunSet.OutputDirectory.data());

        if (SimSet.RunSet.SaveTrackingHistory)
        {
            on_pbShowTracks_clicked();

            ui->leTrackingDataFile->setText(SimSet.RunSet.FileNameTrackingHistory.data());
            EV_showTree();
        }
    }
}

void AParticleSimWin::on_pbShowTracks_clicked()
{
    const QString fileName = ui->leWorkingDirectory->text() + "/" + ui->leTrackingDataFile->text();

    const QStringList LimitTo;
    const QStringList Exclude;

    const int MaxTracks = ui->sbMaxTracks->value();

    const bool SkipPrimaries   = ui->cbSkipPrimaryTracks->isChecked();
    const bool SkipPrimNoInter = ui->cbSkipPrimaryTracksNoInteraction->isChecked();
    const bool SkipSecondaries = ui->cbSkipSecondaryTracks->isChecked();

    QString err = SimManager.buildTracks(fileName, LimitTo, Exclude,
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

    if (!dir.isEmpty()) ui->leWorkingDirectory->setText(dir);
}

void AParticleSimWin::on_pbChooseFileTrackingData_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file with tracking data", ui->leWorkingDirectory->text());
    if (fileName.isEmpty()) return;

    fileName = QFileInfo(fileName).completeBaseName() + "." + QFileInfo(fileName).suffix();
    ui->leTrackingDataFile->setText(fileName);
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

    const QString fileName = ui->leWorkingDirectory->text() + "/" + ui->leTrackingDataFile->text();
    AEventTrackingRecord * record = AEventTrackingRecord::create(); // !!!*** make persistent
    QString err = SimManager.fillTrackingRecord(fileName, ui->sbEvent->value(), record);
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }
    // !!!*** add error processing, separetely process bad event index

    const int ExpLevel = ui->sbEVexpansionLevel->value();

    for (AParticleTrackingRecord * pr : record->getPrimaryParticleRecords())
    {
        QTreeWidgetItem * item = new QTreeWidgetItem(ui->trwEventView);
        fillEvTabViewRecord(item, pr, ExpLevel);
    }
}

/*
#include "geometrywindowclass.h"
void AParticleSimWin::EV_showGeo()
{
    MW->SimulationManager->clearTracks();
    MW->GeometryWindow->ClearTracks(false);

    const int iEv = ui->sbEvent->value();
    if (iEv < 0 || iEv >= MW->EventsDataHub->countEvents()) return;

    if (ui->cbEVtracks->isChecked()) MW->GeometryWindow->ShowEvent_Particles(iEv, !ui->cbEVsupressSec->isChecked());
    if (ui->cbEVpmSig->isChecked())  MW->GeometryWindow->ShowPMsignals(MW->EventsDataHub->Events.at(iEv), false);

    MW->GeometryWindow->DrawTracks();
}
*/

/*
int AParticleSimWin::findEventWithFilters(int currentEv, bool bUp)
{
    std::vector<AEventTrackingRecord *> & TH = MW->SimulationManager->TrackingHistory;
    if (TH.empty()) return -1;
    if (currentEv == 0 && !bUp) return -1;
    if (currentEv >= (int)TH.size() && bUp) return -1;

    const QRegularExpression rx = QRegularExpression("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'

    bool bLimProc = ui->cbEVlimToProc->isChecked();
    bool bLimProc_prim = ui->cbEVlimitToProcPrim->isChecked();

    bool bExclProc = ui->cbEVexcludeProc->isChecked();
    bool bExclProc_prim = ui->cbEVexcludeProcPrim->isChecked();

    bool bLimVols = ui->cbLimitToVolumes->isChecked();

    bool bLimParticles = ui->cbLimitToParticles->isChecked();
    bool bExcludeParticles = ui->cbExcludeParticles->isChecked();

    QStringList LimProc = ui->leEVlimitToProc->text().split(rx, QString::SkipEmptyParts);
    QStringList ExclProc = ui->leEVexcludeProc->text().split(rx, QString::SkipEmptyParts);
    QStringList LimVols = ui->leLimitToVolumes->text().split(rx, QString::SkipEmptyParts);

    QStringList MustContainParticles = ui->leLimitToParticles->text().split(rx, QString::SkipEmptyParts);
    QStringList ExcludeParticles = ui->leExcludeParticles->text().split(rx, QString::SkipEmptyParts);

    if (currentEv > (int)TH.size()) currentEv = (int)TH.size();

    bUp ? currentEv++ : currentEv--;
    while (currentEv >= 0 && currentEv < (int)TH.size())
    {
        const AEventTrackingRecord * er = TH.at(currentEv);

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
    };
    return -1;
}
*/

/*
void AParticleSimWin::on_pbNextEvent_clicked()
{
    QWidget * cw = ui->tabwinDiagnose->currentWidget();
    int i = ui->sbEvent->value();
    if (cw == ui->tabEventViewer)
    {
        if (MW->SimulationManager->TrackingHistory.empty())
        {
            guitools::message("Tracking history is empty!", this);
            return;
        }
        int newi = findEventWithFilters(i, true);
        if (newi == -1 && i != MW->EventsDataHub->countEvents()-1)
            guitools::message("There are no events according to the selected criteria", this);
        else i = newi;
    }
    else i++;

    if (i >= 0 && i < MW->EventsDataHub->countEvents()) ui->sbEvent->setValue(i);
}
*/

/*
void AParticleSimWin::on_pbPreviousEvent_clicked()
{
    QWidget * cw = ui->tabwinDiagnose->currentWidget();
    int i = ui->sbEvent->value();
    if (cw == ui->tabEventViewer)
    {
        if (MW->SimulationManager->TrackingHistory.empty())
        {
            guitools::message("Tracking history is empty!", this);
            return;
        }
        int newi = findEventWithFilters(i, false);
        if (newi == -1 && i != 0)
            guitools::message("There are no events according to the selected criteria", this);
        else i = newi;
    }
    else i--;
    if (i >= 0) ui->sbEvent->setValue(i);
}
*/

/*
void AParticleSimWin::on_sbEvent_valueChanged(int i)
{
    if (EventsDataHub->Events.isEmpty())
        ui->sbEvent->setValue(0);
    else if (i >= EventsDataHub->Events.size())
        ui->sbEvent->setValue(EventsDataHub->Events.size()-1); //will retrigger this method
    else
    {
        QWidget * cw = ui->tabwinDiagnose->currentWidget();

        if (cw == ui->tabText && !bForbidUpdate) ShowOneEventLog(i);
        else if (cw == ui->tabPMhits || cw == ui->tabPmHitViz) on_pbRefreshViz_clicked();
        else if (cw == ui->tabEventViewer) EV_show();
    }
}
*/

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

/*
void AParticleSimWin::on_pbEVgeo_clicked()
{
    EV_showGeo();
}
*/

/*
#include <QMenu>
#include "TGraph.h"
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
    QAction * showPosition = nullptr;  if (st) showPosition = Menu.addAction("Show position");
    QAction * centerA       = nullptr; if (st) centerA = Menu.addAction("Center view at this position");
    Menu.addSeparator();
    QAction * showELDD = Menu.addAction("Show energy linear deposition density");
    QAction* selectedItem = Menu.exec(ui->trwEventView->mapToGlobal(pos));
    if (!selectedItem) return; //nothing was selected
    if (selectedItem == showELDD)
    {
        std::vector<float> dist;
        std::vector<float> ELDD;
        pr->fillELDD(st, dist, ELDD);

        if (!dist.empty())
        {
            TGraph * g = MW->GraphWindow->ConstructTGraph(dist, ELDD, "Deposited energy: linear density", "Distance, mm", "Linear density, keV/mm", 4, 20, 1, 4);
            MW->GraphWindow->Draw(g, "APL");
            //MW->GraphWindow->UpdateRootCanvas();
        }
    }
    else if (selectedItem == showPosition)
    {
        double pos[3];
        for (int i=0; i<3; i++) pos[i] = st->Position[i];
        MW->GeometryWindow->ShowPoint(pos, true);
    }
    else if (selectedItem == centerA)
    {
        double pos[3];
        for (int i=0; i<3; i++) pos[i] = st->Position[i];
        MW->GeometryWindow->CenterView(pos);
    }
}
*/

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

void AParticleSimWin::on_pbEventView_clicked()
{
    EV_showTree();
}

// --- Statistics ---

#include "atrackinghistorycrawler.h"
#include "TH1D.h"
#include "TH2D.h"

void AParticleSimWin::on_pbPTHistRequest_clicked()
{
    // !!!***
    //add multithread
    // move to SimManager?
    AFindRecordSelector Opt;
    ATrackingHistoryCrawler Crawler(ui->leWorkingDirectory->text() + "/" + ui->leTrackingDataFile->text(), false); // !!!*** binary control

    Opt.bParticle = ui->cbPTHistParticle->isChecked();
    Opt.Particle = ui->lePTHistParticle->text();
    Opt.bPrimary = ui->cbPTHistOnlyPrim->isChecked();
    Opt.bSecondary = ui->cbPTHistOnlySec->isChecked();
    Opt.bLimitToFirstInteractionOfPrimary = ui->cbPTHistLimitToFirst->isChecked();

    int    bins = ui->sbPTHistBinsX->value();
    double from = ui->ledPTHistFromX->text().toDouble();
    double to   = ui->ledPTHistToX  ->text().toDouble();

    int    bins2 = ui->sbPTHistBinsY->value();
    double from2 = ui->ledPTHistFromY->text().toDouble();
    double to2   = ui->ledPTHistToY  ->text().toDouble();

    int Selector = ui->twPTHistType->currentIndex(); // 0 - Vol, 1 - Boundary
    if (Selector == 0)
    {
        Opt.bMaterial = ui->cbPTHistVolMat->isChecked();
        Opt.Material = ui->cobPTHistVolMat->currentIndex();
        Opt.bVolume = ui->cbPTHistVolVolume->isChecked();
        Opt.Volume = ui->lePTHistVolVolume->text().toLocal8Bit().data();
        Opt.bVolumeIndex = ui->cbPTHistVolIndex->isChecked();
        Opt.VolumeIndex = ui->sbPTHistVolIndex->value();

        int What = ui->cobPTHistVolRequestWhat->currentIndex();
        switch (What)
        {
        case 0:
          {
            AHistorySearchProcessor_findParticles p;
            Crawler.find(Opt, p);
            QMap<QString, int>::const_iterator it = p.FoundParticles.constBegin();
            ui->ptePTHist->clear();
            ui->ptePTHist->appendPlainText("Particles found:\n");
            while (it != p.FoundParticles.constEnd())
            {
                ui->ptePTHist->appendPlainText(QString("%1   %2 times").arg(it.key()).arg(it.value()));
                ++it;
            }
            break;
          }
        case 1:
          {
            int mode = ui->cobPTHistVolPlus->currentIndex();
            if (mode < 0 || mode > 2)
            {
                guitools::message("Unknown process selection mode", this);
                return;
            }

            AHistorySearchProcessor_findProcesses::SelectionMode sm = static_cast<AHistorySearchProcessor_findProcesses::SelectionMode>(mode);
            AHistorySearchProcessor_findProcesses p(sm);
            Crawler.find(Opt, p);

            QMap<QString, int>::const_iterator it = p.FoundProcesses.constBegin();
            ui->ptePTHist->clear();
            ui->ptePTHist->appendPlainText("Processes found:\n");
            while (it != p.FoundProcesses.constEnd())
            {
                ui->ptePTHist->appendPlainText(QString("%1   %2 times").arg(it.key()).arg(it.value()));
                ++it;
            }

            selectedModeForProcess = mode;
            break;
          }
        case 2:
          {
            AHistorySearchProcessor_findTravelledDistances p(bins, from, to);
            Crawler.find(Opt, p);

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
                return;
            }
            AHistorySearchProcessor_findDepositedEnergy::CollectionMode edm = static_cast<AHistorySearchProcessor_findDepositedEnergy::CollectionMode>(mode);

            if (ui->cbPTHistVolVsTime->isChecked())
            {
                AHistorySearchProcessor_findDepositedEnergyTimed p(edm, bins, from, to, bins2, from2, to2);
                Crawler.find(Opt, p);

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
                Crawler.find(Opt, p);

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
                p = new AHistorySearchProcessor_getDepositionStatsTimeAware(ui->ledTimeFrom->text().toFloat(), ui->ledTimeTo->text().toFloat());
                Crawler.find(Opt, *p);
            }
            else
            {
                p = new AHistorySearchProcessor_getDepositionStats();
                Crawler.find(Opt, *p);
            }

            ui->ptePTHist->clear();
            ui->ptePTHist->appendPlainText("Deposition statistics:\n");
            QMap<QString, AParticleDepoStat>::const_iterator it = p->DepoData.constBegin();
            std::vector< QPair<QString, AParticleDepoStat> > vec;
            double sum = 0;
            while (it != p->DepoData.constEnd())
            {
                vec.push_back( QPair<QString, AParticleDepoStat>(it.key(), it.value()) );
                sum += it.value().sum;
                ++it;
            }
            double sumInv = (sum > 0 ? 100.0/sum : 1.0);

            std::sort(vec.begin(), vec.end(), [](const QPair<QString, AParticleDepoStat> & a, const QPair<QString, AParticleDepoStat> & b)->bool{return a.second.sum > b.second.sum;});

            for (const auto & el : vec)
            {
                const AParticleDepoStat & rec = el.second;
                const double mean = rec.sum / rec.num;
                const double sigma = sqrt( (rec.sumOfSquares - 2.0*mean*rec.sum)/rec.num + mean*mean );

                QString str = QString("%1\t%2 keV (%3%)\t#: %4").arg(el.first).arg(rec.sum).arg( QString::number(rec.sum*sumInv, 'g', 4) ).arg(rec.num);

                if (rec.num > 1)  str += QString("\tmean: %1 keV").arg(mean);
                if (rec.num > 10) str += QString("\tsigma: %1 keV").arg(sigma);

                ui->ptePTHist->appendPlainText(str);
            }
            ui->ptePTHist->appendPlainText("\n---------\n");
            ui->ptePTHist->appendPlainText(QString("sum of all listed depositions: %1 keV").arg(sum));
            delete p;
            break;
         }
        default:
            qWarning() << "Unknown type of volume request";
        }
    }
    else
    {
        //Border
        Opt.bFromMat = ui->cbPTHistVolMatFrom->isChecked();
        Opt.FromMat = ui->cobPTHistVolMatFrom->currentIndex();
        Opt.bFromVolume = ui->cbPTHistVolVolumeFrom->isChecked();
        Opt.bEscaping = ui->cbPTHistEscaping->isChecked();
        Opt.FromVolume = ui->lePTHistVolVolumeFrom->text().toLocal8Bit().data();
        Opt.bFromVolIndex = ui->cbPTHistVolIndexFrom->isChecked();
        Opt.FromVolIndex = ui->sbPTHistVolIndexFrom->value();

        Opt.bToMat = ui->cbPTHistVolMatTo->isChecked();
        Opt.ToMat = ui->cobPTHistVolMatTo->currentIndex();
        Opt.bToVolume = ui->cbPTHistVolVolumeTo->isChecked();
        Opt.bCreated = ui->cbPTHistCreated->isChecked();
        Opt.ToVolume = ui->lePTHistVolVolumeTo->text().toLocal8Bit().data();
        Opt.bToVolIndex = ui->cbPTHistVolIndexTo->isChecked();
        Opt.ToVolIndex = ui->sbPTHistVolIndexTo->value();

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
                Crawler.find(Opt, p);
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
                    Crawler.find(Opt, p);
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
                    Crawler.find(Opt, p);
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
                    Crawler.find(Opt, p);
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
        ui->frPTHistX->setVisible( ui->cobPTHistVolRequestWhat->currentIndex() > 1  && ui->cobPTHistVolRequestWhat->currentIndex() != 4);
        ui->frPTHistY->setVisible( ui->cobPTHistVolRequestWhat->currentIndex() == 3 && ui->cbPTHistVolVsTime->isChecked() );
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
    if (newName == SimSet.FileGenSettings.getFileName()) return;
    SimSet.FileGenSettings.setFileName(newName);
    updateFileParticleGeneratorGui();
}

void AParticleSimWin::updateFileParticleGeneratorGui()
{
    const QString FileName(SimSet.FileGenSettings.getFileName().data());
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
    pg->initWithCheck(ui->cbFileCollectStatistics->isChecked());
//    WindowNavigator->BusyOff(); // <--

    if (AErrorHub::isError()) guitools::message(AErrorHub::getError().data(), this);
    updateFileParticleGeneratorGui();
}

void AParticleSimWin::on_pbFilePreview_clicked()
{
    AErrorHub::clear();

    bool ok = SimManager.Generator_File->initWithCheck(false);
    if (!ok)
        guitools::message(AErrorHub::getError().data(), this);
    else
    {
        QString out(SimManager.Generator_File->getPreview(100).data());
        guitools::message1(out, SimSet.FileGenSettings.getFileName().data(), this);
    }
}
