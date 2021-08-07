#ifndef AMATERIALHUB_H
#define AMATERIALHUB_H

#include "amaterial.h"

#include <QVector>
#include <QString>
#include <QObject>
#include <QStringList>

#include <vector>

class ATracerStateful;
class AGeoObject;

class AMaterialHub : public QObject
{
    Q_OBJECT

    AMaterialHub();
    ~AMaterialHub();

    AMaterialHub(const AMaterialHub&)            = delete;
    AMaterialHub(AMaterialHub&&)                 = delete;
    AMaterialHub& operator=(const AMaterialHub&) = delete;
    AMaterialHub& operator=(AMaterialHub&&)      = delete;

    std::vector<AMaterial*> Materials;

public:
    static       AMaterialHub & getInstance();
    static const AMaterialHub & getConstInstance();

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);

    QStringList getListOfMaterialNames() const;

    void generateGeoMedia();
    void updateRuntimeProperties();

    AMaterial* operator[](int i) {return Materials[i]; } //get pointer to material with index i
    const AMaterial* operator[](int i) const {return Materials[i]; } //get pointer to material with index i

    int countMaterials() const {return Materials.size();}
    QString getMaterialName(int matIndex) const;

    void addNewMaterial(bool fSuppressChangedSignal = false);               //
    void addNewMaterial(QString name, bool fSuppressChangedSignal = false); // !!!*** make single method!

    int  findMaterial(const QString & name) const; //if not found, returns -1; if found, returns material index
    bool DeleteMaterial(int imat); //takes care of overrides of materials with index larger than imat!

    void copyTmpToMaterialCollection(const AMaterial & tmpMaterial);
    void copyMaterialToTmp(int imat, AMaterial & tmpMaterial);

    void addNewMaterial(QJsonObject & json); // !!!*** change to loadMaterial(filename)

    //general purpose requests
    double getDriftSpeed(int iMat) const; //returns in mm / ns
    double getDiffusionSigmaTime(int iMat, double length_mm) const;
    double getDiffusionSigmaTransverse(int iMat, double length_mm) const;
    void   CheckReadyForGeant4Sim(QString & Errors, QString & Warnings, const AGeoObject * World) const;

    QString CheckMaterial(const AMaterial *mat) const; //"" - check passed, otherwise error
    QString CheckMaterial(int iMat) const;       //"" - check passed, otherwise error

private:
    void clearMaterials();
    void ensureMatNameIsUnique(AMaterial * mat);

signals:
    void materialsChanged();

};

#endif // AMATERIALHUB_H
