#include "ademo_si.h"
#include "ademomanager.h"

#include <QDebug>

ADemo_SI::ADemo_SI() :
    AScriptInterface(){}

bool ADemo_SI::beforeRun()
{
    //qDebug() << "Demo: before start evaluation";
    return true;
}

bool ADemo_SI::afterRun()
{
    //qDebug() << "Demo: after end of evaluation";
    return true;
}

void ADemo_SI::abortRun()
{
    ADemoManager::getInstance().abortedByScript(); // !!!*** more universal way in ADemomanager: like ask Dispatcher hub about abort status?
}

bool ADemo_SI::run(int numLocalProc)
{
    return ADemoManager::getInstance().run(numLocalProc);
}
