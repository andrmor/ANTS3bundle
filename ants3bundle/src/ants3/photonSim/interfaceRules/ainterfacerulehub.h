#ifndef AINTERFACERULEHUB_H
#define AINTERFACERULEHUB_H

#include <QObject>
#include <QString>

#include <vector>

class AInterfaceRule;
class QJsonObject;

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

    std::vector<std::vector<AInterfaceRule*>> Rules; // [fromMatIndex][toMatIndex]      nullptr -> rule not defined, using Fresnel

    void updateWaveResolvedProperties();

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    QString checkAll();

public slots:
    void onMaterialDeleted(size_t iMat);
    void onNewMaterialAdded();

};

#endif // AINTERFACERULEHUB_H
