#include "amatcomposition.h"

#include <QDebug>

#include "TGeoElement.h"

void AMatComposition::clear()
{
    ErrorString.clear();
    WarningString.clear();

    CustomElementRecords.clear();
    ParseStringByBracketLevel.clear();
}

bool AMatComposition::parse(const QString & string)
{
    clear();

    ParseString = string.simplified();
    qDebug() << ParseString;

    bool ok = checkChars();
    if (!ok) return false;

    ok = markCustomElements(); // mark element records with custom isotope composition
    if (!ok) return false;

    // removing top level brackets
    ok = splitByBracketLevel(ParseString);
    if (!ok) return false;
    qDebug() << "Top level bracket records:" << ParseStringByBracketLevel;

    // inner level brackets
    for (size_t iR = 0; iR < ParseStringByBracketLevel.size(); iR++)
    {
        QString copy = ParseStringByBracketLevel[iR];
        ok = splitByBracketLevel(copy);
        ParseStringByBracketLevel[iR] = copy;
        if (!ok) return false;
    }

    qDebug() << "\n\nFinal string:" << ParseString;
    qDebug() << "Iso:" << CustomElementRecords;
    qDebug() << "Bracketed:" << ParseStringByBracketLevel;
    qDebug() << "----\n\n";

    const size_t numComponents = ParseStringByBracketLevel.size();
    std::vector<AMatMixRecord> mixtureComponents(numComponents);
    for (size_t iCER = numComponents; iCER-- > 0; )
    {
        std::vector<AMatMixRecord> recs;
        ok = prepareMixRecords(ParseStringByBracketLevel[iCER], recs);
        if (!ok) return false;

        for (AMatMixRecord & r : recs)
        {
            ok = parseMixRecord(r);
            if (!ok) return false;
        }

        // merge
        // ...
        //mixtureComponents.push_back(aaa);
    }



    qDebug() << "<=== parse finished ===>";
    return true;
}

bool AMatComposition::checkChars()
{
    for (QChar ch : ParseString)
    {
        qDebug() << "----------------->" << ch << ch.isLetterOrNumber();
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

bool AMatComposition::markCustomElements()
{
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
            qDebug() << "Found custom element record:" << rec;
            const QString replaceWith = QString("#i%0&").arg(CustomElementRecords.size());
            qDebug() << "Replacing with:" << replaceWith;
            ParseString.replace(start, selectionSize, replaceWith);

            rec.remove(0, 1); rec.chop(1); // killing { and }
            CustomElementRecords.push_back(rec);

            qDebug() << "Continue with the ParseString:" << ParseString;
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
            /*
            if (!ch.isLetterOrNumber())
                if (ch != ':' && ch != '+' && ch != ' ')
                {
                    ErrorString = "Not allowed characted in custom element record: " + QString(ch);
                    return false;
                }
            */
        }

        pos++;
    }

    if (bReadingRecord)
    {
        ErrorString += "Unfinished custom element record [missing }]";
        return false;
    }

    qDebug() << "Found iso records:";
    qDebug() << CustomElementRecords;
    qDebug() << "Returning to parse string:" << ParseString;
    return true;
}

bool AMatComposition::splitByBracketLevel(QString & string)
{
    int start = 0;
    int pos = 0;
    bool bReadingRecord = false;
    int level = 0;
    while (pos < string.length())
    {
        const QChar ch = string[pos];
        qDebug() << "  ch>" << ch;

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
            qDebug() << "Found bracketed record:" << rec;
            const QString replaceWith = QString("#b%0&").arg(ParseStringByBracketLevel.size());
            qDebug() << "Replacing with:" << replaceWith;
            string.replace(start, selectionSize, replaceWith);
            qDebug() << "Continue with the string:" << string;

            rec.remove(0, 1); rec.chop(1);
            ParseStringByBracketLevel.push_back(rec);

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

    qDebug() << "Returning string:" << string;
    return true;
}

bool AMatComposition::prepareMixRecords(const QString & expression, std::vector<AMatMixRecord> & result)
{
    QString str = expression.simplified();
    if (str.isEmpty())
    {
        ErrorString = "Format error: Detected empty record";
        return false;
    }

    str.replace(" ", "+");
    qDebug() << "Processing record:" << str;

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

bool AMatComposition::parseMixRecord(AMatMixRecord & r)
{
    QString formula = r.Formula.simplified() + ":"; //":" is end signal
    qDebug() << "parsing mix:" << formula;

    if (!formula.front().isLetter() || !formula.front().isUpper())
    {
        ErrorString = "Format error in composition:" + formula;
        return false;
    }

    //      qDebug() << Formula;
    bool bReadingElementName = true;
    QString tmp = formula[0];
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
