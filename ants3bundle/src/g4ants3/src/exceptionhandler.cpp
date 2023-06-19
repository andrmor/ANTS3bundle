#include "exceptionhandler.h"
#include "SessionManager.hh"

#include <iostream>

G4bool ExceptionHandler::Notify(const char * originOfException, const char * exceptionCode, G4ExceptionSeverity severity, const char * description)
{
    std::cout << originOfException << std::endl;
    std::cout << exceptionCode << std::endl;
    std::cout << description << std::endl;

    if (severity == JustWarning) return false;

    SessionManager::getInstance().terminateSession(description);
    return true;
}
