#ifndef AINTERFACEWIDGETFACTORY_H
#define AINTERFACEWIDGETFACTORY_H

#include <QFrame>

class QWidget;
class AInterfaceRule;
class GraphWindowClass;
class ABasicInterfaceRule;
class AMetalInterfaceRule;
class AWaveshifterInterfaceRule;
class ASpectralBasicInterfaceRule;
class FsnpInterfaceRule;
class ASurfaceInterfaceRule;
class QPushButton;

class AInterfaceWidgetFactory
{
public:
    static QWidget * createEditWidget(AInterfaceRule * Rule, QWidget * Caller, GraphWindowClass * GraphWindow);
};

class ABasicInterfaceWidget : public QFrame
{
    Q_OBJECT
public:
    ABasicInterfaceWidget(ABasicInterfaceRule * rule);
};

class AMetalInterfaceWidget : public QFrame
{
    Q_OBJECT
public:
    AMetalInterfaceWidget(AMetalInterfaceRule * rule);
};

class AFsnpInterfaceWidget : public QFrame
{
    Q_OBJECT
public:
    AFsnpInterfaceWidget(FsnpInterfaceRule * rule);
};

class ASurfaceInterfaceWidget : public QFrame
{
    Q_OBJECT
public:
    ASurfaceInterfaceWidget(ASurfaceInterfaceRule * rule);
};

class AWaveshifterInterfaceWidget : public QFrame
{
    Q_OBJECT
public:
    AWaveshifterInterfaceWidget(AWaveshifterInterfaceRule * rule, QWidget * caller, GraphWindowClass * graphWindow);

protected:
    AWaveshifterInterfaceRule * Rule;
    QWidget * Caller;
    GraphWindowClass * GraphWindow;

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

class ASpectralBasicInterfaceWidget : public QFrame
{
    Q_OBJECT
public:
    ASpectralBasicInterfaceWidget(ASpectralBasicInterfaceRule * rule, QWidget * caller, GraphWindowClass * graphWindow);

protected:
    ASpectralBasicInterfaceRule * Rule;
    QWidget * Caller;
    GraphWindowClass * GraphWindow;

    QPushButton *pbShow, *pbShowBinned;

private slots:
    void loadSpectralData();  // !!!***
    void showLoaded(); // !!!***
    void showBinned(); // !!!***
    void updateButtons();
};

#endif // AINTERFACEWIDGETFACTORY_H
