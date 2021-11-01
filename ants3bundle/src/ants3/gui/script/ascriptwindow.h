#ifndef ASCRIPTWINDOW_H
#define ASCRIPTWINDOW_H

//#include "aguiwindow.h"
#include <QMainWindow>
#include <QSet>
#include <QHash>
#include <QString>
#include <QList>

#include <vector>

class AScriptInterface;
class AHighlighterScriptWindow;
class QCompleter;
class QStringListModel;
class ATextEdit;
class QPlainTextEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QSplitter;
class QFrame;
class QLineEdit;
class TObject;
class AScriptManager;
class ATabRecord;
class QTabWidget;
class A3Global;

namespace Ui {
class AScriptWindow;
}

enum class AScriptLanguageEnum {JavaScript = 0, Python = 1};

// !!!*** to a separate file!
class AScriptBook
{
public:
    AScriptBook();

    QString             Name;
    std::vector<ATabRecord*> Tabs;
    QTabWidget *        TabWidget   = nullptr; // will be owned by the QTabItemWidget

    void                writeToJson(QJsonObject & json) const;
    //bool              readFromJson(const QJsonObject & json);  // too heavily relies on AScriptWindow, cannot be implemented here without major refactoring

    int                 getCurrentTabIndex() const;
    void                setCurrentTabIndex(int index); // !!!*** possible to use size_t?

    void                setTabName(const QString & name, int index);

    ATabRecord *        getCurrentTab();
    ATabRecord *        getTab(int index);
    const ATabRecord *  getTab(int index) const;
    QTabWidget *        getTabWidget();

    void                removeTabNoCleanup(int index); //used by move  // !!!*** check
    void                removeTab(int index); // !!!*** check
    void                removeAllTabs();
};

class AScriptWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AScriptWindow(QWidget * parent);
    ~AScriptWindow();

    void registerInterfaces();
    void updateGui(); //highlighter, helper etc - call it to take into account all changes introduced by introduction of new interface units!

    void WriteToJson(); // !!!***
    void ReadFromJson(); // !!!***

    void onBusyOn();
    void onBusyOff();

    void setAcceptRejectVisible();
    void updateJsonTree();
    void reportError(QString error, int line = 0);   //0 - no line is highligted
    bool isAccepted() const {return bAccepted;}

private:
    A3Global          & GlobSet;
    AScriptLanguageEnum ScriptLanguage = AScriptLanguageEnum::JavaScript;
    Ui::AScriptWindow * ui             = nullptr;
    QStringList         Functions;

    std::vector<AScriptBook> ScriptBooks;
    int                 iCurrentBook   = -1;
    QTabWidget *        twBooks        = nullptr;

    int                 iMarkedBook    = -1;
    int                 iMarkedTab     = -1;

    bool                bAccepted      = false;

    QSplitter *         splMain        = nullptr;
    QSplitter *         splHelp        = nullptr;
    QPlainTextEdit *    pteOut         = nullptr;
    QTreeWidget *       trwHelp        = nullptr;
    QTreeWidget *       trwJson        = nullptr;
    QFrame *            frHelper       = nullptr;
    QFrame *            frJsonBrowser  = nullptr;
    QPlainTextEdit *    pteHelp        = nullptr;
    QLineEdit *         leFind         = nullptr;
    QLineEdit *         leFindJ        = nullptr;
    QIcon *             RedIcon        = nullptr;

    bool                bShowResult    = true;   //if false, window only reports "success", otherwise eval result is shown  !*! need it?
    bool                bShutDown      = false;

    QSet<QString>       ExpandedItemsInJsonTW;
    QStringList         functionList; //functions to populate tooltip helper
    QHash<QString, QString> DeprecatedOrRemovedMethods;
    QStringList         ListOfDeprecatedOrRemovedMethods;
    QStringList         ListOfConstants;

    void readFromJson(QJsonObject &json);
    void writeToJson(QJsonObject & json);

    void createGuiElements();
    void findText(bool bForward);
    void applyTextFindState();
    void fillSubObject(QTreeWidgetItem* parent, const QJsonObject& obj);
    void fillSubArray(QTreeWidgetItem* parent, const QJsonArray& arr);
    QString getDesc(const QJsonValue &ref);
    void fillHelper(const AScriptInterface *io, QString module);  //fill help TreeWidget according to the data in the obj
    QString getKeyPath(QTreeWidgetItem *item);
    QStringList getListOfMethods(const QObject *obj, QString ObjName, bool fWithArguments = false);  // !!!*** no need name, cponvert to AScriptInterface
    void appendDeprecatedOrRemovedMethods(const AScriptInterface *obj, const QString& name);  // !!!***  no need name, cponvert to AScriptInterface

    void addNewBook();
    void removeBook(int iBook, bool bConfirm = true);
    void removeAllBooksExceptFirst();
    void saveBook(int iBook);
    void loadBook(int iBook, const QString & fileName);
    void loadBook(int iBook, const QJsonObject & json);
    void renameBook(int iBook, const QString & newName);
    void renameBookRequested(int iBook);
    bool isUntouchedBook(int iBook) const;
    QString createNewBookName();

    std::vector<ATabRecord *> & getScriptTabs();
    std::vector<ATabRecord *> & getScriptTabs(int iBook);
    ATabRecord * getTab();
    ATabRecord * getTab(int index);
    ATabRecord * getTab(int index, int iBook);
    const ATabRecord * getTab(int index, int iBook) const;
    QTabWidget * getTabWidget();
    QTabWidget * getTabWidget(int iBook);
    int  getCurrentTabIndex();
    void setCurrentTabIndex(int index);
    void setCurrentTabIndex(int index, int iBook);
    int  countTabs(int iBook) const;
    int  countTabs() const;
    void setTabName(const QString & name, int index, int iBook);
    ATabRecord & addNewTab(int iBook);
    ATabRecord & addNewTab();
    void askRemoveTab(int tab);
    void removeTab(int tab);
    QString createNewTabName(int iBook);
    void renameTab(int tab);
    void markTab(int tab);
    void pasteMarkedTab();
    void copyTab(int iBook);
    void moveTab(int iBook);
    void updateTab(ATabRecord *tab);
    void formatTab(ATabRecord *tab);

public slots:
    void clearOutput();
    void outputHtml(QString text);
    void outputText(QString text);

    void onLoadRequested(QString NewScript);
    void onProgressChanged(int percent);

private slots:
    //context menus
    void twBooks_customContextMenuRequested(const QPoint & pos);
    void on_pbFileName_customContextMenuRequested(const QPoint & pos);
    void onRequestTabWidgetContextMenu(QPoint pos);
    void onContextMenuRequestedByJsonTree(QPoint pos);
    void onContextMenuRequestedByHelp(QPoint pos);
    void showContextMenuForJsonTree(QTreeWidgetItem *item, QPoint pos);

    void highlightErrorLine(int line);
    void updateFileStatusIndication();

    void onCurrentTabChanged(int tab);   //triggered ONLY on visible book and contains only file/find gui for the current script
    void onScriptTabMoved(int from, int to); // !!!*** check swap

    void twBooks_currentChanged(int index);
    void onBookTabMoved(int from, int to);

    //gui buttons
    void on_pbRunScript_clicked();  // !!!***
    void on_pbStop_clicked();       // !!!***
    void on_pbLoad_clicked();
    void on_pbSave_clicked();
    void on_pbSaveAs_clicked();
    void on_pbExample_clicked();  // !!!***
    void on_pbConfig_toggled(bool checked);
    void on_pbHelp_toggled(bool checked);
    void on_pbFileName_clicked();
    void on_pbCloseFindReplaceFrame_clicked();
    void on_pbFindNext_clicked();
    void on_pbFindPrevious_clicked();
    void on_leFind_textChanged(const QString &arg1);
    void on_pbReplaceOne_clicked();
    void on_pbReplaceAll_clicked();
    void on_pbAccept_clicked();
    void on_pbCancel_clicked();

    // main menu actions
    void on_actionSave_all_triggered();
    void on_actionload_session_triggered();
    void on_actionClose_all_books_triggered();
    void on_actionAdd_new_book_triggered();
    void on_actionLoad_book_triggered();
    void on_actionClose_book_triggered();
    void on_actionSave_book_triggered();
    void on_actionIncrease_font_size_triggered();
    void on_actionDecrease_font_size_triggered();
    void on_actionSelect_font_triggered();
/*
    void on_actionShow_all_messenger_windows_triggered();
    void on_actionHide_all_messenger_windows_triggered();
    void on_actionClear_unused_messenger_windows_triggered();
    void on_actionClose_all_messenger_windows_triggered();
*/
    void on_actionShow_Find_Replace_triggered();
    void on_actionReplace_widget_Ctr_r_triggered();
    void on_actionAdd_new_tab_triggered();
    void on_actionRemove_current_tab_triggered();
    void on_actionShortcuts_triggered();

    void onRequestDraw(TObject* obj, QString options, bool fFocus) {emit RequestDraw(obj, options, fFocus);}    
    void onFunctionClicked(QTreeWidgetItem* item, int column);  //help has to be shown
    void onKeyDoubleClicked(QTreeWidgetItem* item, int column);
    void onKeyClicked(QTreeWidgetItem* item, int column);
    void onFindTextChanged(const QString &arg1);
    void onFindTextJsonChanged(const QString &arg1);
    void onF1pressed(QString text);
    void onJsonTWExpanded(QTreeWidgetItem* item);
    void onJsonTWCollapsed(QTreeWidgetItem* item);
    void onDefaulFontSizeChanged(int size);
    void onFindSelected();
    void onReplaceSelected();
    void onFindFunction();
    void onFindVariable();
    void onBack();
    void onForward();

    void receivedOnStart() {emit onStart();}
    void receivedOnAbort();
    void receivedOnSuccess(QString eval);

protected:
    void closeEvent(QCloseEvent * e) override;  // !!!*** does nothing with the script?
//    bool event(QEvent * e) override; // !!!***

signals:
    void WindowShown(QString);
    void WindowHidden(QString);
    void RequestDraw(TObject* obj, QString options, bool fFocus);
    void onStart();
    void onAbort();
//    void onFinish(bool bError); // !!!***
    void success(QString eval); // !!!*** rename
    void requestUpdateConfig();

private:
    void registerInterface(const AScriptInterface * interface); // !!!***
};

class ATabRecord : public QObject
{
    Q_OBJECT
public:
    ATabRecord(const QStringList & functions, AScriptLanguageEnum language);
    ~ATabRecord();

    ATextEdit *     TextEdit            = nullptr;

    QString         FileName;
    QString         TabName;
    bool            bExplicitlyNamed    = false;   //if true save will not auto-rename

    const QStringList & Functions;

    QCompleter *    Completer           = nullptr;
    QStringListModel * CompletitionModel;
    AHighlighterScriptWindow * Highlighter = nullptr;

    QVector<int>    VisitedLines;
    int             IndexVisitedLines   = 0;
    int             MaxLineNumbers      = 20;

    void UpdateHighlight();

    void WriteToJson(QJsonObject & json) const;
    void ReadFromJson(const QJsonObject &json);

    bool wasModified() const;
    void setModifiedStatus(bool flag);

    void goBack();
    void goForward();

private slots:
    void onCustomContextMenuRequested(const QPoint & pos);
    void onLineNumberChanged(int lineNumber);
    void onTextChanged();

signals:
    void requestFindText();
    void requestReplaceText();
    void requestFindFunction();
    void requestFindVariable();
};

#endif // ASCRIPTWINDOW_H