#include "asimsettingshub.h"

ASimSettingsHub & ASimSettingsHub::getInstance()
{
    static ASimSettingsHub instance;
    return instance;
}

const ASimSettingsHub &ASimSettingsHub::getConstInstance()
{
    return getInstance();
}

ASimSettingsHub::ASimSettingsHub()
{

}
