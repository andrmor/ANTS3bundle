#include "afunctionalmodelwidget.h"

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

AFunctionalModelWidget_ThinLens::AFunctionalModelWidget_ThinLens(APFM_ThinLens * model, QWidget * parent) :
    AFunctionalModelWidget(model, parent)
{
    QHBoxLayout * lay = new QHBoxLayout(); lay->setContentsMargins(3,0,3,0);
    lay->addWidget(new QLabel("Focal length:"));
    leFocalLength = new QLineEdit(); leFocalLength->setValidator(DoubleValidator);
    lay->addWidget(leFocalLength);
    lay->addWidget(new QLabel("mm"));
    lay->addStretch(1);

    MainLayout->addLayout(lay);

    leFocalLength->setText(QString::number(model->FocalLength_mm));
}

QString AFunctionalModelWidget_ThinLens::updateModel(APhotonFunctionalModel * model)
{
    APFM_ThinLens * tlm = dynamic_cast<APFM_ThinLens*>(model);
    if (tlm)
    {
        tlm->FocalLength_mm = leFocalLength->text().toDouble();
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

