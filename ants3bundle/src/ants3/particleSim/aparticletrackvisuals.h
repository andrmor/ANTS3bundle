#ifndef ATRACKDRAWOPTIONS_H
#define ATRACKDRAWOPTIONS_H

#include <map>
#include <vector>

#include <QString>

class QJsonObject;
class TVirtualGeoTrack;

class ATrackAttributes
{
public:
    int color = 7;
    int width = 1;
    int style = 1;

    void setTrackAttributes(TVirtualGeoTrack * track) const;

    void writeToJson(QJsonObject& json) const;
    const QJsonObject writeToJson() const;
    void readFromJson(const QJsonObject& json);

    void reset();
};

class AParticleTrackVisuals
{
public:
    static AParticleTrackVisuals & getInstance();

private:
    AParticleTrackVisuals();
    ~AParticleTrackVisuals(){}

    AParticleTrackVisuals(const AParticleTrackVisuals&)            = delete;
    AParticleTrackVisuals(AParticleTrackVisuals&&)                 = delete;
    AParticleTrackVisuals& operator=(const AParticleTrackVisuals&) = delete;
    AParticleTrackVisuals& operator=(AParticleTrackVisuals&&)      = delete;

public:
    ATrackAttributes DefaultAttributes;  //default width/style and color for particle # beyound covered in DefaultParticle_Colors
    std::vector<int> DefaultColors;  // !!!*** change to default for gamma neutron electron positron etc
    std::map<QString, ATrackAttributes> CustomAttributes;

    void writeToJson(QJsonObject& json) const;
    void readFromJson(const QJsonObject& json);

    void applyToParticleTrack(TVirtualGeoTrack * track, const QString & Particle) const;

private:
    void clear(); //clear and reset to default values
};

#endif // ATRACKDRAWOPTIONS_H
