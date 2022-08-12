#include "aopticaloverridetester.h"
#include "ui_aopticaloverridetester.h"
#include "mainwindow.h"
#include "guitools.h"
#include "amaterialhub.h"
#include "ainterfacerule.h"
#include "ainterfacerulehub.h"
#include "aphoton.h"
#include "aphotonstatistics.h"
#include "aphotontrackrecord.h"
#include "ajsontools.h"
#include "a3global.h"
#include "arandomhub.h"
#include "astatisticshub.h"
#include "agraphbuilder.h"
#include "aphotonsimhub.h"

#include <QDoubleValidator>
#include <QLineEdit>
#include <QSpinBox>
#include <QDebug>

#include "TVector3.h"
#include "TRandom2.h"
#include "TGraph.h"
#include "TLegend.h"
#include "TMath.h"
#include "TH1D.h"
#include "TGeoManager.h"
#include "TVirtualGeoTrack.h"

AOpticalOverrideTester::AOpticalOverrideTester(AInterfaceRule ** ovLocal, int matFrom, int matTo, QWidget * parent) :
    QMainWindow(parent),
    MatHub(AMaterialHub::getConstInstance()),
    RandomHub(ARandomHub::getInstance()),
    Stats(AStatisticsHub::getInstance().SimStat),
    pOV(ovLocal), MatFrom(matFrom), MatTo(matTo),
    ui(new Ui::AOpticalOverrideTester)
{
    ui->setupUi(this);
    setWindowTitle("Override tester");

    QDoubleValidator * dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = this->findChildren<QLineEdit *>();
    foreach(QLineEdit *w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);

    const QStringList matNames = AMaterialHub::getConstInstance().getListOfMaterialNames();
    ui->labMaterials->setText( QString("(%1 -> %2)").arg(matNames.at(matFrom)).arg(matNames.at(matTo)) );

    updateGUI();
}

#include <complex>
void AOpticalOverrideTester::updateGUI()
{
    bool bWR = APhotonSimHub::getConstInstance().Settings.WaveSet.Enabled;
    if (!bWR)
    {
        ui->cbWavelength->setEnabled(false);
        ui->cbWavelength->setChecked(false);
    }
    else
        ui->cbWavelength->setEnabled(true);

    const int waveIndex = getWaveIndex();
    QString str1, str2;
    if (MatHub[MatFrom]->Dielectric) str1 = QString::number( MatHub[MatFrom]->getRefractiveIndex(waveIndex) );
    else
    {
        const std::complex<double> ref = MatHub[MatFrom]->getComplexRefractiveIndex(waveIndex);
        str1 = QString("%1 %2*i").arg(QString::number(ref.real(),'g',4)).arg(QString::number(ref.imag(),'g',4));
    }
    if (MatHub[MatTo]->Dielectric) str2 = QString::number( MatHub[MatTo]->getRefractiveIndex(waveIndex) );
    else
    {
        const std::complex<double> ref = MatHub[MatTo]->getComplexRefractiveIndex(waveIndex);
        str2 = QString("%1 %2*i").arg(QString::number(ref.real(),'g',4)).arg(QString::number(ref.imag(),'g',4));
    }
    ui->ledST_Ref1->setText(str1);
    ui->ledST_Ref2->setText(str2);
}

AOpticalOverrideTester::~AOpticalOverrideTester()
{
    delete ui;
}

void AOpticalOverrideTester::writeToJson(QJsonObject &json) const
{
    json["PositionX"] = x();
    json["PositionY"] = y();
}

void AOpticalOverrideTester::readFromJson(const QJsonObject &json)
{
    if (json.isEmpty()) return;

    int x, y;
    jstools::parseJson(json, "PositionX", x);
    jstools::parseJson(json, "PositionY", y);
    if (x>0 && y>0) move(x, y);
}

void AOpticalOverrideTester::on_pbST_RvsAngle_clicked()
{
    if ( !testOverride() ) return;

    int numPhotons = ui->sbST_number->value();
    std::vector<double> Back(91, 0), Forward(91, 0), Absorb(91, 0), NotTrigger(91, 0);
    std::vector<double> Spike(91, 0), BackLobe(91, 0), BackLambert(91, 0), WaveShifted(91, 0);
    std::vector<double> Angle;
    double N[3], K[3];
    N[0] = 0;
    N[1] = 0;
    N[2] = 1.0;

    APhoton ph;
    Stats.clear();

    for (int iAngle = 0; iAngle < 91; iAngle++) //cycle by angle of incidence
    {
        double angle = iAngle;
        if (angle == 90) angle = 89.99;
        Angle.push_back(angle);

        //angle->photon direction
        double cosA = cos(TMath::Pi() * angle / 180.0);
        double sinA = sin(TMath::Pi() * angle / 180.0);
        for (int iPhot=0; iPhot<numPhotons; iPhot++) //cycle by photons
        {
            //have to reset since K is modified by the override object
            K[0] = sinA;
            K[1] = 0;
            K[2] = cosA;

            ph.v[0] = K[0];
            ph.v[1] = K[1];
            ph.v[2] = K[2];
            ph.waveIndex = getWaveIndex();
            AInterfaceRule::OpticalOverrideResultEnum result = (*pOV)->calculate(&ph, N);

            switch (result)
            {
            case AInterfaceRule::NotTriggered: NotTrigger[iAngle]++; break;
            case AInterfaceRule::Absorbed:     Absorb[iAngle]++;     break;
            case AInterfaceRule::Forward:      Forward[iAngle]++;    break;
            case AInterfaceRule::Back:         Back[iAngle]++;       break;
            default:;
            }

            switch ((*pOV)->Status)
            {
            case AInterfaceRule::SpikeReflection: Spike[iAngle]++; break;
            case AInterfaceRule::LobeReflection: BackLobe[iAngle]++; break;
            case AInterfaceRule::LambertianReflection: BackLambert[iAngle]++; break;
            default: ;
            }
        }
        NotTrigger[iAngle] /= numPhotons;
        Absorb[iAngle] /= numPhotons;
        Forward[iAngle] /= numPhotons;
        Back[iAngle] /= numPhotons;

        Spike[iAngle] /= numPhotons;
        BackLobe[iAngle] /= numPhotons;
        BackLambert[iAngle] /= numPhotons;

        WaveShifted[iAngle] = Stats.WaveChanged / numPhotons;
        Stats.WaveChanged = 0; // !!!*** ?
    }

    int what = ui->cobPrVsAngle_WhatToCollect->currentIndex();
    switch (what)
    {
    case 0:
    {
        TGraph * gN = AGraphBuilder::graph(Angle, NotTrigger); AGraphBuilder::configure(gN, "Not triggered", "Angle, deg", "Fraction",   2, 0, 1,   2, 1, 2);
        TGraph * gA = AGraphBuilder::graph(Angle, Absorb);     AGraphBuilder::configure(gA, "Absorption",    "Angle, deg", "Fraction",   1, 0, 1,   1, 1, 2);
        TGraph * gB = AGraphBuilder::graph(Angle, Back);       AGraphBuilder::configure(gB, "Back",          "Angle, deg", "Fraction",   3, 0, 1,   3, 1, 2);
        TGraph * gF = AGraphBuilder::graph(Angle, Forward);    AGraphBuilder::configure(gF, "Forward",       "Angle, deg", "Fraction",   4, 0, 1,   4, 1, 2);
        gN->SetMinimum(0);
        gN->SetMaximum(1.05);

        emit requestDraw(gN, "AL",    true, false);
        emit requestDraw(gA, "Lsame", true, false);
        emit requestDraw(gB, "Lsame", true, false);
        emit requestDraw(gF, "Lsame", true, false);

        emit requestDrawLegend(0.12,0.12, 0.4,0.25, "Basic info");
        break;
    }
    case 1:
    {
        TGraph * gT = AGraphBuilder::graph(Angle, Back);        AGraphBuilder::configure(gT, "All reflections", "Angle, deg", "Fraction",  2, 0, 1,   2, 1, 2);
        TGraph * gS = AGraphBuilder::graph(Angle, Spike);       AGraphBuilder::configure(gS, "Spike",           "Angle, deg", "Fraction",  1, 0, 1,   1, 1, 2);
        TGraph * gL = AGraphBuilder::graph(Angle, BackLobe);    AGraphBuilder::configure(gL, "Lobe",            "Angle, deg", "Fraction",  3, 0, 1,   3, 1, 2);
        TGraph * gD = AGraphBuilder::graph(Angle, BackLambert); AGraphBuilder::configure(gD, "Diffuse",         "Angle, deg", "Fraction",  4, 0, 1,   4, 1, 2);
        gT->SetMinimum(0);
        gT->SetMaximum(1.05);

        emit requestDraw(gT, "AL",    true, false);
        emit requestDraw(gS, "Lsame", true, false);
        emit requestDraw(gL, "Lsame", true, false);
        emit requestDraw(gD, "Lsame", true, false);

        emit requestDrawLegend(0.12,0.12, 0.4,0.25, "Backscattering");
        break;
    }
    case 2:
    {
        TGraph * gW = AGraphBuilder::graph(Angle, WaveShifted);
        AGraphBuilder::configure(gW, "Wavelength shifted", "Angle, deg", "Fraction", 4, 0, 1, 4, 1, 2);
        gW->SetMaximum(1.05);
        gW->SetMinimum(0);
        emit requestDraw(gW, "AL", true, false);
        //emit requestDrawLegend(0.1,0.1, 0.5,0.5, "Waveshifted");
        break;
    }
    }
}

void AOpticalOverrideTester::on_pbCSMtestmany_clicked()
{
    if ( !testOverride() ) return;

    qDebug() << "Surface:->" << (int)((*pOV)->SurfaceSettings.Model);

    //surface normal and photon direction
    TVector3 SurfNorm(0, 0, -1.0);
    double N[3]; //needs to calculate override
    N[0] = SurfNorm.X();
    N[1] = SurfNorm.Y();
    N[2] = SurfNorm.Z();
    TVector3 PhotDir = getPhotonVector();

    Tracks.clear();
    double d = 0.5; //offset - for drawing only

    //preparing and running cycle with photons

    TH1D * hist1 = new TH1D("", "", 100, 0, 0);
    hist1->GetXaxis()->SetTitle("Backscattering angle, degrees");

    APhoton ph;
    Stats.clear();
    AReportForOverride rep;

    const int waveIndex = getWaveIndex();
    const int numPhot = ui->sbST_number->value();
    for (int i = 0; i < numPhot; i++)
    {
        ph.v[0] = PhotDir.X(); //old has output direction after full cycle!
        ph.v[1] = PhotDir.Y();
        ph.v[2] = PhotDir.Z();
        ph.time = 0;
        ph.waveIndex = waveIndex;
        AInterfaceRule::OpticalOverrideResultEnum result = (*pOV)->calculate(&ph, N);

        //in case of absorption or not triggered override, do not build tracks!
        switch (result)
        {
        case AInterfaceRule::Absorbed:     rep.abs++; continue;           // ! ->
        case AInterfaceRule::NotTriggered: rep.notTrigger++; continue;    // ! ->
        case AInterfaceRule::Forward:      rep.forw++; break;
        case AInterfaceRule::Back:         rep.back++; break;
        default:                           rep.error++; continue;         // ! ->
        }

        short col;
        int type;
        if ((*pOV)->Status == AInterfaceRule::SpikeReflection)
        {
            rep.Bspike++;
            type = 0;
            col = 6; //0,magenta for Spike
        }
        else if ((*pOV)->Status == AInterfaceRule::LobeReflection)
        {
            rep.Blobe++;
            type = 1;
            col = 7; //1,teal for Lobe
        }
        else if ((*pOV)->Status == AInterfaceRule::LambertianReflection)
        {
            rep.Blamb++;
            type = 2;
            col = 3; //2,grean for lambert
        }
        else
        {
            type = 666;
            col = kBlue; //blue for error
        }

        Tracks.push_back(ATmpTrackRec(type, col));
        Tracks.back().Nodes.push_back( {d, d, d} );
        Tracks.back().Nodes.push_back( {d + ph.v[0], d + ph.v[1], d + ph.v[2]} );

        double costr = - SurfNorm[0] * ph.v[0] - SurfNorm[1] * ph.v[1] - SurfNorm[2] * ph.v[2];

        hist1->Fill(180.0 / TMath::Pi() * acos(costr));
    }

    emit requestDraw(hist1, "hist", true, true);
    on_pbST_showTracks_clicked();

    rep.waveChanged = Stats.WaveChanged;
    rep.timeChanged = Stats.TimeChanged;
    reportStatistics(rep, numPhot);
}

void AOpticalOverrideTester::showGeometry()
{
    gGeoManager->ClearTracks();
    emit requestClearGeometryViewer();

    double d = 0.5;
    double f = 0.5;

    //surface normal
    TVector3 SurfNorm(0, 0, -1.0);
    int track_index = gGeoManager->AddTrack(1,22);
    TVirtualGeoTrack* track = gGeoManager->GetTrack(track_index);
    track->AddPoint(d, d, d, 0);
    track->AddPoint(d + SurfNorm.X(), d + SurfNorm.Y(), d + SurfNorm.Z(), 0);
    track->SetLineWidth(3);
    track->SetLineStyle(2);
    track->SetLineColor(1);

    //surface
    track_index = gGeoManager->AddTrack(1, 22);
    track = gGeoManager->GetTrack(track_index);
    TVector3 perp = SurfNorm.Orthogonal();
    perp.Rotate(0.25 * TMath::Pi(), SurfNorm);
    for (int i=0; i<5; i++)
    {
        track->AddPoint(d + f * perp.X(), d + f * perp.Y(), d + f * perp.Z(), 0);
        perp.Rotate(0.5 * TMath::Pi(), SurfNorm);
    }
    track->SetLineWidth(3);
    track->SetLineColor(1);

    //intitial photon track
    track_index = gGeoManager->AddTrack(1, 10);
    track = gGeoManager->GetTrack(track_index);
    track->AddPoint(d, d, d, 0);

    TVector3 PhotDir = getPhotonVector();
    track->AddPoint(d - PhotDir.X(), d - PhotDir.Y(), d - PhotDir.Z(), 0);
    track->SetLineColor(kRed);
    track->SetLineWidth(3);

    emit requestShowTracks();
}

void AOpticalOverrideTester::on_pbST_showTracks_clicked()
{
    showGeometry();

    if (Tracks.empty()) return;
    int selector = ui->cobST_trackType->currentIndex() - 1;
    if (selector == 3) return; //do not show any tracks

    int numTracks = 0;
    for(int i = 1; i < Tracks.size() && numTracks < maxNumTracks; i++)
    {
        const ATmpTrackRec & th = Tracks[i];
        //filter
        if (selector > -1)  //-1 - show all
            if (selector != th.Type) continue;

        int track_index = gGeoManager->AddTrack(1,22);
        TVirtualGeoTrack * track = gGeoManager->GetTrack(track_index);
        track->SetLineColor(th.Color);
        track->SetLineWidth(1);
        for (int iNode=0; iNode < th.Nodes.size(); iNode++)
            track->AddPoint(th.Nodes[iNode][0], th.Nodes[iNode][1], th.Nodes[iNode][2], 0);
    }

    emit requestShowTracks();
}

bool AOpticalOverrideTester::testOverride()
{
    if ( !(*pOV) )
    {
        guitools::message("Override not defined!", this);
        return false;
    }

    QString err = (*pOV)->checkOverrideData();
    if (!err.isEmpty())
    {
        guitools::message("Override reports an error:\n" + err, this);
        return false;
    }
    return true;
}

int AOpticalOverrideTester::getWaveIndex()
{
    const double wavelength = ui->ledST_wave->text().toDouble();
    if (ui->cbWavelength->isChecked())
        //return MPcollection->WaveToIndex(wavelength); // always in [0, WaveNodes-1]
        return APhotonSimHub::getConstInstance().Settings.WaveSet.toIndex(wavelength);
    else return -1;
}

const TVector3 AOpticalOverrideTester::getPhotonVector()
{
    TVector3 PhotDir(0, 0, -1.0);
    TVector3 perp(0, 1.0, 0);
    double angle = ui->ledAngle->text().toDouble();
    angle *= -TMath::Pi() / 180.0;
    PhotDir.Rotate(angle, perp);
    return PhotDir;
}

void AOpticalOverrideTester::on_pbST_uniform_clicked()
{
    if ( !testOverride() ) return;

    double N[3]; //normal
    N[0] = 0;
    N[1] = 0;
    N[2] = -1.0;

    double K[3]; //photon direction - new for every photon!

    TH1D* hist1 = new TH1D("", "", 100, 0, 0);
    hist1->GetXaxis()->SetTitle("Backscattering angle, degrees");

    const int waveIndex = getWaveIndex();
    const int numPhot = ui->sbST_number->value();

    APhoton ph;
    Stats.clear();
    AReportForOverride rep;

    for (int i = 0; i < numPhot; i++)
    {
        //diffuse illumination - lambertian is used
        double sin2angle = RandomHub.uniform();
        double angle = asin(sqrt(sin2angle));
        double yOff = cos(angle), zOff = -sin(angle);
        K[0] = 0; K[1] = yOff; K[2] = zOff;  // -z direction on xy plane (incidence angle from 90 to 0)

        ph.v[0] = K[0];
        ph.v[1] = K[1];
        ph.v[2] = K[2];
        ph.time = 0;
        ph.waveIndex = waveIndex;

        AInterfaceRule::OpticalOverrideResultEnum result = (*pOV)->calculate(&ph, N);

        switch (result)
        {
        case AInterfaceRule::Absorbed: rep.abs++; break;
        case AInterfaceRule::NotTriggered: rep.notTrigger++; break;
        case AInterfaceRule::Forward: rep.forw++; break;
        case AInterfaceRule::Back: rep.back++; break;
        default: rep.error++;
        }

        if ((*pOV)->Status == AInterfaceRule::SpikeReflection) rep.Bspike++;
        else if ((*pOV)->Status == AInterfaceRule::LobeReflection) rep.Blobe++;
        else if ((*pOV)->Status == AInterfaceRule::LambertianReflection) rep.Blamb++;

        double costr = N[0]*K[0] + N[1]*K[1] + N[2]*K[2];
        hist1->Fill(180.0 / TMath::Pi() * acos(costr));
    }

    emit requestDraw(hist1, "hist", true, true);

    rep.waveChanged = Stats.WaveChanged;
    rep.timeChanged = Stats.TimeChanged;
    reportStatistics(rep, numPhot);

    showGeometry(); //to clear track vis
}

void AOpticalOverrideTester::on_cbWavelength_toggled(bool)
{
    updateGUI();
}

void AOpticalOverrideTester::on_ledST_wave_editingFinished()
{
    updateGUI();
}

void AOpticalOverrideTester::on_ledAngle_editingFinished()
{
    const double angle = ui->ledAngle->text().toDouble();
    if (angle < 0 || angle >= 90.0 )
    {
        ui->ledAngle->setText("45");
        guitools::message("Angle should be within [0, 90) degrees!", this);
    }

    showGeometry();
}

void AOpticalOverrideTester::closeEvent(QCloseEvent * e)
{
    QMainWindow::closeEvent(e);
    emit closed(true);
}

void AOpticalOverrideTester::reportStatistics(const AReportForOverride &rep, int numPhot)
{
    ui->pte->clear();

    QString t;
    if (rep.error > 0)  t += QString("Error detected: %1\n\n").arg(rep.error);

    t += "Processes:\n";
    if (rep.abs > 0)    t += QString("  Absorption: %1%  (%2)\n").arg(rep.abs/numPhot*100.0).arg(rep.abs);
    if (rep.back > 0)   t += QString("  Back: %1%  (%2)\n").arg(rep.back/numPhot*100.0).arg(rep.back);
    if (rep.forw)       t += QString("  Forward: %1%  (%2)\n").arg(rep.forw/numPhot*100.0).arg(rep.forw);
    if (rep.notTrigger) t += QString("  Not triggered: %1%  (%2)\n").arg(rep.notTrigger/numPhot*100.0).arg(rep.notTrigger);
    t += "\n";

    if (rep.back > 0)
    {
        //show stat of processes
        t += "Backscattering composition:\n";
        if (rep.Bspike > 0) t += QString("  Specular spike: %1%  (%2)\n").arg(rep.Bspike/rep.back*100.0).arg(rep.Bspike);
        if (rep.Blobe > 0)  t += QString("  Diffuse lobe: %1%  (%2)\n").arg(rep.Blobe/rep.back*100.0).arg(rep.Blobe);
        if (rep.Blamb > 0)  t += QString("  Lambertian: %1%  (%2)\n").arg(rep.Blamb/rep.back*100.0).arg(rep.Blamb);
    }

    if (rep.waveChanged > 0)
        t += QString("\nWavelength changed: %1  (%2)\n").arg(rep.waveChanged/numPhot*100.0).arg(rep.waveChanged);

    if (rep.timeChanged > 0)
        t += QString("\nTime changed: %1  (%2)\n").arg(rep.timeChanged/numPhot*100.0).arg(rep.timeChanged);


    ui->pte->appendPlainText(t);
    ui->pte->moveCursor(QTextCursor::Start);
    ui->pte->ensureCursorVisible();
}
