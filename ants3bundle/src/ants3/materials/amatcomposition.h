#ifndef AMATCOMPOSITION_H
#define AMATCOMPOSITION_H

#include <QString>

#include <map>
#include <vector>

class TGeoElement;
class TGeoMaterial;
class QJsonObject;

class AElementRecord
{
public:
    QString Symbol;
    std::vector<std::pair<int,double>> Isotopes;  // [N,atomicFraction]; if empty, use natural;
    double A;

    bool operator<(const AElementRecord & other) const {return (Symbol < other.Symbol);}
    TGeoElement * constructGeoElement() const;
    bool isIdentical(const AElementRecord & other) const;
};

class AMatMixRecord
{
public:
    enum EFractionType {None, Molar, Mass};

    QString       Formula;
    EFractionType FractionType;
    double        Fraction = 1.0;

    std::map<AElementRecord, double> ElementMap;
    double CombinedA = 0;
    double ComputedFraction = 0;

    void computeA();
};

class AMatComposition
{
public:
    AMatComposition();

    double  Density      = 1e-25;     // g/cm3
    double  Temperature  = 298.0;     // K
    bool    Gas          = false;
    double  Pressure_bar = 1.0;
    QString P_gui_units   = "bar";

    bool   UseCustomMeanExEnergy = false;
    double MeanExEnergy = 0;        // eV

    bool setCompositionString(const QString & composition);
    void makeItVacuum();

    QString getCompositionString() const {return CompositionString;}
    QString printComposition() const;

    TGeoMaterial * constructGeoMaterial(const QString & name);

    void writeToJson(QJsonObject & json) const;
    bool readFromJson(const QJsonObject & json); // !!!*** error control

    bool importComposition(TGeoMaterial * mat);

    QString convertPressureToDensity(); // returns error string

    QString ErrorString;

protected:
    void clearParsing();
    bool parse(const QString & string);
    bool checkForbiddenChars();
    bool parseCustomElements();
    bool parseBracketedLevels();
    bool parseMixtures();

    bool prepareMixRecords(const QString & expression, std::vector<AMatMixRecord> & result);
    bool parseMolecule(AMatMixRecord & r);
    bool makeCustomElement(const QString & strRec, AElementRecord & elm); // returns nullptr on error
    bool splitByBracketLevel(QString & string);
    bool mergeRecords(std::vector<AMatMixRecord> &recs, AMatMixRecord & result);
    bool fetchElement(const QString & elementSymbol, AElementRecord & elm);
    bool checkIsotope(const QString & isotopeSymbol, const int & isotopeN);
    void updateCustomElementSymbol(AElementRecord &elm);

    QString CompositionString;
    std::map<AElementRecord, double> ElementMap_AtomNumberFractions;
    std::map<AElementRecord, double> ElementMap_MassFractions;

    // properties used during parsing of the composition string
    QString ParseString;
    std::vector<AElementRecord> CustomElements;
    std::vector<std::pair<QString,AMatMixRecord>> MixtureByLevels;

};

#endif // AMATCOMPOSITION_H
