#include "amercuryeventexplorer.h"
#include "ui_amercuryeventexplorer.h"
#include "alightresponsehub.h"
#include "lrmodel.h"
#include "reconstructor.h"
#include "agraphbuilder.h"

#include "TGraph.h"

#include <QPlainTextEdit>
#include <QDoubleValidator>

AMercuryEventExplorer::AMercuryEventExplorer(QWidget * parent) :
    QDialog(parent), ui(new Ui::AMercuryEventExplorer)
{
    ui->setupUi(this);

    setModal(false);
    setWindowTitle("Mercury event explorer");

    QDoubleValidator * doubVal = new QDoubleValidator(this);
    QList<QLineEdit*> leList = this->findChildren<QLineEdit*>();
    foreach(QLineEdit * led, leList) {led->setValidator(doubVal);}

    ui->pbUpdateMap->setVisible(false);

    QList<QPushButton*> pbList = this->findChildren<QPushButton*>();
    foreach(QPushButton * pb, pbList) {pb->setDefault(false); pb->setAutoDefault(false);}

    connect(this, &AMercuryEventExplorer::rejected, this, [this](){bFinished = true;});
}

AMercuryEventExplorer::~AMercuryEventExplorer()
{
    delete ui;
    delete Events;
}

QString AMercuryEventExplorer::start(Reconstructor * reconstructor, std::vector<std::vector<double>> * events)
{
    Model = ALightResponseHub::getInstance().Model;
    Rec = reconstructor;
    delete Events; Events = events;

    bFinished = true;
    if (!Model) return "Model is not defined";
    if (!Rec) return "Reconstructor is not defined";
    if (!Events || Events->empty()) return "No event data provided";
    bFinished = false;

    onEventChanged();
    return "";
}

void AMercuryEventExplorer::on_pbPrevious_clicked()
{
    int iEv = ui->sbEvent->value();
    if (iEv > 0) ui->sbEvent->setValue(iEv - 1);
    onEventChanged();
}

void AMercuryEventExplorer::on_pbNext_clicked()
{
    int iEv = ui->sbEvent->value();
    if (iEv < Events->size() - 1) ui->sbEvent->setValue(iEv + 1);
    onEventChanged();
}

void AMercuryEventExplorer::on_sbEvent_editingFinished()
{
    int iEv = ui->sbEvent->value();
    if (iEv >= Events->size()) ui->sbEvent->setValue(Events->size() - 1);
    onEventChanged();
}

void AMercuryEventExplorer::onEventChanged()
{
    int iEvent = ui->sbEvent->value();
    if (iEvent < 0 || iEvent >= Events->size()) return;

    if (!Model) return;
    if (!Rec)   return;

    ui->pteOut->clear();
    bGood = Rec->ProcessEvent(Events->at(iEvent));
    QString txt;
    if (bGood)
    {
        double x = Rec->getRecX();
        double y = Rec->getRecY();
        double z = Rec->getRecZ();
        double e = Rec->getRecE();

        double chi2 = Rec->getChi2Min();
        int    dof  = Rec->getDof();

        txt = QString("Reconstruction results:\nX: %0 Y: %1 Z: %2   E: %3   Chi2: %4").arg(x).arg(y).arg(z).arg(e).arg(chi2/dof);
    }
    else
    {
        txt = QString("Reconstruction failed\nStatus: %0").arg(Rec->getRecStatus());
    }

    ui->pteOut->appendPlainText(txt);

    if (ui->pbSignalVsModel->isChecked()) showSignals();
    else if (ui->pbMap->isChecked())      showMap();
}

void AMercuryEventExplorer::showSignals()
{
    if (!Model) return;

    int iEvent = ui->sbEvent->value();
    if (iEvent >= Events->size()) return;

    if (!bGood)
    {
        emit requestDraw(nullptr, "AP", true, false);
        return;
    }

    double x = Rec->getRecX();
    double y = Rec->getRecY();
    double z = Rec->getRecZ();
    double e = Rec->getRecE();

    TGraph * g = new TGraph();
    const int numSens = Model->GetSensorCount();
    for (int iSens = 0; iSens < numSens; iSens++)
    {
        double signal = Events->at(iEvent)[iSens];
        double expect = Model->Eval(iSens, x, y, z) * e;

        g->AddPoint(expect, signal);
    }
    AGraphBuilder::configureTitles(g, "SignalVsExp", "Expected signal from the model", "Signal");
    AGraphBuilder::configureMarkers(g, 2, 20, 1);
    emit requestDraw(g, "AP", true, true);
}

#include "TH2D.h"
#include "TH3D.h"
void AMercuryEventExplorer::showMap()
{
    if (!Model) return;

    int iEvent = ui->sbEvent->value();
    if (iEvent >= Events->size()) return;

    if (!bGood)
    {
        emit requestDraw(nullptr, "AP", true, false);
        return;
    }

    std::array<double, 3> origin;

    if (true)
    {
        origin = {Rec->getRecX(), Rec->getRecY(), Rec->getRecZ()};
    }

    int binsX = ui->sbXbins->value();
    int binsY = ui->sbYbins->value();
    int binsZ = ui->sbZbins->value();

    double rangeX = ui->ledXrange->text().toDouble();
    double rangeY = ui->ledYrange->text().toDouble();
    double rangeZ = ui->ledZrange->text().toDouble();

    double deltaX = rangeX / binsX; double startX = -0.5 * (binsX - 1) * deltaX;
    double deltaY = rangeY / binsY; double startY = -0.5 * (binsY - 1) * deltaY;
    double deltaZ = rangeZ / binsZ; double startZ = -0.5 * (binsZ - 1) * deltaZ;

    bool bChi2 = (ui->cobMapWhat->currentIndex() == 0);
    int dof  = Rec->getDof();
    double fixed = ui->ledMapFixedCoord->text().toDouble();

    TH2D * h2 = nullptr;
    switch (ui->cobMapHow->currentIndex())
    {
    case 0: // XY at fixed Z
        h2 = new TH2D("", "", binsX, origin[0] + startX - 0.5*deltaX, origin[0] - startX + 0.5*deltaX,
                             binsY, origin[1] + startY - 0.5*deltaY, origin[1] - startY + 0.5*deltaY);
        for (int ix = 0; ix < binsX; ix++)
        {
            double x = origin[0] + startX + deltaX * ix;
            for (int iy = 0; iy < binsY; iy++)
            {
                double y = origin[1] + startY + deltaY * iy;
                double val = (bChi2 ? Rec->getChi2autoE(x, y, fixed, true)/dof : Rec->getLogLHautoE(x, y, fixed));
                h2->Fill(x, y, val);
            }
        }
        break;
    case 1: // XZ at fixed Y
        break;
    case 2: // YZ at fixed X
        break;
    case 3: // 3D
        {
            TH3D * h3 = new TH3D("", "", binsX, origin[0] + startX - 0.5*deltaX, origin[0] - startX + 0.5*deltaX,
                                         binsY, origin[1] + startY - 0.5*deltaY, origin[1] - startY + 0.5*deltaY,
                                         binsZ, origin[2] + startZ - 0.5*deltaZ, origin[2] - startZ + 0.5*deltaZ);
            for (int ix = 0; ix < binsX; ix++)
            {
                double x = origin[0] + startX + deltaX * ix;
                for (int iy = 0; iy < binsY; iy++)
                {
                    double y = origin[1] + startY + deltaY * iy;
                    for (int iz = 0; iz < binsZ; iz++)
                    {
                        double z = origin[2] + startZ + deltaZ * iz;
                        double val = (bChi2 ? Rec->getChi2autoE(x, y, z, true)/dof : Rec->getLogLHautoE(x, y, z));
                        h3->Fill(x, y, z, val);
                    }
                }
            }
            emit requestDraw(h3, "box3", true, true);
            return;
        }
    }

    emit requestDraw(h2, "colz", true, true);
}

void AMercuryEventExplorer::on_pbSignalVsModel_clicked(bool checked)
{
    ui->pbSignalVsModel->setChecked(checked);
    if (checked)
    {
        ui->pbMap->setChecked(false);
        showSignals();
    }
}

void AMercuryEventExplorer::on_pbMap_clicked(bool checked)
{
    ui->pbMap->setChecked(checked);
    if (checked)
    {
        ui->pbSignalVsModel->setChecked(false);
        showMap();
    }
}

void AMercuryEventExplorer::on_pbUpdateMap_clicked()
{
    if (ui->pbMap->isChecked()) showMap();
}

void AMercuryEventExplorer::on_pbClose_clicked()
{
    reject();
}

void AMercuryEventExplorer::on_cbSymmetricXY_clicked(bool checked)
{
    ui->ledYrange->setEnabled(!checked);
    ui->sbYbins->setEnabled(!checked);

    if (checked)
    {
        ui->ledYrange->setText(ui->ledXrange->text());
        ui->sbYbins->setValue(ui->sbXbins->value());
    }

    if (ui->pbMap->isChecked()) showMap();
}

void AMercuryEventExplorer::on_ledXrange_editingFinished()
{
    if (ui->cbSymmetricXY->isChecked()) ui->ledYrange->setText(ui->ledXrange->text());
    if (ui->pbMap->isChecked()) showMap();
}

void AMercuryEventExplorer::on_sbXbins_editingFinished()
{
    if (ui->cbSymmetricXY->isChecked()) ui->sbYbins->setValue(ui->sbXbins->value());
    if (ui->pbMap->isChecked()) showMap();
}

