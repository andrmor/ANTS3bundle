#ifndef AGRAPHBUILDER_H
#define AGRAPHBUILDER_H

#include <QVector>

#include <vector>

class TGraph;
class TGraph2D;
class QString;

// TODO:
// refactor to methods:
// graph, title, line, marker

namespace AGraphBuilder
{
    static TGraph * graph(const QVector<double>& x, const QVector<double>& y);

    static TGraph * graph(const std::vector<float>& x, const std::vector<float>& y);

    static TGraph * graph(const QVector<double>& x, const QVector<double>& y,
                          const char *Title, const char *XTitle, const char *YTitle,
                          short MarkerColor=2, int MarkerStyle=20, int MarkerSize=1,
                          short LineColor=2,   int LineStyle=1,    int LineWidth=2);
    static TGraph * graph(const QVector<double>& x, const QVector<double>& y,
                          const QString & Title, const QString & XTitle, const QString & YTitle,
                          short MarkerColor=2, int MarkerStyle=20, int MarkerSize=1,
                          short LineColor=2,   int LineStyle=1,    int LineWidth=2);
    static TGraph * graph(const std::vector<float>& x, const std::vector<float>& y,
                          const char *Title, const char *XTitle, const char *YTitle,
                          short MarkerColor=2, int MarkerStyle=20, int MarkerSize=1,
                          short LineColor=2,   int LineStyle=1,    int LineWidth=2);

    static void configure(TGraph * graph, const QString & GraphTitle,
                          const QString & XTitle, const QString & YTitle,
                          int MarkerColor=2, int MarkerStyle=20, int MarkerSize=1,
                          int LineColor=2,   int LineStyle=1,    int LineWidth=2);

    static TGraph2D * graph(const QVector<double>& x, const QVector<double>& y, const QVector<double>& z);
    static TGraph2D * graph(const QVector<double>& x, const QVector<double>& y, const QVector<double>& z,
                            const char *Title, const char *XTitle, const char *YTitle, const char *ZTitle,
                            short MarkerColor=2, int MarkerStyle=20, int MarkerSize=1,
                            short LineColor=2,   int LineStyle=1,    int LineWidth=2);

}

#endif // AGRAPHBUILDER_H
