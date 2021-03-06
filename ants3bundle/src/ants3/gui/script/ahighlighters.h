#ifndef AHIGHLIGHTERS_H
#define AHIGHLIGHTERS_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>

struct HighlightingRule
{
    QRegularExpression Pattern;
    QTextCharFormat    Format;
};

class AHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    AHighlighter(QTextDocument * parent);

    void setExternalRules(const QStringList & units, const QStringList & functions, const QStringList & deprecatedOrRemoved, const QStringList & constants);

protected:
    void highlightBlock(const QString & text);

    QVector<HighlightingRule> highlightingRules; // !!!*** to std::vector

    bool bMultilineCommentAllowed = true;
    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;
    QTextCharFormat multiLineCommentFormat;
};

class AHighlighterJS : public AHighlighter
{
    Q_OBJECT
public:
    AHighlighterJS(QTextDocument * parent = nullptr);
};

class AHighlighterPython : public AHighlighter
{
    Q_OBJECT
public:
    AHighlighterPython(QTextDocument * parent = nullptr);
};

#endif // AHIGHLIGHTERS_H
