#include "amercuryeventexplorer.h"
#include "ui_amercuryeventexplorer.h"
#include "alightresponsehub.h"
#include "lrmodel.h"
#include "reconstructor.h"
#include "agraphbuilder.h"

#include "TGraph.h"

#include <QPlainTextEdit>

AMercuryEventExplorer::AMercuryEventExplorer(Reconstructor * reconstructor, std::vector<std::vector<double> > *events, QWidget * parent) :
    QDialog(parent), ui(new Ui::AMercuryEventExplorer),
    Rec(reconstructor), Events(events)
{
    ui->setupUi(this);

    Model = ALightResponseHub::getInstance().Model;

    setWindowTitle("Mercury event explorer");

    QList<QPushButton*> list = this->findChildren<QPushButton*>();
    foreach(QPushButton * pb, list) {pb->setDefault(false); pb->setAutoDefault(false);}

    // check Model
    // check numSens vs Events[0].size

    onEventChanged();
}

AMercuryEventExplorer::~AMercuryEventExplorer()
{
    delete ui;
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

void AMercuryEventExplorer::on_pbSignalVsModel_pressed()
{
    ui->pbMap->setChecked(false);
    showSignals();
}

void AMercuryEventExplorer::on_pbMap_pressed()
{
    ui->pbSignalVsModel->setChecked(false);

}

