#ifndef EXCEPTIONHANDLER_H
#define EXCEPTIONHANDLER_H

#include "G4ExceptionHandler.hh"

class ExceptionHandler : public G4ExceptionHandler
{
public:
    G4bool Notify(const char * originOfException, const char * exceptionCode, G4ExceptionSeverity severity, const char * description);
};

#endif // EXCEPTIONHANDLER_H
