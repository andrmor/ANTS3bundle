#include "aphotontrackvisdialog.h"
#include "ui_aphotontrackvisdialog.h"
#include "atrackvisattributes.h"
#include "a3global.h"

#include "TROOT.h"
#include "TColor.h"

APhotonTrackVisDialog::APhotonTrackVisDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::APhotonTrackVisDialog)
{
    ui->setupUi(this);

    QStringList styles = {"Straight", "Short_dash", "Dot", "Short_dash dot", "Dash dot", "Dash 3_dots", "Dash", "Dash 2_dots", "Long_dash", "Long_dash dot"};
    ui->cobStylePrimary->  addItems(styles);
    ui->cobStyleSecondary->addItems(styles);
    ui->cobStyleHitSensor->addItems(styles);

    QList<QPushButton*> list = this->findChildren<QPushButton *>();
    foreach(QPushButton * pb, list) {pb->setDefault(false); pb->setAutoDefault(false);}

    ATrackVisAttributes & vis = A3Global::getInstance().CurrentTrackVisAttributes;
    PrimaryPhotonTracks   = vis.PrimaryPhotonTracks;
    SecondaryPhotonTracks = vis.SecondaryPhotonTracks;
    HitSensorPhotonTracks = vis.HitSensorPhotonTracks;

    updateGui();
}

APhotonTrackVisDialog::~APhotonTrackVisDialog()
{
    delete ui;
}

void APhotonTrackVisDialog::on_pbClose_clicked()
{
    //A3Global & GlobSet = A3Global::getInstance();
    //ATrackVisAttributes & vis = ATrackVisAttributes::getInstance();
    //vis.writeToJson(GlobSet.TrackVisAttributes);
    //GlobSet.saveConfig();

    qDebug() << PrimaryPhotonTracks.Color;

    accept();
}

#include "arootcolorselectordialog.h"
void APhotonTrackVisDialog::updateGui()
{
    const ATrackVisAttributes & opt = A3Global::getInstance().CurrentTrackVisAttributes;

    struct AProps
    {
        ATrackAttributes & attr;
        QComboBox   * cob = nullptr;
        QSpinBox    * sb  = nullptr;
        QPushButton * pb  = nullptr;
    };
    std::vector<AProps> all = { {PrimaryPhotonTracks,   ui->cobStylePrimary,   ui->sbWidthPrimary,   ui->pbColorPrimary},
                                {SecondaryPhotonTracks, ui->cobStyleSecondary, ui->sbWidthSecondary, ui->pbColorSecondary},
                                {HitSensorPhotonTracks, ui->cobStyleHitSensor, ui->sbWidthHitSensor, ui->pbColorHitSensor} };

    for (AProps & prop : all)
    {
        int iStIndex = prop.attr.Style - 1; // 0 does not exist
        if (iStIndex < 0) iStIndex = 0;
        if (iStIndex < prop.cob->count()) prop.cob->setCurrentIndex(iStIndex);

        if (prop.attr.Width > -1) prop.sb->setValue(prop.attr.Width);

        updateColor(prop.pb, prop.attr.Color);
        connect(prop.pb, &QPushButton::clicked, [this, prop]()
        {
            ARootColorSelectorDialog dia(prop.attr.Color, this);
            dia.exec();
            updateColor(prop.pb, prop.attr.Color);
        });

    }

    ui->cbEnableHitSensor->setChecked(opt.UseHitSensorAttributes);
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
