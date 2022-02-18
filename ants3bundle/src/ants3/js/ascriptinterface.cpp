#include "ascriptinterface.h"
#include "ajscripthub.h"

#include <QDebug>

const QString &AScriptInterface::getMethodHelp(const QString & method) const
{
    auto it = Help.find(method);
    if (it == Help.end()) return NoHelp;
    return it->second;
}

void AScriptInterface::abort(const QString & message)
{
    qDebug() << "Abort triggered!" << message;
    AJScriptHub::getInstance().abort(message);
}
