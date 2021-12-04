#ifndef ACORESCRIPTINTERFACE_H
#define ACORESCRIPTINTERFACE_H

#include "ascriptinterface.h"

#include <QVariant>
#include <QSet>
#include <QString>

class AScriptManager;
class CurveFit;

enum class AArrayFormatEnum {StringFormat, IntFormat, DoubleFormat, FloatFormat, CharFormat, SkipFormat};

class ACore_SI : public AScriptInterface
{
    Q_OBJECT

public:
    ACore_SI();
//  ACore_SI(const ACore_SI& other);

public slots:
    void    abort(QString message);

    // Output
    void    clearOutput();
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
    bool setCirrentDir(QString path); // !!!*** does it affect exec dir for worker processes?

    // Text file
    void    saveText(QString text, QString fileName, bool append);
    QString loadText(QString fileName);

    bool saveArray(QString fileName, QVariantList array);
    void saveArrayBinary(const QString & fileName, const QVariantList & array, const QVariantList & format, bool append = false);
    bool saveObject(QString FileName, QVariant Object, bool CanOverride);

    // Load from file
    QVariant loadArray(QString fileName, int columns);
    QVariant loadArray(QString fileName);
    QVariantList loadArrayExtended(const QString & fileName, const QVariantList & format, int fromLine = 0, int untilLine = 1e6, bool bSkipComments = true);
    QVariantList loadArrayExtended3D(const QString & fileName, const QString & topSeparator, const QVariantList & format, int recordsFrom = 0, int recordsUntil = 1e6, bool bSkipComments = true);
    QVariantList loadArrayBinary(const QString & fileName, const QVariantList & format);
    QVariantList loadArrayExtended3Dbinary(const QString &fileName, char dataId, const QVariantList &dataFormat, char separatorId, const QVariantList &separatorFormat, int recordsFrom = 0, int recordsUntil = 1e6);
    QVariant loadObject(QString fileName);

//    QVariant loadArrayFromWeb(QString url, int msTimeout = 3000);

    //dirs
//    QString GetWorkDir();
//    QString GetScriptDir();
//    QString GetExamplesDir();

    //file finder
    QVariant setNewFileFinder(const QString dir, const QString fileNamePattern);
    QVariant getNewFiles();

    QVariantList getDirectories(const QString dir, const QString dirNamePattern);

    //misc
    void processEvents();
//  void reportProgress(int percents);

    const QString startExternalProcess(QString command, QVariant arguments, bool waitToFinish, int milliseconds);

private:
    //file finder
    QSet<QString>   Finder_FileNames;
    QString         Finder_Dir;
    QString         Finder_NamePattern = "*.*";

    void addQVariantToString(const QVariant & var, QString & string) const;
    void readFormattedLine(const QStringList &fields, const QVector<AArrayFormatEnum> &FormatSelector, QVariantList &el);
    bool readFormat(const QVariantList &format, QVector<AArrayFormatEnum> &FormatSelector, bool AllowSkip = true, bool AllowEmptyFormatArray = false);
    bool readFormattedBinaryLine(std::ifstream &inStream, const QVector<AArrayFormatEnum> &FormatSelector, QVariantList &el);
    QString writeFormattedBinaryLine(std::ofstream &outStream, const QVector<AArrayFormatEnum> &FormatSelector, QVariantList &el);
};

#endif // ACORESCRIPTINTERFACE_H
