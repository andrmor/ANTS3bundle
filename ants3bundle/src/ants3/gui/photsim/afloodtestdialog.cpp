#include "afloodtestdialog.h"
#include "aphotonsimsettings.h"
#include "aphotonsourceplotter.h"
#include "amaterialhub.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QDoubleValidator>
#include <QMessageBox>
#include <QCoreApplication>
#include <QElapsedTimer>

#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TGraph.h"
#include "TString.h"

#include <cmath>

AFloodTestDialog::AFloodTestDialog(const APhotonBombsSettings & bombSettings, QWidget * parent) :
    QDialog(parent), BombSet(bombSettings)
{
    setWindowTitle("Flood: test generation");

    QVBoxLayout * v = new QVBoxLayout(this);
    QLabel * labInfo = new QLabel("Uses the current Flood settings and the same generator as the simulation. Results go to the graph window.", this);
    labInfo->setWordWrap(true);
    v->addWidget(labInfo);

    QGridLayout * g = new QGridLayout();
    int row = 0;

    g->addWidget(new QLabel("Points", this), row, 0);
    sbPoints = new QSpinBox(this);
    sbPoints->setRange(1, 10000000);
    sbPoints->setValue(10000);
    g->addWidget(sbPoints, row++, 1);

    cbLimits = new QCheckBox("Apply material/object limits", this);
    const APhotonBombAdvancedSettings & adv = BombSet.AdvancedSettings;
    const bool anyLimit = adv.bOnlyMaterial || adv.bOnlyVolume;
    cbLimits->setChecked(anyLimit);
    cbLimits->setEnabled(anyLimit);
    cbLimits->setToolTip(anyLimit ? "As set in \"Only accept bomb positions which are inside of\"" : "No limits are set");
    g->addWidget(cbLimits, row++, 1);

    g->addWidget(new QLabel("Coordinates", this), row, 0);
    {
        QHBoxLayout * h = new QHBoxLayout();
        rbGlobal = new QRadioButton("global", this);
        rbLocal  = new QRadioButton("local (minus Position)", this);
        rbGlobal->setChecked(true);
        QButtonGroup * bg = new QButtonGroup(this);
        bg->addButton(rbGlobal);
        bg->addButton(rbLocal);
        h->addWidget(rbGlobal);
        h->addWidget(rbLocal);
        h->addStretch();
        g->addLayout(h, row++, 1);
    }

    g->addWidget(new QLabel("Plot", this), row, 0);
    {
        QHBoxLayout * h = new QHBoxLayout();
        rb1D = new QRadioButton("1D", this);
        rb2D = new QRadioButton("2D", this);
        rb3D = new QRadioButton("3D", this);
        rb1D->setChecked(true);
        QButtonGroup * bg = new QButtonGroup(this);
        for (QRadioButton * rb : {rb1D, rb2D, rb3D}) {bg->addButton(rb); h->addWidget(rb);}
        h->addStretch();
        g->addLayout(h, row++, 1);
        for (QRadioButton * rb : {rb1D, rb2D, rb3D})
            connect(rb, &QRadioButton::toggled, this, [this](bool on){ if (on) {updateQuantities(); updateControls();} });
    }

    g->addWidget(new QLabel("Quantity", this), row, 0);
    cobQuantity = new QComboBox(this);
    g->addWidget(cobQuantity, row++, 1);
    connect(cobQuantity, &QComboBox::activated, this, [this](int){ updateControls(); });

    g->addWidget(new QLabel("Bins per axis", this), row, 0);
    sbBins = new QSpinBox(this);
    sbBins->setRange(1, 10000);
    sbBins->setValue(100);
    g->addWidget(sbBins, row++, 1);

    g->addWidget(new QLabel("Range", this), row, 0);
    {
        QVBoxLayout * vr = new QVBoxLayout();
        QHBoxLayout * h = new QHBoxLayout();
        rbAutoRange = new QRadioButton("auto", this);
        QRadioButton * rbManual = new QRadioButton("manual", this);
        rbAutoRange->setChecked(true);
        QButtonGroup * bg = new QButtonGroup(this);
        bg->addButton(rbAutoRange);
        bg->addButton(rbManual);
        h->addWidget(rbAutoRange);
        h->addWidget(rbManual);
        h->addStretch();
        vr->addLayout(h);
        QDoubleValidator * dv = new QDoubleValidator(this);
        for (int i = 0; i < 3; i++)
        {
            QHBoxLayout * ha = new QHBoxLayout();
            ha->addWidget(new QLabel(QString("axis %1: from").arg(i+1), this));
            leFrom[i] = new QLineEdit("0", this);   leFrom[i]->setValidator(dv);  ha->addWidget(leFrom[i]);
            ha->addWidget(new QLabel("to", this));
            leTo[i]   = new QLineEdit("10", this);  leTo[i]->setValidator(dv);    ha->addWidget(leTo[i]);
            ha->addWidget(new QLabel("mm", this));
            vr->addLayout(ha);
        }
        g->addLayout(vr, row++, 1);
        connect(rbAutoRange, &QRadioButton::toggled, this, [this](bool){ updateControls(); });
    }

    cbOverlay = new QCheckBox("Overlay expected curve", this);
    cbOverlay->setToolTip("1D only, for a profile coordinate of the shape. Ignores material/object limits.");
    g->addWidget(cbOverlay, row++, 1);

    v->addLayout(g);

    {
        QHBoxLayout * h = new QHBoxLayout();
        h->addStretch();
        pbGenerate = new QPushButton("Generate", this);
        pbAbort    = new QPushButton("Abort", this);
        pbAbort->setEnabled(false);
        h->addWidget(pbGenerate);
        h->addWidget(pbAbort);
        h->addStretch();
        v->addLayout(h);
        connect(pbGenerate, &QPushButton::clicked, this, &AFloodTestDialog::generate);
        connect(pbAbort,    &QPushButton::clicked, this, [this](){ bAbortRequested = true; });
    }

    labStatus = new QLabel(this);
    labStatus->setWordWrap(true);
    labStatus->setTextInteractionFlags(Qt::TextSelectableByMouse);
    v->addWidget(labStatus);

    updateQuantities();
    updateControls();
}

int AFloodTestDialog::dimension() const
{
    if (rb3D->isChecked()) return 3;
    if (rb2D->isChecked()) return 2;
    return 1;
}

QString AFloodTestDialog::quantityTitle(EQuantity q, int index)
{
    switch (q)
    {
    case QX      : return "X";
    case QY      : return "Y";
    case QZ      : return "Z";
    case QR      : return "R from the Position";
    case QAxial  : return "Axial";
    case QXY     : return (index == 0 ? "X" : "Y");
    case QXZ     : return (index == 0 ? "X" : "Z");
    case QYZ     : return (index == 0 ? "Y" : "Z");
    case QAxialR : return (index == 0 ? "Axial" : "R");
    case QXYZ    : return (index == 0 ? "X" : (index == 1 ? "Y" : "Z"));
    }
    return "";
}

void AFloodTestDialog::updateQuantities()
{
    const AFloodSettings & fs = BombSet.FloodSettings;
    const bool cyl = (fs.Shape == AFloodSettings::Cylinder);
    const QString old = cobQuantity->currentText();

    ShownQuantities.clear();
    QStringList names;
    switch (dimension())
    {
    case 1 :
        ShownQuantities = {QX, QY, QZ, QR};
        names = {"X", "Y", "Z", (fs.Shape == AFloodSettings::Cylinder ? "R (transverse)" : "R from the Position")};
        if (cyl) {ShownQuantities.push_back(QAxial); names << "Axial";}
        break;
    case 2 :
        ShownQuantities = {QXY, QXZ, QYZ};
        names = {"XY", "XZ", "YZ"};
        if (cyl) {ShownQuantities.push_back(QAxialR); names << "Axial vs R";}
        break;
    default :
        ShownQuantities = {QXYZ};
        names = {"XYZ"};
        break;
    }

    cobQuantity->clear();
    cobQuantity->addItems(names);
    const int index = names.indexOf(old);
    cobQuantity->setCurrentIndex(index >= 0 ? index : 0);
    sbBins->setValue(dimension() == 3 ? 30 : 100);
}

void AFloodTestDialog::updateControls()
{
    const int dim = dimension();
    for (int i = 0; i < 3; i++)
    {
        const bool used = (i < dim);
        leFrom[i]->setEnabled(used && !rbAutoRange->isChecked());
        leTo[i]  ->setEnabled(used && !rbAutoRange->isChecked());
    }

    const int iq = cobQuantity->currentIndex();
    const bool profileCoord = (dim == 1 && iq >= 0 && iq < (int)ShownQuantities.size() &&
                               profileCoordinate(BombSet.FloodSettings, ShownQuantities[iq]) >= 0);
    cbOverlay->setEnabled(profileCoord);
}

int AFloodTestDialog::profileCoordinate(const AFloodSettings & fs, EQuantity q)
{
    switch (fs.Shape)
    {
    case AFloodSettings::Box :
        if (q == QX) return 0;
        if (q == QY) return 1;
        if (q == QZ) return 2;
        return -1;
    case AFloodSettings::Cylinder :
        if (q == QR)     return 0;
        if (q == QAxial) return 1;
        return -1;
    case AFloodSettings::Sphere :
        return (q == QR ? 0 : -1);
    }
    return -1;
}

void AFloodTestDialog::quantityValues(const AFloodSettings & fs, EQuantity q, const double * global, bool local, double * out)
{
    const double center[3] = {fs.X0, fs.Y0, fs.Z0};
    double l[3], b[3];
    for (int i = 0; i < 3; i++)
    {
        l[i] = global[i] - center[i];
        b[i] = (local ? l[i] : global[i]);
    }

    const int axis = static_cast<int>(fs.Axis);
    double rTrans2 = 0;
    for (int i = 0; i < 3; i++) if (i != axis) rTrans2 += l[i] * l[i];
    const double r = (fs.Shape == AFloodSettings::Cylinder ? std::sqrt(rTrans2) : std::sqrt(l[0]*l[0] + l[1]*l[1] + l[2]*l[2]));

    switch (q)
    {
    case QX      : out[0] = b[0]; break;
    case QY      : out[0] = b[1]; break;
    case QZ      : out[0] = b[2]; break;
    case QR      : out[0] = r;    break;
    case QAxial  : out[0] = l[axis]; break;
    case QXY     : out[0] = b[0]; out[1] = b[1]; break;
    case QXZ     : out[0] = b[0]; out[1] = b[2]; break;
    case QYZ     : out[0] = b[1]; out[1] = b[2]; break;
    case QAxialR : out[0] = l[axis]; out[1] = r; break;
    case QXYZ    : out[0] = b[0]; out[1] = b[1]; out[2] = b[2]; break;
    }
}

void AFloodTestDialog::autoRange(const AFloodSettings & fs, EQuantity q, bool local, double * from, double * to) const
{
    const double center[3] = {fs.X0, fs.Y0, fs.Z0};
    double half[3];
    const double rOut = 0.5 * fs.OuterDiameter;
    switch (fs.Shape)
    {
    case AFloodSettings::Box :
        half[0] = 0.5 * fs.SizeX;  half[1] = 0.5 * fs.SizeY;  half[2] = 0.5 * fs.SizeZ;
        break;
    case AFloodSettings::Cylinder :
        for (int i = 0; i < 3; i++) half[i] = rOut;
        half[static_cast<int>(fs.Axis)] = 0.5 * fs.Length;
        break;
    case AFloodSettings::Sphere :
        for (int i = 0; i < 3; i++) half[i] = rOut;
        break;
    }

    auto axisRange = [&](int i, int slot)
    {
        const double h = (half[i] > 0 ? half[i] : 1.0);
        const double c = (local ? 0 : center[i]);
        from[slot] = c - h;
        to[slot]   = c + h;
    };
    auto radiusRange = [&](int slot)
    {
        double rMax = rOut;
        if (fs.Shape == AFloodSettings::Box) rMax = std::sqrt(half[0]*half[0] + half[1]*half[1] + half[2]*half[2]);
        from[slot] = 0;
        to[slot]   = (rMax > 0 ? rMax : 1.0);
    };
    auto axialRange = [&](int slot)
    {
        const double h = (fs.Length > 0 ? 0.5 * fs.Length : 1.0);
        from[slot] = -h;
        to[slot]   =  h;
    };

    switch (q)
    {
    case QX      : axisRange(0, 0); break;
    case QY      : axisRange(1, 0); break;
    case QZ      : axisRange(2, 0); break;
    case QR      : radiusRange(0);  break;
    case QAxial  : axialRange(0);   break;
    case QXY     : axisRange(0, 0); axisRange(1, 1); break;
    case QXZ     : axisRange(0, 0); axisRange(2, 1); break;
    case QYZ     : axisRange(1, 0); axisRange(2, 1); break;
    case QAxialR : axialRange(0);   radiusRange(1);  break;
    case QXYZ    : axisRange(0, 0); axisRange(1, 1); axisRange(2, 2); break;
    }
}

void AFloodTestDialog::generate()
{
    updateQuantities();   // the shape may have changed since the dialog was opened
    updateControls();

    AFloodSettings fs = BombSet.FloodSettings;   // copy: runtime state stays in the dialog
    QString warnings;
    const QString err = fs.updateRuntimeProperties(&warnings);
    if (!err.isEmpty())
    {
        QMessageBox::warning(this, "", "Flood: " + err);
        return;
    }

    const APhotonBombAdvancedSettings & adv = BombSet.AdvancedSettings;
    const bool applyLimits = cbLimits->isEnabled() && cbLimits->isChecked();
    TString limitVolume;
    int limitMaterial = -1;
    if (applyLimits && adv.bOnlyVolume) limitVolume = TString(adv.Volume.toLatin1().data());
    if (applyLimits && adv.bOnlyMaterial)
    {
        limitMaterial = AMaterialHub::getConstInstance().findMaterial(adv.Material);
        if (limitMaterial < 0)
        {
            QMessageBox::warning(this, "", "Unknown limiting material: " + adv.Material);
            return;
        }
    }

    const int iq = cobQuantity->currentIndex();
    if (iq < 0 || iq >= (int)ShownQuantities.size()) return;
    const EQuantity q   = ShownQuantities[iq];
    const int       dim = dimension();
    const bool      local = rbLocal->isChecked();
    const int       bins = sbBins->value();

    double from[3] = {0, 0, 0}, to[3] = {1, 1, 1};
    autoRange(fs, q, local, from, to);
    if (!rbAutoRange->isChecked())
    {
        for (int i = 0; i < dim; i++)
        {
            from[i] = leFrom[i]->text().toDouble();
            to[i]   = leTo[i]->text().toDouble();
            if (!(to[i] > from[i]))
            {
                QMessageBox::warning(this, "", "Manual range: 'to' must be larger than 'from'");
                return;
            }
        }
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            leFrom[i]->setText(QString::number(from[i]));
            leTo[i]->setText(QString::number(to[i]));
        }
    }

    const QString cs = (local ? " (local)" : "");
    TH1 * hist = nullptr;
    const TString title = TString("Flood test: ") + cobQuantity->currentText().toLatin1().data();
    if (dim == 1)
    {
        hist = new TH1D("", title, bins, from[0], to[0]);
        hist->GetXaxis()->SetTitle((quantityTitle(q, 0) + cs + ", mm").toLatin1().data());
    }
    else if (dim == 2)
    {
        hist = new TH2D("", title, bins, from[0], to[0], bins, from[1], to[1]);
        hist->GetXaxis()->SetTitle((quantityTitle(q, 0) + cs + ", mm").toLatin1().data());
        hist->GetYaxis()->SetTitle((quantityTitle(q, 1) + cs + ", mm").toLatin1().data());
    }
    else
    {
        hist = new TH3D("", title, bins, from[0], to[0], bins, from[1], to[1], bins, from[2], to[2]);
        hist->GetXaxis()->SetTitle(("X" + cs + ", mm").toLatin1().data());
        hist->GetYaxis()->SetTitle(("Y" + cs + ", mm").toLatin1().data());
        hist->GetZaxis()->SetTitle(("Z" + cs + ", mm").toLatin1().data());
    }

    const int numPoints = sbPoints->value();
    long long attempts = 0;
    int accepted = 0;
    QString failure;
    bAbortRequested = false;
    pbGenerate->setEnabled(false);
    pbAbort->setEnabled(true);
    QElapsedTimer timer;
    timer.start();

    double r[3], val[3];
    for (int iPoint = 0; iPoint < numPoints; iPoint++)
    {
        int watchdog = adv.MaxNodeAttempts;
        bool ok = false;
        while (true)
        {
            attempts++;
            if (!fs.generatePosition(r))
            {
                failure = "Failed to generate a position";
                break;
            }
            if ( (applyLimits && adv.bOnlyVolume   && !APhotonSourcePlotter::isInsideLimitingVolume(r, limitVolume)) ||
                 (applyLimits && adv.bOnlyMaterial && !APhotonSourcePlotter::isInsideLimitingMaterial(r, limitMaterial)) )
            {
                if (--watchdog < 0)
                {
                    failure = "Failed to generate a point inside the limiting volume/material";
                    break;
                }
                continue;
            }
            ok = true;
            break;
        }
        if (!ok) break;

        quantityValues(fs, q, r, local, val);
        if      (dim == 1) hist->Fill(val[0]);
        else if (dim == 2) static_cast<TH2D*>(hist)->Fill(val[0], val[1]);
        else               static_cast<TH3D*>(hist)->Fill(val[0], val[1], val[2]);
        accepted++;

        if (iPoint % 10000 == 9999)
        {
            labStatus->setText(QString("Generated %1 of %2").arg(iPoint + 1).arg(numPoints));
            QCoreApplication::processEvents();
            if (bAbortRequested)
            {
                failure = "Aborted";
                break;
            }
        }
    }

    pbGenerate->setEnabled(true);
    pbAbort->setEnabled(false);

    const char * opt = (dim == 1 ? "hist" : (dim == 2 ? "colz" : "box2"));
    emit requestDraw(hist, opt, true, true);

    // expected curve
    const int iCoord = profileCoordinate(fs, q);
    if (dim == 1 && cbOverlay->isEnabled() && cbOverlay->isChecked() && iCoord >= 0 && accepted > 0)
    {
        const AFloodRuntime & rt = fs.getRuntime();
        const AFloodCoordSampler & s = (iCoord == 0 ? rt.C1 : (iCoord == 1 ? rt.C2 : rt.C3));
        if (s.Mode != AFloodCoordSampler::Fixed)
        {
            double shift = 0;
            if (!local && fs.Shape == AFloodSettings::Box) shift = (iCoord == 0 ? fs.X0 : (iCoord == 1 ? fs.Y0 : fs.Z0));
            const double binWidth = (to[0] - from[0]) / bins;
            TGraph * g = new TGraph();
            const int nPoints = 400;
            for (int i = 0; i <= nPoints; i++)
            {
                const double x = from[0] + (to[0] - from[0]) * i / nPoints;
                g->AddPoint(x, s.expectedPdf(x - shift) * accepted * binWidth);
            }
            g->SetLineColor(2);
            g->SetLineWidth(2);
            emit requestDraw(g, "L same", true, false);
        }
    }

    QString status = QString("%1 accepted · %2 attempts · acceptance %3 % · %4 s")
                         .arg(accepted).arg(attempts).arg(attempts > 0 ? 100.0 * accepted / attempts : 0, 0, 'g', 4)
                         .arg(0.001 * timer.elapsed(), 0, 'f', 2);
    if (!failure.isEmpty())  status += "\n" + failure;
    if (!warnings.isEmpty()) status += "\nWarnings: " + warnings;
    if (applyLimits)         status += "\nThe expected curve ignores the material/object limits.";
    labStatus->setText(status);
}
