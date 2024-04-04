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

void AScriptExampleDatabase::select(const QStringList & tags)
{
    if (tags.isEmpty())
    {
        unselectAll();
        return;
    }

    for (auto & example : Examples)
    {
        example.Selected = false;
        for (const auto & tag : tags)
            if (example.Tags.contains(tag))
            {
                example.Selected = true;
                break;
            }
    }
}

void AScriptExampleDatabase::unselectAll()
{
    for (auto & r : Examples) r.Selected = false;
}

int AScriptExampleDatabase::size() const
{
    return Examples.size();
}

int AScriptExampleDatabase::find(const QString & fileName) const
{
    for (int i = 0; i < Examples.size(); i++)
    {
        if (fileName == Examples[i].FileName)
            return i;
    }
    return -1;
}
