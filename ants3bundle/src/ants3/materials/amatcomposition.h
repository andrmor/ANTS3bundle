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

    bool checkForbiddenChars();
    bool parseCustomElements();
    bool parseBracketedLevels();
    bool parseMixtures();

    bool prepareMixRecords(const QString & expression, std::vector<AMatMixRecord> & result);
    bool parseCompound(AMatMixRecord & r);
    TGeoElement * makeCustomElement(const QString & strRec); // returns nullptr on error
    bool splitByBracketLevel(QString & string);
    void mergeRecords(const std::vector<AMatMixRecord> & recs, AMatMixRecord & result);

protected:
    // properties used during parsing of the composition string
    QString ParseString;
    std::vector<TGeoElement*> CustomElements;
    std::vector<std::pair<QString,AMatMixRecord>> MixtureByLevels;

    void clear();
};

#endif // AMATCOMPOSITION_H
