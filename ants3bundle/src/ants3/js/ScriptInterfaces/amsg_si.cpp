#include "amsg_si.h"
//#include "ajavascriptmanager.h"
#include "atextoutputwindow.h"

#include <QTimer>
#include <QDebug>

AMsg_SI::AMsg_SI(ATextOutputWindow * msgWin) :
    AWindowInterfaceBase(msgWin), MsgWin(msgWin)
{
    Description = "Output to script message window";
}

void AMsg_SI::appendText(const QString & text)
{
    QTimer::singleShot(0, BaseWindow, [this, &text]()
    {
        MsgWin->appendText(text);
    } );
}

void AMsg_SI::appendHtml(const QString & text)
{
    QTimer::singleShot(0, BaseWindow, [this, &text]()
    {
        MsgWin->appendHtml(text);
    } );
}

void AMsg_SI::clear()
{
    QTimer::singleShot(0, BaseWindow, [this]()
    {
        MsgWin->clear();
    } );
}

void AMsg_SI::setFontSize(int size)
{
    QTimer::singleShot(0, BaseWindow, [this, size]()
    {
        MsgWin->setFontSize(size);
    } );
}

/*
void AMsg_SI::RestorelWidget()
{
    if (bGuiThread)
    {
        if (DialogWidget->IsShown())
            DialogWidget->RestoreWidget();
    }        
}

void AMsg_SI::HideWidget()
{
    if (bGuiThread)
    {
        if (DialogWidget->IsShown())
            DialogWidget->HideWidget();
    }
}
*/
