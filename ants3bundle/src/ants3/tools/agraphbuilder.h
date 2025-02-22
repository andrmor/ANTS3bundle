#ifndef AGRAPHBUILDER_H
#define AGRAPHBUILDER_H

#include <QVector>

#include <vector>
#include <complex>

class TGraph;
class TGraph2D;
class QString;

// TODO:
// refactor to methods:
// graph, title, line, marker

namespace AGraphBuilder
{
    TGraph * graph(const QVector<double>& x, const QVector<double>& y);
    TGraph * graph(const std::vector<double> & x, const std::vector<double> & y);
    TGraph * graph(const std::vector<float>& x, const std::vector<float>& y);
    TGraph * graph(const std::vector<std::pair<double, double>> & data);
    TGraph * graph(const std::vector<std::pair<int, double>> & data);
    TGraph * graph(const std::vector<std::pair<double,std::complex<double>>> & data, bool real);

    TGraph * graph(const QVector<double>& x, const QVector<double>& y,
                          const char *Title, const char *XTitle, const char *YTitle,
                          short MarkerColor=2, int MarkerStyle=20, int MarkerSize=1,
                          short LineColor=2,   int LineStyle=1,    int LineWidth=2);
    TGraph * graph(const QVector<double>& x, const QVector<double>& y,
                          const QString & Title, const QString & XTitle, const QString & YTitle,
                          short MarkerColor=2, int MarkerStyle=20, int MarkerSize=1,
                          short LineColor=2,   int LineStyle=1,    int LineWidth=2);
    TGraph * graph(const std::vector<float>& x, const std::vector<float>& y,
                          const char *Title, const char *XTitle, const char *YTitle,
                          short MarkerColor=2, int MarkerStyle=20, int MarkerSize=1,
                          short LineColor=2,   int LineStyle=1,    int LineWidth=2);

    void shift(TGraph * g, double multiply, double add);
    void scale(TGraph * g, double multiply, double add);

    void configure(TGraph * graph,
                   const QString & GraphTitle, const QString & XTitle, const QString & YTitle,
                   int MarkerColor=2, int MarkerStyle=20, double MarkerSize=1.0,
                   int LineColor=2,   int LineStyle=1,    int LineWidth=2);

    TGraph2D * graph(const QVector<double>& x, const QVector<double>& y, const QVector<double>& z);
    TGraph2D * graph(const QVector<double>& x, const QVector<double>& y, const QVector<double>& z,
                            const char *Title, const char *XTitle, const char *YTitle, const char *ZTitle,
                            short MarkerColor=2, int MarkerStyle=20, int MarkerSize=1,
                            short LineColor=2,   int LineStyle=1,    int LineWidth=2);

}

#endif // AGRAPHBUILDER_H
