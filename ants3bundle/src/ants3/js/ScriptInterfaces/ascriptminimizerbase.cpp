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

void AScriptMinimizerBase::setHighPrecision(bool flag)
{
    bHighPrecision = flag;
}

void AScriptMinimizerBase::setVerbosity(int level)
{
    PrintVerbosity = level;
}

void AScriptMinimizerBase::clear()
{
    for (AVarRecordBase * r : Variables) delete r;
    Variables.clear();
}

void AScriptMinimizerBase::addLimitedVariable(QString name, double start, double step, double min, double max)
{
    Variables.push_back( new AVarRecordLimited(name, start, step, min, max) );
}

void AScriptMinimizerBase::addVariable(QString name, double start, double step)
{
    Variables.push_back( new AVarRecordNormal(name, start, step) );
}

void AScriptMinimizerBase::addFixedVariable(QString name, double value)
{
    Variables.push_back( new AVarRecordFixed(name, value) );
}

void AScriptMinimizerBase::addLowerLimitedVariable(QString name, double value, double step, double lowerLimit)
{
    Variables.push_back( new AVarRecordLowerLimited(name, value, step, lowerLimit) );
}

void AScriptMinimizerBase::addUpperLimitedVariable(QString name, double value, double step, double upperLimit)
{
    Variables.push_back( new AVarRecordUpperLimited(name, value, step, upperLimit) );
}

void AScriptMinimizerBase::setSimplex()
{
    Method = 1;
}

void AScriptMinimizerBase::setMigrad()
{
    Method = 0;
}

void AScriptMinimizerBase::setMaxIterations(int num)
{
    MaxIteration = num;
}

void AScriptMinimizerBase::setMaxCalls(int num)
{
    MaxCalls = num;
}

void AScriptMinimizerBase::setTolerance(double val)
{
    Tolerance = val;
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
    RootMinimizer->SetMaxFunctionCalls(MaxCalls);
    RootMinimizer->SetMaxIterations(MaxIteration);
    RootMinimizer->SetTolerance(Tolerance);
    RootMinimizer->SetStrategy( bHighPrecision ? 2 : 1 ); // 1 -> standard,  2 -> try to improve minimum (slower)
    RootMinimizer->SetPrintLevel(PrintVerbosity);

    RootMinimizer->SetFunction(*Funct);

    for (size_t i = 0; i < Variables.size(); i++)
        Variables[i]->AddToMinimizer(i, RootMinimizer);

    bool OK = RootMinimizer->Minimize();
    OK = OK && !wasAborted();

    Results.clear();
    if (OK)
    {
        const double * VarVals = RootMinimizer->X();
        for (size_t i = 0; i < Variables.size(); i++)
            Results << VarVals[i];
    }

    delete Funct;
    delete RootMinimizer;

    return OK;
}

QVariantList AScriptMinimizerBase::getResults()
{
    return Results;
}
