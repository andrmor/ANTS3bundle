#ifndef ASIMSETTINGSHUB_H
#define ASIMSETTINGSHUB_H

#include "aphotsimsettings.h"

class QJsonObject;

#include <QString>

class ASimSettingsHub final
{
public:
    static       ASimSettingsHub & getInstance();
    static const ASimSettingsHub & getConstInstance();

private:
    ASimSettingsHub();
    ~ASimSettingsHub(){}

    ASimSettingsHub(const ASimSettingsHub&)            = delete;
    ASimSettingsHub(ASimSettingsHub&&)                 = delete;
    ASimSettingsHub& operator=(const ASimSettingsHub&) = delete;
    ASimSettingsHub& operator=(ASimSettingsHub&&)      = delete;

public:
    APhotSimSettings PhotSimSet;

};

#endif // ASIMSETTINGSHUB_H
