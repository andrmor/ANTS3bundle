#include "avirtualscriptmanager.h"

#include <QtGlobal>
#include <QJSValue>

void AVirtualScriptManager::addQVariantToString(const QVariant & var, QString & string, EScriptLanguage lang)
{
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
        string += ( lang == AScriptLanguageEnum::JavaScript ? '[' : '(' );
        for (const QVariant & v : var.toList())
        {
            addQVariantToString(v, string, lang);
            string += ", ";
        }
        if (string.endsWith(", ")) string.chop(2);
        //string += ']';
        string += ( lang == AScriptLanguageEnum::JavaScript ? ']' : ')' );
        break;
    case QVariant::String:
        string += "\"";
        string += var.toString();
        string += "\"";
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
        string += "\"";
        string += var.toString();
        string += "\"";
        break;
    default:
        // implicit convertion to string
        string += var.toString();
    }
#endif
}
