#include "ajsontoolsroot.h"
#include "ajsontools.h"
#include "qjsonarray.h"

#include "TH1D.h"
#include "TH2D.h"

bool jstools::parseJson(const QJsonObject & json, const QString & name, TH1D* & distr)
{
    QJsonObject js;
    if (!parseJson(json, name, js))
    {
        distr = nullptr;
        return false;
    }
    distr = jstools::jsonToRegularTh1D(js);
    return true;
}

bool jstools::parseJson(const QJsonObject & json, const QString & name, TH2D* & distr)
{
    QJsonObject js;
    if (!parseJson(json, name, js))
    {
        distr = nullptr;
        return false;
    }
    distr = jstools::jsonToRegularTh2D(js);
    return true;
}

QJsonObject jstools::regularTh1dToJson(TH1D * hist)
{
    QJsonObject json;
    if (!hist) return json;

    const int numBins = hist->GetNbinsX();
    json["Bins"] = numBins;
    json["From"] = hist->GetXaxis()->GetXmin();
    json["To"]   = hist->GetXaxis()->GetXmax();

    QJsonArray ar;
    for (int i = 0; i < numBins+2; i++)
    {
        QJsonArray el;
            el.append(hist->GetBinCenter(i));
            el.append(hist->GetBinContent(i));
        ar.append(el);
    }
    json["Data"] = ar;

    json["Entries"] = hist->GetEntries();

    return json;
}

QJsonObject jstools::regularTh2dToJson(TH2D * hist)
{
    QJsonObject json;
    if (!hist) return json;

    const int numX = hist->GetNbinsX();
    json["Xbins"] = numX;
    const int numY = hist->GetNbinsY();
    json["Ybins"] = numY;
    json["Xfrom"] = hist->GetXaxis()->GetXmin();
    json["Xto"]   = hist->GetXaxis()->GetXmax();
    json["Yfrom"] = hist->GetYaxis()->GetXmin();
    json["Yto"]   = hist->GetYaxis()->GetXmax();

    QJsonArray ar;
    for (int iX = 0; iX < numX+2; iX++)
        for (int iY = 0; iY < numY+2; iY++)
        {
            QJsonArray el;
                el.append(hist->GetXaxis()->GetBinCenter(iX));
                el.append(hist->GetYaxis()->GetBinCenter(iY));
                el.append(hist->GetBinContent(iX, iY));
            ar.append(el);
        }
    json["Data"] = ar;

    json["Entries"] = hist->GetEntries();

    return json;
}

TH1D * jstools::jsonToRegularTh1D(const QJsonObject & json)
{
    bool ok;
    int    Bins; ok = jstools::parseJson(json, "Bins", Bins); if (!ok) return nullptr;
    double From; ok = jstools::parseJson(json, "From", From); if (!ok) return nullptr;
    double To;   ok = jstools::parseJson(json, "To",   To);   if (!ok) return nullptr;

    QJsonArray ar;
    ok = jstools::parseJson(json, "Data", ar); if (!ok) return nullptr;
    if (ar.size() != Bins+2) return nullptr;

    double Entries; ok = jstools::parseJson(json, "Entries", Entries); if (!ok) return nullptr;

    TH1D * hist = new TH1D("", "", Bins, From, To);
    for (int i = 0; i < Bins+2; i++)
    {
        QJsonArray el = ar[i].toArray();
        hist->Fill(el[0].toDouble(), el[1].toDouble());
    }

    hist->BufferEmpty(1); //otherwise set entries will not have effect for histograms with small number of entries (i.e. when buffer is not full)
    hist->SetEntries(Entries);
    return hist;
}

TH2D * jstools::jsonToRegularTh2D(const QJsonObject & json)
{
    bool ok;
    int    Xbins; ok = jstools::parseJson(json, "Xbins", Xbins); if (!ok) return nullptr;
    double Xfrom; ok = jstools::parseJson(json, "Xfrom", Xfrom); if (!ok) return nullptr;
    double Xto;   ok = jstools::parseJson(json, "Xto",   Xto);   if (!ok) return nullptr;
    int    Ybins; ok = jstools::parseJson(json, "Ybins", Ybins); if (!ok) return nullptr;
    double Yfrom; ok = jstools::parseJson(json, "Yfrom", Yfrom); if (!ok) return nullptr;
    double Yto;   ok = jstools::parseJson(json, "Yto",   Yto);   if (!ok) return nullptr;

    QJsonArray ar;
    ok = jstools::parseJson(json, "Data", ar); if (!ok) return nullptr;
    if (ar.size() != (Xbins+2)*(Ybins+2)) return nullptr;

    TH2D * hist = new TH2D("", "", Xbins, Xfrom, Xto, Ybins, Yfrom, Yto);
    for (int i = 0; i < ar.size(); i++)
    {
        QJsonArray el = ar[i].toArray();
        hist->Fill(el[0].toDouble(), el[1].toDouble(), el[2].toDouble());
    }

    double Entries; ok = jstools::parseJson(json, "Entries", Entries); if (!ok) return nullptr;

    hist->BufferEmpty(1);
    hist->SetEntries(Entries);
    return hist;
}

void jstools::histToArray_lowerEdge(const TH1D * hist, std::vector<std::pair<double, double>> & content)
{
    content.clear();
    if (!hist) return;

    const int numBins = hist->GetNbinsX();
    content.reserve(numBins);

    for (int i = 1; i < numBins+1; i++)
        content.push_back({hist->GetBinLowEdge(i), hist->GetBinContent(i)});
}
