#include "a3optinthub.h"

A3OptIntHub::A3OptIntHub()
{

}

A3OptIntHub &A3OptIntHub::getInstance()
{
    static A3OptIntHub instance;
    return instance;
}

const A3OptIntHub &A3OptIntHub::getConstInstance()
{
    return getInstance();
}

void A3OptIntHub::updateWaveResolvedProperties()
{
    /*
    for (int ior=0; ior<Materials[imat]->OpticalOverrides.size(); ior++)
        if (Materials[imat]->OpticalOverrides[ior])
            Materials[imat]->OpticalOverrides[ior]->initializeWaveResolved();
    */
}

void A3OptIntHub::writeToJson(QJsonObject &json) const
{
    /*
    QJsonArray oar;
    for (size_t iFrom=0; iFrom<Materials.size(); iFrom++)
        for (size_t iTo=0; iTo<Materials.size(); iTo++)
        {
            if ( !Materials.at(iFrom)->OpticalOverrides.at(iTo) ) continue;
            QJsonObject js;
            Materials.at(iFrom)->OpticalOverrides[iTo]->writeToJson(js);
            oar.append(js);
        }
    if (!oar.isEmpty()) js["Overrides"] = oar;
    */
}

void A3OptIntHub::readFromJson(const QJsonObject &json)
{
    /*
    //reading overrides if present
    QJsonArray oar = js["Overrides"].toArray();
    for (int i=0; i<oar.size(); i++)
    {
        QJsonObject jj = oar[i].toObject();
        if (jj.contains("Model"))
        {
            //new format
            QString model = jj["Model"].toString();
            int MatFrom = jj["MatFrom"].toInt();
            int MatTo = jj["MatTo"].toInt();
            if (MatFrom>numMats-1 || MatTo>numMats-1)
            {
                qWarning()<<"Attempt to override for non-existent material skipped";
                continue;
            }
            AOpticalOverride* ov = OpticalOverrideFactory(model, this, MatFrom, MatTo);
            if (!ov || !ov->readFromJson(jj))
                qWarning() << Materials[MatFrom]->name  << ": optical override load failed!";
            else Materials[MatFrom]->OpticalOverrides[MatTo] = ov;
        }
    }
    */
}

QString A3OptIntHub::checkOverrides()
{
    /*
    for (const AMaterial * mat : Materials)
        for (AOpticalOverride * ov : qAsConst(mat->OpticalOverrides))
            if (ov)
            {
                QString err = ov->checkOverrideData();
                if ( !err.isEmpty())
                    return QString("Error in optical override from %1 to %2:\n").arg(mat->name, getMaterialName(ov->getMaterialTo())) + err;
            }
    */
    return "";
}

void A3OptIntHub::onNewMaterialAdded()
{

}

void A3OptIntHub::onMaterialDeleted(size_t iMat)
{
    /*
    //clear overrides from other materials to this one
    for (int iOther=0; iOther<size; iOther++)
    {
        delete Materials[iOther]->OpticalOverrides[imat];
        //Materials[iOther]->OpticalOverrides.remove(imat);
        Materials[iOther]->OpticalOverrides.erase( std::next(Materials[iOther]->OpticalOverrides.begin(), imat) );
    }

    //delete this material
    delete Materials[imat];
    //Materials.remove(imat);
    Materials.erase( std::next(Materials.begin(), imat) );

    //update indices of override materials
    for (int i=0; i<size-1; i++)
        for (int j=0; j<size-1; j++)
            if (Materials[i]->OpticalOverrides[j])
                Materials[i]->OpticalOverrides[j]->updateMatIndices(i, j);
    */
}

