#ifndef AFUNCTIONALMODELWIDGET_H
#define AFUNCTIONALMODELWIDGET_H

#include "aphotonfunctionalmodel.h"

#include <QFrame>
#include <QString>

class QVBoxLayout;
class QDoubleValidator;
class QLineEdit;
class QPushButton;
class TObject;

class AFunctionalModelWidget : public QFrame
{
    Q_OBJECT

public:
    AFunctionalModelWidget(APhotonFunctionalModel * model, QWidget * parent = nullptr);
    virtual ~AFunctionalModelWidget(){}

    virtual QString updateModel(APhotonFunctionalModel * model) = 0; // returns error if not possible

    static AFunctionalModelWidget * factory(APhotonFunctionalModel * model, QWidget * parent); // register all NEW MODELS here!

protected:
    QVBoxLayout      * MainLayout      = nullptr;
    QDoubleValidator * DoubleValidator = nullptr;

signals:
    void modified();
    void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);
};

class AFunctionalModelWidget_Dummy : public AFunctionalModelWidget
{
public:
    AFunctionalModelWidget_Dummy(APhotonFunctionalModel * model, QWidget * parent = nullptr);
    QString updateModel(APhotonFunctionalModel * /*model*/) override {return "";}
};

class AFunctionalModelWidget_ThinLens : public AFunctionalModelWidget
{
    Q_OBJECT

public:
    AFunctionalModelWidget_ThinLens(APFM_ThinLens * model, QWidget * parent = nullptr);
    QString updateModel(APhotonFunctionalModel * model) override;

protected:
    QLineEdit * leFocalLength = nullptr;

    QPushButton * pbShow = nullptr;
    QPushButton * pbLoad = nullptr;
    QPushButton * pbDelete = nullptr;

    std::vector<std::pair<double,double>> Spectrum;

    void updateButtons();

private slots:
    void onLoadClicked();
    void onShowClicked();
    void onDeleteClicked();

};

class AFunctionalModelWidget_OpticalFiber : public AFunctionalModelWidget
{
public:
    AFunctionalModelWidget_OpticalFiber(APFM_OpticalFiber * model, QWidget * parent = nullptr);
    QString updateModel(APhotonFunctionalModel * model) override;

protected:

};

#endif // AFUNCTIONALMODELWIDGET_H
