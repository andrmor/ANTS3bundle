#include "avarrecordbase.h"

#include <QDebug>

#include "Minuit2/Minuit2Minimizer.h"

AVarRecordNormal::AVarRecordNormal(const QString & name, double start, double step)
{
    Name  = name.toLatin1().data();
    Value = start;
    Step  = step;
}

void AVarRecordNormal::AddToMinimizer(size_t varIndex, ROOT::Minuit2::Minuit2Minimizer * minimizer)
{
    minimizer->SetVariable(varIndex, Name, Value, Step);
}

void AVarRecordNormal::Debug() const
{
    qDebug() << "Normal" << Value << Step;
}

// ---

AVarRecordFixed::AVarRecordFixed(const QString & name, double value)
{
    Name = name.toLatin1().data();
    Value = value;
}

void AVarRecordFixed::AddToMinimizer(size_t varIndex, ROOT::Minuit2::Minuit2Minimizer * minimizer)
{
    minimizer->SetFixedVariable(varIndex, Name, Value);
}

void AVarRecordFixed::Debug() const
{
    qDebug() << "Fixed" << Value;
}

// ---

AVarRecordLimited::AVarRecordLimited(const QString & name, double start, double step, double min, double max)
{
    Name  = name.toLatin1().data();
    Value = start;
    Step  = step;
    Min   = min;
    Max   = max;
}

void AVarRecordLimited::AddToMinimizer(size_t varIndex, ROOT::Minuit2::Minuit2Minimizer * minimizer)
{
    minimizer->SetLimitedVariable(varIndex, Name, Value, Step, Min, Max);
}

void AVarRecordLimited::Debug() const
{
    qDebug() << "Limited" << Value << Step << Min << Max;
}

// ---

AVarRecordLowerLimited::AVarRecordLowerLimited(const QString & name, double start, double step, double min)
{
    Name  = name.toLatin1().data();
    Value = start;
    Step  = step;
    Min   = min;
}

void AVarRecordLowerLimited::AddToMinimizer(size_t varIndex, ROOT::Minuit2::Minuit2Minimizer * minimizer)
{
    minimizer->SetLowerLimitedVariable(varIndex, Name, Value, Step, Min);
}

void AVarRecordLowerLimited::Debug() const
{
    qDebug() << "LowerLimited" << Value << Step << Min;
}

// ---~

AVarRecordUpperLimited::AVarRecordUpperLimited(const QString & name, double start, double step, double max)
{
    Name  = name.toLatin1().data();
    Value = start;
    Step  = step;
    Max   = max;
}

void AVarRecordUpperLimited::AddToMinimizer(size_t varIndex, ROOT::Minuit2::Minuit2Minimizer * minimizer)
{
    minimizer->SetUpperLimitedVariable(varIndex, Name, Value, Step, Max);
}

void AVarRecordUpperLimited::Debug() const
{
    qDebug() << "UpperLimited" << Value << Step << Max;
}
