#ifndef ATH_H
#define ATH_H

#include "TH1D.h"
#include "TH2D.h"
#include <QString>

//#include <utility> need?
//#include <cstdlib>

class ATH1D : public TH1D
{
public:
    ATH1D(const char *name, const char *title, int bins, double from, double to);
    ATH1D(const TH1D & other);

    const QString Import(double from, double to, const std::vector<double> & binContent, const std::vector<double> & stats); // empty srtring if no error

    void setStats(double * statsArray);

private:
    void SetStatistic(const std::vector<double> & stats);
};

class ATH2D : public TH2D
{
public:
    ATH2D(const char *name, const char *title, int xbins, double xfrom, double xto, int ybins, double yfrom, double yto);

    const QString Import(double xfrom, double xto, double yfrom, double yto, const std::vector<std::vector<double> > &binContent, const std::vector<double> & stats); // empty srtring if no error

private:
    void SetStatistic(const std::vector<double> & stats);
};

#endif // ATH_H
