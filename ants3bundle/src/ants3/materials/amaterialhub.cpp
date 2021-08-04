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
    return 0.01 * Materials.at(iMat)->e_driftVelocity; //given in cm/us - returns in mm/ns
}

double AMaterialHub::getDiffusionSigmaTime(int iMat, double length_mm) const
{
    //sqrt(2Dl/v^3)
    //https://doi.org/10.1016/j.nima.2016.01.094
    const AMaterial * m = Materials.at(iMat);
    if (m->e_driftVelocity == 0 || m->e_diffusion_L == 0) return 0;

    const double v = 0.01 * m->e_driftVelocity; // in mm/ns <- from cm/us
    const double d = m->e_diffusion_L; //now in mm^2/ns

    return sqrt(2.0 * d * length_mm / v) / v; // in ns
}

double AMaterialHub::getDiffusionSigmaTransverse(int iMat, double length_mm) const
{
    //sqrt(2Dl/v)
    const AMaterial * m = Materials.at(iMat);
    if (m->e_driftVelocity == 0 || m->e_diffusion_L == 0) return 0;

    const double v = 0.01 * m->e_driftVelocity; // in mm/ns <- from cm/us
    const double d = m->e_diffusion_T; //now in mm^2/ns

    return sqrt(2.0 * d * length_mm / v); // in mm
}

/*
void AMaterialHub::updateRuntimeProperties()
{
    for (int imat = 0; imat < Materials.size(); imat++)
    {
        UpdateWaveResolvedProperties(imat);
        Materials[imat]->updateRuntimeProperties();
    }
}
*/

QString AMaterialHub::getMaterialName(int matIndex) const
{
    if (matIndex<0 || matIndex >= Materials.size()) return "";
    return Materials.at(matIndex)->name;
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

/*
void A3MatHub::UpdateWaveResolvedProperties(int imat)
{
    //qDebug()<<"Wavelength-resolved?"<<WavelengthResolved;
    //qDebug()<<"--updating wavelength-resolved properties for material index"<<imat;
    if (WavelengthResolved)
    {
        //calculating histograms and "-Binned" for effective data
        Materials[imat]->nWaveBinned.clear();
        if (Materials[imat]->nWave_lambda.size() > 0)
            ConvertToStandardWavelengthes(&Materials[imat]->nWave_lambda, &Materials[imat]->nWave, &Materials[imat]->nWaveBinned);

        Materials[imat]->absWaveBinned.clear();
        if (Materials[imat]->absWave_lambda.size() > 0)
            ConvertToStandardWavelengthes(&Materials[imat]->absWave_lambda, &Materials[imat]->absWave, &Materials[imat]->absWaveBinned);

        Materials[imat]->reemissionProbBinned.clear();
        if (Materials[imat]->reemisProbWave_lambda.size() > 0)
            ConvertToStandardWavelengthes(&Materials[imat]->reemisProbWave_lambda, &Materials[imat]->reemisProbWave, &Materials[imat]->reemissionProbBinned);

        if (Materials[imat]->rayleighMFP != 0)
        {
            Materials[imat]->rayleighBinned.clear();
            double baseWave4 = Materials[imat]->rayleighWave * Materials[imat]->rayleighWave * Materials[imat]->rayleighWave * Materials[imat]->rayleighWave;
            double base = Materials[imat]->rayleighMFP / baseWave4;
            for (int i=0; i<WaveNodes; i++)
            {
                double wave = WaveFrom + WaveStep*i;
                double wave4 = wave*wave*wave*wave;
                Materials[imat]->rayleighBinned.append(base * wave4);
            }
        }

        if (Materials[imat]->PrimarySpectrumHist)
        {
            delete Materials[imat]->PrimarySpectrumHist;
            Materials[imat]->PrimarySpectrumHist = 0;
        }
        if (Materials[imat]->PrimarySpectrum_lambda.size() > 0)
        {
            QVector<double> y;
            ConvertToStandardWavelengthes(&Materials[imat]->PrimarySpectrum_lambda, &Materials[imat]->PrimarySpectrum, &y);
            TString name = "PrimScSp";
            name += imat;
            Materials[imat]->PrimarySpectrumHist = new TH1D(name,"Primary scintillation", WaveNodes, WaveFrom, WaveTo);
            for (int j = 1; j<WaveNodes+1; j++)  Materials[imat]->PrimarySpectrumHist->SetBinContent(j, y[j-1]);
            Materials[imat]->PrimarySpectrumHist->GetIntegral(); //to make thread safe
        }

        if (Materials[imat]->SecondarySpectrumHist)
        {
            delete Materials[imat]->SecondarySpectrumHist;
            Materials[imat]->SecondarySpectrumHist = 0;
        }
        if (Materials[imat]->SecondarySpectrum_lambda.size() > 0)
        {
            QVector<double> y;
            ConvertToStandardWavelengthes(&Materials[imat]->SecondarySpectrum_lambda, &Materials[imat]->SecondarySpectrum, &y);
            TString name = "SecScSp";
            name += imat;
            Materials[imat]->SecondarySpectrumHist = new TH1D(name,"Secondary scintillation", WaveNodes, WaveFrom, WaveTo);
            for (int j = 1; j<WaveNodes+1; j++)  Materials[imat]->SecondarySpectrumHist->SetBinContent(j, y[j-1]);
            Materials[imat]->SecondarySpectrumHist->GetIntegral(); //to make thread safe
        }
    }
    else
    {
        //making empty histograms and "-Binned" for effective data
        Materials[imat]->nWaveBinned.clear();
        Materials[imat]->absWaveBinned.clear();
        Materials[imat]->reemissionProbBinned.clear();
        Materials[imat]->rayleighBinned.clear();
        if (Materials[imat]->PrimarySpectrumHist)
        {
            delete Materials[imat]->PrimarySpectrumHist;
            Materials[imat]->PrimarySpectrumHist = 0;
        }
        if (Materials[imat]->SecondarySpectrumHist)
        {
            delete Materials[imat]->SecondarySpectrumHist;
            Materials[imat]->SecondarySpectrumHist = 0;
        }
    }
}

void A3MatHub::ConvertToStandardWavelengthes(QVector<double>* sp_x, QVector<double>* sp_y, QVector<double>* y)
{
    y->resize(0);

    //qDebug()<<"Data range:"<<sp_x->at(0)<<sp_x->at(sp_x->size()-1);
    double xx, yy;
    for (int i=0; i<WaveNodes; i++)
    {
        xx = WaveFrom + WaveStep*i;
        if (xx <= sp_x->at(0)) yy = sp_y->at(0);
        else
        {
            if (xx >= sp_x->at(sp_x->size()-1)) yy = sp_y->at(sp_x->size()-1);
            else
            {
                //general case
                yy = GetInterpolatedValue(xx, sp_x, sp_y); //reusing interpolation function from functions.h
                if (yy<0) yy = 0; //!!! protection against negative
            }
        }
        //      qDebug()<<xx<<yy;
        y->append(yy);
    }
}
*/

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

        const AMaterial * mat = Materials.at(iM);
        if (!mat->ChemicalComposition.isDefined())
            Errors += QString("\nComposition not defined for %1, while needed for tracking!\n").arg(mat->name);
    }
}
