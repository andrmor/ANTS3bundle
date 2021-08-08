#include "amaterialhub.h"
//#include "a3config.h"
//#include "ainterfacerule.h"
//#include "acommonfunctions.h"
//#include "atracerstateful.h"
#include "ageoobject.h"
#include "ajsontools.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QDir>

#include "TROOT.h"
#include "TGeoMedium.h"
#include "TGeoMaterial.h"
#include "TH1D.h"
#include "TString.h"

#include <cmath>

AMaterialHub::AMaterialHub() {}

AMaterialHub &AMaterialHub::getInstance()
{
    static AMaterialHub instance;
    return instance;
}

const AMaterialHub & AMaterialHub::getConstInstance()
{
    return getInstance();
}

AMaterialHub::~AMaterialHub()
{
    clearMaterials();
}

double AMaterialHub::getDriftSpeed(int iMat) const
{
    return 0.01 * Materials[iMat]->e_driftVelocity; //given in cm/us - returns in mm/ns
}

double AMaterialHub::getDiffusionSigmaTime(int iMat, double length_mm) const
{
    //sqrt(2Dl/v^3)
    //https://doi.org/10.1016/j.nima.2016.01.094
    const AMaterial * m = Materials[iMat];
    if (m->e_driftVelocity == 0 || m->e_diffusion_L == 0) return 0;

    const double v = 0.01 * m->e_driftVelocity; // in mm/ns <- from cm/us
    const double d = m->e_diffusion_L; //now in mm^2/ns

    return sqrt(2.0 * d * length_mm / v) / v; // in ns
}

double AMaterialHub::getDiffusionSigmaTransverse(int iMat, double length_mm) const
{
    //sqrt(2Dl/v)
    const AMaterial * m = Materials[iMat];
    if (m->e_driftVelocity == 0 || m->e_diffusion_L == 0) return 0;

    const double v = 0.01 * m->e_driftVelocity; // in mm/ns <- from cm/us
    const double d = m->e_diffusion_T; //now in mm^2/ns

    return sqrt(2.0 * d * length_mm / v); // in mm
}

void AMaterialHub::updateRuntimeProperties()
{
    for (AMaterial * mat : Materials) mat->updateRuntimeProperties();
}

QString AMaterialHub::getMaterialName(int matIndex) const
{
    if (matIndex < 0 || matIndex >= Materials.size()) return "";
    return Materials[matIndex]->name;
}

QStringList AMaterialHub::getListOfMaterialNames() const
{
    QStringList l;
    for (AMaterial * m : Materials) l << m->name;
    return l;
}

void AMaterialHub::generateGeoMedia()
{
    for (size_t iMat = 0; iMat < Materials.size(); iMat++)
    {
        AMaterial * mat = Materials[iMat];
        mat->generateTGeoMat();
        mat->GeoMed = new TGeoMedium(mat->name.toLocal8Bit().data(), iMat, mat->GeoMat);
    }
}

void AMaterialHub::clearMaterials()
{
    for (AMaterial * mat : Materials)
    {
        delete mat->PrimarySpectrumHist;
        delete mat->SecondarySpectrumHist;

        delete mat;
    }
    Materials.clear();
}

void AMaterialHub::addNewMaterial(bool fSuppressChangedSignal)
{
    AMaterial * m = new AMaterial;

    // !!!*** override handling?

    //appending to the material collection
    Materials.push_back(m);

    if (!fSuppressChangedSignal) emit materialsChanged();
}

void AMaterialHub::addNewMaterial(QString name, bool fSuppressChangedSignal)
{
    addNewMaterial(true);
    Materials.back()->name = name;
    ensureMatNameIsUnique(Materials.back());

    if (!fSuppressChangedSignal) emit materialsChanged();
}

void AMaterialHub::copyMaterialToTmp(int imat, AMaterial & tmpMaterial)
{
    if (imat < 0 || imat >= Materials.size())
    {
        qWarning()<<"Error: attempting to copy non-existent material #"<<imat<< " to tmpMaterial!";
        return;
    }

    //do not want to copy dynamic objects!
    QJsonObject js;
        Materials[imat]->writeToJson(js);
    tmpMaterial.readFromJson(js);
}

void AMaterialHub::copyTmpToMaterialCollection(const AMaterial &tmpMaterial)
{
    const QString name = tmpMaterial.name;
    int index = findMaterial(name);
    if (index == -1)
    {
        //      qDebug()<<"MaterialCollection--> New material: "<<name;
        addNewMaterial(true);
        index = Materials.size() - 1;
    }
    else
    {
        //      qDebug()<<"MaterialCollection--> Material "+name+" already defined; index = "<<index;
    }

    //do not want to copy dynamic objects!
    QJsonObject js;
    tmpMaterial.writeToJson(js);
    Materials[index]->readFromJson(js);

    emit materialsChanged();
}

int AMaterialHub::findMaterial(const QString & name) const
{
    const int size = Materials.size();

    for (int index = 0; index < size; index++)
        if (name == Materials[index]->name) return index;

    return -1;
}

QString AMaterialHub::CheckMaterial(const AMaterial* mat) const
{
    if (!mat) return "nullptr material";
    return mat->checkMaterial();
}

QString AMaterialHub::CheckMaterial(int iMat) const
{
    if (iMat<0 || iMat>=Materials.size()) return "Wrong material index: " + QString::number(iMat);
    return CheckMaterial(Materials[iMat]);
}

bool AMaterialHub::DeleteMaterial(int imat)
{
    int size = Materials.size();
    if (imat<0 || imat >= size)
    {
        qWarning()<<"Attempt to remove material with invalid index!";
        return false;
    }

    //clear dynamic properties of this material
    delete Materials[imat]->PrimarySpectrumHist;
    delete Materials[imat]->SecondarySpectrumHist;

    emit materialsChanged();

    return true;
}

void AMaterialHub::writeToJson(QJsonObject & json) const
{
    QJsonArray ar;
    for (const AMaterial * m : Materials)
    {
        QJsonObject js;
        m->writeToJson(js);
        ar.append(js);
    }

    json["Materials"] = ar;
}

QString AMaterialHub::readFromJson(const QJsonObject & json)
{
    QJsonArray ar;
    bool ok = jstools::parseJson(json, "Materials", ar);
    if (!ok) return "Json does not contain material hub settings";

    clearMaterials();

    for (int i=0; i<ar.size(); i++)
    {
        QJsonObject js = ar[i].toObject();
        addNewMaterial(true);
        Materials.back()->readFromJson(js);
    }

    if (Materials.empty())
    {
        addNewMaterial("Dummy"); //emits the signal!
        return "Materials are empty!";
    }
    else emit materialsChanged();

    return "";
}

void AMaterialHub::addNewMaterial(QJsonObject & json) //have to be sure json is indeed material properties!
{
    addNewMaterial();
    AMaterial * mat = Materials.back();
    mat->readFromJson(json);

    ensureMatNameIsUnique(mat);

    emit materialsChanged();
}

void AMaterialHub::ensureMatNameIsUnique(AMaterial * mat)
{
    QString name = mat->name;
    bool fFound;
    do
    {
        fFound = false;
        for (const AMaterial * m : Materials)
        {
            if (m == mat) continue;
            if (m->name == name)
            {
                fFound = true;
                name += "*";
                break;
            }
        }
    }
    while (fFound);
    mat->name = name;
}

void AMaterialHub::CheckReadyForGeant4Sim(QString & Errors, QString & Warnings, const AGeoObject * World) const
{
    for (int iM = 0; iM<Materials.size(); iM++)
    {
        if (!World->isMaterialInActiveUse(iM)) continue;

        const AMaterial * mat = Materials[iM];
        if (!mat->ChemicalComposition.isDefined())
            Errors += QString("\nComposition not defined for %1, while needed for tracking!\n").arg(mat->name);
    }
}
