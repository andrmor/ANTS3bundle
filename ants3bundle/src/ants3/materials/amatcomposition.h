#ifndef AMATCOMPOSITION_H
#define AMATCOMPOSITION_H

#include <QString>

#include <map>
#include <vector>

class TGeoElement;
class TGeoMaterial;
class QJsonObject;

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
    AMatComposition();

    QString ErrorString;

    bool setCompositionString(const QString & composition);
    void makeItVacuum();

    QString getCompositionString() const {return CompositionString;}
    QString printComposition() const;

    TGeoMaterial * constructGeoMaterial(const QString & name, double density, double temperature);

    void writeToJson(QJsonObject & json) const;
    bool readFromJson(const QJsonObject & json);

    static QString geoMatToCompositionString(TGeoMaterial * mat);

protected:
    bool parse(const QString & string);
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

    QString CompositionString;
    std::map<TGeoElement*, double> ElementMap_AtomNumberFractions;
    std::map<TGeoElement*, double> ElementMap_MassFractions;

    // properties used during parsing of the composition string
    QString ParseString;
    std::vector<TGeoElement*> CustomElements;
    std::vector<std::pair<QString,AMatMixRecord>> MixtureByLevels;

    void clear();
};

#endif // AMATCOMPOSITION_H
