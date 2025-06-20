#ifndef AGEOSHAPE_H
#define AGEOSHAPE_H

#include <QString>
#include <QList>
#include <QStringList>

#include <array>

class TGeoShape;
class QJsonObject;
class QRegularExpression;

class AGeoShape
{
public:
    virtual ~AGeoShape() {}

    virtual bool readFromString(QString /*GenerationString*/) {return false;}

    //general: the same for all objects of the given shape
    virtual QString getShapeType() const = 0;
    virtual QString getShortName() const {return QStringLiteral("New");}
    virtual QString getShapeTemplate() const = 0;  //string used in auto generated help: Name(paramType param1, paramType param2, etc)
    virtual QString getHelp() const = 0;

    //specific for this particular object
    virtual TGeoShape * createGeoShape(const QString /*shapeName*/ = "") = 0;    //creates ROOT's TGeoShape
    virtual QString getGenerationString(bool /*useStrings*/ = false) const = 0;

    virtual QString getScriptString(bool /*useStrings*/) const {return QString();}

    virtual double getHeight() const {return 0;}   //half height, implemented for stacks; if 0, cannot be used in a stack
    virtual QString getFullHeightString() {return "";} //full height string for stacks; returns empty string if geo height is not given using an expression
    virtual double getRelativePosZofCenter() const {return 0;} //for polycones and polygons in stacks
    virtual void   setHeight(double /*dz*/) {}     //for stacks
    virtual double maxSize() const = 0;            //used for world size evaluation
    virtual double minSize() const {return 0;}     //needed only for shapes used by monitors (box tube polygon)

    virtual void introduceGeoConstValues(QString & /*errorStr*/) {}

    virtual bool isGeoConstInUse(const QRegularExpression & /*nameRegExp*/) const = 0;
    virtual void replaceGeoConstName(const QRegularExpression & /*nameRegExp*/, const QString & /*newName*/) {}

    //json
    virtual void writeToJson(QJsonObject &/*json*/) const = 0;
    virtual void readFromJson(const QJsonObject &/*json*/) = 0;

    //from TShape if geometry was loaded from GDML
    virtual bool readFromTShape(TGeoShape* /*Tshape*/) {return false;}

    virtual AGeoShape * clone() const; // uses GeoShapeFactory and save/load to/from json

    virtual void scale(double factor) = 0;

    virtual bool isCompatibleWithGeant4() const {return true;}

protected:
    bool    extractParametersFromString(QString GenerationString, QStringList& parameters, int numParameters);

public:
    static AGeoShape * GeoShapeFactory(const QString ShapeType);  // -=<  SHAPE FACTORY >=-
//    static QList<AGeoShape*> getAvailableShapes();                // list of available shapes for generation of help and highlighter: do not forget to add new here!
};

// -------------- Particular shapes ---------------

class AGeoBox : public AGeoShape
{
public:
    AGeoBox(double dx, double dy, double dz) : dx(dx), dy(dy), dz(dz) {}
    AGeoBox() : dx(10), dy(10), dz(10) {}

    QString getShapeType() const override {return "TGeoBBox";}
    QString getShortName() const override {return QStringLiteral("Box");}
    QString getShapeTemplate() const override {return "TGeoBBox( dx, dy, dz )";}
    QString getHelp() const override;

    bool readFromString(QString GenerationString) override;
    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    TGeoShape* createGeoShape(const QString shapeName = "") override;

    double getHeight() const override {return dz;}
    QString getFullHeightString() override {return str2dz;}
    void setHeight(double dz) override {this->dz = dz;}
    QString getGenerationString(bool useStrings) const override;
    QString getScriptString(bool useStrings) const override;
    double maxSize() const override;
    double minSize() const override;

    void writeToJson(QJsonObject& json) const override;
    void readFromJson(const QJsonObject& json) override;

    bool readFromTShape(TGeoShape* Tshape) override;

    void scale(double factor) override;

    double      dx,     dy,     dz;
    QString str2dx, str2dy, str2dz;
};

class AGeoTube : public AGeoShape
{
public:
    AGeoTube(double rmin, double rmax, double dz) : rmin(rmin), rmax(rmax), dz(dz) {}
    AGeoTube(double r, double dz) : rmin(0), rmax(r), dz(dz) {}
    AGeoTube() : rmin(0), rmax(10), dz(5) {}

    QString getShapeType() const override {return "TGeoTube";}
    QString getShortName() const override {return QStringLiteral("Tube");}
    QString getShapeTemplate() const override {return "TGeoTube( rmin, rmax, dz )";}
    QString getHelp() const override;

    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    bool readFromString(QString GenerationString) override;
    TGeoShape* createGeoShape(const QString shapeName = "") override;

    double getHeight() const override {return dz;}
    QString getFullHeightString() override {return str2dz;}
    void setHeight(double dz) override {this->dz = dz;}
    QString getGenerationString(bool useStrings) const override;
    QString getScriptString(bool useStrings) const override;
    double maxSize() const override;
    double minSize() const override;

    void writeToJson(QJsonObject& json) const override;
    void readFromJson(const QJsonObject& json) override;

    bool readFromTShape(TGeoShape* Tshape) override;

    void scale(double factor) override;

    double      rmin,     rmax,     dz;
    QString str2rmin, str2rmax, str2dz;
};

// comment: "remove compatibility mode" <- is it still valid?
class AGeoScaledShape  : public AGeoShape
{
public:
    AGeoScaledShape(QString BaseShapeGenerationString, double scaleX, double scaleY, double scaleZ);
    AGeoScaledShape() {}
    ~AGeoScaledShape() {delete BaseShape;}

    QString getShapeType() const override {return "TGeoScaledShape";}
    QString getShortName() const override {return QStringLiteral("Scaled");}
    QString getShapeTemplate() const override {return "TGeoScaledShape( TGeoShape(parameters), scaleX, scaleY, scaleZ )";}
    QString getHelp() const override;
    void updateScalingFactors(QString & errorStr);

    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    bool readFromString(QString GenerationString) override;
    TGeoShape* createGeoShape(const QString shapeName = "") override;

    double getHeight() const override;
    QString getFullHeightString() override;
    double getRelativePosZofCenter() const override;
    void setHeight(double dz) override;
    QString getGenerationString(bool useStrings) const override;
    QString getScriptString(bool useStrings) const override; // returns script string for the base shape!
    QString getScriptString_Scaled(bool useStrings) const;
    double maxSize() const override;

    const QString getBaseShapeType() const;
    TGeoShape * generateBaseTGeoShape(const QString & BaseShapeGenerationString) const;

    void writeToJson(QJsonObject& json) const override;
    void readFromJson(const QJsonObject& json) override; // !!!*** check error control

    bool readFromTShape(TGeoShape* Tshape) override;

    void scale(double factor) override;

    bool isCompatibleWithGeant4() const override;

    QString BaseShapeGenerationString;

    double scaleX = 1.0;
    double scaleY = 1.0;
    double scaleZ = 1.0;
    QString strScaleX, strScaleY, strScaleZ;
    AGeoShape * BaseShape = nullptr;
};

class AGeoParaboloid : public AGeoShape
{
public:
    AGeoParaboloid(double rlo, double rhi, double dz) : rlo(rlo), rhi(rhi), dz(dz) {}
    AGeoParaboloid() : rlo(0), rhi(40), dz(10) {}

    QString getShapeType() const override {return "TGeoParaboloid";}
    QString getShortName() const override {return QStringLiteral("Paraboloid");}
    QString getShapeTemplate() const override {return "TGeoParaboloid( rlo, rhi, dz )";}
    QString getHelp() const override;

    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse (const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName (const QRegularExpression & nameRegExp, const QString & newName) override;

    bool readFromString(QString GenerationString) override;
    TGeoShape* createGeoShape(const QString shapeName = "") override;

    double getHeight() const override {return dz;}
    QString getFullHeightString() override {return str2dz;}
    void setHeight(double /*dz*/) override {}
    QString getGenerationString(bool useStrings) const override;
    QString getScriptString(bool useStrings) const override;
    double maxSize() const override;

    void writeToJson(QJsonObject &json) const override;
    void readFromJson(const QJsonObject& json) override;

    bool readFromTShape(TGeoShape* Tshape) override;

    void scale(double factor) override;

    double rlo, rhi, dz;
    QString str2rlo, str2rhi, str2dz;
};

class AGeoCone : public AGeoShape
{
public:
    AGeoCone(double dz, double rminL, double rmaxL, double rminU, double rmaxU) :
        dz(dz), rminL(rminL), rmaxL(rmaxL), rminU(rminU), rmaxU(rmaxU) {}
    AGeoCone(double dz, double rmaxL, double rmaxU) :
        dz(dz), rminL(0), rmaxL(rmaxL), rminU(0), rmaxU(rmaxU) {}
    AGeoCone() :
        dz(10), rminL(0), rmaxL(20), rminU(0), rmaxU(0) {}

    QString getShapeType() const override {return "TGeoCone";}
    QString getShortName() const override {return QStringLiteral("Cone");}
    QString getShapeTemplate() const override {return "TGeoCone( dz, rminL, rmaxL, rminU, rmaxU )";}
    QString getHelp() const override;

    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    bool readFromString(QString GenerationString) override;
    TGeoShape* createGeoShape(const QString shapeName = "") override;

    double getHeight() const override {return dz;}
    QString getFullHeightString() override {return str2dz;}
    void setHeight(double dz) override {this->dz = dz;}
    QString getGenerationString(bool useStrings) const override;
    QString getScriptString(bool useStrings) const override;
    double maxSize() const override;

    void writeToJson(QJsonObject& json) const override;
    void readFromJson(const QJsonObject& json) override;

    bool readFromTShape(TGeoShape* Tshape) override;

    void scale(double factor) override;

    double dz;
    double rminL, rmaxL, rminU, rmaxU;
    QString str2dz, str2rminL, str2rmaxL, str2rminU, str2rmaxU;
};

class AGeoConeSeg : public AGeoCone
{
public:
    AGeoConeSeg(double dz, double rminL, double rmaxL, double rminU, double rmaxU, double phi1, double phi2) :
        AGeoCone(dz, rminL, rmaxL, rminU, rmaxU), phi1(phi1), phi2(phi2) {}
    AGeoConeSeg() : AGeoCone(), phi1(0), phi2(180.0) {}

    QString getShapeType() const override {return "TGeoConeSeg";}
    QString getShortName() const override {return QStringLiteral("ConeSeg");}
    QString getShapeTemplate() const override {return "TGeoConeSeg( dz, rminL, rmaxL, rminU, rmaxU, phi1, phi2 )";}
    QString getHelp() const override;

    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    bool readFromString(QString GenerationString) override;
    TGeoShape* createGeoShape(const QString shapeName = "") override;

    double getHeight() const override {return dz;}
    QString getFullHeightString() override {return str2dz;}
    void setHeight(double dz) override {this->dz = dz;}
    QString getGenerationString(bool useStrings) const override;
    QString getScriptString(bool useStrings) const override;
    double maxSize() const override;

    void writeToJson(QJsonObject& json) const override;
    void readFromJson(const QJsonObject& json) override;

    bool readFromTShape(TGeoShape *Tshape) override;

    double phi1, phi2;
    QString strPhi1, strPhi2;
};

class AGeoPolygon : public AGeoShape
{
public:
    AGeoPolygon(int nedges, double dphi, double dz, double rminL, double rmaxL, double rminU, double rmaxU) :
        nedges(nedges), dphi(dphi), dz(dz), rminL(rminL), rmaxL(rmaxL), rminU(rminU), rmaxU(rmaxU) {}
    AGeoPolygon(int nedges, double dz, double rmaxL, double rmaxU) :
        nedges(nedges), dphi(360.0), dz(dz), rminL(0), rmaxL(rmaxL), rminU(0), rmaxU(rmaxU) {}
    AGeoPolygon() :
        nedges(6), dphi(360.0), dz(10), rminL(0), rmaxL(20), rminU(0), rmaxU(20) {}

    QString getShapeType() const override {return "TGeoPolygon";}
    QString getShortName() const override {return QStringLiteral("Polygon");}
    QString getShapeTemplate() const override {return "TGeoPolygon( nedges, dphi, dz, rminL, rmaxL, rminU, rmaxU )";}
    QString getHelp() const override;

    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    bool readFromString(QString GenerationString) override;
    TGeoShape* createGeoShape(const QString shapeName = "") override;

    double getHeight() const override {return dz;}
    QString getFullHeightString() override {return str2dz;}
    void setHeight(double dz) override {this->dz = dz;}
    QString getGenerationString(bool useStrings) const override;
    QString getScriptString(bool useStrings) const override;
    double maxSize() const override;
    double minSize() const override;

    void writeToJson(QJsonObject& json) const override;
    void readFromJson(const QJsonObject& json) override;

    bool readFromTShape(TGeoShape* /*Tshape*/) override { return false; } //it is not a base root class, so not valid for import from GDML

    void scale(double factor) override;

    int nedges;
    double dphi, dz;
    double rminL, rmaxL, rminU, rmaxU;
    QString strNedges, strdPhi, str2dz, str2rminL, str2rmaxL, str2rminU, str2rmaxU;
};

struct APolyCGsection
{
    double z;
    double rmin, rmax;
    QString strZ, str2rmin, str2rmax;

    APolyCGsection() : z(0), rmin(0), rmax(100) {}
    APolyCGsection(double z, double rmin, double rmax) : z(z), rmin(rmin), rmax(rmax) {}

    bool updateShape(QString & errorStr);
    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName);

    bool fromString(QString string);
    QString toString(bool useStrings) const;
    QString toScriptString(bool useStrings) const;
    void writeToJson(QJsonObject& json) const;
    void readFromJson(const QJsonObject& json);
    bool operator ==( const APolyCGsection &section1) const;

    void scale(double factor);
};

class AGeoPcon : public AGeoShape
{
public:
    AGeoPcon();

    QString getShapeType() const override {return "TGeoPcon";}
    QString getShortName() const override {return QStringLiteral("Pcon");}
    QString getShapeTemplate() const override {return "TGeoPcon( phi, dphi, { z0 : rmin0 : rmaz0 }, { z1 : rmin1 : rmax1 } )";}
    QString getHelp() const override;

    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    bool readFromString(QString GenerationString) override;
    TGeoShape* createGeoShape(const QString shapeName = "") override;

    double getHeight() const override;
    QString getFullHeightString() override; // !!!***
    double getRelativePosZofCenter() const override;
    QString getGenerationString(bool useStrings) const override;
    QString getScriptString(bool useStrings) const override;
    double maxSize() const override;
    double minSize() const override;

    void writeToJson(QJsonObject& json) const override;
    void readFromJson(const QJsonObject& json) override;

    bool readFromTShape(TGeoShape* Tshape) override;

    void scale(double factor) override;

    double phi, dphi;
    QString strPhi, strdPhi;
    std::vector<APolyCGsection> Sections;
};

class AGeoPgon : public AGeoPcon
{
public:
    AGeoPgon() : AGeoPcon(), nedges(6) {}

    QString getShapeType() const override {return "TGeoPgon";}
    QString getShortName() const override {return QStringLiteral("Pgon");}
    QString getShapeTemplate() const override {return "TGeoPgon( phi, dphi, nedges, { z0 : rmin0 : rmaz0 }, { zN : rminN : rmaxN } )";}
    QString getHelp() const override;

    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    bool readFromString(QString GenerationString) override;
    TGeoShape* createGeoShape(const QString shapeName = "") override;

    QString getGenerationString(bool useStrings) const override;
    QString getScriptString(bool useStrings) const override;
    double maxSize() const override;

    void writeToJson(QJsonObject& json) const override;
    void readFromJson(const QJsonObject& json) override;

    bool readFromTShape(TGeoShape* Tshape) override;

    int nedges;
    QString strNedges;
};

class AGeoTrd1 : public AGeoShape
{
public:
    AGeoTrd1(double dx1, double dx2, double dy, double dz) :
        dx1(dx1), dx2(dx2), dy(dy), dz(dz) {}
    AGeoTrd1() : dx1(15), dx2(5), dy(10), dz(10) {}

    QString getShapeType() const override {return "TGeoTrd1";}
    QString getShortName() const override {return QStringLiteral("Trd1_");}
    QString getShapeTemplate() const override {return "TGeoTrd1( dx1, dx2, dy, dz )";}
    QString getHelp() const override;

    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    bool readFromString(QString GenerationString) override;
    TGeoShape* createGeoShape(const QString shapeName = "") override;

    double getHeight() const override {return dz;}
    QString getFullHeightString() override {return str2dz;}
    void setHeight(double dz) override {this->dz = dz;}
    QString getGenerationString(bool useStrings) const override;
    QString getScriptString(bool useStrings) const override;
    double minSize() const override;
    double maxSize() const override;

    void writeToJson(QJsonObject& json) const override;
    void readFromJson(const QJsonObject& json) override;

    bool readFromTShape(TGeoShape* Tshape) override;

    void scale(double factor) override;

    double dx1, dx2, dy, dz;
    QString str2dx1, str2dx2, str2dy, str2dz;
};

class AGeoTrd2 : public AGeoShape
{
public:
    AGeoTrd2(double dx1, double dx2, double dy1, double dy2, double dz) :
        dx1(dx1), dx2(dx2), dy1(dy1), dy2(dy2), dz(dz) {}
    AGeoTrd2() :
        dx1(15), dx2(5), dy1(10), dy2(20), dz(10) {}

    QString getShapeType() const override {return "TGeoTrd2";}
    QString getShortName() const override {return QStringLiteral("Trd2_");}
    QString getShapeTemplate() const override {return "TGeoTrd2( dx1, dx2, dy1, dy2, dz )";}
    QString getHelp() const override;

    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    bool readFromString(QString GenerationString) override;
    TGeoShape* createGeoShape(const QString shapeName = "") override;

    double getHeight() const override {return dz;}
    QString getFullHeightString() override {return str2dz;}
    void setHeight(double dz) override {this->dz = dz;}
    QString getGenerationString(bool useStrings) const override;
    QString getScriptString(bool useStrings) const override;
    double maxSize() const override;

    void writeToJson(QJsonObject& json) const override;
    void readFromJson(const QJsonObject& json) override;

    bool readFromTShape(TGeoShape* Tshape) override;

    void scale(double factor) override;

    double dx1, dx2, dy1, dy2, dz;
    QString str2dx1, str2dx2, str2dy1, str2dy2, str2dz;
};

class AGeoTubeSeg : public AGeoShape
{
public:
    AGeoTubeSeg(double rmin, double rmax, double dz, double phi1, double phi2) :
        rmin(rmin), rmax(rmax), dz(dz), phi1(phi1), phi2(phi2) {}
    AGeoTubeSeg() : rmin(0), rmax(10), dz(5), phi1(0), phi2(180) {}

    QString getShapeType() const override {return "TGeoTubeSeg";}
    QString getShortName() const override {return QStringLiteral("TubeSeg");}
    QString getShapeTemplate() const override {return "TGeoTubeSeg( rmin, rmax, dz, phi1, phi2 )";}
    QString getHelp() const override;

    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    bool readFromString(QString GenerationString) override;
    TGeoShape* createGeoShape(const QString shapeName = "") override;

    double getHeight() const override {return dz;}
    QString getFullHeightString() override {return str2dz;}
    void setHeight(double dz) override {this->dz = dz;}
    QString getGenerationString(bool useStrings) const override;
    QString getScriptString(bool useStrings) const override;
    double maxSize() const override;

    void writeToJson(QJsonObject& json) const override;
    void readFromJson(const QJsonObject& json) override;

    bool readFromTShape(TGeoShape* Tshape) override;

    void scale(double factor) override;

    double      rmin,     rmax,     dz,    phi1,    phi2;
    QString str2rmin, str2rmax, str2dz, strPhi1, strPhi2;
};

class AGeoCtub : public AGeoTubeSeg
{

public:
    AGeoCtub(double rmin, double rmax, double dz, double phi1, double phi2,
             double nxlow, double nylow, double nzlow,
             double nxhi, double nyhi, double nzhi) :
        AGeoTubeSeg(rmin,  rmax,  dz,  phi1,  phi2),
        nxlow(nxlow), nylow(nylow), nzlow(nzlow),
        nxhi(nxhi), nyhi(nyhi), nzhi(nzhi) {}
    AGeoCtub() :
        AGeoTubeSeg(0,  10,  5,  0,  180),
        nxlow(0), nylow(0.64), nzlow(-0.77),
        nxhi(0), nyhi(0.09), nzhi(0.87) {}

    QString getShapeType() const override {return "TGeoCtub";}
    QString getShortName() const override {return QStringLiteral("CTube");}
    QString getShapeTemplate() const override {return "TGeoCtub( rmin, rmax, dz, phi1, phi2, nxlow, nylow, nzlow, nxhi, nyhi, nzhi )";}
    QString getHelp() const override;

    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    bool readFromString(QString GenerationString) override;
    TGeoShape* createGeoShape(const QString shapeName = "") override;

    double getHeight() const override {return dz;}          // wrong!
    QString getFullHeightString() override {return str2dz;} // wrong!
    void setHeight(double dz) override {this->dz = dz;}
    QString getGenerationString(bool useStrings) const override;
    QString getScriptString(bool useStrings) const override;
    double maxSize() const override;

    void writeToJson(QJsonObject& json) const override;
    void readFromJson(const QJsonObject& json) override;

    bool readFromTShape(TGeoShape* Tshape) override;

    double nxlow, nylow, nzlow;
    double nxhi, nyhi, nzhi;
    QString strnxlow, strnylow, strnzlow, strnxhi, strnyhi, strnzhi;
};

class AGeoEltu : public AGeoShape
{
public:
    AGeoEltu(double a, double b, double dz) : a(a), b(b), dz(dz) {}
    AGeoEltu() : a(10), b(20), dz(5) {}

    QString getShapeType() const override {return "TGeoEltu";}
    QString getShortName() const override {return QStringLiteral("ElTube");}
    QString getShapeTemplate() const override {return "TGeoEltu( a, b, dz )";}
    QString getHelp() const override;

    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    bool readFromString(QString GenerationString) override;
    TGeoShape* createGeoShape(const QString shapeName = "") override;

    double getHeight() const override {return dz;}
    QString getFullHeightString() override {return str2dz;}
    void setHeight(double dz) override {this->dz = dz;}
    QString getGenerationString(bool useStrings) const override;
    QString getScriptString(bool useStrings) const override;
    double maxSize() const override;

    void writeToJson(QJsonObject& json) const override;
    void readFromJson(const QJsonObject& json) override;

    bool readFromTShape(TGeoShape* Tshape) override;

    void scale(double factor) override;

    double a, b, dz;
    QString str2a, str2b, str2dz;
};

class AGeoSphere : public AGeoShape
{
public:
    AGeoSphere(double rmax, double rmin, double theta1, double theta2, double phi1, double phi2) :
        rmin(rmin), rmax(rmax), theta1(theta1), theta2(theta2), phi1(phi1), phi2(phi2) {}
    AGeoSphere(double r) : rmin(0), rmax(r), theta1(0), theta2(180.0), phi1(0), phi2(360.0) {}
    AGeoSphere(double rmax, double rmin) : rmin(rmin), rmax(rmax), theta1(0), theta2(180.0), phi1(0), phi2(360.0) {}
    AGeoSphere() : rmin(0), rmax(10.0), theta1(0), theta2(180.0), phi1(0), phi2(360.0) {}

    QString getShapeType() const override {return "TGeoSphere";}
    QString getShortName() const override {return QStringLiteral("Sphere");}
    QString getShapeTemplate() const override {return "TGeoSphere( rmin,  rmax, theta1, theta2, phi1, phi2 )";}
    QString getHelp() const override;

    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse (const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName (const QRegularExpression & nameRegExp, const QString & newName) override;

    bool readFromString(QString GenerationString) override;
    TGeoShape* createGeoShape(const QString shapeName = "") override;

    double getHeight() const override;      // !!!*** wrong! consider rmin too
    QString getFullHeightString() override; // !!!*** to do
    double getRelativePosZofCenter() const override;
    void setHeight(double dz) override {rmax = dz;}
    QString getGenerationString(bool useStrings) const override;
    QString getScriptString(bool useStrings) const override;
    double maxSize() const override { return rmax;}

    void writeToJson(QJsonObject& json) const override;
    void readFromJson(const QJsonObject& json) override;

    bool readFromTShape(TGeoShape* Tshape) override;

    void scale(double factor) override;

    double      rmin,     rmax,    theta1,    theta2,    phi1,    phi2;
    QString str2rmin, str2rmax, strTheta1, strTheta2, strPhi1, strPhi2;

private:
    void computeZupZdown(double & Zup, double & Zdown) const;
};

class AGeoPara : public AGeoShape
{
public:
    AGeoPara(double dx, double dy, double dz, double alpha, double theta, double phi) :
        dx(dx), dy(dy), dz(dz), alpha(alpha), theta(theta), phi(phi) {}
    AGeoPara() : dx(10), dy(10), dz(10), alpha(10), theta(25), phi(45) {}

    QString getShapeType() const override {return "TGeoPara";}
    QString getShortName() const override {return QStringLiteral("Para");}
    QString getShapeTemplate() const override {return "TGeoPara( dX, dY, dZ, alpha, theta, phi )";}
    QString getHelp() const override;

    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    bool readFromString(QString GenerationString) override;
    TGeoShape* createGeoShape(const QString shapeName = "") override;

    double getHeight() const override {return dz;}
    QString getFullHeightString() override {return str2dz;}
    void setHeight(double dz) override {this->dz = dz;}
    QString getGenerationString(bool useStrings) const override;
    QString getScriptString(bool useStrings) const override;
    double maxSize() const override;

    void writeToJson(QJsonObject& json) const override;
    void readFromJson(const QJsonObject& json) override;

    bool readFromTShape(TGeoShape* Tshape) override;

    void scale(double factor) override;

    double dx, dy, dz;
    double alpha, theta, phi;
    QString str2dx, str2dy, str2dz, strAlpha, strTheta, strPhi;
};

class AGeoArb8 : public AGeoShape
{
public:
    AGeoArb8(double dz, std::array<std::pair<double, double>,8> NodesList);
    AGeoArb8();

    QString getShapeType() const override {return "TGeoArb8";}
    QString getShortName() const override {return QStringLiteral("Arb8_");}
    QString getShapeTemplate() const override {return "TGeoArb8( dz,  xL1,yL1, xL2,yL2, xL3,yL3, xL4,yL4, xU1,yU1, xU2,yU2, xU3,yU3, xU4,yU4  )";}
    QString getHelp() const override;

    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse(const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName) override;

    bool readFromString(QString GenerationString) override;
    TGeoShape* createGeoShape(const QString shapeName = "") override;

    double getHeight() const override {return dz;}
    QString getFullHeightString() override {return str2dz;}
    void setHeight(double dz) override {this->dz = dz;}
    QString getGenerationString(bool useStrings) const override;
    QString getScriptString(bool useStrings) const override;
    double maxSize() const override;

    void writeToJson(QJsonObject& json) const override;
    void readFromJson(const QJsonObject& json) override;

    bool readFromTShape(TGeoShape* Tshape) override;

    static bool checkPointsForArb8(std::array<std::pair<double, double>, 8> nodes );

    void scale(double factor) override;

    double dz;
    QString str2dz;
    std::array<std::pair<double, double>,8> Vertices;
    std::array<std::pair<QString, QString>,8> strVertices;

private:
    void init();
};

class AGeoComposite : public AGeoShape
{
public:
    AGeoComposite(const QStringList members, QString GenerationString);
    AGeoComposite() {}

    QString getShapeType() const override {return "TGeoCompositeShape";}
    QString getShortName() const override {return QStringLiteral("Composite");}
    QString getShapeTemplate() const override {return "TGeoCompositeShape( (A + B) * (C - D) )";}
    QString getHelp() const override;

    void scale(double /*factor*/) override {};

    bool readFromString(QString GenerationString) override;
    TGeoShape* createGeoShape(const QString shapeName = "") override;

    bool isGeoConstInUse(const QRegularExpression & /*nameRegExp*/) const override {return false;}

    QString getGenerationString(bool) const override {return GenerationString;}
    QString getScriptString(bool) const override;
    double maxSize() const override {return 0;} // have to ask AGeoObject

    void writeToJson(QJsonObject& json) const override;
    void readFromJson(const QJsonObject& json) override;

    bool readFromTShape(TGeoShape* /*Tshape*/) override {return false;} //cannot be retrieved this way! need cooperation with AGeoObject itself

    QStringList members;
    QString GenerationString;
};

class AGeoTorus  : public AGeoShape
{
public:
    AGeoTorus(double R, double Rmin, double Rmax, double Phi1, double Dphi) :
        R(R), Rmin(Rmin), Rmax(Rmax), Phi1(Phi1), Dphi(Dphi) {}
    AGeoTorus() {}

    QString getShapeType() const override {return "TGeoTorus";}
    QString getShortName() const override {return QStringLiteral("Torus");}
    QString getShapeTemplate() const override {return "TGeoTorus( R, Rmin, Rmax, Phi1, Dphi )";}
    QString getHelp() const override;

    void introduceGeoConstValues(QString & errorStr) override;

    bool isGeoConstInUse (const QRegularExpression & nameRegExp) const override;
    void replaceGeoConstName (const QRegularExpression & nameRegExp, const QString & newName) override;

    bool readFromString(QString GenerationString) override;
    TGeoShape* createGeoShape(const QString shapeName = "") override;

    double getHeight() const override {return Rmax;}
    QString getFullHeightString() override {return str2Rmax;}
    void setHeight(double dz) override {this->Rmax = dz;}
    QString getGenerationString(bool useStrings) const override;
    QString getScriptString(bool useStrings) const override;
    double maxSize() const override;

    void writeToJson(QJsonObject& json) const override;
    void readFromJson(const QJsonObject& json) override;

    bool readFromTShape(TGeoShape* Tshape) override;

    void scale(double factor) override
    {
        R    *= factor;
        Rmin *= factor;
        Rmax *= factor;
    }

    double R = 100.0;
    double Rmin = 0, Rmax = 20.0;
    double Phi1 = 0, Dphi = 360.0;

    QString str2R, str2Rmin, str2Rmax, strPhi1, strDphi;
};

// --- Dummy shape for stack object: needed to implement stacks of stacks
class AStackDummyShape : public AGeoBox
{
public:
    //AStackDummyShape(){}
    double getRelativePosZofCenter() const override {return RelativePosZofCenter;}

    double RelativePosZofCenter = 0;
};

#endif // AGEOSHAPE_H
