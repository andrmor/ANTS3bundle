#include "apaletteselectiondialog.h"
#include "ui_apaletteselectiondialog.h"

#include "TStyle.h"

APaletteSelectionDialog::APaletteSelectionDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::APaletteSelectionDialog)
{
    ui->setupUi(this);
    setWindowTitle("Select color pattern");

    // https://root.cern/doc/master/classTColor.html#C06
    Palettes =
        {
            {"DeepSea",51},          {"GreyScale",52},    {"DarkBodyRadiator",53},
            {"BlueYellow", 54},      {"RainBow",55},      {"InvertedDarkBodyRadiator",56},
            {"Bird",57},             {"Cubehelix",58},    {"GreenRedViolet",59},
            {"BlueRedYellow",60},    {"Ocean",61},        {"ColorPrintableOnGrey",62},
            {"Alpine",63},           {"Aquamarine",64},   {"Army",65},
            {"Atlantic",66},         {"Aurora",67},       {"Avocado",68},
            {"Beach",69},            {"BlackBody",70},    {"BlueGreenYellow",71},
            {"BrownCyan",72},        {"CMYK",73},         {"Candy",74},
            {"Cherry",75},           {"Coffee",76},       {"DarkRainBow",77},
            {"DarkTerrain",78},      {"Fall",79},         {"FruitPunch",80},
            {"Fuchsia",81},          {"GreyYellow",82},   {"GreenBrownTerrain",83},
            {"GreenPink",84},        {"Island",85},       {"Lake",86},
            {"LightTemperature",87}, {"LightTerrain",88}, {"Mint",89},
            {"Neon",90},             {"Pastel",91},       {"Pearl",92},
            {"Pigeon",93},           {"Plum",94},         {"RedBlue",95},
            {"Rose",96},             {"Rust",97},         {"SandyTerrain",98},
            {"Sienna",99},           {"Solar",100},       {"SouthWest",101},
            {"StarryNight",102},     {"Sunset",103},      {"TemperatureMap",104},
            {"Thermometer",105},     {"Valentine",106},   {"VisibleSpectrum",107},
            {"WaterMelon",108},      {"Cool",109},        {"Copper",110},
            {"GistEarth",111},       {"Viridis",112},     {"Cividis",113}
        };

    std::sort(Palettes.begin(), Palettes.end(), [](std::pair<QString,int> lhs, std::pair<QString,int> rhs){return lhs.first < rhs.first;});

    QStringList sl;
    size_t index = 0;
    for (const auto & pair : Palettes)
    {
        sl << pair.first;

        if (pair.first == "Bird") DefaultIndex = index;
        index++;
    }
    ui->cob->addItems(sl);
    ui->cob->setCurrentIndex(-1);
}

APaletteSelectionDialog::~APaletteSelectionDialog()
{
    delete ui;
}

void APaletteSelectionDialog::on_pbClose_clicked()
{
    accept();
}

void APaletteSelectionDialog::on_cob_activated(int index)
{
    gStyle->SetPalette(Palettes[index].second);
    emit requestRedraw();
}

void APaletteSelectionDialog::on_pbUseDefault_clicked()
{
    ui->cob->setCurrentIndex(DefaultIndex);
    on_cob_activated(DefaultIndex);
}

