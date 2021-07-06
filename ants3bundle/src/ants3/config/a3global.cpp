#include "a3global.h"
#include "ajsontools.h"

#include <QDir>
#include <QDebug>

A3Global & A3Global::getInstance()
{
    static A3Global instance;
    return instance;
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

void A3Global::saveConfig()
{
}

void A3Global::loadConfig()
{
}
