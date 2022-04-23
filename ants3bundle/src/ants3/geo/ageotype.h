#ifndef ATYPEGEOOBJECT_H
#define ATYPEGEOOBJECT_H

#include "amonitorconfig.h"

#include <QString>
#include <QRegularExpression>

class QJsonObject;

class AGeoType
{
public:
    virtual ~AGeoType() {}

    // !!!*** String -> pointer (pType and use static QStrings)
    bool isHandlingStandard() const {return Handling == "Standard";}    // Single, Composite, Grid, GridElement, Monitor, Instance
    bool isHandlingSet() const      {return Handling == "Set";}         // Stack, CompositeContainer, Prototype
    bool isHandlingArray() const    {return Handling == "Array";}       // Array, CircularArray, HexagonalArray

    bool isWorld() const            {return Type == "World";}
    bool isPrototypes() const       {return Type == "PrototypeCollection";}
    bool isStack() const            {return Type == "Stack";}
    bool isLogical() const          {return Type == "Logical";}
    bool isSingle() const           {return Type == "Single";}
    bool isCompositeContainer() const {return Type == "CompositeContainer";}
    bool isComposite() const        {return Type == "Composite";}
    bool isArray() const            {return Type == "Array";}
    bool isCircularArray() const    {return Type == "CircularArray";}
    bool isHexagonalArray() const   {return Type == "HexagonalArray";}
    bool isInstance() const         {return Type == "Instance";}
    bool isPrototype() const        {return Type == "Prototype";}
    bool isGrid() const             {return Type == "Grid";}
    bool isGridElement() const      {return Type == "GridElement";}
    bool isMonitor() const          {return Type == "Monitor";}

    virtual bool isGeoConstInUse(const QRegularExpression & /*nameRegExp*/) const {return false;}
    virtual void replaceGeoConstName(const QRegularExpression & /*nameRegExp*/, const QString & /*newName*/) {}

    virtual void writeToJson(QJsonObject & json) const;         // CALL THIS, then save additional properties of the concrete type
    virtual void readFromJson(const QJsonObject & /*json*/) {}  // if present, read properties of the concrete type

    virtual QString introduceGeoConstValues() {return "";}

    static AGeoType * makeTypeObject(const QString & typeStr);  // AGeoType factory

protected:
    //const QString * pType = nullptr;
    QString Type;
    QString Handling;
};

// -- static objects --

class ATypeWorldObject : public AGeoType
{
public:
    ATypeWorldObject() {Type = "World"; Handling = "Static";}

    bool bFixedSize = false;

    void writeToJson(QJsonObject & json) const override;
    void readFromJson(const QJsonObject & json) override;
};

class ATypePrototypeCollectionObject : public AGeoType
{
public:
    ATypePrototypeCollectionObject() {Type = "PrototypeCollection"; Handling = "Logical";}
};

// -- Set objects --

class ATypeStackContainerObject : public AGeoType
{
public:
    ATypeStackContainerObject() {Type = "Stack"; Handling = "Set";}

    QString ReferenceVolume;

    void writeToJson(QJsonObject & json) const override;
    void readFromJson(const QJsonObject & json) override;
};

class ATypeCompositeContainerObject : public AGeoType
{
public:
    ATypeCompositeContainerObject() {Type = "CompositeContainer"; Handling = "Set";}
};


// -- Objects --

class ATypeSingleObject : public AGeoType
{
public:
    ATypeSingleObject() {Type = "Single"; Handling = "Standard";}
};

class ATypeCompositeObject : public AGeoType
{
public:
    ATypeCompositeObject() {Type = "Composite"; Handling = "Standard";}
};

class ATypeArrayObject : public AGeoType
{
public:
    ATypeArrayObject() {Type = "Array"; Handling = "Array";}
    ATypeArrayObject(int numX, int numY, int numZ, double stepX, double stepY, double stepZ, int startIndex = 0)
        : numX(numX), numY(numY), numZ(numZ), stepX(stepX), stepY(stepY), stepZ(stepZ), startIndex(startIndex) {Type = "Array"; Handling = "Array";}

    void Reconfigure(int NumX, int NumY, int NumZ, double StepX, double StepY, double StepZ);

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    void writeToJson(QJsonObject & json) const override;
    void readFromJson(const QJsonObject & json) override;

    QString introduceGeoConstValues() override;

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
    ATypeCircularArrayObject() {Type = "CircularArray"; Handling = "Array";}
    ATypeCircularArrayObject(int num, double angularStep, double radius, int StartIndex = 0)
        : num(num), angularStep(angularStep), radius(radius) {Type = "CircularArray"; Handling = "Array"; startIndex = StartIndex;}

    void Reconfigure(int Num, double AngularStep, double Radius);

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    void writeToJson(QJsonObject & json) const override;
    void readFromJson(const QJsonObject & json) override;

    QString introduceGeoConstValues() override;

    int    num         = 6;
    double angularStep = 30.0; //in degrees
    double radius      = 100.0;
    QString strNum, strAngularStep, strRadius;
};

class ATypeHexagonalArrayObject : public ATypeArrayObject
{
public:
    enum EShapeMode {Hexagonal, XY};
    ATypeHexagonalArrayObject() {Type = "HexagonalArray"; Handling = "Array";}

    void Reconfigure(double step, EShapeMode shape, int rings, int numX, int numY, bool skipOddLast);

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    void writeToJson(QJsonObject & json) const override;
    void readFromJson(const QJsonObject & json) override;

    QString introduceGeoConstValues() override;

    double     Step         = 30.0;
    EShapeMode Shape        = Hexagonal;
    int        Rings        = 3;
    int        NumX         = 5;
    int        NumY         = 4;
    bool       SkipOddLast = true;

    QString    strStep, strRings, strNumX, strNumY;
};

class ATypeGridObject : public AGeoType
{
public:
    ATypeGridObject() {Type = "Grid"; Handling = "Standard";}
};

class ATypeGridElementObject : public AGeoType
{
public:
    ATypeGridElementObject() {Type = "GridElement"; Handling = "Standard";}

    void writeToJson(QJsonObject & json) const override;
    void readFromJson(const QJsonObject & json) override;

    int    shape = 1;       //0 : rectanglar-2wires, 1 : rectangular-crossed,  2 : hexagonal
    double size1 = 10.0;    //half sizes for rectangular, size1 is size of hexagon
    double size2 = 10.0;
    double dz    = 5.0;     //half size in z
};

class ATypeMonitorObject : public AGeoType
{
public:
    ATypeMonitorObject() {Type = "Monitor"; Handling = "Standard";}

    void writeToJson(QJsonObject & json) const override;
    void readFromJson(const QJsonObject & json) override;

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    QString introduceGeoConstValues() override;

    AMonitorConfig config;

    //runtime, not saved
    int index;  //index of this monitor to access statistics   // !!!*** check, seems not in use anymore
};

class ATypePrototypeObject : public AGeoType
{
public:
    ATypePrototypeObject() {Type = "Prototype"; Handling = "Set";}
};

class ATypeInstanceObject : public AGeoType
{
public:
    ATypeInstanceObject(QString PrototypeName = "") : PrototypeName(PrototypeName) {Type = "Instance"; Handling = "Standard";}

    void writeToJson(QJsonObject & json) const override;
    void readFromJson(const QJsonObject & json) override;

    QString PrototypeName;
};

#endif // ATYPEGEOOBJECT_H
