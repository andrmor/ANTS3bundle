#ifndef APHOTONTUNNELHUB_H
#define APHOTONTUNNELHUB_H

#include <QString>

#include <vector>

class QJsonObject;

class ATunnelRecord
{
public:
    int     From;
    int     To;
    int     ModelIndex;
    QString Settings;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
};

class ATunnelRuntimeData
{
public:
    size_t ExitIndex;

    //ATunnelModelBase*
};

class APhotonTunnelHub
{
    APhotonTunnelHub();
    ~APhotonTunnelHub(){}

    APhotonTunnelHub(const APhotonTunnelHub&)            = delete;
    APhotonTunnelHub(APhotonTunnelHub&&)                 = delete;
    APhotonTunnelHub& operator=(const APhotonTunnelHub&) = delete;
    APhotonTunnelHub& operator=(APhotonTunnelHub&&)      = delete;

public:
    static       APhotonTunnelHub & getInstance();
    static const APhotonTunnelHub & getConstInstance();

    void writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);

    void clearAllConnections() {Connections.clear();}

    bool isValidConnection(const ATunnelRecord & rec, bool registerError) const;

    QString addOrModifyConnection(int from, int to, int model, const QString & settings);
    void removeConnection(int from, int to);

    bool updateRuntimeProperties(); // generate AErrorHub error if there are problems

    //std::vector<ATunnelModelBase*> Models;
    std::vector<ATunnelRecord> Connections;

    std::vector<ATunnelRuntimeData> RuntimeData;
};

#endif // APHOTONTUNNELHUB_H
