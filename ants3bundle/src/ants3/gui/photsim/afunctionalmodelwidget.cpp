#include "afunctionalmodelwidget.h"
#include "aphotonsimhub.h"
#include "afiletools.h"
#include "guitools.h"
#include "agraphbuilder.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QPushButton>
#include "QCheckBox"

#include "TGraph.h"

AFunctionalModelWidget::AFunctionalModelWidget(const APhotonFunctionalModel * model, QWidget * parent) :
    QFrame(parent)
{
    MainLayout = new QVBoxLayout(this);
    MainLayout->setContentsMargins(3,0,3,0);

    DoubleValidator = new QDoubleValidator(this);
}

AFunctionalModelWidget * AFunctionalModelWidget::factory(const APhotonFunctionalModel * model, QWidget * parent)
{
    if (!model)
    {
        qWarning() << "Empty photon functional model in widget factory!";
        return new AFunctionalModelWidget_Dummy(model, parent);
    }

    const APFM_Dummy * dm = dynamic_cast<const APFM_Dummy*>(model);
    if (dm) return new AFunctionalModelWidget_Dummy(model, parent);

    const APFM_ThinLens * tlm = dynamic_cast<const APFM_ThinLens*>(model);
    if (tlm) return new AFunctionalModelWidget_ThinLens(tlm, parent);

    const APFM_OpticalFiber * ofm = dynamic_cast<const APFM_OpticalFiber*>(model);
    if (ofm) return new AFunctionalModelWidget_OpticalFiber(ofm, parent);

    const APFM_Filter * fm = dynamic_cast<const APFM_Filter*>(model);
    if (fm) return new AFunctionalModelWidget_Filter(fm, parent);

    qWarning() << "Unknown photon functional model in widget factory:" << model->getType();
    return new AFunctionalModelWidget_Dummy(model, parent);
}

QString AFunctionalModelWidget::getModelDatabase()
{
    QString txt = ""
                         "* Basic optics\n"
                         "@Filter Gray or configurable bandpass optical filter. #Filter\n"
                         "@ThinLens Ideal thin lens firth positive or negative focal length. #Lens\n"
                         "* Tunnels\n"
                         "@OpticalFiber Simplified model of an optical fiber. #Fiber";
    return txt;
}

// ---

AFunctionalModelWidget_Dummy::AFunctionalModelWidget_Dummy(const APhotonFunctionalModel * model, QWidget * parent) :
    AFunctionalModelWidget(model, parent) {}

// ---

AFunctionalModelWidget_ThinLens::AFunctionalModelWidget_ThinLens(const APFM_ThinLens * model, QWidget * parent) :
    AFunctionalModelWidget(model, parent)
{
    QHBoxLayout * lay = new QHBoxLayout(); lay->setContentsMargins(3,0,3,0);

    lay->addWidget( new QLabel(QString("Focal length for not %0-resolved sim:").arg(QChar(0x3bb))) );
    leFocalLength = new QLineEdit(); leFocalLength->setValidator(DoubleValidator);
    connect(leFocalLength, &QLineEdit::editingFinished, this, &AFunctionalModelWidget_ThinLens::modified);
    lay->addWidget(leFocalLength);
    lay->addWidget(new QLabel("mm"));
    lay->addStretch(2);
    MainLayout->addLayout(lay);

    lay = new QHBoxLayout(); lay->setContentsMargins(3,0,3,0);
    lay->addWidget( new QLabel(QString("    Focal length vs %0:").arg(QChar(0x3bb))) );

    pbShow = new QPushButton("Show", this);
    connect(pbShow, &QPushButton::clicked, this, &AFunctionalModelWidget_ThinLens::onShowClicked);
    pbShow->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(pbShow, &QPushButton::customContextMenuRequested, this, &AFunctionalModelWidget_ThinLens::onShowRightClicked);
    lay->addWidget(pbShow);
    pbLoad = new QPushButton("Load", this);
    connect(pbLoad, &QPushButton::clicked, this, &AFunctionalModelWidget_ThinLens::onLoadClicked);
    lay->addWidget(pbLoad);
    pbDelete = new QPushButton("X", this); pbDelete->setMaximumWidth(25);
    connect(pbDelete, &QPushButton::clicked, this, &AFunctionalModelWidget_ThinLens::onDeleteClicked);
    lay->addWidget(pbDelete);
    lay->addStretch(1);
    MainLayout->addLayout(lay);

    leFocalLength->setText(QString::number(model->FocalLength_mm));
    Spectrum = model->FocalLengthSpectrum_mm;
    updateButtons();
}

void AFunctionalModelWidget_ThinLens::updateButtons()
{
    bool bHaveSpectrum = (!Spectrum.empty());

    pbShow->setEnabled(bHaveSpectrum);
    pbDelete->setEnabled(bHaveSpectrum);
}

void AFunctionalModelWidget_ThinLens::onLoadClicked()
{
    QString fileName = guitools::dialogLoadFile(this, "Load file with two columns: wavelength[nm] FocalLength_mm", "Data files (*.txt *.dat); All files (*.*)");
    if (fileName.isEmpty()) return;

    std::vector<std::pair<double,double>> tmp;
    QString err = ftools::loadPairs(fileName, tmp, true);
    if (!err.isEmpty()) guitools::message(err, this);
    else
    {
        for (const auto & p : tmp)
        {
            if (p.second < 1e-60)
            {
                guitools::message("Focal length values should be positive!", this);
                return;
            }
        }

        Spectrum = tmp;
        updateButtons();
        emit modified();
    }
}

void AFunctionalModelWidget_ThinLens::onShowClicked()
{
    if (Spectrum.empty()) return;

    TGraph * g = AGraphBuilder::graph(Spectrum);
    AGraphBuilder::configure(g, "Focal length vs wavelength",
                             "Wavelength, nm", "Focal length, mm",
                             2, 20, 1,
                             2, 1,  1);
    emit requestDraw(g, "APL", true, true);
}

void AFunctionalModelWidget_ThinLens::onShowRightClicked(const QPoint &)
{
    const AWaveResSettings & WaveSet = APhotonSimHub::getInstance().Settings.WaveSet;
    if (!WaveSet.Enabled)
    {
        guitools::message("Simulation is currently configured not to be wavelength-resolved!", this);
        return;
    }

    APFM_ThinLens tmpMod;
    tmpMod.FocalLengthSpectrum_mm = Spectrum;
    QString err = tmpMod.updateRuntimeProperties();
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    if (tmpMod._FocalLengthBinned.empty())
    {
        guitools::message("Wavelength-resolved binned data are empty!", this);
        return;
    }
    else
    {
        std::vector<double> wavelength;
        WaveSet.getWavelengthBins(wavelength);
        TGraph * g = AGraphBuilder::graph(wavelength, tmpMod._FocalLengthBinned);
        AGraphBuilder::configure(g, "Binned focal length vs wavelength",
                                 "Wavelength, nm", "Focal length, mm",
                                 4, 20, 1,
                                 4, 1,  1);
        emit requestDraw(g, "APL", true, true);
    }
}

void AFunctionalModelWidget_ThinLens::onDeleteClicked()
{
    Spectrum.clear();
    updateButtons();
    emit modified();
}

QString AFunctionalModelWidget_ThinLens::updateModel(APhotonFunctionalModel * model)
{
    APFM_ThinLens * tlm = dynamic_cast<APFM_ThinLens*>(model);
    if (tlm)
    {
        tlm->FocalLength_mm = leFocalLength->text().toDouble();
        tlm->FocalLengthSpectrum_mm = Spectrum;
    }
    return "";
}

// ---

AFunctionalModelWidget_OpticalFiber::AFunctionalModelWidget_OpticalFiber(const APFM_OpticalFiber *model, QWidget * parent) :
    AFunctionalModelWidget(model, parent)
{
    QHBoxLayout * lay = new QHBoxLayout(); lay->setContentsMargins(3,0,3,0);

    lay->addWidget( new QLabel(QString("Length:")) );
    leLength = new QLineEdit(); leLength->setValidator(DoubleValidator);
    connect(leLength, &QLineEdit::editingFinished, this, &AFunctionalModelWidget_ThinLens::modified);
    lay->addWidget(leLength);
    lay->addWidget(new QLabel("mm"));
    lay->addStretch();
    MainLayout->addLayout(lay);

    lay = new QHBoxLayout(); lay->setContentsMargins(3,0,3,0);
    lay->addWidget( new QLabel(QString("Max angle for not %0-resolved sim:").arg(QChar(0x3bb))) );
    leMaxAngle = new QLineEdit(); leMaxAngle->setValidator(DoubleValidator);
    connect(leMaxAngle, &QLineEdit::editingFinished, this, &AFunctionalModelWidget_ThinLens::modified);
    lay->addWidget(leMaxAngle);
    lay->addWidget(new QLabel("mm"));
    lay->addStretch();
    MainLayout->addLayout(lay);

    lay = new QHBoxLayout(); lay->setContentsMargins(3,0,3,0);
    lay->addWidget( new QLabel(QString("    Max angle vs %0:").arg(QChar(0x3bb))) );

    pbShow = new QPushButton("Show", this);
    connect(pbShow, &QPushButton::clicked, this, &AFunctionalModelWidget_OpticalFiber::onShowClicked);
    pbShow->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(pbShow, &QPushButton::customContextMenuRequested, this, &AFunctionalModelWidget_OpticalFiber::onShowRightClicked);
    lay->addWidget(pbShow);
    pbLoad = new QPushButton("Load", this);
    connect(pbLoad, &QPushButton::clicked, this, &AFunctionalModelWidget_OpticalFiber::onLoadClicked);
    lay->addWidget(pbLoad);
    pbDelete = new QPushButton("X", this); pbDelete->setMaximumWidth(25);
    connect(pbDelete, &QPushButton::clicked, this, &AFunctionalModelWidget_OpticalFiber::onDeleteClicked);
    lay->addWidget(pbDelete);
    lay->addStretch();
    MainLayout->addLayout(lay);

    leLength->setText(QString::number(model->Length_mm));
    leMaxAngle->setText(QString::number(model->MaxAngle_deg));
    Spectrum = model->MaxAngleSpectrum_deg;
    updateButtons();
}

QString AFunctionalModelWidget_OpticalFiber::updateModel(APhotonFunctionalModel * model)
{
    APFM_OpticalFiber * ofm = dynamic_cast<APFM_OpticalFiber*>(model);
    if (ofm)
    {
        ofm->Length_mm = leLength->text().toDouble();
        ofm->MaxAngle_deg = leMaxAngle->text().toDouble();
        ofm->MaxAngleSpectrum_deg = Spectrum;
    }
    return "";
}

void AFunctionalModelWidget_OpticalFiber::updateButtons()
{
    bool bHaveSpectrum = (!Spectrum.empty());

    pbShow->setEnabled(bHaveSpectrum);
    pbDelete->setEnabled(bHaveSpectrum);
}

void AFunctionalModelWidget_OpticalFiber::onLoadClicked()
{
    QString fileName = guitools::dialogLoadFile(this, "Load file with two columns: wavelength[nm] MaxAngle[deg]", "Data files (*.txt *.dat); All files (*.*)");
    if (fileName.isEmpty()) return;

    std::vector<std::pair<double,double>> tmp;
    QString err = ftools::loadPairs(fileName, tmp, true);
    if (!err.isEmpty()) guitools::message(err, this);
    else
    {
        for (const auto & p : tmp)
        {
            if (p.second < 1e-60)
            {
                guitools::message("Max angle values should be positive!", this);
                return;
            }
        }

        Spectrum = tmp;
        updateButtons();
        emit modified();
    }
}

void AFunctionalModelWidget_OpticalFiber::onShowClicked()
{
    if (Spectrum.empty()) return;

    TGraph * g = AGraphBuilder::graph(Spectrum);
    AGraphBuilder::configure(g, "Max angle vs wavelength",
                             "Wavelength, nm", "Max angle, mm",
                             2, 20, 1,
                             2, 1,  1);
    emit requestDraw(g, "APL", true, true);
}

void AFunctionalModelWidget_OpticalFiber::onShowRightClicked(const QPoint &)
{
    const AWaveResSettings & WaveSet = APhotonSimHub::getInstance().Settings.WaveSet;
    if (!WaveSet.Enabled)
    {
        guitools::message("Simulation is currently configured not to be wavelength-resolved!", this);
        return;
    }

    APFM_OpticalFiber tmpMod;
    tmpMod.MaxAngleSpectrum_deg = Spectrum;
    QString err = tmpMod.updateRuntimeProperties();
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    if (tmpMod._TanMaxAngleSpectrumBinned.empty())
    {
        guitools::message("Wavelength-resolved binned data are empty!", this);
        return;
    }
    else
    {
        std::vector<double> wavelength;
        WaveSet.getWavelengthBins(wavelength);
        TGraph * g = AGraphBuilder::graph(wavelength, tmpMod._TanMaxAngleSpectrumBinned);
        AGraphBuilder::configure(g, "Binned Max Angle vs wavelength",
                                 "Wavelength, nm", "Max angle, mm",
                                 4, 20, 1,
                                 4, 1,  1);
        emit requestDraw(g, "APL", true, true);
    }
}

void AFunctionalModelWidget_OpticalFiber::onDeleteClicked()
{
    Spectrum.clear();
    updateButtons();
    emit modified();
}

// ---

AFunctionalModelWidget_Filter::AFunctionalModelWidget_Filter(const APFM_Filter *model, QWidget * parent) :
    AFunctionalModelWidget(model, parent)
{
    QHBoxLayout * lay = new QHBoxLayout(); lay->setContentsMargins(3,0,3,0);

    cbGray = new QCheckBox("Gray filter");
    connect(cbGray, &QCheckBox::toggled, this, &AFunctionalModelWidget_Filter::onGrayToggled);
    lay->addWidget(cbGray);

    lay->addWidget( new QLabel("Transmission"));
    lNonRes = new QLabel(QString("for not %0-resolved sim").arg(QChar(0x3bb)));
    lay->addWidget(lNonRes);
    lay->addWidget(new QLabel(":"));

    leTransmission = new QLineEdit(); leTransmission->setValidator(DoubleValidator);
    connect(leTransmission, &QLineEdit::editingFinished, this, &AFunctionalModelWidget_ThinLens::modified);
    lay->addWidget(leTransmission);

    lay->addStretch(1);
    MainLayout->addLayout(lay);

    frTrVsLambda = new QFrame();
    {
        lay = new QHBoxLayout(frTrVsLambda); lay->setContentsMargins(3,0,3,0);

        lTrVsLambda = new QLabel(QString("     Transmission vs %0:").arg(QChar(0x3bb)));
        lay->addWidget(lTrVsLambda);

        pbShow = new QPushButton("Show", this);
        connect(pbShow, &QPushButton::clicked, this, &AFunctionalModelWidget_Filter::onShowClicked);
        pbShow->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(pbShow, &QPushButton::customContextMenuRequested, this, &AFunctionalModelWidget_Filter::onShowRightClicked);
        lay->addWidget(pbShow);
        pbLoad = new QPushButton("Load", this);
        connect(pbLoad, &QPushButton::clicked, this, &AFunctionalModelWidget_Filter::onLoadClicked);
        lay->addWidget(pbLoad);
        pbDelete = new QPushButton("X", this); pbDelete->setMaximumWidth(25);
        connect(pbDelete, &QPushButton::clicked, this, &AFunctionalModelWidget_Filter::onDeleteClicked);
        lay->addWidget(pbDelete);
        lay->addStretch(1);
    }
    MainLayout->addWidget(frTrVsLambda);

    leTransmission->setText(QString::number(model->GrayTransmission));
    cbGray->setChecked(model->Gray);
    Spectrum = model->TransmissionSpectrum;
    updateButtons();
}

QString AFunctionalModelWidget_Filter::updateModel(APhotonFunctionalModel * model)
{
    APFM_Filter * fm = dynamic_cast<APFM_Filter*>(model);
    if (fm)
    {
        fm->Gray = cbGray->isChecked();
        fm->GrayTransmission = leTransmission->text().toDouble();
        fm->TransmissionSpectrum = Spectrum;
    }
    return "";
}

void AFunctionalModelWidget_Filter::updateButtons()
{
    bool bHaveSpectrum = (!Spectrum.empty());

    pbShow->setEnabled(bHaveSpectrum);
    pbDelete->setEnabled(bHaveSpectrum);
}

void AFunctionalModelWidget_Filter::onGrayToggled(bool flag)
{
    lNonRes->setVisible(!flag);
    frTrVsLambda->setVisible(!flag);
    emit modified();
}

void AFunctionalModelWidget_Filter::onLoadClicked()
{
    QString fileName = guitools::dialogLoadFile(this, "Load file with two columns: wavelength[nm] Transmission [0..1]", "Data files (*.txt *.dat); All files (*.*)");
    if (fileName.isEmpty()) return;

    std::vector<std::pair<double,double>> tmp;
    QString err = ftools::loadPairs(fileName, tmp, true);
    if (!err.isEmpty()) guitools::message(err, this);
    else
    {
        for (const auto & p : tmp)
        {
            if (p.second < 0 || p.second > 1.0)
            {
                guitools::message("Transmission values should belong to the range [0, 1]!", this);
                return;
            }
        }

        Spectrum = tmp;
        updateButtons();
        emit modified();
    }
}

void AFunctionalModelWidget_Filter::onShowClicked()
{
    if (Spectrum.empty()) return;

    TGraph * g = AGraphBuilder::graph(Spectrum);
    AGraphBuilder::configure(g, "Transmission vs wavelength",
                             "Wavelength, nm", "Transmission",
                             2, 20, 1,
                             2, 1,  1);
    emit requestDraw(g, "APL", true, true);
}

void AFunctionalModelWidget_Filter::onShowRightClicked(const QPoint &)
{
    const AWaveResSettings & WaveSet = APhotonSimHub::getInstance().Settings.WaveSet;
    if (!WaveSet.Enabled)
    {
        guitools::message("Simulation is currently configured not to be wavelength-resolved!", this);
        return;
    }

    APFM_Filter tmpMod;
    tmpMod.TransmissionSpectrum = Spectrum;
    QString err = tmpMod.updateRuntimeProperties();
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

    if (tmpMod._TransmissionBinned.empty())
    {
        guitools::message("Wavelength-resolved binned data are empty!", this);
        return;
    }
    else
    {
        std::vector<double> wavelength;
        WaveSet.getWavelengthBins(wavelength);
        TGraph * g = AGraphBuilder::graph(wavelength, tmpMod._TransmissionBinned);
        AGraphBuilder::configure(g, "Binned transmission vs wavelength",
                                 "Wavelength, nm", "Transmission",
                                 4, 20, 1,
                                 4, 1,  1);
        emit requestDraw(g, "APL", true, true);
    }
}

void AFunctionalModelWidget_Filter::onDeleteClicked()
{
    Spectrum.clear();
    updateButtons();
    emit modified();
}

// ---
