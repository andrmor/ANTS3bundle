#include "acore_si.h"
#include "ascripthub.h"
#include "ascripthub.h"
#include "avirtualscriptmanager.h"
#include "ajscriptmanager.h"
#ifdef ANTS3_PYTHON
    #include "apythonscriptmanager.h"
#endif
#include "afiletools.h"

#ifdef _ALLOW_LAUNCH_EXTERNAL_PROCESS_
#include <QProcess>
#endif

#include <QDateTime>
#include <QFileInfo>
#include <QFile>
#include <QDebug>
#include <QtWidgets/QApplication>
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

    Help["abort"] = "Abort script execution and show text (1st argument) in the output";

    Help["clearOutput"] = "Clear the output text";
    Help["print"] = "Append up to 10 arguments to the output";
    Help["printHtml"] = "Append the provided string to the output assuming HTML format";

    Help["getTimeMark"] = "Return the number of milliseconds since 1970-01-01T00:00:00 Universal Coordinated Time";

    Help["save"] = "Add string (second argument) to the file with the name given by the first argument.\n"
              "Save is not performed (and false is returned) if the file does not exist\n"
              "It is a very slow method!\n"
              "Use \"<br>\" or \"\\n\" to add a line break.\n"
              "For Windows users: pathes have to use \"/\" character, e.g. c:/tmp/file.txt\n";
    Help["saveArray"] = "Appends an array (or array of arrays) with numeric data to the file.\n"
                   "Save is not performed (and false is returned) if the file does not exist.\n"
                   "For Windows users: pathes have to use \"/\" character, e.g. c:/tmp/file.txt\n";
    Help["createFile"] = "Create new or clear an existent file.\n"
                    "The file is NOT rewritten if the second argument is true (or absent) and the file already exists\n"
                    "For Windows users: pathes have to use \"/\" character, e.g. c:/tmp/file.txt\n";
    Help["isFileExists"] = "Return true if file exists";
    Help["loadColumn"] = "Load a column with ascii numeric data from the file.\nSecond argument is the column number starting from 0.";
//    Help["loadArray"] = "Load an array of numerics (or an array of numeric arrays).\nSecond argument is used to limit the number of columns to read";
    Help["evaluate"] = "Evaluate script during another script evaluation. See example ScriptInsideScript.txt";

    Help["setNewFileFinder"] = "Configurer for GetNewFiles() function. dir is the search directory, fileNamePattern: *.* for all files. Function return all filenames found.";
    Help["getNewFiles"] = "Get list (array) of names of new files appeared in the directory configured with SetNewFileFinder()";

    Help["loadArrayExtended"] = "Load array of arrays from file, with inner array read according to format options:\n"
          "'d'-double, 'i'-integer, 's'-string, ''-skip field: e.g. loadArrayExtended('fn.txt', ['d', 'd'])\n"
          "bSkipComments parameters signals to skip lines starting with '#' or '//'"
          "you can specify lin numbers to start from and to: by default it is set to 0 and 1e6";
    Help["loadArrayBinary"] = "Load array of arrays (binary data), with second argument providing the format\n"
          "This parameter should be an array of 's', 'i', 'd', 'f' or 'c' markers (zero-terminating string, int, double, float and char, respectively)";
}

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
    AScriptInterface::abort(message);
}

QVariant ACore_SI::test(QVariant in)
{
    qDebug() << in;
    return in;
}

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

    /*
// timed run
    std::cout << "Timed run\n";
    auto start = std::chrono::high_resolution_clock::now();

//  Code to be timed
    double sum =  0.;
    for (int i=0; i<10000000; i++) {
        sum += p.Eval(6);
    }
    std::cout << sum << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    auto diff = end - start;
    std::cout << std::chrono::duration <double, std::nano> (diff).count()/10000000 << " ns/eval" << std::endl;
    */
    return res;
}

#include "amatcomposition.h"
QString ACore_SI::testComposition(QString comp)
{
    AMatComposition mc;
    bool ok = mc.parse(comp);
    if (!ok)
    {
        abort(mc.ErrorString);
        return "";
    }

    TGeoMaterial * mat = mc.constructGeoMaterial("MatNameTest",1.1, 321.0);
    qDebug() << "\nGeoMat to composition string:\n" << AMatComposition::geoMatToCompositionString(mat);

    return mc.printComposition();
}

/*
int ACore_SI::fun(int i)
{
    qDebug() << "Single arg!";
    return i;
}
*/

int ACore_SI::fun(int i, int j, int k)
{
    qDebug() << "Three args!";
    return i + j + k;
}

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
        qApp->processEvents();
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
    AScriptHub::getInstance().outputText(s, Lang);
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
    AScriptHub::getInstance().outputText(s, Lang);
}

void ACore_SI::printHtml(QString text)
{
    //QString s;
    //AVirtualScriptManager::addQVariantToString(text, s, Lang);
    //AScriptHub::getInstance().outputHtml(s, Lang);
    AScriptHub::getInstance().outputHtml(text, Lang);
}

void ACore_SI::clearOutput()
{
    AScriptHub::getInstance().clearOutput(Lang);
}

QString ACore_SI::getDateTimeStamp()
{
    return QDateTime::currentDateTime().toString("dd.MMM.yyyy H:m:s");
}

void ACore_SI::saveText(QString text, QString fileName, bool append)
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

void ACore_SI::saveBinaryArray(const QVariantList & array, const QVariantList & format, const QString & fileName, bool append)
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
    QRegularExpression rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'

    while (!in.atEnd())
    {
        const QString line = in.readLine();
        if (line.startsWith('#') || line.startsWith("//")) continue; // it is a comment

        const QStringList fields = line.split(rx, Qt::SkipEmptyParts);
        if (fields.isEmpty()) continue;

        bool bOK;
        double first = fields.at(0).toDouble(&bOK); // non-regular comments? !!!*** keep it?
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
    for (int i = 0; i < (int)FormatSelector.size(); i++)
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

QVariantList ACore_SI::loadArray(const QString & fileName, const QVariantList & format, int fromLine, int untilLine)
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
    QRegularExpression rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'

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

QVariantList ACore_SI::load3DArray(const QString &fileName, const QString &topSeparator, const QVariantList &format, int recordsFrom, int recordsUntil)
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
    QRegularExpression rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'

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
                vl1.push_back(vl2);
                vl2.clear();
            }

            if (iEvent >= recordsUntil)
                return vl1;

            continue;
        }

        if (bSkippingRecords) continue;

        if (fields.size() < numEl) continue;

        QVariantList el3;
        readFormattedLine(fields, FormatSelector, el3);
        vl2.push_back(el3);
    }
    vl1.push_back(vl2);

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

void ACore_SI::save3DArray(QVariantList array, QString topLevelSeparator, QVariantList topLevelLabels, QString fileName, bool append)
{
    if (array.size() != topLevelLabels.size())
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

QVariantList ACore_SI::load3DBinaryArray(const QString &fileName, char dataId, const QVariantList &dataFormat, char separatorId, const QVariantList &separatorFormat, int recordsFrom, int recordsUntil)
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
                vl1.push_back(vl2);
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
    vl1.push_back(vl2);

    inStream.close();
    return vl1;
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

    QRegularExpression rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'
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

QVariantList ACore_SI::setNewFileFinder(const QString dir, const QString fileNamePattern)
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

QVariantList ACore_SI::getDirectories(const QString dir, const QString dirNamePattern)
{
    QDir d(dir);
    QStringList dl = d.entryList( QStringList(dirNamePattern), QDir::Dirs);

    QVariantList Dirs;
    for (const QString & s : dl) Dirs << s;
    return Dirs;
}

void ACore_SI::processEvents()
{
    qApp->processEvents();
}

void ACore_SI::requestGuiUpdate()
{
    emit AScriptHub::getInstance().requestUpdateGui();
}

/*
void ACore_SI::reportProgress(int percents)
{
    emit ScriptManager->reportProgress(percents);
    qApp->processEvents();
}
*/

void ACore_SI::createFile(QString fileName)
{
    QFile file(fileName);
    if ( !file.open(QIODevice::WriteOnly) )
        abort("Cannot open file: " + fileName);
}

bool ACore_SI::isFileExist(QString fileName)
{
    return QFileInfo(fileName).exists();
}

bool ACore_SI::deleteFile(QString fileName)
{
    return QFile(fileName).remove();
}

bool ACore_SI::createDir(QString path)
{
    QDir dir("");
    return dir.mkpath(path);
}

QString ACore_SI::getCurrentDir()
{
    return QDir::currentPath();
}

#include <QProcess>
QString ACore_SI::startExternalProcess(QString command, QVariant argumentArray, bool waitToFinish, int milliseconds)
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

    if (waitToFinish)
    {
        process->start(command, arg);
        process->waitForFinished(milliseconds);
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
