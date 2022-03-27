#ifndef AMINIJS_SI_H
#define AMINIJS_SI_H

#include "ascriptminimizerbase.h"

class AFunctorJS
{
public:
    double operator()(const double *p);
};

class AMiniJS_SI : public AScriptMinimizerBase
{
    Q_OBJECT

public:
    ~AMiniJS_SI();

public slots:
    void setFunctorName(const QString & name) override;

protected:
    void                  configureNumVariables(int num) override;
    ROOT::Math::Functor * configureFunctor() override;

private:
    AFunctorJS * Functor = nullptr;
};

#endif // AMINIJS_SI_H
