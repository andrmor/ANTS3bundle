#include "ainspector.h"
#include "ajsontools.h"

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

void AInspector::processRequest(const QJsonObject & json)
{
    QString request;
    jstools::parseJson(json, "Request", request);

    if (request == "MaterialComposition")
    {
        QString matName;
        jstools::parseJson(json, "MaterialName", matName);
        if (matName.isEmpty()) terminate("Empty material name");

        QJsonObject json;
        fillMaterialComposition(matName, json);
        generateResponseFile(json);
    }
    else terminate("Unknown request");
}

#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
void AInspector::fillMaterialComposition(const QString & matName, QJsonObject & json)
{
    qDebug() << "...Mat name:" << matName;

    G4NistManager * matManager = G4NistManager::Instance();

    G4Material * mat = matManager->FindOrBuildMaterial(matName.toLatin1().data());
    if (!mat)
    {
        terminate("Material " + matName + " is not listed in G4NistManager");
        return;
    }

    json["Name"]    = QString(mat->GetName().data());
    json["Density"] = mat->GetDensity()/(g/cm3);
    json["Formula"] = QString(mat->GetChemicalFormula().data());

    int numEl = mat->GetNumberOfElements();
    const G4double * fractions = mat->GetFractionVector();
    const G4int * atoms = mat->GetAtomsVector();
    QString compFr, compAt;
    bool bHasAtomic = true;
    for (int i = 0; i < numEl; i++)
    {
        QString elName = QString(mat->GetElement(i)->GetName().data());
        double weightFr = fractions[i];
        compFr += elName + '/' + QString::number(weightFr) + " + ";
        if (bHasAtomic)
        {
            int atomFr = atoms[i];
            if (atomFr == 0)
            {
                bHasAtomic = false;
                continue;
            }
            compAt += elName + ':' + QString::number(atomFr) + " + ";
        }
    }
    compFr.chop(3);
    json["WeightFractions"] = compFr;
    if (bHasAtomic)
    {
        compAt.chop(3);
        json["AtomFractions"] = compAt;
    }
}
