#include "amatcomposition.h"
#include "ajsontools.h"

#include <QDebug>

#include "TGeoMaterial.h"
#include "TGeoElement.h"

void AMatComposition::clearParsing()
{
    ErrorString.clear();

    CompositionString.clear();
    ParseString.clear();

    CustomElements.clear();
    MixtureByLevels.clear();
}

AMatComposition::AMatComposition()
{
    makeItVacuum();
}

bool AMatComposition::setCompositionString(const QString & composition)
{
    bool ok = parse(composition);
    if (!ok)
    {
        QString err = ErrorString;
        makeItVacuum();
        ErrorString = err;
    }
    return ok;
}

void AMatComposition::makeItVacuum()
{
    clearParsing();

    AElementRecord r; r.Symbol = "H"; r.A = 1.00784;
    ElementMap_AtomNumberFractions[r] = 1.0;
    ElementMap_MassFractions[r] = 1.0;
    CompositionString = "H";
    Density = 1e-24;
    Temperature = 298.0;
    UseCustomMeanExEnergy = false;
    MeanExEnergy = 0;
}

bool AMatComposition::parse(const QString & string)
{
    clearParsing();

    CompositionString = string;
    ParseString = string.simplified();
    //qDebug() << "\n->Composition to parse:" << ParseString;

    bool ok = checkForbiddenChars();
    if (!ok) return false;

    ok = parseCustomElements(); // elements with custom isotope composition
    if (!ok) return false;

    //qDebug() << "->After custom element parsing, the string is:" << ParseString << "\n";

    // pre-parsing brackets to expression levels
    MixtureByLevels.push_back({ParseString, AMatMixRecord()});
    ok = parseBracketedLevels();
    if (!ok) return false;

    //qDebug() << "\n->MixtureByLevels have the following records:";
    //for (const auto & p : MixtureByLevels) qDebug() << "      " << p.first;

    ok = parseMixtures();
    if (!ok) return false;

    //qDebug() << "<=== parse finished ===>";

    ElementMap_AtomNumberFractions = MixtureByLevels.front().second.ElementMap;
    MixtureByLevels.front().second.computeA();
    double combinedA = MixtureByLevels.front().second.CombinedA;

    ElementMap_MassFractions.clear();
    for (const auto & pair : ElementMap_AtomNumberFractions)
    {
        const AElementRecord & element       = pair.first;
        const double         & molarFraction = pair.second;
        const double massFraction = molarFraction * element.A / combinedA;
        ElementMap_MassFractions[element] = massFraction;
    }
    return true;
}

QString AMatComposition::printComposition() const
{
    QString str = "Element\tAtomFraction\tMassFraction\tIsotopes\n";

    bool bAllInt = true;
    double totAtFraction = 0;
    for (const auto & kv : ElementMap_AtomNumberFractions)
    {
        double  atFraction = kv.second;
        totAtFraction += atFraction;
        if (atFraction != std::floor(atFraction)) bAllInt = false;
    }
    if (bAllInt) totAtFraction = 1.0;

    for (const auto & kv : ElementMap_AtomNumberFractions)
    {
        const AElementRecord & ele = kv.first;
        QString baseSymbol = ele.Symbol;
        baseSymbol.resize(2); if (baseSymbol[1] == '_') baseSymbol.chop(1);
        double  atFraction = kv.second / totAtFraction;
        double  maFraction = ElementMap_MassFractions.at(ele);

        QString isoStr;
        const size_t numIso = ele.Isotopes.size();
        QString modSymbol = baseSymbol;
        if (numIso == 0) isoStr = "natural";
        else
        {
            for (const std::pair<int,double> & pair : ele.Isotopes)
            {
                const int    & isoN        = pair.first;
                const double & isoFraction = pair.second;
                QString name = QString::number(isoN) + baseSymbol;
                isoStr += name + ":" + QString::number(isoFraction) + "+";
            }
            isoStr.chop(1);
            modSymbol += "*";
        }

        str += modSymbol + '\t' + QString::number(atFraction) + '\t' + QString::number(maFraction) + '\t' + isoStr + '\n';
    }
    return str;
}

TGeoMaterial * AMatComposition::constructGeoMaterial(const QString & name)
{
    TString tName = name.toLatin1().data();

    TGeoMixture * mix = new TGeoMixture(tName, ElementMap_MassFractions.size(), Density);
    for (const auto & pair : ElementMap_MassFractions)
    {
        const AElementRecord & element      = pair.first;
        const double         & weightFactor = pair.second;
        TGeoElement * geoElm = element.constructGeoElement();
        mix->AddElement(geoElm, weightFactor);
    }
    mix->SetTemperature(Temperature);
    return mix;
}

#include <QRegularExpression>
bool AMatComposition::checkForbiddenChars()
{
    const QRegularExpression letter("[A-Za-z]");

    for (int i = 0; i < ParseString.size(); i++)
    {
        const QChar ch = ParseString[i];
        //qDebug() << "----------------->" << ch << ch.isLetterOrNumber() << ch.isLetter() << cha.match(ch).hasMatch();
        //if (ch.isLetterOrNumber()) continue; // problems with some Portuguese characters - they get through
        if (ch.isNumber()) continue;
        if (letter.match(ch).hasMatch()) continue;
        if (ch == '.') continue;
        if (ch == ' ' || ch == '+') continue;
        if (ch == ':' || ch == '/') continue;
        if (ch == '(' || ch == ')' || ch == '{' || ch == '}') continue;

        ErrorString = "Not allowed char in composition string: " + QString(ch);
        return false;
    }
    return true;
}

bool AMatComposition::parseCustomElements()
{
    //qDebug() << "->Searching for custom element records containing isotope composition";
    int start = 0;
    int pos = 0;
    bool bReadingRecord = false;
    while (pos < ParseString.length())
    {
        const QChar ch = ParseString[pos];
        //qDebug() << "  ch>" << ch;

        if (ch == '{')
        {
            if (bReadingRecord)
            {
                ErrorString += "Double open custom element record [{{]";
                return false;
            }
            start = pos;
            bReadingRecord = true;
            pos++;
            continue;
        }

        if (ch == '}')
        {
            if (!bReadingRecord)
            {
                ErrorString += "Close custom element record without start [}]";
                return false;
            }
            bReadingRecord = false;

            const int selectionSize = pos - start + 1;
            QString rec = ParseString.mid(start, selectionSize);
            //qDebug() << "  Found custom element record:" << rec;
            const QString replaceWith = QString("#i%0&").arg(CustomElements.size());
            //qDebug() << "  Replacing with:" << replaceWith;
            ParseString.replace(start, selectionSize, replaceWith);

            rec.remove(0, 1); rec.chop(1); // killing { and }
            AElementRecord elm;
            bool ok = makeCustomElement(rec, elm);
            if (!ok) return false;
            CustomElements.push_back(elm);

            //qDebug() << "    Continue to look for custom elements in string:" << ParseString;
            pos = pos - selectionSize + replaceWith.length() + 1;
            continue;
        }

        if (bReadingRecord)
        {
            if (ch == '/')
            {
                ErrorString = "mass fractions are not alowed in custom element record: " + QString(ch);
                return false;
            }
        }

        pos++;
    }

    if (bReadingRecord)
    {
        ErrorString += "Unfinished custom element record [missing }]";
        return false;
    }

    //qDebug() << "-->Found custom element records:" << CustomElements;
    return true;
}

bool AMatComposition::parseBracketedLevels()
{
    for (size_t iR = 0; iR < MixtureByLevels.size(); iR++)
    {
        QString copy = MixtureByLevels[iR].first;
        bool ok = splitByBracketLevel(copy);
        MixtureByLevels[iR].first = copy;
        if (!ok) return false;
    }
    return true;
}

bool AMatComposition::parseMixtures()
{
    const size_t numLevels = MixtureByLevels.size();
    if (numLevels < 1)
    {
        ErrorString = "Fatal error: Detected zero levels during parseMixture()";
        return false;
    }

    for (size_t iLevel = numLevels; iLevel-- > 0; )
    {
        std::vector<AMatMixRecord> recs;
        bool ok = prepareMixRecords(MixtureByLevels[iLevel].first, recs);
        if (!ok) return false;

        if (recs.empty())
        {
            ErrorString = "Fatal error: Detected zero compounds during parseMixture()";
            return false;
        }

        for (AMatMixRecord & r : recs)
        {
            ok = parseMolecule(r);
            if (!ok) return false;
        }

        ok = mergeRecords(recs, MixtureByLevels[iLevel].second);
        if (!ok) return false;
    }

    return true;
}

bool AMatComposition::splitByBracketLevel(QString & string)
{
    //qDebug() << "-->Checking for bracketed sub-records";
    int start = 0;
    int pos = 0;
    bool bReadingRecord = false;
    int level = 0;
    while (pos < string.length())
    {
        const QChar ch = string[pos];
        //qDebug() << "  br>" << ch;

        if (ch == '(')
        {
            if (bReadingRecord)
            {
                level++;
            }
            else
            {
                start = pos;
                bReadingRecord = true;
            }
            pos++;
            continue;
        }

        if (ch == ')')
        {
            if (!bReadingRecord)
            {
                ErrorString += "Close bracket without start [))]";
                return false;
            }

            if (level > 0)
            {
                level--;
                pos++;
                continue;
            }

            bReadingRecord = false;

            const int selectionSize = pos - start + 1;
            QString rec = string.mid(start, selectionSize);
            //qDebug() << "    Found bracketed record:" << rec;
            const QString replaceWith = QString("#b%0&").arg(MixtureByLevels.size());
            //qDebug() << "     =>Replacing with:" << replaceWith;
            string.replace(start, selectionSize, replaceWith);
            //qDebug() << "    Continue with the string:" << string;

            rec.remove(0, 1); rec.chop(1);
            MixtureByLevels.push_back( {rec, AMatMixRecord()} );

            pos = pos - selectionSize + replaceWith.length() + 1;
            continue;
        }

        pos++;
    }

    if (bReadingRecord)
    {
        ErrorString += "Not closed bracket [missing )]";
        return false;
    }

    //qDebug() << "    =>Returning string:" << string;
    return true;
}

bool AMatComposition::prepareMixRecords(const QString & expression, std::vector<AMatMixRecord> & result)
{
    // e.g. H2O:80+NaCl:10+#i0&4C:10

    QString str = expression.simplified();
    if (str.isEmpty())
    {
        ErrorString = "Format error: Detected empty record";
        return false;
    }

    str.replace(' ', '+');
    //qDebug() << "Processing mixture record:" << str;

    const QStringList list = str.split('+', Qt::SkipEmptyParts ); //split to fields of formula:fraction
    for (const QString & str : list)
    {
        //qDebug() << str;
        bool haveMolar = str.contains(':');
        bool haveMass  = str.contains('/');
        if (haveMolar && haveMass)
        {
            ErrorString = "Composition record cannot contain both molar [:] and mass [/] fraction symbols";
            return false;
        }

        AMatMixRecord rec;
        if ( !haveMolar && !haveMass)
        {
            rec.Formula = str;
            rec.FractionType = AMatMixRecord::None;
            rec.Fraction = 1.0;
        }
        else
        {
            QChar separator = (haveMolar ? ':' : '/');
            QStringList wList = str.split(separator);
            if (wList.isEmpty() || wList.size() > 2)
            {
                ErrorString = "Format error in composition string: need both formula and fraction";
                return false;
            }

            rec.Formula = wList.first();
            rec.FractionType = ( haveMolar ? AMatMixRecord::Molar : AMatMixRecord::Mass );

            bool ok;
            rec.Fraction = wList.last().toDouble(&ok);
            if (!ok)
            {
                ErrorString = "Format error in composition string: cannot extract fraction";
                return false;
            }
        }
        result.push_back(rec);
    }

    if (result.empty())
    {
        ErrorString = "Format error in composition string: empty field";
        return false;
    }

    if (result.size() == 1)
    {
        if (result.front().FractionType == AMatMixRecord::None)
            result.front().FractionType = AMatMixRecord::Molar;
    }
    else
    {
        bool seenMolar = false;
        bool seenMass  = false;

        for (const AMatMixRecord & r : result)
        {
            if      (r.FractionType == AMatMixRecord::Molar) seenMolar = true;
            else if (r.FractionType == AMatMixRecord::Mass)  seenMass  = true;
        }

        if (seenMolar && seenMass)
        {
            ErrorString = "Composition string of one level (e.q. inside parenthesis) cannot have a mixture of definition by molar and by weight!";
            return false;
        }
        if (!seenMolar && !seenMass) seenMolar = true;

        for (AMatMixRecord & r : result)
        {
            if (seenMolar && r.FractionType == AMatMixRecord::None) r.FractionType = AMatMixRecord::Molar;
            if (seenMass  && r.FractionType == AMatMixRecord::None) r.FractionType = AMatMixRecord::Mass;
        }
    }
    return true;
}

bool AMatComposition::parseMolecule(AMatMixRecord & r)
{
    // e.g. H2O
    // Formula can contain custom isotopes! e.g. #i0&4C or Li#i2&2
    // Formula can be already processed sub-mixture, e.g. #b1&

    //qDebug() << " ->Parsing molecule||sub-mixture:" << r.Formula << " with" << (r.FractionType == AMatMixRecord::Molar ? "molar" : "mass") << "fraction:" << r.Fraction;
    QString formula = r.Formula.simplified() + ":"; //":" is end signal

    if (formula.startsWith("#b"))
    {
        formula.remove("#b");
        formula.remove("&:");
        bool ok;
        int index = formula.toInt(&ok);
        if (!ok || index < 0 || index >= (int)MixtureByLevels.size())
        {
            ErrorString = "Format error in braket expression director";
            return false;
        }

        AMatMixRecord & mixture = MixtureByLevels[index].second;
        if (r.FractionType == AMatMixRecord::Molar && mixture.FractionType == AMatMixRecord::Mass)
        {
            ErrorString = "Cannot use molar fractions for mass-fraction mixtures";
            return false;
        }

        r.Formula = mixture.Formula;
        r.ElementMap = mixture.ElementMap;
        return true;
    }

    const QChar firstChar = formula.front();
    const bool validStart = ( (firstChar.isLetter() && firstChar.isUpper()) || firstChar == '#' );
    if (!validStart)
    {
        ErrorString = "Format error in composition: " + formula;
        return false;
    }

    bool bReadingElementName = true;
    QString tmp(formula.front());
    QString ElementSymbol;
    for (int i = 1; i < formula.size(); i++)
    {
        const QChar c = formula[i];
        if (bReadingElementName)
        {
            if (tmp.front() == '#')
            {
                // custom element
                if (tmp.back() != '&')
                {
                    tmp += c;
                    continue;
                }
            }
            else
            {
                // standard symbol
                if (c.isLetter() && c.isLower()) //continue to acquire the name
                {
                    tmp += c;
                    continue;
                }
            }

            if (c == ':' || c.isDigit() || (c.isLetter() && c.isUpper()) || c == '#')
            {
                if (tmp.isEmpty()) return "Format error in composition: unrecognized character";
                ElementSymbol = tmp;
                if (!c.isDigit())
                {
                    // stat weight of 1
                    AElementRecord elm;
                    bool ok = fetchElement(ElementSymbol, elm);
                    if (!ok)
                    {
                        ErrorString = "Element does not exist: " + ElementSymbol;
                        return false;
                    }
                    auto it = r.ElementMap.find(elm);
                    if (it == r.ElementMap.end()) r.ElementMap[elm] = 1.0;
                    else it->second += 1.0;
                    if (c == ':') break; // end of compound record
                }
                tmp = QString(c);
                if (c.isDigit()) bReadingElementName = false;
            }
            else return "Format error in composition: unrecognized character";
        }
        else
        {
            //reading number
            if (c.isDigit() || c == '.')
            {
                tmp += c;
                continue;
            }
            else if (c ==':' || c.isLetter() || c == '#')
            {
                if (c.isLower())
                {
                    ErrorString = "Format error in composition: lower register symbol in the first character of element symbol";
                    return false;
                }
                bool ok;
                double number = tmp.toDouble(&ok);
                if (!ok)
                {
                    ErrorString = "Format error in composition: record is not a number"; // paranoic
                    return false;
                }
                AElementRecord elm;
                ok = fetchElement(ElementSymbol, elm);
                if (!ok)
                {
                    ErrorString = "Element does not exist: " + ElementSymbol;
                    return false;
                }
                auto it = r.ElementMap.find(elm);
                if (it == r.ElementMap.end()) r.ElementMap[elm] = number;
                else it->second += number;
                if (c == ':') break;
                tmp = QString(c);
                bReadingElementName = true;
            }
            else
            {
                ErrorString = "Format error in composition: unrecognized character";
                return false;
            }
        }
    }
    return true;
}

bool AMatComposition::fetchElement(const QString & elementSymbol, AElementRecord & elm)
{
    if (elementSymbol.startsWith("#i"))
    {
        QString str = elementSymbol;
        str.remove("#i");
        str.remove("&");
        bool ok;
        int recNum = str.toInt(&ok);
        if (recNum < 0 || recNum >= (int)CustomElements.size()) return false;
        elm = CustomElements[recNum];
        return true;
    }

    TGeoElement * geoElement = TGeoElement::GetElementTable()->FindElement(elementSymbol.toLatin1().data());
    if (geoElement)
    {
        elm.Symbol = elementSymbol;
        elm.Isotopes.clear();
        elm.A = geoElement->A();
        return true;
    }

    return false;
}

bool AMatComposition::checkIsotope(const QString & isotopeSymbol, const int & isotopeN)
{
    TGeoElement * geoElement = TGeoElement::GetElementTable()->FindElement(isotopeSymbol.toLatin1().data());
    if (!geoElement)
    {
        ErrorString = "Unknown element in custom element record: " + isotopeSymbol;
        return false;
    }
    int elementZ = geoElement->Z();
    if (isotopeN < elementZ)
    {
        ErrorString = "Isotope cannot have number of nuclons less than the charge: " + isotopeSymbol;
        return false;
    }
    return true;
}

bool AMatComposition::makeCustomElement(const QString & strRec, AElementRecord & elm)
{
    // 10B:95+11B:5
    QString fullStr = strRec.simplified().replace(' ', '+');
    //qDebug() << "    Processing custom element record:" << fullStr;

    QString elementSymbol; // !!!*** need? use directly elm.Symbol
    elm.Isotopes.clear();

    const QStringList list = fullStr.split('+', Qt::SkipEmptyParts ); //split to fields of NElement:Fraction
    if (list.isEmpty())
    {
        ErrorString = "Custom element record is empty";
        return false;
    }

    for (const QString & str : list)
    {
        //qDebug() << "      Isotope record:" << str;
        if (!str.front().isDigit() || str.size() < 2)
        {
            ErrorString = "Custom element record should contain at least one isotope (e.g. 10B)";
            return false;
        }
        enum EState {StateN, StateSymbol, StateFraction};
        EState State = StateN;
        QString strN; int isotopeN;
        QString isotopeSymbol;
        QString strFraction; double fraction;
        bool ok;
        for (QChar ch : str)
        {
            //qDebug() << "      i>" << ch;
            if (State == StateN)
            {
                if (ch.isDigit())
                {
                    strN += ch;
                    continue;
                }
                isotopeN = strN.toInt(&ok);
                if (!ok)
                {
                    ErrorString = "Custom element record: error extracting isotope N (use e.g. 10B or 10B:97+11B:3)";
                    return false;
                }
                State = StateSymbol;
            }
            if (State == StateSymbol)
            {
                if (ch.isLetter())
                {
                    isotopeSymbol += ch;
                    continue;
                }
                if (ch != ':')
                {
                    ErrorString = "Isotope symbol should be followed by a ':' character";
                    return false;
                }

                if (elementSymbol.isEmpty()) elementSymbol = isotopeSymbol;
                else if (isotopeSymbol != elementSymbol)
                {
                    ErrorString = "All isotopes of a custom element should have the same symbol";
                    return false;
                }
                ok = checkIsotope(elementSymbol, isotopeN);
                if (!ok) return false;

                State = StateFraction;
                if (ch == ':') continue;
            }
            if (State == StateFraction)
            {
                if (ch.isDigit() || ch == '.')
                {
                    strFraction += ch;
                    continue;
                }
                ErrorString = "Custom element record: error in extracting fraction";
                return false;
            }
        }

        if (State != StateFraction)
        {
            if (list.size() == 1 && State == StateSymbol)
            {
                // for a single isotope allow skipping the fraction info
                elementSymbol = isotopeSymbol;
                ok = checkIsotope(elementSymbol, isotopeN);
                if (!ok) return false;
                strFraction = "1.0";
            }
            else
            {
                ErrorString = "Custom element record: error extracting isotope fraction (use e.g. 10B or 10B:97+11B:3)";
                return false;
            }
        }
        fraction = strFraction.toDouble(&ok);
        if (!ok)
        {
            ErrorString = "Custom element record: fraction format error";
            return false;
        }

        elm.Isotopes.push_back({isotopeN, fraction});
    }

    elm.Symbol = elementSymbol;

    double sumA  = 0;
    double sumFr = 0;
    for (const auto & pair : elm.Isotopes)
    {
        sumA  += pair.first * pair.second;
        sumFr += pair.second;
    }
    if (sumA == 0 || sumFr == 0)
    {
        ErrorString = "Custom element record: error in sum of fractions (should be positive)";
        return false;
    }
    elm.A = sumA / sumFr;

    updateCustomElementSymbol(elm);

    return true;
}

void AMatComposition::updateCustomElementSymbol(AElementRecord & elm)
{
    // if there is identical, use the same symbol
    for (const AElementRecord & def : CustomElements)
    {
        if (elm.isIdentical(def))
        {
            elm.Symbol = def.Symbol;
            return;
        }
    }

    // symbol should be unique
    QString base = elm.Symbol + "_Custom";
    elm.Symbol = base;
    int index = 0;
    while (true)
    {
        bool bUnique = true;
        for (const AElementRecord & def : CustomElements)
        {
            if (def.Symbol == elm.Symbol)
            {
                bUnique = false;
                break;
            }
        }

        if (bUnique) return;

        index++;
        elm.Symbol = base + QString::number(index);
    }
}

bool AMatComposition::mergeRecords(std::vector<AMatMixRecord> & recs, AMatMixRecord & result)
{
    if (recs.empty())
    {
        ErrorString = "Unexpected empty record list to merge";
        return false;
    }

    if (recs.size() == 1)
    {
        result = recs.front();

        if (recs.front().FractionType == AMatMixRecord::Molar)
        {
            for (auto & kv : result.ElementMap)
                kv.second *= result.Fraction;
        }
        else
        {
            ErrorString = "Sub-expression cannot have mass fraction for a single member";
            return false;
        }
        return true;
    }

    AMatMixRecord::EFractionType Type = recs.front().FractionType;

    // checking attempt to use molar fractions with mass fraction sub-records

    if (Type == AMatMixRecord::Molar)
    {
        result = recs.front();
        for (auto & kv : result.ElementMap)
            kv.second *= result.Fraction;

        for (size_t iRec = 1; iRec < recs.size(); iRec++)
        {
            const AMatMixRecord & other = recs[iRec];
            if (other.FractionType != AMatMixRecord::Molar) // paranoic: cannot happen here
            {
                ErrorString = "Cannot use molar fractions for mass-fraction mixtures";
                return false;
            }

            for (const auto & kv : other.ElementMap)
            {
                auto it = result.ElementMap.find(kv.first);
                if (it == result.ElementMap.end()) result.ElementMap[kv.first] = kv.second * other.Fraction;
                else it->second += kv.second * other.Fraction;
            }
        }
    }
    else
    {
        for (AMatMixRecord & r : recs)
        {
            r.computeA();
            r.ComputedFraction = r.Fraction / r.CombinedA;
        }

        result = recs.front();
        for (auto & kv : result.ElementMap)
            kv.second *= result.ComputedFraction;

        for (size_t iRec = 1; iRec < recs.size(); iRec++)
        {
            const AMatMixRecord & other = recs[iRec];
            for (const auto & kv : other.ElementMap)
            {
                auto it = result.ElementMap.find(kv.first);
                if (it == result.ElementMap.end()) result.ElementMap[kv.first] = kv.second * other.ComputedFraction;
                else it->second += kv.second * other.ComputedFraction;
            }
        }
    }
    return true;
}

void AMatMixRecord::computeA()
{
    CombinedA = 0;
    for (const auto & kv : ElementMap)
    {
        const AElementRecord & elm = kv.first;
        const double         & fraction = kv.second;
        CombinedA += elm.A * fraction;
    }
}

bool AMatComposition::importComposition(TGeoMaterial * mat)
{
    clearParsing();

    Density     = mat->GetDensity();
    Temperature = mat->GetTemperature();

    QString compoString;

    //qDebug() << mat->GetName() << mat->IsMixture(); // !!!*** generalize to mixture
    const int numElements = mat->GetNelements();
    double A, Z, W;

    for (int iEl = 0; iEl < numElements; iEl++)
    {
        mat->GetElementProp(A, Z, W, iEl);
        TGeoElement * geoEl = mat->GetElement(iEl);
        QString elName = TGeoElement::GetElementTable()->GetElement(Z)->GetName();
        if (elName.size() == 2) elName[1] = elName[1].toLower();

        int numIso = geoEl->GetNisotopes();
        if (numIso != 0)
        {
            // custom isotope composition
            QString symbol = elName;
            elName = '{';
            for (int iIso = 0; iIso < numIso; iIso++)
            {
                double isoW = geoEl->GetRelativeAbundance(iIso);
                TGeoIsotope * geoIso = geoEl->GetIsotope(iIso);
                int isoN = geoIso->GetN();
                elName += QString::number(isoN);
                elName += symbol;
                elName += ':';
                elName += QString::number(isoW);
                elName += '+';
            }
            elName.chop(1);
            elName += '}';
        }
        compoString += elName + "/" + QString::number(W) + " + ";
    }

    if (compoString.endsWith(" + ")) compoString.chop(3);

    return setCompositionString(compoString);
}

void AMatComposition::writeToJson(QJsonObject & json) const
{
    QJsonObject js;
        js["CompositionString"]       = CompositionString;
        js["Density"]                 = Density;
        js["Temperature"]             = Temperature;
        js["UseMeanExcitationEnergy"] = UseCustomMeanExEnergy;
        js["MeanExcitationEnergy_eV"] = MeanExEnergy;
    json["CustomComposition"] = js;
}

bool AMatComposition::readFromJson(const QJsonObject & json)
{
#ifdef NOT_NEED_MAT_COMPOSITION
    // this is lsim
    makeItVacuum();
    return true;
#else
    clearParsing();
    QJsonObject js;
    bool ok;
    jstools::parseJson(json, "CustomComposition", js);
        jstools::parseJson(js, "Density", Density);
        jstools::parseJson(js, "Temperature", Temperature);
        jstools::parseJson(js, "UseMeanExcitationEnergy", UseCustomMeanExEnergy);
        jstools::parseJson(js, "MeanExcitationEnergy_eV", MeanExEnergy);

        QString str;
        ok = jstools::parseJson(js, "CompositionString", str);
        if (ok) setCompositionString(str);
        else
        {
            ErrorString = "CompositionString not found, assigning composition of vacuum";
            makeItVacuum();
        }
    return ok;
#endif
}

TGeoElement * AElementRecord::constructGeoElement() const
{
    QString name = Symbol;
    name.resize(2); if (name[1] == '_') name.chop(1);
    TString tBaseName = name.toLatin1().data();
    TGeoElement * elm = TGeoElement::GetElementTable()->FindElement(tBaseName);
    if (!elm)
    {
        qCritical() << "Unexpected problem: not valid element symbol:" << QString(tBaseName.Data());
        exit(1234);
    }

    if (Isotopes.empty()) return elm; // natural
    //else custom element using isotope records
    TString tName = tBaseName + "_Custom";
    const int Z = elm->Z();
    elm = new TGeoElement(tName, tName, Isotopes.size());
    for (const auto & pair : Isotopes)
    {
        const int    & isoN        = pair.first;
        const double & isoFraction = pair.second;
        TString tIsoName;
        tIsoName.Form("%i",isoN);
        tIsoName += tBaseName;
        TGeoIsotope * iso = TGeoIsotope::FindIsotope(tIsoName);
        //qDebug() << tIsoName << isoN << isoFraction << iso;
        if (!iso) iso = new TGeoIsotope(tIsoName, Z, isoN, isoN);
        elm->AddIsotope(iso, isoFraction);
    }
    return elm;
}

bool AElementRecord::isIdentical(const AElementRecord & other) const
{
    if (A != other.A) return false;
    if (Isotopes != other.Isotopes) return false;
    return true;
}
