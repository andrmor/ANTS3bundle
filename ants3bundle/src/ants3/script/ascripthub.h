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
};

#endif // ASCRIPTHUB_H
