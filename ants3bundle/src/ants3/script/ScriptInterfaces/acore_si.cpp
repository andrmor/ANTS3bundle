#include "acore_si.h"
#include "ascripthub.h"
#include "ascripthub.h"
#include "avirtualscriptmanager.h"
#include "ajscriptmanager.h"
#ifdef ANTS3_PYTHON
    #include "apythonscriptmanager.h"
#endif
#include "afiletools.h"
#include "ascriptmessenger.h"

#ifdef _ALLOW_LAUNCH_EXTERNAL_PROCESS_
#include <QProcess>
#endif

#include <QDateTime>
#include <QFileInfo>
#include <QFile>
#include <QDebug>
//#include <QtWidgets/QApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>
#include <QRegularExpression>

#include <iostream>
#include <fstream>
#include <ostream>
#include <ios>

ACore_SI::ACore_SI() : AScriptInterface()
{
    Description = "General-purpose opeartions: abort script, basic text output and file save/load";

    Help["abort"] = "Abort script execution and show given text in the output";

    Help["print"] = "Print up to 10 arguments in the output area. Arrays and objects are convetred to strings, a space is added between the arguments";
    Help["printHtml"] = "Print the html-formatted text in the output area. Can be used, for example, to set text color or use bold font";
    Help["clearOutput"] = "Clear the output area";

    Help["sleep"] = "Pause the script thread for the given number of milliseconds";
    Help["getTimeMarkMilliseconds"] = "Current time in milliseconds since the start, in UTC, of the year 1970";
    Help["getDateTimeStamp"] = "Return text with the current date and time, in the format: hours:minutes:seconds Day Month Year";

    Help["createDir"] = "Create a new directory. Abort if not successful";
    Help["createFile"] = "Create an empty file. Abort if not successful or file already exists";
    Help["deleteFile"] = "Delete the file. Abort if not successful";
    Help["isFileExist"] = "Return true if the file exists, false otherwise";

    Help["getDirectories"] = "Return list of directories matching the provided pattern for the given path";
    Help["setNewFileFinder"] = "Configurer for getNewFiles() method. dir is the search directory, fileNamePattern: e.g. *.dat for all dat files. Return all filenames mathcing the pattern";
    Help["getNewFiles"] = "Get array of names of the new files appeared in the directory configured with setNewFileFinder() method";

    Help["setCurrentDir"] = "Set default output directory. Abort if not successful";
    Help["getCurrentDir"] = "Get default output directory";

    Help["saveText"] = "Save text to the file (create new if does not exist; override if not empty). Abort if not successful";
    Help["appendText"] = "Apend text to the existing file. Abort if not successful";
    Help["loadText"] = {{1,"Load text from the file. Abort if not successful"},
                        {2,"Load a given number of lines form the beginning of the file. Abort if not successful"}};

    Help["saveArray"] = {{2,"Save 1D or 2D numeric array to the file. If the file exists, it will be reset"},
                         {3,"Save 1D or 2D numeric array to the file. If 'append' argument is true, the data are added to the end of the file. "
                            "In this case if the file does not exists, abort is triggered"}};
    Help["loadNumericArray"] = "Load an array of numerics (or an array of numeric arrays). One row corresponds to one line in the file. Separators can be space, tab, comma or colon. Lines starting with # or // are ignored.";

    {
        AScriptHelpEntry se;
        QString txt = "Load array of mixed-type arrays from file, with inner array read according to the array with format options:\n"
                      "'d'-double, 'i'-integer, 's'-string, ''-skip field: e.g. loadArray('fn.txt', ['s', 'd', 'd']) for sub-arrays of 1 string and 2 doubles";
        se.addRecord(2, txt);
        txt += ". The last two arguments specify the limiting 'from' and 'to' line numbers in the file";
        se.addRecord(4, txt);
        Help["loadArray"] = se;
    }

    {
        AScriptHelpEntry se;
        QString txt = "Save array of mixed-type arrays (binary data), with second argument providing the format array. "
                      "This available options are 's', 'i', 'd', 'f' or 'c' markers (zero-terminating string, int, double, float and char, respectively), "
                      "e.g. ['s', 'd', 'd'] for arrays of string and two doubles";
        se.addRecord(3, txt);
        txt += ". if 'apend' is true, data are appended to the end of the exisiting file (abort if file does not exist)";
        se.addRecord(4, txt);
        Help["saveBinaryArray"] = se;
    }

    Help["loadArrayBinary"] = "Load array of mixed-type arrays from a binary file. The second argument defines the format of sub-arrays. "
          "This argument should be an array of type indicators including 's', 'i', 'd', 'f' or 'c' for zero-terminating string, int, double, float and char, respectively. "
          "e.g. ['s', 'd', 'd'] configures reading of sub-arryas of string and two doubles";

    {
        AScriptHelpEntry se;
        QString txt = "Save 3D array to text file. The inner-most array can be of a mixed type. "
                      "The top-level records are separated by inserting in the file the string given by the topLevelSeparator parameter";
        se.addRecord(3, txt);
        txt += ". Optional topLevelLabels argument is an array (or array of mixed-type arrays) with data to be added in the same 'event header' line after topLevelSeparator. "
               "Its size should be equal to the size of the main array";
        se.addRecord(4, txt);
        txt += ". if 'apend' is true, data are appended to the end of the exisiting file (abort if file does not exist)";
        se.addRecord(5, txt);
        Help["save3DArray"] = se;
    }

    {
        AScriptHelpEntry se;
        QString txt = "Load 3D array from text file. The top level records in the file are separated by the string defined by 'topSeparator' argument. "
                      "The inner-most array can be of mixed type, format is defined by an array given by the 'format' argument. The available options:\n"
                      "'d'-double, 'i'-integer, 's'-string, ''-skip field: e.g. ['s', 'd', 'd']) configure sub-arrays of 1 string and 2 doubles";
        se.addRecord(3, txt);
        txt += ".\nIf 'skipEmpty' is true, empty lines are ignored";
        se.addRecord(4, txt);
        txt += ".\nIf 'allowIncomplete' is true, read will not abort if any sub-array contains less elements than defined by the 'format' array";
        se.addRecord(5, txt);
        txt += ".\n'recordsFrom' defines the top level index from which to start";
        se.addRecord(6, txt);
        txt += ". 'recordsUntil' defines the top level index until which to read (exclusive)";
        se.addRecord(7, txt);
        Help["load3DArray"] = se;
    }

    // !!!*** todo
    //Help["save3DBinaryArray"] = ;
    //Help["load3DBinaryArray"] = ;

    Help["saveObject"] = "Save object (dictionaly in Python) to file using json format";
    Help["loadObject"] = "Load object (dictionaly in Python) from a file with json format";

    Help["str"] = "Converts numeric value to string using the given precision (number of digits after the decimal separator)";
    Help["toStr"] = "Converts argument to the string and returns it";
    Help["arraySum"] = "For 1D array, returns sum of all elements.\nFor 2D arrays, returns sum of the last column";
    Help["getExamplesDir"] = "Get ANTS3 directory with script/config examples";
    Help["processEvents"] = "Put this method sparsely inside computationaly-heavy code to improve reaction to user abort. Note that 'print' and 'reportProgress' have the same effect";
    Help["reportProgress"] = "Show progress bar on the script window. The argument is progress value in percent";
    Help["requestGuiUpdate"] = "Update all GUI windows during script execution";

    Help["startExternalProcess"] = "Start external process (command and arguments(s)).\n"
                                   "ANTS3 should be compiled with the following line uncommented in ants3.pro: DEFINES += _ALLOW_LAUNCH_EXTERNAL_PROCESS_";
    Help["startExternalProcessAndWait"] = "Start external process (command and arguments(s)) and wait until it is finished or waiting time exceed the provided value. "
                                          "Return error string if there were errors\n"
                                          "ANTS3 should be compiled with the following line uncommented in ants3.pro: DEFINES += _ALLOW_LAUNCH_EXTERNAL_PROCESS_";
}

bool ACore_SI::beforeRun()
{
    delete Messenger; Messenger = new AScriptMessenger(Lang);
    return true;
}

/*
#include <chrono>
void ACore_SI::test()
{
    size_t a = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    qDebug() << a;
}
*/

/*
void ACore_SI::fun0()
{
    qDebug() << "noooooo argsssss";
}

void ACore_SI::fun1(int i)
{
    qDebug() << "oooooonnnneeee"  << i;
}

void ACore_SI::fun2(int i, double d)
{
    qDebug() << "ttttwwwwooooo"  << i << d;
}
*/

/*
ACore_SI::ACore_SI(const ACore_SI &other) :
    AScriptInterface(other)
{
    ScriptManager = 0; //to be set after copy
}
*/

void ACore_SI::abort(QString message)
{
    //qDebug() << ">Core module: abort triggered!";
    //AJScriptHub::getInstance().abort(message);

    //MessengerTxt->flush();
    //MessengerHtml->flush();
    Messenger->flush();

    AScriptInterface::abort(message);
}

/*
QVariant ACore_SI::test(QVariant in)
{
    qDebug() << in;
    return in;
}
*/

/*
#include "vformula.h"
double ACore_SI::testVFormula(QString formula, QVariantList varNames, QVariantList varValues)
{
    VFormula p1;

    std::vector<std::string> names;
    for (int i = 0; i < varNames.size(); i++) names.push_back(std::string(varNames[i].toString().toLatin1()));
    p1.setVariableNames(names);

    bool ok = p1.parse(formula.toLatin1().data());
    if (!ok)
    {
        abort("VFormula parse error!\n" + QString(p1.ErrorString.data()));
        return 0;
    }

    VFormula p(p1);

    std::cout << "\n----------Map------------\n";
    p.printCVMap();
    std::cout << "\n---------Program---------\n";
    p.printPrg();

    ok = p.validate();
    if (!ok)
    {
        abort("VFormula validation error!\n" + QString(p.ErrorString.data()));
        return 0;
    }

    std::vector<double> values;
    for (int i = 0; i < varValues.size(); i++) values.push_back(varValues[i].toDouble());

    double res = p.eval(values);

    if (!p.ErrorString.empty())
    {
        abort("VFormula eval error!\n" + QString(p.ErrorString.data()));
        return 0;
    }

// timed run
    //std::cout << "Timed run\n";
    //auto start = std::chrono::high_resolution_clock::now();

//  Code to be timed
    //double sum =  0.;
    //for (int i=0; i<10000000; i++) {
    //    sum += p.Eval(6);
    //}
    //std::cout << sum << std::endl;

    //auto end = std::chrono::high_resolution_clock::now();
    //auto diff = end - start;
    //std::cout << std::chrono::duration <double, std::nano> (diff).count()/10000000 << " ns/eval" << std::endl;
    return res;
}
*/

double ACore_SI::arraySum(QVariantList array)
{
    double sum = 0;
    bool ok;

    for (int i = 0; i < array.size(); i++)
    {
        double val = array[i].toDouble(&ok);
        if (ok)
        {
            sum += val;
            continue;
        }

        QVariantList el = array[i].toList();
        if (el.size() > 0)
            sum += el.back().toDouble();
    }

    return sum;
}

/*
#include "amatcomposition.h"
QString ACore_SI::testComposition(QString comp)
{
    AMatComposition mc;
    bool ok = mc.setCompositionString(comp);
    if (!ok)
    {
        abort(mc.ErrorString);
        return "";
    }

    //TGeoMaterial * mat = mc.constructGeoMaterial("MatNameTest",1.1, 321.0);
    //qDebug() << "\nGeoMat to composition string:\n" << AMatComposition::geoMatToCompositionString(mat);

    return mc.printComposition();
}
*/

/*
int ACore_SI::fun(int i)
{
    qDebug() << "Single arg!";
    return i;
}
*/

/*
int ACore_SI::fun(int i, int j, int k)
{
    qDebug() << "Three args!";
    return i + j + k;
}
*/

#include <QElapsedTimer>
void ACore_SI::sleep(int ms)
{
    if (ms == 0) return;

    AScriptHub & SH = AScriptHub::getInstance();
    AVirtualScriptManager * SM = nullptr;
    if (Lang == EScriptLanguage::JavaScript) SM = &SH.getJScriptManager();
#ifdef ANTS3_PYTHON
    if (Lang == EScriptLanguage::Python)     SM = &SH.getPythonManager();
#endif

    QElapsedTimer t;
    t.start();
    do
    {
        QThread::usleep(100);
        //qApp->processEvents();
        SH.processEvents(Lang);
        if (!SM->isRunning()) break;
    }
    while (t.elapsed() < ms);
}

double ACore_SI::getTimeMarkMilliseconds()
{
    return QDateTime::currentMSecsSinceEpoch();
}

void ACore_SI::print(QVariant message)
{
    QString s;
    AVirtualScriptManager::addQVariantToString(message, s, Lang);
    //AScriptHub::getInstance().outputText(s, Lang);
    //MessengerTxt->output(s);
    Messenger->output(s, false);
}

void ACore_SI::print(QVariant m1, QVariant m2, QVariant m3, QVariant m4, QVariant m5, QVariant m6, QVariant m7, QVariant m8, QVariant m9, QVariant m10)
{
    QString s;
    AVirtualScriptManager::addQVariantToString(m1,  s, Lang); s += " ";
    AVirtualScriptManager::addQVariantToString(m2,  s, Lang); s += " ";
    AVirtualScriptManager::addQVariantToString(m3,  s, Lang); s += " ";
    AVirtualScriptManager::addQVariantToString(m4,  s, Lang); s += " ";
    AVirtualScriptManager::addQVariantToString(m5,  s, Lang); s += " ";
    AVirtualScriptManager::addQVariantToString(m6,  s, Lang); s += " ";
    AVirtualScriptManager::addQVariantToString(m7,  s, Lang); s += " ";
    AVirtualScriptManager::addQVariantToString(m8,  s, Lang); s += " ";
    AVirtualScriptManager::addQVariantToString(m9,  s, Lang); s += " ";
    AVirtualScriptManager::addQVariantToString(m10, s, Lang);
    //AScriptHub::getInstance().outputText(s, Lang);
    //MessengerTxt->output(s);
    Messenger->output(s, false);
}

void ACore_SI::printHtml(QString text)
{
    //QString s;
    //AVirtualScriptManager::addQVariantToString(text, s, Lang);
    //AScriptHub::getInstance().outputHtml(s, Lang);
    //AScriptHub::getInstance().outputHtml(text, Lang);

    //MessengerHtml->output(text);
    Messenger->output(text, true);
}

void ACore_SI::clearOutput()
{
    //MessengerTxt->clear();
    //MessengerHtml->clear();
    Messenger->clear();
    AScriptHub::getInstance().clearOutput(Lang);
}

QString ACore_SI::getDateTimeStamp()
{
    return QDateTime::currentDateTime().toString("HH:mm:ss dd MMM yyyy");
}

void ACore_SI::saveText(QString text, QString fileName)
{
    QFile file(fileName);
    if ( !file.open(QIODevice::WriteOnly) )
    {
        abort("Cannot open file: " + fileName);
        return;
    }

    QTextStream outstream(&file);
    outstream << text;
}

void ACore_SI::appendText(QString text, QString fileName)
{
    if (!QFileInfo::exists(fileName))
    {
        abort("File does not exist: " + fileName);
        return;
    }

    QFile file(fileName);
    if ( !file.open(QIODevice::Append) )
    {
        abort("Cannot open file for append: " + fileName);
        return;
    }

    QTextStream outstream(&file);
    outstream << text;
}

QString ACore_SI::loadText(QString fileName)
{
    if (!QFileInfo::exists(fileName))
    {
        abort("File does not exist: " + fileName);
        return "";
    }

    QString str;
    bool bOK = ftools::loadTextFromFile(str, fileName);
    if (!bOK) abort("Error reading file: " + fileName);
    return str;
}

QString ACore_SI::loadText(QString fileName, int numLines)
{
    QString txt;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QFile::Text))
    {
        abort("cannot open file " + fileName);
        return "";
    }

    QTextStream in(&file);
    while (numLines > 0)
    {
        txt += in.readLine() + "\n";
        numLines--;
    }

    file.close();
    return txt;
}

void ACore_SI::saveArray(QVariantList array, QString fileName, bool append)
{
    if (append && !QFileInfo::exists(fileName))
    {
        abort("File does not exist: " + fileName);
        return;
    }
    QFile file(fileName);
    if ( !file.open(append ? QIODevice::Append : QIODevice::WriteOnly) )
    {
        abort("Cannot open file: " + fileName);
        return;
    }

    QTextStream s(&file);
    //if (precision != 6) s.setRealNumberPrecision(precision);

    for (int i = 0; i < array.size(); i++)
    {
        const QVariant & var = array[i];
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if (var.type() == QVariant::List)
#else
        if (var.userType() == QMetaType::QVariantList)
#endif
        {
            QStringList sl = var.toStringList();
            s << sl.join(' ');
        }
        else s << var.toString();
        s << '\n';
    }
}

void ACore_SI::saveBinaryArray(QVariantList array, QString fileName, QVariantList format, bool append)
{
    std::vector<EArrayFormat> FormatSelector;
    bool bFormatOK = readFormat(format, FormatSelector, true);
    if (!bFormatOK)
    {
        abort("Parameter 'format' should be an array of 's', 'i', 'd', 'f', 'c' or '' markers (string, int, double, float, char or skip, respectively)");
        return;
    }

    if (append)
    {
        if ( !QFileInfo::exists(fileName) )
        {
            abort("File does not exist: " + fileName);
            return;
        }
    }

    std::ofstream outStream(fileName.toLatin1().data(),
                            append ? std::ios_base::app | std::ios::binary
                                   : std::ios::out | std::ios::binary );
    if (!outStream.is_open())
    {
        abort("Cannot open file for writing: " + fileName);
        return;
    }

    QString err;
    if (FormatSelector.size() == 1)
    {
        //array
        if ((int)FormatSelector.size() > array.size())
        {
            abort("Format array is longer than the data array element!");
            return;
        }

        for (int iar = 0; iar < array.size(); iar++)
        {
            QVariantList vl;
            vl << array[iar];
            bool ok = writeFormattedBinaryLine(outStream, FormatSelector, vl, err);
            if (!ok)
            {
                abort(err);
                return;
            }
        }
    }
    else
    {
        //array of arrays
        for (int iar = 0; iar < array.size(); iar++)
        {
            QVariantList vl = array[iar].toList();
            if ((int)FormatSelector.size() > vl.size())
            {
                abort("Format array is longer than the data array!");
                return;
            }
            bool ok = writeFormattedBinaryLine(outStream, FormatSelector, vl, err);
            if (!ok)
            {
                abort(err);
                return;
            }
        }
    }
    outStream.close();
}

void ACore_SI::saveObject(QVariantMap Object, QString FileName)
{
    QJsonObject json = QJsonObject::fromVariantMap(Object);
    QJsonDocument saveDoc(json);

    QFile saveFile(FileName);
    if (saveFile.open(QIODevice::WriteOnly))
    {
        saveFile.write(saveDoc.toJson());
        saveFile.close();
    }
    else abort("Cannot open file for writing: " + FileName);
}

QVariantList ACore_SI::loadNumericArray(QString fileName)
{
    QVariantList vl;

    if (!QFileInfo::exists(fileName))
    {
        abort("File does not exist: " + fileName);
        return vl;
    }

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QFile::Text))
    {
        abort("Cannot open file: "+fileName);
        return vl;
    }

    QTextStream in(&file);
    const QRegularExpression rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'

    while (!in.atEnd())
    {
        const QString line = in.readLine();
        if (line.startsWith('#') || line.startsWith("//")) continue; // it is a comment

        const QStringList fields = line.split(rx, Qt::SkipEmptyParts);
        if (fields.isEmpty()) continue;

        bool bOK;
        double first = fields.first().toDouble(&bOK);
        if (!bOK)
        {
            abort("Bad format of the file: numeric values are expected");
            return vl;
        }

        if (fields.size() == 1)
            vl.append(first);
        else
        {
            QVariantList el;
            el << first;
            for (int i = 1; i < fields.size(); i++)
            {
                el << fields[i].toDouble(&bOK);
                if (!bOK)
                {
                    abort("Bad format of the file: numeric values are expected");
                    return vl;
                }
            }
            vl.push_back(el);
        }
    }

    file.close();
    return vl;
}

bool ACore_SI::readFormat(const QVariantList & format, std::vector<EArrayFormat> & FormatSelector, bool AllowSkip, bool AllowEmptyFormatArray)
{
    const int numEl = format.size();
    if (numEl == 0 && !AllowEmptyFormatArray) return false;

    for (int i=0; i<format.size(); i++)
    {
        const QString f = format.at(i).toString();
        EArrayFormat   Option;
        if      (f == "s") Option = StringFormat;
        else if (f == "i") Option = IntFormat;
        else if (f == "ui32") Option = UI32Format;
        else if (f == "d") Option = DoubleFormat;
        else if (f == "f") Option = FloatFormat;
        else if (f == "c") Option = CharFormat;
        else if (f == "")
        {
            if (AllowSkip) Option = SkipFormat;
            else return false;
        }
        else return false;
        FormatSelector.push_back(Option);
    }
    return true;
}

void ACore_SI::readFormattedLine(const QStringList & fields, const std::vector<EArrayFormat> &FormatSelector, QVariantList & el)
{
    for (int i = 0; i < (int)FormatSelector.size() && i < fields.length(); i++)
    {
        const QString & txt = fields.at(i);

        switch (FormatSelector.at(i))
        {
        case EArrayFormat::StringFormat:
            el.push_back(txt);
            break;
        case EArrayFormat::IntFormat:
            el.push_back(txt.toInt());
            break;
        case EArrayFormat::DoubleFormat:
            el.push_back(txt.toDouble());
            break;
        case EArrayFormat::FloatFormat:
            el.push_back(txt.toFloat());
            break;
        case EArrayFormat::CharFormat:
            el.push_back(txt.toLatin1().at(0));
            break;
        case EArrayFormat::SkipFormat:
            continue;
        }
    }
}

QVariantList ACore_SI::loadArray(QString fileName, QVariantList format, int fromLine, int untilLine)
{
    QVariantList vl;

    std::vector<EArrayFormat> FormatSelector;
    bool bFormatOK = readFormat(format, FormatSelector);
    if (!bFormatOK)
    {
        abort("Parameter 'format' should be an array of 's', 'i', 'd' or '' markers (string, int, double and skip_field, respectively)");
        return vl;
    }

    if (!QFileInfo::exists(fileName))
    {
        abort("File does not exist: " + fileName);
        return vl;
    }

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QFile::Text))
    {
        abort("Cannot open file: " + fileName);
        return vl;
    }

    QTextStream in(&file);
    const QRegularExpression rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'

    const int numEl = FormatSelector.size();
    int iLine = -1;
    while (!in.atEnd())
    {
        iLine++;
        if (iLine >= untilLine) break;

        const QString line = in.readLine();
        if (iLine < fromLine) continue;

        const QStringList fields = line.split(rx, Qt::SkipEmptyParts);
        if (fields.isEmpty()) continue;
        if (line.startsWith('#') || line.startsWith("//")) continue;  // comment  !!!*** keep?
        if (fields.size() < numEl) continue;

        QVariantList el;
        readFormattedLine(fields, FormatSelector, el);
        vl.push_back(el);
    }

    file.close();
    return vl;
}

QVariantList ACore_SI::loadArray(QString fileName, QVariantList format)
{
    return loadArray(fileName, format, 0, 2147483647);
}

QVariantList ACore_SI::load3DArray(QString fileName, QString topSeparator, QVariantList format, bool skipEmpty, bool allowIncomplete, int recordsFrom, int recordsUntil)
{
    QVariantList vl1;

    std::vector<EArrayFormat> FormatSelector;
    bool bFormatOK = readFormat(format, FormatSelector);
    if (!bFormatOK)
    {
        abort("Parameter 'format' should be an array of 's', 'i', 'd' or '' markers (string, int, double and skip_field, respectively)");
        return vl1;
    }

    if (!QFileInfo::exists(fileName))
    {
        abort("File does not exist: " + fileName);
        return vl1;
    }

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QFile::Text))
    {
        abort("Cannot open file: "+fileName);
        return vl1;
    }

    QTextStream in(&file);
    const QRegularExpression rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'

    const int numEl = FormatSelector.size();
    int iEvent = -1;
    bool bOnStart = true;
    bool bSkippingRecords = true;
    QVariantList vl2;
    while(!in.atEnd())
    {
        QString line = in.readLine();

        QStringList fields = line.split(rx, Qt::SkipEmptyParts);
        if (fields.isEmpty()) continue;

        const QString & first = fields.first();
        if (first.startsWith(topSeparator))
        {
            //new events
            iEvent++;
            if (iEvent < recordsFrom)
                continue;         // still skipping events
            else
                bSkippingRecords = false;

            if (bOnStart)
                bOnStart = false; //buffer is invalid
            else                  //else save buffer
            {
                if (!vl2.empty() || !skipEmpty) vl1.push_back(vl2);
                vl2.clear();
            }

            if (iEvent >= recordsUntil)
                return vl1;

            continue;
        }

        if (bSkippingRecords) continue;

        if (fields.size() < numEl && !allowIncomplete) continue;

        QVariantList el3;
        readFormattedLine(fields, FormatSelector, el3);
        vl2.push_back(el3);
    }

    if (!vl2.empty() || !skipEmpty) vl1.push_back(vl2);

    file.close();
    return vl1;
}

bool ACore_SI::readFormattedBinaryLine(std::ifstream & inStream, const std::vector<EArrayFormat> &FormatSelector, QVariantList & el)
{
    for (int i = 0; i < (int)FormatSelector.size(); i++)
    {
        switch (FormatSelector.at(i))
        {
        case EArrayFormat::StringFormat:
        {
            QString str;
            char ch;
            while (inStream >> ch)
            {
                if (ch == 0x00) break;
                str += ch;
            }
            //qDebug() << "str:"<<str;
            el.push_back(str);
            break;
        }
        case EArrayFormat::IntFormat:
        {
            int v;
            inStream.read((char*)&v, sizeof(int));
            //qDebug() << "int:"<<v;
            el.push_back(v);
            break;
        }
        case EArrayFormat::UI32Format:
        {
            int v;
            inStream.read((char*)&v, sizeof(uint32_t));
            //qDebug() << "int:"<<v;
            el.push_back(v);
            break;
        }
        case EArrayFormat::DoubleFormat:
        {
            double v;
            inStream.read((char*)&v, sizeof(double));
            //qDebug() << "double:"<<v;
            el.push_back(v);
            break;
        }
        case EArrayFormat::FloatFormat:
        {
            float v;
            inStream.read((char*)&v, sizeof(float));
            //qDebug() << "float:"<<v;
            el.push_back(v);
            break;
        }
        case EArrayFormat::CharFormat:
        {
            char v;
            inStream >> v;
            //qDebug() << "char:"<<v;
            el.push_back(v);
            break;
        }
        case EArrayFormat::SkipFormat:
            continue;
        }
    }

    return !inStream.fail();
}

bool ACore_SI::writeFormattedBinaryLine(std::ofstream & outStream, const std::vector<EArrayFormat> & FormatSelector, QVariantList & el, QString & err)
{
    bool ok;
    for (size_t i = 0; i < FormatSelector.size(); i++)
    {
        switch (FormatSelector.at(i))
        {
        case StringFormat:
        {
            const QString str = el[i].toString();
            for (int iStr = 0; iStr < str.length(); iStr++)
                outStream << str[iStr].toLatin1();
            outStream << '\0';
            break;
        }
        case IntFormat:
        {
            const int v = el[i].toInt(&ok);
            if (!ok)
            {
                err = "Write binary line to file: error in convesion to int";
                return false;
            }
            outStream.write((char*)&v, sizeof(int));
            break;
        }
        case UI32Format:
        {
            const uint32_t v = el[i].toUInt(&ok);
            if (!ok)
            {
                err = "Write binary line to file: error in convesion to uint32";
                return false;
            }
            outStream.write((char*)&v, sizeof(uint32_t));
            break;
        }
        case DoubleFormat:
        {
            const double v = el[i].toDouble(&ok);
            if (!ok)
            {
                err = "Write binary line to file: error in convesion to double";
                return false;
            }
            outStream.write((char*)&v, sizeof(double));
            break;
        }
        case FloatFormat:
        {
            const float v = el[i].toFloat(&ok);
            if (!ok)
            {
                err = "Write binary line to file: error in convesion to float";
                return false;
            }
            outStream.write((char*)&v, sizeof(float));
            break;
        }
        case CharFormat:
        {
            const char v = el[i].toChar().toLatin1();
            outStream << v;
            break;
        }
        case SkipFormat:
            continue;
        }
    }

    return true;
}

QVariantList ACore_SI::loadArrayBinary(const QString &fileName, const QVariantList &format)
{
    QVariantList vl1;

    std::vector<EArrayFormat> FormatSelector;
    bool bFormatOK = readFormat(format, FormatSelector, false);
    if (!bFormatOK)
    {
        abort("Parameter 'format' should be an array of 's', 'i', 'd', 'f' or 'c' markers (string, int, double, float and char, respectively)");
        return vl1;
    }

    if ( !QFileInfo::exists(fileName) )
    {
        abort("File does not exist: " + fileName);
        return vl1;
    }

    std::ifstream inStream(fileName.toLatin1().data(), std::ios::in | std::ios::binary);
    if (!inStream.is_open())
    {
        abort("Cannot open input file: " + fileName);
        return vl1;
    }

    do
    {
        QVariantList el2;
        bool bOK = readFormattedBinaryLine(inStream, FormatSelector, el2);
        if (bOK)
        {
            if (FormatSelector.size() == 1)
                vl1.push_back(el2.at(0));
            else
                vl1.push_back(el2);
        }
    }
    while (!inStream.fail());

    inStream.close();

    if (!inStream.eof()) abort("Format error!");

    return vl1;
}

void ACore_SI::save3DArray(QVariantList array, QString fileName, QString topLevelSeparator, QVariantList topLevelLabels, bool append)
{
    bool useTopLevelLabels = !topLevelLabels.empty();
    if (useTopLevelLabels && array.size() != topLevelLabels.size())
    {
        abort("Mismatch in the sizes of the array and topLevelLabels");
        return;
    }

    if (append && !QFileInfo::exists(fileName))
    {
        abort("File does not exist: " + fileName);
        return;
    }
    QFile file(fileName);
    if ( !file.open(append ? QIODevice::Append : QIODevice::WriteOnly) )
    {
        abort("Cannot open file: " + fileName);
        return;
    }

    QTextStream stream(&file);
    for (int i1 = 0; i1 < array.size(); i1++)
    {
        //header of the event
        stream << topLevelSeparator;

        if (useTopLevelLabels)
        {
            const QVariant & var = topLevelLabels[i1];
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            if (var.type() == QVariant::List)
#else
            if (var.userType() == QMetaType::QVariantList)
#endif
            {
                const QStringList sl = var.toStringList();
                stream << sl.join(' ');
            }
            else stream << var.toString();
        }

        stream << '\n';

        //data
        QVariantList second = array[i1].toList();
        for (int i2 = 0; i2 < second.size(); i2++)
        {
            const QVariant & var = second[i2];
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            if (var.type() == QVariant::List)
#else
            if (var.userType() == QMetaType::QVariantList)
#endif
            {
                const QStringList sl = var.toStringList();
                stream << sl.join(' ');
            }
            else stream << var.toString();
            stream << '\n';
        }
    }
}

QVariantList ACore_SI::load3DBinaryArray(QString fileName, char dataId, QVariantList dataFormat,
                                         char separatorId, QVariantList separatorFormat,
                                         int recordsFrom, int recordsUntil, bool skipEmpty)
{
    QVariantList vl1;

    std::vector<EArrayFormat> DataFormatSelector;
    bool bFormatOK = readFormat(dataFormat, DataFormatSelector, false);
    if (!bFormatOK)
    {
        abort("Parameter 'dataFormat' should be an array of 's', 'i', 'd', 'f' or 'c' markers (string, int, double, float and char, respectively)");
        return vl1;
    }
    std::vector<EArrayFormat> SeparatorFormatSelector;
    bFormatOK = readFormat(separatorFormat, SeparatorFormatSelector, false, true);
    if (!bFormatOK)
    {
        abort("Parameter 'separatorFormat' should be an array of 's', 'i', 'd', 'f' or 'c' markers (string, int, double, float and char, respectively)");
        return vl1;
    }

    if (!QFileInfo::exists(fileName))
    {
        abort("File does not exist: " + fileName);
        return vl1;
    }

    std::ifstream inStream(fileName.toLatin1().data(), std::ios::in | std::ios::binary);
    if (!inStream.is_open())
    {
        abort("Cannot open input file: " + fileName);
        return vl1;
    }

    int iEvent = -1;
    bool bOnStart = true;
    bool bSkippingRecords = true;
    QVariantList vl2;
    char ch;
    while (inStream >> ch)
    {
        if (ch == separatorId)
        {
            //new top separator
            QVariantList dummy;
            //qDebug() << "This is separator";
            readFormattedBinaryLine(inStream, SeparatorFormatSelector, dummy);

            iEvent++;
            if (iEvent < recordsFrom)
                continue;         // still skipping events
            else
                bSkippingRecords = false;

            if (bOnStart)
                bOnStart = false; //buffer is invalid
            else                  //else save buffer
            {
                //vl1.push_back(vl2);
                if (!vl2.empty() || !skipEmpty) vl1.push_back(vl2);
                vl2.clear();
            }

            if (iEvent >= recordsUntil)
                return vl1;
        }
        else if (ch == dataId)
        {
            //qDebug() << "This is data line";
            QVariantList el3;
            readFormattedBinaryLine(inStream, DataFormatSelector, el3);

            if (!bSkippingRecords) vl2.push_back(el3);
        }
        else
        {
            qDebug() << "Format error: got leading char:" << ch;
            abort("Format error!");
            return vl1;
        }
    }
    //vl1.push_back(vl2);
    if (!vl2.empty() || !skipEmpty) vl1.push_back(vl2);

    inStream.close();
    return vl1;
}

void ACore_SI::save3DBinaryArray(QVariantList data, QString fileName, char dataId, QVariantList dataFormat, char separatorId, bool append)
{
    save3DBinaryArray(data, fileName, dataId, dataFormat, separatorId, QVariantList(), QVariantList(), append);
}

void ACore_SI::save3DBinaryArray(QVariantList data, QString fileName, char dataId, QVariantList dataFormat,
                                 char separatorId, QVariantList topLevelLabels, QVariantList separatorFormat, bool append)
{
    QVariantList vl1;

    std::vector<EArrayFormat> DataFormatSelector;
    bool bFormatOK = readFormat(dataFormat, DataFormatSelector, false, false);
    if (!bFormatOK)
    {
        abort("Parameter 'dataFormat' should be an array of 's', 'i', 'd', 'f' or 'c' markers (string, int, double, float and char, respectively)");
        return;
    }


    if (append && !QFileInfo::exists(fileName))
    {
        abort("File does not exist: " + fileName);
        return;
    }

    std::ofstream outStream(fileName.toLatin1().data(), std::ios::out | std::ios::binary);
    if (!outStream.is_open())
    {
        abort("Cannot open file: " + fileName + " for binary save");
        return;
    }

    std::vector<EArrayFormat> SeparatorFormatSelector;
    bool haveLabels = !topLevelLabels.isEmpty();
    if (haveLabels)
    {
        if (data.size() != topLevelLabels.size())
        {
            abort("Size mismatch for the data and label arrays");
            return;
        }
        bFormatOK = readFormat(separatorFormat, SeparatorFormatSelector, false, false);
        if (!bFormatOK)
        {
            abort("Parameter 'separatorFormat' should be an array of 's', 'i', 'd', 'f' or 'c' markers (string, int, double, float and char, respectively)");
            return;
        }
    }

    QString err;
    for (int i0 = 0; i0 < data.size(); i0++)
    {
        outStream << char(separatorId);

        if (haveLabels)
        {
            QVariantList el = topLevelLabels[i0].toList();
            if (el.size() > SeparatorFormatSelector.size())
            {
                abort( QString("Mismatch in size of top level labels and separator format for top index of %0").arg(i0) );
                return;
            }
            bool ok = writeFormattedBinaryLine(outStream, SeparatorFormatSelector, el, err);
            if (!ok)
            {
                abort(err);
                return;
            }
        }

        QVariantList lev1array = data[i0].toList();
        for (int i1 = 0; i1 < lev1array.size(); i1++)
        {
            outStream << char(dataId);

            QVariantList lev2array = lev1array[i1].toList();

            qDebug() << lev2array << DataFormatSelector;

            if (lev2array.size() > DataFormatSelector.size())
            {
                abort( QString("Mismatch in size of data array and separator format for index [%0][%1]").arg(i0).arg(i1) );
                return;
            }
            bool ok = writeFormattedBinaryLine(outStream, DataFormatSelector, lev2array, err);
            if (!ok)
            {
                abort(err);
                return;
            }
        }
    }
}

QVariantMap ACore_SI::loadObject(QString fileName)
{
    QFile loadFile(fileName);
    if (!loadFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Cannot open file " + fileName;
        return QVariantMap();
    }

    QByteArray saveData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    QJsonObject json = loadDoc.object();

    QVariantMap v = json.toVariantMap();
    return v;
}

/*
#include "ainternetbrowser.h"
QVariant ACore_SI::loadArrayFromWeb(QString url, int msTimeout)
{
    AInternetBrowser b(msTimeout);
    QString Reply;
    bool fOK = b.Post(url, "", Reply);
    //  qDebug() << "Post result:"<<fOK;

    if (!fOK)
    {
        abort("Error:\n" + b.GetLastError());
        return 0;
    }
    //  qDebug() << Reply;

    const QRegularExpression rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'
    QVariantList vl;

    QStringList sl = Reply.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
    for (const QString& line : sl)
    {
        QStringList fields = line.split(rx, QString::SkipEmptyParts);

        if (fields.isEmpty()) continue;
        bool bOK;
        double first = fields.at(0).toDouble(&bOK);
        if (!bOK) continue;

        if (fields.size() == 1)
            vl.append(first);
        else
        {
            QVariantList el;
            el << first;
            for (int i=1; i<fields.size(); i++)
                el << fields.at(i).toDouble();
            vl.push_back(el);
        }
    }

    return vl;
}
*/

#include "a3global.h"
QString ACore_SI::getExamplesDir()
{
    return A3Global::getConstInstance().ExamplesDir;
}

QVariantList ACore_SI::setNewFileFinder(QString dir, QString fileNamePattern)
{
    Finder_Dir = dir;
    Finder_NamePattern = fileNamePattern;

    QDir d(dir);
    QStringList files = d.entryList( QStringList(fileNamePattern), QDir::Files);
    //  qDebug() << files;

    QVariantList res;
    for (auto& n : files)
    {
        Finder_FileNames << n;
        res << n;
    }
    return res;
}

QVariantList ACore_SI::getNewFiles()
{
    QVariantList newFiles;
    QDir d(Finder_Dir);
    QStringList files = d.entryList( QStringList(Finder_NamePattern), QDir::Files);

    for (auto& n : files)
    {
        if (!Finder_FileNames.contains(n)) newFiles << QVariant(n);
        Finder_FileNames << n;
    }
    return newFiles;
}

QVariantList ACore_SI::getDirectories(QString dir, QString dirNamePattern)
{
    QDir d(dir);
    QStringList dl = d.entryList( QStringList(dirNamePattern), QDir::Dirs);

    QVariantList Dirs;
    for (const QString & s : dl)
    {
        if (s == ".") continue;
        if (s == "..") continue;
        Dirs << s;
    }
    return Dirs;
}

QString ACore_SI::str(double value, int precision)
{
    return QString::number(value, 'g', precision);
}

QString ACore_SI::toStr(QVariant var)
{
    QString s;
    AVirtualScriptManager::addQVariantToString(var, s, Lang);
    return s;
}

void ACore_SI::processEvents()
{
    AScriptHub::getInstance().processEvents(Lang);
}

void ACore_SI::requestGuiUpdate()
{
    emit AScriptHub::getInstance().requestUpdateGui();
}

void ACore_SI::reportProgress(int percents)
{
    AScriptHub::getInstance().reportProgress(percents, Lang);
}

void ACore_SI::createFile(QString fileName)
{
    QFile file(fileName);
    if (file.exists())
    {
        abort("File already exists: " + fileName);
        return;
    }

    if ( !file.open(QIODevice::WriteOnly) )
        abort("Cannot open file: " + fileName);
}

bool ACore_SI::isFileExist(QString fileName)
{
    return QFileInfo(fileName).exists();
}

void ACore_SI::deleteFile(QString fileName)
{
    bool ok = QFile(fileName).remove();
    if (!ok) abort("Cannot delete file: " + fileName);
}

void ACore_SI::createDir(QString path)
{
    QDir dir("");
    if (!dir.mkpath(path))
        abort("Cannot create directory: " + path);
}

QString ACore_SI::getCurrentDir()
{
    return QDir::currentPath();
}

void ACore_SI::setCurrentDir(QString path)
{
    bool ok = QDir::setCurrent(path);
    if (!ok) abort("cannot set current directory to " + path);
}


#include <QProcess>
QString ACore_SI::startExternalProcessAndWait(QString command, QVariant argumentArray, int maxWaitMilliseconds)
{
#ifndef _ALLOW_LAUNCH_EXTERNAL_PROCESS_
    abort("Launch of external process is not allowed.\nEnable \"_ALLOW_LAUNCH_EXTERNAL_PROCESS_\" in ants3.pro");
    return "";
#else
    QStringList arg;
    QString type = argumentArray.typeName();
    if (type == "QString") arg << argumentArray.toString();
    else if (type == "int") arg << QString::number(argumentArray.toInt());
    else if (type == "double") arg << QString::number(argumentArray.toDouble());
    else if (type == "QVariantList")
    {
        QVariantList vl = argumentArray.toList();
        QJsonArray ar = QJsonArray::fromVariantList(vl);
        for (int i=0; i<ar.size(); i++) arg << ar.at(i).toString();
    }
    else
    {
        abort("Format error in argument list");
        return "";
    }

    QString str = command + " ";
    for (QString & s : arg) str += s + " ";
    qDebug() << "Executing external command:" << str;

    QProcess * process = new QProcess(this);
    QString errorString;

    if (maxWaitMilliseconds > 0)
    {
        process->start(command, arg);
        process->waitForFinished(maxWaitMilliseconds);
        errorString = process->errorString();
        delete process;
    }
    else
    {
        QObject::connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
        process->start(command, arg);
    }

    return errorString;
#endif // ANTS2_ALLOW_EXTERNAL_PROCESS
}

void ACore_SI::startExternalProcess(QString command, QVariant arguments)
{
    startExternalProcessAndWait(command, arguments, 0);
}
