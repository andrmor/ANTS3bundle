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

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);

    QString checkAll();

public slots:
    void onMaterialDeleted(size_t iMat);
    void onNewMaterialAdded();

private:
    const AMaterialHub & MatHub;

};

#endif // AINTERFACERULEHUB_H
