#include "atextoutputwindow.h"

#include <QWidget>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QDebug>

ATextOutputWindow::ATextOutputWindow(const QString & idStr, AGuiWindow * parent) :
    AGuiWindow(idStr, parent)
{
    e = new QPlainTextEdit();
    setCentralWidget(e);
    QVBoxLayout * l = new QVBoxLayout();
    centralWidget()->setLayout(l);
}

void ATextOutputWindow::clear()
{
    e->clear();
}

void ATextOutputWindow::appendText(const QString & text)
{
    if (!isVisible()) show();
    e->appendHtml(text);
}

void ATextOutputWindow::appendHtml(const QString & text)
{
    if (!isVisible()) show();
    e->appendPlainText(text);
}

void ATextOutputWindow::setFontSize(int size)
{
    QFont f = e->font();
    f.setPointSize(size);
    e->setFont(f);
}
