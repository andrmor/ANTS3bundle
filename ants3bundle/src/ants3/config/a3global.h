#ifndef A3GLOBAL_H
#define A3GLOBAL_H

#include "a3farmnoderecord.h"

#include <QString>
#include <QStringList>
#include <QJsonObject>

#include <vector>

class A3Global final
{
public:
    static A3Global & getInstance();
    static const A3Global & getConstInstance();

private:
    A3Global();
    ~A3Global(){}

    A3Global(const A3Global&)            = delete;
    A3Global(A3Global&&)                 = delete;
    A3Global& operator=(const A3Global&) = delete;
    A3Global& operator=(A3Global&&)      = delete;

public:
    QString ExecutableDir;
    QString ExchangeDir;

    int LocalCores = 4;
    std::vector<A3FarmNodeRecord> FarmNodes;

    int NumSegmentsTGeo = 20;

    QString LastSaveDir;
    QString LastLoadDir;

    void configureDirectories();

    void saveConfig();
    void loadConfig();
};

#endif // A3GLOBAL_H
