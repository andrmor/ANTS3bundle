#ifndef ASCRIPTHUB_H
#define ASCRIPTHUB_H

#include "escriptlanguage.h"

#include <QObject>
#include <QString>
#include <vector>

class AJScriptManager;
class AScriptInterface;
class AGeoWin_SI;
class AGeometryWindow;
class AGuiFromScrWin;

#ifdef ANTS3_PYTHON
    class APythonScriptManager;
#endif

class AScriptHub : public QObject
{
    Q_OBJECT

public:
    static AScriptHub      & getInstance();

    static void              abort(const QString & message, EScriptLanguage lang);
    static bool              isAborted(EScriptLanguage lang);


    AJScriptManager        & getJScriptManager() {return *JavaScriptM;}
#ifdef ANTS3_PYTHON
    APythonScriptManager   & getPythonManager()  {return *PythonM;}
#endif

    void addCommonInterface(AScriptInterface * interface, QString name);
    void updateGeoWin(AGeometryWindow * GeoWin);
    void addGuiScriptUnit(AGuiFromScrWin * win);
    void finalizeInit(); // run when initialization is finished (all additional script units already registered)

    void outputText(const QString & text, EScriptLanguage lang);
    void outputHtml(const QString & text, EScriptLanguage lang);
    void outputFromBuffer(const std::vector<std::pair<bool,QString>> & buffer, EScriptLanguage lang);
    void clearOutput(EScriptLanguage lang);

    void processEvents(EScriptLanguage lang);
    void reportProgress(int percents, EScriptLanguage lang);

    QString getPythonVersion();

    QString evaluateScriptAndWaitToFinish(const QString & fileName, EScriptLanguage lang);

    void aboutToQuit();

private:
    AScriptHub();
    ~AScriptHub();

    AScriptHub(const AScriptHub&)            = delete;
    AScriptHub(AScriptHub&&)                 = delete;
    AScriptHub& operator=(const AScriptHub&) = delete;
    AScriptHub& operator=(AScriptHub&&)      = delete;

signals:
    //for gui
    void outputText_JS(QString);
    void outputText_P(QString);
    void outputHtml_JS(QString);
    void outputHtml_P(QString);
    void outputFromBuffer_JS(std::vector<std::pair<bool,QString>> Buffer);
    void outputFromBuffer_P(std::vector<std::pair<bool,QString>> Buffer);
    void clearOutput_JS();
    void clearOutput_P();
    void requestUpdateGui();
    void reportProgress_JS(int percent);
    void reportProgress_P(int percent);

private:
    AJScriptManager      * JavaScriptM = nullptr;
#ifdef ANTS3_PYTHON
    APythonScriptManager * PythonM = nullptr;
#endif

    std::vector<AGeoWin_SI*> geoWinInterfaces;
};

#endif // ASCRIPTHUB_H
