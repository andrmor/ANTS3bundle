#include "ageoobjectdelegate.h"
#include "ageoobject.h"
#include "ageotype.h"
#include "ageoshape.h"
#include "ageospecial.h"
#include "ageoconsts.h"
#include "guitools.h"
#include "aonelinetextedit.h"
#include "aparticleanalyzerwidget.h"

#include <QDebug>
#include <QWidget>
#include <QFrame>
#include <QStringList>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLayout>
#include <QDialog>
#include <QListWidget>
#include <QTableWidget>
#include <QMessageBox>
#include <QFont>
#include <QTableWidget>
#include <QHeaderView>

//#include "TVector3.h"

#include "TObject.h"

AGeoObjectDelegate::AGeoObjectDelegate(const QStringList & materials, QWidget * ParentWidget) :
    AGeoBaseDelegate(ParentWidget)
{
  QFrame * frMainFrame = new QFrame();
  frMainFrame->setFrameShape(QFrame::Box);

  Widget = frMainFrame;
  Widget->setContextMenuPolicy(Qt::CustomContextMenu);

  QPalette palette = frMainFrame->palette();
  //palette.setColor( Widget->backgroundRole(), QColor( 255, 255, 255 ) );
  palette.setColor( Widget->backgroundRole(), palette.color(QPalette::AlternateBase) );
  frMainFrame->setPalette( palette );
  frMainFrame->setAutoFillBackground( true );
  lMF = new QVBoxLayout();
    lMF->setContentsMargins(5,5,5,2);

    //object type
    labType = new QLabel("");
    labType->setAlignment(Qt::AlignCenter);
    QFont font = labType->font(); font.setBold(true); labType->setFont(font);
    lMF->addWidget(labType);

    //name and material line
    QHBoxLayout* hl = new QHBoxLayout();
      hl->setContentsMargins(2,0,2,0);
      QLabel* lname = new QLabel();
      lname->setText("Name:");
      lname->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      lname->setMaximumWidth(50);
      hl->addWidget(lname);
      leName = new QLineEdit();
      connect(leName, &QLineEdit::textChanged, this, &AGeoObjectDelegate::onContentChanged);
      leName->setMaximumWidth(200);
      leName->setContextMenuPolicy(Qt::NoContextMenu);
      hl->addWidget(leName);
      lMat = new QLabel();
      lMat->setText("Material:");
      lMat->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      lname->setMaximumWidth(60);
      hl->addWidget(lMat);
      cobMat = new QComboBox();
      cobMat->setContextMenuPolicy(Qt::NoContextMenu);
      cobMat->addItems(materials);
      connect(cobMat, SIGNAL(activated(int)), this, SLOT(onContentChanged()));
      cobMat->setMinimumWidth(120);
      hl->addWidget(cobMat);
    lMF->addLayout(hl);

    //scale widget
    QHBoxLayout * hbs = new QHBoxLayout();
    hbs->setContentsMargins(2,0,2,0);
        hbs->addStretch();
        cbScale = new QCheckBox("Scale");
        cbScale->setToolTip("Compatibility with Geant4 checked with CERN ROOT 6.30.06");
        connect(cbScale, &QCheckBox::clicked, this, &AGeoObjectDelegate::onScaleToggled);
        connect(cbScale, &QCheckBox::clicked, this, &AGeoObjectDelegate::onContentChanged);
        hbs->addWidget(cbScale);
    scaleWidget = new QWidget();
        QHBoxLayout * hbsw = new QHBoxLayout(scaleWidget);
        hbsw->setContentsMargins(0,0,0,0);
        hbsw->addWidget(new QLabel("in X:")); ledScaleX = new AOneLineTextEdit();
        hbsw->addWidget(ledScaleX);
        hbsw->addWidget(new QLabel("in Y:")); ledScaleY = new AOneLineTextEdit();
        hbsw->addWidget(ledScaleY);
        hbsw->addWidget(new QLabel("in Z:")); ledScaleZ = new AOneLineTextEdit();
        hbsw->addWidget(ledScaleZ);
        for (AOneLineTextEdit * led : {ledScaleX, ledScaleY, ledScaleZ})
        {
            configureHighligherAndCompleter(led);
            connect(led, &AOneLineTextEdit::textChanged, this, &AGeoObjectDelegate::onContentChanged);
            led->setText("1");
        }
    hbs->addWidget(scaleWidget);
    hbs->addStretch();
    lMF->addLayout(hbs);
    scaleWidget->setVisible(false);
    QObject::connect(cbScale, &QCheckBox::toggled, scaleWidget, &QWidget::setVisible);

    // Transform block
    QHBoxLayout * lht = new QHBoxLayout();
        pbTransform = new QPushButton("Transform to ...");
        lht->addWidget(pbTransform);
        connect(pbTransform, &QPushButton::pressed, this, &AGeoObjectDelegate::onChangeShapePressed);
        pbShapeInfo = new QPushButton("Info on this shape");
        connect(pbShapeInfo, &QPushButton::pressed, this, &AGeoObjectDelegate::onHelpRequested);
        lht->addWidget(pbShapeInfo);
    lMF->addLayout(lht);

    // Position and orientation block
    PosOrient = new QWidget();
    PosOrient->setContentsMargins(0,0,0,0);
    PosOrient->setMaximumHeight(80);
    QGridLayout *gr = new QGridLayout();
    gr->setContentsMargins(10, 0, 10, 3);
    gr->setVerticalSpacing(1);

      ledX     = new AOneLineTextEdit(); gr->addWidget(ledX,     0, 1);
      ledY     = new AOneLineTextEdit(); gr->addWidget(ledY,     1, 1);
      ledZ     = new AOneLineTextEdit(); gr->addWidget(ledZ,     2, 1);
      ledPhi   = new AOneLineTextEdit(); gr->addWidget(ledPhi,   0, 3);
      ledTheta = new AOneLineTextEdit(); gr->addWidget(ledTheta, 1, 3);
      ledPsi   = new AOneLineTextEdit(); gr->addWidget(ledPsi,   2, 3);

      const std::vector<AOneLineTextEdit*> ole = {ledX, ledY, ledZ, ledPhi, ledTheta, ledPsi};
      for (AOneLineTextEdit * le : ole)
      {
          configureHighligherAndCompleter(le);
          le->setContextMenuPolicy(Qt::NoContextMenu);
          connect(le, &AOneLineTextEdit::textChanged, this, &AGeoObjectDelegate::onContentChanged);
      }

      QLabel * l;
      l = new QLabel("X:");         gr->addWidget(l, 0, 0);
      l = new QLabel("Y:");         gr->addWidget(l, 1, 0);
      l = new QLabel("Z:");         gr->addWidget(l, 2, 0);

      l = new QLabel("mm    Phi:"); gr->addWidget(l, 0, 2);
      l = new QLabel("mm  Theta:"); gr->addWidget(l, 1, 2);
      l = new QLabel("mm    Psi:"); gr->addWidget(l, 2, 2);

      l = new QLabel("°");          gr->addWidget(l, 0, 4);
      l = new QLabel("°");          gr->addWidget(l, 1, 4);
      l = new QLabel("°");          gr->addWidget(l, 2, 4);

    PosOrient->setLayout(gr);
    lMF->addWidget(PosOrient);

    //special role widget
    crateSpecialRoleWidget();
    lMF->addWidget(RoleWidget);

    // bottom line buttons
    createBottomButtons();
    lMF->addWidget(frBottomButtons);

    frMainFrame->setLayout(lMF);

    ListOfShapesForTransform << "Box"
       << "Tube" << "Tube segment" << "Tube segment cut" << "Tube elliptical"
       << "Trap" << "Trap2"
       << "Polycone"
       << "Polygon simplified" << "Polygon"
       << "Parallelepiped"
       << "Sphere"
       << "Cone" << "Cone segment"
       << "Torus"
       << "Paraboloid"
       << "Arb8";
}

AGeoObjectDelegate::~AGeoObjectDelegate()
{
    //qDebug() << "deleted---------------";
    blockSignals(true);
    delete ShapeCopy; ShapeCopy = nullptr;

    //delete PartAnWidget; PartAnWidget = nullptr; // removed by the layout!
}

#include "QStackedWidget"
#include "asensorhub.h"
void AGeoObjectDelegate::crateSpecialRoleWidget()
{
    RoleWidget = new QWidget();

    QVBoxLayout * rl = new QVBoxLayout(RoleWidget);
    rl->setContentsMargins(2,0,2,0);
        cobRole = new QComboBox();
        cobRole->addItems({"No special role", "Light sensor", "Calorimeter", "Secondary scintillator", "Scintillator", "Photon transport: functional model", "Particle analyzer"});
    rl->addWidget(cobRole);
    rl->setAlignment(cobRole, Qt::AlignHCenter);

    QFrame * frSensor = createSensorGui();
    rl->addWidget(frSensor);
    rl->setAlignment(frSensor, Qt::AlignHCenter);
    connect(cobRole, &QComboBox::currentIndexChanged, frSensor, [frSensor](int index){frSensor->setVisible(index == 1);} );

    QFrame * frCal = createCalorimeterGui();
    rl->addWidget(frCal);
    connect(cobRole, &QComboBox::currentIndexChanged, frCal, [frCal, this](int index)
    {
        frCal->setVisible(index == 2);
        if (ledCalOriginX->text().isEmpty()) updateCalorimeterGui(ACalorimeterProperties());}
    );

    QFrame * frFun = createFunctionalModelGui();
    rl->addWidget(frFun);
    connect(cobRole, &QComboBox::currentIndexChanged, frFun, [frFun](int index){frFun->setVisible(index == 5);} );

    QFrame * frPartAn = createParticleAnalyzerGui();
    rl->addWidget(frPartAn);
    connect(cobRole, &QComboBox::currentIndexChanged, frPartAn, [frPartAn](int index){frPartAn->setVisible(index == 6);} );

    rl->addStretch();

    connect(cobRole, &QComboBox::currentIndexChanged, this, &AGeoObjectDelegate::onContentChanged);
}

QFrame * AGeoObjectDelegate::createSensorGui()
{
    QFrame * frSensor = new QFrame();
    {
        QHBoxLayout * hlSensor = new QHBoxLayout(frSensor);
        hlSensor->setContentsMargins(0,0,0,0);
        cobSensorModel = new QComboBox();
        if (ASensorHub::getConstInstance().isPersistentModelAssignment()) // mode does not change without rebuild, so no need in update
        {
            QLabel * l = new QLabel("Custom model indexes");
            l->setToolTip("Sensor are configured to use persistent model indexes\n"
                          "Check/modify using Sensor Window and scripting tools");
            hlSensor->addWidget(l);
        }
        else
        {
            hlSensor->addWidget(new QLabel("Sensor model:"));
            cobSensorModel->addItems(ASensorHub::getConstInstance().getListOfModelNames());
            hlSensor->addWidget(cobSensorModel);
        }
        frSensor->setVisible(false);
    }

    connect(cobSensorModel, &QComboBox::currentIndexChanged, this, &AGeoObjectDelegate::onContentChanged);

    return frSensor;
}

QFrame * AGeoObjectDelegate::createCalorimeterGui()
{
    QFrame * frCal = new QFrame();
    {
        QVBoxLayout * layMain = new QVBoxLayout(frCal); layMain->setContentsMargins(0,0,0,0); layMain->setSpacing(3);

        QHBoxLayout * layType = new QHBoxLayout(); //chl->setContentsMargins(15,0,15,0);
            layType->addWidget(new QLabel("Mode:"));
            cobCalType = new QComboBox(); cobCalType->addItems({"Energy per event", "3D energy", "3D dose"});
            cobCalType->setToolTip("Energy per event mode: Collect deposited energy for each event independently;\n3D modes: 3D distribution of energy [keV] or dose [grey] per voxel is collected over all events.");
            layType->addWidget(cobCalType);
            layType->addStretch();
            cbCalIncludeHosted = new QCheckBox("Composite calorimeter");
            cbCalIncludeHosted->setToolTip("When checked, all hosted volumes without an assigned special role (recursive!) are monitored by this calorimeter");
            layType->addWidget(cbCalIncludeHosted);
        layMain->addLayout(layType);

        QFrame * frOverEvents = new QFrame();
            QHBoxLayout * layOverEvents = new QHBoxLayout(); layOverEvents->setContentsMargins(0,0,0,0);
                layOverEvents->addWidget(new QLabel("Bins:"));
                leiCalEventDepoBins = new AOneLineTextEdit(); leiCalEventDepoBins->setMinimumWidth(50);
                layOverEvents->addWidget(leiCalEventDepoBins);
                layOverEvents->addWidget(new QLabel("From:"));
                ledCalEventDepoFrom = new AOneLineTextEdit(); ledCalEventDepoFrom->setMinimumWidth(50);
                layOverEvents->addWidget(ledCalEventDepoFrom);
                layOverEvents->addWidget(new QLabel("To:"));
                ledCalEventDepoTo = new AOneLineTextEdit(); ledCalEventDepoTo->setMinimumWidth(50);
                layOverEvents->addWidget(ledCalEventDepoTo);
                layOverEvents->addWidget(new QLabel("keV"));
                //connect(cbCalEventStat, &QCheckBox::toggled, frCalEventStat, &QFrame::setVisible);
                //connect(cbCalEventStat, &QCheckBox::clicked, this, &AGeoObjectDelegate::onContentChanged);
                //hlCalOverEv->addWidget(frCalEventStat);
                //hlCalOverEv->addStretch();
            frOverEvents->setLayout(layOverEvents);
        layMain->addWidget(frOverEvents);

        QFrame * fr3D = new QFrame();
            QGridLayout * gl = new QGridLayout(); gl->setContentsMargins(0,0,0,0);
                ledCalOriginX = new AOneLineTextEdit();
                ledCalOriginY = new AOneLineTextEdit();
                ledCalOriginZ = new AOneLineTextEdit();
                ledCalStepX = new AOneLineTextEdit();
                ledCalStepY = new AOneLineTextEdit();
                ledCalStepZ = new AOneLineTextEdit();
                leiCalBinsX = new AOneLineTextEdit();
                leiCalBinsY = new AOneLineTextEdit();
                leiCalBinsZ = new AOneLineTextEdit();
                cbOffX = new QCheckBox("Local X");
                cbOffY = new QCheckBox("Local Y");
                cbOffZ = new QCheckBox("Local Z");
                QLabel * labX = new QLabel("Local X"); labX->setContextMenuPolicy(Qt::CustomContextMenu); labX->setToolTip("Right-click to \"disable\"");
                QLabel * labY = new QLabel("Local Y"); labY->setContextMenuPolicy(Qt::CustomContextMenu); labY->setToolTip("Right-click to \"disable\"");
                QLabel * labZ = new QLabel("Local Z"); labZ->setContextMenuPolicy(Qt::CustomContextMenu); labZ->setToolTip("Right-click to \"disable\"");
                //gl->addWidget(new QLabel("Local X"), 0, 1, Qt::AlignHCenter);
                //gl->addWidget(new QLabel("Local Y"), 0, 2, Qt::AlignHCenter);
                //gl->addWidget(new QLabel("Local Z"), 0, 3, Qt::AlignHCenter);
                gl->addWidget(labX, 0, 1, Qt::AlignHCenter);
                gl->addWidget(labY, 0, 2, Qt::AlignHCenter);
                gl->addWidget(labZ, 0, 3, Qt::AlignHCenter);
                gl->addWidget(new QLabel("Bins"),   1, 0);
                gl->addWidget(leiCalBinsX,          1, 1);
                gl->addWidget(leiCalBinsY,          1, 2);
                gl->addWidget(leiCalBinsZ,          1, 3);
                gl->addWidget(new QLabel("Origin"), 2, 0);
                gl->addWidget(ledCalOriginX,        2, 1);
                gl->addWidget(ledCalOriginY,        2, 2);
                gl->addWidget(ledCalOriginZ,        2, 3);
                gl->addWidget(new QLabel("Step"),   3, 0);
                gl->addWidget(ledCalStepX,          3, 1);
                gl->addWidget(ledCalStepY,          3, 2);
                gl->addWidget(ledCalStepZ,          3, 3);
                //gl->addWidget(cbOffX,               4, 1, Qt::AlignHCenter);
                //gl->addWidget(cbOffY,               4, 2, Qt::AlignHCenter);
                //gl->addWidget(cbOffZ,               4, 3, Qt::AlignHCenter);
                //gl->addWidget(new QLabel("aaaaaaa"), 5, 1, 1, 3, Qt::AlignCenter);
            fr3D->setLayout(gl);
        layMain->addWidget(fr3D);
        fr3D->setVisible(false);

        QFrame * frOpt3D = new QFrame();
            QHBoxLayout * layOpt = new QHBoxLayout(); layOpt->setContentsMargins(0,0,0,0);
                cbCalRandomize = new QCheckBox("Random bin along step");
                layOpt->addWidget(cbCalRandomize, 0, Qt::AlignCenter);
            frOpt3D->setLayout(layOpt);
        layMain->addWidget(frOpt3D);
        frOpt3D->setVisible(false);

        connect(cobCalType, &QComboBox::currentIndexChanged, this, [this, frOverEvents, fr3D, frOpt3D](int index)
                {
                    frOverEvents->setVisible(index == 0);
                    fr3D->setVisible(index != 0);
                    frOpt3D->setVisible(index != 0);

                    if (index == 1) // energy
                    {
                        cbOffX->setEnabled(true);
                        cbOffY->setEnabled(true);
                        cbOffZ->setEnabled(true);
                    }
                    else if (index == 2) // dose
                    {
                        cbOffX->setChecked(false); cbOffX->setEnabled(false);
                        cbOffY->setChecked(false); cbOffY->setEnabled(false);
                        cbOffZ->setChecked(false); cbOffZ->setEnabled(false);
                    }
                } );

        connect(labX, &QLabel::customContextMenuRequested, this, [this]()
                {
                    if (cobCalType->currentIndex() != 2)
                    {
                        ledCalOriginX->setText("-1e+10");
                        ledCalStepX->setText("2e+10");
                        leiCalBinsX->setText("1");
                    }
                });
        connect(labY, &QLabel::customContextMenuRequested, this, [this]()
                {
                    if (cobCalType->currentIndex() != 2)
                    {
                        ledCalOriginY->setText("-1e+10");
                        ledCalStepY->setText("2e+10");
                        leiCalBinsY->setText("1");
                    }
                });
        connect(labZ, &QLabel::customContextMenuRequested, this, [this]()
                {
                    if (cobCalType->currentIndex() != 2)
                    {
                        ledCalOriginZ->setText("-1e+10");
                        ledCalStepZ->setText("2e+10");
                        leiCalBinsZ->setText("1");
                    }
                });

        /*
        connect(cbOffX, &QCheckBox::toggled, this, [this](bool checked)
                {
                    if (checked && cobCalType->currentIndex() == 2)
                    {
                        cbOffX->setChecked(false);
                        return;
                    }
                    ledCalOriginX->setDisabled(checked);
                    ledCalStepX  ->setDisabled(checked);
                    leiCalBinsX  ->setDisabled(checked);
                    if (checked)
                    {
                        ledCalOriginX->setText("-1e+10");
                        ledCalStepX->setText("2e+10");
                        leiCalBinsX->setText("1");
                    }
                } );
        connect(cbOffY, &QCheckBox::toggled, this, [this](bool checked)
                {
                    ledCalOriginY->setDisabled(checked);
                    ledCalStepY  ->setDisabled(checked);
                    leiCalBinsY  ->setDisabled(checked);
                    if (checked)
                    {
                        ledCalOriginY->setText("-1e+10");
                        ledCalStepY->setText("2e+10");
                        leiCalBinsY->setText("1");
                    }
                } );
        connect(cbOffZ, &QCheckBox::toggled, this, [this](bool checked)
                {
                    ledCalOriginZ->setDisabled(checked);
                    ledCalStepZ  ->setDisabled(checked);
                    leiCalBinsZ  ->setDisabled(checked);
                    if (checked)
                    {
                        ledCalOriginZ->setText("-1e+10");
                        ledCalStepZ->setText("2e+10");
                        leiCalBinsZ->setText("1");
                    }
                } );
        */

        const std::vector<AOneLineTextEdit*> ole = {ledCalOriginX, ledCalOriginY, ledCalOriginZ,
                                                     ledCalStepX, ledCalStepY, ledCalStepZ,
                                                     leiCalBinsX, leiCalBinsY, leiCalBinsZ,
                                                     leiCalEventDepoBins, ledCalEventDepoFrom, ledCalEventDepoTo};
        for (AOneLineTextEdit * le : ole)
        {
            configureHighligherAndCompleter(le);
            le->setContextMenuPolicy(Qt::NoContextMenu);
            connect(le, &AOneLineTextEdit::textChanged, this, &AGeoObjectDelegate::onContentChanged);
        }
    }

    connect(cobCalType,         &QComboBox::activated, this, &AGeoObjectDelegate::onContentChanged);
    connect(cbCalIncludeHosted, &QCheckBox::clicked,   this, &AGeoObjectDelegate::onContentChanged);
    connect(cbCalRandomize,     &QCheckBox::clicked,   this, &AGeoObjectDelegate::onContentChanged);

    connect(cbOffX,         &QCheckBox::clicked,   this, &AGeoObjectDelegate::onContentChanged);
    connect(cbOffY,         &QCheckBox::clicked,   this, &AGeoObjectDelegate::onContentChanged);
    connect(cbOffZ,         &QCheckBox::clicked,   this, &AGeoObjectDelegate::onContentChanged);

    frCal->setVisible(false);

    return frCal;
}

#include "aphotonfunctionalmodel.h"
#include "afunctionalmodelwidget.h"
QFrame * AGeoObjectDelegate::createFunctionalModelGui()
{
    QFrame * frFun = new QFrame();
    {
        vblPhFun = new QVBoxLayout(frFun); vblPhFun->setContentsMargins(0,0,0,0);

            QHBoxLayout * hl = new QHBoxLayout(); hl->setContentsMargins(15,0,15,0);
            hl->addWidget(new QLabel("Model:"));
                lePhFunModelName = new QLineEdit(); lePhFunModelName->setReadOnly(true);
            hl->addWidget(lePhFunModelName);
                pbSelectPhFunModel = new QPushButton("Select model");
                connect(pbSelectPhFunModel, &QPushButton::clicked, this, &AGeoObjectDelegate::onSelectPhFunModelClicked);
            hl->addWidget(pbSelectPhFunModel);
            hl->addStretch();

        vblPhFun->addLayout(hl);

        LocalPhFunModel = new APFM_Dummy();
        PhFunModelWidget = AFunctionalModelWidget::factory(LocalPhFunModel, Widget);
        vblPhFun->addWidget(PhFunModelWidget);
    }

    frFun->setVisible(false);

    return frFun;
}

QFrame * AGeoObjectDelegate::createParticleAnalyzerGui()
{
    QFrame * frame = new QFrame();
    {
        QVBoxLayout * vbl = new QVBoxLayout(frame); vbl->setContentsMargins(0,0,0,0);
        PartAnWidget = new AParticleAnalyzerWidget();
        AGeoParticleAnalyzer tmp;
        PartAnWidget->updateGui(tmp);
        vbl->addWidget(PartAnWidget);
        connect(PartAnWidget, &AParticleAnalyzerWidget::contentChanged, this, &AGeoObjectDelegate::onContentChanged);
    }

    frame->setVisible(false);

    return frame;
}

#include "atreedatabaseselectordialog.h"
void AGeoObjectDelegate::onSelectPhFunModelClicked()
{
    ATreeDatabaseSelectorDialog dialog("Select model", Widget);
    QString err = dialog.readData(AFunctionalModelWidget::getModelDatabase());
    if (!err.isEmpty())
    {
        guitools::message(err, Widget);
        return;
    }

    int res = dialog.exec();
    if (res == QDialog::Accepted)
    {
        APhotonFunctionalModel * model = APhotonFunctionalModel::factory(dialog.SelectedItem);
        if (model) LocalPhFunModel = model;
        else guitools::message("Model selection resulted in unknown model name");

        updatePhFunModelGui();
        onContentChanged();
    }
}

void AGeoObjectDelegate::updatePhFunModelGui()
{
    lePhFunModelName->setText(LocalPhFunModel->getType());

    vblPhFun->removeWidget(PhFunModelWidget);
    delete PhFunModelWidget; PhFunModelWidget = nullptr;

    PhFunModelWidget = AFunctionalModelWidget::factory(LocalPhFunModel, Widget);
    connect(PhFunModelWidget, &AFunctionalModelWidget::modified, this, &AGeoObjectDelegate::onContentChanged);
    connect(PhFunModelWidget, &AFunctionalModelWidget::requestDraw, this, &AGeoObjectDelegate::requestDraw);
    vblPhFun->insertWidget(1, PhFunModelWidget);
}

QString AGeoObjectDelegate::getName() const
{
    return leName->text();
}

#include "ageometryhub.h"
#include "afunctionalmodelwidget.h"
bool AGeoObjectDelegate::updateObject(AGeoObject * obj) const  //react to false in void AGeoWidget::onConfirmPressed()
{
    QString errorStr;
    const QString oldName = obj->Name;
    const QString newName = leName->text();

    if (obj->Type->isHandlingSet() && !obj->Type->isStack())
    {
        //set container object does not have updateable properties except name
        obj->Name = newName;
    }
    else
    {
        // doing tests, if failed, return before assigning anything to the object!
        AGeoShape * shape = ShapeCopy;
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        if (scaled)
        {
            scaled->strScaleX = ledScaleX->text();
            scaled->strScaleY = ledScaleY->text();
            scaled->strScaleZ = ledScaleZ->text();

            if (scaled->strScaleX.isEmpty() || scaled->strScaleY.isEmpty() || scaled->strScaleZ.isEmpty())
            {
                QMessageBox::warning(this->ParentWidget, "", "Empty line!");
                return false;
            }

            scaled->updateScalingFactors(errorStr);
            if (!errorStr.isEmpty())
            {
                QMessageBox::warning(this->ParentWidget, "", errorStr);
                return false;
            }
            shape = scaled->BaseShape;
        }

        if (shape)
        {
            //errorStr.clear();
            shape->introduceGeoConstValues(errorStr);
            if (!errorStr.isEmpty())
            {
                QMessageBox::warning(this->ParentWidget, "", errorStr);
                return false;
            }
        }
        else
        {
            qWarning() << "Something went very wrong, ShapeCopy not found";
            return false;
        }

        //std::vector<double> old =
        //{ obj->Position[0],    obj->Position[1],    obj->Position[2],
        //  obj->Orientation[0], obj->Orientation[1], obj->Orientation[2] };

        std::vector<QString> tempStrs(6);
        std::vector<double> tempDoubles(6);
        bool ok = true;
        ok = ok && processEditBox("X position", ledX,     tempDoubles[0],    tempStrs[0],    ParentWidget);
        ok = ok && processEditBox("Y position", ledY,     tempDoubles[1],    tempStrs[1],    ParentWidget);
        ok = ok && processEditBox("Z position", ledZ,     tempDoubles[2],    tempStrs[2],    ParentWidget);
        if (ledPhi->isEnabled())   ok = ok && processEditBox("Phi orientation",   ledPhi,   tempDoubles[3], tempStrs[3], ParentWidget);
        if (ledTheta->isEnabled()) ok = ok && processEditBox("Theta orientation", ledTheta, tempDoubles[4], tempStrs[4], ParentWidget);
        if (ledPsi->isEnabled())   ok = ok && processEditBox("Psi orientation",   ledPsi,   tempDoubles[5], tempStrs[5], ParentWidget);
        if (!ok) return false;

        std::vector<double>  calDouble(6); calDouble = {-5.0, -5.0, -5.0, 1.0, 1.0, 1.0};
        std::vector<QString> calDoubleStr(6);
        std::vector<int>     calInt(3); calInt = {10,10,10};
        std::vector<QString> calIntStr(3);
        int calEventDepoBins = 190; QString calEventDepoBinsStr;
        double calEventDepoFrom = 100.0; QString calEventDepoFromStr;
        double calEventDepoTo = 2000.0; QString calEventDepoToStr;
        if (cobRole->currentIndex() == 2)
        {
            if (cobCalType->currentIndex() == 0)
            {
                ok = ok && processIntEditBox("Event energy depo bins", leiCalEventDepoBins, calEventDepoBins, calEventDepoBinsStr,  true, true,  ParentWidget);
                ok = ok && processDoubleEditBox("Event energy depo from", ledCalEventDepoFrom, calEventDepoFrom, calEventDepoFromStr,  false, true, false,  ParentWidget);
                ok = ok && processDoubleEditBox("Event energy depo to",   ledCalEventDepoTo,   calEventDepoTo,   calEventDepoToStr,    false, true, false,  ParentWidget);
            }
            else
            {
                ok = ok && processDoubleEditBox("Origin X", ledCalOriginX, calDouble[0], calDoubleStr[0],  false,false,false,  ParentWidget);
                ok = ok && processDoubleEditBox("Origin Y", ledCalOriginY, calDouble[1], calDoubleStr[1],  false,false,false,  ParentWidget);
                ok = ok && processDoubleEditBox("Origin Z", ledCalOriginZ, calDouble[2], calDoubleStr[2],  false,false,false,  ParentWidget);
                ok = ok && processDoubleEditBox("Step X", ledCalStepX,   calDouble[3], calDoubleStr[3],  true, true, false,  ParentWidget);
                ok = ok && processDoubleEditBox("Step Y", ledCalStepY,   calDouble[4], calDoubleStr[4],  true, true, false,  ParentWidget);
                ok = ok && processDoubleEditBox("Step Z", ledCalStepZ,   calDouble[5], calDoubleStr[5],  true, true, false,  ParentWidget);
                ok = ok && processIntEditBox("Bins X", leiCalBinsX, calInt[0], calIntStr[0],  true, true,  ParentWidget);
                ok = ok && processIntEditBox("Bins Y", leiCalBinsY, calInt[1], calIntStr[1],  true, true,  ParentWidget);
                ok = ok && processIntEditBox("Bins Z", leiCalBinsZ, calInt[2], calIntStr[2],  true, true,  ParentWidget);
            }

            if (!ok) return false;

            if (cobCalType->currentIndex() == 2)
            {
                bool badForDose = false;
                for (int iA = 0; iA < 3; iA++)
                    if (calDouble[iA] == -1e+10 && calDouble[3+iA] == 2e+10 && calInt[iA] == 1)
                    {
                        badForDose = true;
                        break;
                    }
                if (badForDose)
                {
                    QMessageBox::warning(this->ParentWidget, "Warning", "For meaningful dose data, configure adequate voxel dimensions!");
                    return false;
                }
            }
        }

        if (cobRole->currentIndex() == 5)
        {
            if (lePhFunModelName->text().isEmpty())
            {
                QMessageBox::warning(ParentWidget, "Warning", "Select default model!");
                return false;
            }

            PhFunModelWidget->updateModel(LocalPhFunModel);
            QString err = LocalPhFunModel->updateRuntimeProperties();
            if (!err.isEmpty())
            {
                QMessageBox::warning(ParentWidget, "Warning", "Error in functional model:\n" + err);
                return false;
            }
        }

        if (cobRole->currentIndex() == 6)
        {
            QString err = PartAnWidget->check();
            if (!err.isEmpty())
            {
                QMessageBox::warning(ParentWidget, "Warning", err);
                return false;
            }
        }

        // ---- all checks are ok, can assign new values to the object ----

        obj->Name = newName;

        obj->Material = cobMat->currentIndex();
        if (obj->Material == -1) obj->Material = 0; //protection

        //inherit materials for composite members
        if (obj->isCompositeMemeber())
        {
            if (obj->Container && obj->Container->Container)
                obj->Material = obj->Container->Container->Material;
        }

        delete obj->Shape; obj->Shape = ShapeCopy->clone();

        for (int i = 0; i < 3; i++)
        {
            obj->PositionStr[i] = tempStrs[i];
            obj->OrientationStr[i] = tempStrs[i+3];

            obj->Position[i] = tempDoubles[i];
            obj->Orientation[i] = tempDoubles[i+3];
        }

        /*
        if (obj->Container && obj->Container->Type->isStack())
        {
            ATypeStackContainerObject * StackTypeObj = static_cast<ATypeStackContainerObject*>(obj->Container->Type);
            if (oldName == StackTypeObj->ReferenceVolume)
                StackTypeObj->ReferenceVolume = newName;
            obj->Container->updateStack();
        }
        */

        // special role
        delete obj->Role; obj->Role = nullptr;
        if (cobRole)
        {
            switch (cobRole->currentIndex())
            {
            case 1:
                obj->Role = new AGeoSensor(cobSensorModel->currentIndex());
                break;
            case 2:
                {
                AGeoCalorimeter * cal = new AGeoCalorimeter({calDouble[0], calDouble[1], calDouble[2]},
                                                            {calDouble[3], calDouble[4], calDouble[5]},
                                                            {calInt[0], calInt[1], calInt[2]} );
                cal->Properties.strOrigin = {calDoubleStr[0], calDoubleStr[1], calDoubleStr[2]};
                cal->Properties.strStep   = {calDoubleStr[3], calDoubleStr[4], calDoubleStr[5]};
                cal->Properties.strBins   = {calIntStr[0], calIntStr[1], calIntStr[2]};

                //cal->Properties.CollectDepoOverEvent = false;
                switch (cobCalType->currentIndex())
                {
                case 0: cal->Properties.DataType = ACalorimeterProperties::DepoPerEvent; break; // cal->Properties.CollectDepoOverEvent = true;
                case 1: cal->Properties.DataType = ACalorimeterProperties::Energy; break;
                case 2: cal->Properties.DataType = ACalorimeterProperties::Dose;   break;
                default:
                    qWarning() << "Not impelemnted calorimeter type in the combo box";
                    cal->Properties.DataType = ACalorimeterProperties::Energy; break;
                }

                cal->Properties.IncludeHostedVolumes = cbCalIncludeHosted->isChecked();

                cal->Properties.RandomizeBin = cbCalRandomize->isChecked();

                //cal->Properties.CollectDepoOverEvent = cbCalEventStat->isChecked();
                if (cal->Properties.DataType == ACalorimeterProperties::DepoPerEvent)
                {
                    cal->Properties.EventDepoBins = calEventDepoBins; cal->Properties.strEventDepoBins = calEventDepoBinsStr;
                    cal->Properties.EventDepoFrom = calEventDepoFrom; cal->Properties.strEventDepoFrom = calEventDepoFromStr;
                    cal->Properties.EventDepoTo   = calEventDepoTo;   cal->Properties.strEventDepoTo   = calEventDepoToStr;
                }

                obj->Role = cal;
                }
                break;
            case 3:
                obj->Role = new AGeoSecScint();
                break;
            case 4:
                obj->Role = new AGeoScint();
                break;
            case 5:
                obj->Role = new AGeoPhotonFunctional(*LocalPhFunModel); // cloned, no transfer!
                break;
            case 6:
                {
                    AGeoParticleAnalyzer * pa = new AGeoParticleAnalyzer();
                    PartAnWidget->updateObject(*pa);
                    obj->Role = pa;
                }
                break;
            default:;
            }
        }
    }

    //additional post-processing
    if (obj->Type->isComposite())
    {
        AGeoObject * logicals = obj->getContainerWithLogical();
        if (logicals) logicals->Name = "CompositeSet_"+obj->Name;
    }
    else if (obj->Type->isGrid())
    {
/*        !!!***
        AGeoObject * GE = obj->getGridElement();
        if (GE)
        {
            GE->Name = "GridElement_" + obj->Name;
            GE->Material = obj->Material;
        }
*/
    }
    else if (obj->isCompositeMemeber())
        obj->updateNameOfLogicalMember(oldName, newName);

    return true;
}

bool AGeoObjectDelegate::processDoubleEditBox(const QString & whatIsIt, AOneLineTextEdit * lineEdit, double & val, QString & str,
                                              bool bForbidZero, bool bForbidNegative, bool bMakeHalf,
                                              QWidget * parent)
{
    str = lineEdit->text();
    if (str.isEmpty())
    {
        QMessageBox::warning(parent, "", "Empty line!");
        return false;
    }

    const AGeoConsts & GC = AGeoConsts::getConstInstance();
    QString errorStr;
    bool ok = GC.updateDoubleParameter(errorStr, str, val, bForbidZero, bForbidNegative, bMakeHalf);
    if (ok) return true;
    QMessageBox::warning(parent, "", errorStr + " in " + whatIsIt + "\n");
    return false;
}

bool AGeoObjectDelegate::processIntEditBox(const QString & whatIsIt, AOneLineTextEdit * lineEdit, int & val, QString & str,
                                          bool bForbidZero, bool bForbidNegative,
                                          QWidget * parent)
{
    str = lineEdit->text();
    if (str.isEmpty())
    {
        QMessageBox::warning(parent, "", "Empty line!");
        return false;
    }

    const AGeoConsts & GC = AGeoConsts::getConstInstance();
    QString errorStr;
    bool ok = GC.updateIntParameter(errorStr, str, val, bForbidZero, bForbidNegative);
    if (ok) return true;
    QMessageBox::warning(parent, "", errorStr + " in " + whatIsIt + "\n");
    return false;
}

void AGeoObjectDelegate::onChangeShapePressed()
{
    QDialog * d = new QDialog(ParentWidget);
    d->setWindowTitle("Select new shape");

    QVBoxLayout * l = new QVBoxLayout(d);
        QListWidget * w = new QListWidget();
        w->addItems(ListOfShapesForTransform);
    l->addWidget(w);
    d->resize(d->width(), 400);

        QHBoxLayout * h = new QHBoxLayout();
            QPushButton * pbAccept = new QPushButton("Change shape");
            pbAccept->setEnabled(false);
            QObject::connect(pbAccept, &QPushButton::clicked, [this, d, w](){this->onShapeDialogActivated(d, w);});
            QObject::connect(w, &QListWidget::activated, [this, d, w](){this->onShapeDialogActivated(d, w);});
            h->addWidget(pbAccept);
            QPushButton * pbCancel = new QPushButton("Cancel");
            QObject::connect(pbCancel, &QPushButton::clicked, d, &QDialog::reject);
            h->addWidget(pbCancel);
    l->addLayout(h);

    l->addWidget(new QLabel("Warning! There is no undo after change!"));

    QObject::connect(w, &QListWidget::itemSelectionChanged, [pbAccept, w]()
    {
        pbAccept->setEnabled(w->currentRow() != -1);
    });

    d->exec();
    delete d;
}

void AGeoObjectDelegate::onScaleToggled()
{
    AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
    if (scaled)
    {
        qDebug() << "Convering scaled to base shape!";
        ShapeCopy = scaled->BaseShape;
        scaled->BaseShape = nullptr;
        delete scaled;
    }
    else
    {
        qDebug() << "Converting shape to scaled!";
        scaled = new AGeoScaledShape();
        scaled->BaseShape = ShapeCopy;
        ShapeCopy = scaled;
    }
}

void AGeoObjectDelegate::addLocalLayout(QLayout * lay)
{
    lMF->insertLayout(2, lay);
}

QString AGeoObjectDelegate::updateScalingFactors() const //not needed anymore need to kill as well as pteshape
{
    QString errorStr;
    AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
    if (scaled)
    {
        scaled->strScaleX = ledScaleX->text();
        scaled->strScaleY = ledScaleY->text();
        scaled->strScaleZ = ledScaleZ->text();

        const AGeoConsts & GC = AGeoConsts::getConstInstance();
        bool ok;
        ok = GC.updateDoubleParameter(errorStr, scaled->strScaleX, scaled->scaleX, true, true, false); if (!ok) return errorStr;
        ok = GC.updateDoubleParameter(errorStr, scaled->strScaleY, scaled->scaleY, true, true, false); if (!ok) return errorStr;
        ok = GC.updateDoubleParameter(errorStr, scaled->strScaleZ, scaled->scaleZ, true, true, false); if (!ok) return errorStr;
        //qDebug() <<scaled->scaleX <<scaled->scaleY <<scaled->scaleZ;
    }
    else
    {
        errorStr = "UpdateScalingFactors: Scaled shape not found!";
        qWarning() << errorStr;
    }
    return errorStr;
}

const AGeoShape * AGeoObjectDelegate::getBaseShapeOfObject(const AGeoObject * obj)
{
    if (!obj || !obj->Shape) return nullptr;
    AGeoScaledShape * scaledShape = dynamic_cast<AGeoScaledShape*>(obj->Shape);
    if (!scaledShape) return nullptr;

    AGeoShape * baseShape = AGeoShape::GeoShapeFactory(scaledShape->getBaseShapeType());
    bool bOK = baseShape->readFromString( scaledShape->BaseShapeGenerationString );
    if (!bOK) qDebug() << "Failed to read shape properties:" << scaledShape->BaseShapeGenerationString;
    return baseShape;
}

void AGeoObjectDelegate::updateTypeLabel()
{
    if (CurrentObject->Type->isGrid())
        DelegateTypeName = "Grid bulk, " + DelegateTypeName;

    if (CurrentObject->isCompositeMemeber())
        DelegateTypeName += " (logical)";
    else if (CurrentObject->Container)
    {
        if (CurrentObject->Container->Type->isHandlingSet())
        {
            if (CurrentObject->Container->Type->isStack())
            {
                DelegateTypeName += ",   stacked";
                if (CurrentObject->isStackReference())
                    DelegateTypeName += ",  stack reference";
            }
        }
    }

    labType->setText(DelegateTypeName);
}

void AGeoObjectDelegate::updateControlUI()
{
    if (CurrentObject->Type->isHandlingSet())
    {
        lMat->setVisible(false);
        cobMat->setVisible(false);
        PosOrient->setVisible(CurrentObject->Type->isStack());
    }

    if (CurrentObject->Container && CurrentObject->Container->Type->isStack())
    {
        ledPhi->setEnabled(false);   //ledPhi->setText("0");
        ledTheta->setEnabled(false); //ledTheta->setText("0");
        ledZ->setEnabled(false);

        if (CurrentObject->isStackReference())
        {
            ledX->setEnabled(false);
            ledY->setEnabled(false);
        }
    }

    if (CurrentObject->isCompositeMemeber())
    {
        cobMat->setEnabled(false);
        cobMat->setCurrentIndex(-1);
    }

    if (CurrentObject->isCompositeMemeber())
    {
        pbShow->setVisible(false);
        pbChangeAtt->setVisible(false);
        pbScriptLine->setVisible(false);
    }
}

/*
void AGeoObjectDelegate::rotate(TVector3 & v, double dPhi, double dTheta, double dPsi) const
{
    v.RotateZ( TMath::Pi()/180.0* dPhi );
    TVector3 X(1,0,0);
    X.RotateZ( TMath::Pi()/180.0* dPhi );
    //v.RotateX( TMath::Pi()/180.0* Theta);
    v.Rotate( TMath::Pi()/180.0* dTheta, X);
    TVector3 Z(0,0,1);
    Z.Rotate( TMath::Pi()/180.0* dTheta, X);
    // v.RotateZ( TMath::Pi()/180.0* Psi );
    v.Rotate( TMath::Pi()/180.0* dPsi, Z );
}
*/

void AGeoObjectDelegate::onShapeDialogActivated(QDialog * d, QListWidget * w)
{
    const QString sel = w->currentItem()->text();
    if      (sel == "Box")                  emit RequestChangeShape(new AGeoBox());
    else if (sel == "Parallelepiped")       emit RequestChangeShape(new AGeoPara());
    else if (sel == "Tube")                 emit RequestChangeShape(new AGeoTube());
    else if (sel == "Tube segment")         emit RequestChangeShape(new AGeoTubeSeg());
    else if (sel == "Tube segment cut")     emit RequestChangeShape(new AGeoCtub());
    else if (sel == "Tube elliptical")      emit RequestChangeShape(new AGeoEltu());
    else if (sel == "Sphere")               emit RequestChangeShape(new AGeoSphere());
    else if (sel == "Trap")                 emit RequestChangeShape(new AGeoTrd1());
    else if (sel == "Trap2")                emit RequestChangeShape(new AGeoTrd2());
    else if (sel == "Cone")                 emit RequestChangeShape(new AGeoCone());
    else if (sel == "Cone segment")         emit RequestChangeShape(new AGeoConeSeg());
    else if (sel == "Paraboloid")           emit RequestChangeShape(new AGeoParaboloid());
    else if (sel == "Torus")                emit RequestChangeShape(new AGeoTorus());
    else if (sel == "Polycone")             emit RequestChangeShape(new AGeoPcon());
    else if (sel == "Polygon simplified")   emit RequestChangeShape(new AGeoPolygon());
    else if (sel == "Polygon")              emit RequestChangeShape(new AGeoPgon());
    else if (sel == "Arb8")                 emit RequestChangeShape(new AGeoArb8());

    else if (sel == "Rectangular slab")     emit RequestChangeSlabShape(0);
    else if (sel == "Round slab")           emit RequestChangeSlabShape(1);
    else if (sel == "Polygon slab")         emit RequestChangeSlabShape(2);

    else
    {
        //nothing selected or unknown shape
        return;
    }
    d->accept();
}

void AGeoObjectDelegate::onHelpRequested()
{
    guitools::message(ShapeHelp, ParentWidget);
}

#include <QJsonObject>
void AGeoObjectDelegate::updateGui(const AGeoObject *obj)
{
    //qDebug() << "update----------------" << obj->Name;
    CurrentObject = obj;
    leName->setText(obj->Name);

    delete ShapeCopy; ShapeCopy = obj->Shape->clone();

    //qDebug() << "--genstring:original/copy->"<<obj->Shape->getGenerationString() << ShapeCopy->getGenerationString();

    int imat = obj->Material;
    if (imat < 0 || imat >= cobMat->count())
    {
        qWarning() << "Material index out of bounds!";
        imat = -1;
    }
    cobMat->setCurrentIndex(imat);

    ledX->setText(obj->PositionStr[0].isEmpty() ? QString::number(obj->Position[0]) : obj->PositionStr[0]);
    ledY->setText(obj->PositionStr[1].isEmpty() ? QString::number(obj->Position[1]) : obj->PositionStr[1]);
    ledZ->setText(obj->PositionStr[2].isEmpty() ? QString::number(obj->Position[2]) : obj->PositionStr[2]);

    ledPhi->  setText(obj->OrientationStr[0].isEmpty() ? QString::number(obj->Orientation[0]) : obj->OrientationStr[0]);
    ledTheta->setText(obj->OrientationStr[1].isEmpty() ? QString::number(obj->Orientation[1]) : obj->OrientationStr[1]);
    ledPsi->  setText(obj->OrientationStr[2].isEmpty() ? QString::number(obj->Orientation[2]) : obj->OrientationStr[2]);

    updateTypeLabel();
    updateControlUI();

    updateLineColorFrame(obj);

    AGeoScaledShape * scaledShape = dynamic_cast<AGeoScaledShape*>(CurrentObject->Shape);
    cbScale->setChecked(scaledShape);
    if (scaledShape)
    {
        ledScaleX->setText(scaledShape->strScaleX.isEmpty() ? QString::number(scaledShape->scaleX) : scaledShape->strScaleX);
        ledScaleY->setText(scaledShape->strScaleY.isEmpty() ? QString::number(scaledShape->scaleY) : scaledShape->strScaleY);
        ledScaleZ->setText(scaledShape->strScaleZ.isEmpty() ? QString::number(scaledShape->scaleZ) : scaledShape->strScaleZ);
    }

    const bool bCanHaveRole = (obj->Type->isSingle() || obj->Type->isComposite());
    RoleWidget->setVisible(bCanHaveRole);
    if (bCanHaveRole && obj->Role)
    {
        AGeoSensor * sens = dynamic_cast<AGeoSensor*>(obj->Role);
        if (sens)
        {
            cobRole->setCurrentIndex(1);
            cobSensorModel->setCurrentIndex(sens->SensorModel);
        }
        else
        {
            AGeoCalorimeter * cal = dynamic_cast<AGeoCalorimeter*>(obj->Role);
            if (cal)
            {
                cobRole->setCurrentIndex(2);
                const ACalorimeterProperties & p = cal->Properties;
                updateCalorimeterGui(p);
            }
            else
            {
                AGeoSecScint * sec = dynamic_cast<AGeoSecScint*>(obj->Role);
                if (sec) cobRole->setCurrentIndex(3);
                else
                {
                    AGeoScint * scint = dynamic_cast<AGeoScint*>(obj->Role);
                    if (scint) cobRole->setCurrentIndex(4);
                    else
                    {
                        AGeoPhotonFunctional * phFunct = dynamic_cast<AGeoPhotonFunctional*>(obj->Role);
                        if (phFunct)
                        {
                            cobRole->setCurrentIndex(5);
                            QJsonObject js;
                            phFunct->DefaultModel->writeToJson(js);
                            delete LocalPhFunModel; LocalPhFunModel = nullptr;
                            LocalPhFunModel = APhotonFunctionalModel::factory(js);
                            updatePhFunModelGui();
                        }
                        else
                        {
                            AGeoParticleAnalyzer * partAn = dynamic_cast<AGeoParticleAnalyzer*>(obj->Role);
                            if (partAn)
                            {
                                cobRole->setCurrentIndex(6);
                                PartAnWidget->updateGui(*partAn);
                            }
                        }
                    }
                }
            }
        }
    }

    if (obj->isPrototypeMember()) pbShow->setEnabled(false);
}

void AGeoObjectDelegate::updateCalorimeterGui(const ACalorimeterProperties & p)
{
    int index = 0;
    switch (p.DataType)
    {
    case ACalorimeterProperties::DepoPerEvent : index = 0; break;
    case ACalorimeterProperties::Energy       : index = 1; break;
    case ACalorimeterProperties::Dose         : index = 2; break;
    default:
        qWarning() << "Unknown calorimter mode";
    }
    cobCalType->setCurrentIndex(index);

    cbCalIncludeHosted->setChecked(p.IncludeHostedVolumes);

    cbCalRandomize->setChecked(p.RandomizeBin);

    ledCalOriginX->setText( p.strOrigin[0].isEmpty() ? QString::number(p.Origin[0]) : p.strOrigin[0] );
    ledCalOriginY->setText( p.strOrigin[1].isEmpty() ? QString::number(p.Origin[1]) : p.strOrigin[1] );
    ledCalOriginZ->setText( p.strOrigin[2].isEmpty() ? QString::number(p.Origin[2]) : p.strOrigin[2] );
    ledCalStepX  ->setText(   p.strStep[0].isEmpty() ? QString::number(  p.Step[0]) :   p.strStep[0] );
    ledCalStepY  ->setText(   p.strStep[1].isEmpty() ? QString::number(  p.Step[1]) :   p.strStep[1] );
    ledCalStepZ  ->setText(   p.strStep[2].isEmpty() ? QString::number(  p.Step[2]) :   p.strStep[2] );
    leiCalBinsX  ->setText(   p.strBins[0].isEmpty() ? QString::number(  p.Bins[0]) :   p.strBins[0] );
    leiCalBinsY  ->setText(   p.strBins[1].isEmpty() ? QString::number(  p.Bins[1]) :   p.strBins[1] );
    leiCalBinsZ  ->setText(   p.strBins[2].isEmpty() ? QString::number(  p.Bins[2]) :   p.strBins[2] );

    cbOffX->setChecked( p.isAxisOff(0) );
    cbOffY->setChecked( p.isAxisOff(1) );
    cbOffZ->setChecked( p.isAxisOff(2) );

    //cbCalEventStat->setChecked(p.CollectDepoOverEvent);
    leiCalEventDepoBins->setText(p.strEventDepoBins.isEmpty() ? QString::number(p.EventDepoBins) : p.strEventDepoBins );
    ledCalEventDepoFrom->setText(p.strEventDepoFrom.isEmpty() ? QString::number(p.EventDepoFrom) : p.strEventDepoFrom );
    ledCalEventDepoTo  ->setText(p.strEventDepoTo.isEmpty()   ? QString::number(p.EventDepoTo)   : p.strEventDepoTo );
}

void AGeoObjectDelegate::onContentChanged()
{
    onContentChangedBase();
}

//---------------

AGeoBoxDelegate::AGeoBoxDelegate(const QStringList &materials, QWidget *parent)
    : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Box";

    ShapeHelp = "A box shape\n"
            "\n"
            "Parameters are the full sizes in X, Y and Z direction\n"
            "\n"
            "The XYZ position is given for the center point\n"
            "\n"
            "Implemented using TGeoBBox(0.5*X_size, 0.5*Y_size, 0.5*Z_size)";

    QGridLayout * gr = new QGridLayout();
    gr->setContentsMargins(50, 0, 50, 3);
    gr->setVerticalSpacing(1);

    gr->addWidget(new QLabel("X full size:"), 0, 0);
    gr->addWidget(new QLabel("Y full size:"), 1, 0);
    gr->addWidget(new QLabel("Z full size:"), 2, 0);

    ex = new AOneLineTextEdit(); gr->addWidget(ex, 0, 1);
    ey = new AOneLineTextEdit(); gr->addWidget(ey, 1, 1);
    ez = new AOneLineTextEdit(); gr->addWidget(ez, 2, 1);

    gr->addWidget(new QLabel("mm"), 0, 2);
    gr->addWidget(new QLabel("mm"), 1, 2);
    gr->addWidget(new QLabel("mm"), 2, 2);

    addLocalLayout(gr);

    std::vector<AOneLineTextEdit*> l = {ex, ey, ez};
    for (AOneLineTextEdit * le : l)
    {
        configureHighligherAndCompleter(le);
        QObject::connect(le, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    }
}

bool AGeoBoxDelegate::updateObject(AGeoObject *obj) const
{
    std::vector<AOneLineTextEdit*> v = {ex, ey, ez};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    AGeoBox * box = dynamic_cast<AGeoBox*>(ShapeCopy);
    if (!box)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        box = dynamic_cast<AGeoBox*>(scaled->BaseShape);
    }

    if (box)
    {
        box->str2dx = ex->text();
        box->str2dy = ey->text();
        box->str2dz = ez->text();
    }
    else qWarning() << "Read delegate: Box shape not found!";

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoBoxDelegate::updateGui(const AGeoObject *obj)
{
    AGeoObjectDelegate::updateGui(obj);

    AGeoBox * box = dynamic_cast<AGeoBox*>(ShapeCopy);
    if (!box)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        box = dynamic_cast<AGeoBox*>(scaled->BaseShape);
    }
    if (box)
    {
        ex->setText(box->str2dx.isEmpty() ? QString::number(box->dx*2.0) : box->str2dx);
        ey->setText(box->str2dy.isEmpty() ? QString::number(box->dy*2.0) : box->str2dy);
        ez->setText(box->str2dz.isEmpty() ? QString::number(box->dz*2.0) : box->str2dz);
    }
    else qWarning() << "Update delegate: Box shape not found!";
}

AGeoTubeDelegate::AGeoTubeDelegate(const QStringList & materials, QWidget *parent)
    : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Tube";

    ShapeHelp = "A cylinderical shape\n"
            "\n"
            "Parameters are the outer and inner diameters and the full height\n"
            "\n"
            "The XYZ position is given for the center point\n"
            "\n"
            "Implemented using TGeoTube(0.5*Inner_diameter, 0.5*Outer_diameter, 0.5*Height)";

    gr = new QGridLayout();
    gr->setContentsMargins(50, 0, 50, 3);
    gr->setVerticalSpacing(1);

    labIm = new QLabel("Inner diameter:");
    gr->addWidget(labIm,                         0, 0);
    gr->addWidget(new QLabel("Outer diameter:"), 1, 0);
    gr->addWidget(new QLabel("Height:"),         2, 0);

    ei = new AOneLineTextEdit(); gr->addWidget(ei, 0, 1);
    eo = new AOneLineTextEdit(); gr->addWidget(eo, 1, 1);
    ez = new AOneLineTextEdit(); gr->addWidget(ez, 2, 1);

    labIu = new QLabel("mm");
    gr->addWidget(labIu,            0, 2);
    gr->addWidget(new QLabel("mm"), 1, 2);
    gr->addWidget(new QLabel("mm"), 2, 2);

    addLocalLayout(gr);

    std::vector<AOneLineTextEdit*> l = {ei, eo, ez};
    for (AOneLineTextEdit * le : l)
    {
        configureHighligherAndCompleter(le);
        QObject::connect(le, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    }
}

bool AGeoTubeDelegate::updateObject(AGeoObject *obj) const
{
    std::vector<AOneLineTextEdit*> v = {ei, eo, ez};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    AGeoTube * tube = dynamic_cast<AGeoTube*>(ShapeCopy);
    if (!tube)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        tube = dynamic_cast<AGeoTube*>(scaled->BaseShape);
    }
    if (tube)
    {
        tube->str2rmin = ei->text();
        tube->str2rmax = eo->text();
        tube->str2dz   = ez->text();
    }
    else qWarning() << "Read delegate: Tube shape not found!";

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoTubeDelegate::updateGui(const AGeoObject *obj)
{
    AGeoObjectDelegate::updateGui(obj);

    AGeoTube * tube = dynamic_cast<AGeoTube*>(ShapeCopy);
    if (!tube)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        tube = dynamic_cast<AGeoTube*>(scaled->BaseShape);
    }
    if (tube)
    {
        ei->setText(tube->str2rmin.isEmpty() ? QString::number(tube->rmin*2.0) : tube->str2rmin);
        eo->setText(tube->str2rmax.isEmpty() ? QString::number(tube->rmax*2.0) : tube->str2rmax);
        ez->setText(tube->str2dz.isEmpty()   ? QString::number(tube->dz*2.0)   : tube->str2dz);
    }
    else qWarning() << "Update delegate: Tube shape not found!";
}

AGeoTubeSegDelegate::AGeoTubeSegDelegate(const QStringList & materials, QWidget * parent) :
    AGeoTubeDelegate(materials, parent)
{
    DelegateTypeName = "Tube segment";

    ShapeHelp = "A cylinderical segment shape\n"
            "\n"
            "Parameters are the outer and inner diameters, the full height\n"
            "and the segment angles from and to\n"
            "\n"
            "The XYZ position is given for the center point of the cylinder\n"
            "\n"
            "Implemented using TGeoTubeSeg(0.5*Inner_diameter, 0.5*Outer_diameter, 0.5*Height, phi_from, phi_to)";

    gr->addWidget(new QLabel("Phi from:"), 3, 0);
    gr->addWidget(new QLabel("Phi to:"),   4, 0);

    ep1 = new AOneLineTextEdit(); gr->addWidget(ep1, 3, 1);
    ep2 = new AOneLineTextEdit(); gr->addWidget(ep2, 4, 1);

    gr->addWidget(new QLabel("°"), 3, 2);
    gr->addWidget(new QLabel("°"), 4, 2);

    std::vector<AOneLineTextEdit*> l = {ep1, ep2};
    for (AOneLineTextEdit * le : l)
    {
        configureHighligherAndCompleter(le);
        QObject::connect(le, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    }
}

bool AGeoTubeSegDelegate::updateObject(AGeoObject *obj) const
{
    std::vector<AOneLineTextEdit*> v = {ei, eo, ez, ep1, ep2};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    AGeoTubeSeg * tubeSeg = dynamic_cast<AGeoTubeSeg*>(ShapeCopy);
    if (!tubeSeg)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        tubeSeg = dynamic_cast<AGeoTubeSeg*>(scaled->BaseShape);
    }

    if (tubeSeg)
    {
        tubeSeg->str2rmin = ei ->text();
        tubeSeg->str2rmax = eo ->text();
        tubeSeg->str2dz   = ez ->text();
        tubeSeg->strPhi1 = ep1->text();
        tubeSeg->strPhi2 = ep2->text();
    }
    else qWarning() << "Read delegate: Tube Segment shape not found!";

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoTubeSegDelegate::updateGui(const AGeoObject *obj)
{
    AGeoObjectDelegate::updateGui(obj);

    AGeoTubeSeg * tubeSeg = dynamic_cast<AGeoTubeSeg*>(ShapeCopy);
    if (!tubeSeg)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        tubeSeg = dynamic_cast<AGeoTubeSeg*>(scaled->BaseShape);
    }

    if (tubeSeg)
    {
        ei ->setText(tubeSeg->str2rmin.isEmpty() ? QString::number(tubeSeg->rmin*2.0) : tubeSeg->str2rmin);
        eo ->setText(tubeSeg->str2rmax.isEmpty() ? QString::number(tubeSeg->rmax*2.0) : tubeSeg->str2rmax);
        ez ->setText(tubeSeg->str2dz  .isEmpty() ? QString::number(tubeSeg->dz  *2.0) : tubeSeg->str2dz);
        ep1->setText(tubeSeg->strPhi1.isEmpty()  ? QString::number(tubeSeg->phi1)     : tubeSeg->strPhi1);
        ep2->setText(tubeSeg->strPhi2.isEmpty()  ? QString::number(tubeSeg->phi2)     : tubeSeg->strPhi2);
    }
    else qWarning() << "Read delegate: Tube Segment shape not found!";
}

AGeoTubeSegCutDelegate::AGeoTubeSegCutDelegate(const QStringList &materials, QWidget *parent) :
    AGeoTubeSegDelegate(materials, parent)
{
    DelegateTypeName = "Tube segment cut";

    ShapeHelp = "A cylinderical segment cut shape\n"
            "\n"
            "Parameters are the outer and inner diameters,\n"
            "the full height in Z direction (from -dz to +dz in local coordinates),\n"
            "the segment angles from and to,\n"
            "and the unit vectors (Nx, Ny, Ny) of the normals for the lower\n"
            "and the upper faces at (0,0,-dz) and (0,0,+dz) local coordinates\n"
            "\n"
            "The XYZ position is given for the point with (0,0,0) local coordinates\n"
            "\n"
            "Implemented using TGeoCtub(0.5*Inner_diameter, 0.5*Outer_diameter, 0.5*Height, phi_from, phi_to, LNx, LNy, LNz, UNx, UNy, UNz)";

    gr->addWidget(new QLabel("Low Nx:"), 5, 0);
    gr->addWidget(new QLabel("Low Ny:"), 6, 0);
    gr->addWidget(new QLabel("Low Nz:"), 7, 0);
    gr->addWidget(new QLabel("Up  Nx:"), 8, 0);
    gr->addWidget(new QLabel("Up  Ny:"), 9, 0);
    gr->addWidget(new QLabel("Up  Nz:"), 10, 0);

    elnx = new AOneLineTextEdit(); gr->addWidget(elnx, 5, 1);
    elny = new AOneLineTextEdit(); gr->addWidget(elny, 6, 1);
    elnz = new AOneLineTextEdit(); gr->addWidget(elnz, 7, 1);
    eunx = new AOneLineTextEdit(); gr->addWidget(eunx, 8, 1);
    euny = new AOneLineTextEdit(); gr->addWidget(euny, 9, 1);
    eunz = new AOneLineTextEdit(); gr->addWidget(eunz, 10, 1);

    for (AOneLineTextEdit * le : {elnx, elny, elnz, eunx, euny, eunz})
    {
        configureHighligherAndCompleter(le);
        QObject::connect(le, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    }
}

bool AGeoTubeSegCutDelegate::updateObject(AGeoObject *obj) const
{
    std::vector<AOneLineTextEdit*> v = {elnx, elny, elnz, eunx, euny, eunz};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    AGeoCtub * ctube = dynamic_cast<AGeoCtub*>(ShapeCopy);
    if (!ctube)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        ctube = dynamic_cast<AGeoCtub*>(scaled->BaseShape);
    }

    if (ctube)
    {
        ctube->strnxlow = elnx->text();
        ctube->strnylow = elny->text();
        ctube->strnzlow = elnz->text();
        ctube->strnxhi  = eunx->text();
        ctube->strnyhi  = euny->text();
        ctube->strnzhi  = eunz->text();
    }
    else qWarning() << "Read delegate: Tube segment cut shape not found!";

    return AGeoTubeSegDelegate::updateObject(obj); // AGeoTubeSeg for the rest of the properties
}

void AGeoTubeSegCutDelegate::updateGui(const AGeoObject *obj)
{
    AGeoTubeSegDelegate::updateGui(obj);

    AGeoCtub * ctube = dynamic_cast<AGeoCtub*>(ShapeCopy);
    if (!ctube)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        ctube = dynamic_cast<AGeoCtub*>(scaled->BaseShape);
    }

    if (ctube)
    {
        elnx->setText(ctube->strnxlow.isEmpty() ? QString::number(ctube->nxlow) : ctube->strnxlow);
        elny->setText(ctube->strnylow.isEmpty() ? QString::number(ctube->nylow) : ctube->strnylow);
        elnz->setText(ctube->strnzlow.isEmpty() ? QString::number(ctube->nzlow) : ctube->strnzlow);
        eunx->setText(ctube->strnxhi .isEmpty() ? QString::number(ctube->nxhi)  : ctube->strnxhi);
        euny->setText(ctube->strnyhi .isEmpty() ? QString::number(ctube->nyhi)  : ctube->strnyhi);
        eunz->setText(ctube->strnzhi .isEmpty() ? QString::number(ctube->nzhi)  : ctube->strnzhi);
    }
    else qWarning() << "Read delegate: Tube segment cut shape not found!";
}

AGeoParaDelegate::AGeoParaDelegate(const QStringList & materials, QWidget *parent)
    : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Parallelepiped";

    ShapeHelp = "A parallelepiped is a shape having 3 pairs of parallel faces\n"
                "  out of which one is parallel with the XY plane (Z face).\n"
                "All faces are parallelograms. The Z faces have 2 edges\n"
                "  parallel to the X-axis.\n"
                "\n"
                "The shape has the center in the origin and is defined by\n"
                "  the full lengths of the projections of the edges on X, Y and Z.\n"
                "The lower Z face is positioned at -0.5*Zsize,\n"
                "  while the upper at +0.5*Zsize.\n"
                "Alpha: angle between the segment defined by the centers of the\n"
                "  X-parallel edges and Y axis [-90,90] in degrees\n"
                "Theta: theta angle of the segment defined by the centers of the Z faces\n"
                "Phi: phi angle of the same segment\n"
                "\n"
                "Implemented using TGeoPara(0.5*X_size, 0.5*Y_size, 0.5*Z_size, Alpha, Theta, Phi)";

    QGridLayout * gr = new QGridLayout();
    gr->setContentsMargins(50, 0, 50, 3);
    gr->setVerticalSpacing(1);

    gr->addWidget(new QLabel("X full size:"), 0, 0);
    gr->addWidget(new QLabel("Y full size:"), 1, 0);
    gr->addWidget(new QLabel("Z full size:"), 2, 0);
    gr->addWidget(new QLabel("Alpha:"),     3, 0);
    gr->addWidget(new QLabel("Theta:"),     4, 0);
    gr->addWidget(new QLabel("Phi:"),       5, 0);

    ex = new AOneLineTextEdit(); gr->addWidget(ex, 0, 1);
    ey = new AOneLineTextEdit(); gr->addWidget(ey, 1, 1);
    ez = new AOneLineTextEdit(); gr->addWidget(ez, 2, 1);
    ea = new AOneLineTextEdit(); gr->addWidget(ea, 3, 1);
    et = new AOneLineTextEdit(); gr->addWidget(et, 4, 1);
    ep = new AOneLineTextEdit(); gr->addWidget(ep, 5, 1);

    gr->addWidget(new QLabel("mm"), 0, 2);
    gr->addWidget(new QLabel("mm"), 1, 2);
    gr->addWidget(new QLabel("mm"), 2, 2);
    gr->addWidget(new QLabel("°"),  3, 2);
    gr->addWidget(new QLabel("°"),  4, 2);
    gr->addWidget(new QLabel("°"),  5, 2);

    addLocalLayout(gr);

    std::vector<AOneLineTextEdit*> l = {ex, ey, ez, ea, et, ep};
    for (AOneLineTextEdit * le : l)
    {
        configureHighligherAndCompleter(le);
        QObject::connect(le, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    }
}

bool AGeoParaDelegate::updateObject(AGeoObject *obj) const
{
    std::vector<AOneLineTextEdit*> v = {ex, ey, ez, ea, et, ep};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    AGeoPara * para = dynamic_cast<AGeoPara*>(ShapeCopy);
    if (!para)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        para = dynamic_cast<AGeoPara*>(scaled->BaseShape);
    }

    if (para)
    {
        para->str2dx   = ex->text();
        para->str2dy   = ey->text();
        para->str2dz   = ez->text();
        para->strAlpha = ea->text();
        para->strTheta = et->text();
        para->strPhi   = ep->text();
    }
    else qWarning() << "Read delegate: Parallelepiped shape not found!";

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoParaDelegate::updateGui(const AGeoObject *obj)
{
    AGeoObjectDelegate::updateGui(obj);

    AGeoPara * para = dynamic_cast<AGeoPara*>(ShapeCopy);
    if (!para)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        para = dynamic_cast<AGeoPara*>(scaled->BaseShape);
    }

    if (para)
    {
        ex->setText(para->str2dx  .isEmpty() ? QString::number(para->dx*2.0) : para->str2dx);
        ey->setText(para->str2dy  .isEmpty() ? QString::number(para->dy*2.0) : para->str2dy);
        ez->setText(para->str2dz  .isEmpty() ? QString::number(para->dz*2.0) : para->str2dz);
        ea->setText(para->strAlpha.isEmpty() ? QString::number(para->alpha)  : para->strAlpha);
        et->setText(para->strTheta.isEmpty() ? QString::number(para->theta)  : para->strTheta);
        ep->setText(para->strPhi  .isEmpty() ? QString::number(para->phi)    : para->strPhi);
    }
    else qWarning() << "Read delegate: Box shape not found!";
}

AGeoSphereDelegate::AGeoSphereDelegate(const QStringList & materials, QWidget *parent)
    : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Sphere";

    ShapeHelp = "A spherical sector, defined by\n"
                "the external and internal diameters,\n"
                "and two pairs of polar angles (in degrees):\n"
                "\n"
                "Theta from: [0, 180)\n"
                "Theta to:   (0, 180] with a condition theta_from < theta_to\n"
                "\n"
                "Phi from: [0, 360)\n"
                "Phi to:   (0, 360] with a condition phi_from < phi_to\n"
                "\n"
                "Implemented using TGeoSphere(0.5*Inner_Diameter, 0.5*Outer_Diameter, ThetaFrom, ThetaTo, PhiFrom, PhiTo)";

    QGridLayout * gr = new QGridLayout();
    gr->setContentsMargins(50, 0, 50, 3);
    gr->setVerticalSpacing(1);

    gr->addWidget(new QLabel("Outer diameter:"), 0, 0);
    gr->addWidget(new QLabel("Inner diameter:"), 1, 0);
    gr->addWidget(new QLabel("Theta from:"), 2, 0);
    gr->addWidget(new QLabel("Theta to:"),     3, 0);
    gr->addWidget(new QLabel("Phi from:"),     4, 0);
    gr->addWidget(new QLabel("Phi to:"),       5, 0);

    eod = new AOneLineTextEdit(); gr->addWidget(eod, 0, 1);
    eid = new AOneLineTextEdit(); gr->addWidget(eid, 1, 1);
    et1 = new AOneLineTextEdit(); gr->addWidget(et1, 2, 1);
    et2 = new AOneLineTextEdit(); gr->addWidget(et2, 3, 1);
    ep1 = new AOneLineTextEdit(); gr->addWidget(ep1, 4, 1);
    ep2 = new AOneLineTextEdit(); gr->addWidget(ep2, 5, 1);

    gr->addWidget(new QLabel("mm"), 0, 2);
    gr->addWidget(new QLabel("mm"), 1, 2);
    gr->addWidget(new QLabel("°"),  2, 2);
    gr->addWidget(new QLabel("°"),  3, 2);
    gr->addWidget(new QLabel("°"),  4, 2);
    gr->addWidget(new QLabel("°"),  5, 2);

    addLocalLayout(gr);

    std::vector<AOneLineTextEdit*> l = {eod, eid, et1, et2, ep1, ep2};
    for (AOneLineTextEdit * le : l)
    {
        configureHighligherAndCompleter(le);
        QObject::connect(le, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    }
}

bool AGeoSphereDelegate::updateObject(AGeoObject *obj) const
{
    std::vector<AOneLineTextEdit*> v = {eod, eid, et1, et2, ep1, ep2};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    AGeoSphere * sphere = dynamic_cast<AGeoSphere*>(ShapeCopy);
    if (!sphere)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        sphere = dynamic_cast<AGeoSphere*>(scaled->BaseShape);
    }
    if (sphere)
    {
        sphere->str2rmax  = eod->text();
        sphere->str2rmin  = eid->text();
        sphere->strTheta1 = et1->text();
        sphere->strTheta2 = et2->text();
        sphere->strPhi1   = ep1->text();
        sphere->strPhi2   = ep2->text();
    }
    else qWarning() << "Read delegate: Sphere shape not found!";

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoSphereDelegate::updateGui(const AGeoObject *obj)
{
    AGeoObjectDelegate::updateGui(obj);

    AGeoSphere * sphere = dynamic_cast<AGeoSphere*>(ShapeCopy);
    if (!sphere)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        sphere = dynamic_cast<AGeoSphere*>(scaled->BaseShape);
    }
    if (sphere)
    {
        eid->setText(sphere->str2rmin.isEmpty()  ? QString::number(sphere->rmin*2.0) : sphere->str2rmin);
        eod->setText(sphere->str2rmax.isEmpty()  ? QString::number(sphere->rmax*2.0) : sphere->str2rmax);
        et1->setText(sphere->strTheta1.isEmpty() ? QString::number(sphere->theta1)   : sphere->strTheta1);
        et2->setText(sphere->strTheta2.isEmpty() ? QString::number(sphere->theta2)   : sphere->strTheta2);
        ep1->setText(sphere->strPhi1.isEmpty()   ? QString::number(sphere->phi1)     : sphere->strPhi1);
        ep2->setText(sphere->strPhi2.isEmpty()   ? QString::number(sphere->phi2)     : sphere->strPhi2);
    }
    else qWarning() << "Update delegate: Sphere shape not found!";
}

AGeoConeDelegate::AGeoConeDelegate(const QStringList &materials, QWidget *parent)
    : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Cone";

    ShapeHelp = "A conical shape with the axis parallel to Z,\n"
                "  limited by two XY planes,\n"
                "  one at Z = -0.5*height, and\n"
                "  the other at Z = +0.5*height.\n"
                "\n"
                "The shape is also defined by two pairs\n"
                " of external/internal diameters,\n"
                " one pair at the lower plane and the other at the upper one.\n"
                "\n"
                "Implemented using TGeoCone(0.5*Height, 0.5*LowerInDiam, 0.5*LowerOutDiam, 0.5*UpperInDiam, 0.5*UpperOutDiam)";

    gr = new QGridLayout();
    gr->setContentsMargins(50, 0, 50, 3);
    gr->setVerticalSpacing(1);

    gr->addWidget(new QLabel("Height:"),               0, 0);
    gr->addWidget(new QLabel("Lower outer diameter:"), 1, 0);
    gr->addWidget(new QLabel("Lower inner diameter:"), 2, 0);
    gr->addWidget(new QLabel("Upper outer diameter:"), 3, 0);
    gr->addWidget(new QLabel("Upper inner diameter:"), 4, 0);

    ez  = new AOneLineTextEdit(); gr->addWidget(ez, 0, 1);
    elo = new AOneLineTextEdit(); gr->addWidget(elo, 1, 1);
    eli = new AOneLineTextEdit(); gr->addWidget(eli, 2, 1);
    euo = new AOneLineTextEdit(); gr->addWidget(euo, 3, 1);
    eui = new AOneLineTextEdit(); gr->addWidget(eui, 4, 1);

    gr->addWidget(new QLabel("mm"), 0, 2);
    gr->addWidget(new QLabel("mm"), 1, 2);
    gr->addWidget(new QLabel("mm"), 2, 2);
    gr->addWidget(new QLabel("mm"), 3, 2);
    gr->addWidget(new QLabel("mm"), 4, 2);

    addLocalLayout(gr);

    for (AOneLineTextEdit * le : {ez, eli, elo, eui, euo})
    {
        configureHighligherAndCompleter(le);
        QObject::connect(le, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    }
}

bool AGeoConeDelegate::updateObject(AGeoObject *obj) const
{
    std::vector<AOneLineTextEdit*> v = {ez, eli, elo, eui, euo};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    AGeoCone * cone = dynamic_cast<AGeoCone*>(ShapeCopy);
    if (!cone)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        cone = dynamic_cast<AGeoCone*>(scaled->BaseShape);
    }

    if (cone)
    {
        cone->str2dz    = ez ->text();
        cone->str2rminL = eli->text();
        cone->str2rmaxL = elo->text();
        cone->str2rminU = eui->text();
        cone->str2rmaxU = euo->text();
    }
    else qWarning() << "Read delegate: Cone shape not found!";

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoConeDelegate::updateGui(const AGeoObject *obj)
{
    AGeoObjectDelegate::updateGui(obj);

    AGeoCone * cone = dynamic_cast<AGeoCone*>(ShapeCopy);
    if (!cone)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        cone = dynamic_cast<AGeoCone*>(scaled->BaseShape);
    }

    if (cone)
    {
        ez ->setText(cone->str2dz.isEmpty()    ? QString::number(cone->dz   *2.0) : cone->str2dz);
        eli->setText(cone->str2rminL.isEmpty() ? QString::number(cone->rminL*2.0) : cone->str2rminL);
        elo->setText(cone->str2rmaxL.isEmpty() ? QString::number(cone->rmaxL*2.0) : cone->str2rmaxL);
        eui->setText(cone->str2rminU.isEmpty() ? QString::number(cone->rminU*2.0) : cone->str2rminU);
        euo->setText(cone->str2rmaxU.isEmpty() ? QString::number(cone->rmaxU*2.0) : cone->str2rmaxU);
    }
    else qWarning() << "Read delegate: Cone shape not found!";
}

AGeoConeSegDelegate::AGeoConeSegDelegate(const QStringList &materials, QWidget *parent)
    : AGeoConeDelegate(materials, parent)
{
    DelegateTypeName = "Cone segment";

    ShapeHelp = "A conical segment shape with the axis parallel to Z,\n"
                "  limited by two XY planes,\n"
                "  one at Z = -0.5*height, and\n"
                "  the other at Z = +0.5*height.\n"
                "\n"
                "The shape is also defined by two pairs\n"
                " of external/internal diameters,\n"
                " one pair at the lower plane and the other at the upper one.\n"
                "\n"
                "The segment angles are in degrees,\n"
                "  Phi from:  in the range [0, 360)\n"
                "  Phi to:    in the ramge (0, 360], should be smaller than Phi_from.\n"
                "\n"
                "Implemented using TGeoCone(0.5*Height, 0.5*LowerInDiam, 0.5*LowerOutDiam, 0.5*UpperInDiam, 0.5*UpperoutDiam, PhiFrom, PhiTo)";

    gr->addWidget(new QLabel("Phi from:"), 5, 0);
    gr->addWidget(new QLabel("Phi to:"),   6, 0);

    ep1 = new AOneLineTextEdit(); gr->addWidget(ep1, 5, 1);
    ep2 = new AOneLineTextEdit(); gr->addWidget(ep2, 6, 1);

    gr->addWidget(new QLabel("°"), 5, 2);
    gr->addWidget(new QLabel("°"), 6, 2);

    std::vector<AOneLineTextEdit*> l = {ep1, ep2};
    for (AOneLineTextEdit * le : l)
    {
        configureHighligherAndCompleter(le);
        QObject::connect(le, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    }
}

bool AGeoConeSegDelegate::updateObject(AGeoObject *obj) const
{
    std::vector<AOneLineTextEdit*> v = {ep1, ep2};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    AGeoConeSeg * coneSeg = dynamic_cast<AGeoConeSeg*>(ShapeCopy);
    if (!coneSeg)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        coneSeg = dynamic_cast<AGeoConeSeg*>(scaled->BaseShape);
    }

    if (coneSeg)
    {
        coneSeg->strPhi1 = ep1->text();
        coneSeg->strPhi2 = ep2->text();
    }
    else qWarning() << "Read delegate: Cone Segment shape not found!";

    return AGeoConeDelegate::updateObject(obj); // cone delegate to update the rest of the propertires!
}

void AGeoConeSegDelegate::updateGui(const AGeoObject *obj)
{
    AGeoConeDelegate::updateGui(obj);

    AGeoConeSeg * coneSeg = dynamic_cast<AGeoConeSeg*>(ShapeCopy);
    if (!coneSeg)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        coneSeg = dynamic_cast<AGeoConeSeg*>(scaled->BaseShape);
    }

    if (coneSeg)
    {
        ep1->setText(coneSeg->strPhi1.isEmpty() ? QString::number(coneSeg->phi1) : coneSeg->strPhi1);
        ep2->setText(coneSeg->strPhi2.isEmpty() ? QString::number(coneSeg->phi2) : coneSeg->strPhi2);
    }
    else qWarning() << "Read delegate: Cone Segment shape not found!";
}

AGeoElTubeDelegate::AGeoElTubeDelegate(const QStringList &materials, QWidget *parent)
    : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Elliptical tube";

    ShapeHelp = "An elliptical tube\n"
            "\n"
            "Parameters are the diameters in X and Y directions and the full height\n"
            "\n"
            "The XYZ position is given for the center point\n"
            "\n"
            "Implemented using TGeoEltu(0.5*X_full_size, 0.5*Y_full_size, 0.5*Height)";

    gr = new QGridLayout();
    gr->setContentsMargins(50, 0, 50, 3);
    gr->setVerticalSpacing(1);

    gr->addWidget(new QLabel("X full size:"), 0, 0);
    gr->addWidget(new QLabel("Y full size:"), 1, 0);
    gr->addWidget(new QLabel("Height:"),      2, 0);

    ex = new AOneLineTextEdit(); gr->addWidget(ex, 0, 1);
    ey = new AOneLineTextEdit(); gr->addWidget(ey, 1, 1);
    ez = new AOneLineTextEdit(); gr->addWidget(ez, 2, 1);

    gr->addWidget(new QLabel("mm"), 0, 2);
    gr->addWidget(new QLabel("mm"), 1, 2);
    gr->addWidget(new QLabel("mm"), 2, 2);

    addLocalLayout(gr);

    std::vector<AOneLineTextEdit*> l = {ex, ey, ez};
    for (AOneLineTextEdit * le : l)
    {
        configureHighligherAndCompleter(le);
        QObject::connect(le, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    }
}

bool AGeoElTubeDelegate::updateObject(AGeoObject *obj) const
{
    std::vector<AOneLineTextEdit*> v = {ex, ey, ez};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    AGeoEltu* elTube = dynamic_cast<AGeoEltu*>(ShapeCopy);
    if (!elTube)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*> (ShapeCopy);
        elTube = dynamic_cast<AGeoEltu*> (scaled->BaseShape);
    }
    if (elTube)
    {
        elTube->str2a  = ex->text();
        elTube->str2b  = ey->text();
        elTube->str2dz = ez->text();
    }
    else qWarning() << "Read delegate: EllipticalTube shape not found!";

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoElTubeDelegate::updateGui(const AGeoObject *obj)
{
    AGeoObjectDelegate::updateGui(obj);

    AGeoEltu * elTube = dynamic_cast<AGeoEltu*> (ShapeCopy);
    if (!elTube)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*> (ShapeCopy);
        elTube = dynamic_cast<AGeoEltu*> (scaled->BaseShape);
    }
    if (elTube)
    {
        ex->setText(elTube->str2a .isEmpty() ? QString::number(elTube-> a*2.0) : elTube->str2a);
        ey->setText(elTube->str2b .isEmpty() ? QString::number(elTube-> b*2.0) : elTube->str2b);
        ez->setText(elTube->str2dz.isEmpty() ? QString::number(elTube->dz*2.0) : elTube->str2dz);
    }
    else qWarning() << "Update delegate: Elliptical Tube shape not found!";
}

AGeoTrapXDelegate::AGeoTrapXDelegate(const QStringList &materials, QWidget *parent)
    : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Trap";

    ShapeHelp = "A trapezoidal prizm\n"
            "\n"
            "The two of the opposite faces are parallel to XY plane\n"
            "  and are positioned in Z at ± 0.5*Height.\n"
            "Full X size is given for the lower and upper planes.\n"
            "\n"
            "Implemented using TGeoTrd1(0.5*X_lower_size, 0.5*X_upper_size, 0.5*Y_size, 0.5*Height)";

    QGridLayout * gr = new QGridLayout();
    gr->setContentsMargins(50, 0, 50, 3);
    gr->setVerticalSpacing(1);

    gr->addWidget(new QLabel("X lower size:"), 0, 0);
    gr->addWidget(new QLabel("X upper size:"), 1, 0);
    gr->addWidget(new QLabel("Y size:"),       2, 0);
    gr->addWidget(new QLabel("Height:"),       3, 0);

    exl = new AOneLineTextEdit(); gr->addWidget(exl, 0, 1);
    exu = new AOneLineTextEdit(); gr->addWidget(exu, 1, 1);
    ey  = new AOneLineTextEdit(); gr->addWidget(ey,  2, 1);
    ez  = new AOneLineTextEdit(); gr->addWidget(ez,  3, 1);

    gr->addWidget(new QLabel("mm"), 0, 2);
    gr->addWidget(new QLabel("mm"), 1, 2);
    gr->addWidget(new QLabel("mm"), 2, 2);
    gr->addWidget(new QLabel("mm"), 3, 2);

    addLocalLayout(gr);

    std::vector<AOneLineTextEdit*> l = {exl, exu, ey, ez};
    for (AOneLineTextEdit * le : l)
    {
        configureHighligherAndCompleter(le);
        QObject::connect(le, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    }
}

bool AGeoTrapXDelegate::updateObject(AGeoObject *obj) const
{
    std::vector<AOneLineTextEdit*> v = {exl, exu, ey, ez};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    AGeoTrd1 * trap = dynamic_cast<AGeoTrd1*>(ShapeCopy);
    if (!trap)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        trap = dynamic_cast<AGeoTrd1*>(scaled->BaseShape);
    }

    if (trap)
    {
        trap->str2dx1 = exl->text();
        trap->str2dx2 = exu->text();
        trap->str2dy  = ey->text();
        trap->str2dz  = ez->text();
    }
    else qWarning() << "Read delegate: Trap shape not found!";

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoTrapXDelegate::updateGui(const AGeoObject *obj)
{
    AGeoObjectDelegate::updateGui(obj);

    AGeoTrd1 * trap = dynamic_cast<AGeoTrd1*>(ShapeCopy);
        if (!trap)
        {
            AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
            trap = dynamic_cast<AGeoTrd1*>(scaled->BaseShape);
        }

        if (trap)
        {
            exl->setText(trap->str2dx1.isEmpty() ? QString::number(trap->dx1 * 2.0) : trap->str2dx1);
            exu->setText(trap->str2dx2.isEmpty() ? QString::number(trap->dx2 * 2.0) : trap->str2dx2);
            ey-> setText(trap->str2dy .isEmpty() ? QString::number(trap->dy  * 2.0) : trap->str2dy);
            ez-> setText(trap->str2dz .isEmpty() ? QString::number(trap->dz  * 2.0) : trap->str2dz);
        }
        else qWarning() << "Read delegate: Trap shape not found!";
}

AGeoTrapXYDelegate::AGeoTrapXYDelegate(const QStringList &materials, QWidget *parent)
    : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Trap2";

    ShapeHelp = "A trape2 shape\n"
            "\n"
            "The two of the opposite faces are parallel to XY plane\n"
            "  and are positioned in Z at ± 0.5*Height.\n"
            "Full X and Y sizes are given for the lower and upper planes.\n"
            "\n"
            "Implemented using TGeoTrd2(0.5*X_lower_size, 0.5*X_upper_size, 0.5*Y_lower_size, 0.5*Y_upper_size, 0.5*Height)";

    QGridLayout * gr = new QGridLayout();
    gr->setContentsMargins(50, 0, 50, 3);
    gr->setVerticalSpacing(1);

    gr->addWidget(new QLabel("X lower size:"), 0, 0);
    gr->addWidget(new QLabel("X upper size:"), 1, 0);
    gr->addWidget(new QLabel("Y lower size:"), 2, 0);
    gr->addWidget(new QLabel("Y upper size:"), 3, 0);
    gr->addWidget(new QLabel("Height:"),       4, 0);

    exl = new AOneLineTextEdit(); gr->addWidget(exl, 0, 1);
    exu = new AOneLineTextEdit(); gr->addWidget(exu, 1, 1);
    eyl = new AOneLineTextEdit(); gr->addWidget(eyl, 2, 1);
    eyu = new AOneLineTextEdit(); gr->addWidget(eyu, 3, 1);
    ez  = new AOneLineTextEdit(); gr->addWidget(ez,  4, 1);

    gr->addWidget(new QLabel("mm"), 0, 2);
    gr->addWidget(new QLabel("mm"), 1, 2);
    gr->addWidget(new QLabel("mm"), 2, 2);
    gr->addWidget(new QLabel("mm"), 3, 2);
    gr->addWidget(new QLabel("mm"), 4, 2);

    addLocalLayout(gr);

    for (AOneLineTextEdit * le : {exl, exu, eyl, eyu, ez})
    {
        configureHighligherAndCompleter(le);
        QObject::connect(le, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    }
}

bool AGeoTrapXYDelegate::updateObject(AGeoObject *obj) const
{
    std::vector<AOneLineTextEdit*> v = {exl, exu, eyl, eyu, ez};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    AGeoTrd2 * trapxy = dynamic_cast<AGeoTrd2*>(ShapeCopy);
    if (!trapxy)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        trapxy = dynamic_cast<AGeoTrd2*>(scaled->BaseShape);
    }

    if (trapxy)
    {
        trapxy->str2dx1 = exl->text();
        trapxy->str2dx2 = exu->text();
        trapxy->str2dy1 = eyl->text();
        trapxy->str2dy2 = eyu->text();
        trapxy->str2dz  = ez ->text();
    }
    else qWarning() << "Read delegate: Trap2 shape not found!";

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoTrapXYDelegate::updateGui(const AGeoObject *obj)
{
    AGeoObjectDelegate::updateGui(obj);

    AGeoTrd2 * trapxy = dynamic_cast<AGeoTrd2*>(ShapeCopy);
    if (!trapxy)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        trapxy = dynamic_cast<AGeoTrd2*>(scaled->BaseShape);
    }

    if (trapxy)
    {
        exl->setText(trapxy->str2dx1.isEmpty() ? QString::number(trapxy->dx1 * 2.0) : trapxy->str2dx1);
        exu->setText(trapxy->str2dx2.isEmpty() ? QString::number(trapxy->dx2 * 2.0) : trapxy->str2dx2);
        eyl->setText(trapxy->str2dy1.isEmpty() ? QString::number(trapxy->dy1 * 2.0) : trapxy->str2dy1);
        eyu->setText(trapxy->str2dy2.isEmpty() ? QString::number(trapxy->dy2 * 2.0) : trapxy->str2dy2);
        ez-> setText(trapxy->str2dz .isEmpty() ? QString::number(trapxy->dz  * 2.0) : trapxy->str2dz);
    }
    else qWarning() << "Read delegate: Trap2 shape not found!";
}

AGeoParaboloidDelegate::AGeoParaboloidDelegate(const QStringList &materials, QWidget *parent)
    : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Paraboloid";

    ShapeHelp = "A paraboloid shape\n"
            "\n"
            "Defined by the revolution surface generated by a parabola\n"
            "  and is bound by two planes perpendicular to Z axis.\n"
            "The parabola equation is taken in the form: z = a*r^2 + b,\n"
            "  where r^2 = x^2 + y^2.\n"
            "The coefficients a and b are computed from the input values\n"
            "  which are the diameters of the circular sections cut by\n"
            "  two planes, lower at -0.5*Height, and the upper at +0.5*Height:\n"
            "  a*(0.5*Lower_diameter)^2 + b   for the lower plane and\n"
            "  a*(0.5*Upper_diameter)^2 + b   for the upper one.\n"
            "\n"
            "Implemented using TGeoParaboloid(0.5*Lower_diameter, 0.5*Upper_diameter, 0.5*Height)";

    QGridLayout * gr = new QGridLayout();
    gr->setContentsMargins(50, 0, 50, 3);
    gr->setVerticalSpacing(1);

    gr->addWidget(new QLabel("Lower diameter:"), 0, 0);
    gr->addWidget(new QLabel("Upper diameter:"), 1, 0);
    gr->addWidget(new QLabel("Height:"),         2, 0);

    el = new AOneLineTextEdit(); gr->addWidget(el, 0, 1);
    eu = new AOneLineTextEdit(); gr->addWidget(eu, 1, 1);
    ez = new AOneLineTextEdit(); gr->addWidget(ez, 2, 1);

    gr->addWidget(new QLabel("mm"), 0, 2);
    gr->addWidget(new QLabel("mm"), 1, 2);
    gr->addWidget(new QLabel("mm"), 2, 2);

    addLocalLayout(gr);

    std::vector<AOneLineTextEdit*> l = {el, eu, ez};
    for (AOneLineTextEdit * le : l)
    {
        configureHighligherAndCompleter(le);
        QObject::connect(le, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    }
}

bool AGeoParaboloidDelegate::updateObject(AGeoObject *obj) const
{
    std::vector<AOneLineTextEdit*> v = {el, eu, ez};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    AGeoParaboloid * paraboloid = dynamic_cast<AGeoParaboloid*>(ShapeCopy);
    if (!paraboloid)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        paraboloid = dynamic_cast<AGeoParaboloid*>(scaled->BaseShape);
    }
    if (paraboloid)
    {
        paraboloid->str2rlo = el->text();
        paraboloid->str2rhi = eu->text();
        paraboloid->str2dz  = ez->text();
    }
    else qWarning() << "Update delegate: Paraboloid shape not found!";

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoParaboloidDelegate::updateGui(const AGeoObject *obj)
{
    AGeoObjectDelegate::updateGui(obj);

    AGeoParaboloid * paraboloid = dynamic_cast<AGeoParaboloid*>(ShapeCopy);
    if (!paraboloid)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        paraboloid = dynamic_cast<AGeoParaboloid*>(scaled->BaseShape);
    }
    if (paraboloid)
    {
        el->setText(paraboloid->str2rlo.isEmpty() ? QString::number(paraboloid->rlo*2.0) : paraboloid->str2rlo);
        eu->setText(paraboloid->str2rhi.isEmpty() ? QString::number(paraboloid->rhi*2.0) : paraboloid->str2rhi);
        ez->setText(paraboloid->str2dz.isEmpty()  ? QString::number(paraboloid->dz*2.0)  : paraboloid->str2dz);
    }
    else qWarning() << "Update delegate: Paraboloid shape not found!";
}

AGeoTorusDelegate::AGeoTorusDelegate(const QStringList &materials, QWidget *parent)
    : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Torus";

    ShapeHelp = "A torus segment\n"
            "\n"
            "Defined by the axial, inner and outer diameters\n"
            "  and the segment angles (in degrees)\n"
            "Phi from: in the range [0, 360),\n"
            "Phi to:   in the range (0, 360], Phi_to > Phi_from\n"
            "\n"
            "Implemented using TGeoTorus(0.5*Axial_diameter, 0.5*Inner_diameter, 0.5*Outer_diameter, Phi_from, Phi_to)";

    QGridLayout * gr = new QGridLayout();
    gr->setContentsMargins(50, 0, 50, 3);
    gr->setVerticalSpacing(1);

    gr->addWidget(new QLabel("Axial diameter:"), 0, 0);
    gr->addWidget(new QLabel("Outer diameter:"), 1, 0);
    gr->addWidget(new QLabel("Inner diameter:"), 2, 0);
    gr->addWidget(new QLabel("Phi from:"),       3, 0);
    gr->addWidget(new QLabel("Phi to:"),         4, 0);

    ead = new AOneLineTextEdit(); gr->addWidget(ead, 0, 1);
    edo = new AOneLineTextEdit(); gr->addWidget(edo, 1, 1);
    edi = new AOneLineTextEdit(); gr->addWidget(edi, 2, 1);
    ep0 = new AOneLineTextEdit(); gr->addWidget(ep0, 3, 1);
    epe = new AOneLineTextEdit(); gr->addWidget(epe, 4, 1);

    gr->addWidget(new QLabel("mm"), 0, 2);
    gr->addWidget(new QLabel("mm"), 1, 2);
    gr->addWidget(new QLabel("mm"), 2, 2);
    gr->addWidget(new QLabel("°"),  3, 2);
    gr->addWidget(new QLabel("°"),  4, 2);

    addLocalLayout(gr);

    for (AOneLineTextEdit * le : {ead, edi, edo, ep0, epe})
    {
        configureHighligherAndCompleter(le);
        QObject::connect(le, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    }
}

bool AGeoTorusDelegate::updateObject(AGeoObject *obj) const
{
    std::vector<AOneLineTextEdit*> v = {ead, edi, edo, ep0, epe};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    AGeoTorus * torus = dynamic_cast<AGeoTorus*>(ShapeCopy);
    if (!torus)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        torus = dynamic_cast<AGeoTorus*>(scaled->BaseShape);
    }
    if (torus)
    {
        torus->str2R    = ead->text();
        torus->str2Rmin = edi->text();
        torus->str2Rmax = edo->text();
        torus->strPhi1  = ep0->text();
        torus->strDphi  = epe->text();
    }
    else qWarning() << "Update delegate: Torus shape not found!";

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoTorusDelegate::updateGui(const AGeoObject *obj)
{
    AGeoObjectDelegate::updateGui(obj);

    AGeoTorus * torus = dynamic_cast<AGeoTorus*>(ShapeCopy);
    if (!torus)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        torus = dynamic_cast<AGeoTorus*>(scaled->BaseShape);
    }
    if (torus)
    {
        ead->setText(torus->str2R.isEmpty()    ? QString::number(torus->R*2.0)    : torus->str2R);
        edi->setText(torus->str2Rmin.isEmpty() ? QString::number(torus->Rmin*2.0) : torus->str2Rmin);
        edo->setText(torus->str2Rmax.isEmpty() ? QString::number(torus->Rmax*2.0) : torus->str2Rmax);
        ep0->setText(torus->strPhi1.isEmpty()  ? QString::number(torus->Phi1)     : torus->strPhi1);
        epe->setText(torus->strDphi.isEmpty()  ? QString::number(torus->Dphi)     : torus->strDphi);
    }
    else qWarning() << "Update delegate: Torus shape not found!";
}

AGeoPolygonDelegate::AGeoPolygonDelegate(const QStringList &materials, QWidget *parent)
    : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Polygon (simplified)";

    ShapeHelp = "A polygon section\n"
            "\n"
            "The shape is limited by the upper and lower planes,\n"
            "  positioned in Z at +0.5*Height and -0.5*Height\n"
            "For each plane are defined\n"
            "  the outer and inner diameters of the circles,\n"
            "  inscribed in the corresponding polygon.\n"
            "\n"
            "Section angle: in the range (0, 360] degrees\n"
            "\n"
            "Implemented using TGeoPgon";

    QGridLayout * gr = new QGridLayout();
    gr->setContentsMargins(50, 0, 50, 3);
    gr->setVerticalSpacing(1);

    QLabel * lab;
    lab   = new QLabel("Number of edges:");                  gr->addWidget(lab,   0, 0);
    lab   = new QLabel("Height:");                           gr->addWidget(lab,   1, 0);
    labLO = new QLabel("Lower outer diameter of incircle:"); gr->addWidget(labLO, 2, 0);
    labLI = new QLabel("Lower inner diameter of incircle:"); gr->addWidget(labLI, 3, 0);
    labUO = new QLabel("Upper outer diameter of incircle:"); gr->addWidget(labUO, 4, 0);
    labUI = new QLabel("Upper inner diameter of incircle:"); gr->addWidget(labUI, 5, 0);
    labA  = new QLabel("Angle:");                            gr->addWidget(labA,  6, 0);

    en  = new AOneLineTextEdit(); gr->addWidget(en,  0, 1); en->bIntegerTooltip = true;
    ez  = new AOneLineTextEdit(); gr->addWidget(ez,  1, 1);
    elo = new AOneLineTextEdit(); gr->addWidget(elo, 2, 1);
    eli = new AOneLineTextEdit(); gr->addWidget(eli, 3, 1);
    euo = new AOneLineTextEdit(); gr->addWidget(euo, 4, 1);
    eui = new AOneLineTextEdit(); gr->addWidget(eui, 5, 1);
    edp = new AOneLineTextEdit(); gr->addWidget(edp, 6, 1);

    lab    = new QLabel("mm"); gr->addWidget(lab,    1, 2);
    lab    = new QLabel("mm"); gr->addWidget(lab,    2, 2);
    labLIu = new QLabel("mm"); gr->addWidget(labLIu, 3, 2);
    labUOu = new QLabel("mm"); gr->addWidget(labUOu, 4, 2);
    labUIu = new QLabel("mm"); gr->addWidget(labUIu, 5, 2);
    labAu  = new QLabel("°");  gr->addWidget(labAu,  6, 2);

    addLocalLayout(gr);

    for (AOneLineTextEdit * le : {en, edp, ez, eli, elo, eui, euo})
    {
        configureHighligherAndCompleter(le);
        QObject::connect(le, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    }
}

bool AGeoPolygonDelegate::updateObject(AGeoObject *obj) const
{
    std::vector<AOneLineTextEdit*> v = {en, edp, ez, eli, elo, eui, euo};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    AGeoPolygon * polygon = dynamic_cast<AGeoPolygon*>(ShapeCopy);
    if (!polygon)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        polygon = dynamic_cast<AGeoPolygon*>(scaled->BaseShape);
    }

    if (polygon)
    {
        polygon->strNedges = en ->text();
        polygon->strdPhi   = edp->text();
        polygon->str2dz    = ez ->text();
        polygon->str2rminL = eli->text();
        polygon->str2rmaxL = elo->text();
        polygon->str2rminU = eui->text();
        polygon->str2rmaxU = euo->text();
    }
    else qWarning() << "Read delegate: Polygon shape not found!";

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoPolygonDelegate::updateGui(const AGeoObject *obj)
{
    AGeoObjectDelegate::updateGui(obj);

    AGeoPolygon * polygon = dynamic_cast<AGeoPolygon*>(ShapeCopy);
    if (!polygon)
    {
    AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
    polygon = dynamic_cast<AGeoPolygon*>(scaled->BaseShape);
    }

    if (polygon)
    {
        en ->setText(polygon->strNedges .isEmpty() ? QString::number(polygon->nedges)      : polygon->strNedges);
        edp->setText(polygon->strdPhi  .isEmpty()  ? QString::number(polygon->dphi)        : polygon->strdPhi);
        ez ->setText(polygon->str2dz   .isEmpty()  ? QString::number(polygon->dz    * 2.0) : polygon->str2dz);
        eli->setText(polygon->str2rminL.isEmpty()  ? QString::number(polygon->rminL * 2.0) : polygon->str2rminL);
        elo->setText(polygon->str2rmaxL.isEmpty()  ? QString::number(polygon->rmaxL * 2.0) : polygon->str2rmaxL);
        eui->setText(polygon->str2rminU.isEmpty()  ? QString::number(polygon->rminU * 2.0) : polygon->str2rminU);
        euo->setText(polygon->str2rmaxU.isEmpty()  ? QString::number(polygon->rmaxU * 2.0) : polygon->str2rmaxU);
    }
    else qWarning() << "Read delegate: Polygon shape not found!";
}

AGeoPconDelegate::AGeoPconDelegate(const QStringList &materials, QWidget *parent)
    : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Polycone";

    ShapeHelp = "A shape constructed of an arbitrary number of cone sections\n"
            "\n"
            "A section is limited by the upper and lower planes,\n"
            "  shared by the neighboring sections.\n"
            "For each plane are defined:\n"
            "  Z position, and\n"
            "  outer and inner diameters.\n"
            "\n"
            "Phi from: in the range [0, 360) degrees,\n"
            "Phi to:   in the range (0, 360] degrees, Phi_to > Phi_from\n"
            "\n"
            "Implemented using TGeoPcon";

    lay = new QVBoxLayout();
    lay->setContentsMargins(50, 0, 50, 0);
    lay->setSpacing(3);

    lay->addWidget(new QLabel("Defined planes (should be monotonic in Z), all in mm:"));

        tab = new QTableWidget();
        tab->setMaximumHeight(150);
        tab->verticalHeader()->setSectionsMovable(true);
        tab->setDropIndicatorShown(true);
        QObject::connect(tab->verticalHeader(), &QHeaderView::sectionMoved, this, &AGeoPconDelegate::onReorderSections, Qt::QueuedConnection);
    lay->addWidget(tab);

        QHBoxLayout * hl = new QHBoxLayout();
        QPushButton * pbAddAbove = new QPushButton("Add above");
        connect(pbAddAbove, &QPushButton::clicked, this, &AGeoPconDelegate::onAddAbove);
        hl->addWidget(pbAddAbove);

        QPushButton * pbAddBelow = new QPushButton("Add below");
        connect(pbAddBelow, &QPushButton::clicked, this, &AGeoPconDelegate::onAddBellow);
        hl->addWidget(pbAddBelow);

        QPushButton * pbRemoveRow = new QPushButton("Remove plane");
        connect(pbRemoveRow, &QPushButton::clicked, [this]()
        {
            int row = tab->currentRow();
            if (row == -1) guitools::message("Select a row to remove!", this->ParentWidget);
            else
            {
                tab->removeRow(row);
                //ContentChanged();
                onContentChangedBase();
            }
        });
        hl->addWidget(pbRemoveRow);
    lay->addLayout(hl);

        QGridLayout * gr = new QGridLayout();
        gr->setContentsMargins(0, 0, 0, 3);
        gr->setVerticalSpacing(1);
        gr->addWidget(new QLabel("Phi from:"), 0, 0);
        gr->addWidget(new QLabel("Phi range:"),   1, 0);
        ep0 = new AOneLineTextEdit(); gr->addWidget(ep0, 0, 1);
        epe = new AOneLineTextEdit(); gr->addWidget(epe, 1, 1);
        gr->addWidget(new QLabel("°"),  0, 2);
        gr->addWidget(new QLabel("°"),  1, 2);
        for (AOneLineTextEdit * le : {ep0, epe})
        {
            configureHighligherAndCompleter(le);
            QObject::connect(le, &AOneLineTextEdit::textChanged, this, &AGeoPconDelegate::onContentChangedBase);
        }
    lay->addLayout(gr);

    addLocalLayout(lay);
}

bool AGeoPconDelegate::updateObject(AGeoObject *obj) const
{
    if (!tab)
    {
        qWarning() << "Tab widget not found!";
        return false;
    }
    readGui();

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoPconDelegate::addOneLineTextEdits(int row)
{
    for (int ic = 0; ic < 3; ic++)
    {
        AOneLineTextEdit * e = new AOneLineTextEdit("", tab);
        configureHighligherAndCompleter(e);
        tab->setCellWidget(row, ic, e);
    }
}

void AGeoPconDelegate::readGui() const
{
    std::vector<AOneLineTextEdit*> v = {ep0, epe};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return;
    }

    AGeoPcon * pcon = dynamic_cast<AGeoPcon*>(ShapeCopy);
    if (!pcon)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        pcon = dynamic_cast<AGeoPcon*>(scaled->BaseShape);
    }

    if (pcon)
    {
        pcon->strPhi  = ep0->text();
        pcon->strdPhi = epe->text();

        const int rows = tab->rowCount();
        pcon->Sections.clear();
        for (int ir = 0; ir < rows; ir++)
        {
            std::vector<QString> edits;
            for (int ic = 0; ic < 3; ic++)
            {
                AOneLineTextEdit * edit = static_cast<AOneLineTextEdit *>(tab->cellWidget(ir, ic));
                edits.push_back(edit->text());
            }
            //if (edits[0].isEmpty() || edits[1].isEmpty()|| edits[2].isEmpty() ) continue;
            if (edits[0].isEmpty() || edits[1].isEmpty()|| edits[2].isEmpty() )
            {
                QMessageBox::warning(this->ParentWidget, "", "Empty line!");
                return;
            }
            APolyCGsection * Section = new APolyCGsection;

            Section->strZ     = edits[0];
            Section->str2rmin = edits[1];
            Section->str2rmax = edits[2];

            pcon->Sections.push_back(*Section);
        }
    }
    else qWarning() << "Read delegate: PolyCone shape not found!";
}

void AGeoPconDelegate::updateGui(const AGeoObject *obj)
{
    AGeoObjectDelegate::updateGui(obj);

    AGeoPcon * pcon = dynamic_cast<AGeoPcon*>(ShapeCopy);
    if (!pcon)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        pcon = dynamic_cast<AGeoPcon*>(scaled->BaseShape);
    }

    if (pcon)
    {
        ep0->setText(pcon->strPhi .isEmpty() ? QString::number(pcon->phi) : pcon->strPhi);
        epe->setText(pcon->strdPhi.isEmpty() ? QString::number(pcon->dphi): pcon->strdPhi);
        updateTableW(pcon);
    }
    else qWarning() << "Read delegate: PolyCone shape not found!";
}

void AGeoPconDelegate::updateTableW(AGeoPcon * pcon)
{
    tab->clear();
    tab->setColumnCount(3);
    tab->setHorizontalHeaderLabels(QStringList({"Z position", "Inner diameter", "Outer diameter"}));

    const int numPlanes = pcon->Sections.size();
    tab->setRowCount(numPlanes);
    for (int iP = 0; iP < numPlanes; iP++)
    {
        const APolyCGsection & Section = pcon->Sections.at(iP);

        std::vector<AOneLineTextEdit*> le(3, nullptr);
        for (int i = 0; i < 3; i++)
        {
            le[i] = new AOneLineTextEdit("", tab);
            configureHighligherAndCompleter(le[i]);
            QObject::connect(le[i], &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
            QObject::connect(le[i], &AOneLineTextEdit::editingFinished, this, &AGeoPconDelegate::onCellEdited);
            tab->setCellWidget(iP, i, le[i]);
        }

        le[0]->setText(Section.strZ    .isEmpty() ? QString::number(Section.z)          : Section.strZ);
        le[1]->setText(Section.str2rmin.isEmpty() ? QString::number(Section.rmin * 2.0) : Section.str2rmin);
        le[2]->setText(Section.str2rmax.isEmpty() ? QString::number(Section.rmax * 2.0) : Section.str2rmax);

        tab->setRowHeight(iP, rowHeight);
    }
}

void AGeoPconDelegate::onCellEdited()
{
    readGui();

    AGeoPcon * pcon = dynamic_cast<AGeoPcon*>(ShapeCopy);
    if (!pcon)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        pcon = dynamic_cast<AGeoPcon*>(scaled->BaseShape);
    }

    QString dummyErrorStr;
    if (pcon) pcon->introduceGeoConstValues(dummyErrorStr);
}

void AGeoPconDelegate::onAddAbove()
{
    int row = tab->currentRow();
    if (row == -1) row = 0;
    AGeoPcon * pcon = dynamic_cast<AGeoPcon*>(ShapeCopy);
    if (!pcon)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        pcon = dynamic_cast<AGeoPcon*>(scaled->BaseShape);
    }
    if (pcon)
    {
        APolyCGsection newSection = pcon->Sections[row];
        if (row == 0)
        {
            newSection.z -= 10.0;
            if (!newSection.strZ.isEmpty()) newSection.strZ += "-10";
        }
        else newSection.strZ = QString("%1").arg((pcon->Sections[row].z + pcon->Sections[row-1].z)/2);
        pcon->Sections.insert(pcon->Sections.begin() + row, newSection);

    }
    updateTableW(pcon);
}

void AGeoPconDelegate::onAddBellow()
{
    const int num = tab->rowCount();
    int row = tab->currentRow();
    if (row == -1) row = num-1;
    AGeoPcon * pcon = dynamic_cast<AGeoPcon*>(ShapeCopy);
    if (!pcon)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        pcon = dynamic_cast<AGeoPcon*>(scaled->BaseShape);
    }

    if (pcon)
    {
        APolyCGsection newSection = pcon->Sections[row];
        if (row == num-1)
        {
            newSection.z += 10.0;
            if (!newSection.strZ.isEmpty()) newSection.strZ += "+10";
        }
        else newSection.strZ = QString("%1").arg((pcon->Sections[row].z + pcon->Sections[row+1].z)/2);
        //qDebug() <<"new section" <<newSection.z;
        pcon->Sections.insert(pcon->Sections.begin() + row + 1, newSection);
    }
    updateTableW(pcon);
}

void AGeoPconDelegate::onReorderSections(int, int oldVisualIndex, int newVisualIndex)
{
    AGeoPcon * pcon = dynamic_cast<AGeoPcon*>(ShapeCopy);
    if (!pcon)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        pcon = dynamic_cast<AGeoPcon*>(scaled->BaseShape);
    }

    if (pcon)
    {
        APolyCGsection tempSection = pcon->Sections[oldVisualIndex];
        pcon->Sections[oldVisualIndex] = pcon->Sections[newVisualIndex];
        pcon->Sections[newVisualIndex] = tempSection;

        updateTableW(pcon);

        tab->verticalHeader()->blockSignals(true);  // -->
        tab->verticalHeader()->moveSection(newVisualIndex, oldVisualIndex);//swaps back table rows oldVisualIndex and newVisualIndex
        tab->verticalHeader()->blockSignals(false); // <--

        onContentChangedBase();
    }
    else qWarning() << "PolyCone not found in move row";
}

AGeoPgonDelegate::AGeoPgonDelegate(const QStringList &materials, QWidget *parent)
    : AGeoPconDelegate(materials, parent)
{
    DelegateTypeName = "Polygon";

    ShapeHelp = "A shape constructed of an arbitrary number of polygon sections\n"
            "\n"
            "A section is limited by the upper and lower planes,\n"
            "  shared by the neighboring sections.\n"
            "For each plane are defined:\n"
            "  Z position, and\n"
            "  outer and inner diameters of circles, inscribed in the polygon.\n"
            "\n"
            "Phi from: in the range [0, 360) degrees,\n"
            "Phi to:   in the range (0, 360] degrees, Phi_to > Phi_from\n"
            "\n"
            "Implemented using TGeoPgon";

    QHBoxLayout * h = new QHBoxLayout();
    h->setContentsMargins(0, 0, 0, 1);
    h->setSpacing(1);
    QLabel * lab = new QLabel("Edges:");
    h->addWidget(lab);
    eed = new AOneLineTextEdit;
    h->addWidget(eed);
    h->addStretch();

    lay->insertLayout(0, h);
    tab->setHorizontalHeaderLabels(QStringList({"Z position", "Outer size", "Inner size"}));

    configureHighligherAndCompleter(eed);
    QObject::connect(eed, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
}

bool AGeoPgonDelegate::updateObject(AGeoObject *obj) const
{
    std::vector<AOneLineTextEdit*> v = {eed};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    AGeoPgon * pgon = dynamic_cast<AGeoPgon*>(ShapeCopy);
    if (!pgon)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        pgon = dynamic_cast<AGeoPgon*>(scaled->BaseShape);
    }

    if (pgon) pgon->strNedges = eed->text();
    else qWarning() << "Read delegate: Polygon shape not found!";

    return AGeoPconDelegate::updateObject(obj); // AGeoPcon for the rest of parameters!
}

void AGeoPgonDelegate::updateGui(const AGeoObject *obj)
{
    AGeoPconDelegate::updateGui(obj);

    AGeoPgon * pgon = dynamic_cast<AGeoPgon*>(ShapeCopy);
    if (!pgon)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        pgon = dynamic_cast<AGeoPgon*>(scaled->BaseShape);
    }

    if (pgon)
    {
        eed->setText(pgon->strNedges.isEmpty() ? QString::number(pgon->nedges) : pgon->strNedges);
    }
    else qWarning() << "Read delegate: Polygon shape not found!";
}

AGeoCompositeDelegate::AGeoCompositeDelegate(const QStringList &materials, QWidget *parent)
    : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Composite shape";

    ShapeHelp = "Composite shape\n"
            "\n"
            "Composite shapes are boolean combinations\n"
            "  of two or more shape components. The supported operations are:\n"
            "\n"
            "  (+) - union\n"
            "  (*) - intersection\n"
            "  (-) - subtraction.\n"
            "\n"
            "If nested structures are needed, brackets can be used, e.g.,\n"
            "   (Shape1 + Shape2) - (Shape3 * (Shape4 + Shape5))\n"
            "\n"
            "Implemented using TGeoCompositeShape\n"
            "\n"
            "WARNING!\n"
            "Use composite shapes only as the last resort.\n"
            "Navigation in geometries containing composite shapes are very inefficient!";

    QVBoxLayout * v = new QVBoxLayout();
    v->setContentsMargins(50, 0, 50, 3);

    v->addWidget(new QLabel("Use logical volume names and\n'+', '*', and '-' operands; brackets for nested"));
        te = new QPlainTextEdit();
        QFont font = te->font();
        font.setPointSize(te->font().pointSize() + 2);
        te->setFont(font);
    v->addWidget(te);
    connect(te, &QPlainTextEdit::textChanged, this, &AGeoCompositeDelegate::onContentChangedBase);

    addLocalLayout(v);

    cbScale->setChecked(false);
    cbScale->setEnabled(false);
}

bool AGeoCompositeDelegate::updateObject(AGeoObject *obj) const
{
    AGeoComposite * comp = dynamic_cast<AGeoComposite*>(ShapeCopy);
    if (!comp)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        comp = dynamic_cast<AGeoComposite*>(scaled->BaseShape);
    }

    if (comp)
    {
        QString Str = te->document()->toPlainText();
        comp->GenerationString = "TGeoCompositeShape( " + Str + " )";
    }
    else qWarning() << "Composite shape not found!";

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoCompositeDelegate::updateGui(const AGeoObject *obj)
{
    AGeoObjectDelegate::updateGui(obj);

    AGeoComposite * comp = dynamic_cast<AGeoComposite*>(ShapeCopy);
    if (!comp)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        comp = dynamic_cast<AGeoComposite*>(scaled->BaseShape);
    }

    if (comp)
    {
        //qDebug() <<"updte delegate"<<comp->GenerationString.simplified();
        QString s = comp->GenerationString.simplified();
        s.remove("TGeoCompositeShape(");
        s.chop(1);

        te->clear();
        te->appendPlainText(s.simplified());
    }
    else qWarning() << "Read delegate: Composite shape not found!";
}

AGeoArb8Delegate::AGeoArb8Delegate(const QStringList &materials, QWidget *parent)
    : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Arb8";

    ShapeHelp = "An Arb8 shape of Cern Root\n"
                "\n"
                "The shape is defined by two quadrilaterals sitting on parallel planes,\n"
                "   at +0.5*Height and -0.5*Height.\n"
                "Quadrilaterals are defined each by 4 vertices\n"
                "   with the coordinates (Xi,Yi,+/-0.5*Height), i from 0 to 3.\n"
                "\n"
                "The lateral surface of the Arb8 is defined by the 4 pairs of\n"
                "   edges corresponding to vertices (i,i+1) on both planes.\n"
                "If M and M' are the middles of the segments (i,i+1)\n"
                "   at the different planes,\n"
                "   a lateral surface is obtained by sweeping the edge at\n"
                "   -0.5*Height along MM',\n"
                "   so that it will match the corresponding one at +0.5*Height.\n"
                "Since the points defining the edges are arbitrary,\n"
                "   the lateral surfaces are not necessary planes –\n"
                "   but twisted planes having a twist angle linear-dependent on Z.\n"
                "\n"
                "Vertices have to be defined clockwise in the XY pane at both planes!\n"
                "\n"
                "Any two or more vertices in each plane\n"
                "   can have the same (X,Y) coordinates!\n"
                "   It this case, the top and bottom quadrilaterals become triangles,\n"
                "   segments or points.\n"
                "\n"
                "The lateral surfaces are not necessary defined by a pair of segments,\n"
                "   but by pair segment-point (making a triangle)\n"
                "   or point-point (making a line).\n"
                "Any choice is valid as long as at one of the end-caps is at least a triangle.\n";

    QVBoxLayout * v = new QVBoxLayout();
    v->setContentsMargins(50, 0, 50, 0);
    v->setSpacing(3);
    QGridLayout * gr = new QGridLayout();
        gr->setContentsMargins(0, 0, 0, 0);
        gr->addWidget(new QLabel("Height:"), 0, 0);
        ez = new AOneLineTextEdit(); gr->addWidget(ez,  0, 1);
        configureHighligherAndCompleter(ez);
        connect(ez, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
        gr->addWidget(new QLabel("mm"), 0, 2);
    v->addLayout(gr);

    ve.clear();
    for (int iul = 0; iul < 2; iul++)
    {
        v->addWidget(new QLabel(iul == 0 ? "Lower plane (positions clockwise!):" : "Upper plane (positions clockwise!):"));

        std::vector<AEditEdit> tmpV(4);

        QGridLayout * gri = new QGridLayout();
        gri->setContentsMargins(0, 0, 0, 0);
        gri->setVerticalSpacing(3);

        for (int i = 0; i < 4; i++)
        {
            gri->addWidget(new QLabel("  x:"),    i, 0);
            tmpV[i].X = new AOneLineTextEdit;
            configureHighligherAndCompleter(tmpV[i].X);
            connect(tmpV[i].X, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
            gri->addWidget(tmpV[i].X,             i, 1);
            gri->addWidget(new QLabel("mm   y:"), i, 2);
            tmpV[i].Y = new AOneLineTextEdit;
            configureHighligherAndCompleter(tmpV[i].Y);
            connect(tmpV[i].Y, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
            gri->addWidget(tmpV[i].Y,             i, 3);
            gri->addWidget(new QLabel("mm"),      i, 4);
        }
        ve.push_back(tmpV);
        v->addLayout(gri);
    }
    addLocalLayout(v);
}

bool AGeoArb8Delegate::updateObject(AGeoObject * obj) const
{
    std::vector<AOneLineTextEdit*> v = {ez};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    AGeoArb8 * arb8 = dynamic_cast<AGeoArb8*>(ShapeCopy);
    if (!arb8)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        arb8 = dynamic_cast<AGeoArb8*>(scaled->BaseShape);
    }

    if (arb8)
    {
        arb8->str2dz = ez->text();

        for (int iul =0; iul < 2; iul++)
        {
            for (int i=0; i < 4; i++)
            {
                std::vector<AOneLineTextEdit*> v = {ve[iul][i].X, ve[iul][i].Y};
                if (isLeEmpty(v))
                {
                    QMessageBox::warning(this->ParentWidget, "", "Empty line!");
                    return false;
                }

                const int iInVert = iul * 4 + i;
                arb8->strVertices[iInVert].first  = ve[iul][i].X->text();
                arb8->strVertices[iInVert].second = ve[iul][i].Y->text();
            }
        }
    }
    else qWarning() << "Read delegate: Arb8 shape not found!";

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoArb8Delegate::updateGui(const AGeoObject *obj)
{
    AGeoObjectDelegate::updateGui(obj);

    AGeoArb8 * arb8 = dynamic_cast<AGeoArb8*>(ShapeCopy);
    if (!arb8)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(ShapeCopy);
        arb8 = dynamic_cast<AGeoArb8*>(scaled->BaseShape);
    }

    if (arb8)
    {
        ez->setText(arb8->str2dz.isEmpty() ? QString::number(2.0 * arb8->dz) : arb8->str2dz);

        for (int iul = 0; iul < 2; iul++)
        {
            for (int i = 0; i < 4; i++)
            {
                const int iInVert = iul * 4 + i;
                const std::pair<double, double>   & V = arb8->Vertices[iInVert];
                const std::pair<QString, QString> & S = arb8->strVertices[iInVert];
                AEditEdit & CEE = ve[iul][i];
                CEE.X->setText( S.first.isEmpty()  ? QString::number(V.first)  : S.first  );
                CEE.Y->setText( S.second.isEmpty() ? QString::number(V.second) : S.second );
            }
        }
    }
    else qWarning() << "Read delegate: Arb8 shape not found!";
}

AGeoArrayDelegate::AGeoArrayDelegate(const QStringList &materials, QWidget *parent)
   : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Array";

    QVBoxLayout * lVer = new QVBoxLayout();
    lVer->setContentsMargins(5, 3, 5, 3);
    lVer->setSpacing(3);

    QGridLayout * grAW = new QGridLayout();
    grAW->setContentsMargins(0,0,0,0);
    grAW->setVerticalSpacing(0);

    QLabel * la = nullptr;
    la = new QLabel("Number in X:"); grAW->addWidget(la, 0, 0);
    la = new QLabel("Number in Y:"); grAW->addWidget(la, 1, 0);
    la = new QLabel("Number in Z:"); grAW->addWidget(la, 2, 0);

    la = new QLabel("Step in X:");
    la->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grAW->addWidget(la, 0, 2);

    la = new QLabel("Step in Y:");
    la->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grAW->addWidget(la, 1, 2);

    la = new QLabel("Step in Z:");
    la->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grAW->addWidget(la, 2, 2);

    la = new QLabel("mm"); grAW->addWidget(la, 0, 4);
    la = new QLabel("mm"); grAW->addWidget(la, 1, 4);
    la = new QLabel("mm"); grAW->addWidget(la, 2, 4);

    ledNumX  = new AOneLineTextEdit("", Widget); grAW->addWidget(ledNumX,  0, 1); ledNumX->bIntegerTooltip = true;
    ledNumY  = new AOneLineTextEdit("", Widget); grAW->addWidget(ledNumY,  1, 1); ledNumY->bIntegerTooltip = true;
    ledNumZ  = new AOneLineTextEdit("", Widget); grAW->addWidget(ledNumZ,  2, 1); ledNumZ->bIntegerTooltip = true;
    ledStepX = new AOneLineTextEdit("", Widget); grAW->addWidget(ledStepX, 0, 3);
    ledStepY = new AOneLineTextEdit("", Widget); grAW->addWidget(ledStepY, 1, 3);
    ledStepZ = new AOneLineTextEdit("", Widget); grAW->addWidget(ledStepZ, 2, 3);

    connect(ledNumX, &AOneLineTextEdit::textChanged, this, [this](){updateArrayStepEnable(ledNumX, ledStepX);});
    connect(ledNumY, &AOneLineTextEdit::textChanged, this, [this](){updateArrayStepEnable(ledNumY, ledStepY);});
    connect(ledNumZ, &AOneLineTextEdit::textChanged, this, [this](){updateArrayStepEnable(ledNumZ, ledStepZ);});

    lVer->addLayout(grAW);

    cbCenterSym = new QCheckBox("Center-symmetric");
    lVer->addWidget(cbCenterSym, 0, Qt::AlignHCenter);

    QHBoxLayout * lHor = new QHBoxLayout();
    lHor->addStretch();
    lHor->addWidget(new QLabel("Index of the first node:"));
    ledStartIndex = new AOneLineTextEdit("", Widget); ledStartIndex->bIntegerTooltip = true;
    lHor->addWidget(ledStartIndex);
    QLabel * ledHelp = new QLabel("|?|"); ledHelp->setToolTip("Can use \"ParentIndex\" expression in the formula to access index of the containing volume");
    lHor->addWidget(ledHelp);
    lHor->addStretch();

    lVer->addLayout(lHor);

    addLocalLayout(lVer);

    std::vector<AOneLineTextEdit*> l = {ledNumX, ledNumY, ledNumZ, ledStepX, ledStepY, ledStepZ, ledStartIndex};
    for (AOneLineTextEdit * le : l)
    {
        //le->setMaximumWidth(75);
        le->setContextMenuPolicy(Qt::NoContextMenu);
        configureHighligherAndCompleter(le);
        QObject::connect(le, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    }
    QObject::connect(cbCenterSym, &QCheckBox::clicked, this, &AGeoBaseDelegate::onContentChangedBase);

    cbScale->setChecked(false);
    cbScale->setVisible(false);

    lMat->setVisible(false);
    cobMat->setVisible(false);
    //ledPhi->setText("0");
    //ledPhi->setEnabled(false);
    //ledTheta->setText("0");
    //ledTheta->setEnabled(false);

    pbTransform->setVisible(false);
    pbShapeInfo->setVisible(false);
}

void AGeoArrayDelegate::updateArrayStepEnable(AOneLineTextEdit * editNum, AOneLineTextEdit * editStep)
{
    QString text = editNum->text();
    bool bAtLeastTwo = true;
    bool ok;
    double val = text.toDouble(&ok);
    if (!ok)
    {
        QString errorStr;
        ok = AGeoConsts::getConstInstance().evaluateFormula(errorStr, text, val);
    }
    if (ok) bAtLeastTwo = (val >= 2);
    editStep->setEnabled(bAtLeastTwo);
}


bool AGeoArrayDelegate::updateObject(AGeoObject * obj) const
{
    std::vector<AOneLineTextEdit*> v = {ledNumX, ledNumY, ledNumZ, ledStepX, ledStepY, ledStepZ, ledStartIndex};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    if (!CurrentObject->Type->isArray()) return false;

    ATypeArrayObject a;
    a.strNumX  = ledNumX->text();
    a.strNumY  = ledNumY->text();
    a.strNumZ  = ledNumZ->text();
    a.strStepX = ledStepX->text();
    a.strStepY = ledStepY->text();
    a.strStepZ = ledStepZ->text();
    a.bCenterSymmetric = cbCenterSym->isChecked();
    a.strStartIndex = ledStartIndex->text();

    QString errorStr;
    a.introduceGeoConstValues(errorStr);
    if (!errorStr.isEmpty())
    {
        QMessageBox::warning(this->ParentWidget, "", errorStr);
        return false;
    }

    ATypeArrayObject * array = static_cast<ATypeArrayObject*>(obj->Type);
    *array = a;

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoArrayDelegate::updateGui(const AGeoObject * obj)
{
    AGeoObjectDelegate::updateGui(obj);

    if (obj->Type->isArray())
    {
        ATypeArrayObject* array = static_cast<ATypeArrayObject*>(obj->Type);

        ledNumX->setText(array->strNumX.isEmpty() ? QString::number(array->numX) : array->strNumX);
        ledNumY->setText(array->strNumY.isEmpty() ? QString::number(array->numY) : array->strNumY);
        ledNumZ->setText(array->strNumZ.isEmpty() ? QString::number(array->numZ) : array->strNumZ);
        ledStepX->setText(array->strStepX.isEmpty() ? QString::number(array->stepX) : array->strStepX);
        ledStepY->setText(array->strStepY.isEmpty() ? QString::number(array->stepY) : array->strStepY);
        ledStepZ->setText(array->strStepZ.isEmpty() ? QString::number(array->stepZ) : array->strStepZ);
        cbCenterSym->setChecked(array->bCenterSymmetric);
        ledStartIndex->setText(array->strStartIndex.isEmpty() ? QString::number(array->startIndex) : array->strStartIndex);
    }
}

AGeoCircularArrayDelegate::AGeoCircularArrayDelegate(const QStringList &materials, QWidget *parent)
    : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Circular array";

    QVBoxLayout * lVer = new QVBoxLayout();
    lVer->setContentsMargins(5, 3, 5, 3);
    lVer->setSpacing(3);

    QGridLayout * grAW = new QGridLayout();
    grAW->setContentsMargins(0,0,0,0);
    grAW->setVerticalSpacing(0);

    QLabel * la = nullptr;
    la = new QLabel("Number:");       grAW->addWidget(la, 0, 0);
    la = new QLabel("Angular step:"); grAW->addWidget(la, 1, 0);
    la = new QLabel("Radius:");       grAW->addWidget(la, 2, 0);

    la = new QLabel("deg");           grAW->addWidget(la, 1, 2);
    la = new QLabel("mm");            grAW->addWidget(la, 2, 2);

    ledNum         = new AOneLineTextEdit("", Widget); grAW->addWidget(ledNum, 0, 1);         ledNum->bIntegerTooltip = true;
    ledAngularStep = new AOneLineTextEdit("", Widget); grAW->addWidget(ledAngularStep, 1, 1);
    ledRadius      = new AOneLineTextEdit("", Widget); grAW->addWidget(ledRadius, 2, 1);

    lVer->addLayout(grAW);

    QHBoxLayout * lHor = new QHBoxLayout();
    lHor->addStretch();
    lHor->addWidget(new QLabel("Index of the first node:"));
    ledStartIndex = new AOneLineTextEdit("", Widget); ledStartIndex->bIntegerTooltip = true;
    lHor->addWidget(ledStartIndex);
    lHor->addStretch();

    lVer->addLayout(lHor);

    addLocalLayout(lVer);

    std::vector<AOneLineTextEdit*> l = {ledNum, ledAngularStep, ledRadius, ledStartIndex};
    for (AOneLineTextEdit * le : l)
    {
        //le->setMaximumWidth(75);
        le->setContextMenuPolicy(Qt::NoContextMenu);
        configureHighligherAndCompleter(le);
        QObject::connect(le, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    }

    cbScale->setChecked(false);
    cbScale->setVisible(false);

    lMat->setVisible(false);
    cobMat->setVisible(false);
    //ledPhi->setText("0");
    //ledPhi->setEnabled(false);
    //ledTheta->setText("0");
    //ledTheta->setEnabled(false);

    pbTransform->setVisible(false);
    pbShapeInfo->setVisible(false);
}

bool AGeoCircularArrayDelegate::updateObject(AGeoObject *obj) const
{
    std::vector<AOneLineTextEdit*> v = {ledNum, ledAngularStep, ledRadius, ledStartIndex};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    if (!CurrentObject->Type->isCircularArray()) return false;

    ATypeCircularArrayObject a;
    a.strNum         = ledNum->text();
    a.strAngularStep = ledAngularStep->text();
    a.strRadius      = ledRadius->text();
    a.strStartIndex = ledStartIndex->text();

    QString errorStr;
    a.introduceGeoConstValues(errorStr);
    if (!errorStr.isEmpty())
    {
        QMessageBox::warning(this->ParentWidget, "", errorStr);
        return false;
    }

    ATypeCircularArrayObject * array = static_cast<ATypeCircularArrayObject*>(obj->Type);
    *array = a;

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoCircularArrayDelegate::updateGui(const AGeoObject *obj)
{
    AGeoObjectDelegate::updateGui(obj);

    ATypeCircularArrayObject * array = dynamic_cast<ATypeCircularArrayObject*>(obj->Type);

    if (array)
    {
        ledNum->setText(array->strNum.isEmpty() ? QString::number(array->num) : array->strNum);
        ledAngularStep->setText(array->strAngularStep.isEmpty() ? QString::number(array->angularStep) : array->strAngularStep);
        ledRadius->setText(array->strRadius.isEmpty() ? QString::number(array->radius) : array->strRadius);
        ledStartIndex->setText(array->strStartIndex.isEmpty() ? QString::number(array->startIndex) : array->strStartIndex);
    }
}

// ---

AGeoHexagonalArrayDelegate::AGeoHexagonalArrayDelegate(const QStringList & materials, QWidget * parent)
    : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Hexagonal array";

    QVBoxLayout * lVer = new QVBoxLayout();
    lVer->setContentsMargins(5, 3, 5, 3);
    lVer->setSpacing(3);

    QGridLayout * grAW = new QGridLayout();
    grAW->setContentsMargins(0,0,0,0);
    grAW->setVerticalSpacing(0);

    QLabel * la = nullptr;

    la = new QLabel("Pitch:");        grAW->addWidget(la, 0, 0);
    ledStep = new AOneLineTextEdit("", Widget); grAW->addWidget(ledStep, 0, 1);
    la = new QLabel("mm");            grAW->addWidget(la, 0, 2);

    //la = new QLabel("Shape:");        grAW->addWidget(la, 1, 0);
    cobShape = new QComboBox(Widget); grAW->addWidget(cobShape, 0, 3);
    cobShape->addItems({"Hexagonal shape", "Rectangular shape"});

    QLabel * laR = new QLabel("Rings:"); grAW->addWidget(laR, 2, 0);
    ledNumRings = new AOneLineTextEdit("", Widget); grAW->addWidget(ledNumRings, 2, 1); ledNumRings->bIntegerTooltip = true;

    QLabel * laX = new QLabel("Number in X:"); grAW->addWidget(laX, 3, 0);
    ledNumX      = new AOneLineTextEdit("", Widget); grAW->addWidget(ledNumX, 3, 1); ledNumX->bIntegerTooltip = true;
    QLabel * laY = new QLabel("in Y:"); grAW->addWidget(laY, 3, 2, Qt::AlignRight);
    ledNumY      = new AOneLineTextEdit("", Widget); grAW->addWidget(ledNumY, 3, 3); ledNumY->bIntegerTooltip = true;

    cbSkipFirstEven = new QCheckBox("Skip first on even rows");
    grAW->addWidget(cbSkipFirstEven, 4, 1, 2, 1);

    cbSkipLastOdd = new QCheckBox("Skip last on odd rows");
    grAW->addWidget(cbSkipLastOdd, 4, 3, 2, 1);

    lVer->addLayout(grAW);

    QObject::connect(cobShape, &QComboBox::currentIndexChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    QObject::connect(cbSkipFirstEven, &QCheckBox::stateChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    QObject::connect(cbSkipLastOdd, &QCheckBox::stateChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    QObject::connect(cobShape, &QComboBox::currentIndexChanged, this, [this, laR, laX, laY](int index)
    {
        laR->setVisible(index == 0);
        ledNumRings->setVisible(index == 0);

        laX->setVisible(index == 1);
        laY->setVisible(index == 1);
        ledNumX->setVisible(index == 1);
        ledNumY->setVisible(index == 1);
        cbSkipFirstEven->setVisible(index == 1);
        cbSkipLastOdd->setVisible(index == 1);
    });
    cobShape->setCurrentIndex(1);
    cobShape->setCurrentIndex(0);

    QHBoxLayout * lHor = new QHBoxLayout();
    lHor->addStretch();
    lHor->addWidget(new QLabel("Index of the first node:"));
    ledStartIndex = new AOneLineTextEdit("", Widget); ledStartIndex->bIntegerTooltip = true;
    lHor->addWidget(ledStartIndex);
    lHor->addStretch();

    lVer->addLayout(lHor);

    addLocalLayout(lVer);

    std::vector<AOneLineTextEdit*> l = {ledStep, ledNumRings, ledNumX, ledNumY, ledStartIndex};
    for (AOneLineTextEdit * le : l)
    {
        //le->setMaximumWidth(75);
        le->setContextMenuPolicy(Qt::NoContextMenu);
        configureHighligherAndCompleter(le);
        QObject::connect(le, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    }

    cbScale->setChecked(false);
    cbScale->setVisible(false);

    lMat->setVisible(false);
    cobMat->setVisible(false);

    pbTransform->setVisible(false);
    pbShapeInfo->setVisible(false);
}

bool AGeoHexagonalArrayDelegate::updateObject(AGeoObject *obj) const
{
    std::vector<AOneLineTextEdit*> v = {ledStep, ledNumRings, ledNumX, ledNumY, ledStartIndex};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    if (!CurrentObject->Type->isHexagonalArray()) return false;

    ATypeHexagonalArrayObject a;
    a.strStep       = ledStep->text();
    a.strRings      = ledNumRings->text();
    a.strNumX       = ledNumX->text();
    a.strNumY       = ledNumY->text();
    a.strStartIndex = ledStartIndex->text();

    a.Shape         = ( cobShape->currentIndex() == 0 ? ATypeHexagonalArrayObject::Hexagonal : ATypeHexagonalArrayObject::XY );
    a.SkipEvenFirst = cbSkipFirstEven->isChecked();
    a.SkipOddLast   = cbSkipLastOdd->isChecked();

    QString errorStr;
    a.introduceGeoConstValues(errorStr);
    if (!errorStr.isEmpty())
    {
        QMessageBox::warning(this->ParentWidget, "", errorStr);
        return false;
    }

    ATypeHexagonalArrayObject * array = static_cast<ATypeHexagonalArrayObject*>(obj->Type);
    *array = a;

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoHexagonalArrayDelegate::updateGui(const AGeoObject *obj)
{
    AGeoObjectDelegate::updateGui(obj);

    ATypeHexagonalArrayObject * array = dynamic_cast<ATypeHexagonalArrayObject*>(obj->Type);

    if (array)
    {
        cobShape->setCurrentIndex( array->Shape == ATypeHexagonalArrayObject::Hexagonal ? 0 : 1);
        ledStep->setText(array->strStep.isEmpty() ? QString::number(array->Step) : array->strStep);
        ledNumRings->setText(array->strRings.isEmpty() ? QString::number(array->Rings) : array->strRings);
        ledNumX->setText(array->strNumX.isEmpty() ? QString::number(array->NumX) : array->strNumX);
        ledNumY->setText(array->strNumY.isEmpty() ? QString::number(array->NumY) : array->strNumY);
        cbSkipFirstEven->setChecked(array->SkipEvenFirst);
        cbSkipLastOdd->setChecked(array->SkipOddLast);

        ledStartIndex->setText(array->strStartIndex.isEmpty() ? QString::number(array->startIndex) : array->strStartIndex);
    }
}

// ---

AGeoSetDelegate::AGeoSetDelegate(const QStringList &materials, QWidget *parent)
   : AGeoObjectDelegate(materials, parent)
{
     pbTransform->setVisible(false);
     pbShapeInfo->setVisible(false);

     cbScale->setChecked(false);
     cbScale->setVisible(false);
}

#include <QClipboard>
void AGeoSetDelegate::updateGui(const AGeoObject *obj)
{
    if (obj->Type->isCompositeContainer())
    {
        DelegateTypeName = "Container of logical shapes";
        pbShow->setVisible(false);
        pbChangeAtt->setVisible(false);
        pbScriptLine->setVisible(false);
    }
    else if (obj->Type->isStack())
    {
        DelegateTypeName = "Stack";

        QString thick = "--";
        AGeoBox * box = nullptr;
        if (obj->Shape) box = dynamic_cast<AGeoBox*>(obj->Shape);
        if (box) thick = QString::number(2*box->dz);

        QVBoxLayout * lay = new QVBoxLayout();
        lay->setAlignment(Qt::AlignHCenter);
        lay->addWidget(new QLabel(" "));
            QHBoxLayout * hl = new QHBoxLayout();
            hl->addStretch();
            hl->addWidget(new QLabel(QString("Stack thickness: %0 mm").arg(thick)));
            QPushButton * bInfo = new QPushButton("Info");
            connect(bInfo, &QPushButton::clicked, this, [this]()
                    {
                        QString txt = ""
                        "Stack is a logical object: it does not appear in the constructed geometry directly.\n"
                        "\n"
                        "The containing objects are auto-positioned to be 'in contact' in Z direction.\n"
                        "\n"
                        "The stack is placed inside the mother volume at the 'center' position.\n"
                        "There are two options which define where is the stack center:\n"
                        "1. The stack reference volume is NOT defined:\n"
                        "   In this case the center of the stack is the middle point of the stack total thickness.\n"
                        "2. A stack reference point is defined:\n"
                        "   In this case the center of the stack is the center of the reference object.\n"
                        "\n"
                        "The stack rotation is in respect to the stack center.\n"
                        "\n"
                        "The stack elements can be shifted lateraly and rotated around the axis.\n"
                        "\n"
                        "The stack can contain elementary objects (except composite objects) and other stacks,\n"
                        "but cannot host arrays or prototypes/instances.\n"
                        "If an instance has to be 'stacked', put a box in the stack and place the instance inside.\n"
                        ;

                        guitools::message1(txt, "info", this->ParentWidget);
                    });
            if (thick != "--")
            {
                QPushButton * bCopyNumber = new QPushButton("Copy");
                bCopyNumber->setContextMenuPolicy(Qt::CustomContextMenu);
                bCopyNumber->setToolTip("Left click: numerical value; right click: expression (if available)");
                connect(bCopyNumber, &QPushButton::clicked, this, [obj]()
                {
                    QClipboard * clipboard = QGuiApplication::clipboard();
                    QString txt = "n.a.";
                    if (obj->Shape) txt = QString::number(2.0 * obj->Shape->getHeight());
                    clipboard->setText(txt);
                });
                connect(bCopyNumber, &QPushButton::customContextMenuRequested, this, [obj]()
                {
                    QClipboard * clipboard = QGuiApplication::clipboard();
                    QString txt = "n.a.";
                    if (obj->Shape) txt = obj->Shape->getFullHeightString();
                    clipboard->setText(txt);
                });
                hl->addWidget(bCopyNumber);
            }
            hl->addWidget(bInfo);
            hl->addStretch();
        lay->addLayout(hl);
        lay->addWidget(new QLabel(" "));
        //lay->addWidget(new QLabel("Rotation in respect to the center of the stack!"));
        addLocalLayout(lay);
    }
    else qWarning() << "Unexpected object type in AGeoSetDelegate::Update()";

    AGeoObjectDelegate::updateGui(obj);
}

AWorldDelegate::AWorldDelegate(const QStringList & materials, QWidget * ParentWidget) :
    AGeoBaseDelegate(ParentWidget)
{
    QFrame * frMainFrame = new QFrame();
    frMainFrame->setFrameShape(QFrame::Box);

    Widget = frMainFrame;
    Widget->setContextMenuPolicy(Qt::CustomContextMenu);

    QPalette palette = frMainFrame->palette();
    //palette.setColor( Widget->backgroundRole(), QColor( 255, 255, 255 ) );
    palette.setColor( Widget->backgroundRole(), palette.color(QPalette::AlternateBase) );
    frMainFrame->setPalette( palette );
    frMainFrame->setAutoFillBackground( true );

    QVBoxLayout * lMF = new QVBoxLayout();
      lMF->setContentsMargins(5,5,5,2);

      QLabel * labType = new QLabel("World");
      labType->setAlignment(Qt::AlignCenter);
      QFont font = labType->font();
      font.setBold(true);
      labType->setFont(font);
      lMF->addWidget(labType);

      QHBoxLayout* hl = new QHBoxLayout();
        hl->setContentsMargins(2,0,2,0);

        QLabel * lMat = new QLabel();
        lMat->setText("Material:");
        lMat->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lMat->setMaximumWidth(60);
        hl->addWidget(lMat);

        cobMat = new QComboBox();
        cobMat->setContextMenuPolicy(Qt::NoContextMenu);
        cobMat->addItems(materials);
        //connect(cobMat, SIGNAL(activated(int)), this, SIGNAL(ContentChanged()));
        connect(cobMat, &QComboBox::activated, this, &AGeoBaseDelegate::onContentChangedBase);
        cobMat->setMinimumWidth(120);
        hl->addWidget(cobMat);
      lMF->addLayout(hl);

      QHBoxLayout * h = new QHBoxLayout();
            h->addStretch();
            cbFixedSize = new QCheckBox("Fixed size");
            cbFixedSize->setChecked(true);
            connect(cbFixedSize, &QCheckBox::clicked, this, &AGeoBaseDelegate::onContentChangedBase);
            h->addWidget(cbFixedSize);

            QVBoxLayout * v1 = new QVBoxLayout();
                v1->setContentsMargins(2,0,2,0);
                v1->addWidget(new QLabel("Size XY:"));
                v1->addWidget(new QLabel("Size Z:"));
            h->addLayout(v1);

            QVBoxLayout * v2 = new QVBoxLayout();
                v2->setContentsMargins(2,0,2,0);
                ledSizeXY = new AOneLineTextEdit(); v2->addWidget(ledSizeXY);
                ledSizeZ  = new AOneLineTextEdit(); v2->addWidget(ledSizeZ);
                for (AOneLineTextEdit * le : {ledSizeXY, ledSizeZ})
                {
                    configureHighligherAndCompleter(le);
                    connect(le, &AOneLineTextEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
                }
                connect(cbFixedSize, &QCheckBox::toggled, ledSizeXY, &AOneLineTextEdit::setEnabled);
                connect(cbFixedSize, &QCheckBox::toggled, ledSizeZ,  &AOneLineTextEdit::setEnabled);
            h->addLayout(v2);
            h->addStretch();
    lMF->addLayout(h);

    createBottomButtons();
    pbScriptLine->setEnabled(false);
    lMF->addWidget(frBottomButtons);

    frMainFrame->setLayout(lMF);
}

QString AWorldDelegate::getName() const
{
    return "World";
}

bool AWorldDelegate::updateObject(AGeoObject * obj) const
{
    std::vector<AOneLineTextEdit*> v = {ledSizeXY, ledSizeZ};
    if (isLeEmpty(v))
    {
        QMessageBox::warning(this->ParentWidget, "", "Empty line!");
        return false;
    }

    obj->Material = cobMat->currentIndex();
    if (obj->Material == -1) obj->Material = 0; //protection

    ATypeWorldObject * typeWorld = static_cast<ATypeWorldObject*>(obj->Type);
    typeWorld->bFixedSize = cbFixedSize->isChecked();

    //protection
    AGeoBox * box = static_cast<AGeoBox*>(obj->Shape);
    box->dx = 500.0;
    box->dy = 500.0;
    box->dz = 500.0;

    const AGeoConsts & GC = AGeoConsts::getConstInstance();
    QString errorStr;
    bool ok;
    double dx, dz;
    QString strSizeXY = ledSizeXY->text();
    QString strSizeZ  = ledSizeZ ->text();
    ok = GC.updateDoubleParameter(errorStr, strSizeXY, dx);
    if (!ok)
    {
        QMessageBox::warning(this->ParentWidget, "", errorStr + " in SizeXY\n");
        return false;
    }
    ok = GC.updateDoubleParameter(errorStr, strSizeZ,  dz);
    if (!ok)
    {
        QMessageBox::warning(this->ParentWidget, "", errorStr + "in SizeZ\n");
        return false;
    }

    box->dx = dx;
    box->dy = dx;
    box->dz = dz;
    box->str2dx = strSizeXY;
    box->str2dy = strSizeXY;
    box->str2dz = strSizeZ;

    return true;
}

void AWorldDelegate::updateGui(const AGeoObject *obj)
{
    int imat = obj->Material;
    if (imat < 0 || imat >= cobMat->count())
    {
        qWarning() << "Material index out of bounds!";
        imat = -1;
    }
    cobMat->setCurrentIndex(imat);

    ATypeWorldObject * typeWorld = static_cast<ATypeWorldObject*>(obj->Type);
    cbFixedSize->setChecked(typeWorld->bFixedSize);

    const AGeoBox * box = static_cast<const AGeoBox*>(obj->Shape);
    ledSizeXY->setText(box->str2dx.isEmpty() ? QString::number(box->dx*2.0) : box->str2dx);
    ledSizeZ ->setText(box->str2dz.isEmpty() ? QString::number(box->dz*2.0) : box->str2dz);

    updateLineColorFrame(obj);
}

AGeoInstanceDelegate::AGeoInstanceDelegate(const QStringList &materials, QWidget *parent)
   : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Instance";

    QHBoxLayout * hl = new QHBoxLayout();
    hl->setContentsMargins(50, 0, 50, 0);

    QLabel * la  = new QLabel("Instance of:");                   hl->addWidget(la);
    leInstanceOf = new QLineEdit();                              hl->addWidget(leInstanceOf);
    QPushButton * pbToProto = new QPushButton("Show prototype"); hl->addWidget(pbToProto);

    QObject::connect(leInstanceOf, &QLineEdit::textChanged, this, &AGeoBaseDelegate::onContentChangedBase);
    QObject::connect(pbToProto, &QPushButton::clicked, [this](){
        emit RequestShowPrototype(leInstanceOf->text());
    });

    addLocalLayout(hl);

    cbScale->setChecked(false);
    cbScale->setVisible(false);

    lMat->setVisible(false);
    cobMat->setVisible(false);

    pbTransform->setVisible(false);
    pbShapeInfo->setVisible(false);

    pbChangeAtt->setEnabled(false);
}

bool AGeoInstanceDelegate::updateObject(AGeoObject * obj) const
{
    const QString ProtoName = leInstanceOf->text();

    ATypeInstanceObject * instance = dynamic_cast<ATypeInstanceObject*>(obj->Type);
    if (instance)
    {
        if (ProtoName != instance->PrototypeName)
        {
            bool bValid;
            emit RequestIsValidPrototypeName(ProtoName, bValid);
            if (bValid) instance->PrototypeName = ProtoName;
            else
            {
                QMessageBox::warning(this->ParentWidget, "", "This is not a valid prototype name: " + ProtoName);
                return false;
            }
        }
    }

    return AGeoObjectDelegate::updateObject(obj);
}

void AGeoInstanceDelegate::updateGui(const AGeoObject * obj)
{
    AGeoObjectDelegate::updateGui(obj);

    ATypeInstanceObject * inst = dynamic_cast<ATypeInstanceObject*>(obj->Type);
    if (inst) leInstanceOf->setText(inst->PrototypeName);
}

AGeoPrototypeDelegate::AGeoPrototypeDelegate(const QStringList & materials, QWidget * parent)
    : AGeoObjectDelegate(materials, parent)
{
    DelegateTypeName = "Prototype";

    cbScale->setChecked(false);
    cbScale->setVisible(false);

    lMat->setVisible(false);
    cobMat->setVisible(false);

    pbTransform->setVisible(false);
    pbShapeInfo->setVisible(false);
}
