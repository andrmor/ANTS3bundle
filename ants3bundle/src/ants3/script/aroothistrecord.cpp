#include "aroothistrecord.h"
#include "apeakfinder.h"

#include <QDebug>

#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TF1.h"
#include "TAxis.h"

ARootHistRecord::ARootHistRecord(TObject *hist, const QString &title, const QString &type) :
    ARootObjBase(hist, title, type) {}

TObject* ARootHistRecord::GetObject()
{
    return Object;
}

void ARootHistRecord::setTitle(const QString & title)
{
    QMutexLocker locker(&Mutex);
    TH1 * h = dynamic_cast<TH1*>(Object);
    h->SetTitle(title.toLatin1().data());
}

void ARootHistRecord::setAxisTitles(const QString &x_Title, const QString &y_Title, const QString &z_Title)
{
    QMutexLocker locker(&Mutex);

    if (Type == "TH1D")
    {
        TH1D* h = static_cast<TH1D*>(Object);
        h->SetXTitle(x_Title.toLatin1().data());
        h->SetYTitle(y_Title.toLatin1().data());
    }
    else if (Type == "TH2D")
    {
        TH2D* h = static_cast<TH2D*>(Object);
        h->SetXTitle(x_Title.toLatin1().data());
        h->SetYTitle(y_Title.toLatin1().data());
        h->SetZTitle(z_Title.toLatin1().data());
    }
}

void ARootHistRecord::setLineProperties(int lineColor, int lineStyle, int lineWidth)
{
    QMutexLocker locker(&Mutex);

    TH1* h = dynamic_cast<TH1*>(Object);
    if (h)
    {
        h->SetLineColor(lineColor);
        h->SetLineWidth(lineWidth);
        h->SetLineStyle(lineStyle);
    }
}

void ARootHistRecord::setMarkerProperties(int markerColor, int markerStyle, double markerSize)
{
    TH1* h = dynamic_cast<TH1*>(Object);
    if (h)
    {
        h->SetMarkerColor(markerColor);
        h->SetMarkerStyle(markerStyle);
        h->SetMarkerSize(markerSize);
    }
}

void ARootHistRecord::setFillColor(int color)
{
    TH1* h = dynamic_cast<TH1*>(Object);
    if (h) h->SetFillColor(color);
}

void ARootHistRecord::setXLabels(const std::vector<QString> & labels)
{
    int numLabels = labels.size();

    TH1 * h = dynamic_cast<TH1*>(Object);
    if (h)
    {
        TAxis * axis = h->GetXaxis();
        if (!axis) return;

        const int numBins = axis->GetNbins();
        for (int i = 1; i <= numLabels && i <= numBins ; i++)
            axis->SetBinLabel(i, labels.at(i-1).toLatin1().data());
    }
}

void ARootHistRecord::setXDivisions(int primary, int secondary, int tertiary, bool canOptimize)
{
    TH1* h = dynamic_cast<TH1*>(Object);
    if (h)
    {
        TAxis * axis = h->GetXaxis();
        if (!axis) return;

        axis->SetNdivisions(primary, secondary, tertiary, canOptimize);
    }
}

void ARootHistRecord::setYDivisions(int primary, int secondary, int tertiary, bool canOptimize)
{
    TH1* h = dynamic_cast<TH1*>(Object);
    if (h)
    {
        TAxis * axis = h->GetYaxis();
        if (!axis) return;

        axis->SetNdivisions(primary, secondary, tertiary, canOptimize);
    }
}

void ARootHistRecord::setXLabelProperties(double size, double offset)
{
    TH1* h = dynamic_cast<TH1*>(Object);
    if (h)
    {
        TAxis * axis = h->GetXaxis();
        if (!axis) return;

        axis->SetLabelSize(size);
        axis->SetLabelOffset(offset);
    }
}

void ARootHistRecord::setYLabelProperties(double size, double offset)
{
    TH1* h = dynamic_cast<TH1*>(Object);
    if (h)
    {
        TAxis * axis = h->GetYaxis();
        if (!axis) return;

        axis->SetLabelSize(size);
        axis->SetLabelOffset(offset);
    }
}

void ARootHistRecord::fill1D(double x, double weight)
{
    if (Type == "TH1D")
    {
        QMutexLocker locker(&Mutex);
        TH1D* h = static_cast<TH1D*>(Object);
        h->Fill(x, weight);
    }
}

void ARootHistRecord::fill2D(double x, double y, double weight)
{
    if (Type == "TH2D")
    {
        QMutexLocker locker(&Mutex);
        TH2D* h = static_cast<TH2D*>(Object);
        h->Fill(x, y, weight);
    }
}

void ARootHistRecord::fill3D(double x, double y, double z, double weight)
{
    if (Type == "TH3D")
    {
        QMutexLocker locker(&Mutex);
        TH3D * h = static_cast<TH3D*>(Object);
        h->Fill(x, y, z, weight);
    }
}

void ARootHistRecord::fill1D(const std::vector<double> & x, const std::vector<double> & weight)
{
    if (x.size() == weight.size() && Type == "TH1D")
    {
        QMutexLocker locker(&Mutex);
        TH1D * h = static_cast<TH1D*>(Object);
        for (size_t i = 0; i < x.size(); i++)
            h->Fill(x[i], weight[i]);
    }
}

void ARootHistRecord::fill2D(const std::vector<double> & x, const std::vector<double> & y, const std::vector<double> & weight)
{
    if (x.size() == weight.size() && y.size() == weight.size() && Type == "TH2D")
    {
        QMutexLocker locker(&Mutex);
        TH2D * h = static_cast<TH2D*>(Object);
        for (size_t i = 0; i < x.size(); i++)
            h->Fill(x[i], y[i], weight[i]);
    }
}

void ARootHistRecord::fill3D(const std::vector<double> & x, const std::vector<double> & y, const std::vector<double> & z, const std::vector<double> & weight)
{
    if (x.size() == weight.size() && y.size() == weight.size() && z.size() == weight.size() && Type == "TH3D")
    {
        QMutexLocker locker(&Mutex);
        TH3D * h = static_cast<TH3D*>(Object);
        for (size_t i = 0; i < x.size(); i++)
            h->Fill(x[i], y[i], z[i], weight[i]);
    }
}

void ARootHistRecord::setMax(double max)
{
    TH1* h = dynamic_cast<TH1*>(Object);
    if (h) h->SetMaximum(max);
}

void ARootHistRecord::setMin(double min)
{
    TH1* h = dynamic_cast<TH1*>(Object);
    if (h) h->SetMinimum(min);
}

void ARootHistRecord::Save(const QString &fileName) const
{
    QMutexLocker locker(&Mutex);

    if (Type == "TH1D")
    {
        TH1D* h = static_cast<TH1D*>(Object);
        TH1D* hc = new TH1D(*h);
        locker.unlock();
        hc->SaveAs(fileName.toLatin1());
        delete hc;
    }
    else //if (Type == "TH2D")
    {
        TH2D* h = static_cast<TH2D*>(Object);
        TH2D* hc = new TH2D(*h);
        locker.unlock();
        hc->SaveAs(fileName.toLatin1());
        delete hc;
    }
}

bool ARootHistRecord::Divide(ARootHistRecord *other)
{
    other->externalLock();
    QMutexLocker locker(&Mutex);

    TH1* h1 = dynamic_cast<TH1*>(Object);
    TH1* h2 = dynamic_cast<TH1*>(other->GetObject());

    bool bOK = false;
    if (h1 && h2)
        bOK = h1->Divide(h2);

    other->externalUnlock();
    return bOK;
}

void ARootHistRecord::Smooth(int times)
{
    QMutexLocker locker(&Mutex);

    if (Type == "TH1D")
    {
        TH1D* h = static_cast<TH1D*>(Object);
        h->Smooth(times);
    }
    else if (Type == "TH2D")
    {
        TH2D* h = static_cast<TH2D*>(Object);
        h->Smooth(times);
    }
}

void ARootHistRecord::Smear(double sigma)
{
    QMutexLocker locker(&Mutex);

    if (Type != "TH1D") return;

    TH1D * h = static_cast<TH1D*>(Object);

    int bins = h->GetXaxis()->GetNbins();
    double start = h->GetBinCenter(1);
    double end   = h->GetBinCenter(bins);

    double dx = (end - start) / (bins - 1);

    int span = 4.0 * sigma / dx;
    qDebug() << bins << start << end << dx << span;
    if (span == 0) return;

    // G(y) = 1/(sigma*sqrt(2*Pi)) * exp( - y*y / (2*sigma*sigma)) where y = x - mju

    double Original[bins+1];
    double Convoluted[bins+1];
    for (int iBin = 1; iBin < bins+1; iBin++)
    {
        Original[iBin] = h->GetBinContent(iBin);
        Convoluted[iBin] = 0;
    }

    double factor = 1.0 / (sigma * sqrt(2 * M_PI));

    for (int iBin = 1; iBin < bins+1; iBin++)
        for (int iG = -span; iG < span+1; iG++)
        {
            int iThis = iBin + iG;
            if (iThis < 1) continue;
            if (iThis > bins) continue;

            double delta = dx * iG;
            Convoluted[iThis] += Original[iBin]   * factor * exp( - delta * delta / (2*sigma*sigma));
        }

    for (int iBin = 1; iBin < bins+1; iBin++) h->SetBinContent(iBin, Convoluted[iBin]);
}

void ARootHistRecord::Scale(double ScaleIntegralTo, bool bDividedByBinWidth)
{
    QMutexLocker locker(&Mutex);

    if (Type == "TH1D")
    {
        TH1* h = dynamic_cast<TH1*>(Object);
        h->Scale( ScaleIntegralTo, ( bDividedByBinWidth ? "width" : "" ) );
    }
}

bool ARootHistRecord::MedianFilter(int span, int spanRight)
{
    QMutexLocker locker(&Mutex);

    int deltaLeft = span;
    int deltaRight = spanRight;

    if (deltaRight == -1)
    {
        deltaLeft  = ( span % 2 == 0 ? span / 2 - 1 : span / 2 );
        deltaRight = span / 2;
    }
    //qDebug() << "Delta left:"<< deltaLeft <<" Delta right:"<< deltaRight;

    TH1* h = dynamic_cast<TH1*>(Object);
    if (!h) return false;

    QVector<double> Filtered;
    int num = h->GetNbinsX();
    for (int iThisBin = 1; iThisBin <= num; iThisBin++)  // 0-> underflow; num+1 -> overflow
    {
        QVector<double> content;
        for (int i = iThisBin - deltaLeft; i <= iThisBin + deltaRight; i++)
        {
            if (i < 1 || i > num) continue;
            content << h->GetBinContent(i);
        }

        std::sort(content.begin(), content.end());
        int size = content.size();
        double val;
        if (size == 0) val = 0;
        else val = ( size % 2 == 0 ? (content[size / 2 - 1] + content[size / 2]) / 2 : content[size / 2] );

        Filtered.append(val);
    }
    //qDebug() << "Result:" << Filtered;

    for (int iThisBin = 1; iThisBin <= num; iThisBin++)
        h->SetBinContent(iThisBin, Filtered.at(iThisBin-1));

    return true;
}

double ARootHistRecord::getIntegral(bool bMultipliedByBinWidth)
{
    TH1* h = dynamic_cast<TH1*>(Object);
    if (!h) return 1.0;
    return ( bMultipliedByBinWidth ? h->Integral("width") : h->Integral() );
}

int ARootHistRecord::getEntries()
{
    TH1* h = dynamic_cast<TH1*>(Object);
    if (!h) return 0;

    return h->GetEntries();
}

void ARootHistRecord::GetStatistics(int &num, std::vector<double> &mean, std::vector<double> &std)
{
    TH2 * h2 = dynamic_cast<TH2*>(Object);
    if (h2)
    {
        num     = h2->GetEntries();
        mean[0] = h2->GetMean(1);
        mean[1] = h2->GetMean(2);
        std[0]  = h2->GetStdDev(1);
        std[1]  = h2->GetStdDev(2);
    }
    TH1 * h1 = dynamic_cast<TH1*>(Object);
    if (h1)
    {
        num     = h1->GetEntries();
        mean[0] = h1->GetMean(1);
        std[0]  = h1->GetStdDev(1);
    }
}

void ARootHistRecord::setEntries(int num)
{
    TH1* h = dynamic_cast<TH1*>(Object);
    if (!h) return;

    h->SetEntries(num);
}

double ARootHistRecord::getMaximum() const
{
    TH1 * h = dynamic_cast<TH1*>(Object);
    if (!h) return 1.0;

    return h->GetMaximum();
}

double ARootHistRecord::getMinimum() const
{
    TH1 * h = dynamic_cast<TH1*>(Object);
    if (!h) return 1.0;

    return h->GetMinimum();
}

bool ARootHistRecord::getContent1D(std::vector<double> & x, std::vector<double> & w) const
{
    if (!Type.startsWith("TH1")) return false;

    QMutexLocker locker(&Mutex);

    TH1 * h = dynamic_cast<TH1*>(Object);
    if (!h) return false;

    const int num = h->GetNbinsX();
    x.reserve(num - 2);
    w.reserve(num - 2);
    for (int i = 1; i <= num; i++)
    {
        x.push_back(h->GetBinCenter(i));
        w.push_back(h->GetBinContent(i));
    }
    return true;
}

bool ARootHistRecord::getContent2D(std::vector<double> & x, std::vector<double> & y, std::vector<double> & w) const
{
    if (!Type.startsWith("TH2")) return false;

    QMutexLocker locker(&Mutex);

    TH2 * h = dynamic_cast<TH2*>(Object);
    if (!h) return false;

    const int numX = h->GetNbinsX();
    const int numY = h->GetNbinsY();
    const int num = (numX - 2) * (numY - 2);
    x.reserve(num);
    y.reserve(num);
    w.reserve(num);
    for (int iy = 1; iy <= numY; iy++)
        for (int ix = 1; ix <= numX; ix++)
        {
            x.push_back(h->GetXaxis()->GetBinCenter(ix));
            y.push_back(h->GetYaxis()->GetBinCenter(iy));
            w.push_back(h->GetBinContent(ix, iy));
        }
    return true;
}

bool ARootHistRecord::getContent3D(std::vector<double> & x, std::vector<double> & y, std::vector<double> & z, std::vector<double> & w) const
{
    if (!Type.startsWith("TH3")) return false;

    QMutexLocker locker(&Mutex);

    TH3 * h = dynamic_cast<TH3*>(Object);
    if (!h) return false;

    const int numX = h->GetNbinsX();
    const int numY = h->GetNbinsY();
    const int numZ = h->GetNbinsZ();
    const int num = (numX - 2) * (numY - 2) * (numZ - 2);
    x.reserve(num);
    y.reserve(num);
    z.reserve(num);
    w.reserve(num);
    for (int iz = 1; iz <= numZ; iz++)
        for (int iy = 1; iy <= numY; iy++)
            for (int ix = 1; ix <= numX; ix++)
            {
                x.push_back(h->GetXaxis()->GetBinCenter(ix));
                y.push_back(h->GetYaxis()->GetBinCenter(iy));
                z.push_back(h->GetZaxis()->GetBinCenter(iz));
                w.push_back(h->GetBinContent(ix, iy, iz));
            }
    return true;
}

bool ARootHistRecord::getUnderflow(double & undeflow) const
{
    if (!Type.startsWith("TH1")) return false;
    TH1* h = dynamic_cast<TH1*>(Object);
    if (!h) return false;

    undeflow = h->GetBinContent(0);
    return true;
}

bool ARootHistRecord::getOverflow(double & overflow) const
{
    if (!Type.startsWith("TH1")) return false;
    TH1* h = dynamic_cast<TH1*>(Object);
    if (!h) return false;

    int num = h->GetNbinsX();
    overflow = h->GetBinContent(num+1);
    return true;
}

double ARootHistRecord::GetRandom()
{
    TH1 * h = dynamic_cast<TH1*>(Object);
    if (!h) return 0;

    return h->GetRandom();
}

std::vector<double> ARootHistRecord::GetRandomMultiple(int numRandoms)
{
    std::vector<double> res;
    TH1 * h = dynamic_cast<TH1*>(Object);
    if (h)
    {
        res.reserve(numRandoms);
        for (int i=0; i<numRandoms; i++)
            res.push_back( h->GetRandom() );
    }
    return res;
}

bool ARootHistRecord::is1D() const
{
    return Type.startsWith("TH1");
}

bool ARootHistRecord::is2D() const
{
    return Type.startsWith("TH2");
}

bool ARootHistRecord::is3D() const
{
    return Type.startsWith("TH3");
}

QVector<double> ARootHistRecord::FitGaussWithInit(const QVector<double> &InitialParValues, const QString options)
{
    QMutexLocker locker(&Mutex);

    QVector<double> res;

    if (InitialParValues.size() != 3) return res;
    if (Type.startsWith("TH1"))
    {
        TH1* h = static_cast<TH1*>(Object);

        TF1 *f1 = new TF1("f1","[0]*exp(-0.5*((x-[1])/[2])^2)");
        f1->SetParameters(InitialParValues.at(0), InitialParValues.at(1), InitialParValues.at(2));

        int status = h->Fit(f1, options.toLatin1());
        if (status == 0)
        {
            for (int i=0; i<3; i++) res << f1->GetParameter(i);
            for (int i=0; i<3; i++) res << f1->GetParError(i);
        }
    }
    return res;
}

std::vector<double> ARootHistRecord::findPeaks(double sigma, double threshold)
{
    QMutexLocker locker(&Mutex);

    std::vector<double> res;

    TH1 * h = dynamic_cast<TH1*>(Object);
    if (h)
    {
        APeakFinder pf(h);
        res = pf.findPeaks(sigma, threshold);
    }

    return res;
}

QVector<double> ARootHistRecord::FitGauss(const QString &options)
{
    QMutexLocker locker(&Mutex);

    QVector<double> res;
    if (Type.startsWith("TH1"))
    {
        TH1* h = static_cast<TH1*>(Object);
        TF1 *f1 = new TF1("f1", "gaus");
        int status = h->Fit(f1, options.toLatin1().data());
        if (status == 0)
        {
            for (int i=0; i<3; i++) res << f1->GetParameter(i);
            for (int i=0; i<3; i++) res << f1->GetParError(i);
        }
    }
    return res;
}
