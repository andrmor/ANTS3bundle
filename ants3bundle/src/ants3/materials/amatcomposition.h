#ifndef AMATCOMPOSITION_H
#define AMATCOMPOSITION_H

#include <QString>

#include <map>
#include <vector>

class TGeoElement;
class TGeoMaterial;

class AMatMixRecord
{
public:
    enum EFractionType {None, Molar, Mass};

    QString       Formula;
    EFractionType FractionType;
    double        Fraction = 1.0;

    std::map<TGeoElement*, double> ElementMap;
    double CombinedA = 0;
    double ComputedFraction = 0;

    void computeA();
};

class AMatComposition
{
public:
    AMatComposition(){}

    QString CompositionString;
    std::map<TGeoElement*, double> ElementMap_AtomNumberFractions;
    std::map<TGeoElement*, double> ElementMap_MassFractions;
    QString ErrorString;

    bool parse(const QString & string);

    QString printComposition() const;

    TGeoMaterial * constructGeoMaterial(const QString & name, double density, double temperature);

    static QString geoMatToCompositionString(TGeoMaterial * mat);

protected:
    bool checkForbiddenChars();
    bool parseCustomElements();
    bool parseBracketedLevels();
    bool parseMixtures();

    bool prepareMixRecords(const QString & expression, std::vector<AMatMixRecord> & result);
    bool parseMolecule(AMatMixRecord & r);
    TGeoElement * makeCustomElement(const QString & strRec); // returns nullptr on error
    bool splitByBracketLevel(QString & string);
    bool mergeRecords(std::vector<AMatMixRecord> &recs, AMatMixRecord & result);
    TGeoElement * findElement(const QString & elementSymbol);

    // properties used during parsing of the composition string
    QString ParseString;
    std::vector<TGeoElement*> CustomElements;
    std::vector<std::pair<QString,AMatMixRecord>> MixtureByLevels;

    void clear();
};

#endif // AMATCOMPOSITION_H
