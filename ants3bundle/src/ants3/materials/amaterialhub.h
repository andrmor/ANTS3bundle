#ifndef AMATERIALHUB_H
#define AMATERIALHUB_H

#include "amaterial.h"

#include <QVector>
#include <QString>
#include <QObject>
#include <QStringList>

#include <vector>
#include <string>

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

    AMaterial* operator[](int i) {return Materials[i];}
    const AMaterial* operator[](int i) const {return Materials[i];}

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);

    void    generateGeoMedia();
    void    updateRuntimeProperties();

    void    addNewMaterial(bool fSuppressChangedSignal = false);               //
    void    addNewMaterial(QString name, bool fSuppressChangedSignal = false); // !!!*** make single method!
    void    addNewMaterial(QJsonObject & json); // !!!*** change to loadMaterial(filename)

    bool    renameMaterial(int iMat, const QString & newName);

    int     findMaterial(const QString & name) const; //if not found, returns -1; if found, returns material index

    QString tryRemoveMaterial(int iMat); // !!!*** add check for PhotonSources!

    void    copyToMaterials(const AMaterial & tmpMaterial);  // update if name exists, otherwise creates new one
    void    copyMaterialToTmp(int imat, AMaterial & tmpMaterial);

    int     countMaterials() const {return Materials.size();}
    QString getMaterialName(int matIndex) const;
    QStringList getListOfMaterialNames() const;
    std::vector<std::string> getMaterialNames() const;
    double  getDriftSpeed(int iMat) const; //returns in mm / ns
    double  getDiffusionSigmaTime(int iMat, double length_mm) const;
    double  getDiffusionSigmaTransverse(int iMat, double length_mm) const;
    void    checkReadyForGeant4Sim(QString & Errors) const;

    QString CheckMaterial(const AMaterial *mat) const; //"" - check passed, otherwise error
    QString CheckMaterial(int iMat) const;       //"" - check passed, otherwise error

private:
    void    removeMaterial(int iMat); // !!!*** propagate to PhotonSources!
    void    clearMaterials();
    void    ensureMatNameIsUnique(AMaterial * mat);

signals:
    void materialsChanged();

};

#endif // AMATERIALHUB_H
