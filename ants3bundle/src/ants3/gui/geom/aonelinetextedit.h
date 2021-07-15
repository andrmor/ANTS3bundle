#ifndef AONELINETEXTEDIT_H
#define AONELINETEXTEDIT_H

#include <QPlainTextEdit>
#include <QSize>

class QCompleter;

class AOneLineTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    AOneLineTextEdit(QWidget * parent = nullptr);

    void setText(const QString & text);
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
    void editingFinished();
    void escapePressed();

public:
    QCompleter * Completer = nullptr;
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

    QVector<AHighlightingRule> HighlightingRules;

protected:
    void highlightBlock(const QString & text) override;
};

#endif // AONELINETEXTEDIT_H
