#ifndef AINTERFACEWIDGETFACTORY_H
#define AINTERFACEWIDGETFACTORY_H

#include <QString>
#include <QFrame>

class QWidget;
class AInterfaceRule;
class ABasicInterfaceRule;
class AMetalInterfaceRule;
class AWaveshifterInterfaceRule;
class ASpectralBasicInterfaceRule;
class FsnpInterfaceRule;
class ASurfaceInterfaceRule;
class AUnifiedRule;
class QPushButton;
class TObject;
class AInterfaceRuleWidget;

// --- Widget factory ---

class AInterfaceWidgetFactory
{
public:
    static AInterfaceRuleWidget * createEditWidget(AInterfaceRule * rule, QWidget * parent);
};

// --- Base class for the widget ----

class AInterfaceRuleWidget : public QFrame
{
    Q_OBJECT
public:
    AInterfaceRuleWidget(QWidget * parent) : QFrame(), Parent(parent) {}

protected:
    QWidget * Parent = nullptr;

signals:
    void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);
    void requestDrawLegend(double x1, double y1, double x2, double y2, QString title);
};

// ----- Sub-classes ------

class ABasicInterfaceWidget : public AInterfaceRuleWidget
{
    Q_OBJECT
public:
    ABasicInterfaceWidget(ABasicInterfaceRule * rule, QWidget * parent);
};

class AMetalInterfaceWidget : public AInterfaceRuleWidget
{
    Q_OBJECT
public:
    AMetalInterfaceWidget(AMetalInterfaceRule * rule, QWidget * parent);
};

class AFsnpInterfaceWidget : public AInterfaceRuleWidget
{
    Q_OBJECT
public:
    AFsnpInterfaceWidget(FsnpInterfaceRule * rule, QWidget * parent);
};

class ASurfaceInterfaceWidget : public AInterfaceRuleWidget
{
    Q_OBJECT
public:
    ASurfaceInterfaceWidget(ASurfaceInterfaceRule * rule, QWidget * parent);
};

class AUnifiedInterfaceWidget : public AInterfaceRuleWidget
{
    Q_OBJECT
public:
    AUnifiedInterfaceWidget(AUnifiedRule * rule, QWidget * parent);
};

class AWaveshifterInterfaceWidget : public AInterfaceRuleWidget
{
    Q_OBJECT
public:
    AWaveshifterInterfaceWidget(AWaveshifterInterfaceRule * rule, QWidget * parent);

protected:
    AWaveshifterInterfaceRule * Rule;

    QPushButton *pbShowRP, *pbShowRPbinned, *pbShowES, *pbShowESbinned;

private slots:
    void loadReemissionProbability(); // !!!***
    void loadEmissionSpectrum(); // !!!***
    void showReemissionProbability(); // !!!***
    void showEmissionSpectrum();  // !!!***
    void showBinnedReemissionProbability(); // !!!***
    void showBinnedEmissionSpectrum();  // !!!***
    void updateButtons();
};

class ASpectralBasicInterfaceWidget : public AInterfaceRuleWidget
{
    Q_OBJECT
public:
    ASpectralBasicInterfaceWidget(ASpectralBasicInterfaceRule * rule, QWidget * caller);

protected:
    ASpectralBasicInterfaceRule * Rule;

    QPushButton *pbShow, *pbShowBinned;

private slots:
    void loadSpectralData();
    void showLoaded(); // !!!***
    void showBinned(); // !!!***
    void updateButtons();
};

#endif // AINTERFACEWIDGETFACTORY_H
