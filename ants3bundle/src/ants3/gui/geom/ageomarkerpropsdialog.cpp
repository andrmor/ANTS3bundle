#include "ageomarkerpropsdialog.h"
#include "TColor.h"
#include "arootmarkerconfigurator.h"
#include "a3global.h"
#include "arootcolorselectordialog.h"
#include "guitools.h"
#include "ajsontools.h"

#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QMenuBar>
#include <QTimer>

#include "TROOT.h"

AGeoMarkerPropsDialog::AGeoMarkerPropsDialog(AGeoMarkerPropDatabase & geoMarkProps, const std::set<QString> &presentMarkerTypes, QWidget * parent) :
    QDialog(parent), GeoMarkProps(geoMarkProps), PresentMarkerTypes(presentMarkerTypes)
{
    setWindowTitle("Marker configurator");

    setWindowModality(Qt::WindowModal);

    DoubleValidator = new QDoubleValidator(this);
    DoubleValidator->setBottom(0);
    IntValidator = new QIntValidator(this);
    IntValidator->setBottom(1);

    MarkerStyles = {"1: small dot", "2: cross","3: asterisk","4: circle","5: diagonal cross","6: rhomb dot","7: square dot","8: large round dot",
                    "20: filled circle", "21: filled square","22: filled triangle","23: inv filled triangle","24: circle","25: square",
                    "26: triangle", "27: romb","28: big cross","29: filled star","30: star","32: inverted triangle","33: filled romb","34: filled cross",
                    "35: open diamon cross", "36: open square diagonal", "37: open three triangle", "38: octagon with cross", "39: full three triangles",
                    "40: open four triangleX", "41: full four triangleX", "42: open double diamond", "43: full double diamond", "44: open four triangle+",
                    "45: full four triangle+", "46: open cross X", "47: full cross X", "48: four squares X", "49: four squares+"};
    for (int i = 1;  i < 9;  i++) StyleMap.push_back(i);
    for (int i = 9;  i < 20; i++) StyleMap.push_back(8);
    for (int i = 20; i < 31; i++) StyleMap.push_back(9 + i - 20);
    StyleMap.push_back(3);
    for (int i = 32; i < 50; i++) StyleMap.push_back(20 + i - 32);

    QVBoxLayout * layMain = new QVBoxLayout(this);

    QMenuBar * menuBar = new QMenuBar(this);
    layMain->setMenuBar(menuBar);

    layGr = new QGridLayout();
    layMain->addLayout(layGr);

    layMain->addWidget(guitools::makeLine(true));

    QHBoxLayout * layM = new QHBoxLayout();
        layM->addStretch();
        layM->addWidget(new QLabel("Global size multiplier:"));
        ledMult = new QLineEdit(); ledMult->setValidator(DoubleValidator);
            connect(ledMult, &QLineEdit::editingFinished, this, &AGeoMarkerPropsDialog::updateMultiplier);
        layM->addWidget(ledMult);
        layM->addStretch();
        QPushButton * pbAccept = new QPushButton("Close");
            pbAccept->setDefault(false);
            pbAccept->setAutoDefault(false);
            connect(pbAccept, &QPushButton::clicked, this, &AGeoMarkerPropsDialog::onApplyAndClose);
        layM->addWidget(pbAccept);
        layM->addStretch();
    layMain->addLayout(layM);

    updatePropsGui();

    // populating menu
    QMenu * fileM = menuBar->addMenu("File");
    fileM->addAction("Save", this, &AGeoMarkerPropsDialog::save);
    fileM->addSeparator();
    fileM->addAction("Load", this, &AGeoMarkerPropsDialog::load);

    QMenu * defM = menuBar->addMenu("Default");   //&File
    defM->addAction("Load default on this computer", this, &AGeoMarkerPropsDialog::restoreDefault);
    defM->addSeparator();
    defM->addAction("Set current as default on this computer", this, &AGeoMarkerPropsDialog::makeDefault);
    defM->addSeparator();
    defM->addAction("Load Ants3 default", this, &AGeoMarkerPropsDialog::factoryReset);

    // shift
    QTimer::singleShot(0, this, [this](){move(x(), y() + 1.5*height());});
}

void AGeoMarkerPropsDialog::updatePropsGui()
{
    while (QLayoutItem * item = layGr->takeAt(0))
    {
        delete item->widget();
        delete item;
    }

    layGr->addWidget(new QLabel("Marker type"), 0, 0, Qt::AlignHCenter);
    layGr->addWidget(new QLabel("Style"),       0, 1, Qt::AlignHCenter);
    layGr->addWidget(new QLabel("Size"),        0, 2, Qt::AlignHCenter);
    layGr->addWidget(new QLabel("Width"),       0, 3, Qt::AlignHCenter);
    layGr->addWidget(new QLabel("Color"),       0, 4, Qt::AlignHCenter);

    LocalData.resize(GeoMarkProps.Data.size());
    int iRec = 0;
    for (const auto & [type, props] : GeoMarkProps.Data)
    {
        LocalData[iRec] = {type, props};

        QLabel * labT = new QLabel(type);
        layGr->addWidget(labT, iRec+1, 0);

        if (type == "Undefined")
            layGr->addWidget(new QLabel("Custom"), iRec+1, 1);
        else
        {
            QComboBox * cobStyle = new QComboBox();
            cobStyle->addItems(MarkerStyles);
            layGr->addWidget(cobStyle, iRec+1, 1);
            connect(cobStyle, &QComboBox::activated, [this, iRec, cobStyle]()
                    {
                        QString st = cobStyle->currentText();
                        QStringList l = st.split(':');
                        QString num = l.first();
                        LocalData[iRec].second.Style = num.toInt();
                        applyToGlobalAndRedraw();
                    });
            int iStyle = 1;
            if (props.Style > 0 && props.Style <= StyleMap.size()) iStyle = StyleMap[props.Style-1];
            cobStyle->setCurrentIndex(iStyle - 1);

            QLineEdit * ledSize = new QLineEdit();
            ledSize->setValidator(DoubleValidator);
            ledSize->setMinimumWidth(50);
            ledSize->setMaximumWidth(50);
            layGr->addWidget(ledSize, iRec+1, 2);
            connect(ledSize, &QLineEdit::editingFinished, [this, iRec, ledSize]()
                    {
                        LocalData[iRec].second.Size = ledSize->text().toDouble();
                        applyToGlobalAndRedraw();
                    });
            ledSize->setText(QString::number(props.Size));

            QLineEdit * ledWidth = new QLineEdit();
            ledWidth->setValidator(IntValidator);
            ledWidth->setMinimumWidth(50);
            ledWidth->setMaximumWidth(50);
            layGr->addWidget(ledWidth, iRec+1, 3);
            connect(ledWidth, &QLineEdit::editingFinished, [this, iRec, ledWidth]()
                    {
                        LocalData[iRec].second.LineWidth = ledWidth->text().toInt();
                        applyToGlobalAndRedraw();
                    });
            ledWidth->setText(QString::number(props.LineWidth));

            QPushButton * pbColor = new QPushButton("   ");
            pbColor->setMinimumHeight(25);
            pbColor->setFlat(true);
            pbColor->setDefault(false);
            pbColor->setAutoDefault(false);
            layGr->addWidget(pbColor, iRec+1, 4);
            connect(pbColor, &QPushButton::clicked, [this, iRec, pbColor]()
                    {
                        ARootColorSelectorDialog dia(LocalData[iRec].second.Color, this);
                        dia.exec();
                        updateColor(pbColor, LocalData[iRec].second.Color);
                        applyToGlobalAndRedraw();
                    });
            updateColor(pbColor, props.Color);
        }

        updateTypePresent(labT);

        QPushButton * pbInfo = new QPushButton("?");
        pbInfo->setMaximumWidth(20);
        pbInfo->setDefault(false);
        pbInfo->setAutoDefault(false);
        connect(pbInfo, &QPushButton::clicked, [this, labT](){showInfo(labT->text());});
        layGr->addWidget(pbInfo, iRec+1, 5);

        iRec++;
    }

    ledMult->setText(QString::number(GeoMarkProps.SizeMultiplier));
}

void AGeoMarkerPropsDialog::applyToGlobalAndRedraw()
{
    copyLocalToGlobal();
    emit requestRedraw(false, false, true);
}

void AGeoMarkerPropsDialog::onApplyAndClose()
{
    copyLocalToGlobal();
    emit requestRedraw(false, false, true);
    accept();
}

void AGeoMarkerPropsDialog::updateMultiplier()
{
    SizeMultiplier = ledMult->text().toDouble();
    applyToGlobalAndRedraw();
}

void AGeoMarkerPropsDialog::save()
{
    QString fn = guitools::dialogSaveFile(this, "Save marker settings to file", "*.json");
    if (fn.isEmpty()) return;
    if (!fn.endsWith(".json")) fn += ".json";

    copyLocalToGlobal();
    QJsonObject json;
    GeoMarkProps.writeToJson(json);
    bool ok = jstools::saveJsonToFile(json, fn);
    if (!ok) guitools::message("Failed to save to file " + fn);
}

void AGeoMarkerPropsDialog::load()
{
    QString fn = guitools::dialogLoadFile(this, "Load marker settings from file", "*.json");
    if (fn.isEmpty()) return;

    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, fn);
    if (!ok || json.isEmpty())
    {
        guitools::message("Failed to load from file " + fn);
        return;
    }

    GeoMarkProps.readFromJson(json);
    updatePropsGui();
    emit requestRedraw(false, false, true);
}

void AGeoMarkerPropsDialog::restoreDefault()
{
    GeoMarkProps = A3Global::getInstance().GeoMarkersDefaults;
    updatePropsGui();
    emit requestRedraw(false, false, true);
}

void AGeoMarkerPropsDialog::makeDefault()
{
    copyLocalToGlobal();
    A3Global::getInstance().GeoMarkersDefaults = GeoMarkProps;
}

void AGeoMarkerPropsDialog::factoryReset()
{
    GeoMarkProps.fillDefault();
    updatePropsGui();
    emit requestRedraw(false, false, true);
}

void AGeoMarkerPropsDialog::copyLocalToGlobal()
{
    for (const std::pair<QString, AGeoMarkerProperties> & pair : LocalData)
        GeoMarkProps.Data[pair.first] = pair.second;
    GeoMarkProps.SizeMultiplier = SizeMultiplier;
}

void AGeoMarkerPropsDialog::updateColor(QPushButton * pb, int color)
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

void AGeoMarkerPropsDialog::showInfo(QString type)
{
    guitools::message(GeoMarkProps.getInfo(type), this);
}

void AGeoMarkerPropsDialog::updateTypePresent(QLabel * lab)
{
    QString text = lab->text();
    if (std::find(PresentMarkerTypes.begin(), PresentMarkerTypes.end(), text) != PresentMarkerTypes.end())
    {
        QFont font = lab->font();
        font.setBold(true);
        font.setWeight(QFont::DemiBold);
        lab->setFont(font);
    }
}
