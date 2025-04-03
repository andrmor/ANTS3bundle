#ifndef ADRAWOBJECT_H
#define ADRAWOBJECT_H

#include "adrawmarginsrecord.h"
#include "amultidrawrecord.h"

#include <QString>

class TObject;
class AMultidrawRecord;
class QJsonObject;

class ADrawObject
{
public:
    ADrawObject() {}
    virtual ~ADrawObject() {} //does not own TObject!

    ADrawObject(TObject * pointer, const char* options);
    ADrawObject(TObject * pointer, const QString & options, bool enabled = true);
    ADrawObject(TObject * pointer, const QString & options, bool enabled, bool bLogScaleX, bool bLogScaleY);

    bool      Multidraw = false;
    TObject * Pointer = nullptr;
    QString   Name;
    QString   Options;
    bool      bEnabled  = true;
    bool      bLogScaleX = false;
    bool      bLogScaleY = false;

    AMultidrawRecord MultidrawSettings;

    ADrawMarginsRecord CustomMargins;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    void shiftMultidrawIndexesBy(int val);

private:
    void extractName();
};

#endif // ADRAWOBJECT_H
