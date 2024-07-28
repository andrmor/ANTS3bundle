#include "aconfig_si.h"
#include "aconfig.h"
#include "ageometryhub.h"
#include "ajsontools.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <QDebug>

AConfig_SI::AConfig_SI() :
    Config(AConfig::getInstance())
{
    Description = "Read/change/save/load configuration\nToggle 'Config' button to access the configuration tree";

    Help["load"]  = "Load configuration from file (.json).";
    Help["save"]  = "Save configuration to file (.json).";

    Help["getConfig"]  = "Get the entire configuration as an object";
    Help["setConfig"]  = "Set the entire configuration from an object";

    Help["replace"] = "Replace the value of the Key in the config with the new one. Key value can be basic types (bool, double, string) as well as arrays and objects.\n"
                      "Use rebuildDetector() method to take changes in effect!";
    Help["getKeyValue"] = "Get the value of the Key: it can be basic types (bool, double, string) as well as arrays and objects.";

    Help["rebuildDetector"]  = "Force to rebuild detector. It is necessary to take into account any changes introduced in the configuration!"; //\nNot anymore required after calling config.Replace - it is called automatically if Detector settings were modified.";
    Help["updateGui"] = "Update GUI during script execution according to current settings in config.";

}

bool AConfig_SI::load(QString FileName)
{
    if (!bGuiThread)
    {
        abort("Script in threads: cannot modify detector configuration!");
        return false;
    }

    QString err = Config.load(FileName, false);
    if (err.isEmpty()) return true;

    abort("Failed to load config from file " + FileName + " -->\n" + err);
    return false;
}

bool AConfig_SI::save(QString FileName)
{
    QString err = Config.save(FileName);
    if (err.isEmpty()) return true;
    abort("Failed to save config to file " + FileName + " -->\n" + err);
    return false;
}

QVariantMap AConfig_SI::getConfig() const
{
    return Config.JSON.toVariantMap();
}

bool AConfig_SI::setConfig(QVariantMap ConfigObject)
{
    if (!bGuiThread)
    {
        abort("Script in threads: cannot modify detector configuration!");
        return false;
    }

    QJsonObject json = QJsonObject::fromVariantMap(ConfigObject);

    QString err = Config.readFromJson(json, false);
    if (err.isEmpty()) return true;

    abort("Failed to set config from object:\n" + err);
    return false;
}

bool AConfig_SI::replace(QString Key, QVariant Value)
{
    if (!bGuiThread)
    {
        abort("Only GUI thread can modify detector configuration!");
        return false;
    }

    LastError = "";
    //qDebug() << Key << val << val.typeName();
    QString type = Value.typeName();

    QJsonValue jv;
    QString rep;
    if (type == "int")
    {
        jv = QJsonValue(Value.toInt());
        rep = QString::number(Value.toInt());
    }
    else if (type == "double")
    {
        jv = QJsonValue(Value.toDouble());
        rep = QString::number(Value.toDouble());
    }
    else if (type == "bool")
    {
        jv = QJsonValue(Value.toBool());
        rep = Value.toBool() ? "true" : "false";
    }
    else if (type == "QString")
    {
        jv = QJsonValue(Value.toString());
        rep = Value.toString();
    }
    else if (type == "QVariantList")
    {
        QVariantList vl = Value.toList();
        QJsonArray ar = QJsonArray::fromVariantList(vl);
        jv = ar;
        rep = "-Array-";
    }
    else if (type == "QVariantMap")
    {
        QVariantMap mp = Value.toMap();
        QJsonObject ob = QJsonObject::fromVariantMap(mp);
        jv = ob;
        rep = "-Object-";
    }
    else
    {
        abort("Wrong argument type (" + type + ") in Replace with the key line:\n" + Key);
        return false;
    }

    if (!expandKey(Key)) return false;

    bool ok = modifyJsonValue(Config.JSON, Key, jv);
    if (ok)
    {
        // not needed here as it was in ants2: user has to trigger updateConfig() and, optionally, updateGui()

/*
        //qDebug() << "-------Key:"<<Key;
        if (Key.startsWith("DetectorConfig")) //rebuild detector if detector settings were changed
            Config->GetDetector()->BuildDetector();
        else if (Key.startsWith("ReconstructionConfig.LRFmakeJson"))
            Config->AskForLRFGuiUpdate();
        else if (Key.startsWith("ReconstructionConfig"))
            Config->AskForReconstructionGuiUpdate();
        else if (Key.startsWith("SimulationConfig"))
            Config->AskForSimulationGuiUpdate();
*/

        return true;
    }

    abort("core.replace(" + Key + ", " + rep + ") format error:\n" + LastError);
    return false;
}

QVariant AConfig_SI::getKeyValue(QString Key)
{
    qDebug() << this << "get "<< Key << "triggered";

    LastError = "";

    if (!expandKey(Key)) return 0; //aborted anyway
    //qDebug() << "Key after expansion:"<<Key;

    QJsonObject obj = Config.JSON;
    int indexOfDot;
    QString path = Key;
    do
    {
        indexOfDot = path.indexOf('.');
        QString propertyName = path.left(indexOfDot);
        QString path1 = (indexOfDot>0 ? path.mid(indexOfDot+1) : QString());
        path = path1;
        //qDebug() << "property, path"<<propertyName<<path;

        QString name;
        std::vector<int> indexes;
        bool ok = keyToNameAndIndex(propertyName, name, indexes);
        if (!ok)
        {
            abort("Get key value for "+Key+" format error");
            return false;
        }
        propertyName = name;
        //qDebug() << "Attempting to extract:"<<propertyName<<indexes;
        QJsonValue subValue = obj[propertyName];
        //qDebug() << "QJsonValue extraction success?" << (subValue != QJsonValue());
        if (subValue == QJsonValue())
        {
            LastError = "Property not found:"+propertyName;
            qDebug() << LastError;
            return false;
        }

        //updating QJsonObject
        if (indexes.empty())
        { //not an array
            if (path.isEmpty())
            {
                //qDebug() << "QJsonValue to attempt to report back:"<<subValue;
                QVariant res = subValue.toVariant();
                //qDebug() << "QVariant:"<<res;
                return res;
            }
            else obj = subValue.toObject();
        }
        else
        { //it is an array
            //if path is not empty, subValue has to be either object or array
            QJsonArray arr = subValue.toArray();
            for (size_t i = 0; i < indexes.size(); i++)
            {
                int index = indexes.at(i);
                if (index < 0 || index > arr.size()-1)
                {
                    LastError = "Array index is out of bounds ("+QString::number(index)+") for property anme "+propertyName;
                    qDebug() << LastError;
                    return false;
                }
                if (i == indexes.size()-1)
                {
                    //it is the last index
                    if (path.isEmpty())
                    {
                        QJsonValue jv = arr[index];
                        QVariant res = jv.toVariant();
                        return res;
                    }
                    else obj = arr[index].toObject();
                }
                else arr = arr[index].toArray();
            }

            if (obj.isEmpty())
            {
                LastError = "Array element of "+propertyName+" is not QJsonObject!";
                qDebug() << LastError;
                return false;
            }
        }
    }
    while (indexOfDot > 0);

    abort("Get key value for " + Key + " error");
    return 0;
}

void AConfig_SI::updateConfig()
{
    if (!bGuiThread)
    {
        abort("Only GUI thread can modify detector configuration!");
        return;
    }

    QString err = Config.updateConfigFromJSON(false);
    if (!err.isEmpty()) abort("Error in configuration JSON:\n" + err);
}

/*
void AConfig_SI::updateGui()
{
    if (!bGuiThread) return;

    emit Config.configLoaded();
}
*/

bool AConfig_SI::keyToNameAndIndex(QString Key, QString & Name, std::vector<int> & Indexes)
{
    Indexes.clear();
    if (Key.contains('['))
    { // it is an array
        if (Key.startsWith('[') || !Key.endsWith(']'))
        {
            LastError = "Format error: "+Key;
            qDebug() << LastError;
            return false;
        }
        QStringList ArFields = Key.split('[');
        //qDebug() << ArFields;
        Name = ArFields.first();
        ArFields.removeFirst();
        for (int i=0; i<ArFields.size(); i++)
        {
            QString a = ArFields[i];
            if (!a.endsWith(']'))
            {
                LastError = "Format error: "+Key;
                qDebug() << LastError;
                return false;
            }
            a.chop(1);
            bool ok;
            int index = a.toInt(&ok);
            if (!ok)
            {
                LastError = "Format error: "+Key;
                qDebug() << LastError;
                return false;
            }
            Indexes.push_back(index);
        }
    }
    else // it is not an array
        Name = Key;

    //qDebug() << "Extracted Name/Indexes:"<<Name<<Indexes;
    return true;
}

bool AConfig_SI::modifyJsonValue(QJsonObject &obj, const QString &path, const QJsonValue &newValue)
{
    int indexOfDot = path.indexOf('.');
    QString propertyName = path.left(indexOfDot);
    QString subPath = indexOfDot>0 ? path.mid(indexOfDot+1) : QString();
    //qDebug() << "subPath:"<<subPath;

    QString name;
    std::vector<int> indexes;
    bool ok = keyToNameAndIndex(propertyName, name, indexes);
    if (!ok) return false;
    propertyName = name;

    //qDebug() << "Attempting to extract:"<<propertyName<<indexes;
    QJsonValue subValue = obj[propertyName];
    //qDebug() << "QJsonvalue extraction success?" << (subValue != QJsonValue());
    if (subValue == QJsonValue())
    {
        LastError = "Property not found:"+propertyName;
        qDebug() << LastError;
        return false;
    }

    //updating QJsonObject
    if(indexes.empty())
    { // it is an object
        if (subPath.isEmpty()) subValue = newValue;
        else
        {
            QJsonObject obj1;
            obj1 = subValue.toObject();
            bool ok = modifyJsonValue(obj1, subPath, newValue);
            if (!ok) return false;
            subValue = obj1;
        }
    }
    else
    { // it is an array
        QVector<QJsonArray> arrays;
        arrays << subValue.toArray();
        for (size_t i = 0; i < indexes.size()-1; i++)  //except the last one - it has special treatment
        {
            int index = indexes.at(i);
            if (index<0 || index>arrays.last().size()-1)
            {
                LastError = "Array index is out of bounds for property name "+propertyName;
                qDebug() << LastError;
                return false;
            }
            arrays << arrays.last()[index].toArray();
        }

        if (subPath.isEmpty())
        { //it is the last subkey
            arrays.last()[indexes.back()] = newValue;
            //propagating to parent arrays
            for (int i=arrays.size()-2; i>-1; i--) arrays[i][indexes[i]] = arrays[i+1];
            subValue = arrays.first();
        }
        else
        { //not the last subkey, so it should be an object
            if (indexes.back() >= arrays.last().size())
            {
                LastError = "Requested index in "+propertyName+" is larger than or equal to the container size";
                qDebug() << LastError;
                return false;
            }
            QJsonObject obj1 = arrays.last()[indexes.back()].toObject();
            if (obj1.isEmpty())
            {
                LastError = "Array element of "+propertyName+" is not QJsonObject!";
                qDebug() << LastError;
                return false;
            }
            bool ok = modifyJsonValue(obj1, subPath, newValue);
            if (!ok) return false;
            arrays.last()[indexes.back()] = obj1;
            //propagating to parent arrays
            for (int i=arrays.size()-2; i>-1; i--) arrays[i][indexes[i]] = arrays[i+1];
            subValue = arrays.first();
        }
    }

    obj[propertyName] = subValue;
    return true;
}

void AConfig_SI::find(const QJsonObject &obj, QStringList Keys, QStringList &Found, QString Path)
{
    // script interface replaces ".." or the leading "." with an empty string
    bool fLast = (Keys.size() == 1); //it is the last key in the Keys list
    if (fLast && Keys.last()=="")
    { //bad format - Keys have to end with a concrete key
        LastError = "Bad format - Last object has to be concrete";
        qDebug() << LastError;
        return;
    }

    //qDebug() << "find1 triggered for:"<<Keys;
    QString Key = Keys.first();
    QString Name;
    std::vector<int> indexes;
    if (Key != "")
    { //looking for a concrete key
        if (!keyToNameAndIndex(Key, Name, indexes)) return; //format error

        if (obj.contains(Name))
        { //object does contain the key name! (we do not check here if the index is adequate)
            Path += "." + Key;
            if (fLast)
            { //mission accomplished
                Found.append(Path.mid(1)); //remove the starting "." in the found path
                return;
            }
            //pass to the next key in the chain
            Keys.removeFirst();
            if (indexes.empty())
                find(obj[Name].toObject(), Keys, Found, Path);
            else
            {
                QJsonArray ar = obj[Name].toArray();
                for (size_t i = 0; i < indexes.size()-1; i++) ar = ar[indexes[i]].toArray();
                QJsonObject arob = ar[indexes.back()].toObject();
                find(arob, Keys, Found, Path);
            }
        }
        else return; //does not contains the given key
    }
    else
    { // "" Key
        QString Key = Keys.at(1); //this is next key to find
        if (Key == "") return;  //format problem - cannot be "" followed by ""
        //does the object contain the next key?
        if (!keyToNameAndIndex(Key, Name, indexes)) return; //format error
        if  (obj.contains(Name))
        { //object does contain the key!
            //we can reuse the function:
            Keys.removeFirst(); //remove ""
            find(obj, Keys, Found, Path);
        }
        else
        { //have to check every sub-object
            foreach(QString oneKey, obj.keys())
            {
                QJsonValue Val = obj[oneKey];
                if (Val.isObject())
                    find(obj[oneKey].toObject(), Keys, Found, Path+"."+oneKey);
                else if (Val.isArray())
                {
                    QJsonArray arr = Val.toArray();
                    for (int i=0; i<arr.size(); i++)
                        find(arr[i].toObject(), Keys, Found, Path+"."+oneKey+"["+QString::number(i)+"]");
                }
                //else do nothing for other types
            }
        }
    }
}

bool AConfig_SI::expandKey(QString &Key)
{
    if (Key.startsWith(".") || Key.contains(".."))
    {
        QStringList keys = Key.split(".");
        //qDebug() << keys;
        QStringList res;
        find(Config.JSON, keys, res);

        if (!LastError.isEmpty())
        {
            abort("Search error for config key "+Key+":" + LastError);
            return false;
        }
        if (res.isEmpty())
        {
            abort("Search error for config key "+Key+":" +
                       "Not found config object according to this key");
            return false;
        }
        if (res.size()>1)
        {
            abort("Search error for config key "+Key+":" +
                       "Match is not unique!");
            return false;
        }

        Key = res.first();
        //qDebug() << "*-----------"<<res;
    }

    return true;
}
