#include "amatcomposition.h"

#include <QDebug>
//#include <QRegularExpression>

void AMatComposition::clear()
{
    ErrorString.clear();
    WarningString.clear();

    CustomElementRecords.clear();
    Bracketed.clear();
}

bool AMatComposition::parse(const QString & string)
{
    clear();

    ParseString = string.simplified();
    qDebug() << ParseString;

    bool ok = checkChars();
    if (!ok) return false;

    ok = parseCustomElements(); // parsing element records with custom isotope composition
    if (!ok) return false;

    // removing top level brackets
    ok = parseTopLevelBrackets(ParseString);
    if (!ok) return false;
    qDebug() << "Top level bracket records:" << Bracketed;

    // inner level brackets
    for (size_t iR = 0; iR < Bracketed.size(); iR++)
    {
        QString copy = Bracketed[iR];
        ok = parseTopLevelBrackets(copy);
        Bracketed[iR] = copy;
        if (!ok) return false;
    }

    qDebug() << "\n\nFinal string:" << ParseString;
    qDebug() << "Iso:" << CustomElementRecords;
    qDebug() << "Bracketed:" << Bracketed;


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

bool AMatComposition::parseCustomElements()
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

bool AMatComposition::parseTopLevelBrackets(QString & string)
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
            const QString replaceWith = QString("#b%0&").arg(Bracketed.size());
            qDebug() << "Replacing with:" << replaceWith;
            string.replace(start, selectionSize, replaceWith);
            qDebug() << "Continue with the string:" << string;

            rec.remove(0, 1); rec.chop(1);
            Bracketed.push_back(rec);

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
