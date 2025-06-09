#ifndef ASCRIPTMINIMIZERBASE_H
#define ASCRIPTMINIMIZERBASE_H

#include "ascriptinterface.h"
#include "avarrecordbase.h"

#include <vector>

#include <QObject>
#include <QVariant>

class AScriptManager;
class AJavaScriptManager;
class AFunctorBase;

namespace ROOT { namespace Math { class Functor; } }

class AScriptMinimizerBase : public AScriptInterface
{
    Q_OBJECT

public:
    AScriptMinimizerBase();
    ~AScriptMinimizerBase();

    virtual bool   wasAborted() const = 0;

public slots:
    virtual void   setFunctorName(const QString & name) = 0;

    void           addVariable(QString name, double start, double step);
    void           addLowerLimitedVariable(QString name, double value, double step, double lowerLimit);
    void           addUpperLimitedVariable(QString name, double value, double step, double upperLimit);
    void           addLimitedVariable(QString name, double start, double step, double min, double max);
    void           addFixedVariable(QString name, double value);

    void           setSimplex();
    void           setMigrad();

    void           setMaxIterations(int num);
    void           setMaxCalls(int num);
    void           setTolerance(double val);

    void           setHighPrecision(bool flag);
    void           setVerbosity(int level);

    bool           run();

    QVariantList   getResults();

    void           clear();

protected:
    virtual void                  configureNumVariables(int num) = 0;
    virtual ROOT::Math::Functor * configureFunctor() = 0;

    std::vector<AVarRecordBase*> Variables;
    QVariantList    Results;

    bool            bHighPrecision = false;
    int             PrintVerbosity = -1;
    int             Method = 1; // 0-Migrad, 1-Simplex
    int             MaxCalls = 500;
    int             MaxIteration = 1000;
    double          Tolerance = 0.001;

};

#endif // ASCRIPTMINIMIZERBASE_H
