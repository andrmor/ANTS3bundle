#ifndef AINTERFACERULEHUB_H
#define AINTERFACERULEHUB_H

#include <QObject>
#include <QString>

#include <vector>

class AInterfaceRule;
class QJsonObject;
class AMaterialHub;

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

    AInterfaceRule * getRuleFast(int MatFrom, int MatTo) const {return Rules[MatFrom][MatTo];} // TODO: size_t and const !!!***

    std::vector<std::vector<AInterfaceRule*>> Rules; // [fromMatIndex][toMatIndex]      nullptr -> rule not defined, using Fresnel

    void updateWaveResolvedProperties();

    void onMaterialRemoved(int iMat);
    void onMaterialAdded(); // a new material can only be appended to the end of the list!

    void clearRules();

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);

    QString checkAll();

private:
    const AMaterialHub & MatHub;

signals:
    void rulesLoaded();

};

#endif // AINTERFACERULEHUB_H
