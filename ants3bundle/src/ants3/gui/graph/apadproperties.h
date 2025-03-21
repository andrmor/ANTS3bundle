#ifndef APADPROPERTIES_H
#define APADPROPERTIES_H

#include "apadgeometry.h"

#include <QString>

#include <vector>

class TObject;
class QJsonObject;
class TPad;

class APadProperties
{
public:
    APadProperties();
    APadProperties(TPad * pad);

    void updatePadGeometry();
    void applyPadGeometry();

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    QString toString() const;

    TPad * tPad = nullptr;
    APadGeometry padGeo;

    std::vector<TObject*> tmpObjects;
};

#endif // APADPROPERTIES_H
