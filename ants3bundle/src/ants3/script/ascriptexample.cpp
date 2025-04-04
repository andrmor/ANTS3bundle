#include "ascriptexample.h"

#include <QDebug>

bool AScriptExample::readFromRecord(QString text)
{
    ErrorString.clear();

    text.remove("\r");  // Windows artefact :)
    QStringList list = text.split("#");

    for (int i = 0; i < list.size(); i++)
    {
        QString & r = list[i];
        if (r.startsWith("file"))
        {
            QStringList fi = r.split("\n");            
            if (fi.size() < 2)
            {
                ErrorString = "Bad format in file name record";
                qWarning() << ErrorString;
                return false;
            }
            FileName = fi.at(1);           
        }
        else if (r.startsWith("tags"))
        {
            QStringList t1 = r.split("\n");
            if (t1.size() > 0)
            {
                QStringList t2 = t1[1].split(":");
                for (int j = 0; j < t2.size(); j++)
                    Tags << t2.at(j);
            }
        }
        else if (r.startsWith("info"))
        {
            r.remove("info\n");
            Description = r;
        }
    }

    return (!FileName.isEmpty());
}
