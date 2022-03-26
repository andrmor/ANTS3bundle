#include "ahighlighters.h"

#include <QDebug>

AHighlighter::AHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent){}

void AHighlighter::setExternalRules(const QStringList & units, const QStringList & functions, const QStringList & deprecatedOrRemoved, const QStringList & constants)
{
    QVector<HighlightingRule> hr;

    HighlightingRule rule;

    QTextCharFormat customKeywordFormat;
    QColor color = Qt::darkCyan;
    customKeywordFormat.setForeground(color.darker(110));
    //customKeywordFormat.setFontWeight(QFont::Bold);
    //customKeywordFormat.setFontItalic(true);
    for (const QString& pattern : functions)
    {
        rule.Pattern = QRegularExpression("\\b"+pattern+"(?=\\()");
        rule.Format = customKeywordFormat;
        hr.append(rule);

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
        hr.append(rule);
    }

    QTextCharFormat unitFormat;
    unitFormat.setForeground(Qt::darkMagenta);
    for (const QString& pattern : units)
    {
        rule.Pattern = QRegularExpression("\\b"+pattern+"\\b");
        rule.Format = unitFormat;
        hr.append(rule);
    }

    /*
    for (const QString &pattern : constants)
    {
        rule.pattern = QRegularExpression("\\b"+pattern+"\\b(?![\\(\\{\\[])");
        rule.format = customKeywordFormat;
        hr.append(rule);
    }
    */

    highlightingRules = hr + highlightingRules; //so e.g. comments and quatation rule have higher priority
}

void AHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : highlightingRules)
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
    keywordFormat.setForeground(Qt::blue);
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
        highlightingRules.append(rule);
    }

    QTextCharFormat includeFormat;
    includeFormat.setForeground(QColor(227, 146, 48));
    rule.Pattern = QRegularExpression("#include\\b");
    rule.Format = includeFormat;
    highlightingRules.append(rule);

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
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::darkGreen);

    QTextCharFormat quotationFormat;
    quotationFormat.setForeground(Qt::darkGreen);
    QRegularExpression rx("\"([^\"\\\\]*(\\\\.[^\"\\\\]*)*)\"|\'([^\'\\\\]*(\\\\.[^\'\\\\]*)*)\'");
    //qDebug() << "----------------------"<< rx.isValid();
    //rx.setMinimal(true); //fixes the problem with "xdsfdsfds" +variable+ "dsfdsfdsf"
    rule.Pattern = rx;
    rule.Format = quotationFormat;
    highlightingRules.append(rule);

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
    keywordPatterns << "and" << "assert" << "break" << "class" << "continue" <<
                       "def" << "del" << "elif" << "else" << "except" <<
                       "exec" << "finally" << "for" << "from" << "global" <<
                       "if" << "import" << "in" << "is" << "lambda" <<
                       "not" << "or" << "pass" << "print" << "raise" <<
                       "return" << "try" << "while" <<
                       "Data" << "Float" << "Int" << "Numeric" << "Oxphys" <<
                       "array" << "close" << "float" << "int" << "input" <<
                       "open" << "range" << "type" << "write" << "zeros";

    for (const QString &pattern : keywordPatterns)
    {
        QString pattern1 = QString("\\b") + pattern + "\\b";

        rule.Pattern = QRegularExpression(pattern1);
        rule.Format = keywordFormat;
        highlightingRules.append(rule);
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
    highlightingRules.append(rule);

    QTextCharFormat quotationFormat;
    quotationFormat.setForeground(Qt::darkGreen);
    //QRegularExpression rx("((?<![\\\\])['\"])((?:.(?!(?<![\\\\])\\1))*.?)\\1");
    QRegularExpression rx("\"([^\"\\\\]*(\\\\.[^\"\\\\]*)*)\"|\'([^\'\\\\]*(\\\\.[^\'\\\\]*)*)\'");
    //qDebug() << "----------------------"<< rx.isValid();
    rule.Pattern = rx;
    rule.Format = quotationFormat;
    highlightingRules.append(rule);

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
    highlightingRules.append(rule);
}
