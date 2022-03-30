#ifndef AGEOWRITER_H
#define AGEOWRITER_H

#include <QVector>
#include <QString>

class AGeoWriter
{
public:
    enum EDraw {PMs, PhotMons, PartMons};

    AGeoWriter();

    //draw on PMs/Monitors related
    QVector<QString> SymbolMap;
    QVector< QVector < double > > numbersX;
    QVector< QVector < double > > numbersY;

    QString showText(const QVector<QString> & strData, int color, EDraw onWhat); // returns error

private:
    void generateSymbolMap();
};

#endif // AGEOWRITER_H
