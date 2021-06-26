#ifndef A3WORKDISTRCONFIG_H
#define A3WORKDISTRCONFIG_H

#include <QString>
#include <QStringList>
#include <QVector>

class QJsonObject;

class A3NodeWorkerConfig
{
public:
    QString          ConfigFile;
    QVector<QString> InputFiles;
    QVector<QString> OutputFiles;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
};

class A3WorkNodeConfig
{
public:
    QString Address;
    int     Port;

    QVector<A3NodeWorkerConfig> Workers;

    bool isLocalNode() const;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
};

class A3WorkDistrConfig
{
public:
    QString          Command;
    QString          ExchangeDir;
    QVector<QString> CommonFiles;

    QVector<A3WorkNodeConfig> Nodes;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    //runtime
    int NumEvents;
};

#endif // A3WORKDISTRCONFIG_H
