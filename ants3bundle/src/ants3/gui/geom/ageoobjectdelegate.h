#ifndef AGEOOBJECTDELEGATE_H
#define AGEOOBJECTDELEGATE_H

#include "ageobasedelegate.h"

#include <QString>
#include <QStringList>

#include <vector>

class AGeoObject;
class AGeoShape;
class QWidget;
class QVBoxLayout;
class QGridLayout;
class QLineEdit;
class QLabel;
class QCheckBox;
class QComboBox;
class QPlainTextEdit;
class QPushButton;
class QLayout;
class QDialog;
class QListWidget;
class QTableWidget;
class AOneLineTextEdit;
class QFrame;
class ACalorimeterProperties;
class APhotonFunctionalModel;
class AFunctionalModelWidget;
class AParticleAnalyzerWidget;

class AGeoObjectDelegate : public AGeoBaseDelegate
{
    Q_OBJECT

public:
    AGeoObjectDelegate(const QStringList & materials, QWidget * ParentWidget = nullptr);
    virtual ~AGeoObjectDelegate();

    QString getName() const override;

    bool    updateObject(AGeoObject * obj) const override;

    void    updateGui(const AGeoObject * obj) override;

protected:
    QVBoxLayout * lMF = nullptr;      //main layout

    QWidget * scaleWidget = nullptr;
    AOneLineTextEdit * ledScaleX   = nullptr;
    AOneLineTextEdit * ledScaleY   = nullptr;
    AOneLineTextEdit * ledScaleZ   = nullptr;

    const AGeoObject * CurrentObject = nullptr;

    AGeoShape * ShapeCopy = nullptr;

    QLabel    * labType = nullptr;
    QCheckBox * cbScale = nullptr;

    QString DelegateTypeName = "Object";
    QString ShapeHelp;

    QPushButton * pbTransform = nullptr;
    QPushButton * pbShapeInfo = nullptr;

    QLineEdit* leName;
    QComboBox* cobMat;
    QWidget* PosOrient;
    QLabel* lMat;
    AOneLineTextEdit *ledX,   *ledY,     *ledZ;
    AOneLineTextEdit *ledPhi, *ledTheta, *ledPsi;

    QStringList ListOfShapesForTransform;

    QWidget   * RoleWidget = nullptr;
    QComboBox * cobRole = nullptr;
    QComboBox * cobSensorModel = nullptr;
    QComboBox * cobCalType;
    QCheckBox * cbCalIncludeHosted;
    QCheckBox * cbCalRandomize;
    AOneLineTextEdit *ledCalOriginX, *ledCalOriginY, *ledCalOriginZ;
    AOneLineTextEdit *ledCalStepX, *ledCalStepY, *ledCalStepZ;
    AOneLineTextEdit *leiCalBinsX, *leiCalBinsY, *leiCalBinsZ;
    QCheckBox *cbOffX, *cbOffY, *cbOffZ;
    //QCheckBox * cbCalEventStat;
    AOneLineTextEdit * leiCalEventDepoBins, *ledCalEventDepoFrom, *ledCalEventDepoTo;

    QLineEdit * lePhFunModelName = nullptr;
    QPushButton * pbSelectPhFunModel = nullptr;
    APhotonFunctionalModel * LocalPhFunModel = nullptr;  // !!!*** add to destructor?
    AFunctionalModelWidget * PhFunModelWidget = nullptr; // !!!*** add to destructor?
    QVBoxLayout * vblPhFun = nullptr;

    AParticleAnalyzerWidget * PartAnWidget = nullptr;

private slots:
    void onContentChanged();          // only to enter the editing mode! Object update is performed only on confirm button click!
    void onHelpRequested();           // dialog with AGeoShape list can be accessed here
    void onChangeShapePressed();
    void onScaleToggled();

protected:
    void addLocalLayout(QLayout * lay);
    QString updateScalingFactors() const;
    const AGeoShape * getBaseShapeOfObject(const AGeoObject *obj);
    void updateTypeLabel();
    void updateControlUI();
    void crateSpecialRoleWidget();
    void updateCalorimeterGui(const ACalorimeterProperties & p);
    void updatePhFunModelGui();

private:
    void onShapeDialogActivated(QDialog * d, QListWidget * w);
    void onSelectPhFunModelClicked();

    QFrame * createSensorGui();
    QFrame * createCalorimeterGui();
    QFrame * createFunctionalModelGui();
    QFrame * createParticleAnalyzerGui();

signals:
    void RequestChangeShape(AGeoShape * newShape);
    void RequestChangeSlabShape(int Shape);

public:
    static bool processDoubleEditBox(const QString & whatIsIt, AOneLineTextEdit * lineEdit, double & val, QString & str, bool bForbidZero, bool bForbidNegative, bool bMakeHalf, QWidget * parent);
    static bool processIntEditBox(const QString & whatIsIt, AOneLineTextEdit * lineEdit, int & val, QString & str, bool bForbidZero, bool bForbidNegative, QWidget * parent);
};


class AGeoBoxDelegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoBoxDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit * ex = nullptr;
    AOneLineTextEdit * ey = nullptr;
    AOneLineTextEdit * ez = nullptr;
};

class AWorldDelegate : public AGeoBaseDelegate
{
    Q_OBJECT

public:
    AWorldDelegate(const QStringList & materials, QWidget * ParentWidget);

    bool updateObject(AGeoObject * obj) const override;

    QString getName() const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    QComboBox        * cobMat      = nullptr;
    QCheckBox        * cbFixedSize = nullptr;
    AOneLineTextEdit * ledSizeXY   = nullptr;
    AOneLineTextEdit * ledSizeZ    = nullptr;
};

class AGeoTubeDelegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoTubeDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit * ei = nullptr;
    AOneLineTextEdit * eo = nullptr;
    AOneLineTextEdit * ez = nullptr;

    QGridLayout * gr    = nullptr;
    QLabel      * labIm = nullptr;
    QLabel      * labIu = nullptr;
};

class AGeoTubeSegDelegate : public AGeoTubeDelegate
{
    Q_OBJECT

public:
    AGeoTubeSegDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit * ep1 = nullptr;
    AOneLineTextEdit * ep2 = nullptr;
};

class AGeoTubeSegCutDelegate : public AGeoTubeSegDelegate
{
    Q_OBJECT

public:
    AGeoTubeSegCutDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit * elnx = nullptr;
    AOneLineTextEdit * elny = nullptr;
    AOneLineTextEdit * elnz = nullptr;
    AOneLineTextEdit * eunx = nullptr;
    AOneLineTextEdit * euny = nullptr;
    AOneLineTextEdit * eunz = nullptr;
};

class AGeoElTubeDelegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoElTubeDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit * ex = nullptr;
    AOneLineTextEdit * ey = nullptr;
    AOneLineTextEdit * ez = nullptr;

    QGridLayout * gr = nullptr;
};

class AGeoParaDelegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoParaDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit * ex = nullptr;
    AOneLineTextEdit * ey = nullptr;
    AOneLineTextEdit * ez = nullptr;
    AOneLineTextEdit * ea = nullptr;
    AOneLineTextEdit * et = nullptr;
    AOneLineTextEdit * ep = nullptr;
};

class AGeoSphereDelegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoSphereDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit * eod = nullptr;
    AOneLineTextEdit * eid = nullptr;
    AOneLineTextEdit * et1 = nullptr;
    AOneLineTextEdit * et2 = nullptr;
    AOneLineTextEdit * ep1 = nullptr;
    AOneLineTextEdit * ep2 = nullptr;
};

class AGeoConeDelegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoConeDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit * ez  = nullptr;
    AOneLineTextEdit * eli = nullptr;
    AOneLineTextEdit * elo = nullptr;
    AOneLineTextEdit * eui = nullptr;
    AOneLineTextEdit * euo = nullptr;

    QGridLayout * gr = nullptr;
};

class AGeoConeSegDelegate : public AGeoConeDelegate
{
    Q_OBJECT

public:
    AGeoConeSegDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit * ep1 = nullptr;
    AOneLineTextEdit * ep2 = nullptr;
};

class AGeoTrapXDelegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoTrapXDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit * exl = nullptr;
    AOneLineTextEdit * exu = nullptr;
    AOneLineTextEdit * ey = nullptr;
    AOneLineTextEdit * ez = nullptr;
};

class AGeoTrapXYDelegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoTrapXYDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit * exl = nullptr;
    AOneLineTextEdit * exu = nullptr;
    AOneLineTextEdit * eyl = nullptr;
    AOneLineTextEdit * eyu = nullptr;
    AOneLineTextEdit * ez  = nullptr;
};

class AGeoParaboloidDelegate : public AGeoObjectDelegate                      //paraboloiid
{
    Q_OBJECT

public:
    AGeoParaboloidDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit * el = nullptr;
    AOneLineTextEdit * eu = nullptr;
    AOneLineTextEdit * ez = nullptr;
};

class AGeoTorusDelegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoTorusDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit * ead = nullptr;
    AOneLineTextEdit * edi = nullptr;
    AOneLineTextEdit * edo = nullptr;
    AOneLineTextEdit * ep0 = nullptr;
    AOneLineTextEdit * epe = nullptr;
};

class AGeoPolygonDelegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoPolygonDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    virtual void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit  * en = nullptr;
    AOneLineTextEdit * edp = nullptr;
    AOneLineTextEdit * ez  = nullptr;
    AOneLineTextEdit * elo = nullptr;
    AOneLineTextEdit * eli = nullptr;
    AOneLineTextEdit * euo = nullptr;
    AOneLineTextEdit * eui = nullptr;

    QLabel * labLO  = nullptr;
    QLabel * labLI  = nullptr;
    QLabel * labUO  = nullptr;
    QLabel * labUI  = nullptr;
    QLabel * labA   = nullptr;

    QLabel * labLIu = nullptr;
    QLabel * labUOu = nullptr;
    QLabel * labUIu = nullptr;
    QLabel * labAu  = nullptr;
};

class AGeoPcon;
class AGeoPconDelegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoPconDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit * ep0 = nullptr;
    AOneLineTextEdit * epe = nullptr;

    QTableWidget * tab = nullptr;
    QVBoxLayout  * lay = nullptr;

    const int rowHeight = 23;

    void addOneLineTextEdits(int row);
    void readGui() const;
    void updateTableW(AGeoPcon * pcon);

private slots:
    void onReorderSections(int /*logicalIndex*/, int oldVisualIndex, int newVisualIndex);
    void onCellEdited();  // !!!*** check error handling
    void onAddAbove();
    void onAddBellow();
};

class AGeoPgonDelegate : public AGeoPconDelegate
{
    Q_OBJECT

public:
    AGeoPgonDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit  * eed = nullptr;
};

class AGeoCompositeDelegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoCompositeDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    QPlainTextEdit * te = nullptr;
};

struct AEditEdit
{
    AOneLineTextEdit * X = nullptr;
    AOneLineTextEdit * Y = nullptr;
};

class AGeoArb8Delegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoArb8Delegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit * ez = nullptr;

    std::vector<std::vector<AEditEdit>> ve; //[0..1][0..3] - upper/lower, 4 points
};

class AGeoArrayDelegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoArrayDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit *ledNumX, *ledNumY, *ledNumZ;
    AOneLineTextEdit *ledStepX, *ledStepY, *ledStepZ;
    AOneLineTextEdit *ledStartIndex;
    QCheckBox        *cbCenterSym;

private slots:
    void updateArrayStepEnable(AOneLineTextEdit *editNum, AOneLineTextEdit *editStep);
};

class AGeoCircularArrayDelegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoCircularArrayDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    AOneLineTextEdit *ledNum;
    AOneLineTextEdit *ledAngularStep;
    AOneLineTextEdit *ledRadius;
    AOneLineTextEdit *ledStartIndex;
};

class AGeoHexagonalArrayDelegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoHexagonalArrayDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    QComboBox        * cobShape;
    AOneLineTextEdit * ledStep;
    AOneLineTextEdit * ledNumRings;
    AOneLineTextEdit * ledNumX;
    AOneLineTextEdit * ledNumY;
    QCheckBox        * cbSkipFirstEven;
    QCheckBox        * cbSkipLastOdd;

    AOneLineTextEdit * ledStartIndex;
};

class AGeoSetDelegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoSetDelegate(const QStringList & materials, QWidget * parent);

    void updateGui(const AGeoObject * obj) override; // !!!*** add button to copy thickness in Geo Consts
};

class AGeoInstanceDelegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoInstanceDelegate(const QStringList & materials, QWidget * parent);

    bool updateObject(AGeoObject * obj) const override;

    void updateGui(const AGeoObject * obj) override;

protected:
    QLineEdit * leInstanceOf = nullptr;

signals:
    void RequestShowPrototype(QString ProtoName);
    void RequestIsValidPrototypeName(const QString & ProtoName, bool & bResult) const;
};

class AGeoPrototypeDelegate : public AGeoObjectDelegate
{
    Q_OBJECT

public:
    AGeoPrototypeDelegate(const QStringList & materials, QWidget * parent);
};

#endif // AGEOOBJECTDELEGATE_H
