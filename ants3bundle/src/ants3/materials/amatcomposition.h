#ifndef AMATCOMPOSITION_H
#define AMATCOMPOSITION_H

#include <QString>

//#include "TGeoElement.h"

class AMatComposition
{
public:
    AMatComposition(){}

    bool parse(const QString & string);

    QString ErrorString;
    QString WarningString;

protected:
    std::vector<QString> CustomElementRecords;
    std::vector<QString> Bracketed;

    bool checkChars();
    bool parseCustomElements();
    bool parseTopLevelBrackets(QString & string);

private:
    QString ParseString;

    void clear();
};

#endif // AMATCOMPOSITION_H
