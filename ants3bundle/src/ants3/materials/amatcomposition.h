#ifndef AMATCOMPOSITION_H
#define AMATCOMPOSITION_H

#include <QString>

#include <map>

//#include "TGeoElement.h"

class TGeoElement;

class AMatMixRecord
{
public:
    enum EFractionType {None, Molar, Mass};

    QString       Formula;
    EFractionType FractionType;
    double        Fraction = 1.0;

    std::map<TGeoElement*, double> ElementMap;
};

class AMatComposition
{
public:
    AMatComposition(){}

    bool parse(const QString & string);

    QString ErrorString;
    QString WarningString;

protected:
    std::vector<QString> CustomElementRecords;
    std::vector<QString> ParseStringByBracketLevel;

    bool checkChars();
    bool markCustomElements();
    bool splitByBracketLevel(QString & string);

    bool prepareMixRecords(const QString & expression, std::vector<AMatMixRecord> & result);
    bool parseMixRecord(AMatMixRecord & r);

private:
    QString ParseString;

    void clear();
};

#endif // AMATCOMPOSITION_H
