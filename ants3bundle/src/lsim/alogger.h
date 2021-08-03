#ifndef ALOGGER_H
#define ALOGGER_H

class QString;
class QFile;
class QTextStream;

#include <QTextStream>

#define LOG ALogger::log()

class ALogger
{
public:
    static ALogger & getInstance();

    void open(const QString & fileName);

    static QTextStream & log();

private:
    ALogger(){}
    ~ALogger();

    ALogger(const ALogger&)            = delete;
    ALogger(ALogger&&)                 = delete;
    ALogger& operator=(const ALogger&) = delete;
    ALogger& operator=(ALogger&&)      = delete;

private:
    QFile       * Ofile = nullptr;
    QTextStream * Log   = nullptr;
};

#endif // ALOGGER_H
