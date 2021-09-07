#ifndef APARTICLERUNSETTINGS_H
#define APARTICLERUNSETTINGS_H

#include <QString>

class QJsonObject;

class AParticleRunSettings
{
public:
    int     Seed = 0;

    int     EventFrom = 0;
    int     EventTo   = 0;

    bool    AsciiOutput    = true;
    int     AsciiPrecision = 6;

    QString OutputDirectory;

    bool    SaveTrackingData = true;
    QString FileNameTrackingData = "TrackingData.txt";

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    void clear();

    QString getGdmlFileName() const {return "Detector.gdml";}
    QString getPrimariesFileName(int index) const {return QString("primaries-%1.txt").arg(index); }
};

#endif // APARTICLERUNSETTINGS_H
