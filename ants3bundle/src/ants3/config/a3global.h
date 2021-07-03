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

    QString DispatcherExecutable = "dispatcher";

    std::vector<A3FarmNodeRecord> FarmNodes;

    void configureDirectories();

    bool saveConfig();
    bool loadConfig();
};

#endif // A3GLOBAL_H
