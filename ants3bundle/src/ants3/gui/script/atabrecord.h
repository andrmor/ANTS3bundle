#ifndef ATABRECORD_H
#define ATABRECORD_H

#include "escriptlanguage.h"

#include <QObject>
#include <QList>

class ATextEdit;
class QCompleter;
class QStringListModel;
class AHighlighter;
class QJsonObject;
class QPoint;
class QLabel;

class ATabRecord : public QObject
{
    Q_OBJECT
public:
    ATabRecord(const QStringList & functions, EScriptLanguage language, QLabel * labelHelpTooltip);
    ~ATabRecord();

    ATextEdit         * TextEdit = nullptr;

    QString             FileName;
    QString             TabName;
    bool                bExplicitlyNamed = false;   //if true save will not auto-rename

    const QStringList & Functions;

    QCompleter        * Completer = nullptr;
    QStringListModel  * CompletitionModel;
    AHighlighter      * Highlighter = nullptr;

    QList<int>          VisitedLines; // access from both sides
    int                 IndexVisitedLines = 0;
    int                 MaxLineNumbers    = 20;

    void updateHighlight();

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    bool wasModified() const;
    void setModifiedStatus(bool flag);

    void goBack();
    void goForward();

    bool saveTextToFile(const QString & fileName) const;

private slots:
    void onCustomContextMenuRequested(const QPoint & pos);
    void onLineNumberChanged(int lineNumber);
    void onTextChanged(); // !!!*** not valid approach for Python!

signals:
    void requestFindText();
    void requestReplaceText();
    void requestFindFunction();
    void requestFindVariable();
};

#endif // ATABRECORD_H
