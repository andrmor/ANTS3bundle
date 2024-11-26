#include "ainterfacewidgetfactory.h"
#include "ainterfacerule.h"
#include "abasicinterfacerule.h"
#include "ametalinterfacerule.h"
#include "fsnpinterfacerule.h"
#include "asurfaceinterfacerule.h"
#include "aunifiedrule.h"
#include "awaveshifterinterfacerule.h"
#include "aspectralbasicinterfacerule.h"
#include "guitools.h"
#include "aphotonsimhub.h"

#include <QObject>
#include <QDebug>
#include <QWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QComboBox>
#include <QPushButton>

QWidget * AInterfaceWidgetFactory::createEditWidget(AInterfaceRule * Rule, QWidget * Caller, GraphWindowClass * GraphWindow)
{
    ASpectralBasicInterfaceRule * spir = dynamic_cast<ASpectralBasicInterfaceRule*>(Rule); // has to be before ABasicInterfaceRule
    if (spir) return new ASpectralBasicInterfaceWidget(spir, Caller, GraphWindow);

    ABasicInterfaceRule * bir = dynamic_cast<ABasicInterfaceRule*>(Rule);
    if (bir) return new ABasicInterfaceWidget(bir);

    AMetalInterfaceRule * mir = dynamic_cast<AMetalInterfaceRule*>(Rule);
    if (mir) return new AMetalInterfaceWidget(mir);

    FsnpInterfaceRule * fir = dynamic_cast<FsnpInterfaceRule*>(Rule);
    if (fir) return new AFsnpInterfaceWidget(fir);

    ASurfaceInterfaceRule * sir = dynamic_cast<ASurfaceInterfaceRule*>(Rule);
    if (sir) return new ASurfaceInterfaceWidget(sir);

    AUnifiedRule * uir = dynamic_cast<AUnifiedRule*>(Rule);
    if (uir) return new AUnifiedInterfaceWidget(uir);

    AWaveshifterInterfaceRule * wir = dynamic_cast<AWaveshifterInterfaceRule*>(Rule);
    if (wir) return new AWaveshifterInterfaceWidget(wir, Caller, GraphWindow);


    qWarning() << "Unknown interface rule!";
    QFrame * f = new QFrame();
    f->setFrameStyle(QFrame::Box);
    f->setMinimumHeight(100);
    return f;
}

ABasicInterfaceWidget::ABasicInterfaceWidget(ABasicInterfaceRule * rule) : QFrame()
{
    setFrameStyle(QFrame::Box);

    QHBoxLayout * hl = new QHBoxLayout(this);
        QVBoxLayout * l = new QVBoxLayout();
            QLabel * lab = new QLabel("Absorption:");
        l->addWidget(lab);
            lab = new QLabel("Specular reflection:");
        l->addWidget(lab);
            lab = new QLabel("Scattering:");
        l->addWidget(lab);
    hl->addLayout(l);
        l = new QVBoxLayout();
            QLineEdit * le = new QLineEdit(QString::number(rule->Abs));
            QDoubleValidator* val = new QDoubleValidator(this);
            val->setNotation(QDoubleValidator::StandardNotation);
            val->setBottom(0);
            //val->setTop(1.0); //Qt(5.8.0) BUG: check does not work
            val->setDecimals(6);
            le->setValidator(val);
            QObject::connect(le, &QLineEdit::editingFinished, [le, rule]() { rule->Abs = le->text().toDouble(); } );
        l->addWidget(le);
            le = new QLineEdit(QString::number(rule->Spec));
            le->setValidator(val);
            QObject::connect(le, &QLineEdit::editingFinished, [le, rule]() { rule->Spec = le->text().toDouble(); } );
        l->addWidget(le);
            le = new QLineEdit(QString::number(rule->Scat));
            le->setValidator(val);
            QObject::connect(le, &QLineEdit::editingFinished, [le, rule]() { rule->Scat = le->text().toDouble(); } );
        l->addWidget(le);
    hl->addLayout(l);
        l = new QVBoxLayout();
            lab = new QLabel("");
        l->addWidget(lab);
            lab = new QLabel("");
        l->addWidget(lab);
            QComboBox* com = new QComboBox();
            com->addItem("Isotropic (4Pi)"); com->addItem("Lambertian, 2Pi back"); com->addItem("Lambertian, 2Pi forward");
            com->setCurrentIndex(rule->ScatterModel);
            QObject::connect(com, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), [rule](int index) { rule->ScatterModel = index; } );
        l->addWidget(com);
    hl->addLayout(l);
}

AMetalInterfaceWidget::AMetalInterfaceWidget(AMetalInterfaceRule * rule) : QFrame()
{
    setFrameStyle(QFrame::Box);

    QHBoxLayout* hl = new QHBoxLayout(this);
    QVBoxLayout* l = new QVBoxLayout();
    QLabel* lab = new QLabel("Refractive index, real:");
    l->addWidget(lab);
    lab = new QLabel("Refractive index, imaginary:");
    l->addWidget(lab);
    hl->addLayout(l);
    l = new QVBoxLayout();
    QLineEdit* le = new QLineEdit(QString::number(rule->RealN));
    QDoubleValidator* val = new QDoubleValidator(this);
    val->setNotation(QDoubleValidator::StandardNotation);
    //val->setBottom(0);
    val->setDecimals(6);
    le->setValidator(val);
    QObject::connect(le, &QLineEdit::editingFinished, [le, rule]() { rule->RealN = le->text().toDouble(); } );
    l->addWidget(le);
    le = new QLineEdit(QString::number(rule->ImaginaryN));
    le->setValidator(val);
    QObject::connect(le, &QLineEdit::editingFinished, [le, rule]() { rule->ImaginaryN = le->text().toDouble(); } );
    l->addWidget(le);
    hl->addLayout(l);
}

AFsnpInterfaceWidget::AFsnpInterfaceWidget(FsnpInterfaceRule * rule) : QFrame()
{
    setFrameStyle(QFrame::Box);

    QHBoxLayout* l = new QHBoxLayout(this);
        QLabel* lab = new QLabel("Albedo:");
    l->addWidget(lab);
        QLineEdit* le = new QLineEdit(QString::number(rule->Albedo));
        QDoubleValidator* val = new QDoubleValidator(this);
        val->setNotation(QDoubleValidator::StandardNotation);
        val->setBottom(0);
        //val->setTop(1.0); //Qt(5.8.0) BUG: check does not work
        val->setDecimals(6);
        le->setValidator(val);
        QObject::connect(le, &QLineEdit::editingFinished, [le, rule]() { rule->Albedo = le->text().toDouble(); } );
    l->addWidget(le);
}

AWaveshifterInterfaceWidget::AWaveshifterInterfaceWidget(AWaveshifterInterfaceRule * rule, QWidget * caller, GraphWindowClass * graphWindow) :
    QFrame(), Rule(rule), Caller(caller), GraphWindow(graphWindow)
{
    setFrameStyle(QFrame::Box);

    QVBoxLayout* vl = new QVBoxLayout(this);
        QHBoxLayout* l = new QHBoxLayout();
            QLabel* lab = new QLabel("Reemission generation:");
        l->addWidget(lab);
            QComboBox* com = new QComboBox();
            com->addItem("Isotropic (4Pi)"); com->addItem("Lambertian, 2Pi back"); com->addItem("Lambertian, 2Pi forward");
            com->setCurrentIndex(rule->ReemissionModel);
            QObject::connect(com, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), [rule](int index) { rule->ReemissionModel = index; } );
        l->addWidget(com);
    vl->addLayout(l);
        l = new QHBoxLayout();
            QVBoxLayout* vv = new QVBoxLayout();
                lab = new QLabel("Reemission probability:");
            vv->addWidget(lab);
                lab = new QLabel("Emission spectrum:");
            vv->addWidget(lab);
        l->addLayout(vv);
            vv = new QVBoxLayout();
                QPushButton* pb = new QPushButton("Load");
                pb->setToolTip("Every line of the file should contain 2 numbers:\nwavelength[nm] reemission_probability[0..1]");
                QObject::connect(pb, &QPushButton::clicked, this, &AWaveshifterInterfaceWidget::loadReemissionProbability);
            vv->addWidget(pb);
                pb = new QPushButton("Load");
                pb->setToolTip("Every line of the file should contain 2 numbers:\nwavelength[nm] relative_intencity[>=0]");
                QObject::connect(pb, &QPushButton::clicked, this, &AWaveshifterInterfaceWidget::loadEmissionSpectrum);
            vv->addWidget(pb);
        l->addLayout(vv);
            vv = new QVBoxLayout();
                pbShowRP = new QPushButton("Show");
                QObject::connect(pbShowRP, &QPushButton::clicked, this, &AWaveshifterInterfaceWidget::showReemissionProbability);
            vv->addWidget(pbShowRP);
                pbShowES = new QPushButton("Show");
                QObject::connect(pbShowES, &QPushButton::clicked, this, &AWaveshifterInterfaceWidget::showEmissionSpectrum);
            vv->addWidget(pbShowES);
        l->addLayout(vv);
            vv = new QVBoxLayout();
                pbShowRPbinned = new QPushButton("Binned");
                QObject::connect(pbShowRPbinned, &QPushButton::clicked, this, &AWaveshifterInterfaceWidget::showBinnedReemissionProbability);
            vv->addWidget(pbShowRPbinned);
                pbShowESbinned = new QPushButton("Binned");
                QObject::connect(pbShowESbinned, &QPushButton::clicked, this, &AWaveshifterInterfaceWidget::showBinnedEmissionSpectrum);
            vv->addWidget(pbShowESbinned);
        l->addLayout(vv);
    vl->addLayout(l);
        lab = new QLabel("If simulation is NOT wavelength-resolved, this override does nothing!");
        lab->setAlignment(Qt::AlignCenter);
    vl->addWidget(lab);
    updateButtons();
}

void AWaveshifterInterfaceWidget::loadReemissionProbability()
{
    /*
    AGlobalSettings& GlobSet = AGlobalSettings::getInstance();
    QString fileName = QFileDialog::getOpenFileName(caller, "Load reemission probability", GlobSet.LastOpenDir, "Data files (*.dat *.txt);;All files (*)");
    if (fileName.isEmpty()) return;
    GlobSet.LastOpenDir = QFileInfo(fileName).absolutePath();
    QVector<double> X, Y;
    int ret = LoadDoubleVectorsFromFile(fileName, &X, &Y);
    if (ret == 0)
    {
        ReemissionProbability_lambda = X;
        ReemissionProbability = Y;
        updateButtons();
    }
    */
}

void AWaveshifterInterfaceWidget::loadEmissionSpectrum()
{
    /*
    AGlobalSettings& GlobSet = AGlobalSettings::getInstance();
    QString fileName = QFileDialog::getOpenFileName(caller, "Load emission spectrum", GlobSet.LastOpenDir, "Data files (*.dat *.txt);;All files (*)");
    if (fileName.isEmpty()) return;
    GlobSet.LastOpenDir = QFileInfo(fileName).absolutePath();
    QVector<double> X, Y;
    int ret = LoadDoubleVectorsFromFile(fileName, &X, &Y);
    if (ret == 0)
    {
        EmissionSpectrum_lambda = X;
        EmissionSpectrum = Y;
        updateButtons();
    }
    */
}

void AWaveshifterInterfaceWidget::showReemissionProbability()
{
    if (Rule->ReemissionProbability_lambda.isEmpty())
    {
        guitools::message("No data were loaded", Caller);
        return;
    }
/*
    TGraph* gr = GraphWindow->ConstructTGraph(ReemissionProbability_lambda, ReemissionProbability, "Reemission probability", "Wavelength, nm", "Reemission probability, a.u.", 2, 20, 1, 2, 2);
    gr->SetMinimum(0);
    GraphWindow->Draw(gr, "apl");
*/
}

void AWaveshifterInterfaceWidget::showEmissionSpectrum()
{
    if (Rule->EmissionSpectrum_lambda.isEmpty())
    {
        guitools::message("No data were loaded", Caller);
        return;
    }
/*
    TGraph* gr = GraphWindow->ConstructTGraph(EmissionSpectrum_lambda, EmissionSpectrum,
                                              "Emission spectrum", "Wavelength, nm", "Relative intensity, a.u.",
                                              4, 20, 1,
                                              4, 2);
    gr->SetMinimum(0);
    GraphWindow->Draw(gr, "apl");
*/
}

void AWaveshifterInterfaceWidget::showBinnedReemissionProbability()
{
    Rule->initializeWaveResolved();

    //TODO run checker

    /*
    if (!WaveSet.Enabled)
    {
        guitools::message("Simulation is NOT wavelength resolved, override is inactive!", caller);
        return;
    }

    const int WaveNodes = WaveSet.countNodes();
    QVector<double> waveIndex;
    for (int i=0; i<WaveNodes; i++) waveIndex << i;
    TGraph* gr = GraphWindow->ConstructTGraph(waveIndex, ReemissionProbabilityBinned,
                                              "Reemission probability (binned)", "Wave index", "Reemission probability, a.u.",
                                              2, 20, 1,
                                              2, 2);
    gr->SetMinimum(0);
    GraphWindow->Draw(gr, "apl");
*/
}

void AWaveshifterInterfaceWidget::showBinnedEmissionSpectrum()
{
    Rule->initializeWaveResolved();

    //TODO run checker
/*
    if (!WaveSet.Enabled)
    {
        guitools::message("Simulation is NOT wavelength resolved, override is inactive!", caller);
        return;
    }

    if ( !Spectrum ) //paranoic
    {
        guitools::message("Spectrum is not defined!", caller);
        return;
    }

    double integral = Spectrum->ComputeIntegral();
    if (integral <= 0)
    {
        guitools::message("Binned emission spectrum: integral <=0, override will report an error!", caller);
        return;
    }

    TH1D* SpectrumCopy = new TH1D(*Spectrum);
    SpectrumCopy->SetTitle("Binned emission spectrum");
    SpectrumCopy->GetXaxis()->SetTitle("Wavelength, nm");
    SpectrumCopy->GetYaxis()->SetTitle("Relative intensity, a.u.");
//    GraphWindow->Draw(SpectrumCopy, "hist"); //gets ownership of the copy
*/
}

void AWaveshifterInterfaceWidget::updateButtons()
{
    pbShowRP->setDisabled(Rule->ReemissionProbability_lambda.isEmpty());
    pbShowES->setDisabled(Rule->EmissionSpectrum_lambda.isEmpty());
    bool bWR = true; // WaveSet.Enabled;  !!!***
    pbShowRPbinned->setDisabled(!bWR || Rule->ReemissionProbability_lambda.isEmpty());
    pbShowESbinned->setDisabled(!bWR || Rule->EmissionSpectrum_lambda.isEmpty());
}

// -------------

ASpectralBasicInterfaceWidget::ASpectralBasicInterfaceWidget(ASpectralBasicInterfaceRule * rule, QWidget * caller, GraphWindowClass * graphWindow) :
    QFrame(), Rule(rule), Caller(caller), GraphWindow(graphWindow)
{
    setFrameStyle(QFrame::Box);

    QVBoxLayout* vl = new QVBoxLayout(this);
        QHBoxLayout* l = new QHBoxLayout();
            QLabel* lab = new QLabel("Absorption, reflection and scattering:");
        l->addWidget(lab);
            QPushButton* pb = new QPushButton("Load");
            pb->setToolTip("Every line of the file should contain 4 numbers:\nwavelength[nm] absorption_prob[0..1] reflection_prob[0..1] scattering_prob[0..1]");
            QObject::connect(pb, &QPushButton::clicked, this, &ASpectralBasicInterfaceWidget::loadSpectralData);
        l->addWidget(pb);
            pbShow = new QPushButton("Show");
            QObject::connect(pbShow, &QPushButton::clicked, this, &ASpectralBasicInterfaceWidget::showLoaded);
        l->addWidget(pbShow);
            pbShowBinned = new QPushButton("Binned");
            QObject::connect(pbShowBinned, &QPushButton::clicked, this, &ASpectralBasicInterfaceWidget::showBinned);
        l->addWidget(pbShowBinned);
    vl->addLayout(l);
        l = new QHBoxLayout();
            lab = new QLabel("Scattering model:");
        l->addWidget(lab);
            QComboBox* com = new QComboBox();
            com->addItem("Isotropic (4Pi)"); com->addItem("Lambertian, 2Pi back"); com->addItem("Lambertian, 2Pi forward");
            com->setCurrentIndex(Rule->ScatterModel);
            QObject::connect(com, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), [rule](int index) { rule->ScatterModel = index; } );
         l->addWidget(com);
    vl->addLayout(l);
         l = new QHBoxLayout();
            lab = new QLabel("For photons with WaveIndex=-1, assume wavelength of:");
         l->addWidget(lab);
            QLineEdit* le = new QLineEdit(QString::number(Rule->effectiveWavelength));
            QDoubleValidator* val = new QDoubleValidator(this);
            val->setNotation(QDoubleValidator::StandardNotation);
            val->setBottom(0);
            val->setDecimals(6);
            le->setValidator(val);
            QObject::connect(le, &QLineEdit::editingFinished, [le, rule]() { rule->effectiveWavelength = le->text().toDouble(); } );
        l->addWidget(le);
            lab = new QLabel("nm");
        l->addWidget(lab);
    vl->addLayout(l);

    updateButtons();
}

void ASpectralBasicInterfaceWidget::loadSpectralData()
{
    QString fileName = guitools::dialogLoadFile(this, "Load data from file", "Data files (*.dat *.txt);;All files (*.*)");
    if (fileName.isEmpty()) return;

    QString err = Rule->loadData(fileName);
    if (!err.isEmpty()) guitools::message(err, this);
}

#include "TMultiGraph.h"
#include "agraphbuilder.h"
#include "graphwindowclass.h"
#include "TAxis.h"
void ASpectralBasicInterfaceWidget::showLoaded()
{
    std::vector<double> fresnel(Rule->Wave.size());
    for (size_t i = 0; i < Rule->Wave.size(); i++)
        fresnel[i] = 1.0 - Rule->ProbLoss[i] - Rule->ProbRef[i] - Rule->ProbDiff[i];

    TMultiGraph * mg = new TMultiGraph();
    TGraph * gLoss = AGraphBuilder::graph(Rule->Wave, Rule->ProbLoss);
    AGraphBuilder::configure(gLoss, "Absorption", "Wavelength, nm", "", 2, 20, 1, 2);
    mg->Add(gLoss, "LP");
    TGraph* gRef = AGraphBuilder::graph(Rule->Wave, Rule->ProbRef);
    AGraphBuilder::configure(gRef, "Specular reflection", "Wavelength, nm", "", 4, 21, 1, 4);
    mg->Add(gRef, "LP");
    TGraph* gDiff = AGraphBuilder::graph(Rule->Wave, Rule->ProbDiff);
    AGraphBuilder::configure(gDiff, "Diffuse scattering", "Wavelength, nm", "", 7, 22, 1, 7);
    mg->Add(gDiff, "LP");
    TGraph* gFr = AGraphBuilder::graph(Rule->Wave, fresnel);
    AGraphBuilder::configure(gFr, "Fresnel", "Wavelength, nm", "", 1, 24, 1, 1, 1, 1);
    mg->Add(gFr, "LP");

    mg->SetMinimum(0);
    GraphWindow->Draw(mg, "apl");
    mg->GetXaxis()->SetTitle("Wavelength, nm");
    mg->GetYaxis()->SetTitle("Probability");
    //GraphWindow->addLegend(0.7,0.8, 0.95,0.95, "");
}

void ASpectralBasicInterfaceWidget::showBinned()
{
    /*
    bool bWR;
    double WaveFrom, WaveTo, WaveStep;
    int WaveNodes;
    MatCollection->GetWave(bWR, WaveFrom, WaveTo, WaveStep, WaveNodes);

    initializeWaveResolved();

    //TODO run checker

    if (!bWR)
    {
        QString s = "Simulation is configured as not wavelength-resolved\n";
        s +=        "All photons will have the same properties:\n";
        s +=QString("Absorption: %1").arg(probLoss) + "\n";
        s +=QString("Specular reflection: %1").arg(probRef) + "\n";
        s +=QString("Scattering: %1").arg(probDiff);
        message(s, widget);
        return;
    }

    QVector<double> waveIndex;
    for (int i=0; i<WaveNodes; i++) waveIndex << i;

    QVector<double> Fr;
    for (int i=0; i<waveIndex.size(); i++)
        Fr << (1.0 - ProbLossBinned.at(i) - ProbRefBinned.at(i) - ProbDiffBinned.at(i));

    TMultiGraph* mg = new TMultiGraph();
    TGraph* gLoss = GraphWindow->ConstructTGraph(waveIndex, ProbLossBinned, "Loss", "Wave index", "Loss", 2, 20, 1, 2);
    mg->Add(gLoss, "LP");
    TGraph* gRef = GraphWindow->ConstructTGraph(waveIndex, ProbRefBinned, "Specular reflection", "Wave index", "Reflection", 4, 21, 1, 4);
    mg->Add(gRef, "LP");
    TGraph* gDiff = GraphWindow->ConstructTGraph(waveIndex, ProbDiffBinned, "Diffuse scattering", "Wave index", "Scatter", 7, 22, 1, 7);
    mg->Add(gDiff, "LP");
    TGraph* gFr = GraphWindow->ConstructTGraph(waveIndex, Fr, "Fresnel", "Wave index", "", 1, 24, 1, 1, 1, 1);
    mg->Add(gFr, "LP");

    mg->SetMinimum(0);
    GraphWindow->Draw(mg, "apl");
    mg->GetXaxis()->SetTitle("Wave index");
    mg->GetYaxis()->SetTitle("Probability");
    GraphWindow->AddLegend(0.7,0.8, 0.95,0.95, "");
    */
}

void ASpectralBasicInterfaceWidget::updateButtons()
{
    pbShow->setDisabled(Rule->Wave.empty());
    const AWaveResSettings & WaveSet = APhotonSimHub::getConstInstance().Settings.WaveSet;
    pbShowBinned->setDisabled(!WaveSet.Enabled || Rule->Wave.empty());
}

// -------

ASurfaceInterfaceWidget::ASurfaceInterfaceWidget(ASurfaceInterfaceRule *rule)
{
    setFrameStyle(QFrame::Box);

    QHBoxLayout* l = new QHBoxLayout(this);
        QLabel* lab = new QLabel("Using Fresnel equations and Snell's law");
        lab->setAlignment(Qt::AlignHCenter);
    l->addWidget(lab);
    /*
        QLineEdit* le = new QLineEdit(QString::number(rule->Albedo));
        QDoubleValidator* val = new QDoubleValidator(this);
        val->setNotation(QDoubleValidator::StandardNotation);
        val->setBottom(0);
        //val->setTop(1.0); //Qt(5.8.0) BUG: check does not work
        val->setDecimals(6);
        le->setValidator(val);
        QObject::connect(le, &QLineEdit::editingFinished, [le, rule]() { rule->Albedo = le->text().toDouble(); } );
    l->addWidget(le);
    */
}

AUnifiedInterfaceWidget::AUnifiedInterfaceWidget(AUnifiedRule * rule)
{
    setFrameStyle(QFrame::Box);

    QGridLayout * lay = new QGridLayout(this);

    lay->addWidget(new QLabel("Specular Spike:"), 0, 0);
    lay->addWidget(new QLabel("Specular Lobe:"), 0, 2);
    lay->addWidget(new QLabel("Diffuse Lobe:"), 1, 0);
    lay->addWidget(new QLabel("Backscatter Spike:"), 1, 2);

    QDoubleValidator* val = new QDoubleValidator(this);
    val->setNotation(QDoubleValidator::StandardNotation);
    val->setBottom(0.0);
    val->setTop(1.0);
    val->setDecimals(6);

    QLineEdit * leSS = new QLineEdit(QString::number(rule->Cspec));     leSS->setValidator(val);
    QLineEdit * leSL = new QLineEdit(QString::number(rule->Cspeclobe)); leSL->setValidator(val);
    QLineEdit * leDL = new QLineEdit(QString::number(rule->Cdiflobe));  leDL->setValidator(val);
    QLineEdit * leBS = new QLineEdit(QString::number(rule->Cback));     leBS->setValidator(val);

    lay->addWidget(leSS, 0, 1);
    lay->addWidget(leSL, 0, 3);
    lay->addWidget(leDL, 1, 1);
    lay->addWidget(leBS, 1, 3);

    QObject::connect(leSS, &QLineEdit::editingFinished, [leSS, rule]() { rule->Cspec = leSS->text().toDouble(); } );
    QObject::connect(leSL, &QLineEdit::editingFinished, [leSL, rule]() { rule->Cspeclobe = leSL->text().toDouble(); } );
    QObject::connect(leDL, &QLineEdit::editingFinished, [leDL, rule]() { rule->Cdiflobe = leDL->text().toDouble(); } );
    QObject::connect(leBS, &QLineEdit::editingFinished, [leBS, rule]() { rule->Cback = leBS->text().toDouble(); } );
}
