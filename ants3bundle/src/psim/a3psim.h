#ifndef A3PSIM_H
#define A3PSIM_H

#include <QObject>
#include <QString>

class QFile;
class QTextStream;

class A3PSim : public QObject
{
    Q_OBJECT

public:
    A3PSim(const QString & fileName, const QString & dir, bool debug = false);
    ~A3PSim();

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

#endif // A3PSIM_H
