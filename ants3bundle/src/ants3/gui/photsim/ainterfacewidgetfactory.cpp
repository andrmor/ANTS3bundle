#include "ainterfacewidgetfactory.h"
#include "TH2.h"
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
//#include "agraphwindow.h"
#include "alutinterfacerule.h"

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

    ALutInterfaceRule * lir = dynamic_cast<ALutInterfaceRule*>(rule);
    if (lir) return new ALUTInterfaceWidget(lir, parent);


    qWarning() << "Unknown interface rule!";
    AInterfaceRuleWidget * f = new AInterfaceRuleWidget(parent);
    //f->setFrameStyle(QFrame::Box);
    //f->setFrameStyle(QFrame::StyledPanel);
    f->setMinimumHeight(100);
    return f;
}

ABasicInterfaceWidget::ABasicInterfaceWidget(ABasicInterfaceRule * rule, QWidget * parent) :
    AInterfaceRuleWidget(parent)
{
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
    QHBoxLayout* l = new QHBoxLayout(this);
        QLabel* lab = new QLabel("Using Fresnel equations and Snell's law");
        lab->setAlignment(Qt::AlignHCenter);
    l->addWidget(lab);
}

AUnifiedInterfaceWidget::AUnifiedInterfaceWidget(AUnifiedRule * rule, QWidget * parent) :
    AInterfaceRuleWidget(parent)
{
    QVBoxLayout * mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0,0,0,0);

    QGridLayout * lay = new QGridLayout();

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

    mainLay->addLayout(lay);

    QHBoxLayout * hLay = new QHBoxLayout();
        hLay->setContentsMargins(0,0,0,0);
        QCheckBox * cbSkip = new QCheckBox("Override Fresnel:"); cbSkip->setChecked(rule->SkipFresnel);
        QObject::connect(cbSkip, &QCheckBox::clicked, [rule](bool checked) { rule->SkipFresnel = checked; } );
        hLay->addWidget(cbSkip);
        hLay->addWidget(new QLabel("reflection:"));
        QLineEdit * leRef = new QLineEdit(QString::number(rule->ReflectionOverride)); leRef->setValidator(val);
        hLay->addWidget(leRef);
        hLay->addWidget(new QLabel("absorption:"));
        QLineEdit * leAbs = new QLineEdit(QString::number(rule->AbsorptionOverride)); leAbs->setValidator(val);
        hLay->addWidget(leAbs);
        QObject::connect(leRef, &QLineEdit::editingFinished, [leRef, rule]() { rule->ReflectionOverride = leRef->text().toDouble(); } );
        QObject::connect(leAbs, &QLineEdit::editingFinished, [leAbs, rule]() { rule->AbsorptionOverride = leAbs->text().toDouble(); } );
        QObject::connect(cbSkip, &QCheckBox::toggled, leRef, &QLineEdit::setEnabled);
        QObject::connect(cbSkip, &QCheckBox::toggled, leAbs, &QLineEdit::setEnabled);
        if (!cbSkip->isChecked())
        {
            leRef->setEnabled(false);
            leAbs->setEnabled(false);
        }
    mainLay->addLayout(hLay);
}

// --------------

ALUTInterfaceWidget::ALUTInterfaceWidget(ALutInterfaceRule * rule, QWidget * parent) :
    AInterfaceRuleWidget(parent), Rule(rule)
{
    QVBoxLayout * mainLay = new QVBoxLayout(this);
        QHBoxLayout * lay = new QHBoxLayout();
            QPushButton * pb = new QPushButton("Load LUT");
        lay->addWidget(pb);
            labInfo = new QLabel("");
            labInfo->setAlignment(Qt::AlignHCenter);
        lay->addWidget(labInfo);
    mainLay->addLayout(lay);
        frShow = new QFrame();
        QVBoxLayout * layV = new QVBoxLayout(frShow);
            QHBoxLayout * lay2 = new QHBoxLayout();
            lay2->setContentsMargins(0,0,0,0);
            lay2->addWidget(new QLabel("Show:"));
            pbShowRef = new QPushButton("Reflection");
            lay2->addWidget(pbShowRef);
            lay2->addWidget(new QLabel("or"));
            pbShowTrans = new QPushButton("Transmission");
            lay2->addWidget(pbShowTrans);
            lay2->addWidget(new QLabel("for"));
            cobAngles = new QComboBox();
            lay2->addWidget(cobAngles);
            lay2->addWidget(new QLabel("deg incidence"));
        layV->addLayout(lay2);
            pbShowAbs = new QPushButton("Show fractions vs incidence angle");
        layV->addWidget(pbShowAbs, 0, Qt::AlignHCenter);

    mainLay->addWidget(frShow);

    QObject::connect(pb,          &QPushButton::clicked, this, &ALUTInterfaceWidget::onLoadLutPressed);
    QObject::connect(pbShowRef  , &QPushButton::clicked, this, &ALUTInterfaceWidget::onShowReflectionPressed);
    QObject::connect(pbShowTrans, &QPushButton::clicked, this, &ALUTInterfaceWidget::onShowTransmittedPressed);
    QObject::connect(pbShowAbs,   &QPushButton::clicked, this, &ALUTInterfaceWidget::onShowProbabilitiesPressed);

    updateLutGui();
}

void ALUTInterfaceWidget::updateLutGui()
{
    bool bHaveData = false;

    if (Rule->DataReflection.empty() && Rule->DataTransmission.empty())
        labInfo->setText("LUT not loaded");
    else
    {
        bHaveData = true;
        int size = Rule->DataReflection.size();
        if (size == 0) size = Rule->DataTransmission.size();
        QString txt = QString("Angle of inceidence bins: %0").arg(size);
        labInfo->setText(txt);

        pbShowRef->  setEnabled(!Rule->DataReflection.empty());
        pbShowTrans->setEnabled(!Rule->DataTransmission.empty());
        pbShowAbs->  setEnabled(!Rule->DataAbsorption.empty());

        cobAngles->clear();
        for (const auto & pair : (Rule->DataReflection.empty() ? Rule->DataTransmission : Rule->DataReflection))
            cobAngles->addItem(QString::number(pair.first));
    }

    frShow->setEnabled(bHaveData);
}

#include "ajsontools.h"
void ALUTInterfaceWidget::onLoadLutPressed()
{
    QString fileName = guitools::dialogLoadFile(this, "Load LUT json file", "Json files (*.json);;All files (*.*)");
    if (fileName.isEmpty()) return;

    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, fileName);
    if (!ok) guitools::message("Failed to read json from file!", this);
    else
    {
        QString err = Rule->loadLUT(json);
        if (!err.isEmpty()) guitools::message(err, this);
    }
    updateLutGui();
}

void ALUTInterfaceWidget::onShowReflectionPressed()
{
    //showMesh(true);
    //showMeshNiceButSlow(true);
    showMeshNiceAndFast(true);
}

void ALUTInterfaceWidget::onShowTransmittedPressed()
{
    //showMesh(false);
    //showMeshNiceButSlow(false);
    showMeshNiceAndFast(false);
}

#include "TGraph.h"
void ALUTInterfaceWidget::onShowProbabilitiesPressed()
{
    std::vector<double> angles;

    std::vector<double> reflected;
    for (const std::pair<double,std::vector<double>> & pair : Rule->DataReflection)
    {
        angles.push_back(pair.first);
        double sum = std::reduce(pair.second.begin(), pair.second.end(), 0.0);
        reflected.push_back(sum);
    }
    bool bHaveRef = !reflected.empty();

    std::vector<double> transmitted;
    for (const std::pair<double,std::vector<double>> & pair : Rule->DataTransmission)
    {
        if (!bHaveRef) angles.push_back(pair.first);
        double sum = std::reduce(pair.second.begin(), pair.second.end(), 0.0);
        transmitted.push_back(sum);
    }
    bool bHaveTrans = !transmitted.empty();

    std::vector<double> absorbed;
    for (const std::pair<double,double> & pair : Rule->DataAbsorption)
        absorbed.push_back(pair.second);
    bool bHaveAbs = !absorbed.empty();

    for (size_t i = 0; i < angles.size(); i++)
    {
        double sum = 0;
        if (bHaveAbs)   sum += absorbed[i];
        if (bHaveRef)   sum += reflected[i];
        if (bHaveTrans) sum += transmitted[i];
        if (sum == 0) continue;

        if (bHaveAbs)   absorbed[i]    /= sum;
        if (bHaveRef)   reflected[i]   /= sum;
        if (bHaveTrans) transmitted[i] /= sum;
    }

    if (!bHaveRef)   reflected   = std::vector<double>(angles.size(), 0);
    if (!bHaveTrans) transmitted = std::vector<double>(angles.size(), 0);
    if (!bHaveAbs)   absorbed    = std::vector<double>(angles.size(), 0);

    TGraph * gR = AGraphBuilder::graph(angles, reflected);   AGraphBuilder::configure(gR, "Reflection",   "Angle of incidence, deg", "Fraction",   3, 0, 1,   3, 1, 2);
    TGraph * gT = AGraphBuilder::graph(angles, transmitted); AGraphBuilder::configure(gT, "Transmission", "Angle of incidence, deg", "Fraction",   4, 0, 1,   4, 1, 2);
    TGraph * gA = AGraphBuilder::graph(angles, absorbed);    AGraphBuilder::configure(gA, "Absorption",   "Angle of incidence, deg", "Fraction",   1, 0, 1,   1, 1, 2);

    gR->SetMinimum(0);
    gR->SetMaximum(1.05);

    emit requestDraw(gR, "AL",    true, false);
    emit requestDraw(gT, "Lsame", true, false);
    emit requestDraw(gA, "Lsame", true, true);

    emit requestDrawLegend(0.12,0.12, 0.4,0.25, "");
}

#include "ageomeshhandler.h"
#include "TView.h"
#include "TPolyLine3D.h"
#include "TPolyMarker3D.h"
#include "TGraph2D.h"
#include "TColor.h"
#include "TMath.h"
void ALUTInterfaceWidget::showMesh(bool reflection)
{
    QString err = Rule->check();
    if (!err.isEmpty()) return;

    AGeoMeshHandler * mesh = Rule->getTransMesh();

    std::vector<AGeoMeshHandler::Vec3> & vertices = mesh->vertices;
    std::vector<AGeoMeshHandler::Triangle> triangles = mesh->triangles;

    std::vector<double> & Data = reflection
        ? Rule->DataReflection  [cobAngles->currentIndex()].second
        : Rule->DataTransmission[cobAngles->currentIndex()].second;

    TGraph2D * baseGraph = new TGraph2D();
    baseGraph->AddPoint(0,0,1);
    baseGraph->AddPoint(1,0,0);
    baseGraph->AddPoint(0,1,0);
    baseGraph->AddPoint(-1,0,0);
    baseGraph->AddPoint(0,-1,0);
    baseGraph->SetMarkerStyle(1);
    //baseGraph->SetMarkerColor(kWhite);
    baseGraph->SetMinimum(0);
    baseGraph->SetMaximum(1);
    baseGraph->GetXaxis()->SetLimits(-1,1);
    baseGraph->GetYaxis()->SetLimits(-1,1);
    baseGraph->GetXaxis()->SetLabelSize(0);
    baseGraph->GetYaxis()->SetLabelSize(0);
    baseGraph->GetZaxis()->SetLabelSize(0);
    baseGraph->GetXaxis()->SetTitleSize(0);
    baseGraph->GetYaxis()->SetTitleSize(0);
    baseGraph->GetZaxis()->SetTitleSize(0);
    baseGraph->GetXaxis()->SetTickLength(0);
    baseGraph->GetYaxis()->SetTickLength(0);
    baseGraph->GetZaxis()->SetTickLength(0);
    emit requestDraw(baseGraph, "P", true, true);

    TPolyLine3D *line3d = new TPolyLine3D(2);
    line3d->SetPoint(0, -1.0, 0.0, 1); // Start coordinates (x, y, z)
    line3d->SetPoint(1, 0, 0, 0); // End coordinates (x, y, z)
    line3d->SetLineColor(kBlue);
    line3d->SetLineWidth(4);
    emit requestDraw(line3d, "same", true, false);

    TList * meshList = new TList();

    int numColors = TColor::GetNumberOfColors();
    auto minMax = std::minmax_element(Data.begin(), Data.end());
    double valMin = *minMax.first;
    double valMax = *minMax.second;
    double valRange = (valMax == valMin) ? 1.0 : (valMax - valMin);

    for (size_t i = 0; i < triangles.size(); ++i)
    {
        double normVal = (Data[i] - valMin) / valRange;
        int colorIdx = TColor::GetColorPalette(normVal * (numColors - 1));

        const auto& v0 = vertices[triangles[i][0]];
        const auto& v1 = vertices[triangles[i][1]];
        const auto& v2 = vertices[triangles[i][2]];

        double x[4] = {v0[0], v1[0], v2[0], v0[0]};
        double y[4] = {v0[1], v1[1], v2[1], v0[1]};
        double z[4] = {v0[2], v1[2], v2[2], v0[2]};

        TPolyLine3D *poly = new TPolyLine3D(4, x, y, z);
        poly->SetLineColor(colorIdx); // When filled, LineColor acts as the brush color for some 3D viewers
        poly->SetLineWidth(1);

        meshList->Add(poly);
    }

    emit requestDraw(meshList, "fsame", true, true);
}

void ALUTInterfaceWidget::showMeshNiceButSlow(bool reflection)
{
    QString err = Rule->check();
    if (!err.isEmpty()) return;

    AGeoMeshHandler * mesh = Rule->getTransMesh();

    std::vector<AGeoMeshHandler::Vec3> & vertices = mesh->vertices;
    std::vector<AGeoMeshHandler::Triangle> triangles = mesh->triangles;

    std::vector<double> & Data = reflection
                                    ? Rule->DataReflection  [cobAngles->currentIndex()].second
                                    : Rule->DataTransmission[cobAngles->currentIndex()].second;


    const int nPointsPerTriangle = 5;
    const int numColors = TColor::GetNumberOfColors();

    auto minMax = std::minmax_element(Data.begin(), Data.end());
    double valMin = *minMax.first;
    double valMax = *minMax.second;
    double valRange = (valMax == valMin) ? 1.0 : (valMax - valMin);

    // FIX: Keep vectors strictly sized to numColors (0 to numColors - 1)
    std::vector<std::vector<double>> colorX(numColors);
    std::vector<std::vector<double>> colorY(numColors);
    std::vector<std::vector<double>> colorZ(numColors);

    // 2. Loop over all custom triangles to populate points uniformly
    for (size_t i = 0; i < triangles.size(); ++i)
    {
        double normVal = (Data[i] - valMin) / valRange;

        // Safe protection against floating-point rounding exceeding 1.0
        if (normVal < 0.0) normVal = 0.0;
        if (normVal > 1.0) normVal = 1.0;

        // FIX: Calculate a local index from 0 to numColors-1 for our tracking vectors
        int paletteBin = static_cast<int>(normVal * (numColors - 1));

        // Pull coordinates for the 3 vertices defining this face
        const auto& p0 = vertices[triangles[i][0]];
        const auto& p1 = vertices[triangles[i][1]];
        const auto& p2 = vertices[triangles[i][2]];

        // Approximate a grid that will sum up roughly to nPointsPerTriangle
        int steps = std::max(2, (int)std::sqrt(2 * nPointsPerTriangle));

        for (int u = 0; u <= steps; ++u)
        {
            for (int v = 0; v <= steps - u; ++v)
            {
                double w0 = (double)u / steps;
                double w1 = (double)v / steps;
                double w2 = 1.0 - w0 - w1;

                // Compute exact internal coordinate position
                double px = w0 * p0[0] + w1 * p1[0] + w2 * p2[0];
                double py = w0 * p0[1] + w1 * p1[1] + w2 * p2[1];
                double pz = w0 * p0[2] + w1 * p1[2] + w2 * p2[2];

                // Safely push back using the guaranteed 0-bounded local index
                colorX[paletteBin].push_back(px);
                colorY[paletteBin].push_back(py);
                colorZ[paletteBin].push_back(pz);
            }
        }
    }

    // 3. Create ONE foundational Graph layer to initialize the 3D frame & viewport
    TGraph2D *baseGraph = new TGraph2D(vertices.size());
    for (size_t i = 0; i < vertices.size(); ++i)
        baseGraph->SetPoint(i, vertices[i][0], vertices[i][1], vertices[i][2]);
    baseGraph->SetMarkerStyle(1);
    baseGraph->SetMarkerColor(kWhite);
    emit requestDraw(baseGraph, "P", true, true);

    // 4. Draw the sampled point clouds on top ("same")
    for (int bin = 0; bin < numColors; ++bin)
    {
        int nPointsInColor = colorX[bin].size();
        if (nPointsInColor == 0) continue;

        TPolyMarker3D * pm3d = new TPolyMarker3D(nPointsInColor);
        for (int p = 0; p < nPointsInColor; ++p) {
            pm3d->SetPoint(p, colorX[bin][p], colorY[bin][p], colorZ[bin][p]);
        }

        // FIX: Translate the safe local loop index back to ROOT's global color unique ID
        int actualRootColorIdx = TColor::GetColorPalette(bin);

        pm3d->SetMarkerColor(actualRootColorIdx);
        pm3d->SetMarkerStyle(20); // Solid circular dot
        pm3d->SetMarkerSize(0.6);

        //pm3d->Draw("same");
        emit requestDraw(pm3d, "same", true, false);
    }
}


#include "apersistentutils3d.h"
/*
class AMyPolyMarker3D : public TPolyMarker3D
{
public:
    AMyPolyMarker3D(int n) : TPolyMarker3D(n) {ResetBit(kCanDelete);}
    AMyPolyMarker3D() : TPolyMarker3D(n) {ResetBit(kCanDelete);}
    ~AMyPolyMarker3D(){qDebug() << "---++--AMyPolyMarker3D:destr--++---";}

    virtual TObject * Clone(const char * newname = "") const {TObject * clone = TPolyMarker3D::Clone(newname); clone->ResetBit(kCanDelete); return clone;}
};
*/

void ALUTInterfaceWidget::showMeshNiceAndFast(bool reflection)
{
    QString err = Rule->check();
    if (!err.isEmpty()) return;

    AGeoMeshHandler * mesh = Rule->getTransMesh();

    std::vector<AGeoMeshHandler::Vec3> & vertices = mesh->vertices;
    std::vector<AGeoMeshHandler::Triangle> triangles = mesh->triangles;

    std::vector<double> & Data = reflection
                                    ? Rule->DataReflection  [cobAngles->currentIndex()].second
                                    : Rule->DataTransmission[cobAngles->currentIndex()].second;


    const int nPointsPerTriangle = 5;
    const int numColors = TColor::GetNumberOfColors();

    auto minMax = std::minmax_element(Data.begin(), Data.end());
    double valMin = *minMax.first;
    double valMax = *minMax.second;
    double valRange = (valMax == valMin) ? 1.0 : (valMax - valMin);

    // FIX: Keep vectors strictly sized to numColors (0 to numColors - 1)
    std::vector<std::vector<double>> colorX(numColors);
    std::vector<std::vector<double>> colorY(numColors);
    std::vector<std::vector<double>> colorZ(numColors);

    // 2. Loop over all custom triangles to populate points uniformly
    for (size_t i = 0; i < triangles.size(); ++i)
    {
        double normVal = (Data[i] - valMin) / valRange;

        // Safe protection against floating-point rounding exceeding 1.0
        if (normVal < 0.0) normVal = 0.0;
        if (normVal > 1.0) normVal = 1.0;

        // FIX: Calculate a local index from 0 to numColors-1 for our tracking vectors
        int paletteBin = static_cast<int>(normVal * (numColors - 1));

        // Pull coordinates for the 3 vertices defining this face
        const auto& p0 = vertices[triangles[i][0]];
        const auto& p1 = vertices[triangles[i][1]];
        const auto& p2 = vertices[triangles[i][2]];

        // Approximate a grid that will sum up roughly to nPointsPerTriangle
        int steps = std::max(2, (int)std::sqrt(2 * nPointsPerTriangle));

        for (int u = 0; u <= steps; ++u)
        {
            for (int v = 0; v <= steps - u; ++v)
            {
                double w0 = (double)u / steps;
                double w1 = (double)v / steps;
                double w2 = 1.0 - w0 - w1;

                // Compute exact internal coordinate position
                double px = w0 * p0[0] + w1 * p1[0] + w2 * p2[0];
                double py = w0 * p0[1] + w1 * p1[1] + w2 * p2[1];
                double pz = w0 * p0[2] + w1 * p1[2] + w2 * p2[2];

                // Safely push back using the guaranteed 0-bounded local index
                colorX[paletteBin].push_back(px);
                colorY[paletteBin].push_back(py);
                colorZ[paletteBin].push_back(pz);
            }
        }
    }

    /*
    // 3. Create ONE foundational Graph layer to initialize the 3D frame & viewport
    TGraph2D *baseGraph = new TGraph2D(vertices.size());
    for (size_t i = 0; i < vertices.size(); ++i)
        baseGraph->SetPoint(i, vertices[i][0], vertices[i][1], vertices[i][2]);
    baseGraph->SetMarkerStyle(1);
    baseGraph->SetMarkerColor(kWhite);
    emit requestDraw(baseGraph, "P", true, true);
    */
    drawBaseGraph();


    // 4. Draw the sampled point clouds on top ("same")
    TList * meshList = new TList();
    for (int bin = 0; bin < numColors; ++bin)
    {
        int nPointsInColor = colorX[bin].size();
        if (nPointsInColor == 0) continue;

        //TPolyMarker3D * pm3d = new TPolyMarker3D(nPointsInColor);
        //AMyPolyMarker3D * pm3d = new AMyPolyMarker3D(nPointsInColor);  pm3d->ResetBit(kCanDelete);
        APersistentPolymarker3D * pm3d = new APersistentPolymarker3D(nPointsInColor);
        for (int p = 0; p < nPointsInColor; ++p)
            pm3d->SetPoint(p, colorX[bin][p], colorY[bin][p], colorZ[bin][p]);

        // FIX: Translate the safe local loop index back to ROOT's global color unique ID
        int actualRootColorIdx = TColor::GetColorPalette(bin);

        pm3d->SetMarkerColor(actualRootColorIdx);
        pm3d->SetMarkerStyle(20); // Solid circular dot
        pm3d->SetMarkerSize(0.6);

        //pm3d->Draw("same");
        meshList->Add(pm3d);
    }
    emit requestDraw(meshList, "same", true, false);

    drawDirectionLine(cobAngles->currentText().toDouble(), reflection);
    drawSurfaceCircle(100, 0.1);
}

/*
class AMyPolyLine3D : public TPolyLine3D
{
public:
    AMyPolyLine3D(int n) : TPolyLine3D(n) {ResetBit(kCanDelete);}
    ~AMyPolyLine3D(){qDebug() << "-------AMyPolyLine3D:destr-------";}

    virtual TObject * Clone(const char * newname = "") const {TObject * clone = TPolyLine3D::Clone(newname); clone->ResetBit(kCanDelete); return clone;}
};
*/

void ALUTInterfaceWidget::drawDirectionLine(double angle, bool reflection)
{
    angle *= 3.1415926535/180.0;
    double factor = (reflection ? 1.2 : 0.6);

    //AMyPolyLine3D * line3d = new AMyPolyLine3D(2); line3d->ResetBit(kCanDelete);
    APersistentPolyLine3D * line3d = new APersistentPolyLine3D(2);
    line3d->SetPoint(0, -factor*sin(angle), 0.0, (reflection ? 1.0 : -1.0) * factor*cos(angle));
    line3d->SetPoint(1, 0, 0, 0);
    line3d->SetLineColor(kRed);
    line3d->SetLineWidth(4);
    emit requestDraw(line3d, "same", true, false);

    //AMyPolyLine3D * line3dout = new AMyPolyLine3D(2); line3dout->ResetBit(kCanDelete);
    APersistentPolyLine3D * line3dout = new APersistentPolyLine3D(2);
    line3dout->SetPoint(0, 1.2*sin(angle), 0.0, 1.2*cos(angle));
    line3dout->SetPoint(1, 0, 0, 0);
    line3dout->SetLineColor(kRed);
    line3dout->SetLineStyle(2);
    line3dout->SetLineWidth(2);
    emit requestDraw(line3dout, "same", true, false);
}

void ALUTInterfaceWidget::drawSurfaceCircle(int nPoints, double radius)
{
    APersistentPolyLine3D * circle = new APersistentPolyLine3D(nPoints + 1);

    for (int i = 0; i <= nPoints; ++i)
    {
        double theta = 2.0 * 3.1415926535 * i / nPoints;
        double x = radius * std::cos(theta);
        double y = radius * std::sin(theta);
        double z = 0.0;

        circle->SetPoint(i, x, y, z);
    }

    circle->SetLineColor(kBlack);
    circle->SetLineWidth(2);

    emit requestDraw(circle, "same", true, false);
}

void ALUTInterfaceWidget::drawBaseGraph()
{
    TGraph2D * g = new TGraph2D();
    g->AddPoint(1,1,1);
    g->AddPoint(-1,1,1);
    g->AddPoint(-1,-1,-1);
    g->AddPoint(1,-1,-1);

    g->SetMinimum(0);
    g->SetMaximum(1.1);

    //g->GetHistogram()->GetXaxis()->SetNdivisions(101, false); // copy to basket will forget this

    emit requestDraw(g, "P", true, true);
}
