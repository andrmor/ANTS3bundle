#ifndef A3GLOBAL_H
#define A3GLOBAL_H

#include <QString>
//#include <QStringList>
#include <QJsonObject>

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

    int NumSegmentsTGeo = 20;

    bool AutoCheckGeometry = true;

    QString LastSaveDir;
    QString LastLoadDir;

    // !!!*** rename
    QJsonObject ScriptWindowJson;
    int DefaultFontSize_ScriptWindow = 12;
    QString DefaultFontFamily_ScriptWindow; //empty => Qt standard settings will be used
    bool DefaultFontWeight_ScriptWindow = false;
    bool DefaultFontItalic_ScriptWindow = false;

    void    configureDirectories();
    std::string checkExchangeDir();

    void    saveConfig();
    void    loadConfig();
};

#endif // A3GLOBAL_H
