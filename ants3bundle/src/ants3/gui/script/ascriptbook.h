#ifndef ASCRIPTBOOK_H
#define ASCRIPTBOOK_H

#include <QString>

#include <vector>

class QTabWidget;
class ATabRecord;
class QJsonObject;

class AScriptBook
{
public:
    AScriptBook();

    QString            Name;
    std::vector<ATabRecord*> Tabs;
    QTabWidget       * TabWidget   = nullptr; // will be owned by the QTabItemWidget

    void               writeToJson(QJsonObject & json) const;
    //bool             readFromJson(const QJsonObject & json);  // too heavily relies on AScriptWindow, cannot be implemented here without major refactoring

    int                getCurrentTabIndex() const;
    void               setCurrentTabIndex(int index); // !!!*** possible to use size_t?

    void               setTabName(const QString & name, int index);

    ATabRecord       * getCurrentTab();
    ATabRecord       * getTab(int index);
    const ATabRecord * getTab(int index) const;
    QTabWidget       * getTabWidget();

    void               removeTabNoCleanup(int index); //used by move
    void               removeTab(int index);
    void               removeAllTabs();

    QString            saveAllModifiedFiles();
};

#endif // ASCRIPTBOOK_H
