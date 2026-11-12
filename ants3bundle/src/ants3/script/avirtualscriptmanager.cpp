#include "avirtualscriptmanager.h"

#include <QtGlobal>
#include <QJSValue>

void AVirtualScriptManager::addQVariantToString(const QVariant & var, QString & string, EScriptLanguage lang, bool bAddQuotation)
{
    if (var.isNull()) return;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (var.typeName() == QStringLiteral("QJSValue") )
    {
        addQVariantToString(var.value<QJSValue>().toVariant(), string, lang);
        return;
    }

    switch (var.type())
    {
    case QVariant::Map:
    {
        string += '{';
        const QMap<QString, QVariant> map = var.toMap();
        for (const QString & k : map.keys())  // !!!*** refactor
        {
            string += QString("\"%1\":").arg(k);
            addQVariantToString(map.value(k), string, lang);
            string += ", ";
        }
        if (string.endsWith(", ")) string.chop(2);
        string += '}';
        break;
    }
    case QVariant::List:
        //string += '[';
        string += ( lang == EScriptLanguage::JavaScript ? '[' : '(' );
        for (const QVariant & v : var.toList())
        {
            addQVariantToString(v, string, lang);
            string += ", ";
        }
        if (string.endsWith(", ")) string.chop(2);
        //string += ']';
        string += ( lang == EScriptLanguage::JavaScript ? ']' : ')' );
        break;
    case QVariant::String:
        if (bAddQuotation) string += "\"";
        string += var.toString();
        if (bAddQuotation) string += "\"";
        break;
    default:
        // implicit convertion to string
        string += var.toString();
    }
#else
    if (var.metaType().name() == QStringLiteral("QJSValue") )
    {
        addQVariantToString(var.value<QJSValue>().toVariant(), string, lang);
        return;
    }

    switch (var.userType())
    {
    case QMetaType::QVariantMap :
    {
        string += '{';
        const QMap<QString, QVariant> map = var.toMap();
        for (const QString & k : map.keys())
        {
            string += QString("\"%1\":").arg(k);
            addQVariantToString(map.value(k), string, lang);
            string += ", ";
        }
        if (string.endsWith(", ")) string.chop(2);
        string += '}';
        break;
    }
    case QMetaType::QVariantList :
        //string += '[';
        string += ( lang == EScriptLanguage::JavaScript ? '[' : '(' );
        for (const QVariant & v : var.toList())
        {
            addQVariantToString(v, string, lang);
            string += ", ";
        }
        if (string.endsWith(", ")) string.chop(2);
        //string += ']';
        string += ( lang == EScriptLanguage::JavaScript ? ']' : ')' );
        break;
    case QMetaType::QString:
        if (bAddQuotation) string += "\"";
        string += var.toString();
        if (bAddQuotation) string += "\"";
        break;
    default:
        // implicit convertion to string
        string += var.toString();
    }
#endif
}

#include "ascripthub.h"
void AVirtualScriptManager::doBeforeScriptEval()
{
    bAborted = false;
    bFinished = false;

    AScriptHub::getInstance().doBeforeScriptEval();
}

void AVirtualScriptManager::doAfterScriptEval()
{
    // if uncommented, after script finished the json is updated even if config.upateConfig() was NOT called.
    // that will make it less intuitive to understand that without updateConfig the settings are not updateded during the execution seeing the correct result after the script eval ended
    //AScriptHub::getInstance().enableJsonAccess(); // hubs --> Json (if changes registered!)
}

#include "ascriptinterface.h"
QString AVirtualScriptManager::checkModuleOverrideAttempt(const QString & script, bool isPython)
{
    const std::vector<AScriptInterface*> & interfaces = getInterfaces();
    QSet<QString> names;
    for (const AScriptInterface * inter : interfaces)
        names.insert(inter->Name);

    const int size = script.size();

    auto isIdentifierChar = [](QChar c) { return c.isLetterOrNumber() || c == '_'; };
    auto isIdentifierStart = [](QChar c) { return c.isLetter() || c == '_'; };

    int i = 0;
    while (i < size)
    {
        const QChar c = script[i];

        // Skip single-line comments: // and #
        if (c == '#' ||
            (c == '/' && i + 1 < size && script[i + 1] == '/'))
        {
            while (i < size && script[i] != '\n')
                ++i;
            continue;
        }

        // Skip block comments: /* ... */
        if (c == '/' && i + 1 < size && script[i + 1] == '*')
        {
            i += 2;
            while (i + 1 < size &&
                   !(script[i] == '*' && script[i + 1] == '/'))
                ++i;

            i = std::min(i + 2, size);
            continue;
        }

        // Skip quoted strings
        /*
        if (c == '"' || c == '\'' || c == '`')
        {
            const QChar quote = c;
            ++i;

            while (i < size)
            {
                if (script[i] == '\\')
                {
                    i += 2;
                    continue;
                }

                if (script[i++] == quote)
                    break;
            }
            continue;
        }
        */
        if (c == '"' || c == '\'' || c == '`')
        {
            const QChar quote = c;

            // Python triple-quoted strings
            if (isPython && c != '`' &&
                i + 2 < size &&
                script[i + 1] == quote &&
                script[i + 2] == quote)
            {
                i += 3;

                while (i + 2 < size)
                {
                    if (script[i] == '\\')
                    {
                        i = std::min(i + 2, size);
                        continue;
                    }

                    if (script[i] == quote &&
                        script[i + 1] == quote &&
                        script[i + 2] == quote)
                    {
                        i += 3;
                        break;
                    }

                    ++i;
                }

                continue;
            }

            // Ordinary quoted strings
            ++i;

            while (i < size)
            {
                if (script[i] == '\\')
                {
                    i = std::min(i + 2, size);
                    continue;
                }

                if (script[i++] == quote)
                    break;
            }

            continue;
        }

        // Find identifiers
        if (!isIdentifierStart(c))
        {
            ++i;
            continue;
        }

        const int start = i++;

        while (i < size && isIdentifierChar(script[i]))
            ++i;

        const QString identifier = script.mid(start, i - start);

        if (!names.contains(identifier)) continue;

        // Ignore object properties: something.hist = ...
        int before = start - 1;
        while (before >= 0 && script[before].isSpace())
            --before;

        if (before >= 0 && script[before] == '.')
            continue;

        // Skip whitespace after identifier
        int after = i;
        while (after < size && script[after].isSpace())
            ++after;

        // Detect assignment, but not comparison (== or ===)
        if (after < size && script[after] == '=' &&
            (after + 1 == size || script[after + 1] != '='))
        {
            //return QString("Overriding script units is forbidden. Suspicious text found: %1").arg(identifier);
            const int lineStart = script.lastIndexOf('\n', start - 1) + 1;

            int lineEnd = script.indexOf('\n', start);
            if (lineEnd == -1) lineEnd = size;

            const QString line = script.mid(lineStart, lineEnd - lineStart).trimmed();
            const int lineNumber = script.left(start).count('\n') + 1;

            return QString("Overriding script units is forbidden!\n"
                           "Line %1: %2").arg(lineNumber).arg(line);
        }
    }

    return QString();
}
