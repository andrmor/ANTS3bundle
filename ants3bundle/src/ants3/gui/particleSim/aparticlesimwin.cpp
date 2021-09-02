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
}

void AParticleSimWin::updateG4Gui()
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

    AParticleSourceDialog ParticleSourceDialog(SourceGenSettings.SourceData.at(isource), this);

    int res = ParticleSourceDialog.exec(); // if detector is rebuild (this->readSimSettingsFromJson() is triggered), ParticleSourceDialog is signal-blocked and rejected
    if (res == QDialog::Rejected) return;

    AParticleSourceRecord & ps = ParticleSourceDialog.getResult();
    ps.updateLimitedToMat();
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
    s.GunParticles.push_back(GunParticleStruct());
    SimSet.SourceGenSettings.append(s);

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

    const QString SourceName = SimSet.SourceGenSettings.SourceData.at(isource).name.data();
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
    ASourceGenSettings & SourceGenSettings = SimSet.SourceGenSettings;
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
            QLabel* lab = new QLabel(pr.name.data());
            lab->setMinimumWidth(110);
            QFont f = lab->font();
            f.setBold(true);
            lab->setFont(f);
        l->addWidget(lab);
        l->addWidget(new QLabel( QString(pr.getShapeString().data()) + ','));
        l->addWidget(new QLabel( QString("%1 particle%2").arg(pr.GunParticles.size()).arg( pr.GunParticles.size()>1 ? "s" : "" ) ) );
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
#include "aparticlesimmanager.h"
void AParticleSimWin::on_pbGunTest_clicked()
{
    AParticleSimManager & SimManager = AParticleSimManager::getInstance();
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
//        pg = SimManager->FileParticleGenerator;
//        SimManager->FileParticleGenerator->InitWithCheck(SimulationManager->Settings.partSimSet.FileGenSettings, false);
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

    SimSet.FileGenSettings.ValidationMode = AFileGenSettings::Relaxed;
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
void AParticleSimWin::drawSource(int iSource)
{
    //check iSource is correct  !!!***
    const AParticleSourceRecord & p = SimSet.SourceGenSettings.SourceData.at(iSource);

    int index = p.shape;
    double X0 = p.X0;
    double Y0 = p.Y0;
    double Z0 = p.Z0;
    double Phi = p.Phi*3.1415926535/180.0;
    double Theta = p.Theta*3.1415926535/180.0;
    double Psi = p.Psi*3.1415926535/180.0;
    double size1 = p.size1;
    double size2 = p.size2;
    double size3 = p.size3;
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
    if (!Gun)
    {
        guitools::message("Particle gun is not defined", this);
        return;
    }

    bool bOK = Gun->init();
    if (!bOK)
    {
        guitools::message( QString("Failed to initialize particle gun!\n%0").arg(Gun->ErrorString.data()), this);
        return;
    }
    Gun->setStartEvent(0);
//    if (ui->cobParticleGenerationMode->currentIndex() == 1) updateFileParticleGeneratorGui();

    const double WorldSizeXY = AGeometryHub::getInstance().getWorldSizeXY();
    const double WorldSizeZ  = AGeometryHub::getInstance().getWorldSizeZ();
    double Length = std::max(WorldSizeXY, WorldSizeZ)*0.4;
    double R[3], K[3];
    std::vector<AParticleRecord> GP;
    int numTracks = 0;
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
    AParticleSimManager::getInstance().simulate();
}
