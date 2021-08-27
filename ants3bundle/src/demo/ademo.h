#ifndef ADEMO_H
#define ADEMO_H

#include <QObject>
#include <QString>

class QFile;
class QTextStream;

class ADemo : public QObject
{
    Q_OBJECT

public:
    ADemo(const QString & fileName, const QString & dir, bool debug = false);
    ~ADemo();

public slots:
    void start();

private slots:
    void onProgressTimer();

protected:
    QString       FileName;
    QString       FileDir;
    bool          Debug = true;

    QFile       * Ofile = nullptr;
    QTextStream * Log   = nullptr;

    int           EventsProcessed = 0;
};

#endif // ADEMO_H
