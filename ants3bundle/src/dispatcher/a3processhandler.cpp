#include "a3processhandler.h"
#include "ajsontools.h"

#include <QProcess>
#include <QDebug>

A3ProcessHandler::A3ProcessHandler(const QString & program, const QStringList & args) :
    A3WorkerHandler(), Program(program), Args(args) {}

A3ProcessHandler::~A3ProcessHandler()
{
    //qDebug() << "DEBUG:PH->Destr for ProcessHandler";
    // normally, doExit has to be called before destructing this object!
    if (Process)
    {
        //qDebug() << "DEBUG:PH->Process was not soft-stopped, killing";
        killProcess();
    }
}

bool A3ProcessHandler::isRunning()
{
    if (!Process) return false;
    return (Process->state() != QProcess::NotRunning);
}

bool A3ProcessHandler::start()
{
    if (Process) return false;

    EventsDone = 0;

    Process = new QProcess();
    Process->setProcessChannelMode(QProcess::MergedChannels);

    //QObject::connect(G4antsProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), [&isRunning](){isRunning = false; qDebug() << "----FINISHED!-----";});
    //QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus){ /* ... */ });
    //QObject::connect(G4antsProcess,&QProcess::readyReadStandardOutput, this, &AParticleSourceSimulator::onG4ProcessSendMessage);

    QObject::connect(Process, &QProcess::readyReadStandardOutput, this, &A3ProcessHandler::onReadReady);

    qDebug() << Program << Args;
    Process->start(Program, Args);
    return Process->waitForStarted(10000);
}

void A3ProcessHandler::abort()
{
    Process->kill();
}

#include <QDebug>
void A3ProcessHandler::onReadReady()
{
    const QString in = Process->readAllStandardOutput();

    const QStringList input = in.split('\n', Qt::SkipEmptyParts);
    for (const QString & message : input)
    {
        if ( message.startsWith("$$>") && message.endsWith("<$$") )
        {
            QString str = message;
            str.remove("$$>");
            str.remove("<$$");
            EventsDone = str.toDouble();
            emit updateProgress(EventsDone);
            continue;
        }
        else if (message == "Type conversion already registered from type QSharedPointer<QNetworkSession> to type QObject*")
        {
            //bug fix for Qt
            continue;
        }
        emit receivedMessage(message);
    }

    //output += QString(in);
}

void A3ProcessHandler::sendMessage(QString txt)
{
    //qDebug() << "DEBUG:PH->sending message:"<<txt;
    Process->write(txt.toLatin1());
}

#include <QThread>
#include <QCoreApplication>
#include <QTimer>
void A3ProcessHandler::doExit()
{
    if (Process)
    {
        //qDebug() << "DEBUG:PH->wait for procress to finish...";
        for (int i=0; i < 20; i++)
        {
            qApp->processEvents();
            if (Process->state() == QProcess::NotRunning)
            {
                //qDebug() << "DEBUG:PH->done, signalling the process to deleteLater";
                Process->deleteLater();
                Process = nullptr;
                break;
            }
            QThread::msleep(50);
        }

        if (Process)
        {
            //qDebug() << "DEBUG:PH->soft exit failed, deleting process!";
            killProcess();
        }
    }

    deleteLater();
}

void A3ProcessHandler::killProcess()
{
    if (Process)
    {
        Process->closeWriteChannel();
        Process->closeReadChannel(QProcess::StandardOutput);
        Process->closeReadChannel(QProcess::StandardError);
        Process->kill();

        //delete Process;
        Process->deleteLater();
        Process = nullptr;
    }
}
