#ifndef ATEXTEDIT_H
#define ATEXTEDIT_H

#include "escriptlanguage.h"

#include <QPlainTextEdit>
#include <QObject>
#include <QWidget>
#include <QSet>

class QCompleter;
class ALeftField;

class ATextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    ATextEdit(QWidget * parent = nullptr);
    ~ATextEdit() {}

    void setCompleter(QCompleter * completer);

    void setScriptLanguage(EScriptLanguage lang) {ScriptLanguage = lang;}

    void SetFontSize(int size);
    void RefreshExtraHighlight();
    void setTextCursorSilently(const QTextCursor & tc);

    void setDeprecatedOrRemovedMethods(const QHash<QString, QString>* DepRem) {DeprecatedOrRemovedMethods = DepRem;}

    int & TabInSpaces;
    QStringList functionList;
    QString FindString;
    const QHash<QString, QString> * DeprecatedOrRemovedMethods = nullptr;

public slots:
    void paste();
    void align();

protected:
    bool event(QEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void insertCompletion(const QString &completion);    
    void onCursorPositionChanged(); // !!!***
    void updateLineNumberAreaWidth();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    friend class ALeftField;
    int  previousLineNumber = 0; // !!!*** checked by not set
    bool bMonitorLineChange = true;
    EScriptLanguage ScriptLanguage = EScriptLanguage::JavaScript;
    QCompleter * Completer = nullptr;
    ALeftField * LeftField  = nullptr;
    bool Pressed_2 = false;

    QString textUnderCursor() const;
    QString SelectObjFunctUnderCursor(QTextCursor * cursor = nullptr) const;
    QString SelectTextToLeft(QTextCursor cursor, int num) const;
    bool InsertClosingBracket() const;
    bool findInList(QString text, QString &tmp) const;
    void setFontSizeAndEmitSignal(int size);

    void paintLeftField(QPaintEvent *event); // !!!*** make compatible with dark theme
    int  getWidthLeftField() const;

    void checkBracketsOnLeft(QList<QTextEdit::ExtraSelection> &extraSelections, const QColor &color);
    void checkBracketsOnRight(QList<QTextEdit::ExtraSelection> &extraSelections, const QColor &color);

    bool TryShowFunctionTooltip(QTextCursor *cursor);

    int getIndent(const QString &line) const;
    void setIndent(QString &line, int indent);
    void convertTabToSpaces(QString &line);
    int getSectionCounterChange(const QString &line) const;
    void pasteText(const QString &text);

    bool onKeyPressed_interceptShortcut(int key, bool shift);

signals:
    void requestHelp(QString);
    void editingFinished();
    void fontSizeChanged(int size);
    void lineNumberChanged(int lineNumber);

};

class ALeftField : public QWidget
{
public:
    ALeftField(ATextEdit& edit) : QWidget(&edit), edit(edit) {}

    QSize sizeHint() const override { return QSize(edit.getWidthLeftField(), 0); }
protected:
    void paintEvent(QPaintEvent *event) override { edit.paintLeftField(event); }

private:
    ATextEdit& edit;
};

#endif // ATEXTEDIT_H
