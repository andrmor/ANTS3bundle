#include "a3processhandler.h"
#include "ajsontools.h"

#include <QProcess>
//careful with the QDEBUG! Use "QDEBUG:" on start or it will be interpreted as sim finished

A3ProcessHandler::A3ProcessHandler(const QString & program, const QStringList & args) :
    A3WorkerHandler(), Program(program), Args(args) {}

A3ProcessHandler::~A3ProcessHandler()
{
    qDebug() << "Destr for process handler";
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
    QObject::connect(Process, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadReady()));

    Process->start(Program, Args);
    return Process->waitForStarted(1000);
}

void A3ProcessHandler::abort()
{

}

#include <QDebug> // make sure to use "DEBUG:" on start of th emessage!!!
void A3ProcessHandler::onReadReady()
{
    QString in = Process->readAllStandardOutput();
    //if (bVerbose) qDebug() << "...handler received message:\n" << in;

    // TODO: split using '\n', do processing of all messages one by one


    if (in.startsWith("DEBUG:"))
    {
        qDebug() << in;
        return;
    }
    else if (in == "Type conversion already registered from type QSharedPointer<QNetworkSession> to type QObject*\n")
    {
        //bug fix for Qt
        return;
    }

    if (in.startsWith("$$>") && in.endsWith("<$$\n"))
    {
        in.remove("$$>");
        in.remove("<$$\n");
        EventsDone = in.toDouble();
        emit updateProgress(EventsDone);
        return;
    }
    emit receivedMessage(in);
    //output += QString(in);
}

void A3ProcessHandler::sendMessage(QString txt)
{
    qDebug() << "DEBUG:PH->sending message:"<<txt;
    Process->write(txt.toLatin1());
}

#include <QThread>
#include <QCoreApplication>
#include <QTimer>
void A3ProcessHandler::doExit()
{
    qDebug() << "DEBUG:PH->soft exit processing...";

    if (Process)
    {
       // qDebug() << "DEBUG:sending exit message to the process";
       // Process->write("$$EXIT\n");

        for (int i=0; i < 10; i++)
        {
            qApp->processEvents();
            if (Process->state() == QProcess::NotRunning) break;
            QThread::msleep(50);
        }

        if (Process->state() != QProcess::NotRunning)
        {
            Process->closeWriteChannel();
            Process->closeReadChannel(QProcess::StandardOutput);
            Process->closeReadChannel(QProcess::StandardError);
            Process->kill();
        }
        delete Process; Process = nullptr;
    }

    deleteLater();
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
