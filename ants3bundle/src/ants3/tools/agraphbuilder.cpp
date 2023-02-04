#include "agraphbuilder.h"

#include "TGraph.h"
#include "TGraph2D.h"
#include "TAxis.h"
#include "TVectorD.h"

TGraph * AGraphBuilder::graph(const QVector<double> & x, const QVector<double> & y)
{
    int numEl = x.size();
    TVectorD xx(numEl);
    TVectorD yy(numEl);
    for (int i=0; i < numEl; i++)
    {
        xx[i] = x.at(i);
        yy[i] = y.at(i);
    }

    TGraph* gr = new TGraph(xx,yy);
    gr->SetFillStyle(0);
    gr->SetFillColor(0);
    return gr;
}

TGraph *AGraphBuilder::graph(const std::vector<double> &x, const std::vector<double> &y)
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
    gr->SetFillStyle(0);
    gr->SetFillColor(0);
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
    gr->SetFillStyle(0);
    gr->SetFillColor(0);
    return gr;
}

TGraph * AGraphBuilder::graph(const QVector<double> &x, const QVector<double> &y,
                                          const char *Title, const char *XTitle, const char *YTitle,
                                          short MarkerColor, int MarkerStyle, int MarkerSize,
                                          short LineColor,   int LineStyle,   int LineWidth)
{
    TGraph* gr = graph(x,y);
    gr->SetTitle(Title); gr->GetXaxis()->SetTitle(XTitle); gr->GetYaxis()->SetTitle(YTitle);
    gr->SetMarkerStyle(MarkerStyle); gr->SetMarkerColor(MarkerColor); gr->SetMarkerSize(MarkerSize);
    gr->SetEditable(false); gr->GetYaxis()->SetTitleOffset((Float_t)1.30);
    gr->SetLineWidth(LineWidth); gr->SetLineColor(LineColor); gr->SetLineStyle(LineStyle);
    return gr;
}

TGraph * AGraphBuilder::graph(const QVector<double> &x, const QVector<double> &y,
                                          const QString &Title, const QString &XTitle, const QString &YTitle,
                                          short MarkerColor, int MarkerStyle, int MarkerSize,
                                          short LineColor,   int LineStyle,   int LineWidth)
{
    TGraph* gr = graph(x,y);
    gr->SetTitle(Title.toLatin1().data()); gr->GetXaxis()->SetTitle(XTitle.toLatin1().data()); gr->GetYaxis()->SetTitle(YTitle.toLatin1().data());
    gr->SetMarkerStyle(MarkerStyle); gr->SetMarkerColor(MarkerColor); gr->SetMarkerSize(MarkerSize);
    gr->SetEditable(false); gr->GetYaxis()->SetTitleOffset((Float_t)1.30);
    gr->SetLineWidth(LineWidth); gr->SetLineColor(LineColor); gr->SetLineStyle(LineStyle);
    return gr;
}

TGraph * AGraphBuilder::graph(const std::vector<float> &x, const std::vector<float> &y,
                                          const char *Title, const char *XTitle, const char *YTitle,
                                          short MarkerColor, int MarkerStyle, int MarkerSize,
                                          short LineColor,   int LineStyle,   int LineWidth)
{
    TGraph* gr = graph(x,y);
    gr->SetTitle(Title); gr->GetXaxis()->SetTitle(XTitle); gr->GetYaxis()->SetTitle(YTitle);
    gr->SetMarkerStyle(MarkerStyle); gr->SetMarkerColor(MarkerColor); gr->SetMarkerSize(MarkerSize);
    gr->SetEditable(false); gr->GetYaxis()->SetTitleOffset(1.30f);
    gr->SetLineWidth(LineWidth); gr->SetLineColor(LineColor); gr->SetLineStyle(LineStyle);
    return gr;
}

TGraph2D * AGraphBuilder::graph(const QVector<double> &x, const QVector<double> &y, const QVector<double> &z)
{
    int numEl = x.size();
    TGraph2D* gr = new TGraph2D(numEl, (double*)x.data(), (double*)y.data(), (double*)z.data());
    gr->SetFillStyle(0);
    gr->SetFillColor(0);
    return gr;
}

TGraph2D * AGraphBuilder::graph(const QVector<double>& x, const QVector<double>& y, const QVector<double>& z,
                                              const char *Title, const char *XTitle, const char *YTitle, const char *ZTitle,
                                              short MarkerColor, int MarkerStyle, int MarkerSize,
                                              short LineColor, int LineStyle, int LineWidth)
{
    TGraph2D* gr = graph(x,y,z);
    gr->SetTitle(Title); gr->GetXaxis()->SetTitle(XTitle); gr->GetYaxis()->SetTitle(YTitle); gr->GetZaxis()->SetTitle(ZTitle);
    gr->SetMarkerStyle(MarkerStyle); gr->SetMarkerColor(MarkerColor); gr->SetMarkerSize(MarkerSize);
    gr->GetYaxis()->SetTitleOffset((Float_t)1.30);
    gr->SetLineWidth(LineWidth); gr->SetLineColor(LineColor); gr->SetLineStyle(LineStyle);
    return gr;
}

void AGraphBuilder::configure(TGraph * graph, const QString & GraphTitle,
                                      const QString & XTitle, const QString & YTitle,
                                      int MarkerColor, int MarkerStyle, int MarkerSize,
                                      int LineColor,   int LineStyle, int LineWidth)
{
    graph->SetTitle(GraphTitle.toLatin1().data());

    graph->GetXaxis()->SetTitle(XTitle.toLatin1().data());
    graph->GetYaxis()->SetTitle(YTitle.toLatin1().data());

    graph->SetMarkerColor(MarkerColor); graph->SetMarkerStyle(MarkerStyle); graph->SetMarkerSize(MarkerSize);
    graph->SetLineColor(LineColor);     graph->SetLineStyle(LineStyle);     graph->SetLineWidth(LineWidth);

    graph->SetEditable(false);
    graph->GetYaxis()->SetTitleOffset(1.30f);
}

TGraph * AGraphBuilder::graph(const std::vector<std::pair<double, double>> & data)
{
    int numEl = (int)data.size();
    TVectorD xx(numEl);
    TVectorD yy(numEl);
    for (int i=0; i < numEl; i++)
    {
        xx[i] = data[i].first;
        yy[i] = data[i].second;
    }

    TGraph * gr = new TGraph(xx,yy);
    gr->SetFillStyle(0);
    gr->SetFillColor(0);
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
    gr->SetFillStyle(0);
    gr->SetFillColor(0);
    return gr;
}
