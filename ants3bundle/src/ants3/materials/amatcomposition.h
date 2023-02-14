#ifndef AMATCOMPOSITION_H
#define AMATCOMPOSITION_H

#include <QString>

#include <map>
#include <vector>

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
    std::vector<TGeoElement*> CustomElementRecords;
    std::vector<QString> ParseStringByBracketLevel;

    bool checkForbiddenChars();
    bool parseCustomElementRecords();
    bool splitByBracketLevel(QString & string);

    bool prepareMixRecords(const QString & expression, std::vector<AMatMixRecord> & result);
    bool parseMixRecord(AMatMixRecord & r);
    TGeoElement * makeCustomElement(const QString & strRec); // returns nullptr on error

private:
    QString ParseString;

    void clear();
};

#endif // AMATCOMPOSITION_H
