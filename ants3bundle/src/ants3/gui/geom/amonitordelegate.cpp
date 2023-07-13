#include "amonitordelegate.h"
#include "amonitordelegateform.h"

#include <QWidget>
#include <QFrame>
#include <QPalette>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

AMonitorDelegate::AMonitorDelegate(QWidget *ParentWidget) :
    AGeoBaseDelegate(ParentWidget)
{
    QFrame * frMainFrame = new QFrame();
    frMainFrame->setFrameShape(QFrame::Box);

    Widget = frMainFrame;

    QPalette palette = frMainFrame->palette();
    //palette.setColor( frMainFrame->backgroundRole(), QColor( 255, 255, 255 ) );
    palette.setColor( frMainFrame->backgroundRole(), palette.color(QPalette::AlternateBase) );
    frMainFrame->setPalette( palette );
    frMainFrame->setAutoFillBackground( true );
    //frMainFrame->setMinimumHeight(380);
    //frMainFrame->setMaximumHeight(380);
    //frMainFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    QVBoxLayout* vl = new QVBoxLayout();
    vl->setContentsMargins(5,5,5,5);

    //object type
    labType = new QLabel("Monitor");
    labType->setAlignment(Qt::AlignCenter);
    QFont font = labType->font();
    font.setBold(true);
    labType->setFont(font);
    vl->addWidget(labType);

    del = new AMonitorDelegateForm(Widget);
    del->UpdateVisibility();
    connect(del, &AMonitorDelegateForm::contentChanged, this, &AMonitorDelegate::onContentChanged);
    connect(del, &AMonitorDelegateForm::showSensDirection, this, &AMonitorDelegate::requestShowSensitiveFaces);
    vl->addWidget(del);

    // bottom line buttons
    createBottomButtons();
    vl->addWidget(frBottomButtons);

    frMainFrame->setLayout(vl);
}

QString AMonitorDelegate::getName() const
{
    return del->getName();
}

bool AMonitorDelegate::updateObject(AGeoObject *obj) const
{
    return del->updateObject(obj);
}

#include "ageotype.h"
#include "ageoobject.h"
void AMonitorDelegate::Update(const AGeoObject * obj)
{
    ATypeMonitorObject * tmo = dynamic_cast<ATypeMonitorObject*>(obj->Type);
    QString txt;
    if (tmo->config.PhotonOrParticle == 0) txt = "Photon monitor";
    else txt = "Particle monitor";
    labType->setText(txt);

    updateLineColorFrame(obj);

    bool bOK = del->updateGUI(obj);
    if (!bOK) return;
}

void AMonitorDelegate::onContentChanged()
{
    onContentChangedBase();
    Widget->layout()->activate();
}
