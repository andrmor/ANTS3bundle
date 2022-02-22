#ifndef AGRAPH_SI
#define AGRAPH_SI

#include "ascriptinterface.h"

#include <QString>
#include <QVariant>
#include <QVariantList>

class AScriptObjStore;
class TObject;

class AGraph_SI : public AScriptInterface
{
    Q_OBJECT

public:
    AGraph_SI();
    //AGraph_SI(const AGraph_SI& other);
    //~AGraph_SI(){}

    //bool InitOnRun() override {}
    //bool IsMultithreadCapable() const override {return true;}

public slots:
    void create1D(QString GraphName);
    void create1DErr(QString GraphName);
    void create2D(QString GraphName);

    void configureAbortIfAlreadyExists(bool flag) {bAbortIfExists = flag;}

    void SetMarkerProperties(QString GraphName, int MarkerColor, int MarkerStyle, double MarkerSize);
    void SetLineProperties(QString GraphName, int LineColor, int LineStyle, int LineWidth);
    void SetTitles(QString GraphName, QString X_Title, QString Y_Title, QString GraphTitle = "");

    void AddPoint(QString GraphName, double x, double y);
    void AddPoint(QString GraphName, double x, double y, double errorY);
    void AddPoint(QString GraphName, double x, double y, double errorX, double errorY);
    void AddPoints(QString GraphName, QVariantList xArray, QVariantList yArray);
    void AddPoints(QString GraphName, QVariantList xArray, QVariantList yArray, QVariantList yErrArray);
    void AddPoints(QString GraphName, QVariantList xArray, QVariantList yArray, QVariantList xErrArray, QVariantList yErrArray);
    void AddPoints(QString GraphName, QVariantList xyArray);

    void AddPoint2D(QString GraphName, double x, double y, double z);

    void SetYRange(const QString& GraphName, double min, double max);
    void SetMinimum(const QString& GraphName, double min);
    void SetMaximum(const QString& GraphName, double max);
    void SetXRange(const QString& GraphName, double min, double max);
    void SetXDivisions(const QString& GraphName, int numDiv);
    void SetYDivisions(const QString& GraphName, int numDiv);

    void Sort(const QString& GraphName);

    void Draw(QString GraphName, QString options = "APL");

    void LoadTGraph(const QString& NewGraphName, const QString& FileName);
    void Save(const QString& GraphName, const QString& FileName);
    const QVariant GetPoints(const QString& GraphName);

    //void ImportFromBasket(const QString& NewGraphName, const QString& BasketName, int index = 0);

    bool Delete(QString GraphName);
    void DeleteAllGraph();

signals:
    void RequestDraw(TObject* obj, QString options, bool fFocus);

private:
    AScriptObjStore & TmpHub;

    bool           bAbortIfExists = false;

};

#endif // AGRAPH_SI
