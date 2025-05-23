#ifndef ASCRIPTWINDOW_H
#define ASCRIPTWINDOW_H

#include "aguiwindow.h"
#include "ascriptbook.h"
#include "escriptlanguage.h"

#include <QSet>
#include <QHash>
#include <QString>

#include <vector>

class AScriptHub;
class ATextOutputWindow;
class AScriptInterface;
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
class AVirtualScriptManager;
class AScriptExampleExplorer;
class QLabel;

namespace Ui {
class AScriptWindow;
}

class AScriptWindow : public AGuiWindow
{
    Q_OBJECT

public:
    AScriptWindow(EScriptLanguage lang, QWidget * parent);
    ~AScriptWindow();

    void registerInterfaces();
    void updateGui(); //highlighter, helper etc - call it to take into account all changes introduced by introduction of new interface units!

    void WriteToJson();
    void ReadFromJson();

    void onBusyOn();
    void onBusyOff();

    void updateJsonTree();
    void reportError(QString error, int line = 0);   //0 - no line is highligted

    AGuiWindow * ScriptMsgWin = nullptr;

private:
    EScriptLanguage     ScriptLanguage = EScriptLanguage::JavaScript;

    QLabel            * lHelp = nullptr;

    AScriptHub        & ScriptHub;
    A3Global          & GlobSet;

    Ui::AScriptWindow * ui = nullptr;

    AVirtualScriptManager * ScriptManager = nullptr;

    QStringList         Methods;

    std::vector<AScriptBook> ScriptBooks;
    int                 iCurrentBook   = -1;
    QTabWidget *        twBooks        = nullptr;

    int                 iMarkedBook    = -1;
    int                 iMarkedTab     = -1;

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

    bool                bShutDown      = false;

    QSet<QString>       ExpandedItemsInJsonTW;
    //QStringList         functionList; //functions to populate tooltip helper
    std::vector<std::pair<QString,int>> ListOfMethods; //used to populate tooltip
    QHash<QString, QString> DeprecatedOrRemovedMethods;
    QStringList         UnitNames;
    QStringList         ListOfDeprecatedOrRemovedMethods;
    //QStringList         ListOfConstants;

    AScriptExampleExplorer * ExampleExplorer = nullptr;

    void readFromJson(const QJsonObject & json);
    void writeToJson(QJsonObject & json) const;

    void createGuiElements();
    void findText(bool bForward);
    void applyTextFindState();
    void fillSubObject(QTreeWidgetItem * parent, const QJsonObject& obj);
    void fillSubArray(QTreeWidgetItem * parent, const QJsonArray& arr);
    QString getDesc(const QJsonValue & ref);
    void fillHelper(const AScriptInterface * io);
    QString getKeyPath(QTreeWidgetItem * item);
    void getListOfMethods(const AScriptInterface * interface, std::vector<std::pair<QString,int>> & vec, bool withArguments);

    void appendDeprecatedAndRemovedMethods(const AScriptInterface *obj); // !!!***

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
    void updateTab(ATabRecord *tab); // !!!*** move to tab, use references for everything
    void formatTab(ATabRecord *tab); // !!!*** partially move to tab

public slots:
    void clearOutput();
    void outputHtml(QString text);
    void outputText(QString text);
    void outputFromBuffer(std::vector<std::pair<bool, QString>> buffer);

    void onRequestAddScript(const QString & script);
    void onLoadFromFileRequested(const QString & fileName);
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
    void onScriptTabMoved(int from, int to);

    void twBooks_currentChanged(int index);
    void onBookTabMoved(int from, int to);

    //gui buttons
    void on_pbRunScript_clicked();
    void on_pbStop_clicked();
    void on_pbLoad_clicked();
    void on_pbSave_clicked();
    void on_pbSaveAs_clicked();
    void on_pbExample_clicked();
    void on_pbConfig_toggled(bool checked);
    void on_pbHelp_toggled(bool checked);
    void on_pbFileName_clicked();
    void on_pbCloseFindReplaceFrame_clicked();
    void on_pbFindNext_clicked();
    void on_pbFindPrevious_clicked();
    void on_leFind_textChanged(const QString &arg1);
    void on_pbReplaceOne_clicked();
    void on_pbReplaceAll_clicked();

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
    //void onF1pressed(QString text);
    void onF1pressedExtended(std::pair<QString,int> methodNumArgspair);
    void onJsonTWExpanded(QTreeWidgetItem* item);
    void onJsonTWCollapsed(QTreeWidgetItem* item);
    void onDefaulFontSizeChanged(int size);
    void onFindSelected();
    void onReplaceSelected();
    void onFindFunction();
    void onFindVariable(); // !!!*** not python compatible
    void onBack();
    void onForward();

    void receivedOnStart() {emit onStart();}
    void receivedOnAbort();
    void receivedOnSuccess(QString eval);

    void onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *);
    void on_actionSet_tab_equivalent_to_space_characters_triggered();

    void on_aAlphabeticOrder_triggered();

protected:
//    void closeEvent(QCloseEvent * e) override;  // does nothing with the script?
    bool event(QEvent * e) override;

signals:
    void WindowShown(QString);
    void WindowHidden(QString);
    void RequestDraw(TObject* obj, QString options, bool fFocus);
    void onStart();
    void onAbort();
//    void onFinish(bool bError);
    void success(QString eval);   // maybe signal of the manager is enough?
    void requestUpdateConfig();

    void requestUpdateGui();

private:
    void updateMethodHelp();
    void updateRemovedAndDeprecatedMethods();
    void updateAutocompleterAndHeighlighter();
    void addScript(const QString & script);
    QString getQTypeAlias(QString type); // e.g. QString --> string; QVariantList --> array
};

#endif // ASCRIPTWINDOW_H
