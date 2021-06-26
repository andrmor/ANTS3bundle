#ifndef A3CONFIG_H
#define A3CONFIG_H

//later split in classes

#include <QString>
#include <QStringList>
#include <QJsonObject>

class A3Config final
{
public:
    static A3Config & getInstance();

private:
    A3Config();
    ~A3Config(){}

    A3Config(const A3Config&)            = delete;
    A3Config(A3Config&&)                 = delete;
    A3Config& operator=(const A3Config&) = delete;
    A3Config& operator=(A3Config&&)      = delete;

public:
    QString     from = "b";
    QString     to   = "B";
    QString     lines;

    QJsonObject JSON;
    QString     ConfigFileName = "config.json";

    bool saveConfig();
    bool loadConfig();
};

#endif // A3CONFIG_H
