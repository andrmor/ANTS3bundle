#ifndef AFUNCTIONALMODELWIDGET_H
#define AFUNCTIONALMODELWIDGET_H

#include "aphotonfunctionalmodel.h"

#include <QFrame>
#include <QString>

class QVBoxLayout;
class QDoubleValidator;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QLabel;
class TObject;

class AFunctionalModelWidget : public QFrame
{
    Q_OBJECT

public:
    AFunctionalModelWidget(const APhotonFunctionalModel * model, QWidget * parent = nullptr);
    virtual ~AFunctionalModelWidget(){}

    virtual QString updateModel(APhotonFunctionalModel * model) = 0; // returns error if not possible

    static AFunctionalModelWidget * factory(const APhotonFunctionalModel *model, QWidget * parent); // register all NEW MODELS here!
    static QString getModelDatabase();                                                         // register all NEW MODELS here!

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
    AFunctionalModelWidget_Dummy(const APhotonFunctionalModel * model, QWidget * parent = nullptr);
    QString updateModel(APhotonFunctionalModel * /*model*/) override {return "";}
};

class AFunctionalModelWidget_ThinLens : public AFunctionalModelWidget
{
    Q_OBJECT

public:
    AFunctionalModelWidget_ThinLens(const APFM_ThinLens * model, QWidget * parent = nullptr);
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
    void onShowRightClicked(const QPoint &);
    void onDeleteClicked();

};

class AFunctionalModelWidget_OpticalFiber : public AFunctionalModelWidget
{
public:
    AFunctionalModelWidget_OpticalFiber(const APFM_OpticalFiber * model, QWidget * parent = nullptr);
    QString updateModel(APhotonFunctionalModel * model) override;

protected:

};

class AFunctionalModelWidget_Filter : public AFunctionalModelWidget
{
    Q_OBJECT

public:
    AFunctionalModelWidget_Filter(const APFM_Filter * model, QWidget * parent = nullptr);
    QString updateModel(APhotonFunctionalModel * model) override;

protected:
    QCheckBox * cbGray = nullptr;
    QLineEdit * leTransmission = nullptr;

    QPushButton * pbShow = nullptr;
    QPushButton * pbLoad = nullptr;
    QPushButton * pbDelete = nullptr;

    QLabel * lTrVsLambda = nullptr;
    QLabel * lNonRes = nullptr;
    QFrame * frTrVsLambda = nullptr;

    std::vector<std::pair<double,double>> Spectrum;

    void updateButtons();

private slots:
    void onGrayToggled(bool flag);
    void onLoadClicked();
    void onShowClicked();
    void onShowRightClicked(const QPoint &);
    void onDeleteClicked();

};


#endif // AFUNCTIONALMODELWIDGET_H
