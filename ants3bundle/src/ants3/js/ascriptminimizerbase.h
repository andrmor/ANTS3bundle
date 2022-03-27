#ifndef ASCRIPTMINIMIZERBASE_H
#define ASCRIPTMINIMIZERBASE_H

#include "ascriptinterface.h"
#include "avarrecordbase.h"

#include <QObject>
#include <QVector>
#include <QVariant>

#include <string>

class MainWindow;
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

public slots:

    void           setHighPrecision(bool flag) {bHighPrecision = flag;}
    void           setVerbosity(int level) {PrintVerbosity = level;}

    void           clear();
    virtual void   setFunctorName(const QString & name) = 0;
    void           addVariable(QString name, double start, double step, double min, double max);
    void           addVariable(QString name, double start, double step);
    void           addFixedVariable(QString name, double value);
    void           addLowerLimitedVariable(QString name, double value, double step, double lowerLimit);
    void           addUpperLimitedVariable(QString name, double value, double step, double upperLimit);

    void           addAllVariables(QVariant array);

    void           setSimplex();
    void           setMigrad();

    bool           run();

    QVariant       getResults() const {return Results;}

protected:
    virtual void                  configureNumVariables(int num) = 0;
    virtual ROOT::Math::Functor * configureFunctor() = 0;

    QVector<AVarRecordBase*> Variables;
    QVariantList    Results;

    bool            bHighPrecision = false;
    int             PrintVerbosity = -1;
    int             Method = 0; // 0-Migrad, 1-Simplex

};

#endif // ASCRIPTMINIMIZERBASE_H
