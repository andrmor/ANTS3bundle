#ifndef ASCRIPTMSGWINDOW_H
#define ASCRIPTMSGWINDOW_H

#include "aguiwindow.h"

#include <QObject>
#include <QString>

class QWidget;
class QPlainTextEdit;

class AScriptMsgWindow : public AGuiWindow
{
    Q_OBJECT

public:
    AScriptMsgWindow(AGuiWindow * parent);

public slots:
    void clear();
    void append(const QString & text);

    void setFontSize(int size);

private:
    QPlainTextEdit * e = nullptr;

};

#endif // ASCRIPTMSGWINDOW_H
