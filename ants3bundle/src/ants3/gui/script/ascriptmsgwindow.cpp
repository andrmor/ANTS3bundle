#include "ascriptmsgwindow.h"

#include <QWidget>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QDebug>

AScriptMsgWindow::AScriptMsgWindow(AGuiWindow * parent) :
    AGuiWindow("JsMsg", parent)
{
    QVBoxLayout* l = new QVBoxLayout(this);
    setLayout(l);
    e = new QPlainTextEdit();
    l->addWidget(e);

    setWindowTitle("Script messenger window");
}

void AScriptMsgWindow::clear()
{
    e->clear();
}

void AScriptMsgWindow::append(const QString & text)
{
    if (!isVisible()) show();
    e->appendHtml(text);
}

void AScriptMsgWindow::setFontSize(int size)
{
    QFont f = e->font();
    f.setPointSize(size);
    e->setFont(f);
}
