#ifndef GUITOOLS_H
#define GUITOOLS_H

#include <QString>

class QWidget;

namespace guitools
{
    void message(QString text, QWidget* parent = nullptr);
    void message1(const QString & text, const QString &title, QWidget * parent = nullptr);
    bool confirm(const QString &text, QWidget* parent = nullptr);

    void inputInteger(const QString & text, int & input, int min, int max, QWidget * parent = nullptr);
    void inputString(const QString & label, QString & input, QWidget * parent = nullptr);
}

#endif // GUITOOLS_H
