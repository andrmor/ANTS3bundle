#ifndef A3GLOBAL_H
#define A3GLOBAL_H

//later split in classes

#include <QString>
#include <QStringList>
#include <QJsonObject>

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

    void configureDirectories();

    bool saveConfig();
    bool loadConfig();
};

#endif // A3GLOBAL_H
