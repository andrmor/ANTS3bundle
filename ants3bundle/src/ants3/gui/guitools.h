#ifndef GUITOOLS_H
#define GUITOOLS_H

#include <QString>
#include <QIcon>

#include <vector>

class QWidget;

namespace guitools
{
    void message(QString text, QWidget* parent = nullptr);
    void message1(const QString & text, const QString &title, QWidget * parent = nullptr);
    bool confirm(const QString &text, QWidget* parent = nullptr);

    void inputInteger(const QString & text, int & input, int min, int max, QWidget * parent = nullptr);
    void inputString(const QString & label, QString & input, QWidget * parent = nullptr);

    bool assureWidgetIsWithinVisibleArea(QWidget* w);

    QIcon   createColorCircleIcon(QSize size, Qt::GlobalColor color);
    QPixmap createColorCirclePixmap(QSize size, Qt::GlobalColor color);

    QString dialogSaveFile(QWidget * parent, const QString & text, const QString & filePattern);
    QString dialogLoadFile(QWidget * parent, const QString & text, const QString & filePattern);

    QString dialogDirectory(QWidget * parent, const QString & text, const QString & initialDir, bool DefaultRead = false, bool DefaultWrite = false);

    bool    extractNumbersFromQString(const QString & input, std::vector<int> & extracted);
}

#endif // GUITOOLS_H
