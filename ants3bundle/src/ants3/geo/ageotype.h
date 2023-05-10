#ifndef ATYPEGEOOBJECT_H
#define ATYPEGEOOBJECT_H

#include "amonitorconfig.h"

#include <QString>

class QJsonObject;
class QRegularExpression;

class AGeoType
{
public:
    static AGeoType * makeTypeObject(const QString & typeStr);  // AGeoType factory

    virtual ~AGeoType() {}

    bool isWorld() const;
    bool isPrototypeCollection() const;
    bool isStack() const;
    bool isLogical() const;
    bool isSingle() const;
    bool isCompositeContainer() const;
    bool isComposite() const;
    bool isArray() const;
    bool isCircularArray() const;
    bool isHexagonalArray() const;
    bool isInstance() const;
    bool isPrototype() const;
    bool isGrid() const;
    bool isGridElement() const;
    bool isMonitor() const;

    bool isHandlingStandard() const;    // Single, Monitor, Instance, Composite, Grid, GridElement
    bool isHandlingSet() const;         // Stack, Prototype, CompositeContainer
    bool isHandlingArray() const;       // Array, CircularArray, HexagonalArray

    virtual bool isGeoConstInUse(const QRegularExpression & /*nameRegExp*/) const {return false;}
    virtual void replaceGeoConstName(const QRegularExpression & /*nameRegExp*/, const QString & /*newName*/) {}

    void writeToJson(QJsonObject & json) const;
    virtual void readFromJson(const QJsonObject & /*json*/) {}

    virtual void introduceGeoConstValues(QString & /*errorStr*/) {}

    virtual void scale(double /*factor*/) {}

protected:
    const QString * pType = nullptr;

    virtual void doWriteToJson(QJsonObject & /*json*/) const {}
};

// -- static objects --

class ATypeWorldObject : public AGeoType
{
public:
    ATypeWorldObject();

    bool bFixedSize = false;

    void doWriteToJson(QJsonObject & json) const override;
    void readFromJson(const QJsonObject & json) override;
};

class ATypePrototypeCollectionObject : public AGeoType
{
public:
    ATypePrototypeCollectionObject();
};

// -- Objects --

class ATypeSingleObject : public AGeoType
{
public:
    ATypeSingleObject();
};

class ATypeCompositeObject : public AGeoType
{
public:
    ATypeCompositeObject();
};

// -- Set objects --

class ATypeStackContainerObject : public AGeoType
{
public:
    ATypeStackContainerObject();

    QString ReferenceVolume;

    void doWriteToJson(QJsonObject & json) const override;
    void readFromJson(const QJsonObject & json) override;
};

class ATypeCompositeContainerObject : public AGeoType
{
public:
    ATypeCompositeContainerObject();
};

// -- Prototype / Instance --

class ATypePrototypeObject : public AGeoType
{
public:
    ATypePrototypeObject();
};

class ATypeInstanceObject : public AGeoType
{
public:
    ATypeInstanceObject(QString PrototypeName = "");

    void doWriteToJson(QJsonObject & json) const override;
    void readFromJson(const QJsonObject & json) override;

    QString PrototypeName;
};

// -- Arrays --

class ATypeArrayObject : public AGeoType
{
public:
    ATypeArrayObject();
    ATypeArrayObject(int numX, int numY, int numZ, double stepX, double stepY, double stepZ, int startIndex = 0);

    void Reconfigure(int NumX, int NumY, int NumZ, double StepX, double StepY, double StepZ);

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    void doWriteToJson(QJsonObject & json) const override;
    void readFromJson(const QJsonObject & json) override;

    void introduceGeoConstValues(QString & errorStr) override; // !!!***

    void scale(double factor) override;

    bool bCenterSymmetric = true;
    int numX = 2;
    int numY = 2;
    int numZ = 1;
    double stepX = 25.0;
    double stepY = 25.0;
    double stepZ = 25.0;
    QString strNumX, strNumY, strNumZ, strStepX, strStepY, strStepZ;
    int startIndex = 0;
    QString strStartIndex;
};

class ATypeCircularArrayObject : public ATypeArrayObject
{
public:
    ATypeCircularArrayObject();
    ATypeCircularArrayObject(int num, double angularStep, double radius, int StartIndex = 0);

    void Reconfigure(int Num, double AngularStep, double Radius);

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    void doWriteToJson(QJsonObject & json) const override;
    void readFromJson(const QJsonObject & json) override;

    void introduceGeoConstValues(QString & errorStr) override;

    void scale(double factor) override;

    int    num         = 6;
    double angularStep = 30.0; //in degrees
    double radius      = 100.0;
    QString strNum, strAngularStep, strRadius;
};

class ATypeHexagonalArrayObject : public ATypeArrayObject
{
public:
    enum EShapeMode {Hexagonal, XY};
    ATypeHexagonalArrayObject();

    void reconfigure(double step, EShapeMode shape, int rings, int numX, int numY, bool skipOddLast);

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    void doWriteToJson(QJsonObject & json) const override;
    void readFromJson(const QJsonObject & json) override;

    void introduceGeoConstValues(QString & errorStr) override;

    void scale(double factor) override;

    double     Step         = 30.0;
    EShapeMode Shape        = Hexagonal;
    int        Rings        = 3;
    int        NumX         = 5;
    int        NumY         = 4;
    bool       SkipOddLast = true;

    QString    strStep, strRings, strNumX, strNumY;
};

// -- Monitors --

class ATypeMonitorObject : public AGeoType
{
public:
    ATypeMonitorObject();

    void doWriteToJson(QJsonObject & json) const override;
    void readFromJson(const QJsonObject & json) override;

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    void introduceGeoConstValues(QString & errorStr) override;

    // no scaling! otherwise applied twice

    AMonitorConfig config;

    //runtime, not saved
    int index;  //index of this monitor to access statistics   // !!!*** check, seems not in use anymore
};

// -- Optical grids --

class ATypeGridObject : public AGeoType
{
public:
    ATypeGridObject();
};

class ATypeGridElementObject : public AGeoType
{
public:
    ATypeGridElementObject();

    void doWriteToJson(QJsonObject & json) const override;
    void readFromJson(const QJsonObject & json) override;

    int    shape = 1;       //0 : rectanglar-2wires, 1 : rectangular-crossed,  2 : hexagonal
    double size1 = 10.0;    //half sizes for rectangular, size1 is size of hexagon
    double size2 = 10.0;
    double dz    = 5.0;     //half size in z
};

#endif // ATYPEGEOOBJECT_H
