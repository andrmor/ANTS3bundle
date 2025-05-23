#include "amonitordelegateform.h"
#include "ui_amonitordelegateform.h"
#include "ageoobject.h"
#include "ageotype.h"
#include "aonelinetextedit.h"
#include "ageobasedelegate.h"
#include "ageoconsts.h"
#include "guitools.h"

#include <QDebug>

#include <vector>

AMonitorDelegateForm::AMonitorDelegateForm(QWidget * parent) :
    QWidget(parent),
    ui(new Ui::AMonitorDelegateForm)
{
    ui->setupUi(this);

    ui->pbContentChanged->setVisible(false);

    ui->cobEnergyUnits->setCurrentIndex(2);

    leSize1 = new AOneLineTextEdit(); ui->laySize->insertWidget(1, leSize1);
    leSize2 = new AOneLineTextEdit(); ui->laySize->insertWidget(4, leSize2);

    leX     = new AOneLineTextEdit(); ui->layX->insertWidget(1, leX);
    leY     = new AOneLineTextEdit(); ui->layY->insertWidget(1, leY);
    leZ     = new AOneLineTextEdit(); ui->layZ->insertWidget(1, leZ);

    lePhi   = new AOneLineTextEdit(); ui->layOrientation->addWidget(lePhi);
    leTheta = new AOneLineTextEdit(); ui->layOrientation->addWidget(leTheta);
    lePsi   = new AOneLineTextEdit(); ui->layOrientation->addWidget(lePsi);

    for (AOneLineTextEdit * le : {leSize1, leSize2, leX, leY, leZ, lePhi, leTheta, lePsi})
    {
        AGeoBaseDelegate::configureHighligherAndCompleter(le);
        connect(le, &AOneLineTextEdit::textChanged, this, &AMonitorDelegateForm::contentChanged);
    }

    //installing double validators for edit boxes
    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = findChildren<QLineEdit*>();
    for (QLineEdit * w : qAsConst(list))
        if (w->objectName().startsWith("led")) w->setValidator(dv);
}

AMonitorDelegateForm::~AMonitorDelegateForm()
{
    delete ui;
}

bool AMonitorDelegateForm::updateGUI(const AGeoObject *obj)
{
    ATypeMonitorObject* mon = dynamic_cast<ATypeMonitorObject*>(obj->Type);
    if (!mon)
    {
        qWarning() << "Attempt to use non-monitor object to update monitor delegate!";
        return false;
    }
    const AMonitorConfig& config = mon->config;

    ui->leName->setText(obj->Name);
    if (config.shape == 0) ui->cobShape->setCurrentIndex(0);
    else ui->cobShape->setCurrentIndex(1);

    leSize1->setText( config.str2size1.isEmpty() ? QString::number(2.0*config.size1) : config.str2size1);
    leSize2->setText( config.str2size2.isEmpty() ? QString::number(2.0*config.size2) : config.str2size2);

    leX->    setText( obj->PositionStr[0].isEmpty() ? QString::number(obj->Position[0]) : obj->PositionStr[0]);
    leY->    setText( obj->PositionStr[1].isEmpty() ? QString::number(obj->Position[1]) : obj->PositionStr[1]);
    leZ->    setText( obj->PositionStr[2].isEmpty() ? QString::number(obj->Position[2]) : obj->PositionStr[2]);

    lePhi->  setText( obj->OrientationStr[0].isEmpty() ? QString::number(obj->Orientation[0]) : obj->OrientationStr[0]);
    leTheta->setText( obj->OrientationStr[1].isEmpty() ? QString::number(obj->Orientation[1]) : obj->OrientationStr[1]);
    lePsi->  setText( obj->OrientationStr[2].isEmpty() ? QString::number(obj->Orientation[2]) : obj->OrientationStr[2]);

    int sens = 0;
    if (config.bLower && !config.bUpper) sens = 1;
    else if (config.bLower && config.bUpper) sens = 2;
    ui->cobSensitiveDirection->setCurrentIndex(sens);

    ui->cbStopTracking->setChecked(config.bStopTracking);

    ui->leParticle->setText(config.Particle);

    int prsec = 0;
    if (config.bSecondary && !config.bPrimary) prsec = 1;
    else if (config.bSecondary && config.bPrimary) prsec = 2;
    ui->cobPrimarySecondary->setCurrentIndex(prsec);

    int dirin = 0;
    if (config.bIndirect && !config.bDirect) dirin = 1;
    else if (config.bIndirect &&  config.bDirect) dirin = 2;
    ui->cobDirectIndirect->setCurrentIndex(dirin);

    ui->sbXbins->setValue(config.xbins);
    ui->sbYbins->setValue(config.ybins);
    ui->sbTimeBins->setValue(config.timeBins);
    ui->sbAngleBins->setValue(config.angleBins);
    ui->sbWaveBins->setValue(config.waveBins);
    ui->sbEnergyBins->setValue(config.energyBins);

    ui->ledTimeFrom->setText( QString::number(config.timeFrom) );
    ui->ledAngleFrom->setText( QString::number(config.angleFrom) );
    ui->ledWaveFrom->setText( QString::number(config.waveFrom) );
    ui->ledEnergyFrom->setText( QString::number(config.energyFrom) );

    ui->ledTimeTo->setText( QString::number(config.timeTo) );
    ui->ledAngleTo->setText( QString::number(config.angleTo) );
    ui->ledWaveTo->setText( QString::number(config.waveTo) );
    ui->ledEnergyTo->setText( QString::number(config.energyTo) );

    int tIndex = 0;
        if      (config.timeUnits == "ns") tIndex = 0;
        else if (config.timeUnits == "us") tIndex = 1;
        else if (config.timeUnits == "ms") tIndex = 2;
        else if (config.timeUnits == "s")  tIndex = 3;
        else qWarning() << "Unknown time unit for monitor:" << config.timeUnits;
    ui->cobTimeUnits->setCurrentIndex(tIndex);
    int enIndex = 0;
        if      (config.energyUnits == "meV") enIndex = 0;
        else if (config.energyUnits == "eV")  enIndex = 1;
        else if (config.energyUnits == "keV") enIndex = 2;
        else if (config.energyUnits == "MeV") enIndex = 3;
        else qWarning() << "Unknown energy unit for monitor:" << config.energyUnits;
    ui->cobEnergyUnits->setCurrentIndex(enIndex);

    setPhotonOrParticle(config.PhotonOrParticle);

    return true;
}

QString AMonitorDelegateForm::getName() const
{
    return ui->leName->text();
}

#include "ageometryhub.h"
bool AMonitorDelegateForm::updateObject(AGeoObject * obj)
{
    ATypeMonitorObject* mon = dynamic_cast<ATypeMonitorObject*>(obj->Type);
    AMonitorConfig & config = mon->config;

    const AGeoConsts & GC = AGeoConsts::getConstInstance();
    QString ErrorStr;

    QString strSize1 = leSize1->text();
    double Size1;
    bool ok = GC.updateDoubleParameter(ErrorStr, strSize1, Size1);
    if (!ok)
    {
        guitools::message(ErrorStr + " in Size1\n", this);
        return false;
    }

    QString strSize2 = leSize2->text();
    double Size2;
    ok = GC.updateDoubleParameter(ErrorStr, strSize2, Size2);
    if (!ok)
    {
        guitools::message(ErrorStr + " in Size2\n", this);
        return false;
    }

    std::vector<QString> tempStrs(6);
    std::vector<double>  tempDoubles(6);
    ok = true;
    ok = ok && AGeoBaseDelegate::processEditBox("X position",      leX,     tempDoubles[0], tempStrs[0], this->parentWidget());
    ok = ok && AGeoBaseDelegate::processEditBox("Y position",      leY,     tempDoubles[1], tempStrs[1], this->parentWidget());
    ok = ok && AGeoBaseDelegate::processEditBox("Z position",      leZ,     tempDoubles[2], tempStrs[2], this->parentWidget());
    ok = ok && AGeoBaseDelegate::processEditBox("Phi orientation", lePhi,   tempDoubles[3], tempStrs[3], this->parentWidget());
    ok = ok && AGeoBaseDelegate::processEditBox("Phi orientation", leTheta, tempDoubles[4], tempStrs[4], this->parentWidget());
    ok = ok && AGeoBaseDelegate::processEditBox("Phi orientation", lePsi,   tempDoubles[5], tempStrs[5], this->parentWidget());
    if (!ok) return false;


    // ---- all checks are passed, can assign values now ----

    obj->Name = getName();

    config.shape = ui->cobShape->currentIndex();
    config.size1 = Size1; config.str2size1 = strSize1;
    config.size2 = Size2; config.str2size2 = strSize2;
    //obj->updateMonitorShape();

    for (int i = 0; i < 3; i++)
    {
        obj->PositionStr[i]    = tempStrs[i];
        obj->OrientationStr[i] = tempStrs[i+3];

        obj->Position[i]       = tempDoubles[i];
        obj->Orientation[i]    = tempDoubles[i+3];
    }

    int sens = ui->cobSensitiveDirection->currentIndex();
    switch (sens)
    {
      case 0: config.bUpper = true;  config.bLower = false; break;
      case 1: config.bUpper = false; config.bLower = true;  break;
      case 2: config.bUpper = true;  config.bLower = true;  break;
      default: qWarning() << "Bad sensitive directions!";
    }

    config.bStopTracking = ui->cbStopTracking->isChecked();

    if (config.PhotonOrParticle == 1)
    {
        config.Particle = ui->leParticle->text();

        int prsec =  ui->cobPrimarySecondary->currentIndex();
        switch (prsec)
        {
        case 0: config.bPrimary = true;  config.bSecondary = false; break;
        case 1: config.bPrimary = false; config.bSecondary = true;  break;
        case 2: config.bPrimary = true;  config.bSecondary = true;  break;
        default: qWarning() << "Bad primary/secondary selector";
        }

        int dirin =  ui->cobDirectIndirect->currentIndex();
        switch (dirin)
        {
        case 0: config.bDirect = true;  config.bIndirect = false; break;
        case 1: config.bDirect = false; config.bIndirect = true;  break;
        case 2: config.bDirect = true;  config.bIndirect = true;  break;
        default: qWarning() << "Bad direct/indirect selector";
        }
    }

    config.xbins = ui->sbXbins->value();
    config.ybins = ui->sbYbins->value();
    config.timeBins = ui->sbTimeBins->value();
    config.angleBins = ui->sbAngleBins->value();
    config.waveBins = ui->sbWaveBins->value();
    config.energyBins = ui->sbEnergyBins->value();

    config.timeFrom = ui->ledTimeFrom->text().toDouble();
    config.angleFrom = ui->ledAngleFrom->text().toDouble();
    config.waveFrom = ui->ledWaveFrom->text().toDouble();
    config.energyFrom = ui->ledEnergyFrom->text().toDouble();

    config.timeTo = ui->ledTimeTo->text().toDouble();
    config.angleTo = ui->ledAngleTo->text().toDouble();
    config.waveTo = ui->ledWaveTo->text().toDouble();
    config.energyTo = ui->ledEnergyTo->text().toDouble();

    config.timeUnits = ui->cobTimeUnits->currentText();
    config.energyUnits = ui->cobEnergyUnits->currentText();

    return true;
}

void AMonitorDelegateForm::UpdateVisibility()
{
    on_cobShape_currentIndexChanged(ui->cobShape->currentIndex());
}

void AMonitorDelegateForm::on_cobShape_currentIndexChanged(int index)
{
    bool bRectangular = (index == 0);

    ui->labSize2->setVisible(bRectangular);
    ui->labSize2mm->setVisible(bRectangular);
    leSize2->setVisible(bRectangular);
    ui->labSizeX->setText( (bRectangular ? "Size X:" : "Diameter:") );
}

void AMonitorDelegateForm::setPhotonOrParticle(int index)
{
    ui->frParticle->setVisible(index == 1);

    ui->labWave->setVisible(index == 0);
    ui->labWaveBins->setVisible(index == 0);
    ui->labWaveFrom->setVisible(index == 0);
    ui->labWaveTo->setVisible(index == 0);
    ui->ledWaveFrom->setVisible(index == 0);
    ui->ledWaveTo->setVisible(index == 0);
    ui->labWaveUnits->setVisible(index == 0);
    ui->sbWaveBins->setVisible(index == 0);

    ui->labEnergy->setVisible(index == 1);
    ui->labEnergyBins->setVisible(index == 1);
    ui->labEnergyFrom->setVisible(index == 1);
    ui->labEnergyTo->setVisible(index == 1);
    ui->ledEnergyFrom->setVisible(index == 1);
    ui->ledEnergyTo->setVisible(index == 1);
    ui->cobEnergyUnits->setVisible(index == 1);
    ui->sbEnergyBins->setVisible(index == 1);
}

void AMonitorDelegateForm::on_pbContentChanged_clicked()
{
    emit contentChanged();
}

void AMonitorDelegateForm::on_pbShowSensitiveDirection_clicked()
{
    emit showSensDirection();
}
