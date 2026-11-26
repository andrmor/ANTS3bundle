#include "ajscriptworker.h"
#include "ascriptinterface.h"

#include <QJSEngine>
#include <QDebug>

AJScriptWorker::~AJScriptWorker()
{
    //qDebug() << "Destr for JavaScriptWorker";
    delete Engine;

    // do not delete script interfaces, it is automatic!
}

static void evaluateBuiltIn(QJSEngine * engine, const QString & code, const char * what)
{
    const QJSValue res = engine->evaluate(code);
    if (res.isError())
        qWarning() << "Failed to define built-in JS function" << what << ":"
                   << res.property("message").toString() << "line" << res.property("lineNumber").toInt();
}

void AJScriptWorker::onRegisterInterface(AScriptInterface * interface, QString name)
{
    interface->Name = name;
    QJSValue sv = Engine->newQObject(interface);
    Engine->globalObject().setProperty(name, sv);
    Interfaces.push_back(interface);

    if (name == "core")
    {
        QString str = "function print()\n"
                      "{\n"
                      "   var str = ''\n"
                      "   for (var ii = 0; ii < arguments.length; ii++)\n"
                      "   {\n"
                      "      if (ii > 0) str += ' '\n"
                      "      str += core.toStr(arguments[ii])\n"
                      "   }\n"
                      "   core.print(str)\n"
                      "}";
        //Engine->evaluate(str);
        evaluateBuiltIn(Engine, str, "print");

        str = "function printHtml()\n"
              "{\n"
              "   var str = ''\n"
              "   for (var ii = 0; ii < arguments.length; ii++)\n"
              "   {\n"
              "      if (ii > 0) str += ' '\n"
              "      str += core.toStr(arguments[ii])\n"
              "   }\n"
              "   core.printHtml(str)\n"
              "}";
        //Engine->evaluate(str);
        evaluateBuiltIn(Engine, str, "printHtml");

        str = "var includeTab = (function(evalGlobal) {\n"
              "    var f = function(tabName, bookName) {\n"
              "        if (bookName === undefined) bookName = '';\n"
              "        if (f.depth >= 16) { core.abort('includeTab: nesting too deep (circular include?)'); return; }\n"
              "        var code = core.getTabScript(tabName, bookName);   // aborts by itself if not found\n"
              "        f.depth++;\n"
              "        try       { evalGlobal(code); }\n"
              "        catch (e) {\n"
              "            if (e instanceof Error) {\n"
              "                e.message = \"In included tab '\" + tabName + \"'\" +\n"
              "                            (e.lineNumber > 0 ? ', line ' + e.lineNumber : '') + ': ' + e.message;\n"
              "                e.lineNumber = -1;\n"
              "            }\n"
              "            throw e;\n"
              "        }\n"
              "        finally   { f.depth--; }\n"
              "    };\n"
              "    f.depth = 0;\n"
              "    return f;\n"
              "})(\n"
              "    // The eval helper must be created at the top level of the script, not inside the wrapper:\n"
              "    // created inside a function, the declarations of the evaluated code do not become global in V4\n"
              "    function(code) { (0, eval)(code); }\n"
              ");\n";
        evaluateBuiltIn(Engine, str, "includeTab");
        //Engine->evaluate(str);
        evaluateBuiltIn(Engine, str, "includeTab");
    }
}

void AJScriptWorker::abort()
{
    for (AScriptInterface * inter : Interfaces) inter->abortRun();

    Engine->setInterrupted(true);
}

bool AJScriptWorker::isError() const
{
    return Result.isError();
}

QString AJScriptWorker::getErrorDescription() const
{
    if (bBusy) return "";

    bool bError = Result.isError();
    if (!bError) return "";

    return Result.property("message").toString();
}

int AJScriptWorker::getErrorLineNumber()
{
    if (bBusy) return -1;

    bool bError = Result.isError();
    if (!bError) return -1;

    return Result.property("lineNumber").toInt();
}

bool AJScriptWorker::isCallable(const QString & name) const
{
    QJSValue prop = Engine->globalObject().property(name);
    return prop.isCallable();
}

double AJScriptWorker::runMinimizationFunction(const QString & name, const double * p, int numParameters)
{
    QJSValue prop = Engine->globalObject().property(name);

    QJSValueList input;
    for (int i=0; i<numParameters; i++) input << p[i];

    return prop.call(input).toNumber();
}

bool AJScriptWorker::callFunctionNoArguments(const QString & functionName)
{
    QJSValue prop = Engine->globalObject().property(functionName);
    return !prop.call().isError();
}

void AJScriptWorker::initialize()
{
    Engine = new QJSEngine();
}

void AJScriptWorker::evaluate(const QString & script)
{
    if (bBusy) return;

    for (AScriptInterface * inter : Interfaces) inter->beforeRun(); // !!!*** error control!

    bBusy = true;
    Engine->setInterrupted(false);

    QJSValue inc = Engine->globalObject().property("includeTab"); // on abort does not reset the counter!
    if (inc.isCallable()) inc.setProperty("depth", 0);

    Result = Engine->evaluate(script);

    //qDebug() << "Script eval result:\n" << Result.toString();

    for (AScriptInterface * inter : Interfaces) inter->afterRun(); // !!!*** error control!

    bool ok = !Result.isError();

    bBusy = false;
    emit evalFinished(ok);
}

void AJScriptWorker::exit()
{
    //qDebug() << "Exit triggered for JS worker!";

    Engine->setInterrupted(true);
    do {} while (!Engine->isInterrupted());

    emit stopped();
}

void AJScriptWorker::onRequestGarbageCollection()
{
    if (bBusy) return;

    bBusy = true;
    Engine->collectGarbage();
    bBusy = false;
}
