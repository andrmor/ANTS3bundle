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
class TObject;
class LRModel;
class ALrfPlotter;
class AGeoMarkerClass;
class TVirtualGeoTrack;
class Reconstructor;

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

    void prepareToWait();
    void waitForGuiCallFinished(EScriptLanguage lang);

    QString getPythonVersion();

    QString evaluateScriptAndWaitToFinish(const QString & fileName, EScriptLanguage lang);

    void aboutToQuit();

    // if json was manipulated by config.replace(), and config.updateConfig() was not yet called, access to method dirtectly modifying hubs should be blocked
    void abortIfHubAccessBlocked(EScriptLanguage lang);
    void registerJsonModified_HubsNotYetUpdated(bool flag); // set to true by config.replace()
    // if hubs (config directly) was modified, json manipulation should be conducted after coping hubs to json (otherwise the chnages are lost) --> automatic, no warning
    void copyHubsToJsonConfig();
    void registerHubsModified_JsonNotYetUpdated(bool flag); // should be set to true by any method that manipulates hubs directly
    // init related to these features:
    void doBeforeScriptEval();

public slots:
    void onGuiReportTaskCompleted();

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
    void requestDraw(TObject * obj, QString options, bool fFocus); // connected using Queued Connection inside graphwindow class; object ownership is transferred to graph window!
    void requestDrawCollection(std::vector<std::pair<TObject*, QString>> objectsAndOptions, bool fFocus); // connected using Queued Connection inside graphwindow class; object ownership is transferred to graph window!
    void requestAddToBasket(QString title);
    void requestShowLightResponseExplorer(LRModel * model); // mercury SI
    void requestShowPlotterDialog();   // mercury SI
    void requestShowEventExplorer(Reconstructor * rec, std::vector<std::vector<double>> * events, std::vector<std::array<double,3>> * truePositions);   // mercury SI; truePositions can be nullptr

    // signals for geo window (which can be dynamically replaced, so connection is also dynamic, see MainWindow::connectSignalSlotsForGeoWin)
    void requestRedraw();
    void requestShowTracks();
    void requestClearTracks();
    void requestClearMarkers();
    void requestSaveImage(QString fileName);
    void requestAddMarkers(AGeoMarkerClass * markers);
    void requestAddTrack(TVirtualGeoTrack * track);

private:
    AJScriptManager      * JavaScriptM = nullptr;
#ifdef ANTS3_PYTHON
    APythonScriptManager * PythonM = nullptr;
#endif

    std::vector<AGeoWin_SI*> geoWinInterfaces;

    // used with queued calls from script to gui to wait for an operation to finish
    bool WaitingForTaskCompleted = false;

    bool FlagHubsChanged = false;
    bool FlagJsonChanged = false;
};

#endif // ASCRIPTHUB_H
