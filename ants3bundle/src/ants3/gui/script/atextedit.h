#ifndef ATEXTEDIT_H
#define ATEXTEDIT_H

#include "escriptlanguage.h"

#include <vector>

#include <QPlainTextEdit>
#include <QObject>
#include <QWidget>
#include <QLabel>

class QCompleter;
class ALeftField;

class ATextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    ATextEdit(EScriptLanguage lang, QWidget * parent = nullptr);
    ~ATextEdit() {}

    void setCompleter(QCompleter * completer);

    void setFontSize(int size);
    void refreshExtraHighlight();
    void setTextCursorSilently(const QTextCursor & tc);

    void setDeprecatedOrRemovedMethods(const QHash<QString, QString>* DepRem) {DeprecatedOrRemovedMethods = DepRem;}

    void showMethodHelpForCursor();

    bool saveTextToFile(const QString & fileName) const;

    void hideHelpLabel();

    std::vector<std::pair<QString,int>> * ListOfMethods = nullptr;
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

    EScriptLanguage ScriptLanguage = EScriptLanguage::JavaScript;
    int & TabInSpaces;

    int  previousLineNumber = 0;
    bool bMonitorLineChange = true;
    QCompleter * Completer = nullptr;
    ALeftField * LeftField  = nullptr;
    bool Pressed_2 = false;

    bool   MethodTooltipVisible = false;
    bool   ForcedMethodTooltipSelection = false;
    size_t SelectedMethodInTooltip  = 0;
    size_t NumberOfMethodsInTooltip = 1;

    QLabel * lHelp = nullptr;

    QString textUnderCursor() const;
    QString selectObjFunctUnderCursor(QTextCursor * cursor = nullptr) const;
    QString SelectTextToLeft(QTextCursor cursor, int num) const;
    bool InsertClosingBracket() const;
    void findMatchingMethods(const QString & text, std::vector<std::pair<QString,int>> & pairs) const;  // !!!*** add to search in deprecated & removed !!!*** optimize
    void setFontSizeAndEmitSignal(int size);

    void paintLeftField(QPaintEvent *event); // !!!*** make compatible with dark theme
    int  getWidthLeftField() const;

    void checkBracketsOnLeft(QList<QTextEdit::ExtraSelection> &extraSelections, const QColor &color);
    void checkBracketsOnRight(QList<QTextEdit::ExtraSelection> &extraSelections, const QColor &color);

    bool tryShowFunctionTooltip(const QTextCursor & cursor);

    int getIndent(const QString &line) const;
    void setIndent(QString &line, int indent);
    void convertTabToSpaces(QString &line);
    int getSectionCounterChange(const QString &line) const;
    void pasteText(const QString &text);

    bool onKeyPressed_interceptShortcut(int key, bool shift);

    int computeIntroducedNumberOfArguments(const QTextCursor & cursor, bool cursorInArguments);
    int computeCurrentArgument(QTextCursor & tc);

    void findMathcingMethodsForCursor(const QTextCursor & cursor, std::vector<std::pair<QString, int>> & matchingMethods, bool & cursorIsInArguments);

signals:
    //void requestHelp(QString); // !!!*** to be removed
    void requestHelpWithArgs(std::pair<QString,int> methodNumArgsPair);
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
