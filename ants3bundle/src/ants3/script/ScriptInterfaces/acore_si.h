#ifndef ACORESCRIPTINTERFACE_H
#define ACORESCRIPTINTERFACE_H

#include "ascriptinterface.h"

#include <QVariant>
#include <QSet>
#include <QString>
#include <QJSValue>

#include <vector>

class CurveFit;
class AScriptMessenger;

class ACore_SI : public AScriptInterface
{
    Q_OBJECT

public:
    ACore_SI();
//  ACore_SI(const ACore_SI& other);

    AScriptInterface * cloneBase() const override {return new ACore_SI();}

    bool beforeRun() override;

public slots:
    // Internal tests
    //QVariant test(QVariant in);
    //double testVFormula(QString formula, QVariantList varNames, QVariantList varValues);
    //QString testComposition(QString comp);
    //int fun(int i, int j, int k);
    //int fun(int i);

    void         abort(QString message);

    // Output
    void         print(QVariant message);
    void         print(QVariant m1, QVariant m2, QVariant m3=QVariant(), QVariant m4=QVariant(), QVariant m5=QVariant(), QVariant m6=QVariant(), QVariant m7=QVariant(), QVariant m8=QVariant(), QVariant m9=QVariant(), QVariant m10=QVariant());
    void         printHtml(QString text);
    void         clearOutput();

    // Time
    void         sleep(int ms);
    double       getTimeMarkMilliseconds();
    QString      getDateTimeStamp();

    // Basic io
    void         createDir(QString path);
    void         createFile(QString fileName);
    void         deleteFile(QString fileName);
    bool         isFileExist(QString fileName);

    void         setCurrentDir(QString path);
    QString      getCurrentDir();

    QVariantList getDirectories(QString dir, QString dirNamePattern);
    QVariantList setNewFileFinder(QString dir, QString fileNamePattern);
    QVariantList getNewFiles();

    // Text
    void         saveText(QString text, QString fileName);
    void         appendText(QString text, QString fileName);
    QString      loadText(QString fileName);
    // !!!*** read line from file

    // Ascii array
    void         saveArray(QVariantList array, QString fileName, bool append = false);
    QVariantList loadNumericArray(QString fileName);
    QVariantList loadArray(const QString & fileName, const QVariantList & format);
    QVariantList loadArray(const QString & fileName, const QVariantList & format, int fromLine, int untilLine);

    // Binary array
    void         saveBinaryArray(const QVariantList & array, const QVariantList & format, const QString & fileName, bool append);
    QVariantList loadArrayBinary(const QString & fileName, const QVariantList & format);

    // 3D arrays
    void         save3DArray(QVariantList array, QString topLevelSeparator, QVariantList topLevelLabels, QString fileName, bool append);
    QVariantList load3DArray(const QString & fileName, const QString & topSeparator, const QVariantList & format, int recordsFrom, int recordsUntil, bool skipEmpty, bool allowIncomplete);
    QVariantList load3DBinaryArray(const QString &fileName, char dataId, const QVariantList &dataFormat, char separatorId, const QVariantList &separatorFormat, int recordsFrom = 0, int recordsUntil = 1e6, bool skipEmpty = false);

    // Object
    void        saveObject(QVariantMap Object, QString FileName);
    QVariantMap loadObject(QString fileName);

//    QVariant loadArrayFromWeb(QString url, int msTimeout = 3000);

    //misc
    QString str(double value, int precision);
    QString toStr(QVariant var);
    double  arraySum(QVariantList array);
    QString getExamplesDir();
    void    processEvents();
    void    reportProgress(int percents);

    void requestGuiUpdate();

    QString startExternalProcess(QString command, QVariant arguments, bool waitToFinish, int milliseconds);

private:
    enum EArrayFormat {StringFormat, IntFormat, UI32Format, DoubleFormat, FloatFormat, CharFormat, SkipFormat};

    //file finder
    QSet<QString>   Finder_FileNames;
    QString         Finder_Dir;
    QString         Finder_NamePattern = "*.*";

    AScriptMessenger * Messenger = nullptr;

    void readFormattedLine(const QStringList &fields, const std::vector<EArrayFormat> &FormatSelector, QVariantList &el);
    bool readFormat(const QVariantList & format, std::vector<EArrayFormat> & FormatSelector, bool AllowSkip = true, bool AllowEmptyFormatArray = false);
    bool readFormattedBinaryLine(std::ifstream &inStream, const std::vector<EArrayFormat> &FormatSelector, QVariantList &el);
    bool writeFormattedBinaryLine(std::ofstream &outStream, const std::vector<EArrayFormat> &FormatSelector, QVariantList &el, QString &err);
};

#endif // ACORESCRIPTINTERFACE_H
