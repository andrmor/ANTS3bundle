#ifndef AGRAPH_SI
#define AGRAPH_SI

#include "ascriptinterface.h"

#include <QString>
#include <QVariantList>

class TObject;
class ARootObjCollection;

class AGraph_SI : public AScriptInterface
{
    Q_OBJECT

public:
    AGraph_SI();
    //AGraph_SI(const AGraph_SI& other);
    //~AGraph_SI(){}

    //bool InitOnRun() override {}
    //bool IsMultithreadCapable() const override {return true;}

    AScriptInterface * cloneBase() const {return new AGraph_SI();}

public slots:
    void new1D(QString graphName);
    void new1DErr(QString graphName);
    void new2D(QString graphName);

    void addPoint(QString graphName, double x, double y);
    void addPoint(QString graphName, double x, double y, double z);
    void addPoint(QString graphName, double x, double y, double errorX, double errorY);

    void addPoints(QString graphName, QVariantList array);
    void addPoints(QString graphName, QVariantList array, int indexX, int indexY);
    void addPoints(QString graphName, QVariantList xArray, QVariantList yArray);
    void addPoints(QString graphName, QVariantList xArray, QVariantList yArray, QVariantList xErrArray, QVariantList yErrArray);
    //void addPoints(QString graphName, QVariantList xArray, QVariantList yArray, QVariantList zArray);

    void draw(QString graphName, QString options = "APL");

    void setMarkerProperties(QString graphName, int color, int style, double size);
    void setLineProperties(QString graphName, int color, int style, int width);

    void setTitle(QString graphName, QString graphTitle);
    void setAxisTitles(QString graphName, QString x_Title, QString y_Title);

    void setYRange(QString graphName, double min, double max);
    void setMinimum(QString graphName, double min);
    void setMaximum(QString graphName, double max);
    void setXRange(QString graphName, double min, double max);
    void setXDivisions(QString graphName, int numDiv);
    void setYDivisions(QString graphName, int numDiv);
    void setXCustomLabelsByIndex(QString graphName, QVariantList textLabels, double tiltAngleDegrees, bool rightAligned);
    void setXCustomLabelsByValue(QString graphName, QVariantList xValues, QVariantList textLabels, double tiltAngleDegrees, bool rightAligned);

    void sort(QString graphName);

    QVariantList getData(QString GraphName);

    void load(QString graphName, QString fileName, QString graphNameInFile = "");
    void save(QString graphName, QString fileName);

    bool remove(QString graphName);
    void removeAll();

    void configureAbortIfAlreadyExists(bool flag) {AbortIfExists = flag;}

signals:
    void requestDraw(TObject * obj, QString options, bool fFocus);

private:
    ARootObjCollection & Graphs;

    bool AbortIfExists = false;

};

#endif // AGRAPH_SI
