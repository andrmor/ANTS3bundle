#include "ageobasedelegate.h"
#include "aonelinetextedit.h"
#include "ageoconsts.h"
#include "ageoobject.h"

#include <QDebug>
#include <QCompleter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QRegularExpression>

AGeoBaseDelegate::AGeoBaseDelegate(QWidget *ParentWidget) :
    ParentWidget(ParentWidget) {}

bool AGeoBaseDelegate::isLeEmpty(const std::vector<AOneLineTextEdit*> & v) const
{
    for (AOneLineTextEdit * a : v)
        if (a->text().isEmpty()) return true;

    return false;
}

void AGeoBaseDelegate::postUpdate()
{
    if (frBottomButtons) frBottomButtons->setEnabled(true);
}

void AGeoBaseDelegate::createBottomButtons()
{
    frBottomButtons = new QFrame();
    QHBoxLayout * abl = new QHBoxLayout(frBottomButtons);
    abl->setContentsMargins(0,0,0,0);

    pbShow = new QPushButton("Show");
    QObject::connect(pbShow, &QPushButton::clicked, this, &AGeoBaseDelegate::RequestShow);
    abl->addWidget(pbShow);
    pbChangeAtt = new QPushButton("Color/line");
    QObject::connect(pbChangeAtt, &QPushButton::clicked, this, &AGeoBaseDelegate::RequestChangeVisAttributes);
    abl->addWidget(pbChangeAtt);

    frLineColor = new QFrame();
    frLineColor->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    frLineColor->setMaximumWidth(3);
    frLineColor->setMinimumWidth(3);
    frLineColor->setVisible(false);
    abl->addWidget(frLineColor);

    pbScriptLine = new QPushButton("Script to clipboard");
    pbScriptLine->setToolTip("Click right mouse button to generate script recursively");
    QObject::connect(pbScriptLine, &QPushButton::clicked, this, &AGeoBaseDelegate::RequestScriptToClipboard);
    pbScriptLine->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(pbScriptLine, &QPushButton::customContextMenuRequested, this, &AGeoBaseDelegate::RequestScriptRecursiveToClipboard);
    abl->addWidget(pbScriptLine);
}

#include "TColor.h"
#include "TROOT.h"
void AGeoBaseDelegate::updateLineColorFrame(const AGeoObject * obj)
{
    if (!obj) return;
    TColor * tc = gROOT->GetColor(obj->color);
    if (!tc) return;
    int red   = 255 * tc->GetRed();
    int green = 255 * tc->GetGreen();
    int blue  = 255 * tc->GetBlue();
    frLineColor->setStyleSheet(QString("background-color: rgb(%0,%1,%2)").arg(red).arg(green).arg(blue));
    frLineColor->setVisible(true);
}

void AGeoBaseDelegate::onContentChangedBase()
{
    if (frBottomButtons) frBottomButtons->setEnabled(false);
    emit contentChanged();
}

void AGeoBaseDelegate::configureHighligherAndCompleter(AOneLineTextEdit * edit, int iUntilIndex)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    int numConsts = GC.countConstants();
    if (iUntilIndex == -1 || iUntilIndex > numConsts) iUntilIndex = numConsts;

    ABasicHighlighter * highlighter = new ABasicHighlighter(edit->document());

    QTextCharFormat GeoConstantFormat;
    GeoConstantFormat.setForeground(Qt::darkMagenta);
    //GeoConstantFormat.setFontWeight(QFont::Bold);

    QStringList sl;
    AHighlightingRule rule;
    for (int i = 0; i < iUntilIndex; i++)
    {
        const QString & name = GC.getName(i);
        if (name.isEmpty()) continue;

        rule.pattern = QRegularExpression("\\b" + name + "\\b");
        rule.format = GeoConstantFormat;
        highlighter->HighlightingRules.push_back(rule);
        sl << name;
    }

    rule.pattern = QRegularExpression("\\bParentIndex\\b");
    rule.format = GeoConstantFormat;
    highlighter->HighlightingRules.push_back(rule);
    sl << "ParentIndex";

    QTextCharFormat FormulaFormat;
    FormulaFormat.setForeground(Qt::blue);
    //GeoConstantFormat.setFontWeight(QFont::Bold);

    const std::vector<QString> & words = AGeoConsts::getConstInstance().getTFormulaReservedWords();
    for (const QString & word : words)
    {
        rule.pattern = QRegularExpression("\\b" + word + "\\b");
        rule.format = FormulaFormat;
        highlighter->HighlightingRules.push_back(rule);
    }

    edit->Completer = new QCompleter(sl, edit);
    edit->Completer->setCaseSensitivity(Qt::CaseInsensitive); //Qt::CaseSensitive
    edit->Completer->setCompletionMode(QCompleter::PopupCompletion); //QCompleter::UnfilteredPopupCompletion
    edit->Completer->setFilterMode(Qt::MatchContains); //Qt::MatchStartsWith
    edit->Completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel); //QCompleter::CaseSensitivelySortedModel
    edit->Completer->setWrapAround(false);

    edit->Completer->setWidget(edit);
    QObject::connect(edit->Completer, SIGNAL(activated(QString)), edit, SLOT(insertCompletion(QString)));
}

bool AGeoBaseDelegate::processEditBox(const QString & whatIsIt, AOneLineTextEdit *lineEdit, double &val, QString &str, QWidget *parent)
{
    str = lineEdit->text();
    if (str.isEmpty())
    {
        QMessageBox::warning(parent, "", "Empty line in " + whatIsIt);
        return false;
    }

    const AGeoConsts & GC = AGeoConsts::getConstInstance();
    QString errorStr;
    bool ok = GC.updateDoubleParameter(errorStr, str, val, false, false, false);
    if (ok) return true;
    QMessageBox::warning(parent, "", errorStr + " in " + whatIsIt);
    return false;
}
