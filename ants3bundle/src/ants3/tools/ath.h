#ifndef ATH_H
#define ATH_H

#include <array>

#include <QString>

#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"

class ATH1D : public TH1D
{
public:
    ATH1D(const char *name, const char *title, int bins, double from, double to);
    ATH1D(const TH1D & other);

    QString Import(double from, double to, const std::vector<double> & binContent, const std::vector<double> & stats); // empty string if no error

    bool mergeIdentical(const TH1D & other);

    void setStats(double * statsArray);
    void setStats(const std::array<double,5> & statsArray);

    static void merge(TH1D* & to, TH1D* const & from);

private:
    void SetStatistic(const std::vector<double> & stats);
};

class ATH2D : public TH2D
{
public:
    ATH2D(const char *name, const char *title, int xbins, double xfrom, double xto, int ybins, double yfrom, double yto);
    ATH2D(const TH2D & other);

    QString Import(double xfrom, double xto, double yfrom, double yto, const std::vector<std::vector<double> > &binContent, const std::vector<double> & stats); // empty srtring if no error

    bool mergeIdentical(const TH2D & other);

    static void merge(TH2D* & to, TH2D* const & from);

private:
    void SetStatistic(const std::vector<double> & stats);
};

class ATH3D : public TH3D
{
public:
    ATH3D(const char *name, const char *title,
          int xbins, double xfrom, double xto,
          int ybins, double yfrom, double yto,
          int zbins, double zfrom, double zto);

    void setStatistics(const std::array<double,11> & stats);
};

#endif // ATH_H
