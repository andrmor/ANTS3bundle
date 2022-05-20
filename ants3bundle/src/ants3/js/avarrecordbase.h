#ifndef AVARRECORDBASE_H
#define AVARRECORDBASE_H

#include <string>
#include <QString>

namespace ROOT { namespace Minuit2 { class Minuit2Minimizer; } }

class AVarRecordBase
{
public:
    virtual ~AVarRecordBase() {}

    virtual void AddToMinimizer(size_t varIndex, ROOT::Minuit2::Minuit2Minimizer * minimizer) = 0;
    virtual void Debug() const = 0;

protected:
    std::string Name;
    double      Value;
    double      Step;
    double      Min;
    double      Max;
};

class AVarRecordNormal : public AVarRecordBase
{
public:
    AVarRecordNormal(const QString & name, double start, double step);
    void AddToMinimizer(size_t varIndex, ROOT::Minuit2::Minuit2Minimizer * minimizer) override;
    void Debug() const override;
};

class AVarRecordFixed : public AVarRecordBase
{
public:
    AVarRecordFixed(const QString & name, double value);
    void AddToMinimizer(size_t varIndex, ROOT::Minuit2::Minuit2Minimizer * minimizer) override;
    void Debug() const override;
};

class AVarRecordLimited : public AVarRecordBase
{
public:
    AVarRecordLimited(const QString & name, double start, double step, double min, double max);
    void AddToMinimizer(size_t varIndex, ROOT::Minuit2::Minuit2Minimizer * minimizer) override;
    void Debug() const override;
};

class AVarRecordLowerLimited : public AVarRecordBase
{
public:
    AVarRecordLowerLimited(const QString & name, double start, double step, double min);
    void AddToMinimizer(size_t varIndex, ROOT::Minuit2::Minuit2Minimizer * minimizer) override;
    void Debug() const override;
};

class AVarRecordUpperLimited : public AVarRecordBase
{
public:
    AVarRecordUpperLimited(const QString & name, double start, double step, double max);
    void AddToMinimizer(size_t varIndex, ROOT::Minuit2::Minuit2Minimizer * minimizer) override;
    void Debug() const override;
};

#endif // AVARRECORDBASE_H
