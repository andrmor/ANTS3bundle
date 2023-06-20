#include "aconfigexamplebrowser.h"
#include "ui_aconfigexamplebrowser.h"
#include "afiletools.h"
#include "guitools.h"

AConfigExampleBranch::~AConfigExampleBranch()
{
    clear();
}

void AConfigExampleBranch::clear()
{
    Title.clear();
    Items.clear();

    ParentBranch = nullptr;
    for (AConfigExampleBranch * b : SubBranches) delete b;
    SubBranches.clear();
}

AConfigExampleBrowser::AConfigExampleBrowser(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AConfigExampleBrowser)
{
    ui->setupUi(this);

    setWindowTitle("Config example browser");
}

AConfigExampleBrowser::~AConfigExampleBrowser()
{
    delete ui;
}

void AConfigExampleBrowser::on_pbReadDatabase_clicked()
{
    QString fn = "/home/andr/tmp/ConfigExamples.txt";
    QString err = readDatabase(fn);
    if (!err.isEmpty())
    {
        guitools::message(err);
        return;
    }


}

QString AConfigExampleBrowser::readDatabase(QString fileName)
{
    QString txt;
    bool ok = ftools::loadTextFromFile(txt, fileName);
    if (!ok) return "Failed to read database file: " + fileName;

    MainBranch.clear();
    AConfigExampleBranch * currentBranch = &MainBranch;
    int currentLevel = 1; // '*' --> 1; '**' --> 2 etc

    const QStringList sl = txt.split('\n');
    for (const QString & str : sl)
    {
        QString line = str.simplified();
        if (line.isEmpty()) continue;
        if (!line.startsWith('*') && !line.startsWith('@')) return "Format error in database file: records should start with * or @ symbol";

        if (line.startsWith('@'))
        {
            QString err = extractItem(line, currentBranch);
            if (!err.isEmpty()) return err;
        }

        if (line.startsWith('*'))
        {
            QString err = extractBranch(line, currentBranch, currentLevel);
            if (!err.isEmpty()) return err;
        }
    }

    return "";
}

QString AConfigExampleBrowser::extractItem(const QString &line, AConfigExampleBranch * currentBranch)
{
    int iChar = 1;
    while (iChar < line.size() && line[iChar] == ' ') iChar++;
    if (iChar >= line.size()) return "Cannot find filename in record: " + line;

    AConfigExampleItem item;

    while (iChar < line.size() && line[iChar] != ' ')
    {
        item.FileName += line[iChar];
        iChar++;
    }

    iChar++;
    if (iChar >= line.size()) return "Cannot find example description in record: " + line;

    while (iChar < line.size() && line[iChar] != '#')
    {
        item.Description += line[iChar];
        iChar++;
    }

    if (item.Description.simplified().isEmpty()) return "Cannot find example description in record: " + line;

    while (iChar < line.size())
        if (line[iChar] == '#')
        {
            QString tag;
            iChar++;
            while (iChar < line.size() && line[iChar] != '#')
            {
                tag += line[iChar];
                iChar++;
            }
            if (tag.endsWith(' ')) tag.chop(1);
            if (!tag.isEmpty()) item.Tags.push_back(tag); // !!!*** check uniqness?
        }

    qDebug() << item.FileName << item.Description << item.Tags;
    currentBranch->Items.push_back(item);

    return "";
}

QString AConfigExampleBrowser::extractBranch(const QString & line, AConfigExampleBranch * currentBranch, int & currentlevel)
{


    return "";
}

void AConfigExampleBrowser::on_pbLoadExample_clicked()
{

}

