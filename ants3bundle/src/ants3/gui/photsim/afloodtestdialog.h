#ifndef AFLOODTESTDIALOG_H
#define AFLOODTESTDIALOG_H

#include <QDialog>

class APhotonBombsSettings;
class AFloodSettings;
class QSpinBox;
class QCheckBox;
class QRadioButton;
class QComboBox;
class QLineEdit;
class QLabel;
class QPushButton;
class TObject;

// Generates flood positions with the same code as lsim and shows their distributions in the graph window
class AFloodTestDialog : public QDialog
{
    Q_OBJECT

public:
    AFloodTestDialog(const APhotonBombsSettings & bombSettings, QWidget * parent = nullptr);

signals:
    void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);

private:
    const APhotonBombsSettings & BombSet;

    QSpinBox     * sbPoints   = nullptr;
    QCheckBox    * cbLimits   = nullptr;
    QRadioButton * rbGlobal   = nullptr;
    QRadioButton * rbLocal    = nullptr;
    QRadioButton * rb1D       = nullptr;
    QRadioButton * rb2D       = nullptr;
    QRadioButton * rb3D       = nullptr;
    QComboBox    * cobQuantity = nullptr;
    QSpinBox     * sbBins     = nullptr;
    QRadioButton * rbAutoRange = nullptr;
    QLineEdit    * leFrom[3];
    QLineEdit    * leTo[3];
    QCheckBox    * cbOverlay  = nullptr;
    QPushButton  * pbGenerate = nullptr;
    QPushButton  * pbAbort    = nullptr;
    QLabel       * labStatus  = nullptr;

    bool bAbortRequested = false;

    enum EQuantity {QX, QY, QZ, QR, QAxial, QXY, QXZ, QYZ, QAxialR, QXYZ};
    std::vector<EQuantity> ShownQuantities;

    void updateQuantities();
    void updateControls();
    void generate();

    int    dimension() const;
    void   autoRange(const AFloodSettings & fs, EQuantity q, bool local, double * from, double * to) const;
    static void quantityValues(const AFloodSettings & fs, EQuantity q, const double * global, bool local, double * out);
    static QString quantityTitle(EQuantity q, int index);
    static int  profileCoordinate(const AFloodSettings & fs, EQuantity q);   // -1 if not a profile coordinate
};

#endif // AFLOODTESTDIALOG_H
