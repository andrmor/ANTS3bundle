#include "afunctionalmodelwidget.h"
#include "afiletools.h"
#include "guitools.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>

AFunctionalModelWidget::AFunctionalModelWidget(APhotonFunctionalModel * model, QWidget * parent) :
    QFrame(parent)
{
    MainLayout = new QVBoxLayout(this);
    MainLayout->setContentsMargins(3,0,3,0);

    DoubleValidator = new QDoubleValidator(this);
}

AFunctionalModelWidget * AFunctionalModelWidget::factory(APhotonFunctionalModel * model, QWidget * parent)
{
    if (!model)
    {
        qWarning() << "Empty photon functional model in widget factory!";
        return new AFunctionalModelWidget_Dummy(model, parent);
    }

    APFM_Dummy * dm = dynamic_cast<APFM_Dummy*>(model);
    if (dm) return new AFunctionalModelWidget_Dummy(model, parent);

    APFM_ThinLens * tlm = dynamic_cast<APFM_ThinLens*>(model);
    if (tlm) return new AFunctionalModelWidget_ThinLens(tlm, parent);

    APFM_OpticalFiber * ofm = dynamic_cast<APFM_OpticalFiber*>(model);
    if (ofm) return new AFunctionalModelWidget_OpticalFiber(ofm, parent);

    qWarning() << "Unknown photon functional model in widget factory:" << model->getType();
    return new AFunctionalModelWidget_Dummy(model, parent);
}

// ---

AFunctionalModelWidget_Dummy::AFunctionalModelWidget_Dummy(APhotonFunctionalModel * model, QWidget * parent) :
    AFunctionalModelWidget(model, parent) {}

// ---

#include <QPushButton>
AFunctionalModelWidget_ThinLens::AFunctionalModelWidget_ThinLens(APFM_ThinLens * model, QWidget * parent) :
    AFunctionalModelWidget(model, parent)
{
    QHBoxLayout * lay = new QHBoxLayout(); lay->setContentsMargins(3,0,3,0);

    lay->addWidget( new QLabel(QString("Focal length vs %0:").arg(QChar(0x3bb))) );

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

    lay->addStretch(2);

    lay->addWidget( new QLabel(QString("For not %0-resolved sim:").arg(QChar(0x3bb))) );

    leFocalLength = new QLineEdit(); leFocalLength->setValidator(DoubleValidator);
    connect(leFocalLength, &QLineEdit::editingFinished, this, &AFunctionalModelWidget_ThinLens::modified);
    lay->addWidget(leFocalLength);

    lay->addWidget(new QLabel("mm"));

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

#include "agraphbuilder.h"
#include "TGraph.h"
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

#include "aphotonsimhub.h"
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
        guitools::message("Wavelength-resolved binned data are empty!", this);
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
                                 "WaveIndex", "Focal length, mm",
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

AFunctionalModelWidget_OpticalFiber::AFunctionalModelWidget_OpticalFiber(APFM_OpticalFiber * model, QWidget * parent) :
    AFunctionalModelWidget(model, parent)
{

}

QString AFunctionalModelWidget_OpticalFiber::updateModel(APhotonFunctionalModel * model)
{
    return "";
}

// ---

