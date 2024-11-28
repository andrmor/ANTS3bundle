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
#include "agraphbuilder.h"
#include "graphwindowclass.h"

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

#include "TGraph.h"
#include "TH1D.h"

AInterfaceRuleWidget * AInterfaceWidgetFactory::createEditWidget(AInterfaceRule * rule, QWidget * parent)
{
    ASpectralBasicInterfaceRule * spir = dynamic_cast<ASpectralBasicInterfaceRule*>(rule); // has to be before ABasicInterfaceRule
    if (spir) return new ASpectralBasicInterfaceWidget(spir, parent);

    ABasicInterfaceRule * bir = dynamic_cast<ABasicInterfaceRule*>(rule);
    if (bir) return new ABasicInterfaceWidget(bir, parent);

    AMetalInterfaceRule * mir = dynamic_cast<AMetalInterfaceRule*>(rule);
    if (mir) return new AMetalInterfaceWidget(mir, parent);

    FsnpInterfaceRule * fir = dynamic_cast<FsnpInterfaceRule*>(rule);
    if (fir) return new AFsnpInterfaceWidget(fir, parent);

    ASurfaceInterfaceRule * sir = dynamic_cast<ASurfaceInterfaceRule*>(rule);
    if (sir) return new ASurfaceInterfaceWidget(sir, parent);

    AUnifiedRule * uir = dynamic_cast<AUnifiedRule*>(rule);
    if (uir) return new AUnifiedInterfaceWidget(uir, parent);

    AWaveshifterInterfaceRule * wir = dynamic_cast<AWaveshifterInterfaceRule*>(rule);
    if (wir) return new AWaveshifterInterfaceWidget(wir, parent);


    qWarning() << "Unknown interface rule!";
    AInterfaceRuleWidget * f = new AInterfaceRuleWidget(parent);
    f->setFrameStyle(QFrame::Box);
    f->setMinimumHeight(100);
    return f;
}

ABasicInterfaceWidget::ABasicInterfaceWidget(ABasicInterfaceRule * rule, QWidget * parent) :
    AInterfaceRuleWidget(parent)
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

AMetalInterfaceWidget::AMetalInterfaceWidget(AMetalInterfaceRule * rule, QWidget * parent) :
    AInterfaceRuleWidget(parent)
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

AFsnpInterfaceWidget::AFsnpInterfaceWidget(FsnpInterfaceRule * rule, QWidget * parent) :
    AInterfaceRuleWidget(parent)
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

#include <QCheckBox>
AWaveshifterInterfaceWidget::AWaveshifterInterfaceWidget(AWaveshifterInterfaceRule * rule, QWidget * parent) :
    AInterfaceRuleWidget(parent), Rule(rule)
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
        QCheckBox * cbConserveEnergy = new QCheckBox("Conserve energy");
        cbConserveEnergy->setChecked(rule->ConserveEnergy);
        QObject::connect(cbConserveEnergy, &QCheckBox::clicked, [rule](bool checked) { rule->ConserveEnergy = checked; } );
    vl->addWidget(cbConserveEnergy);
        lab = new QLabel("If simulation is NOT wavelength-resolved, this rule does nothing!");
        lab->setAlignment(Qt::AlignCenter);
    vl->addWidget(lab);
    updateButtons();
}

void AWaveshifterInterfaceWidget::loadReemissionProbability()
{
    QString fileName = guitools::dialogLoadFile(Parent, "Load reemission probability", "Data files (*.dat *.txt);;All files (*)");
    if (fileName.isEmpty()) return;

    QString err = Rule->loadReemissionProbability(fileName);
    if (!err.isEmpty()) guitools::message(err, Parent);

    updateButtons();
}

void AWaveshifterInterfaceWidget::loadEmissionSpectrum()
{
    QString fileName = guitools::dialogLoadFile(Parent, "Load emission spectrum", "Data files (*.dat *.txt);;All files (*)");
    if (fileName.isEmpty()) return;

    QString err = Rule->loadEmissionSpectrum(fileName);
    if (!err.isEmpty()) guitools::message(err, Parent);

    updateButtons();
}

void AWaveshifterInterfaceWidget::showReemissionProbability()
{
    if (Rule->ReemissionProbability.empty())
    {
        guitools::message("No data were loaded", Parent);
        return;
    }

    TGraph * gr = AGraphBuilder::graph(Rule->ReemissionProbability);
    AGraphBuilder::configure(gr, "Reemission probability", "Wavelength, nm", "Reemission probability", 2, 20, 1, 2, 2);
    gr->SetMinimum(0);
    emit requestDraw(gr, "apl", true, true);
}

void AWaveshifterInterfaceWidget::showEmissionSpectrum()
{
    if (Rule->EmissionSpectrum.empty())
    {
        guitools::message("No data were loaded", Parent);
        return;
    }

    TGraph * gr = AGraphBuilder::graph(Rule->EmissionSpectrum);
    AGraphBuilder::configure(gr, "Emission spectrum", "Wavelength, nm", "Relative intensity, a.u.", 4, 20, 1, 4, 2);
    gr->SetMinimum(0);
    emit requestDraw(gr, "apl", true, true);
}

void AWaveshifterInterfaceWidget::showBinnedReemissionProbability()
{
    Rule->initializeWaveResolved();

    const AWaveResSettings & WaveSet = APhotonSimHub::getConstInstance().Settings.WaveSet;
    if (!WaveSet.Enabled)
    {
        guitools::message("Simulation is NOT wavelength resolved, override is inactive!", Parent);
        return;
    }

    std::vector<double> waveIndexes = WaveSet.getVectorOfIndexes();
    TGraph * gr = AGraphBuilder::graph(waveIndexes, Rule->ReemissionProbabilityBinned);
    AGraphBuilder::configure(gr, "BinnedReemissionProbability", "Wave index", "Reemission probability, a.u.",
                                              2, 20, 1,
                                              2, 2);
    gr->SetMinimum(0);
    emit requestDraw(gr, "apl", true, true);
}

void AWaveshifterInterfaceWidget::showBinnedEmissionSpectrum()
{
    Rule->initializeWaveResolved();

    const AWaveResSettings & WaveSet = APhotonSimHub::getConstInstance().Settings.WaveSet;
    if (!WaveSet.Enabled)
    {
        guitools::message("Simulation is NOT wavelength resolved, override is inactive!", Parent);
        return;
    }

    if ( !Rule->Spectrum ) //paranoic
    {
        guitools::message("Reemission spectrum is not defined!", Parent);
        return;
    }

    double integral = Rule->Spectrum->ComputeIntegral();
    if (integral <= 0)
    {
        guitools::message("Binned emission spectrum: integral <= 0, override will report an error!", Parent);
        return;
    }

    TH1D * SpectrumCopy = new TH1D(*Rule->Spectrum);
    SpectrumCopy->SetTitle("Binned emission spectrum");
    SpectrumCopy->GetXaxis()->SetTitle("Wavelength, nm");
    SpectrumCopy->GetYaxis()->SetTitle("Relative intensity, a.u.");
    emit requestDraw(SpectrumCopy, "hist", true, true);
}

void AWaveshifterInterfaceWidget::updateButtons()
{
    pbShowRP->setDisabled(Rule->ReemissionProbability.empty());
    pbShowES->setDisabled(Rule->EmissionSpectrum.empty());
    bool bWR = APhotonSimHub::getConstInstance().Settings.WaveSet.Enabled;
    pbShowRPbinned->setDisabled(!bWR || Rule->ReemissionProbability.empty());
    pbShowESbinned->setDisabled(!bWR || Rule->EmissionSpectrum.empty());
}

// -------------

ASpectralBasicInterfaceWidget::ASpectralBasicInterfaceWidget(ASpectralBasicInterfaceRule * rule, QWidget * parent) :
    AInterfaceRuleWidget(parent), Rule(rule)
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

void ASpectralBasicInterfaceWidget::showLoaded()
{
    std::vector<double> fresnel(Rule->Wave.size());
    double max = 0.01;
    for (size_t i = 0; i < Rule->Wave.size(); i++)
    {
        fresnel[i] = 1.0 - Rule->ProbLoss[i] - Rule->ProbRef[i] - Rule->ProbDiff[i];
        max = std::max({max, Rule->ProbLoss[i],  Rule->ProbRef[i], Rule->ProbDiff[i], fresnel[i]});
    }

    TGraph * gLoss = AGraphBuilder::graph(Rule->Wave, Rule->ProbLoss);
    AGraphBuilder::configure(gLoss, "Absorption", "Wavelength, nm", "Probability", 2, 20, 1, 2);
    gLoss->SetMinimum(0);
    gLoss->SetMaximum(max);
    emit requestDraw(gLoss, "ALP", true, false);

    TGraph * gRef = AGraphBuilder::graph(Rule->Wave, Rule->ProbRef);
    AGraphBuilder::configure(gRef, "Specular reflection", "Wavelength, nm", "Probability", 4, 21, 1, 4);
    emit requestDraw(gRef, "LPsame", true, false);

    TGraph* gDiff = AGraphBuilder::graph(Rule->Wave, Rule->ProbDiff);
    AGraphBuilder::configure(gDiff, "Diffuse scattering", "Wavelength, nm", "Probability", 7, 22, 1, 7);
    emit requestDraw(gDiff, "LPsame", true, false);

    TGraph* gFr = AGraphBuilder::graph(Rule->Wave, fresnel);
    AGraphBuilder::configure(gFr, "Fresnel", "Wavelength, nm", "Probability", 1, 24, 1, 1, 1, 1);
    emit requestDraw(gFr, "LPsame", true, true);

    emit requestDrawLegend(0.7,0.8, 0.95,0.95, "");
}

void ASpectralBasicInterfaceWidget::showBinned()
{
    Rule->initializeWaveResolved();

    const AWaveResSettings & WaveSet = APhotonSimHub::getConstInstance().Settings.WaveSet;
    if (!WaveSet.Enabled)
    {
        QString s =  "Simulation is configured as not wavelength-resolved\n";
        s +=         "All photons will have the same properties:\n";
        s += QString("Absorption: %1").arg(Rule->ProbLossBinned[Rule->effectiveWaveIndex]) + "\n";
        s += QString("Specular reflection: %1").arg(Rule->ProbRefBinned[Rule->effectiveWaveIndex]) + "\n";
        s += QString("Scattering: %1").arg(Rule->ProbDiffBinned[Rule->effectiveWaveIndex]);
        guitools::message(s, Parent);
        return;
    }

    std::vector<double> waveIndexes = WaveSet.getVectorOfIndexes();

    std::vector<double> fresnel(waveIndexes.size());
    double max = 0.01;
    for (size_t i = 0; i < waveIndexes.size(); i++)
    {
        fresnel[i] = 1.0 - Rule->ProbLossBinned[i] - Rule->ProbRefBinned[i] - Rule->ProbDiffBinned[i];
        max = std::max({max, Rule->ProbLossBinned[i],  Rule->ProbRefBinned[i], Rule->ProbDiffBinned[i], fresnel[i]});
    }

    TGraph * gLoss = AGraphBuilder::graph(waveIndexes, Rule->ProbLossBinned);
    AGraphBuilder::configure(gLoss, "Absorption", "WaveIndex", "Probability", 2, 20, 1, 2);
    gLoss->SetMinimum(0);
    gLoss->SetMaximum(max);
    emit requestDraw(gLoss, "AP", true, false);

    TGraph * gRef = AGraphBuilder::graph(waveIndexes, Rule->ProbRefBinned);
    AGraphBuilder::configure(gRef, "Specular reflection", "WaveIndex", "Probability", 4, 21, 1, 4);
    emit requestDraw(gRef, "Psame", true, false);

    TGraph* gDiff = AGraphBuilder::graph(waveIndexes, Rule->ProbDiffBinned);
    AGraphBuilder::configure(gDiff, "Diffuse scattering", "WaveIndex", "Probability", 7, 22, 1, 7);
    emit requestDraw(gDiff, "Psame", true, false);

    TGraph* gFr = AGraphBuilder::graph(waveIndexes, fresnel);
    AGraphBuilder::configure(gFr, "Fresnel", "WaveIndex", "Probability", 1, 24, 1, 1, 1, 1);
    emit requestDraw(gFr, "Psame", true, true);

    emit requestDrawLegend(0.7,0.8, 0.95,0.95, "");
}

void ASpectralBasicInterfaceWidget::updateButtons()
{
    pbShow->setDisabled(Rule->Wave.empty());
}

// -------

ASurfaceInterfaceWidget::ASurfaceInterfaceWidget(ASurfaceInterfaceRule * rule, QWidget * parent) :
    AInterfaceRuleWidget(parent)
{
    setFrameStyle(QFrame::Box);

    QHBoxLayout* l = new QHBoxLayout(this);
        QLabel* lab = new QLabel("Using Fresnel equations and Snell's law");
        lab->setAlignment(Qt::AlignHCenter);
    l->addWidget(lab);
}

AUnifiedInterfaceWidget::AUnifiedInterfaceWidget(AUnifiedRule * rule, QWidget * parent) :
    AInterfaceRuleWidget(parent)
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
