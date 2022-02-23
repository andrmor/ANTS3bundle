#include "ascriptinterface.h"
#include "ajscripthub.h"

#include <QDebug>

const QString &AScriptInterface::getMethodHelp(const QString & method) const
{
    auto it = Help.find(method);
    if (it == Help.end()) return NoHelp;
    return it->second;
}

#include <QMetaObject>
#include <QMetaMethod>
QString AScriptInterface::help() const
{
    QString output;

    const int numMethods = this->metaObject()->methodCount();
    for (int iMethod = 0; iMethod < numMethods; iMethod++)
    {
        const QMetaMethod & m = this->metaObject()->method(iMethod);
        const bool bSlot   = (m.methodType() == QMetaMethod::Slot);
        const bool bPublic = (m.access() == QMetaMethod::Public);
        if (bSlot && bPublic)
        {
            const QString name(m.name());
            if (name == "deleteLater") continue;
            if (name == "help") continue;

            auto it = Help.find(name);
            QString helpText = (it == Help.end() ? "" : it->second);
            if (!helpText.isEmpty()) helpText.replace("\n", "\"\n   \"");
            output += QString("Help[\"%0\"] = \"%1\";\n").arg(name, helpText);
        }
    }

    return output;
}

void AScriptInterface::abort(const QString & message)
{
    qDebug() << "Abort triggered!" << message;
    AJScriptHub::getInstance().abort(message);
}
