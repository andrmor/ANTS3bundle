#include "guitools.h"
#include "a3global.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QFileDialog>

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

QIcon guitools::createColorCircleIcon(QSize size, Qt::GlobalColor color)
{
    /*
    QPixmap pm(size.width()-2, size.height()-2);
    pm.fill(Qt::transparent);
    QPainter b(&pm);
    b.setBrush(QBrush(color));
    if (color == Qt::white) b.setPen(Qt::white);
    b.drawEllipse(0, 2, size.width()-5, size.width()-5);  //was -3 -3 before, and no y shift
    */
    return QIcon(createColorCirclePixmap(size, color));
}

QPixmap guitools::createColorCirclePixmap(QSize size, Qt::GlobalColor color)
{
    QPixmap pm(size.width()-2, size.height()-2);
    pm.fill(Qt::transparent);
    QPainter b(&pm);
    b.setBrush(QBrush(color));
    if (color == Qt::white) b.setPen(Qt::white);
    b.drawEllipse(0, 2, size.width()-5, size.width()-5);  //was -3 -3 before, and no y shift
    return pm;
}

// file pattern example:   "Images (*.png *.xpm *.jpg);;Text files (*.txt);;XML files (*.xml)"
QString guitools::dialogSaveFile(QWidget * parent, const QString & text, const QString & filePattern)
{
    A3Global & GlobSet = A3Global::getInstance();
    QString fileName = QFileDialog::getSaveFileName(parent, text, GlobSet.LastSaveDir, filePattern);
    if (fileName.isEmpty()) return "";
    GlobSet.LastSaveDir = QFileInfo(fileName).absolutePath();
    return fileName;
}

QString guitools::dialogLoadFile(QWidget *parent, const QString &text, const QString &filePattern)
{
    A3Global & GlobSet = A3Global::getInstance();
    QString fileName = QFileDialog::getOpenFileName(parent, text, GlobSet.LastLoadDir, filePattern);
    if (fileName.isEmpty()) return "";
    GlobSet.LastLoadDir = QFileInfo(fileName).absolutePath();
    return fileName;
}

QString guitools::dialogDirectory(QWidget * parent, const QString & text, const QString & initialDir, bool DefaultRead, bool DefaultWrite)
{
    QString dir = QFileDialog::getExistingDirectory(parent, text, initialDir,
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) return "";
    A3Global & GlobSet = A3Global::getInstance();
    if (DefaultRead)  GlobSet.LastLoadDir = dir;
    if (DefaultWrite) GlobSet.LastSaveDir = dir;
    return dir;
}

