#ifndef ATEXTOUTPUTWINDOW_H
#define ATEXTOUTPUTWINDOW_H

#include "aguiwindow.h"

#include <QObject>
#include <QString>

class QWidget;
class QPlainTextEdit;

class ATextOutputWindow : public AGuiWindow
{
    Q_OBJECT

public:
    ATextOutputWindow(const QString & idStr, AGuiWindow * parent);

public slots:
    void clear();
    void appendText(const QString & text);
    void appendHtml(const QString & text);

    void setFontSize(int size);

private:
    QPlainTextEdit * e = nullptr;

};

#endif // ATEXTOUTPUTWINDOW_H
