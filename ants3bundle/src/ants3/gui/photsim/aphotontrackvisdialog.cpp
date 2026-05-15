#include "aphotontrackvisdialog.h"
#include "ui_aphotontrackvisdialog.h"
#include "atrackvisattributes.h"
#include "a3global.h"
#include "arootcolorselectordialog.h"

#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>

#include "TROOT.h"
#include "TColor.h"

APhotonTrackVisDialog::APhotonTrackVisDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::APhotonTrackVisDialog), CurrentAts(A3Global::getInstance().CurrentTrackVisAttributes)
{
    ui->setupUi(this);
    setWindowTitle("Photon track attributes");

    QStringList styles = {"Straight", "Short_dash", "Dot", "Short_dash dot", "Dash dot", "Dash 3_dots", "Dash", "Dash 2_dots", "Long_dash", "Long_dash dot"};
    ui->cobStylePrimary->  addItems(styles);
    ui->cobStyleSecondary->addItems(styles);
    ui->cobStyleHitSensor->addItems(styles);

    AllTypes = { {&CurrentAts.PrimaryPhotonTracks,   ui->cobStylePrimary,   ui->sbWidthPrimary,   ui->pbColorPrimary},
                 {&CurrentAts.SecondaryPhotonTracks, ui->cobStyleSecondary, ui->sbWidthSecondary, ui->pbColorSecondary},
                 {&CurrentAts.HitSensorPhotonTracks, ui->cobStyleHitSensor, ui->sbWidthHitSensor, ui->pbColorHitSensor} };

    for (AProps & prop : AllTypes)
    {
        connect(prop.cob, &QComboBox::activated, [prop](int index)
                {
                    prop.attr->Style = index + 1;
                });

        connect(prop.sb, &QSpinBox::valueChanged, [prop](int width)
                {
                    prop.attr->Width = width;
                });

        connect(prop.pb, &QPushButton::clicked, [this, prop]()
                {
                    ARootColorSelectorDialog dia(prop.attr->Color, this);
                    dia.exec();
                    updateColor(prop.pb, prop.attr->Color);
                });
    }

    connect(ui->cbEnableHitSensor, &QCheckBox::clicked, [this](bool checked)
            {
                CurrentAts.UseHitSensorAttributes = checked;
            });

    QList<QPushButton*> list = this->findChildren<QPushButton *>();
    foreach(QPushButton * pb, list) {pb->setDefault(false); pb->setAutoDefault(false);}

    updateGui();
}

APhotonTrackVisDialog::~APhotonTrackVisDialog()
{
    delete ui;
}

void APhotonTrackVisDialog::on_pbClose_clicked()
{
    accept();
}

void APhotonTrackVisDialog::updateGui()
{
    for (AProps & prop : AllTypes)
    {
        int iStIndex = prop.attr->Style - 1; // 0 does not exist
        if (iStIndex < 0) iStIndex = 0;
        if (iStIndex < prop.cob->count()) prop.cob->setCurrentIndex(iStIndex);

        if (prop.attr->Width > -1) prop.sb->setValue(prop.attr->Width);

        updateColor(prop.pb, prop.attr->Color);
    }

    ui->cbEnableHitSensor->setChecked(CurrentAts.UseHitSensorAttributes);
}

void APhotonTrackVisDialog::updateColor(QPushButton * pb, int color)
{
    TColor * tc = gROOT->GetColor(color);
    int red = 255;
    int green = 255;
    int blue = 255;
    float alpha = 0.5;
    if (tc)
    {
        red   = 255 * tc->GetRed();
        green = 255 * tc->GetGreen();
        blue  = 255 * tc->GetBlue();
        alpha = tc->GetAlpha();
    }
    pb->setStyleSheet( QString("background-color:rgba(%1,%2,%3,%4);border:none;").arg(red).arg(green).arg(blue).arg(alpha) );
}
