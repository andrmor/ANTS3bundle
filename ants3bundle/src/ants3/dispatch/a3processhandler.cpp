#include "a3processhandler.h"
#include "ajsontools.h"

#include <QProcess>
#include <QDebug>
//Use "DEBUG:" on start of QDebug!

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
    if (Process)
    {
        ErrorString = "Already started";
        return false;
    }
    EventsDone = 0;

    Process = new QProcess();
    Process->setProcessChannelMode(QProcess::MergedChannels);

    //QObject::connect(G4antsProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), [&isRunning](){isRunning = false; qDebug() << "----FINISHED!-----";});//this, &MainWindow::on_cameraControlExit);
    //QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus){ /* ... */ });
    //QObject::connect(G4antsProcess,&QProcess::readyReadStandardOutput, this, &AParticleSourceSimulator::onG4ProcessSendMessage);

    QObject::connect(Process, &QProcess::readyReadStandardOutput, this, &A3ProcessHandler::onReadReady);

    Process->start(Program, Args);
    return Process->waitForStarted(1000);
}

void A3ProcessHandler::abort()
{

}

#include <QDebug> // make sure to use "DEBUG:" on start of the message!!!
void A3ProcessHandler::onReadReady()
{
    QString in = Process->readAllStandardOutput();
    //qDebug() << "DEBUG:PH->received message:\n" << in;

    const QStringList input = in.split('\n', Qt::SkipEmptyParts);
    for (const QString & message : input)
    {
        //qDebug() << "DEBUG:PH->Processing message:" << in;
        if (message.startsWith("DEBUG:"))
        {
            qDebug() << message;
            continue;
        }
        else if ( message.startsWith("$$>") && message.endsWith("<$$") )
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

// ----

QString A3ProcessHandler::makeErrorMessage(const QString &ErrorDescription)
{
    QJsonObject js;
    js["Error"] = ErrorDescription;
    return jstools::jsonToString(js);
}

bool A3ProcessHandler::isErrorMessage(const QString &message, QString &ErrorDescription)
{
    QJsonObject js = jstools::strToJson(message);
    return jstools::parseJson(js, "Error", ErrorDescription);
}

QString A3ProcessHandler::makeFinishMessage()
{
    QJsonObject js;
    js["Status"] = "Finish";
    return jstools::jsonToString(js);
}

bool A3ProcessHandler::isFinishMessage(const QString &message)
{
    QJsonObject js = jstools::strToJson(message);
    QString str;
    jstools::parseJson(js, "Status", str);
    return (str == "Finish");
}
