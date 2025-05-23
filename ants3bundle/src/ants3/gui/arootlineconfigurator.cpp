#include "arootlineconfigurator.h"
#include "guitools.h"

#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QLabel>
#include <QComboBox>
#include <QFrame>
#include <QPushButton>

#include "TROOT.h"
#include "TColor.h"

ARootLineConfigurator::ARootLineConfigurator(int & color, int & width, int & style, QWidget * parent)
    : QDialog(parent), ReturnColor(color), ReturnWidth(width), ReturnStyle(style)
{
    setMouseTracking(true);
    this->setWindowTitle("Select ROOT line properties");

    //  kWhite =0, kBlack =1, kGray =920, kRed =632,
    //  kGreen =416, kBlue =600, kYellow =400, kMagenta =616,
    //  kCyan =432, kOrange =800, kSpring =820, kTeal =840,
    //  kAzure =860, kViolet =880, kPink =900
    BaseColors << 880 << 900 << 800 << 820 << 840 << 860 << 9;
    //AlternativeBaseColors << 600 << 616 << 632 << 400 << 416 << 432;

    //QPushButton* dummy = new QPushButton(this); //intercepts "accept" of the dialog
    //dummy->setVisible(false);

    QFrame* frMain = new QFrame(this);
    frMain->setFrameShape(QFrame::Box);
    frMain->setGeometry(3, BaseColors.size()*SquareSize + 3, SquareSize*20-6, 50);
    QHBoxLayout* lay = new QHBoxLayout();
    lay->setContentsMargins(50,0,50,0);
    QLabel* labW = new QLabel();
    labW->setText("Width:");
    labW->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    lay->addWidget(labW);

    sbWidth = new QSpinBox();
    sbWidth->setMinimum(0);
    sbWidth->setValue(width);
    connect(sbWidth, &QSpinBox::valueChanged, this, &ARootLineConfigurator::onUserAction);
    lay->addWidget(sbWidth);

    QLabel * labS = new QLabel();
    labS->setText("Style:");
    labS->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    lay->addWidget(labS);

    comStyle = new QComboBox();
    comStyle->addItem("Straight");
    comStyle->addItem("Short_dash");
    comStyle->addItem("Dot");
    comStyle->addItem("Short_dash dot");
    comStyle->addItem("Dash dot");
    comStyle->addItem("Dash 3_dots");
    comStyle->addItem("Dash");
    comStyle->addItem("Dash 2_dots");
    comStyle->addItem("Long_dash");
    comStyle->addItem("Long_dash dot");
    comStyle->setMinimumHeight(20);
    comStyle->setMinimumWidth(150);
    if (style > 0 && style < 11)  comStyle->setCurrentIndex(style-1);
    else                          comStyle->setCurrentIndex(0);
    connect(comStyle, &QComboBox::currentIndexChanged, this, &ARootLineConfigurator::onUserAction);
    lay->addWidget(comStyle);

    QLabel * labC = new QLabel();
    labC->setText("Color:");
    labC->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    lay->addWidget(labC);

    sbColor = new QSpinBox();
    sbColor->setMaximum(999);
    sbColor->setValue(color);
    connect(sbColor, &QSpinBox::valueChanged,    this, &ARootLineConfigurator::previewColor);
    connect(sbColor, &QSpinBox::editingFinished, this, &ARootLineConfigurator::updateColorFrame);
    lay->addWidget(sbColor);

    frCol = new QFrame();
    frCol->setMaximumSize(30, 30);
    frCol->setFrameShape(QFrame::Box);
    frCol->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    updateColorFrame();
    lay->addWidget(frCol);
    frMain->setLayout(lay);

    QPushButton * pbAccept = new QPushButton("Accept new values", this);
    pbAccept->setDefault(false);
    pbAccept->setGeometry(20*SquareSize/2-75, frMain->y()+frMain->height()+5, 150, 30);
    connect(pbAccept, &QPushButton::pressed, this, &ARootLineConfigurator::onAccept);

    resize(SquareSize*20, pbAccept->y()+pbAccept->height()+5);
}

void ARootLineConfigurator::paintEvent(QPaintEvent *)
{
    QPainter p;
    p.begin(this);

    p.setPen(Qt::NoPen);

    for (int i=0; i<BaseColors.size(); i++)
        PaintColorRow(&p, i, BaseColors.at(i));

    p.end();
}

void ARootLineConfigurator::PaintColorRow(QPainter * p, int row, int colorBase)
{
    for (int i=0; i<20; i++)
    {
        int c = -9 +i +colorBase;
        TColor * tc = gROOT->GetColor(c);

        int red   = 255 * tc->GetRed();
        int green = 255 * tc->GetGreen();
        int blue  = 255 * tc->GetBlue();
        p->setBrush(QBrush(QColor( red, green, blue )));
        p->drawRect(i*SquareSize, row*SquareSize, SquareSize,SquareSize);
    }
}

void ARootLineConfigurator::onAccept()
{
    ReturnWidth = sbWidth->value();
    ReturnStyle = 1 + comStyle->currentIndex();
    ReturnColor = sbColor->value();
    accept();
}

void ARootLineConfigurator::onUserAction()
{
    emit propertiesChanged(sbColor->value(), sbWidth->value(), 1 + comStyle->currentIndex());
}

//#include <QtGlobal>
void ARootLineConfigurator::mousePressEvent(QMouseEvent *e)
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
    sbColor->setValue(color);
    //qDebug() << color << "at row/num:"<<row<<num;
    sbColor->setValue(color);
    updateColorFrame();
}

void ARootLineConfigurator::updateColorFrame()
{
    TColor *tc = gROOT->GetColor(sbColor->value());

    previewColor();

    if (!tc)
    {
        guitools::message("Not valid color!", this);
        sbColor->setValue(1);
    }
    else onUserAction();
}

void ARootLineConfigurator::previewColor()
{
    TColor * tc = gROOT->GetColor(sbColor->value());

    int red   = 255;
    int green = 255;
    int blue  = 255;

    if (tc)
    {
        red   = 255 * tc->GetRed();
        green = 255 * tc->GetGreen();
        blue  = 255 * tc->GetBlue();
    }
    frCol->setStyleSheet( QString("background-color:rgb(%1,%2,%3)").arg(red).arg(green).arg(blue) );
}

