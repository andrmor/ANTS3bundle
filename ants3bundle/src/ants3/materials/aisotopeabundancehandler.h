#ifndef AISOTOPEABUNDANCEHANDLER_H
#define AISOTOPEABUNDANCEHANDLER_H

#include <QSet>
#include <QVector>
#include <QMap>
#include <QString>
#include <QPair>

class AChemicalElement;
class TGeoElement;
class TString;

class AIsotopeAbundanceHandler
{
    AIsotopeAbundanceHandler();

    AIsotopeAbundanceHandler(const AIsotopeAbundanceHandler&)            = delete;
    AIsotopeAbundanceHandler(AIsotopeAbundanceHandler&&)                 = delete;
    AIsotopeAbundanceHandler& operator=(const AIsotopeAbundanceHandler&) = delete;
    AIsotopeAbundanceHandler& operator=(AIsotopeAbundanceHandler&&)      = delete;

public:
    static AIsotopeAbundanceHandler & getInstance();

    void loadNaturalAbunances();

    bool isNaturalAbundanceTableEmpty() const {return NaturalAbundancies.isEmpty();}
    bool isElementExist(const QString& elSymbol) const {return AllPossibleElements.contains(elSymbol);}

    QStringList getListOfElements() const;
    int getZ(const QString & Symbol) const;
    QString getSymbol(int Z) const;

    const QString fillIsotopesWithNaturalAbundances(AChemicalElement & element) const;

    TGeoElement* generateTGeoElement(const AChemicalElement *el, const TString &matName) const; //does not own!

private:
    QVector<QString> ElementsByZ;
    QSet<QString> AllPossibleElements; //set with all possible element symbols until and including Einsteinium Es (99)
    QMap<QString, int> SymbolToNumber;

    QMap<QString, QVector<QPair<int, double> > > NaturalAbundancies; //Key - element name, contains QVector<mass, abund>

    bool isNatural(const AChemicalElement *el) const;
};

#endif // AISOTOPEABUNDANCEHANDLER_H
