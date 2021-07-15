#ifndef A3OPTINTHUB_H
#define A3OPTINTHUB_H

#include <QObject>
#include <QString>

#include <vector>

class AOpticalOverride;
class QJsonObject;

class A3OptIntHub : public QObject
{
    Q_OBJECT

    explicit A3OptIntHub();
    ~A3OptIntHub(){}

    A3OptIntHub(const A3OptIntHub&)            = delete;
    A3OptIntHub(A3OptIntHub&&)                 = delete;
    A3OptIntHub& operator=(const A3OptIntHub&) = delete;
    A3OptIntHub& operator=(A3OptIntHub&&)      = delete;

public:
    static       A3OptIntHub & getInstance();
    static const A3OptIntHub & getConstInstance();

    std::vector<std::vector<AOpticalOverride*>> OpticalOverrides; // [fromMatIndex][toMatIndex]      nullptr -> override not defined

    void updateWaveResolvedProperties();

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    QString checkOverrides();

public slots:
    void onMaterialDeleted(size_t iMat);
    void onNewMaterialAdded();

};

#endif // A3OPTINTHUB_H
