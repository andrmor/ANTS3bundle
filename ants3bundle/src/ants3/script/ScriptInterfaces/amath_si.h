#ifndef AMATHSCRIPTINTERFACE_H
#define AMATHSCRIPTINTERFACE_H

#include "ascriptinterface.h"

#include <QVariantList>

class ARandomHub;

class AMath_SI : public AScriptInterface
{
    Q_OBJECT
    Q_PROPERTY(double pi READ pi)
    double pi() const { return 3.141592653589793238462643383279502884; }

public:
    AMath_SI();

    AScriptInterface * cloneBase() const {return new AMath_SI();}

public slots:
    double abs(double val);
    double acos(double val);
    double asin(double val);
    double atan(double val);
    double atan2(double y, double x);
    double ceil(double val);
    double cos(double val);
    double sin(double val);
    double cosh(double val);
    double sinh(double val);
    double exp(double val);
    double floor(double val);
    double log(double val);
    double log10(double val);
    double max(double val1, double val2);
    double min(double val1, double val2);
    double pow(double val, double power);
    double sqrt(double val);
    double tan(double val);
    double round(double val);
    double random();
    double gauss(double mean, double sigma);
    double poisson(double mean);
    double maxwell(double a);  // a is sqrt(kT/m)
    double exponential(double tau);

    QVariantList getAnglesBetween3DVectors(QVariantList arrayOfVectors);

    QVariantList generateDirectionIsotropic();

    // NOT multi-thread friendly!
    //QVariantList fit1D(QVariantList array, QString tformula, QVariantList range = QVariantList(), QVariantList startParValues = QVariantList(), bool extendedOutput = false);
    QVariantList fit1D(QVariantList array, QString tformula, QVariantList startParValues = QVariantList(), bool extendedOutput = false);

    QVariantList fft(QVariantList array, int maxN = -1);
    QVariantList fftMulti(QVariantList arrayOfArrays, int maxN = -1);

    double evalFormula(QString formula, QVariantList varNames, QVariantList varValues);

private:
    ARandomHub & RandomHub;
};

#endif // AMATHSCRIPTINTERFACE_H
