#include "atextoutputwindow.h"

#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QScrollBar>
#include <QDebug>

ATextOutputWindow::ATextOutputWindow(const QString & idStr, AGuiWindow * parent) :
    AGuiWindow(idStr, parent)
{
    e = new QTextEdit();
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

    //e->appendPlainText(text);
    QTextCursor cursor(e->document());
    cursor.movePosition(QTextCursor::End);
    if (!e->document()->isEmpty())
        cursor.insertBlock(QTextBlockFormat(), QTextCharFormat()); // new paragraph, reset formatting
    cursor.insertText(text, QTextCharFormat());                    // inserted literally, never as HTML

    e->verticalScrollBar()->setValue(e->verticalScrollBar()->maximum());
}

void ATextOutputWindow::appendHtml(const QString & text)
{
    if (!isVisible()) show();
    //e->appendHtml(text);
    e->append(text);
}

void ATextOutputWindow::setFontSize(int size)
{
    QFont f = e->font();
    f.setPointSize(size);
    e->setFont(f);
}
