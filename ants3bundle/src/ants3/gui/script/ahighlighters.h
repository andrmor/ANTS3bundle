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

    void setExternalRules(const QStringList & units, const QStringList &functions, const QStringList &deprecatedOrRemoved, const QStringList &constants);

protected:
    void highlightBlock(const QString & text);

    virtual void setLanguageRules() {}

    bool bMultilineCommentAllowed = true;
    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;

    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat customKeywordFormat;
    QTextCharFormat unitFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat charFormat;
    QTextCharFormat deprecatedOrRemovedFormat;
};

class AHighlighterJS : public AHighlighter
{
    Q_OBJECT
public:
    AHighlighterJS(QTextDocument * parent = nullptr);

protected:
    void setLanguageRules() override;
};

class AHighlighterPython : public AHighlighter
{
    Q_OBJECT
public:
    AHighlighterPython(QTextDocument * parent = nullptr);

    void setLanguageRules() override;
};

#endif // AHIGHLIGHTERS_H
