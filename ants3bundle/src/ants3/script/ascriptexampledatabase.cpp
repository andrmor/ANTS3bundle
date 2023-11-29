#include "ascriptexampledatabase.h"

#include <QDebug>

AScriptExampleDatabase::AScriptExampleDatabase(QString configText)
{
    QStringList list = configText.split("#end");

    for (int i = 0; i < list.size(); i++)
    {
        AScriptExample example;
        if (example.readFromRecord(list.at(i)))
        {
            Examples.append(example);
            for (int j = 0; j < example.Tags.size(); j++)
                if (!Tags.contains(example.Tags.at(j))) Tags.append(example.Tags.at(j));
        }
    }

    //qDebug() << "Extracted"<<Examples.size()<<"examples";
    Tags.sort();
}

void AScriptExampleDatabase::select(QStringList tags)
{
    if (tags.isEmpty())
    {
        unselectAll();
        return;
    }

    for (int i = 0; i < Examples.size(); i++)
    {
        Examples[i].Selected = false;
        for (int j = 0; j < tags.size(); j++)
        {
            if (Examples.at(i).Tags.contains(tags.at(j)))
            {
                Examples[i].Selected = true;
                continue;
            }
        }
    }
}

void AScriptExampleDatabase::unselectAll()
{
    for (int i = 0; i < Examples.size(); i++)
        Examples[i].Selected = false;
}

int AScriptExampleDatabase::find(QString fileName)
{
    for (int i = 0; i < Examples.size(); i++)
    {
        if (Examples.at(i).FileName == fileName)
            return i;
    }
    return -1;
}
