#include "aisotopeabundancehandler.h"
#include "aisotope.h"
#include "achemicalelement.h"
#include "afiletools.h"

#include <QDebug>
#include <QRegularExpression>

#include "TGeoElement.h"

AIsotopeAbundanceHandler &AIsotopeAbundanceHandler::getInstance()
{
    static AIsotopeAbundanceHandler instance;
    return instance;
}

AIsotopeAbundanceHandler::AIsotopeAbundanceHandler()
{
    QVector<QString> all;
    all << "H"<<"He"<<"Li"<<"Be"<<"B"<<"C"<<"N"<<"O"<<"F"<<"Ne"<<"Na"<<"Mg"<<"Al"<<"Si"<<"P"<<"S"<<"Cl"<<"Ar"<<"K"<<"Ca"<<"Sc"<<"Ti"<<"V"<<"Cr"<<"Mn"<<"Fe"<<"Co"<<"Ni"<<"Cu"<<"Zn"<<"Ga"<<"Ge"<<"As"<<"Se"<<"Br"<<"Kr"<<"Rb"<<"Sr"<<"Y"<<"Zr"<<"Nb"<<"Mo"<<"Tc"<<"Ru"<<"Rh"<<"Pd"<<"Ag"<<"Cd"<<"In"<<"Sn"<<"Sb"<<"Te"<<"I"<<"Xe"<<"Cs"<<"Ba"<<"La"<<"Ce"<<"Pr"<<"Nd"<<"Pm"<<"Sm"<<"Eu"<<"Gd"<<"Tb"<<"Dy"<<"Ho"<<"Er"<<"Tm"<<"Yb"<<"Lu"<<"Hf"<<"Ta"<<"W"<<"Re"<<"Os"<<"Ir"<<"Pt"<<"Au"<<"Hg"<<"Tl"<<"Pb"<<"Bi"<<"Po"<<"At"<<"Rn"<<"Fr"<<"Ra"<<"Ac"<<"Th"<<"Pa"<<"U"<<"Np"<<"Pu"<<"Am"<<"Cm"<<"Bk"<<"Cf"<<"Es";
    all << "Fm"<<"Md"<<"No"<<"Lr"<<"Rf"<<"Db"<<"Sg"<<"Bh"<<"Hs";

    for (const QString& s : qAsConst(all))
        AllPossibleElements << s;

    for (int i=0; i<all.size(); i++)
        SymbolToNumber.insert(all[i], i+1);

    loadNaturalAbunances();
}

const QStringList AIsotopeAbundanceHandler::getListOfElements() const
{
    return SymbolToNumber.keys();
}

int AIsotopeAbundanceHandler::getZ(const QString &Symbol) const
{
    return SymbolToNumber[Symbol];
}

void AIsotopeAbundanceHandler::loadNaturalAbunances()
{
    NaturalAbundancies.clear();

    QString NaturalAbundances;
    bool bOK = ftools::loadTextFromFile(NaturalAbundances, ":/data/data/NaturalAbundance.txt");
    if (!bOK)
    {
            qCritical() << "Cannot load file with natural abundances!";
    }
    else
    {
        const QStringList SL = NaturalAbundances.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
        for (const QString & s : SL)
        {
            QStringList f = s.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (f.size() != 2) continue;
            bool bOK = true;
            double Abundancy = (f.last()).toDouble(&bOK);
            if (!bOK) continue;
            QString mass = f.first();
            mass.remove(QRegularExpression("[a-zA-Z]"));
            int Mass = mass.toInt(&bOK);
            if (!bOK) continue;
            QString Element = f.first();
            Element.remove(QRegularExpression("[0-9]"));
            //qDebug() << Element << Mass << Abundancy;
            QVector<QPair<int, double> > tmp;
            tmp << QPair<int, double>(Mass, Abundancy);
            if (NaturalAbundancies.contains(Element)) (NaturalAbundancies[Element]) << tmp;
            else NaturalAbundancies[Element] = tmp;
        }
    }
}

bool AIsotopeAbundanceHandler::isNatural(const AChemicalElement *el) const
{
    const QString name = el->Symbol;
    if (!NaturalAbundancies.contains(name)) return false;

    QVector<QPair<int, double> > list = NaturalAbundancies[name];
    if (list.size() != el->countIsotopes()) return false;

    for (int i=0; i<list.size(); i++)
    {
        const QPair<int, double> & p = list[i];
        if (p.first != el->Isotopes[i].Mass) return false;
        if ( std::abs(p.second - el->Isotopes[i].Abundancy)/p.second > 0.001) return false; // 0.1% tolerance
    }
    return true;
}

const QString AIsotopeAbundanceHandler::fillIsotopesWithNaturalAbundances(AChemicalElement & element) const
{
    QString& name = element.Symbol;
    if (!NaturalAbundancies.contains(name))
        return QString("Element ") + name + " not found in dataset of natural abundances";

    element.Isotopes.clear();
    QVector<QPair<int, double> > list = NaturalAbundancies[name];
    for (auto& pair : list)
        element.Isotopes << AIsotope(name, pair.first, pair.second);
    return "";
}

TGeoElement *AIsotopeAbundanceHandler::generateTGeoElement(const AChemicalElement *el, const TString& matName) const
{
    TString tName(el->Symbol.toLatin1().data());

    //strategy: if isotope composition is natural, return standard root element (no isotopes)
    //otherwise return custom element with isotope composition

    TGeoElement* geoEl = 0;
    if ( isNatural(el) ) geoEl = TGeoElement::GetElementTable()->FindElement(tName);
    if (geoEl) return geoEl;

    tName += '_';
    tName += matName;

    geoEl = new TGeoElement(tName, tName, el->countIsotopes());
    int Z = SymbolToNumber[el->Symbol];
    for (const AIsotope& iso : el->Isotopes)
    {
        const TString thisIsoName = iso.getTName();
        TGeoIsotope *geoIsotope = TGeoIsotope::FindIsotope(thisIsoName);
        if (!geoIsotope)
            geoIsotope = new TGeoIsotope(thisIsoName, Z, iso.Mass, iso.Mass);
        geoEl->AddIsotope(geoIsotope, iso.Abundancy);
    }
    return geoEl;
}
