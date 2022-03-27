#include "ascriptminimizerbase.h"

#include <QDebug>

#include "TMath.h"
#include "Math/Functor.h"
#include "Minuit2/Minuit2Minimizer.h"

AScriptMinimizerBase::AScriptMinimizerBase()
{
    Description = "Access to CERN ROOT minimizer";
}

AScriptMinimizerBase::~AScriptMinimizerBase()
{
    clear();
}

void AScriptMinimizerBase::clear()
{
    for (AVarRecordBase* r : Variables) delete r;
    Variables.clear();
}

void AScriptMinimizerBase::addVariable(QString name, double start, double step, double min, double max)
{
    Variables << new AVarRecordLimited(name, start, step, min, max);
}

void AScriptMinimizerBase::addVariable(QString name, double start, double step)
{
    Variables << new AVarRecordNormal(name, start, step);
}

void AScriptMinimizerBase::addFixedVariable(QString name, double value)
{
    Variables << new AVarRecordFixed(name, value);
}

void AScriptMinimizerBase::addLowerLimitedVariable(QString name, double value, double step, double lowerLimit)
{
    Variables << new AVarRecordLowerLimited(name, value, step, lowerLimit);
}

void AScriptMinimizerBase::addUpperLimitedVariable(QString name, double value, double step, double upperLimit)
{
    Variables << new AVarRecordUpperLimited(name, value, step, upperLimit);
}

void AScriptMinimizerBase::addAllVariables(QVariant array)
{
    //if (array.type() != QMetaType::QVariantList )
    if (array.type() != QVariant::List )
    {
        abort("DefineAllVariables(): has to be an array containing initializers of the variables");
        return;
    }

    clear();

    QVariantList vl = array.toList();
    if (vl.isEmpty())
    {
        abort("DefineAllVariables(): array of initializers is empty");
        return;
    }

    for (int i=0; i<vl.size(); i++)
    {
        //  qDebug() << "Adding variable #"<<i;
        QVariantList var = vl.at(i).toList();
        switch (var.size())
        {
        case 1:  // fixed
            Variables << new AVarRecordFixed(QString::number(i), var.at(0).toDouble());
            break;
        case 2:  // normal
            Variables << new AVarRecordNormal(QString::number(i), var.at(0).toDouble(), var.at(1).toDouble());
            break;
        case 4:
            qDebug() << var << var.at(2).isNull();
            if (var.at(2).isNull()) //upper limited
            {
                Variables << new AVarRecordUpperLimited(QString::number(i), var.at(0).toDouble(), var.at(1).toDouble(), var.at(3).toDouble());
                break;
            }
            else if (var.at(3).isNull()) //lower limited
            {
                Variables << new AVarRecordLowerLimited(QString::number(i), var.at(0).toDouble(), var.at(1).toDouble(), var.at(2).toDouble());
                break;
            }
            else
            {
                Variables << new AVarRecordLimited(QString::number(i), var.at(0).toDouble(), var.at(1).toDouble(), var.at(2).toDouble(), var.at(3).toDouble());
                break;
            }
        default:
            abort("DefineAllVariables(): variable definition arrays have to be of length 1, 2 or 4");
            return;
        }
        //  Variables.last()->Debug();
    }
}

void AScriptMinimizerBase::setSimplex()
{
    Method = 1;
}

void AScriptMinimizerBase::setMigrad()
{
    Method = 0;
}

bool AScriptMinimizerBase::run()
{
    if (Variables.size() == 0)
    {
        abort("Variables are not defined!");
        return false;
    }
    configureNumVariables(Variables.size());

    ROOT::Math::Functor * Funct = configureFunctor();
    if (!Funct)
    {
        abort("Minimization function is not defined!");
        return false;
    }

    ROOT::Minuit2::Minuit2Minimizer * RootMinimizer = new ROOT::Minuit2::Minuit2Minimizer( Method==0 ? ROOT::Minuit2::kMigrad : ROOT::Minuit2::kSimplex );
    RootMinimizer->SetMaxFunctionCalls(500);
    RootMinimizer->SetMaxIterations(1000);
    RootMinimizer->SetTolerance(0.001);
    RootMinimizer->SetPrintLevel(PrintVerbosity);
    RootMinimizer->SetStrategy( bHighPrecision ? 2 : 1 ); // 1 -> standard,  2 -> try to improve minimum (slower)

    RootMinimizer->SetFunction(*Funct);

    //setting up variables   -  start step min max etc
    for (int i = 0; i < Variables.size(); i++)
        Variables[i]->AddToMinimizer(i, RootMinimizer);

    //  qDebug() << "Starting minimization";
    bool fOK = RootMinimizer->Minimize();
//fOK = fOK && !ScriptManager->isEvalAborted(); // !!!***

    qDebug()<<"Minimization success? "<<fOK;

    Results.clear();
    if (fOK)
    {
        const double *VarVals = RootMinimizer->X();
        for (int i=0; i<Variables.size(); i++)
        {
            //  qDebug() << i << "-->--"<<VarVals[i];
            Results << VarVals[i];
        }
    }

    delete Funct;
    delete RootMinimizer;

    return fOK;
}
