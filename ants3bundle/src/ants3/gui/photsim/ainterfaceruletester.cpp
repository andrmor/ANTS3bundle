#include "ainterfaceruletester.h"
#include "ui_ainterfaceruletester.h"
#include "guitools.h"
#include "amaterialhub.h"
#include "ainterfacerule.h"
#include "ainterfacerulehub.h"
#include "aphoton.h"
#include "aphotonstatistics.h"
#include "ajsontools.h"
#include "arandomhub.h"
#include "astatisticshub.h"
#include "agraphbuilder.h"
#include "aphotonsimhub.h"
#include "ageometryhub.h"
#include "ametalinterfacerule.h"
#include "aphotontracer.h"
#include "alightsensorevent.h" // needed by aphotontracer

#include <QDoubleValidator>
#include <QLineEdit>
#include <QDebug>

#include "TVector3.h"
#include "TGraph.h"
#include "TMath.h"
#include "TH1D.h"
#include "TGeoManager.h"
#include "TVirtualGeoTrack.h"

#include <complex>

AInterfaceRuleTester::AInterfaceRuleTester(AInterfaceRule* & ovLocal, int matFrom, int matTo, QWidget * parent) :
    AGuiWindow("RuleTester", parent),
    MatHub(AMaterialHub::getConstInstance()),
    GeoHub(AGeometryHub::getInstance()),
    RandomHub(ARandomHub::getInstance()),
    Stats(AStatisticsHub::getInstance().SimStat),
    Rule(ovLocal), MatFrom(matFrom), MatTo(matTo),
    ui(new Ui::AInterfaceRuleTester)
{
    ui->setupUi(this);
    setWindowTitle("Interface rule tester");

    QDoubleValidator * dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = this->findChildren<QLineEdit *>();
    foreach(QLineEdit *w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);

    const QStringList matNames = AMaterialHub::getConstInstance().getListOfMaterialNames();
    ui->labMaterials->setText( QString("%1 --> %2").arg(matNames.at(matFrom)).arg(matNames.at(matTo)) );

    updateGUI();

    DummyLightSensorEvent = new ALightSensorEvent();
    QTextStream * dummyStream = nullptr;
    PhotonTracer = new APhotonTracer(*DummyLightSensorEvent, dummyStream, dummyStream, dummyStream);
    // dont forget to 'configure' with the actual override and photon
}

void AInterfaceRuleTester::updateGUI()
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
        str2 = QString("%1 + i*%2").arg(QString::number(ref.real(),'g',4)).arg(QString::number(ref.imag(),'g',4));
    }
    if (Rule && Rule->getType() == "DielectricToMetal")
    {
        AMetalInterfaceRule * mir = static_cast<AMetalInterfaceRule*>(Rule);
        str2 = QString("%1 + i*%2").arg(QString::number(mir->RealN,'g',4)).arg(QString::number(mir->ImaginaryN,'g',4));
    }
    ui->labRefractiveIndexFrom->setText(str1);
    ui->labRefractiveIndexTo->setText(str2);
}

AInterfaceRuleTester::~AInterfaceRuleTester()
{
    delete ui;

    delete PhotonTracer;
    delete DummyLightSensorEvent;
    delete ReverseRule;
}

void AInterfaceRuleTester::writeToJson(QJsonObject &json) const
{
    json["PositionX"] = x();
    json["PositionY"] = y();
}

void AInterfaceRuleTester::readFromJson(const QJsonObject &json)
{
    if (json.isEmpty()) return;

    int x, y;
    jstools::parseJson(json, "PositionX", x);
    jstools::parseJson(json, "PositionY", y);
    if (x>0 && y>0) move(x, y);
}

#include <numeric>
void AInterfaceRuleTester::on_pbProcessesVsAngle_clicked()
{
    ui->pte->clear();
    if ( !beforeRun() ) return;

    int numPhotons = ui->sbST_number->value();
    std::vector<double> Reflected(100, 0), Transmitted(100, 0), Absorbed(100, 0), Undefined(100, 0);
    std::vector<double> Spike(100, 0), BackSpike(100, 0), BackLobe(100, 0), BackLambert(100, 0), WaveShifted(100, 0);
    std::vector<double> Angle;
    double N[3], K[3];
    N[0] = 0;
    N[1] = 0;
    N[2] = -1.0;

    APhoton ph;
    Stats.clear();

    setEnabled(false); AbortCycle = false; QApplication::processEvents();
    for (int iAngle = 0; iAngle < 91; iAngle++) //cycle by angle of incidence
    {
        double angle = iAngle;
        if (angle == 90) angle = 89.999;
        Angle.push_back(angle);

        //angle->photon direction
        double cosA = cos(TMath::Pi() * angle / 180.0);
        double sinA = sin(TMath::Pi() * angle / 180.0);
        for (int iPhot = 0; iPhot < numPhotons; iPhot++)
        {
            if (iPhot % 1000 == 0) qApp->processEvents();
            if (AbortCycle)
            {
                qDebug() << "Close clicked during cycle";
                return;
            }

            //have to reset since K is modified by the override object
            K[0] = sinA;
            K[1] = 0;
            K[2] = cosA;

            ph.v[0] = K[0];
            ph.v[1] = K[1];
            ph.v[2] = -K[2];
            ph.waveIndex = getWaveIndex();

            EInterfaceResult result = runSinglePhoton(N, ph);
            switch (result)
            {
            case EInterfaceResult::Undefined   : Undefined[iAngle]++;   continue;
            case EInterfaceResult::Absorbed    : Absorbed[iAngle]++;    continue;
            case EInterfaceResult::Transmitted : Transmitted[iAngle]++; break;
            case EInterfaceResult::Reflected   : Reflected[iAngle]++;   break;
            }

            switch (Rule->Status)
            {
            case AInterfaceRule::SpikeReflection:            Spike[iAngle]++;       break;
            case AInterfaceRule::BackscatterSpikeReflection: BackSpike[iAngle]++;   break;
            case AInterfaceRule::LobeReflection:             BackLobe[iAngle]++;    break;
            case AInterfaceRule::LambertianReflection:       BackLambert[iAngle]++; break;
            default: ;
            }
        }

        Undefined[iAngle] /= numPhotons;
        Absorbed[iAngle] /= numPhotons;
        Transmitted[iAngle] /= numPhotons;
        Reflected[iAngle] /= numPhotons;

        Spike[iAngle] /= numPhotons;
        BackSpike[iAngle] /= numPhotons;
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
        TGraph * gA = AGraphBuilder::graph(Angle, Absorbed);    AGraphBuilder::configure(gA, "Absorption",   "Angle of incidence, deg", "Fraction",   1, 0, 1,   1, 1, 2);
        TGraph * gB = AGraphBuilder::graph(Angle, Reflected);   AGraphBuilder::configure(gB, "Reflection",   "Angle of incidence, deg", "Fraction",   3, 0, 1,   3, 1, 2);
        TGraph * gF = AGraphBuilder::graph(Angle, Transmitted); AGraphBuilder::configure(gF, "Transmission", "Angle of incidence, deg", "Fraction",   4, 0, 1,   4, 1, 2);
        TGraph * gU = nullptr;
        int sum = std::accumulate(Undefined.begin(), Undefined.end(), 0);
        if (sum > 0)
        {
                 gU = AGraphBuilder::graph(Angle, Undefined);   AGraphBuilder::configure(gU, "Error",       "Angle of incidence, deg", "Fraction",   2, 0, 1,   2, 1, 2);
        }

        gA->SetMinimum(0);
        gA->SetMaximum(1.05);

        emit requestDraw(gA,         "AL",    true, false);
        emit requestDraw(gB,         "Lsame", true, false);
        emit requestDraw(gF,         "Lsame", true, true);
        if (gU) emit requestDraw(gU, "Lsame", true, true);

        emit requestDrawLegend(0.12,0.12, 0.4,0.25, "");
        break;
    }
    case 1:
    {
        TGraph * gT =  AGraphBuilder::graph(Angle, Reflected);   AGraphBuilder::configure(gT,  "All reflections", "Angle of incidence, deg", "Fraction",  2,   0, 1,     2, 1, 2);
        TGraph * gS =  AGraphBuilder::graph(Angle, Spike);       AGraphBuilder::configure(gS,  "Spike",           "Angle of incidence, deg", "Fraction",  1,   0, 1,     1, 1, 2);
        TGraph * gBS = AGraphBuilder::graph(Angle, BackSpike);   AGraphBuilder::configure(gBS, "BackwardSpike",   "Angle of incidence, deg", "Fraction",  797, 0, 1,   797, 1, 2);
        TGraph * gL =  AGraphBuilder::graph(Angle, BackLobe);    AGraphBuilder::configure(gL,  "Lobe",            "Angle of incidence, deg", "Fraction",  3,   0, 1,     3, 1, 2);
        TGraph * gD =  AGraphBuilder::graph(Angle, BackLambert); AGraphBuilder::configure(gD,  "Diffuse",         "Angle of incidence, deg", "Fraction",  4,   0, 1,     4, 1, 2);
        gT->SetMinimum(0);
        gT->SetMaximum(1.05);

        emit requestDraw(gT,  "AL",    true, false);
        emit requestDraw(gS,  "Lsame", true, false);
        emit requestDraw(gBS, "Lsame", true, false);
        emit requestDraw(gL,  "Lsame", true, false);
        emit requestDraw(gD,  "Lsame", true, true);

        emit requestDrawLegend(0.12,0.75, 0.4,0.93, "Reflection fractions");
        break;
    }
    case 2:
    {
        TGraph * gW = AGraphBuilder::graph(Angle, WaveShifted);
        AGraphBuilder::configure(gW, "Wavelength shifted", "Angle of incidence, deg", "Fraction", 4, 0, 1, 4, 1, 2);
        gW->SetMaximum(1.05);
        gW->SetMinimum(0);
        emit requestDraw(gW, "AL", true, true);
        //emit requestDrawLegend(0.1,0.1, 0.5,0.5, "Waveshifted");
        break;
    }
    }

    setEnabled(true);
}

EInterfaceResult AInterfaceRuleTester::runSinglePhoton(double * globalNormal, APhoton & photon)
{
    PhotonTracer->configureForInterfaceRuleTester(MatFrom, MatTo, Rule, globalNormal, photon);
    EInterfaceResult result = PhotonTracer->processInterface();
    PhotonTracer->readBackPhoton(photon);

    // have to catch for rough surface: Status is 'Transmitted', but the direction is backward
    if (result != EInterfaceResult::Transmitted) return result;
    if (photon.v[0]*globalNormal[0] + photon.v[1]*globalNormal[1] + photon.v[2]*globalNormal[2] > 0) return result; // good direction

    if (!Rule || Rule->isPolishedSurface())
    {
        qWarning() << "Unexpected 'transmitted with wrong direction' environment!";
        return EInterfaceResult::Absorbed;
    }

    //qDebug() << "\"Bad photon\":"  << photon.v[0] << photon.v[1] << photon.v[2] << (int)result << "<-0123 is Undefined, Absorbed, Reflected, Transmitted";

    std::array<double,3> reversedNormal;
    for (size_t i = 0; i < 3; i++) reversedNormal[i] = -globalNormal[i];

    double reversed = true;
    do
    {
        if (reversed)
        {
            PhotonTracer->configureForInterfaceRuleTester(MatTo, MatFrom, ReverseRule, reversedNormal.data(), photon);
            result = PhotonTracer->processInterface();
            PhotonTracer->readBackPhoton(photon);
            if (result == EInterfaceResult::Transmitted &&
                photon.v[0]*reversedNormal[0] + photon.v[1]*reversedNormal[1] + photon.v[2]*reversedNormal[2] < 0)
            {
                //again the same problem
                reversed = false;
                continue;
            }
            // valid photon
            // in the reversed case we need to reverse the result "interpretation"
            if (result == EInterfaceResult::Transmitted)
            {
                result = EInterfaceResult::Reflected;
                Rule->Status = AInterfaceRule::LobeReflection;
            }
            else if (result == EInterfaceResult::Reflected)
                result = EInterfaceResult::Transmitted;
            break; // done
        }
        else
        {
            PhotonTracer->configureForInterfaceRuleTester(MatFrom, MatTo, Rule, globalNormal, photon);
            result = PhotonTracer->processInterface();
            PhotonTracer->readBackPhoton(photon);
            if (result == EInterfaceResult::Transmitted &&
                photon.v[0]*globalNormal[0] + photon.v[1]*globalNormal[1] + photon.v[2]*globalNormal[2] < 0)
            {
                //again the same problem
                reversed = true;
                continue;
            }
            break; // done
        }
    }
    while (true);

    //qDebug() << "After:" << photon.v[0] << photon.v[1] << photon.v[2] << (int)result << "<-0123 is Undefined, Absorbed, Reflected, Transmitted";
    return result;
}

void AInterfaceRuleTester::on_pbTracePhotons_clicked()
{
    if ( !beforeRun() ) return;

    //qDebug() << "Surface:->" << (int)((*pOV)->SurfaceSettings.Model);

    //surface normal and photon direction
    TVector3 SurfNorm(0, 0, -1.0);
    double N[3]; //needs to calculate override   // !!!*** memory leak, see also the other two methods
    N[0] = SurfNorm.X();
    N[1] = SurfNorm.Y();
    N[2] = SurfNorm.Z();
    TVector3 PhotDir = getPhotonVector();

    Tracks.clear();
    double d = 0.5; //offset - for drawing only

    //preparing and running cycle with photons

    TH1D * histBack = new TH1D("", "Reflected",   100, 0, 100);
    TH1D * histForw = new TH1D("", "Transmitted", 100, 0, 100);
    histBack->GetXaxis()->SetTitle("Angle from global normal, degrees"); histBack->SetLineWidth(2); histBack->SetLineColor(2);
    histForw->GetXaxis()->SetTitle("Angle from global normal, degrees"); histForw->SetLineWidth(2);

    APhoton ph;
    Stats.clear();
    AReportForOverride rep;

    const int waveIndex = getWaveIndex();
    const int numPhot = ui->sbST_number->value();
    setEnabled(false); AbortCycle = false; QApplication::processEvents();
    for (int i = 0; i < numPhot; i++)
    {
        if (i % 1000 == 0) qApp->processEvents();
        if (AbortCycle)
        {
            qDebug() << "Close clicked during cycle";
            return;
        }

        ph.v[0] = PhotDir.X(); //old has output direction after full cycle!
        ph.v[1] = PhotDir.Y();
        ph.v[2] = PhotDir.Z();
        ph.time = 0;
        ph.waveIndex = waveIndex;

        EInterfaceResult result = runSinglePhoton(N, ph);
        switch (result)
        {
        case EInterfaceResult::Undefined   : rep.error++; continue;
        case EInterfaceResult::Absorbed    : rep.abs++;   continue;
        case EInterfaceResult::Transmitted : rep.forw++;  break;
        case EInterfaceResult::Reflected   : rep.back++;  break;
        }

        short col;
        int type;
        bool bBack = false;
        bool bForw = false;
        if (Rule->Status == AInterfaceRule::SpikeReflection)
        {
            rep.Bspike++;
            type = 0;
            col = 6; //0,magenta for Spike
            bBack = true;
        }
        else if (Rule->Status == AInterfaceRule::BackscatterSpikeReflection)
        {
            rep.Bbackspike++;
            type = 0;
            col = 6; //0,magenta as for normal Spike
            bBack = true;
        }
        else if (Rule->Status == AInterfaceRule::LobeReflection)
        {
            rep.Blobe++;
            type = 1;
            col = 7; //1,teal for Lobe
            bBack = true;
        }
        else if (Rule->Status == AInterfaceRule::LambertianReflection)
        {
            rep.Blamb++;
            type = 2;
            col = 3; //2,grean for lambert
            bBack = true;
        }
        else
        {
            type = 666;
            col = kBlue; //blue for transmitted or error
            bForw = true;
        }

        Tracks.push_back(ATmpTrackRec(type, col));
        Tracks.back().Nodes.push_back( {d, d, d} );
        Tracks.back().Nodes.push_back( {d + ph.v[0], d + ph.v[1], d + ph.v[2]} );

        double costr = - SurfNorm[0] * ph.v[0] - SurfNorm[1] * ph.v[1] - SurfNorm[2] * ph.v[2];

        if (bBack) histBack->Fill(180.0 / TMath::Pi() * acos(costr));
        if (bForw) histForw->Fill(180.0 - 180.0 / TMath::Pi() * acos(costr));
    }

    double max = 1.05 * std::max(histBack->GetMaximum(), histForw->GetMaximum());
    histBack->SetMaximum(max);
    histForw->SetMaximum(max);

    emit requestDraw(histBack, "hist", true, true);
    emit requestDraw(histForw, "histsame", true, true);
    emit requestDrawLegend(0.7,0.8, 0.95,0.95, "");

    on_pbST_showTracks_clicked();

    rep.waveChanged = Stats.WaveChanged;
    rep.timeChanged = Stats.TimeChanged;
    reportStatistics(rep, numPhot);

    setEnabled(true);
}

void AInterfaceRuleTester::showGeometry()
{
    emit requestClearGeometryViewer();
    GeoHub.GeoManager->ClearTracks();

    double d = 0.5;
    double f = 0.5;

    //surface normal
    TVector3 SurfNorm(0, 0, -1.0);
    int track_index = GeoHub.GeoManager->AddTrack(1,22);
    TVirtualGeoTrack* track = GeoHub.GeoManager->GetTrack(track_index);
    track->AddPoint(d, d, d, 0);
    track->AddPoint(d + SurfNorm.X(), d + SurfNorm.Y(), d + SurfNorm.Z(), 0);
    track->SetLineWidth(3);
    track->SetLineStyle(2);
    track->SetLineColor(1);

    //surface
    track_index = GeoHub.GeoManager->AddTrack(1, 22);
    track = GeoHub.GeoManager->GetTrack(track_index);
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
    track_index = GeoHub.GeoManager->AddTrack(1, 10);
    track = GeoHub.GeoManager->GetTrack(track_index);
    track->AddPoint(d, d, d, 0);

    TVector3 PhotDir = getPhotonVector();
    track->AddPoint(d - PhotDir.X(), d - PhotDir.Y(), d - PhotDir.Z(), 0);
    track->SetLineColor(kRed);
    track->SetLineWidth(3);

    emit requestShowTracks(false);
}

void AInterfaceRuleTester::on_pbST_showTracks_clicked()
{
    if (Tracks.empty()) return;
    int selector = ui->cobST_trackType->currentIndex() - 1;
    if (selector == 3) return; //do not show any tracks

    showGeometry();

    size_t numTracks = 0;
    for(size_t iTrack = 0; iTrack < Tracks.size() && numTracks < MaxNumTracks; iTrack++)
    {
        const ATmpTrackRec & th = Tracks[iTrack];
        //filter
        if (selector > -1)  //-1 - show all
            if (selector != th.Type) continue;

        int track_index = GeoHub.GeoManager->AddTrack(1,22);
        TVirtualGeoTrack * track = GeoHub.GeoManager->GetTrack(track_index);
        track->SetLineColor(th.Color);
        track->SetLineWidth(1);
        for (size_t iNode=0; iNode < th.Nodes.size(); iNode++)
            track->AddPoint(th.Nodes[iNode][0], th.Nodes[iNode][1], th.Nodes[iNode][2], 0);
    }

    emit requestShowTracks(true);
}

bool AInterfaceRuleTester::beforeRun()
{
    if (!Rule)
    {
        guitools::message("Override not defined!", this);
        return false;
    }

    QString err = Rule->checkOverrideData(); // also prepares runtime
    if (!err.isEmpty())
    {
        guitools::message("Override reports an error:\n" + err, this);
        return false;
    }

    delete ReverseRule; ReverseRule = nullptr;
    if (Rule->isRoughSurface() && !Rule->SurfaceSettings.KillPhotonsRefractedBackward)
    {
        QJsonObject json;
        Rule->writeToJson(json);
        ReverseRule = AInterfaceRule::interfaceRuleFactory(Rule->getType(), Rule->getMaterialTo(), Rule->getMaterialFrom()); // reversed materials!
        bool ok = false;
        if (ReverseRule) ok = ReverseRule->readFromJson(json);
        if (!ok)
        {
            guitools::message("Failed to create the reverse interface rule!", this);
            return false;
        }
        QString err = ReverseRule->checkOverrideData(); // also prepares runtime
        if (!err.isEmpty())
        {
            guitools::message("Reverse rule error:\n" + err, this);
            return false;
        }
    }

    return true;
}

int AInterfaceRuleTester::getWaveIndex()
{
    const double wavelength = ui->ledST_wave->text().toDouble();
    if (ui->cbWavelength->isChecked())
        //return MPcollection->WaveToIndex(wavelength); // always in [0, WaveNodes-1]
        return APhotonSimHub::getConstInstance().Settings.WaveSet.toIndex(wavelength);
    else return -1;
}

TVector3 AInterfaceRuleTester::getPhotonVector()
{
    TVector3 PhotDir(0, 0, -1.0);
    TVector3 perp(0, 1.0, 0);
    double angle = ui->ledAngle->text().toDouble();
    angle *= -TMath::Pi() / 180.0;
    PhotDir.Rotate(angle, perp);
    return PhotDir;
}

void AInterfaceRuleTester::on_pbDiffuseIrradiation_clicked()
{
    ui->pte->clear();
    if ( !beforeRun() ) return;

    double N[3]; //normal
    N[0] = 0;
    N[1] = 0;
    N[2] = -1.0;

    double K[3]; //photon direction - new for every photon!

    TH1D * histBack = new TH1D("", "Reflected",   100, 0, 100);
    TH1D * histForw = new TH1D("", "Transmitted", 100, 0, 100);
    histBack->GetXaxis()->SetTitle("Angle from global normal, degrees"); histBack->SetLineWidth(2); histBack->SetLineColor(2);
    histForw->GetXaxis()->SetTitle("Angle from global normal, degrees"); histForw->SetLineWidth(2);

    const int waveIndex = getWaveIndex();
    const int numPhot = ui->sbST_number->value();

    APhoton ph;
    Stats.clear();
    AReportForOverride rep;

    setEnabled(false); AbortCycle = false; QApplication::processEvents();
    for (int i = 0; i < numPhot; i++)
    {
        if (i % 1000 == 0) qApp->processEvents();
        if (AbortCycle)
        {
            qDebug() << "Close clicked during cycle";
            return;
        }

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

        bool bBack = false;
        bool bForw = false;
        EInterfaceResult result = runSinglePhoton(N, ph);
        switch (result)
        {
        case EInterfaceResult::Undefined   : rep.error++;               continue;
        case EInterfaceResult::Absorbed    : rep.abs++;                 continue;
        case EInterfaceResult::Transmitted : rep.forw++;  bForw = true; break;
        case EInterfaceResult::Reflected   : rep.back++;  bBack = true; break;
        }

        if      (Rule->Status == AInterfaceRule::SpikeReflection)            rep.Bspike++;
        else if (Rule->Status == AInterfaceRule::BackscatterSpikeReflection) rep.Bbackspike++;
        else if (Rule->Status == AInterfaceRule::LobeReflection)             rep.Blobe++;
        else if (Rule->Status == AInterfaceRule::LambertianReflection)       rep.Blamb++;

        double costr = - N[0] * ph.v[0] - N[1] * ph.v[1] - N[2] * ph.v[2];

        if (bBack) histBack->Fill(180.0 / TMath::Pi() * acos(costr));
        if (bForw) histForw->Fill(180.0 - 180.0 / TMath::Pi() * acos(costr));
    }

    double max = 1.05 * std::max(histBack->GetMaximum(), histForw->GetMaximum());
    histBack->SetMaximum(max);
    histForw->SetMaximum(max);

    emit requestDraw(histBack, "hist", true, true);
    emit requestDraw(histForw, "histsame", true, true);
    emit requestDrawLegend(0.7,0.8, 0.95,0.95, "");

    rep.waveChanged = Stats.WaveChanged;
    rep.timeChanged = Stats.TimeChanged;
    reportStatistics(rep, numPhot);

    showGeometry(); //to clear track vis

    setEnabled(true);
}

void AInterfaceRuleTester::on_cbWavelength_toggled(bool)
{
    updateGUI();
}

void AInterfaceRuleTester::on_ledST_wave_editingFinished()
{
    updateGUI();
}

void AInterfaceRuleTester::on_ledAngle_editingFinished()
{
    const double angle = ui->ledAngle->text().toDouble();
    if (angle < 0 || angle >= 90.0 )
    {
        ui->ledAngle->setText("45");
        guitools::message("Angle should be within [0, 90) degrees!", this);
    }

    showGeometry();
}

void AInterfaceRuleTester::closeEvent(QCloseEvent * e)
{
    AbortCycle = true;
    storeGeomStatus();
    QMainWindow::closeEvent(e);
    emit closed(true);
}

void AInterfaceRuleTester::reportStatistics(const AReportForOverride &rep, int numPhot)
{
    ui->pte->clear();

    QString t;
    if (rep.error > 0)  t += QString("Error detected: %1\n\n").arg(rep.error);

    t += "Outcomes:\n";
    if (rep.abs > 0)    t += QString("  Absorbed: %1%  (%2)\n").arg(rep.abs/numPhot*100.0).arg(rep.abs);
    if (rep.back > 0)   t += QString("  Reflected: %1%  (%2)\n").arg(rep.back/numPhot*100.0).arg(rep.back);
    if (rep.forw)       t += QString("  Transmitted: %1%  (%2)\n").arg(rep.forw/numPhot*100.0).arg(rep.forw);
    if (rep.error)      t += QString("  Error: %1%  (%2)\n").arg(rep.error/numPhot*100.0).arg(rep.error);
    t += "\n";

    if (rep.back > 0)
    {
        //show stat of processes
        t += "Reflection composition:\n";
        if (rep.Bspike > 0)     t += QString("  Specular spike: %1%  (%2,   %3% of total)\n").arg(rep.Bspike/rep.back*100.0).arg(rep.Bspike).arg(100.0*rep.Bspike/numPhot);
        if (rep.Bbackspike > 0) t += QString("  Backward specular spike: %1%  (%2,   %3% of total)\n").arg(rep.Bbackspike/rep.back*100.0).arg(rep.Bbackspike).arg(100.0*rep.Bbackspike/numPhot);
        if (rep.Blobe > 0)      t += QString("  Specular lobe: %1%  (%2,   %3% of total)\n").arg(rep.Blobe/rep.back*100.0).arg(rep.Blobe).arg(100.0*rep.Blobe/numPhot);
        if (rep.Blamb > 0)      t += QString("  Lambertian: %1%  (%2,   %3% of total)\n").arg(rep.Blamb/rep.back*100.0).arg(rep.Blamb).arg(100.0*rep.Blamb/numPhot);
    }

    if (rep.waveChanged > 0)
        t += QString("\nWavelength changed: %1  (%2)\n").arg(rep.waveChanged/numPhot*100.0).arg(rep.waveChanged);

    if (rep.timeChanged > 0)
        t += QString("\nTime changed: %1  (%2)\n").arg(rep.timeChanged/numPhot*100.0).arg(rep.timeChanged);


    ui->pte->appendPlainText(t);
    ui->pte->moveCursor(QTextCursor::Start);
    ui->pte->ensureCursorVisible();
}
