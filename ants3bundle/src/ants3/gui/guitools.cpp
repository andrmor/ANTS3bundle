#include "guitools.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QApplication>
#include <QScreen>

void guitools::message(QString text, QWidget* parent)
{
    QMessageBox mb(parent);
    mb.setWindowFlags(mb.windowFlags() | Qt::WindowStaysOnTopHint);
    mb.setText(text);
    if (!parent) mb.move(200,200);
    mb.exec();
}

bool guitools::confirm(const QString & text, QWidget * parent)
{
    QMessageBox::StandardButton reply = QMessageBox::question(parent, "Confirmation request", text,
                                                              QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Yes);
    if (reply == QMessageBox::Yes) return true;
    return false;
}

void guitools::message1(const QString & text, const QString & title, QWidget *parent)
{
    QDialog d(parent);
    QVBoxLayout * l = new QVBoxLayout(&d);
    QPlainTextEdit * e = new QPlainTextEdit;
    e->appendPlainText(text);
    l->addWidget(e);
    QPushButton * pb = new QPushButton("Close");
    QObject::connect(pb, &QPushButton::clicked, &d, &QDialog::accept);
    l->addWidget(pb);

    QTextCursor curs = e->textCursor();
    curs.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    e->setTextCursor(curs);

    d.resize(800, 400);
    d.setWindowTitle(title);
    d.exec();
}

void guitools::inputInteger(const QString &text, int &input, int min, int max, QWidget *parent)
{
    bool ok;
    int res = QInputDialog::getInt(parent, "", text, input, min, max, 1, &ok);
    if (ok) input = res;
}

void guitools::inputString(const QString & label, QString & input, QWidget *parent)
{
    bool ok;
    QString res = QInputDialog::getText(parent, "", label, QLineEdit::Normal, input, &ok);
    if (ok) input = res;
}

bool guitools::AssureWidgetIsWithinVisibleArea(QWidget * w)
{
    QList<QScreen *> listScr = qApp->screens();

    bool bVis = false;
    for (QScreen * scr : qAsConst(listScr))
    {
        QRect ts = scr->geometry();
        if (ts.contains(w->x(), w->y()))
        {
            bVis = true;
            break;
        }
    }
    if (!bVis) w->move(50,50);

    return bVis;
}
