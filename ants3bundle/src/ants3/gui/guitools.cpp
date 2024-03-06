#include "guitools.h"
#include "a3global.h"
#include "ajsontools.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QFileDialog>

void guitools::message(QString text, QWidget* parent)
{
    QMessageBox mb(parent);
    mb.setWindowTitle("ANTS3"); //"ANTS3 message");
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

void guitools::message1notModal(const QString &text, const QString &title, QWidget *parent)
{
    QDialog * d = new QDialog(parent);
    d->setModal(false);
    QVBoxLayout * l = new QVBoxLayout(d);
    QPlainTextEdit * e = new QPlainTextEdit();
    e->appendPlainText(text);
    l->addWidget(e);
    QPushButton * pb = new QPushButton("Close");
    QObject::connect(pb, &QPushButton::clicked, d, &QDialog::reject);
    QObject::connect(d, &QDialog::rejected, d, &QObject::deleteLater);
    l->addWidget(pb);

    QTextCursor curs = e->textCursor();
    curs.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    e->setTextCursor(curs);

    d->resize(800, 400);
    d->setWindowTitle(title);
    d->show();
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

bool guitools::assureWidgetIsWithinVisibleArea(QWidget * w)
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

#include <QRegularExpression>
bool guitools::extractNumbersFromQString(const QString & input, std::vector<int> & extracted)
{
    const QRegularExpression rx("(\\,|\\-)");
    QStringList fields = input.split(rx, Qt::SkipEmptyParts);
    if (fields.size() == 0) return false;

    fields = input.split(',', Qt::SkipEmptyParts);

    for (int i=0; i<fields.size(); i++)
    {
        const QString & thisField = fields[i];

        //are there "-" separated fields?
        QStringList subFields = thisField.split("-", Qt::SkipEmptyParts);

        if (subFields.size() > 2 || subFields.size() == 0) return false;
        else if (subFields.size() == 1)
        {

            bool ok;
            int iVal = subFields[0].toInt(&ok);
            if (ok) extracted.push_back(iVal);
            else return false;
        }
        else
        {
            bool ok1, ok2;
            int iVal1 = subFields[0].toInt(&ok1);
            int iVal2 = subFields[1].toInt(&ok2);
            if (ok1 && ok2)
            {
                if (iVal2 < iVal1) return false;
                else
                {
                    for (int j = iVal1; j <= iVal2; j++)
                        extracted.push_back(j);
                }
            }
            else return false;
        }
    }
    return true;
}

bool guitools::isDarkTheme()
{
    const QPalette defaultPalette;
    return defaultPalette.color(QPalette::WindowText).lightness()
         > defaultPalette.color(QPalette::Window).lightness();
}

void guitools::parseJsonToQLineEdit(const QJsonObject & json, const QString & name, QLineEdit * le)
{
    QString str;
    bool ok = jstools::parseJson(json, name, str);
    if (ok) le->setText(str);
}

void guitools::parseJsonToQCheckBox(const QJsonObject &json, const QString &name, QCheckBox *cb)
{
    bool flag;
    bool ok = jstools::parseJson(json, name, flag);
    if (ok) cb->setChecked(flag);
}
