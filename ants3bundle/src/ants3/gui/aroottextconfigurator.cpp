#include "aroottextconfigurator.h"
#include "ui_aroottextconfigurator.h"
#include "guitools.h"

#include <QDebug>
#include <QPushButton>
#include <QPainter>
#include <QMouseEvent>
#include <QDoubleValidator>

#include "TAttText.h"
#include "TColor.h"
#include "TROOT.h"

ARootTextConfigurator::ARootTextConfigurator(int & color, int & align, int & font, float &size, QWidget * parent) :
    QDialog(parent),
    ui(new Ui::ARootTextConfigurator),
    Color(color), Align(align), Font(font), Size(size)
{
    ui->setupUi(this);

    setMouseTracking(true);
    setWindowTitle("ROOT text properties");

    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = this->findChildren<QLineEdit *>();
    foreach(QLineEdit *w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);

    QPushButton* dummy = new QPushButton(this); //intercepts eneter key hits
    dummy->setDefault(true);
    dummy->setVisible(false);

    BaseColors = {880, 900, 800, 820, 840, 860, 9};
    ui->frColorPanel->setFixedSize(SquareSize * 20, SquareSize * BaseColors.size());
    ui->frCol->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->frCol->setFixedSize(30, 30);

    setFixedWidth(width());

    ui->sbColor->setValue(color);
    setupAlignmentControls();
    int fontIndex = font / 10;
    int precision = font % 10;
    ui->sbFont->setValue(fontIndex);
    ui->cobFontType->setCurrentIndex( precision == 3 ? 1 : 0 );
    ui->ledSize->setText( QString::number(size) );

    updateColorFrame();

    connect(ui->sbFont, &QSpinBox::valueChanged, this, &ARootTextConfigurator::onUserAction); // here and not automatic to avoid trigger on fill
}

ARootTextConfigurator::~ARootTextConfigurator()
{
    delete ui;
}

void ARootTextConfigurator::paintEvent(QPaintEvent *)
{
    QPainter p;
    p.begin(this);

    p.setPen(Qt::NoPen);

    for (int i=0; i<BaseColors.size(); i++)
      PaintColorRow(&p, i, BaseColors.at(i));

    p.end();
}

#include <QtGlobal>
void ARootTextConfigurator::mousePressEvent(QMouseEvent *e)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    int row = e->pos().y() / SquareSize;
    int num = e->pos().x() / SquareSize;
#else
    int row = e->position().y() / SquareSize;
    int num = e->position().x() / SquareSize;
#endif

    if (row >= BaseColors.size()) return;

    int color = -9 + num + BaseColors.at(row);
    ui->sbColor->setValue(color);

    updateColorFrame();
    onUserAction();
}

void ARootTextConfigurator::setupAlignmentControls()
{
    int vert = Align % 10;
    int hor  = Align / 10.0;
    //qDebug() << align << hor << vert;
    ui->cobHorizontalAlignment->setCurrentIndex(hor-1);
    ui->cobVerticalAlignment->setCurrentIndex(vert-1);
}

int ARootTextConfigurator::readAlignment()
{
    return 10 * (1 + ui->cobHorizontalAlignment->currentIndex()) + (1 + ui->cobVerticalAlignment->currentIndex());
}

void ARootTextConfigurator::PaintColorRow(QPainter *p, int row, int colorBase)
{
    for (int i=0; i<20; i++)
    {
        int c = -9 +i +colorBase;
        TColor *tc = gROOT->GetColor(c);
        int red = 255*tc->GetRed();
        int green = 255*tc->GetGreen();
        int blue = 255*tc->GetBlue();
        p->setBrush(QBrush(QColor( red, green, blue )));
        p->drawRect( i*SquareSize, row*SquareSize, SquareSize,SquareSize);
    }
}

void ARootTextConfigurator::updateColorFrame()
{
    TColor *tc = gROOT->GetColor(ui->sbColor->value());

    previewColor();

    if (!tc)
    {
        guitools::message("Not a valid color index!", this);
        ui->sbColor->setValue(1);
    }
}

void ARootTextConfigurator::previewColor()
{
    TColor *tc = gROOT->GetColor(ui->sbColor->value());

    int red = 255;
    int green = 255;
    int blue = 255;

    if (tc)
    {
        red = 255*tc->GetRed();
        green = 255*tc->GetGreen();
        blue = 255*tc->GetBlue();
    }
    ui->frCol->setStyleSheet(  QString("background-color:rgb(%1,%2,%3)").arg(red).arg(green).arg(blue)  );
}

void ARootTextConfigurator::onUserAction()
{
    emit propertiesChanged(ui->sbColor->value(),
                           readAlignment(),
                           ui->sbFont->value() * 10 + (ui->cobFontType->currentIndex() == 1 ? 3 : 2),
                           ui->ledSize->text().toFloat());
}

void ARootTextConfigurator::on_pbAccept_clicked()
{
    Color = ui->sbColor->value();
    Font  = ui->sbFont->value() * 10 + (ui->cobFontType->currentIndex() == 1 ? 3 : 2);
    Size  = ui->ledSize->text().toFloat();
    Align = readAlignment();

    accept();
}

void ARootTextConfigurator::on_pbCancel_clicked()
{
    reject();
}

ARootAxisTitleTextConfigurator::ARootAxisTitleTextConfigurator(int &color, int &align, int &font, float &size, QWidget *parent) :
    ARootTextConfigurator(color, align, font, size, parent)
{
    setWindowTitle("Axis title text properties");
    setupAlignmentControls();
}

void ARootAxisTitleTextConfigurator::setupAlignmentControls()
{
    ui->cobVerticalAlignment->setVisible(false);
    ui->cobHorizontalAlignment->clear();
    ui->cobHorizontalAlignment->addItems({"Right", "Center"});

    ui->cobHorizontalAlignment->setCurrentIndex(Align == 0);
}

int ARootAxisTitleTextConfigurator::readAlignment()
{
    return ui->cobHorizontalAlignment->currentIndex();
}

ARootAxisLabelTextConfigurator::ARootAxisLabelTextConfigurator(int &color, int &align, int &font, float &size, QWidget *parent) :
    ARootTextConfigurator(color, align, font, size, parent)
{
    setWindowTitle("Axis title text properties");
    setupAlignmentControls();
}

void ARootAxisLabelTextConfigurator::setupAlignmentControls()
{
    ui->cobVerticalAlignment->setVisible(false);
    ui->cobHorizontalAlignment->setVisible(false);
    ui->labAlign->setVisible(false);
    ui->lineAlign->setVisible(false);
}

int ARootAxisLabelTextConfigurator::readAlignment()
{
    // nothing to do
    return 0;
}

void ARootTextConfigurator::on_cobHorizontalAlignment_activated(int)
{
    onUserAction();
}

void ARootTextConfigurator::on_cobVerticalAlignment_activated(int)
{
    onUserAction();
}

void ARootTextConfigurator::on_ledSize_editingFinished()
{
    onUserAction();
}

void ARootTextConfigurator::on_cobFontType_activated(int)
{
    onUserAction();
}

