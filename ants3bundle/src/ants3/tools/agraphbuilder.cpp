#include "agraphbuilder.h"

#include "TGraph.h"
#include "TGraph2D.h"
#include "TAxis.h"
#include "TVectorD.h"

TGraph * AGraphBuilder::graph(const std::vector<double> &x, const std::vector<double> &y)
{
    int numEl = (int)x.size();
    TVectorD xx(numEl);
    TVectorD yy(numEl);
    for (int i=0; i < numEl; i++)
    {
        xx[i] = x.at(i);
        yy[i] = y.at(i);
    }

    TGraph * gr = new TGraph(xx,yy);
    applyDefaults(gr);
    return gr;
}

TGraph * AGraphBuilder::graph(const std::vector<float> &x, const std::vector<float> &y)
{
    int numEl = (int)x.size();
    TVectorD xx(numEl);
    TVectorD yy(numEl);
    for (int i=0; i < numEl; i++)
    {
        xx[i] = x.at(i);
        yy[i] = y.at(i);
    }

    TGraph* gr = new TGraph(xx,yy);
    applyDefaults(gr);
    return gr;
}

TGraph * AGraphBuilder::graph(const std::vector<std::pair<double, double>> & data)
{
    const size_t numEl = data.size();
    TVectorD xx(numEl);
    TVectorD yy(numEl);
    for (size_t i = 0; i < numEl; i++)
    {
        xx[i] = data[i].first;
        yy[i] = data[i].second;
    }

    TGraph * gr = new TGraph(xx,yy);
    applyDefaults(gr);
    return gr;
}

TGraph * AGraphBuilder::graph(const std::vector<std::pair<int, double>> & data)
{
    const size_t numEl = data.size();
    TVectorD xx(numEl);
    TVectorD yy(numEl);
    for (size_t i = 0; i < numEl; i++)
    {
        xx[i] = data[i].first;
        yy[i] = data[i].second;
    }

    TGraph * gr = new TGraph(xx,yy);
    applyDefaults(gr);
    return gr;
}

TGraph * AGraphBuilder::graph(const std::vector<std::pair<double, std::complex<double>>> & data, bool real)
{
    int numEl = (int)data.size();
    TVectorD xx(numEl);
    TVectorD yy(numEl);
    for (int i = 0; i < numEl; i++)
    {
        xx[i] = data[i].first;
        yy[i] = (real ? data[i].second.real() : data[i].second.imag());
    }

    TGraph * gr = new TGraph(xx,yy);
    applyDefaults(gr);
    return gr;
}

// -----

void AGraphBuilder::applyDefaults(TGraph * graph)
{
    graph->SetFillStyle(0);
    graph->SetFillColor(0);
    graph->SetEditable(false);
    graph->GetYaxis()->SetTitleOffset(1.30f);
}

// -----

void AGraphBuilder::configure(TGraph * graph, const QString & graphTitle,
                              const QString & xTitle, const QString & yTitle,
                              int markerColor, int markerStyle, double markerSize,
                              int lineColor,   int lineStyle, int lineWidth)
{
    graph->SetTitle(graphTitle.toLatin1().data());

    graph->GetXaxis()->SetTitle(xTitle.toLatin1().data());
    graph->GetYaxis()->SetTitle(yTitle.toLatin1().data());

    graph->SetMarkerColor(markerColor); graph->SetMarkerStyle(markerStyle); graph->SetMarkerSize(markerSize);
    graph->SetLineColor(lineColor);     graph->SetLineStyle(lineStyle);     graph->SetLineWidth(lineWidth);
}

void AGraphBuilder::configureTitles(TGraph * graph, const QString &graphTitle, const QString &xTitle, const QString &yTitle)
{
    graph->SetTitle(graphTitle.toLatin1().data());
    graph->GetXaxis()->SetTitle(xTitle.toLatin1().data());
    graph->GetYaxis()->SetTitle(yTitle.toLatin1().data());
}

void AGraphBuilder::configureMarkers(TGraph * graph, int markerColor, int markerStyle, double markerSize)
{
    graph->SetMarkerColor(markerColor);
    graph->SetMarkerStyle(markerStyle);
    graph->SetMarkerSize(markerSize);
}

void AGraphBuilder::configureLine(TGraph *graph, int lineColor, int lineStyle, int lineWidth)
{
    graph->SetLineColor(lineColor);
    graph->SetLineStyle(lineStyle);
    graph->SetLineWidth(lineWidth);
}

// -----

void AGraphBuilder::shift(TGraph * g, double multiply, double add)
{
    int numPoints = g->GetN();

    for (int i = 0; i < numPoints; i++)
    {
        double x = g->GetPointX(i);
        x *= multiply;
        x += add;
        g->SetPointX(i, x);
    }
}

void AGraphBuilder::scale(TGraph * g, double multiply, double add)
{
    int numPoints = g->GetN();

    for (int i = 0; i < numPoints; i++)
    {
        double y = g->GetPointY(i);
        y *= multiply;
        y += add;
        g->SetPointY(i, y);
    }
}
