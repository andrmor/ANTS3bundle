#ifndef ACORESCRIPTINTERFACE_H
#define ACORESCRIPTINTERFACE_H

#include "ascriptinterface.h"

#include <QVariant>
#include <QSet>
#include <QString>
#include <QJSValue>

#include <vector>

class CurveFit;

class ACore_SI : public AScriptInterface
{
    Q_OBJECT

public:
    ACore_SI();
//  ACore_SI(const ACore_SI& other);

    AScriptInterface * cloneBase() const {return new ACore_SI();}

public slots:
    void    abort(QString message);

    QVariant test(QVariant in);

    // Output
    void    clearOutput();
    //void    print(QVariant message); // before was QJSValue
    void    print(QVariant m1, QVariant m2, QVariant m3, QVariant m4, QVariant m5, QVariant m6);
    void    print(QVariant m1, QVariant m2, QVariant m3, QVariant m4, QVariant m5);
    void    print(QVariant m1, QVariant m2, QVariant m3, QVariant m4);
    void    print(QVariant m1, QVariant m2, QVariant m3);
    void    print(QVariant m1, QVariant m2);
    void    print(QVariant message);
    void    printHtml(QVariant message);

    // Time
    void    sleep(int ms);
    double  getTimeMark();
    QString getDateTimeStamp();

    // Basic io
    void createFile(QString fileName);
    bool isFileExist(QString fileName);
    bool deleteFile(QString fileName);
    bool createDir(QString path);
    QString getCurrentDir();
    //bool setCirrentDir(QString path); // affects exec dir for worker processes?

    // Text
    void    saveText(QString text, QString fileName, bool append);
    QString loadText(QString fileName);
    // !!!*** read line from file

    // Ascii array
    void         saveArray(QVariantList array, QString fileName, bool append);
    QVariantList loadNumericArray(QString fileName);
    QVariantList loadArray(const QString & fileName, const QVariantList & format, int fromLine, int untilLine);

    // Binary array
    void         saveBinaryArray(const QVariantList & array, const QVariantList & format, const QString & fileName, bool append);
    QVariantList loadArrayBinary(const QString & fileName, const QVariantList & format);

    // 3D arrays
    void         save3DArray(QVariantList array, QString topLevelSeparator, QVariantList topLevelLabels, QString fileName, bool append);
    QVariantList load3DArray(const QString & fileName, const QString & topSeparator, const QVariantList & format, int recordsFrom, int recordsUntil);
    QVariantList load3DBinaryArray(const QString &fileName, char dataId, const QVariantList &dataFormat, char separatorId, const QVariantList &separatorFormat, int recordsFrom = 0, int recordsUntil = 1e6);

    // Object
    void        saveObject(QVariantMap Object, QString FileName);
    QVariantMap loadObject(QString fileName);

//    QVariant loadArrayFromWeb(QString url, int msTimeout = 3000);

    //file finder
    QVariantList setNewFileFinder(const QString dir, const QString fileNamePattern);
    QVariantList getNewFiles();

    QVariantList getDirectories(const QString dir, const QString dirNamePattern);

    //misc
    QString getExamplesDir();
    void    processEvents();
//  void    reportProgress(int percents);

    void requestGuiUpdate();

    const QString startExternalProcess(QString command, QVariant arguments, bool waitToFinish, int milliseconds);

private:
    enum EArrayFormat {StringFormat, IntFormat, DoubleFormat, FloatFormat, CharFormat, SkipFormat};
    //file finder
    QSet<QString>   Finder_FileNames;
    QString         Finder_Dir;
    QString         Finder_NamePattern = "*.*";

    void readFormattedLine(const QStringList &fields, const std::vector<EArrayFormat> &FormatSelector, QVariantList &el);
    bool readFormat(const QVariantList & format, std::vector<EArrayFormat> & FormatSelector, bool AllowSkip = true, bool AllowEmptyFormatArray = false);
    bool readFormattedBinaryLine(std::ifstream &inStream, const std::vector<EArrayFormat> &FormatSelector, QVariantList &el);
    bool writeFormattedBinaryLine(std::ofstream &outStream, const std::vector<EArrayFormat> &FormatSelector, QVariantList &el, QString &err);
};

#endif // ACORESCRIPTINTERFACE_H
