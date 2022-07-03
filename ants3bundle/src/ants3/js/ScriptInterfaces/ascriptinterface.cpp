#include "ascriptinterface.h"
#include "ascripthub.h"

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
    QString output = "\n";

    const int numMethods = this->metaObject()->methodCount();
    QString lastName;
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
            if (name == lastName) continue;
            lastName = name;

            auto it = Help.find(name);
            QString helpText = (it == Help.end() ? "" : it->second);
            if (!helpText.isEmpty()) helpText.replace("\n", "\\n\"\n   \"");
            output += QString("Help[\"%0\"] = \"%1\";\n").arg(name, helpText);
        }
    }

    return output;
}

void AScriptInterface::abort(const QString & message)
{
    qDebug() << "Abort triggered!" << message;
    AScriptHub::getInstance().abort(message);
}
