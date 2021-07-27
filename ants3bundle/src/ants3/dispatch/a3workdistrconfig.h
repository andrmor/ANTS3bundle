#ifndef A3WORKDISTRCONFIG_H
#define A3WORKDISTRCONFIG_H

#include <QString>

#include <vector>

class QJsonObject;

class A3NodeWorkerConfig
{
public:
    QString              ConfigFile;
    std::vector<QString> InputFiles;
    std::vector<QString> OutputFiles;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
};

class A3WorkNodeConfig
{
public:
    QString Address;
    int     Port;

    std::vector<A3NodeWorkerConfig> Workers;

    bool isLocalNode() const;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
};

class A3WorkDistrConfig
{
public:
    QString              Command;
    QString              ExchangeDir;
    std::vector<QString> CommonFiles;

    std::vector<A3WorkNodeConfig> Nodes;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    //runtime
    int NumEvents;
};

#endif // A3WORKDISTRCONFIG_H
