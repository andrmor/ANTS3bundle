#include "ainspector.h"
#include "ajsontools.h"

#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"

#ifdef ANTS3_NCRYSTAL
#include "G4NCrystal/G4NCrystal.hh"
#endif

#include <QDebug>

AInspector::AInspector(const QString & dir, const QString & fileName) :
    WorkingDir(dir), RequestFileName(fileName) {}

void AInspector::start()
{
    const QString fn = WorkingDir + '/' + RequestFileName;
    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, fn);
    if (!ok) terminate("Cannot load request from file:" + fn);

    processRequest(json);

    exit(0);
}

void AInspector::terminate(const QString & returnMessage)
{
    qDebug() << "$$>" << returnMessage;

    ErrorMessage = returnMessage;
    generateResponseFile(QJsonObject());

    exit(1);
}

void AInspector::generateResponseFile(const QJsonObject & responseJson)
{
    QJsonObject json;

    bool bSuccess = ErrorMessage.isEmpty();
    json["Success"] = bSuccess;
    if (bSuccess) json["Response"] = responseJson;
    else          json["Error"]    = ErrorMessage;

    jstools::saveJsonToFile(json, WorkingDir + "/response.json");
}

#include "G4Version.hh"
void AInspector::processRequest(const QJsonObject & json)
{
    QString request;
    jstools::parseJson(json, "Request", request);

    if (request == "MaterialComposition")
    {
        QString matName;
        jstools::parseJson(json, "MaterialName", matName);
        if (matName.isEmpty()) terminate("Empty material name");

        //qDebug() << "...Mat name:" << matName;
        G4NistManager * matManager = G4NistManager::Instance();
        G4Material * mat = matManager->FindOrBuildMaterial(matName.toLatin1().data());
        if (!mat)
        {
            terminate("Material " + matName + " is not listed in G4NistManager");
            return;
        }

        QJsonObject json;
        fillMaterialComposition(mat, json);
        generateResponseFile(json);
    }
    #ifdef ANTS3_NCRYSTAL
    else if (request == "MaterialCompositionNCrystal")
    {
        QString matName;
        jstools::parseJson(json, "MaterialName", matName);
        if (matName.isEmpty()) terminate("Empty material name");

        G4Material * mat = G4NCrystal::createMaterial(matName.toLatin1().data());
        if (!mat)
        {
            terminate("Unknown generation string for NCrystal material: " + matName);
            return;
        }

        QJsonObject json;
        fillMaterialComposition(mat, json);
        generateResponseFile(json);
    }
    #endif
    else if (request == "Geant4Version")
    {
        QJsonObject json;
        QString str = G4Version.data();
        str.remove("$");
        str.remove("Name:");
        str.remove("geant4-");
        str = str.simplified();
        json["Version"] = str;
        generateResponseFile(json);
    }
    else terminate("Unknown request");
}

void AInspector::fillMaterialComposition(G4Material * mat, QJsonObject & json)
{
    json["Name"]    = QString(mat->GetName().data());
    json["Density"] = mat->GetDensity()/(g/cm3);
    json["Formula"] = QString(mat->GetChemicalFormula().data());
    json["Temperature"] = mat->GetTemperature();
    json["MeanExcitationEnergy"] = mat->GetIonisation()->GetMeanExcitationEnergy()/eV;

    int numEl = mat->GetNumberOfElements();
    const G4double * fractions = mat->GetFractionVector();
    const G4int * atoms = mat->GetAtomsVector();
    QString compFr, compAt;
    bool bHasAtomic = true;

    // cannot say if atomic fractions are real (they can be reported as 1 even if they are not actually configured)
    double A = 0;
    for (int i = 0; i < numEl; i++) A += mat->GetElement(i)->GetA() * atoms[i];

    for (int i = 0; i < numEl; i++)
    {
        QString elName = QString(mat->GetElement(i)->GetName().data());
        double weightFr = fractions[i];
        compFr += elName + '/' + QString::number(weightFr) + " + ";

        if (bHasAtomic)
        {
            int atomFr = atoms[i];
            if (atomFr == 0 || weightFr != atomFr * mat->GetElement(i)->GetA() / A)
            {
                bHasAtomic = false;
                continue;
            }
            compAt += elName + ':' + QString::number(atomFr) + " + ";
        }
        //qDebug() << elName << weightFr << atoms[i] << atoms[i] * mat->GetElement(i)->GetA() / A << (weightFr == atoms[i] * mat->GetElement(i)->GetA() / A);
    }

    compFr.chop(3);
    json["WeightFractions"] = compFr;
    if (bHasAtomic)
    {
        compAt.chop(3);
        json["AtomFractions"] = compAt;
    }

    //qDebug() << "Mean exc en:" << mat->GetIonisation()->GetMeanExcitationEnergy()/eV;
}
