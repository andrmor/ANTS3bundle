#include "ahighlighters.h"
#include "guitools.h"

#include <QDebug>

AHighlighter::AHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent){}

void AHighlighter::setExternalRules(const QStringList & units, const QStringList & functions, const QStringList & deprecatedOrRemoved)
{
    if (HighlightingRulesInEffect.size() > HighlightingRulesPermanent.size()) return;

    std::vector<HighlightingRule> hr;

    HighlightingRule rule;

    QTextCharFormat customKeywordFormat;
    QColor color = ( guitools::isDarkTheme() ? Qt::cyan : Qt::darkCyan );
    customKeywordFormat.setForeground(color.darker(110));
    //customKeywordFormat.setFontWeight(QFont::Bold);
    //customKeywordFormat.setFontItalic(true);
    for (const QString& pattern : functions)
    {
        rule.Pattern = QRegularExpression("\\b"+pattern+"(?=\\()");
        rule.Format = customKeywordFormat;
        hr.push_back(rule);

        //QStringList f = pattern.split(".", Qt::SkipEmptyParts);
        //if (f.size() > 1 && !f.first().isEmpty()) units << f.first();
    }

    QTextCharFormat deprecatedOrRemovedFormat;
    color = Qt::red;
    deprecatedOrRemovedFormat.setForeground(color.darker(110));
    for (const QString& pattern : deprecatedOrRemoved)
    {
        rule.Pattern = QRegularExpression("\\b"+pattern+"(?=\\()");
        rule.Format = deprecatedOrRemovedFormat;
        hr.push_back(rule);
    }

    QTextCharFormat unitFormat;
    unitFormat.setForeground( guitools::isDarkTheme() ? Qt::magenta : Qt::darkMagenta);
    for (const QString& pattern : units)
    {
        rule.Pattern = QRegularExpression("\\b"+pattern+"\\b");
        rule.Format = unitFormat;
        hr.push_back(rule);
    }

    //HighlightingRulesInEffect = hr + HighlightingRulesPermanent; //so e.g. comments and quatation rule have higher priority
    HighlightingRulesInEffect.clear();
    HighlightingRulesInEffect.insert( HighlightingRulesInEffect.end(), hr.begin(), hr.end() );
    HighlightingRulesInEffect.insert( HighlightingRulesInEffect.end(), HighlightingRulesPermanent.begin(), HighlightingRulesPermanent.end() );
}

void AHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule & rule : HighlightingRulesInEffect)
    {
        QRegularExpressionMatchIterator matchIterator = rule.Pattern.globalMatch(text);
        while ( matchIterator.hasNext() )
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.Format);
        }
    }

    if (bMultilineCommentAllowed)
    {
        setCurrentBlockState(0);

        int startIndex = 0;
        if (previousBlockState() != 1)
            startIndex = text.indexOf(commentStartExpression);

        while (startIndex >= 0)
        {
            QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
            int endIndex = match.capturedStart();
            int commentLength = 0;
            if (endIndex == -1)
            {
                setCurrentBlockState(1);
                commentLength = text.length() - startIndex;
            }
            else
                commentLength = endIndex - startIndex + match.capturedLength();

            setFormat(startIndex, commentLength, multiLineCommentFormat);
            startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
        }
    }
}

// ---------------------

AHighlighterJS::AHighlighterJS(QTextDocument * parent) :
    AHighlighter(parent)
{
    HighlightingRule rule;

    QTextCharFormat keywordFormat;
    keywordFormat.setForeground( guitools::isDarkTheme() ? QColor(Qt::blue).lighter(150) : Qt::blue);
    //keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bbreak\\b" << "\\bcatch\\b" << "\\bcontinue\\b" << "\\b.length\\b" << "\\barguments\\b"
                    << "\\bdo\\b" << "\\bwhile\\b" << "\\bfor\\b" << "\\bwith\\b" << "\\bdelete\\b"
                    << "\\bin\\b" << "\\bfunction\\b" << "\\bif\\b"
                    << "\\belse\\b" << "\\breturn\\b" << "\\bswitch\\b"
                    << "\\bthrow\\b" << "\\btry\\b"
                    << "\\blet\\b" << "\\bconst\\b" << "\\bvar\\b"
                    << "\\Array\\b"  // !!!***
                    << "\\bpush\\b" << "\\btypeof\\b"
                    << "\\bsplice\\b"
                    << "\\bMath\\s*.\\b" << "\\bArray\\s*.\\b" << "\\bString\\s*.\\b";
    for (const QString & pattern : keywordPatterns)
    {
        rule.Pattern = QRegularExpression(pattern);
        if (!rule.Pattern.isValid()) qDebug() << "-------------------------" << pattern;
        rule.Format = keywordFormat;
        HighlightingRulesPermanent.push_back(rule);
    }

    QTextCharFormat includeFormat;
    includeFormat.setForeground(QColor(227, 146, 48));
    rule.Pattern = QRegularExpression("#include\\b");
    rule.Format = includeFormat;
    HighlightingRulesPermanent.push_back(rule);

    /*
    QTextCharFormat classFormat;
    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta);
    rule.Pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.Format = classFormat;
    highlightingRules.append(rule);
    */

    QTextCharFormat singleLineCommentFormat;
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.Pattern = QRegularExpression("//[^\n]*");
    rule.Format = singleLineCommentFormat;
    HighlightingRulesPermanent.push_back(rule);

    multiLineCommentFormat.setForeground(Qt::darkGreen);

    QTextCharFormat quotationFormat;
    quotationFormat.setForeground(guitools::isDarkTheme() ? QColor(Qt::darkGreen).lighter(150) : Qt::darkGreen);
    QRegularExpression rx("\"([^\"\\\\]*(\\\\.[^\"\\\\]*)*)\"|\'([^\'\\\\]*(\\\\.[^\'\\\\]*)*)\'");
    //qDebug() << "----------------------"<< rx.isValid();
    //rx.setMinimal(true); //fixes the problem with "xdsfdsfds" +variable+ "dsfdsfdsf"
    rule.Pattern = rx;
    rule.Format = quotationFormat;
    HighlightingRulesPermanent.push_back(rule);

    /*
    QTextCharFormat charFormat;
    charFormat.setForeground(Qt::darkGreen);
    rule.Pattern = QRegularExpression("'.*'");
    rule.Format = charFormat;
    highlightingRules.append(rule);
    */

    /*
    QTextCharFormat functionFormat;
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.Pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.Format = functionFormat;
    highlightingRules.append(rule);
    */

    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression   = QRegularExpression("\\*/");
}

// ---------------------

AHighlighterPython::AHighlighterPython(QTextDocument *parent) :
    AHighlighter(parent)
{
    bMultilineCommentAllowed = false;
    multiLineCommentFormat.setForeground(Qt::darkGreen);

    HighlightingRule rule;

    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(Qt::blue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "and" << "as" << "assert" << "break" << "class" << "close" << "continue" <<
                       "def" << "del" << "elif" << "else" << "except" <<
                       "exec" << "False" << "finally" << "float" << "for" << "from" << "global" <<
                       "if" << "import" << "in" << "int" << "input" << "is" << "lambda" << "list" <<
                       "None" << "nonlocal" << "not" << "or" << "open" << "pass" << "raise" << "range" <<
                       "return" << "string" << "True"<< "try" << "tuple" <<
                       "type" << "with" << "while" << "yield" <<
                    // and common functions
                    "abs" << "len" << "max" << "min" << "print" << "round" << "sorted" << "sum";

    for (const QString &pattern : keywordPatterns)
    {
        QString pattern1 = QString("\\b") + pattern + "\\b";

        rule.Pattern = QRegularExpression(pattern1);
        rule.Format = keywordFormat;
        HighlightingRulesPermanent.push_back(rule);
    }

    /*
    QTextCharFormat classFormat;
    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta);
    rule.Pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.Format = classFormat;
    highlightingRules.append(rule);
    */

    QTextCharFormat singleLineCommentFormat;
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.Pattern = QRegularExpression("#[^\n]*");
    rule.Format = singleLineCommentFormat;
    HighlightingRulesPermanent.push_back(rule);

    QTextCharFormat quotationFormat;
    quotationFormat.setForeground(Qt::darkGreen);
    //QRegularExpression rx("((?<![\\\\])['\"])((?:.(?!(?<![\\\\])\\1))*.?)\\1");
    QRegularExpression rx("\"([^\"\\\\]*(\\\\.[^\"\\\\]*)*)\"|\'([^\'\\\\]*(\\\\.[^\'\\\\]*)*)\'");
    //qDebug() << "----------------------"<< rx.isValid();
    rule.Pattern = rx;
    rule.Format = quotationFormat;
    HighlightingRulesPermanent.push_back(rule);

    /*
    QTextCharFormat functionFormat;
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.Pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.Format = functionFormat;
    highlightingRules.append(rule);
    */

    QTextCharFormat includeFormat;
    includeFormat.setForeground(QColor(227, 146, 48));
    rule.Pattern = QRegularExpression("^[ \t]*#include\\b");
    rule.Format = includeFormat;
    HighlightingRulesPermanent.push_back(rule);
}
