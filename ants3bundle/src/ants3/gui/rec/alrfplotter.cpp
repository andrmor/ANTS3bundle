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

QString ALrfPlotter::drawRadial(int iSens, bool showNodes)
{
    LRModel * model = ALightResponseHub::getInstance().Model;
    if (!model) return "Response model is not defined";

    if (iSens < 0 || iSens >= model->GetSensorCount()) return "Invalid sensor index";
    LRF * lrf = model->GetLRF(iSens);
    if (!lrf) return "LRF is not defined for the requested sensor";

    LRFaxial * axial = dynamic_cast<LRFaxial*>(lrf);
    if (axial)  // then just one line
    {
        TGraph * g = new TGraph(); // will be owned by the graph window
        g->SetLineWidth(2);
        g->SetLineColor(4);
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
        g->SetMinimum(0);

        emit AScriptHub::getInstance().requestDraw(g, "AL", true);

        if (showNodes)
        {
            int nodes   = axial->getNint();
            double rmax = axial->getRmax();

            double Rmin = axial->Compress(0);
            double Rmax = axial->Compress(rmax);
            double DX = Rmax - Rmin;

            // !!!*** should be delegated to the library
            int lastnode = -1;
            std::vector<double> GrX;
            for (int ix = 0; ix < 102; ix++)
            {
                double x = rmax * ix / 100.0;
                double X = axial->Compress(x);
                int node = X * nodes/DX;
                if (node > lastnode)
                {
                    GrX.push_back(x);
                    lastnode = node;
                }
            }

            TGraph * gN = new TGraph(); // will be owned by the graph window
            gN->SetMarkerStyle(8);
            gN->SetMarkerSize(1);
            gN->SetMarkerColor(4);
            gN->SetTitle( TString("LRF_nodes #") + iSens);
            gN->GetXaxis()->SetTitle("Radial distance, mm");
            gN->GetYaxis()->SetTitle("LRF_nodes");
            for (double r : GrX) gN->AddPoint(r, axial->evalAxial(r));
            emit AScriptHub::getInstance().requestDraw(gN, "Psame", true);
        }

        if (PlotData) drawDataAxial();
        return "";
    }

    LRFxy * xy = dynamic_cast<LRFxy*>(lrf);
    if (xy)
    {
        const size_t numProfiles = 36;
        const double z0 = 0;

        std::vector<std::pair<TObject*, QString>> graphs;

        double dx = xy->getXmax() - xy->getXmin();
        double dy = xy->getYmax() - xy->getYmin();
        double maxRange = std::max(dx, dy);

        double step = maxRange / NumPointsInRadialGraph;

        double x0 = model->GetX(iSens);
        double y0 = model->GetY(iSens);

        for (size_t iPr = 0; iPr < numProfiles; iPr++)
        {
            TGraph * g = new TGraph(); // will be owned by the graph window
            g->SetLineWidth(1);
            g->SetLineColor(4);
            g->GetXaxis()->SetTitle("R, mm");
            g->GetYaxis()->SetTitle("LRF");
            g->SetTitle( TString("LRF #") + iSens);

            double angle = iPr * 2 * 3.1415926535 / numProfiles;
            double val = 0;
            double radius = 0;
            do
            {
                double x = x0 + radius * cos(angle);
                double y = y0 + radius * sin(angle);
                val = xy->eval(x, y, z0);
                if (val != 0) g->AddPoint(radius, val);
                radius += step;
            }
            while (val != 0);

            g->GetXaxis()->SetRangeUser(0, g->GetXaxis()->GetXmax());
            g->SetMinimum(0);
            graphs.push_back({g, (iPr == 0 ? "AL" : "Lsame")});
        }

        emit AScriptHub::getInstance().requestDrawCollection(graphs, true);
        return "";
    }

    return "Not implemented LRF type in ALrfDrawer::drawRadial";
}

QString ALrfPlotter::drawXY(int iSens)
{
    LRModel * model = ALightResponseHub::getInstance().Model;
    if (!model) return "Response model is not defined";

    if (iSens < 0 || iSens >= model->GetSensorCount()) return "Invalid sensor index";
    LRF * lrf = model->GetLRF(iSens);
    if (!lrf) return "LRF is not defined for the requested sensor";

    TGraph2D * g = new TGraph2D(); // will be owned by the graph window
    g->SetLineWidth(1);
    g->SetLineColor(4);
    g->SetTitle( TString("LRF #") + iSens);
    g->GetXaxis()->SetTitle("X, mm");
    g->GetYaxis()->SetTitle("Y, mm");
    g->GetZaxis()->SetTitle("LRF");

    double xFrom = lrf->getXmin();
    double xTo   = lrf->getXmax();
    double xStep = (xTo - xFrom) / NumPointsInXYGraph;

    double yFrom = lrf->getYmin();
    double yTo   = lrf->getYmax();
    double yStep = (yTo - yFrom) / NumPointsInXYGraph;

    //double minVal = 0;
    //double maxVal = 0;
    for (size_t iX = 0; iX < NumPointsInXYGraph; iX++)
    {
        double x = xFrom + xStep * iX;
        for (size_t iY = 0; iY < NumPointsInXYGraph; iY++)
        {
            double y = yFrom + yStep * iY;
            double val = lrf->eval(x, y, 0);
            g->AddPoint(x, y, val);

            //if (val > maxVal) maxVal = val;
            //if (val < minVal) minVal = val;
        }
    }
    g->SetMinimum(0);

    emit AScriptHub::getInstance().requestDraw(g, "tri", true);
    return "";
}

#include "TH2D.h"
QString ALrfPlotter::drawRadial_Data(int iSens, bool addLRF)
{
    LRModel * model = ALightResponseHub::getInstance().Model;
    if (!model) return "Response model is not defined";

    if (iSens < 0 || iSens >= model->GetSensorCount()) return "Invalid sensor index";
    LRF * lrf = model->GetLRF(iSens);
    if (!lrf) return "LRF is not defined for the requested sensor";

    LRFaxial * axial = dynamic_cast<LRFaxial*>(lrf);
    if (axial)
    {
        TH2D * h = new TH2D("", "", 100, 0, 0, 100, 0, 0);

        const size_t numEvents = DataSignals.size();
        for (size_t iEv = 0; iEv < numEvents; iEv++)
        {
            const std::array<double,4> & event = DataPositions[iEv];
            const double & energy = event[3];
            const bool goodEvent = (energy > 0);
            if (!goodEvent) continue;
            //if (Options.check_z && (pos[2]<Options.z0-Options.dz || pos[2]>Options.z0+Options.dz)) continue;

            double x0 = model->GetX(iSens);
            double y0 = model->GetY(iSens);
            double r = hypot(event[0] - x0, event[1] - y0);

            double signal = DataSignals[iEv][iSens];
            //if (Options.scale_by_energy) fdata /= factor;
            //if (Options.plot_diff)
            //{
            //    double fdiff = fdata - LRFs->getLRF(fUseOldModule, PMnumber, pos);
            //    hist2D->Fill(r, fdiff);
            //}
            //else
            h->Fill(r, signal);
        }

        h->GetXaxis()->SetTitle("Radial distance, mm");
        h->GetYaxis()->SetTitle("Signal");

        emit AScriptHub::getInstance().requestDraw(h, "colz", true);
    }
    return "";
}

void ALrfPlotter::drawDataAxial()
{
    const size_t numEv = DataSignals.size();
    if (numEv == 0) return;
    if (numEv != DataPositions.size()) return;


    for (size_t iEv = 0; iEv < numEv; iEv++)
    {

    }
}
