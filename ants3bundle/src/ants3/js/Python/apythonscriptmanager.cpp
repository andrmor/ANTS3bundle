#include "apythoninterface.h" // MUST be first

#include "apythonscriptmanager.h"
#include "ascriptinterface.h"

APythonScriptManager::APythonScriptManager(QObject *parent)
    : AVirtualScriptManager{parent}
{
    PyInterface = new APythonInterface();
}

void APythonScriptManager::registerInterface(AScriptInterface * interface, QString name)
{
    PyInterface->registerUnit(interface, name);
    interface->Name = name;
    Interfaces.push_back(interface);
}

void APythonScriptManager::finalizeInit()
{
    PyInterface->initialize();
}

const std::vector<AScriptInterface *> & APythonScriptManager::getInterfaces() const
{
    return Interfaces;
}

bool APythonScriptManager::evaluate(const QString & script)
{
    PyInterface->evalScript(script);
    return true;
}

QVariant APythonScriptManager::getResult() {return "";}

bool APythonScriptManager::isError() const
{
    return false;
}

int APythonScriptManager::getErrorLineNumber()
{
    return -1;
}
