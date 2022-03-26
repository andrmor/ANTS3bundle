#ifndef ATABRECORD_H
#define ATABRECORD_H

#include "ascriptlanguageenum.h"

#include <QObject>
#include <QVector>

class ATextEdit;
class QCompleter;
class QStringListModel;
class AHighlighter;
class QJsonObject;
class QStringList;
class QPoint;

class ATabRecord : public QObject
{
    Q_OBJECT
public:
    ATabRecord(const QStringList & functions, AScriptLanguageEnum language);
    ~ATabRecord();

    ATextEdit *     TextEdit            = nullptr;

    QString         FileName;
    QString         TabName;
    bool            bExplicitlyNamed    = false;   //if true save will not auto-rename

    const QStringList & Functions;

    QCompleter *    Completer           = nullptr;
    QStringListModel * CompletitionModel;
    AHighlighter *  Highlighter = nullptr;

    QVector<int>    VisitedLines;   // !!!*** to std::vector
    int             IndexVisitedLines   = 0;
    int             MaxLineNumbers      = 20;

    void updateHighlight();

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject &json);

    bool wasModified() const;
    void setModifiedStatus(bool flag);

    void goBack();
    void goForward();

private slots:
    void onCustomContextMenuRequested(const QPoint & pos);
    void onLineNumberChanged(int lineNumber);
    void onTextChanged();

signals:
    void requestFindText();
    void requestReplaceText();
    void requestFindFunction();
    void requestFindVariable();
};

#endif // ATABRECORD_H
