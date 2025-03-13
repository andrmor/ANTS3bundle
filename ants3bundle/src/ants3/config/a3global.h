#ifndef A3GLOBAL_H
#define A3GLOBAL_H

#include "adrawmarginsrecord.h"

#include <QString>
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
    // Dirs
    QString ExchangeDir;   // can be changed, therefore it is saved
    QString ExecutableDir;
    QString ConfigDir;     // global settings are saved there
    QString TmpOutputDir;
    QString QuicksaveDir;
    QString ExamplesDir;
    QString ResourcesDir;
    QString LastSaveDir;
    QString LastLoadDir;

    // Misc controls
    bool AutoCheckGeometry = true;
    int  NumSegmentsTGeo = 20;
    int  BinsX = 100;
    int  BinsY = 100;
    int  BinsZ = 100;
    bool OpenImageExternalEditor = true;

    // Script window
    QJsonObject JavaScriptJson;
    QJsonObject PythonJson;
    QString SW_FontFamily;         //empty => Qt standard settings will be used
    int     SW_FontSize   = 12;
    int     SW_FontWeight = false;
    bool    SW_Italic     = false;
    int     TabInSpaces = 7;

    // Margins
    ADrawMarginsRecord DefaultDrawMargins;

    // GeoTreeWindow
    bool NewGeoObjectAddedLast = false;

    QJsonObject TrackVisAttributes;

    // WebSocket server
    int     DefaultWebSocketPort = 1234;
    QString DefaultWebSocketIP = "127.0.0.1";

    void    init();
    bool    checkExchangeDir();

    void    saveConfig();
    void    loadConfig();

    QString getQuickFileName(int index) const;

private:
    const QString ConfigFileName = "globalconfig.json";
};

#endif // A3GLOBAL_H
