#ifndef AONELINETEXTEDIT_H
#define AONELINETEXTEDIT_H

#include <QPlainTextEdit>
#include <QSize>

#include <vector>

class QCompleter;

class AOneLineTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    AOneLineTextEdit(const QString & txt = "", QWidget * parent = nullptr);    // !!!*** avoid using palettes: clashes with dark theme!

    void setText(const QString & text); // also sets tooltip
    QString text() const;

    void setFrame(bool flag);

public slots:
    void insertCompletion(const QString & completion);

protected:
    QSize sizeHint() const override;

    void keyPressEvent(QKeyEvent * e) override;
    void focusOutEvent(QFocusEvent * event) override;

private slots:
    void clearTooltip();

signals:
    void enterPressed();
    void editingFinished(); // if enter is pressed, this signal is emitted enterPressed()
    void escapePressed();

public:
    QCompleter * Completer = nullptr;
    bool bIntegerTooltip   = false;
};


#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
class QTextDocument;
struct AHighlightingRule
{
    QRegularExpression pattern;
    QTextCharFormat format;
};

class ABasicHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    ABasicHighlighter(QTextDocument * textDoc) : QSyntaxHighlighter(textDoc){}

    std::vector<AHighlightingRule> HighlightingRules;

protected:
    void highlightBlock(const QString & text) override;
};

#endif // AONELINETEXTEDIT_H
