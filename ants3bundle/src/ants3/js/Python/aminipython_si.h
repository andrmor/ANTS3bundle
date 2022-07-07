#ifndef AMINIPYTHON_SI_H
#define AMINIPYTHON_SI_H

#include "ascriptminimizerbase.h"

class AFunctorPython
{
public:
    double operator()(const double *p);
};

class APythonScriptManager;

class AMiniPython_SI : public AScriptMinimizerBase
{
    Q_OBJECT

public:
    ~AMiniPython_SI();

    bool wasAborted() const override;

    AScriptInterface * cloneBase() const override {return new AMiniPython_SI();}

public slots:
    void setFunctorName(const QString & name) override;

protected:
    void                  configureNumVariables(int num) override;
    ROOT::Math::Functor * configureFunctor() override;

private:
    AFunctorPython * Functor = nullptr;
};

#endif // AMINIPYTHON_SI_H
