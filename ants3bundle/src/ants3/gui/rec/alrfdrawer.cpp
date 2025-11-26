#include "alrfdrawer.h"
#include "lrmodel.h"
#include "lrf.h"
#include "lrfaxial.h"
#include "lrfxy.h"
#include "ascripthub.h"

#include "TGraph.h"
#include "TGraph2D.h"
#include "TAxis.h"

ALrfDrawer::ALrfDrawer(LRModel * model) :
    Model(model) {}

QString ALrfDrawer::drawRadial(int iSens, bool showNodes)
{
    if (!Model) return "Light response model is not defined";

    if (iSens < 0 || iSens >= Model->GetSensorCount()) return "Invalid sensor index";
    LRF * lrf = Model->GetLRF(iSens);
    if (!lrf) return "LRF is not defined for the requested sensor";

    TGraph * g = new TGraph(); // will be owned by the graph window
    g->SetLineWidth(2);
    g->SetLineColor(4);
    g->SetTitle( TString("LRF #") + iSens);
    g->GetXaxis()->SetTitle("Radial distance, mm");
    g->GetYaxis()->SetTitle("LRF");

    LRFaxial * axial = dynamic_cast<LRFaxial*>(lrf);
    if (axial)
    {
        double from = axial->GetRmin();
        double to   = axial->getRmax();

        double step = (to - from) / NumPointsInRadialGraph;

        double minVal = 0;
        double maxVal = 0;
        for (size_t iR = 0; iR < NumPointsInRadialGraph; iR++)
        {
            double r = step * iR;
            double val = axial->evalAxial(r);
            g->AddPoint(r, val);

            if (val > maxVal) maxVal = val;
            if (val < minVal) minVal = val;
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
    }

    return "";
}

QString ALrfDrawer::drawXY(int iSens, bool showNodes)
{
    if (!Model) return "Light response model is not defined";

    if (iSens < 0 || iSens >= Model->GetSensorCount()) return "Invalid sensor index";
    LRF * lrf = Model->GetLRF(iSens);
    if (!lrf) return "LRF is not defined for the requested sensor";

    TGraph2D * g = new TGraph2D(); // will be owned by the graph window
    g->SetLineWidth(1);
    g->SetLineColor(4);
    g->SetTitle( TString("LRF #") + iSens);
    g->GetXaxis()->SetTitle("X, mm");
    g->GetYaxis()->SetTitle("Y, mm");
    g->GetZaxis()->SetTitle("LRF");

    //LRFxy * xy = dynamic_cast<LRFxy*>(lrf);
    if (true)
    {
        double xFrom = lrf->getXmin();
        double xTo   = lrf->getXmax();
        double xStep = (xTo - xFrom) / NumPointsInXYGraph;

        double yFrom = lrf->getYmin();
        double yTo   = lrf->getYmax();
        double yStep = (yTo - yFrom) / NumPointsInXYGraph;

        double minVal = 0;
        double maxVal = 0;
        for (size_t iX = 0; iX < NumPointsInXYGraph; iX++)
        {
            double x = xFrom + xStep * iX;
            for (size_t iY = 0; iY < NumPointsInXYGraph; iY++)
            {
                double y = yFrom + yStep * iY;
                double val = lrf->eval(x, y, 0);
                g->AddPoint(x, y, val);

               if (val > maxVal) maxVal = val;
               if (val < minVal) minVal = val;
            }
        }
        g->SetMinimum(0);

        emit AScriptHub::getInstance().requestDraw(g, "tri", true);

        /*
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
        */
    }

    return "";
}
