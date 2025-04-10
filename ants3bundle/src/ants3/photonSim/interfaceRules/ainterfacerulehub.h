#ifndef AINTERFACERULEHUB_H
#define AINTERFACERULEHUB_H

#include <QObject>
#include <QString>

#include <vector>
#include <map>
#include <set>

#include "TString.h"

class AInterfaceRule;
class QJsonObject;
class AMaterialHub;

// !!!*** check memory leak on rule replacement!

class AInterfaceRuleHub : public QObject
{
    Q_OBJECT

    explicit AInterfaceRuleHub();
    ~AInterfaceRuleHub(){}

    AInterfaceRuleHub(const AInterfaceRuleHub&)            = delete;
    AInterfaceRuleHub(AInterfaceRuleHub&&)                 = delete;
    AInterfaceRuleHub& operator=(const AInterfaceRuleHub&) = delete;
    AInterfaceRuleHub& operator=(AInterfaceRuleHub&&)      = delete;

public:
    static       AInterfaceRuleHub & getInstance();
    static const AInterfaceRuleHub & getConstInstance();

    void updateVolumesFromTo();  // run it before filling GeoManager
    bool isFromVolume(const TString & name) const;
    bool isToVolume(const TString & name) const;

    AInterfaceRule * getMaterialRuleFast(int MatFrom, int MatTo) const {return MaterialRules[MatFrom][MatTo];} // TODO: size_t and const !!!***
    AInterfaceRule * getVolumeRule(const TString & from, const TString & to) const;

    bool setMaterialRule(int matFrom, int matTo, AInterfaceRule * rule);

    void setVolumeRule(const TString & from, const TString & to, AInterfaceRule * rule);
    void removeVolumeRule(const TString & from, const TString & to);
    void moveVolumeRule(const TString & oldFrom, const TString & oldTo, const TString & newFrom, const TString & newTo);

    void onMaterialRemoved(int iMat);
    void onMaterialAdded(); // a new material can only be appended to the end of the list!

    void clearRules();

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);

    void updateRuntimeProperties();

    QString checkAll();

    std::vector<std::vector<AInterfaceRule*>> MaterialRules; // [fromMatIndex][toMatIndex]      nullptr -> rule not defined, using Fresnel

    std::map<std::pair<TString, TString>, AInterfaceRule*> VolumeRules;
    std::set<TString> VolumesFrom; //runtime
    std::set<TString> VolumesTo;   //runtime

private:
    const AMaterialHub & MatHub;

    QString readMaterialRulesFromJson(const QJsonObject & json);
    QString readVolumeRulesFromJson(const QJsonObject & json);


signals:
    void rulesLoaded();

};

#endif // AINTERFACERULEHUB_H
