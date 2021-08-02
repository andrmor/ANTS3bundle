#ifndef ALOGGER_H
#define ALOGGER_H

class QString;
class QFile;
class QTextStream;

class ALogger
{
public:
    static ALogger & getInstance();

    void open(const QString & fileName);

    static void log(const QString & text);

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
