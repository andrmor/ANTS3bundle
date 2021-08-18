#ifndef AROOTHISTAPPENDERS_H
#define AROOTHISTAPPENDERS_H

// TODO: refactor to histtools namespace, move here all hist-related from jstools and filetools

class TH1D;
class TH2D;

void appendTH1D(TH1D* & toHist, const TH1D * fromHist);
void appendTH2D(TH2D* & toHist, const TH2D * fromHist);

void appendTH1DwithStat(TH1D * & toHist, TH1D *fromHist); // !!!***

#endif // AROOTHISTAPPENDERS_H
