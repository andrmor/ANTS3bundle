#include "arootgraphrecord.h"

#include <QMutexLocker>
#include <QDebug>

#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraph2D.h"
#include "TAxis.h"
#include "TAttLine.h"
#include "TAttMarker.h"
#include "TAttFill.h"

ARootGraphRecord::ARootGraphRecord(TObject *graph, const QString &name, const QString &type) :
    ARootObjBase(graph, name, type) {}

TObject *ARootGraphRecord::GetObject()
{
    TGraph* gr = dynamic_cast<TGraph*>(Object);
    if (gr)
    {
        gr->SetFillColor(0);
        gr->SetFillStyle(0);

        gr->SetLineColor(LineColor);
        gr->SetLineWidth(LineWidth);
        gr->SetLineStyle(LineStyle);
        gr->SetMarkerColor(MarkerColor);
        gr->SetMarkerSize(MarkerSize);
        gr->SetMarkerStyle(MarkerStyle);

        gr->SetEditable(false);

        gr->GetYaxis()->SetTitleOffset(1.30f);
        gr->GetXaxis()->SetTitle(TitleX.toLatin1().data());
        gr->GetYaxis()->SetTitle(TitleY.toLatin1().data());
    }
    else
    {
        TGraph2D * gr = dynamic_cast<TGraph2D*>(Object);
        if (gr)
        {
            gr->SetFillColor(0);
            gr->SetFillStyle(0);

            gr->SetLineColor(LineColor);
            gr->SetLineWidth(LineWidth);
            gr->SetLineStyle(LineStyle);
            gr->SetMarkerColor(MarkerColor);
            gr->SetMarkerSize(MarkerSize);
            gr->SetMarkerStyle(MarkerStyle);

            gr->GetYaxis()->SetTitleOffset(1.30f);
            gr->GetXaxis()->SetTitle(TitleX.toLatin1().data());
            gr->GetYaxis()->SetTitle(TitleY.toLatin1().data());
        }
    }

    return Object;
}

void ARootGraphRecord::setMarkerProperties(int markerColor, int markerStyle, double markerSize)
{
    MarkerColor = markerColor, MarkerStyle = markerStyle, MarkerSize = markerSize;
}

void ARootGraphRecord::setLineProperties(int lineColor, int lineStyle, int lineWidth)
{
    LineColor = lineColor,   LineStyle = lineStyle,    LineWidth = lineWidth;
}

void ARootGraphRecord::setTitle(QString graphTitle)
{
    QMutexLocker locker(&Mutex);

    TGraph * g = dynamic_cast<TGraph*>(Object);
    if (g)
    {
        Title = graphTitle;
        g->SetTitle(graphTitle.toLatin1().data());
        g->SetName(graphTitle.toLatin1().data());
    }
}

void ARootGraphRecord::setAxisTitles(const QString & titleX, const QString & titleY)
{
    TitleX = titleX; TitleY = titleY;
}

void ARootGraphRecord::addPoint(double x, double y, double errorX, double errorY)
{
    QMutexLocker locker(&Mutex);

    if (Type == "TGraph")
    {
        TGraph* g = dynamic_cast<TGraph*>(Object);
        g->SetPoint(g->GetN(), x, y);
    }
    else if (Type == "TGraphErrors")
    {
        TGraphErrors* g = dynamic_cast<TGraphErrors*>(Object);
        if (g)
        {
            const int iBin = g->GetN();
            g->SetPoint(iBin, x, y);
            g->SetPointError(iBin, errorX, errorY);
        }
    }
}

ARootObjBase::EStatus ARootGraphRecord::addPoints(const std::vector<double> & xArr, const std::vector<double> & yArr)
{
    QMutexLocker locker(&Mutex);

    const size_t size = xArr.size();
    if (size != yArr.size()) return ARootObjBase::DataMimatch;

    TGraph * g = dynamic_cast<TGraph*>(Object);
    if (g)
    {
        for (size_t i = 0; i < size; i++)
            g->SetPoint(g->GetN(), xArr[i], yArr[i]);
        return ARootObjBase::OK;
    }
    else return ARootObjBase::NotApplicable;
}

ARootObjBase::EStatus ARootGraphRecord::addPoints(const std::vector<double> & xArr, const std::vector<double> & yArr, const std::vector<double> & xErrArr, const std::vector<double> & yErrArr)
{
    QMutexLocker locker(&Mutex);

    if (xArr.size() != yArr.size() || xArr.size() != xErrArr.size() || xArr.size() != yErrArr.size()) return ARootObjBase::DataMimatch;

    TGraphErrors * g = dynamic_cast<TGraphErrors*>(Object);
    if (g)
    {
        for (size_t i = 0; i < xArr.size(); i++)
        {
            const int iBin = g->GetN();
            g->SetPoint(iBin, xArr[i], yArr[i]);
            g->SetPointError(iBin, xErrArr[i], yErrArr[i]);
        }
        return ARootObjBase::OK;
    }
    else return ARootObjBase::NotApplicable;
}

ARootObjBase::EStatus ARootGraphRecord::addPoints(const std::vector<double> &xArr, const std::vector<double> &yArr, const std::vector<double> &zArr)
{
    QMutexLocker locker(&Mutex);

    if (xArr.size() != yArr.size() || xArr.size() != zArr.size()) return ARootObjBase::DataMimatch;

    TGraph2D * g = dynamic_cast<TGraph2D*>(Object);
    if (g)
    {
        for (size_t i = 0; i < xArr.size(); i++)
        {
            const int iBin = g->GetN();
            g->SetPoint(iBin, xArr[i], yArr[i], zArr[i]);
        }
        return ARootObjBase::OK;
    }
    else return ARootObjBase::NotApplicable;
}

void ARootGraphRecord::sort()
{
    QMutexLocker locker(&Mutex);

    if (Type == "TGraph")
    {
        TGraph* g = dynamic_cast<TGraph*>(Object);
        g->Sort();
    }
}

void ARootGraphRecord::setYRange(double min, double max)
{
    QMutexLocker locker(&Mutex);

    if (Type == "TGraph")
    {
        TGraph* g = dynamic_cast<TGraph*>(Object);
        if (g)
        {
            g->SetMinimum(min);
            g->SetMaximum(max);
        }
    }
}

void ARootGraphRecord::setMinimum(double min)
{
    QMutexLocker locker(&Mutex);

    TGraph * g = dynamic_cast<TGraph*>(Object);
    //if (Type == "TGraph" || Type == "TGraphErrors")
    if (g)
    {
        //TGraph* g = dynamic_cast<TGraph*>(Object);
        //if (g)
        g->SetMinimum(min);
        //    g->GetYaxis()->SetRangeUser(min, g->GetMaximum());
    }
}

void ARootGraphRecord::setMaximum(double max)
{
    QMutexLocker locker(&Mutex);

    if (Type == "TGraph")
    {
        TGraph* g = dynamic_cast<TGraph*>(Object);
        if (g)
            g->SetMaximum(max);
    }
}

void ARootGraphRecord::setXRange(double min, double max)
{
    QMutexLocker locker(&Mutex);

    if (Type == "TGraph")
    {
        TGraph* g = dynamic_cast<TGraph*>(Object);
        if (g)
        {
            TAxis* axis = g->GetXaxis();
            if (axis) axis->SetLimits(min, max);
        }
    }
}

void ARootGraphRecord::setXDivisions(int numDiv)
{
    QMutexLocker locker(&Mutex);

    TGraph* g = dynamic_cast<TGraph*>(Object);
    if (g)
    {
        TAxis* ax = g->GetXaxis();
        if (ax) ax->SetNdivisions(numDiv);
    }
}

void ARootGraphRecord::setYDivisions(int numDiv)
{
    QMutexLocker locker(&Mutex);

    TGraph* g = dynamic_cast<TGraph*>(Object);
    if (g)
    {
        TAxis* ax = g->GetYaxis();
        if (ax) ax->SetNdivisions(numDiv);
    }
}

void ARootGraphRecord::addPoint2D(double x, double y, double z)
{
    QMutexLocker locker(&Mutex);

    if (Type == "TGraph2D")
    {
        TGraph2D * g = dynamic_cast<TGraph2D*>(Object);
        g->SetPoint(g->GetN(), x, y, z);
    }
}

void ARootGraphRecord::getData(std::vector<double> & x, std::vector<double> & y, std::vector<double> & z,
                               std::vector<double> & errx, std::vector<double> & erry)
{
    QMutexLocker locker(&Mutex);

    if (Type == "TGraph")
    {
        TGraph * g = dynamic_cast<TGraph*>(Object);
        if (g)
        {
            const int numPoints = g->GetN();
            x.resize(numPoints);
            y.resize(numPoints);
            for (int ip = 0; ip < numPoints; ip++) g->GetPoint(ip, x[ip], y[ip]);
        }
    }
    else if (Type == "TGraphErrors")
    {
        TGraphErrors * g = dynamic_cast<TGraphErrors*>(Object);
        if (g)
        {
            const int numPoints = g->GetN();
            x.resize(numPoints); y.resize(numPoints);
            errx.resize(numPoints); erry.resize(numPoints);
            for (int ip = 0; ip < numPoints; ip++)
            {
                g->GetPoint(ip, x[ip], y[ip]);
                errx[ip] = g->GetErrorX(ip);
                erry[ip] = g->GetErrorY(ip);
            }
        }
    }
    else if (Type == "TGraph2D")
    {
        TGraph2D * g = dynamic_cast<TGraph2D*>(Object);
        if (g)
        {
            const int numPoints = g->GetN();
            x.resize(numPoints);
            y.resize(numPoints);
            z.resize(numPoints);
            for (int ip = 0; ip < numPoints; ip++) g->GetPoint(ip, x[ip], y[ip], z[ip]);
        }
    }
}

void ARootGraphRecord::exportRoot(const QString &fileName)
{
    QMutexLocker locker(&Mutex);

//    if (Type == "TGraph")
//    {
//        TGraph* g = dynamic_cast<TGraph*>(Object);
//        if (g) g->SaveAs(fileName.toLatin1().data(), LastDrawOption.toLatin1().data());
//    }

    Object->SaveAs(fileName.toLatin1().data(), LastDrawOption.toLatin1().data());
}
