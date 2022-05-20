#include "exceptionhandler.h"
#include "SessionManager.hh"

#include <QDebug>

G4bool ExceptionHandler::Notify(const char * originOfException, const char * exceptionCode, G4ExceptionSeverity severity, const char * description)
{
    qDebug() << originOfException;
    qDebug() << exceptionCode;
    qDebug() << description;

    SessionManager::getInstance().terminateSession(description);
    return true;
}
