#include "amatcomposition.h"

#include <QDebug>

#include "TGeoElement.h"

void AMatComposition::clear()
{
    ErrorString.clear();
    WarningString.clear();

    CustomElements.clear();
    MixtureByLevels.clear();
}

bool AMatComposition::parse(const QString & string)
{
    clear();

    ParseString = string.simplified();
    qDebug() << "Composition to parse:" << ParseString;

    bool ok = checkForbiddenChars();
    if (!ok) return false;

    ok = parseCustomElements(); // custom element = element with a custom isotope composition
    if (!ok) return false;

    qDebug() << "After custom element parsing, the string is:" << ParseString;

    // pre-parsing brackets to expression levels
    MixtureByLevels.push_back({ParseString, AMatMixRecord()});
    ok = parseBracketedLevels();
    if (!ok) return false;

    qDebug() << "MixtureByLevels have the following records:";
    for (const auto & p : MixtureByLevels) qDebug() << "      " << p.first;
    qDebug() << "----\n\n";

    ok = parseMixtures();
    if (!ok) return false;

    qDebug() << "<=== parse finished ===>";

    qDebug() << "Molar composition:";
    for (const auto & kv : MixtureByLevels.front().second.ElementMap)
    {
        qDebug() << kv.first->GetName() << " - " << kv.second;
    }

    return true;
}

bool AMatComposition::checkForbiddenChars()
{
    for (int i = 0; i < ParseString.size(); i++)
    {
        const QChar ch = ParseString[i];
        //qDebug() << "----------------->" << ch << ch.isLetterOrNumber();
        if (ch.isLetterOrNumber()) continue;
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
    qDebug() << "Searching for custom element records containing isotope composition";
    int start = 0;
    int pos = 0;
    bool bReadingRecord = false;
    while (pos < ParseString.length())
    {
        const QChar ch = ParseString[pos];
        qDebug() << "  ch>" << ch;

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
            qDebug() << "  Found custom element record:" << rec;
            const QString replaceWith = QString("#i%0&").arg(CustomElements.size());
            qDebug() << "  Replacing with:" << replaceWith;
            ParseString.replace(start, selectionSize, replaceWith);

            rec.remove(0, 1); rec.chop(1); // killing { and }
            TGeoElement * ele = makeCustomElement(rec);
            if (!ele) return false;
            CustomElements.push_back(ele);

            qDebug() << "  Continue to look for custom elements in string:" << ParseString;
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

    qDebug() << "Found custom element records:" << CustomElements;
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
            ok = parseCompound(r);
            if (!ok) return false;
        }

        mergeRecords(recs, MixtureByLevels[iLevel].second);
    }

    return true;
}

bool AMatComposition::splitByBracketLevel(QString & string)
{
    qDebug() << "Checking for bracketed sub-records";
    int start = 0;
    int pos = 0;
    bool bReadingRecord = false;
    int level = 0;
    while (pos < string.length())
    {
        const QChar ch = string[pos];
        qDebug() << "  br>" << ch;

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
            qDebug() << "  Found bracketed record:" << rec;
            const QString replaceWith = QString("#b%0&").arg(MixtureByLevels.size());
            qDebug() << "  Replacing with:" << replaceWith;
            string.replace(start, selectionSize, replaceWith);
            qDebug() << "  Continue with the string:" << string;

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

    qDebug() << "  Returning string:" << string;
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
    qDebug() << "Processing mixture record:" << str;

    const QStringList list = str.split('+', Qt::SkipEmptyParts ); //split to fields of formula:fraction
    for (const QString & str : list)
    {
        qDebug() << str;

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
        for (AMatMixRecord r : result)
        {
            if (seenMolar && r.FractionType == AMatMixRecord::None) r.FractionType = AMatMixRecord::Molar;
            if (seenMass  && r.FractionType == AMatMixRecord::None) r.FractionType = AMatMixRecord::Mass;
        }
    }
    return true;
}

bool AMatComposition::parseCompound(AMatMixRecord & r)
{
    // Formula can contain custom isotopes! e.g. #i0&4C
    // Formula can contain already processed sub-mixtures, e.g. #b1&

    QString formula = r.Formula.simplified() + ":"; //":" is end signal
    qDebug() << "parsing mix record:" << formula << "  with fraction:" << r.Fraction;

    if (!formula.front().isLetter() || !formula.front().isUpper())
    {
        ErrorString = "Format error in composition:" + formula;
        return false;
    }

    //      qDebug() << Formula;
    bool bReadingElementName = true;
    QString tmp(formula.front());
    QString ElementSymbol;
    for (int i = 1; i < formula.size(); i++)
    {
        QChar c = formula[i];
        if (bReadingElementName)
        {
            if (c.isLetter() && c.isLower()) //continue to acquire the name
            {
                tmp += c;
                continue;
            }
            else if (c == ':' || c.isDigit() || (c.isLetter() && c.isUpper()))
            {
                if (tmp.isEmpty()) return "Format error in composition: unrecognized character";
                ElementSymbol = tmp;
                if (c == ':' || (c.isLetter() && c.isUpper()))
                {
                    TGeoElement * ele = TGeoElement::GetElementTable()->FindElement(ElementSymbol.toLatin1().data());
                    if (!ele)
                    {
                        ErrorString = "Element does not exist: " + ElementSymbol;
                        return false;
                    }
                    auto it = r.ElementMap.find(ele);
                    if (it == r.ElementMap.end()) r.ElementMap[ele] = 1.0;
                    else it->second += 1.0;
                    if (c == ':') break;
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
            else if (c ==':' || c.isLetter())
            {
                if (c.isLower())
                {
                    ErrorString = "Format error in composition: lower register symbol in the first character of element symbol";
                    return false;
                }
                double number = tmp.toDouble();
                TGeoElement * ele = TGeoElement::GetElementTable()->FindElement(ElementSymbol.toLatin1().data());
                if (!ele)
                {
                    ErrorString = "Element does not exist: " + ElementSymbol;
                    return false;
                }
                auto it = r.ElementMap.find(ele);
                if (it == r.ElementMap.end()) r.ElementMap[ele] = number;
                else it->second += number;
                if (c==':') break;
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

TGeoElement * AMatComposition::makeCustomElement(const QString & strRec)
{
    // 10B:95+11B:5
    QString fullStr = strRec.simplified().replace(' ', '+');
    qDebug() << "    Processing custom element record:" << fullStr;

    QString elementSymbol;
    std::vector<std::pair<int,double>> isotopeVec; // pairs of [N,Fraction]

    const QStringList list = fullStr.split('+', Qt::SkipEmptyParts ); //split to fields of NElement:Fraction
    if (list.isEmpty())
    {
        ErrorString = "Custom element record is empty";
        return nullptr;
    }

    for (const QString & str : list)
    {
        qDebug() << "      Isotope record:" << str;
        if (!str.front().isDigit() || str.size() < 2)
        {
            ErrorString = "Custom element record should contain at least one isotope (e.g. 10B)";
            return nullptr;
        }
        enum EState {StateN, StateSymbol, StateFraction};
        EState State = StateN;
        QString strN; int isotopeN;
        QString isotopeSymbol;
        QString strFraction; double fraction;
        bool ok;
        for (QChar ch : str)
        {
            qDebug() << "      i>" << ch;
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
                    return nullptr;
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
                    return nullptr;
                }

                if (elementSymbol.isEmpty())
                {
                    elementSymbol = isotopeSymbol;
                    if (!TGeoElement::GetElementTable()->FindElement(elementSymbol.toLatin1().data()))
                    {
                        ErrorString = "Unknown element in custom element record: " + elementSymbol;
                        return nullptr;
                    }
                }
                else if (isotopeSymbol != elementSymbol)
                {
                    ErrorString = "All isotopes of a custom element should have the same symbol";
                    return nullptr;
                }

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
                return nullptr;
            }
        }

        if (State != StateFraction)
        {
            if (list.size() == 1 && State == StateSymbol)
            {
                // for a single isotope allow skipping the fraction info
                elementSymbol = isotopeSymbol;
                if (!TGeoElement::GetElementTable()->FindElement(elementSymbol.toLatin1().data()))
                {
                    ErrorString = "Unknown element in custom element record: " + elementSymbol;
                    return nullptr;
                }
                strFraction = "1.0";
            }
            else
            {
                ErrorString = "Custom element record: error extracting isotope fraction (use e.g. 10B or 10B:97+11B:3)";
                return nullptr;
            }
        }
        fraction = strFraction.toDouble(&ok);
        if (!ok)
        {
            ErrorString = "Custom element record: fraction format error";
            return nullptr;
        }

        isotopeVec.push_back({isotopeN, fraction});
    }

    qDebug() << "    Constructing custom element using parsed info:" << elementSymbol << isotopeVec;

    TString tBaseName = elementSymbol.toLatin1().data();
    TString tEleName = tBaseName + "_Custom"; // !!!*** need unique name?

    int Z = TGeoElement::GetElementTable()->FindElement(elementSymbol.toLatin1().data())->Z(); // confirmed above that exists
    TGeoElement * elm = new TGeoElement(tEleName, tEleName, isotopeVec.size());

    for (const auto & r : isotopeVec)
    {
        const int    & isoN        = r.first;
        const double & isoFraction = r.second;
        TString tIsoName = tBaseName; tBaseName += isoN;
        TGeoIsotope * iso = TGeoIsotope::FindIsotope(tIsoName);
        if (!iso) iso = new TGeoIsotope(tIsoName, Z, isoN, isoN);
        elm->AddIsotope(iso, isoFraction);
    }

    return elm;
}

void AMatComposition::mergeRecords(const std::vector<AMatMixRecord> & recs, AMatMixRecord & result)
{
    if (recs.empty()) return;

    if (recs.size() == 1)
    {
        result = recs.front();
        return;
    }

    AMatMixRecord::EFractionType Type = recs.front().FractionType;

    if (Type == AMatMixRecord::Molar)
    {
        result = recs.front();
        for (auto & kv : result.ElementMap)
            kv.second *= result.Fraction;

        for (size_t iRec = 1; iRec < recs.size(); iRec++)
        {
            const AMatMixRecord & other = recs[iRec];
            for (const auto & kv : other.ElementMap)
            {
                auto it = result.ElementMap.find(kv.first);
                if (it == result.ElementMap.end()) result.ElementMap[kv.first] = kv.second * other.Fraction;
                else it->second += kv.second * other.Fraction;
            }
        }
    }
}
