#include "ageoconstexpressiondialog.h"
#include "a3geoconwin.h"
#include "aonelinetextedit.h"
#include "ageobasedelegate.h"
#include "ageoconsts.h"
#include "guitools.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

AGeoConstExpressionDialog::AGeoConstExpressionDialog(A3GeoConWin * geoConW, int index) :
    QDialog(geoConW), GW(geoConW), Index(index)
{
    AGeoConsts & GC = AGeoConsts::getInstance();

    setWindowTitle(QString("Expression for constant:  %0").arg(GC.getName(index)));
    resize(500, 125);

    QVBoxLayout * lMain = new QVBoxLayout(this);

        lMain->addStretch();

        lMain->addWidget(new QLabel("Use TFormula syntax and constants defined above this one"), 0, Qt::AlignHCenter);

        OriginalText = AGeoConsts::getInstance().getExpression(index);
        ed = new AOneLineTextEdit(OriginalText);
        //ed->setMinimumHeight(30);
        AGeoBaseDelegate::configureHighligherAndCompleter(ed, Index);
        connect(ed, &AOneLineTextEdit::enterPressed, this, &AGeoConstExpressionDialog::onAcceptPressed);
        lMain->addWidget(ed);

        lMain->addStretch();

        QHBoxLayout * lBut = new QHBoxLayout();
            QPushButton * pbAccept = new QPushButton("Accept");
            connect(pbAccept, &QPushButton::clicked, this, &AGeoConstExpressionDialog::onAcceptPressed);
            lBut->addWidget(pbAccept);
            QPushButton * pbCancel = new QPushButton("Cancel");
            connect(pbCancel, &QPushButton::clicked, this, &AGeoConstExpressionDialog::onCancelPressed);
            lBut->addWidget(pbCancel);
        lMain->addLayout(lBut);

    pbAccept->setDefault(true);
}

#include "aerrorhub.h"
void AGeoConstExpressionDialog::onAcceptPressed()
{
    AErrorHub::clear();

    AGeoConsts & GC = AGeoConsts::getInstance();

    QString newText = ed->text();
    bool ok;
    double newVal = newText.toDouble(&ok);
    if (ok)
    {
        GC.setNewExpression(Index, "");
        GC.setNewValue(Index, newVal);
        accept();
        return;
    }

    /*  // disabled as misbehave in situations when bad syntax is loaded from file
    if (newText == GC.getExpression(Index))
    {
        reject();
        return;
    }
    */

    QString errorStr = GC.setNewExpression(Index, newText);

    if (!errorStr.isEmpty())
    {
        blockSignals(true);
        GC.setNewExpression(Index, OriginalText);
        guitools::message(errorStr, this);
        blockSignals(false);
        return;
    }

    accept();
}

void AGeoConstExpressionDialog::onCancelPressed()
{
    reject();
}
