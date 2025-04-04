#ifndef ADRAWEXPLORERWIDGET_H
#define ADRAWEXPLORERWIDGET_H

#include "adrawobject.h"

#include <QTreeWidget>

#include <vector>

class AGraphWindow;
class QTreeWidgetItem;
class TObject;
class TH2;
class TAxis;
class TGaxis;
class QColor;
class QIcon;
class TAttLine;
class TAttMarker;
class TAttFill;
class AGraphRasterWindow;

class ADrawExplorerWidget : public QTreeWidget
{
    Q_OBJECT
public:
    ADrawExplorerWidget(AGraphWindow & GraphWindow, std::vector<ADrawObject> & DrawObjects);

    void updateGui();

    TH2 * getObjectForCustomProjection() {return objForCustomProjection;}

    void activateCustomGuiForItem(int index);    
    void showObjectContextMenu(const QPoint &pos, int index);
    void manipulateTriggered();

    void customProjection(ADrawObject &obj);

private slots:
    void onContextMenuRequested(const QPoint & pos);
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);

protected:
    void dropEvent(QDropEvent * event) override;

private:
    AGraphWindow         & GraphWindow;
    AGraphRasterWindow   & Raster;

    std::vector<ADrawObject> & DrawObjects;

private:
    void addToDrawObjectsAndRegister(TObject * pointer, const QString & options);

    void rename(ADrawObject &obj);
    void toggleEnable(ADrawObject &obj);
    void remove(int index);
    void setAttributes(int index);
    void showPanel(ADrawObject &obj);
    void fitPanel(ADrawObject &obj);
    void scale(ADrawObject &obj);
    void scale(std::vector<ADrawObject> & drawObjects);
    void scaleIntegralToUnity(ADrawObject &obj);
    void scaleToUnity(ADrawObject &obj);
    void scaleCDR(ADrawObject &obj);
    void scaleAllSameMax();
    void scaleAllIntegralsToUnity();
    void shift(ADrawObject &obj);
    void shift(std::vector<ADrawObject> & drawObjects);
    void drawIntegral(ADrawObject &obj);
    void fraction(ADrawObject &obj);
    void fwhm(int index);
    void linDraw(int index);
    void boxDraw(int index);
    void ellipseDraw(int index);
    void onMoveUpAction(int index);
    void onMoveDownAction(int index);

    void linFit(int index);
    void expFit(int index);
    void gauss2Fit(int index);
    void interpolate(ADrawObject &obj);
    void median(ADrawObject &obj);
    void projection(ADrawObject &obj, int axis);
    //void splineFit(int index);
    void editAxis(ADrawObject &obj, int axisIndex);
    void setCustomMargins(ADrawObject & obj);
    void addAxis(int axisIndex);
    void saveRoot(ADrawObject &obj);
    void saveAsTxt(ADrawObject &obj, bool fUseBinCenters);
    void extract(ADrawObject &obj);
    void editPave(ADrawObject &obj);
    void editTGaxis(ADrawObject &obj);

    bool canScale(ADrawObject &obj);
    void doScale(ADrawObject &obj, double sf);
    bool getDrawMax(ADrawObject &obj, double &max);
    void copyAxisProperties(TGaxis & grAxis, TAxis  & axis);
    void copyAxisProperties(TAxis  & axis,   TGaxis & grAxis);
    QString generateOptionForSecondaryAxis(int axisIndex, double u1, double u2);

    void constructIconForObject(QIcon & icon, const ADrawObject & drObj);
    void construct1DIcon(QIcon & icon, const TAttLine *line, const TAttMarker *marker, const TAttFill *fill);
    void construct2DIcon(QIcon & icon);
    void convertRootColoToQtColor(int rootColor, QColor & qtColor);

    void updateGui_multidrawMode(); // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

private:
    TH2 * objForCustomProjection = nullptr;

    const int IconWidth  = 61;
    const int IconHeight = 31;

signals:
    void requestShowLegendDialog();
};

#endif // ADRAWEXPLORERWIDGET_H
