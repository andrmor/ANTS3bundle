#include "ascriptbook.h"
#include "atabrecord.h"
#include "ajsontools.h"

#include <QTabWidget>

AScriptBook::AScriptBook()
{
    TabWidget = new QTabWidget();

    TabWidget->setMovable(true);
    TabWidget->setMinimumHeight(25);
    TabWidget->setContextMenuPolicy(Qt::CustomContextMenu);
}

void AScriptBook::writeToJson(QJsonObject & json) const
{
    json["Name"] = Name;
    json["CurrentTab"] = getCurrentTabIndex();

    QJsonArray ar;
    for (const ATabRecord * r : Tabs)
    {
        QJsonObject js;
        r->writeToJson(js);
        ar.append(js);
    }
    json["ScriptTabs"] = ar;
}

void AScriptBook::setCurrentTabIndex(int index)
{
    if (index < 0 || index >= Tabs.size()) return;
    if (TabWidget) TabWidget->setCurrentIndex(index);
}

void AScriptBook::setTabName(const QString & name, int index)
{
    if (index < 0 || index >= Tabs.size()) return;
    Tabs[index]->TabName = name;
    if (TabWidget) TabWidget->setTabText(index, name);
}

int AScriptBook::getCurrentTabIndex() const
{
    if (!TabWidget) return -1;
    return TabWidget->currentIndex();
}

ATabRecord * AScriptBook::getCurrentTab()
{
    int iCurrentTab = getCurrentTabIndex();
    if (iCurrentTab == -1) return nullptr;
    return Tabs[iCurrentTab];
}

ATabRecord *AScriptBook::getTab(int index)
{
    if (index < 0 || index >= Tabs.size()) return nullptr;
    return Tabs[index];
}

const ATabRecord *AScriptBook::getTab(int index) const
{
    if (index < 0 || index >= Tabs.size()) return nullptr;
    return Tabs[index];
}

QTabWidget *AScriptBook::getTabWidget()
{
    return TabWidget;
}

void AScriptBook::removeTabNoCleanup(int index)
{
    if (index < 0 || index >= Tabs.size()) return;

    if (TabWidget)
    {
        if (index < TabWidget->count()) TabWidget->removeTab(index);
        else qWarning() << "Bad TabWidget index";
    }
    //Tabs.removeAt(index);
    Tabs.erase(Tabs.begin() + index);
}

void AScriptBook::removeTab(int index)
{
    if (index < 0 || index >= Tabs.size()) return;

    if (index < TabWidget->count())
    {
        TabWidget->removeTab(index);
        delete Tabs[index];
        //Tabs.removeAt(index);
        Tabs.erase(Tabs.begin() + index);
    }
}

void AScriptBook::removeAllTabs()
{
    for (int i = Tabs.size() - 1; i > -1; i--)
        removeTab(i);
}
