#ifndef AGRAPHBUILDER_H
#define AGRAPHBUILDER_H

#include <QString>

#include <vector>
#include <complex>

class TGraph;
class TGraph2D;
class QString;

namespace AGraphBuilder
{
    TGraph * graph(const std::vector<double> & x, const std::vector<double> & y);
    TGraph * graph(const std::vector<float>  & x, const std::vector<float>  & y);
    TGraph * graph(const std::vector<std::pair<double, double>> & data);
    TGraph * graph(const std::vector<std::pair<int, double>> & data);
    TGraph * graph(const std::vector<std::pair<double,std::complex<double>>> & data, bool real);

    void applyDefaults(TGraph * graph);

    void configure(TGraph * graph,
                   const QString & graphTitle, const QString & xTitle, const QString & yTitle,
                   int markerColor=2, int markerStyle=20, double markerSize=1.0,
                   int lineColor=2,   int lineStyle=1,    int    lineWidth=2);

    void configureTitles(TGraph * graph, const QString & graphTitle, const QString & xTitle, const QString & yTitle);
    void configureMarkers(TGraph * graph, int markerColor = 2, int markerStyle = 20, double markerSize = 1.0);
    void configureLine(TGraph * graph, int lineColor=2, int lineStyle=1, int lineWidth=2);

    void shift(TGraph * g, double multiply, double add);
    void scale(TGraph * g, double multiply, double add);
}

#endif // AGRAPHBUILDER_H
