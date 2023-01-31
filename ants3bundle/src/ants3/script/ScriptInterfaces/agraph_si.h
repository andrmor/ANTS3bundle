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


    void addPoint(QString GraphName, double x, double y);
    void addPoint(QString GraphName, double x, double y, double errorY);
    void addPoint(QString GraphName, double x, double y, double errorX, double errorY);

    void addPoint2D(QString GraphName, double x, double y, double z); // refactor addPoint(QString GraphName, double x, double y, double errorY_or_z);

    void addPoints(QString GraphName, QVariantList xArray, QVariantList yArray);
    void addPoints(QString GraphName, QVariantList xArray, QVariantList yArray, QVariantList yErrArray);
    void addPoints(QString GraphName, QVariantList xArray, QVariantList yArray, QVariantList xErrArray, QVariantList yErrArray);
    void addPoints(QString GraphName, QVariantList xyArray);

    void draw(QString GraphName, QString options = "APL");

    void setMarkerProperties(QString GraphName, int MarkerColor, int MarkerStyle, double MarkerSize);
    void setLineProperties(QString GraphName, int LineColor, int LineStyle, int LineWidth);
    void setTitles(QString GraphName, QString X_Title, QString Y_Title, QString GraphTitle = "");

    void setYRange(QString GraphName, double min, double max);
    void setMinimum(QString GraphName, double min);
    void setMaximum(QString GraphName, double max);
    void setXRange(QString GraphName, double min, double max);
    void setXDivisions(QString GraphName, int numDiv);
    void setYDivisions(QString GraphName, int numDiv);

    void sort(QString GraphName);

    QVariantList getPoints(QString GraphName);  // !!!*** make more general: for TGraphErrors and TGraph2D

    void loadTGraph(QString NewGraphName, QString FileName); // !!!*** add for TGraphErrors and TGraph2D
    void saveRoot(QString GraphName, QString FileName);

    //void ImportFromBasket(const QString& NewGraphName, const QString& BasketName, int index = 0);

    bool remove(QString GraphName);
    void removeAllGraph();

    void configureAbortIfAlreadyExists(bool flag) {AbortIfExists = flag;}

signals:
    void requestDraw(TObject * obj, QString options, bool fFocus);

private:
    ARootObjCollection & Graphs;

    bool AbortIfExists = false;

};

#endif // AGRAPH_SI
