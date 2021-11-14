#ifndef AFILEHANDLERBASE_H
#define AFILEHANDLERBASE_H

#include <QString>
#include <QDateTime>

class ANodeRecord;
class QFile;
class QTextStream;
class QJsonObject;

class AFileSettingsBase
{
public:
    virtual ~AFileSettingsBase(){}

    enum           FileFormatEnum {Undefined = 0, Invalid, Ascii, Binary};

    QString        FileName;
    FileFormatEnum FileFormat = Undefined;
    int            NumEvents  = 0;
    QDateTime      LastModified;

    bool           isValidated() const;
    QString        getFormatName() const;

    void           writeToJson(QJsonObject & json) const;
    void           readFromJson(const QJsonObject & json);

    void           clear();
    virtual void   clearStatistics(){}

protected:
    virtual void   doWriteToJson(QJsonObject & json) const {}
    virtual void   doReadFromJson(const QJsonObject & json) {}
};

class AFileHandlerBase
{
public:
    AFileHandlerBase(AFileSettingsBase & settings);
    virtual ~AFileHandlerBase();

    virtual void determineFormat();                 // very simplistic, better to make more strict
    virtual bool checkFile(bool collectStatistics); // !!!*** add statistics!

    bool init();
    bool gotoEvent(int iEvent);

    //bool readNext(??? & record); // returns false if event ended
    void acknowledgeNextEvent() {EventEndReached = false;}

    bool copyToFile(int fromEvent, int toEvent, const QString & fileName);

private:
    AFileSettingsBase & Settings;

    int             CurrentEvent    = -1;
    bool            EventEndReached = false;

    //resources for ascii input
    QFile         * inTextFile    = nullptr;
    QTextStream   * inTextStream  = nullptr;
    QString         LineText;
    //resources for binary input
    std::ifstream * inStream      = nullptr;
    char            Header        = 0x00;

    QString         FileType; // file type description, e.g. "photon bomb"

    void clearResources();
    bool processEventHeader();
};

#endif // AFILEHANDLERBASE_H
