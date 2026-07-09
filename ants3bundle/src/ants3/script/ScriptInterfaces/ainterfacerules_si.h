#ifndef AINTERFACERULES_SI_H
#define AINTERFACERULES_SI_H

#include "ascriptinterface.h"

#include <QString>
#include <QVariantMap>

class AInterfaceRuleHub;
class ALutInterfaceRule;

class AInterfaceRules_SI : public AScriptInterface
{
    Q_OBJECT

public:
    AInterfaceRules_SI();

    AScriptInterface * cloneBase() const {return new AInterfaceRules_SI();}

public slots:
    QVariantMap generateSurfaceLut(QString heightmapFile, QString outputLutFile, QVariantMap params);
    QVariantMap getLutInfo(QString lutFile);

    void setLutMaterialRule(QString matFrom, QString matTo, QString lutFile);
    void setLutVolumeRule(QString volFrom, QString volTo, QString lutFile);

    void clearMaterialRule(QString matFrom, QString matTo);
    void clearVolumeRule(QString volFrom, QString volTo);

private:
    AInterfaceRuleHub & RuleHub;

    ALutInterfaceRule * makeLutRule(int matFrom, int matTo, const QString & lutFile);  // nullptr + abort on error
};

#endif // AINTERFACERULES_SI_H
