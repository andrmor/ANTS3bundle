#include "a3global.h"
#include "ajsontools.h"

#include <QDir>
#include <QDebug>

A3Global & A3Global::getInstance()
{
    static A3Global instance;
    return instance;
}

const A3Global &A3Global::getConstInstance()
{
    return getInstance();
}

A3Global::A3Global()
{

}

void A3Global::configureDirectories()
{
    ExecutableDir = QDir::currentPath();
    ExchangeDir = ExecutableDir + "/Exchange";
    QDir dir = QDir(ExchangeDir);
    if (!dir.exists()) QDir().mkdir(ExchangeDir);
    qDebug() << "Executable dir set to:"<< ExecutableDir;
    qDebug() << "Exchange dir set to:"<< ExchangeDir;
}

std::string A3Global::checkExchangeDir()
{
    if (ExchangeDir.isEmpty())       return "Exchange directory is not set!";
    if (!QDir(ExchangeDir).exists()) return "Exchange directory does not exist!";
    return "";
}

void A3Global::saveConfig()
{
}

void A3Global::loadConfig()
{
}
