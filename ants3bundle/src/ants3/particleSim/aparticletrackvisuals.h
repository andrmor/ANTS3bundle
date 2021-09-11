#ifndef ATRACKDRAWOPTIONS_H
#define ATRACKDRAWOPTIONS_H

#include <QVector>

#include <vector>

class QJsonObject;
class TVirtualGeoTrack;

class ATrackAttributes
{
public:
    int color = 7;
    int width = 1;
    int style = 1;

    void setTrackAttributes(TVirtualGeoTrack* track) const;

    void writeToJson(QJsonObject& json) const;
    const QJsonObject writeToJson() const;
    void readFromJson(const QJsonObject& json);

    void reset();
};

class AParticleTrackVisuals
{
public:
    AParticleTrackVisuals();

    ATrackAttributes TA_DefaultParticle;  //default width/style and color for particle # beyound covered in DefaultParticle_Colors
    std::vector<int> DefaultParticle_Colors;
    QVector<ATrackAttributes*> CustomParticle_Attributes;

    void writeToJson(QJsonObject& json) const;
    void readFromJson(const QJsonObject& json);

    void applyToParticleTrack(TVirtualGeoTrack *track, int ParticleId) const;
    int  getParticleColor(int ParticleId) const;

private:
    void clear(); //clear and reset to default values
    void clearCustomParticleAttributes();
};

#endif // ATRACKDRAWOPTIONS_H
