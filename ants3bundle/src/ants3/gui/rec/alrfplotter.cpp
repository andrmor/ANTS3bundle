#include "alrfplotter.h"
#include "lrmodel.h"
#include "lrf.h"
#include "lrfaxial.h"
#include "lrfxy.h"
#include "ascripthub.h"
#include "alightresponsehub.h"

#include "TGraph.h"
#include "TGraph2D.h"
#include "TAxis.h"
#include "TH2D.h"

QString ALrfPlotter::drawRadial(int iSens, bool showLrf, bool showNodes, bool addData, bool differenceOption)
{
    LRModel * model = ALightResponseHub::getInstance().Model;
    if (!model) return "Response model is not defined";

    if (iSens < 0 || iSens >= model->GetSensorCount()) return "Invalid sensor index";
    LRF * lrf = model->GetLRF(iSens);
    if (!lrf) return "LRF is not defined for the requested sensor";

    if (dynamic_cast<LRFaxial*>(lrf))
    {
        // axial and axial3d
        if (addData)   doDrawRadialData(iSens, differenceOption);
        if (showLrf)   doDrawRadialLrf(iSens, addData);
        if (showNodes) doDrawRadialNodes(iSens);
        return "";
    }

    doDrawRadialForNonAxial(iSens);

    return "";
}

QString ALrfPlotter::drawXY(int iSens, bool showLrf, bool addData, bool differenceOption)
{
    LRModel * model = ALightResponseHub::getInstance().Model;
    if (!model) return "Response model is not defined";

    if (iSens < 0 || iSens >= model->GetSensorCount()) return "Invalid sensor index";
    LRF * lrf = model->GetLRF(iSens);
    if (!lrf) return "LRF is not defined for the requested sensor";

    if (addData)
    {
        if (differenceOption) doDrawXYDiff(iSens);
        else
        {
            doDrawXYData(iSens);
            if (showLrf) doDrawXYLrf(iSens, addData);
        }
    }
    else doDrawXYLrf(iSens, false);

    return "";
}

int ALrfPlotter::countSensors() const
{
    LRModel * model = ALightResponseHub::getInstance().Model;
    if (!model) return 0;

    return model->GetSensorCount();
}

void ALrfPlotter::doDrawRadialData(int iSens, bool differenceOption)
{
    LRModel * model = ALightResponseHub::getInstance().Model;

    LRF * lrf = model->GetLRF(iSens);
    LRFaxial * axial = dynamic_cast<LRFaxial*>(lrf);
    if (axial)
    {
        const size_t numEvents = DataSignals.size();

        double x0 = model->GetX(iSens);
        double y0 = model->GetY(iSens);

        double xFrom, xTo, yFrom, yTo;
        computeRadialDataSpan(iSens, differenceOption, xFrom, xTo, yFrom, yTo);
        if (UseFixedRange)
        {
            xFrom = RangeMin;
            xTo   = RangeMax;
        }
        if (UseFixedVertical)
        {
            yFrom = VerticalMin;
            yTo   = VerticalMax;
        }

        TH2D * h = new TH2D("", "",
                            XDataBins,       xFrom, xTo,
                            VerticalNumBins, yFrom, yTo);

        for (size_t iEv = 0; iEv < numEvents; iEv++)
        {
            const std::array<double,4> & event = DataPositions[iEv];
            const double & energy = event[3];
            const bool goodEvent = (energy > 0);
            if (!goodEvent) continue;
            //if (Options.check_z && (pos[2]<Options.z0-Options.dz || pos[2]>Options.z0+Options.dz)) continue;

            double r = hypot(event[0] - x0, event[1] - y0);

            double signal = DataSignals[iEv][iSens];
            signal /= energy;     //if (Options.scale_by_energy)
            if (differenceOption)
            {
               double diff = signal - axial->evalAxial(r);
               h->Fill(r, diff);
            }
            else h->Fill(r, signal);
        }

        h->GetXaxis()->SetTitle("Radial distance, mm");
        h->GetYaxis()->SetTitle(differenceOption ? "Difference" : "Amplitude/Energy");

        emit requestDraw(h, "colz", true, true);
    }
}

void ALrfPlotter::computeRadialDataSpan(int iSens, bool differenceOption, double & xFrom, double & xTo, double & yFrom, double & yTo)
{
    LRModel * model = ALightResponseHub::getInstance().Model;

    LRF * lrf = model->GetLRF(iSens);
    LRFaxial * axial = dynamic_cast<LRFaxial*>(lrf);

    const size_t numEvents = DataSignals.size();

    double x0 = model->GetX(iSens);
    double y0 = model->GetY(iSens);

    xFrom = xTo = 0;
    yFrom = yTo = 0;

    for (size_t iEv = 0; iEv < numEvents; iEv++)
    {
        const std::array<double,4> & event = DataPositions[iEv];
        const double & energy = event[3];
        const bool goodEvent = (energy > 0);
        if (!goodEvent) continue;
        //if (Options.check_z && (pos[2]<Options.z0-Options.dz || pos[2]>Options.z0+Options.dz)) continue;

        double r = hypot(event[0] - x0, event[1] - y0);
        if (r < xFrom) xFrom = r;
        if (r > xTo)   xTo   = r;

        double val = DataSignals[iEv][iSens];
        val /= energy;
        if (differenceOption)
            val = val - axial->evalAxial(r);

        if (val < yFrom) yFrom = val;
        if (val > yTo)   yTo   = val;
    }
}

void ALrfPlotter::doDrawRadialLrf(int iSens, bool onTopOfData)
{
    LRModel * model = ALightResponseHub::getInstance().Model;

    LRF * lrf = model->GetLRF(iSens);
    LRFaxial * axial = dynamic_cast<LRFaxial*>(lrf);
    if (axial)
    {
        TGraph * g = new TGraph(); // will be owned by the graph window
        g->SetLineWidth(2);
        g->SetLineColor(2);
        g->SetTitle( TString("LRF #") + iSens);
        g->GetXaxis()->SetTitle("Radial distance, mm");
        g->GetYaxis()->SetTitle("LRF");

        double from = axial->GetRmin();
        double to   = axial->getRmax();

        double step = (to - from) / NumPointsInRadialGraph;

        for (size_t iR = 0; iR < NumPointsInRadialGraph; iR++)
        {
            double r = step * iR;
            double val = axial->evalAxial(r);
            if (val != 0) g->AddPoint(r, val);
        }

        g->SetMinimum(UseFixedVertical ? VerticalMin : 0);
        if (UseFixedVertical && (VerticalMax > VerticalMin)) g->SetMaximum(VerticalMax);

        if (UseFixedRange && (RangeMin < RangeMax)) g->GetHistogram()->GetXaxis()->SetLimits(RangeMin, RangeMax);

        emit requestDraw(g, onTopOfData ? "Lsame" : "AL", true, true);
    }
}

void ALrfPlotter::doDrawRadialNodes(int iSens)
{
    LRModel * model = ALightResponseHub::getInstance().Model;
    LRF * lrf = model->GetLRF(iSens);
    LRFaxial * axial = dynamic_cast<LRFaxial*>(lrf);
    if (axial)
    {
        TGraph * gN = new TGraph(); // will be owned by the graph window
        gN->SetMarkerStyle(8);
        gN->SetMarkerSize(1);
        gN->SetMarkerColor(2);
        gN->SetTitle( TString("LRF_nodes #") + iSens);
        gN->GetXaxis()->SetTitle("Radial distance, mm");
        gN->GetYaxis()->SetTitle("LRF_nodes");
        const std::vector<double> GrX = axial->GetNodes();
        for (double r : GrX) gN->AddPoint(r, axial->evalAxial(r));
        emit requestDraw(gN, "Psame", true, true);
    }
}

// ---- XY ----

void ALrfPlotter::doDrawXYData(int iSens)
{
    TGraph2D * g = new TGraph2D(); // will be owned by the graph window

    const size_t numEvents = DataSignals.size();
    for (size_t iEv = 0; iEv < numEvents; iEv++)
    {
        const std::array<double,4> & event = DataPositions[iEv];
        const double & energy = event[3];
        const bool goodEvent = (energy > 0);
        if (!goodEvent) continue;
        //if (Options.check_z && (pos[2]<Options.z0-Options.dz || pos[2]>Options.z0+Options.dz)) continue;

        double signal = DataSignals[iEv][iSens];
        signal /= energy;     //if (Options.scale_by_energy)
        g->AddPoint(event[0], event[1], signal);
    }

    g->SetMinimum(UseFixedVertical ? VerticalMin : 0);
    if (UseFixedVertical) g->SetMaximum(VerticalMax);

    g->SetMarkerSize(0.5);
    g->SetMarkerStyle(20);
    g->SetMarkerColor(4);
    g->SetTitle( TString("LRF #") + iSens);
    g->GetXaxis()->SetTitle("X, mm");
    g->GetYaxis()->SetTitle("Y, mm");
    g->GetZaxis()->SetTitle("Amplitude/Energy");

    emit requestDraw(g, "p", true, true);
}

void ALrfPlotter::doDrawXYDiff(int iSens)
{
    LRModel * model = ALightResponseHub::getInstance().Model;
    LRF * lrf = model->GetLRF(iSens);

    TH2D * h  = new TH2D("", "", XDataBins, 0, 0, YDataBins, 0, 0); // will be owned by the graph window
    TH2D * h1 = new TH2D("", "", XDataBins, 0, 0, YDataBins, 0, 0); // normalization (local)
    h->SetLineColor(4);
    h->SetTitle( TString("LRF #") + iSens);
    h->GetXaxis()->SetTitle("X, mm");
    h->GetYaxis()->SetTitle("Y, mm");
    h->GetZaxis()->SetTitle("Amplitude/Energy - LRF");

    const size_t numEvents = DataSignals.size();
    for (size_t iEv = 0; iEv < numEvents; iEv++)
    {
        const std::array<double,4> & event = DataPositions[iEv];
        const double & energy = event[3];
        const bool goodEvent = (energy > 0);
        if (!goodEvent) continue;
        //if (Options.check_z && (pos[2]<Options.z0-Options.dz || pos[2]>Options.z0+Options.dz)) continue;

        double signal = DataSignals[iEv][iSens];
        signal /= energy;     //if (Options.scale_by_energy)
        double val = signal - lrf->eval(event[0], event[1], 0);
        h-> Fill(event[0], event[1], val);
        h1->Fill(event[0], event[1], 1);
    }

    h->Divide(h1);
    delete h1;

    if (UseFixedVertical)
    {
        h->SetMinimum(VerticalMin);
        h->SetMaximum(VerticalMax);
    }

    emit requestDraw(h, "colz", true, true);
}

void ALrfPlotter::doDrawXYLrf(int iSens, bool onTopOfData)
{
    LRModel * model = ALightResponseHub::getInstance().Model;
    LRF * lrf = model->GetLRF(iSens);

    TGraph2D * g = new TGraph2D(); // will be owned by the graph window

    double xFrom = lrf->getXmin();
    double xTo   = lrf->getXmax();
    double xStep = (xTo - xFrom) / NumPointsInXYGraph;

    double yFrom = lrf->getYmin();
    double yTo   = lrf->getYmax();
    double yStep = (yTo - yFrom) / NumPointsInXYGraph;

    for (size_t iX = 0; iX < NumPointsInXYGraph; iX++)
    {
        double x = xFrom + xStep * iX;
        for (size_t iY = 0; iY < NumPointsInXYGraph; iY++)
        {
            double y = yFrom + yStep * iY;
            double val = lrf->eval(x, y, 0);
            g->AddPoint(x, y, val);
        }
    }

    g->SetMinimum(UseFixedVertical ? VerticalMin : 0);
    if (UseFixedVertical) g->SetMaximum(VerticalMax);

    g->SetLineWidth(1);
    g->SetLineColor(2);
    g->SetTitle( TString("LRF #") + iSens);
    g->GetXaxis()->SetTitle("X, mm");
    g->GetYaxis()->SetTitle("Y, mm");
    g->GetZaxis()->SetTitle("LRF");

    emit requestDraw(g, (onTopOfData ? "triwsame" : "tri"), true, true);
}

void ALrfPlotter::doDrawRadialForNonAxial(int iSens)
{
    LRModel * model = ALightResponseHub::getInstance().Model;
    LRF * lrf = model->GetLRF(iSens);

    const double x0 = model->GetX(iSens);
    const double y0 = model->GetY(iSens);

    double xMax = lrf->getXmax(); double xMin = lrf->getXmin();
    double yMax = lrf->getYmax(); double yMin = lrf->getYmin();
    std::vector<std::pair<double,double>> corners = {{xMax,yMax},
                                                     {xMax,yMin},
                                                     {xMin,yMin},
                                                     {xMin,yMax}};

    double maxRadius = 0;
    for (size_t i = 0; i < 4; i++)
    {
        double dx = corners[i].first  - x0;
        double dy = corners[i].second - y0;
        double radius = sqrt(dx*dx + dy*dy);
        if (radius > maxRadius) maxRadius = radius;
    }

    double rStep = maxRadius / NumPointsInRadialGraph;

    //double z0 = ui->ledZcenter->text().toDouble();

    for (int iProf = 0; iProf < NumberRadialProfiles; iProf++)
    {
        TGraph * g = new TGraph(); // will be owned by the graph window
        g->SetLineWidth(1);
        g->SetLineColor(2);
        g->SetTitle( TString("LRF #") + iSens);
        g->GetXaxis()->SetTitle("Radial distance, mm");
        g->GetYaxis()->SetTitle("LRF");

        double angle = 2.0 * 3.1415926535 / NumberRadialProfiles * iProf;
        for (size_t iR = 0; iR < NumPointsInRadialGraph; iR++)
        {
            double radius = rStep * iR;
            double x = x0 + radius * cos(angle);
            double y = y0 + radius * sin(angle);
            if (!lrf->inDomain(x, y, 0)) break;

            double val = lrf->eval(x, y, 0);
            g->AddPoint(radius, val);
        }

        g->SetMinimum(UseFixedVertical ? VerticalMin : 0);
        if (UseFixedVertical && (VerticalMax > VerticalMin)) g->SetMaximum(VerticalMax);

        if (UseFixedRange && (RangeMin < RangeMax)) g->GetHistogram()->GetXaxis()->SetLimits(RangeMin, RangeMax);
        else                                        g->GetHistogram()->GetXaxis()->SetLimits(0, maxRadius);

        emit requestDraw(g, iProf == 0 ? "AL" : "Lsame", true, true);
    }
}
