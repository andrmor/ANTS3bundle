#include "ahist_si.h"
#include "ascriptobjstore.h"
#include "arootobjcollection.h"
#include "arootgraphrecord.h"
#include "aroothistrecord.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>

#include "TObject.h"
#include "TH1D.h"
#include "TH1.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TH2.h"
#include "TF2.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraph2D.h"
#include "TF1.h"
#include "TFile.h"
#include "TKey.h"

//----------------- HIST  -----------------
AHist_SI::AHist_SI()
    : TmpHub(AScriptObjStore::getInstance())
{
    Description = "CERN ROOT histograms - TH1D and TH2D";

    Help["FitGauss"] = "Fit histogram with a Gaussian. The returned result (is successful) contains an array [Constant,Mean,Sigma,ErrConstant,ErrMean,ErrSigma]"
                    "\nOptional 'options' parameter is directly forwarded to TH1::Fit()";
    Help["FitGaussWithInit"] = "Fit histogram with a Gaussian. The returned result (is successful) contains an array [Constant,Mean,Sigma,ErrConstant,ErrMean,ErrSigma]"
                            "\nInitialParValues is an array of initial parameters of the values [Constant,Mean,Sigma]"
                            "\nOptional 'options' parameter is directly forwarded to TH1::Fit()";
}

//AHist_SI::AHist_SI(const AHist_SI &other) :
//    AScriptInterface(other),
//    TmpHub(other.TmpHub) {}

void AHist_SI::create(QString HistName, int bins, double start, double stop)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw histograms!");
        return;
    }

    TH1D* hist = new TH1D("", HistName.toLatin1().data(), bins, start, stop);
    ARootHistRecord* rec = new ARootHistRecord(hist, HistName, "TH1D");

    bool bOK = TmpHub.Hists.append(HistName, rec, bAbortIfExists);
    if (!bOK)
    {
        delete rec;
        abort("Histogram " + HistName+" already exists!");
    }
    else
    {
        hist->GetYaxis()->SetTitleOffset(1.30f);
    }
}

void AHist_SI::create(QString HistName, int binsX, double startX, double stopX,  int binsY, double startY, double stopY)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw histograms!");
        return;
    }

    TH2D* hist = new TH2D("", HistName.toLatin1().data(), binsX, startX, stopX, binsY, startY, stopY);
    ARootHistRecord* rec = new ARootHistRecord(hist, HistName, "TH2D");

    bool bOK = TmpHub.Hists.append(HistName, rec, bAbortIfExists);
    if (!bOK)
    {
        delete rec;
        abort("Histogram " + HistName+" already exists!");
    }
    else
    {
        hist->GetYaxis()->SetTitleOffset(1.30f);
    }
}

void AHist_SI::create(QString HistName, int binsX,double startX,double stopX, int binsY,double startY,double stopY, int binsZ,double startZ,double stopZ)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw histograms!");
        return;
    }

    TH3D * hist = new TH3D("", HistName.toLatin1().data(), binsX, startX, stopX, binsY, startY, stopY, binsZ, startZ, stopZ);
    ARootHistRecord* rec = new ARootHistRecord(hist, HistName, "TH3D");

    bool bOK = TmpHub.Hists.append(HistName, rec, bAbortIfExists);
    if (!bOK)
    {
        delete rec;
        abort("Histogram " + HistName+" already exists!");
    }
    else
    {
        hist->GetYaxis()->SetTitleOffset(1.30f);
    }
}

void AHist_SI::SetXCustomLabels(const QString &HistName, QVariantList Labels)
{
    if (!bGuiThread)
    {
        abort("Cannot perform this operation in non-GUI thread!");
        return;
    }

    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r) abort("Histogram " + HistName + " not found!");
    else
    {
        QVector<QString> lab;
        for (int i=0; i<Labels.size(); i++)
            lab << Labels.at(i).toString();
        r->SetXLabels(lab);
    }
}

void AHist_SI::SetTitle(const QString &HistName, const QString &Title)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
        abort("Histogram " + HistName + " not found!");
    else
        r->SetTitle(Title);
}

void AHist_SI::SetTitles(const QString& HistName, QString X_Title, QString Y_Title, QString Z_Title)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
        abort("Histogram " + HistName + " not found!");
    else
        r->SetAxisTitles(X_Title, Y_Title, Z_Title);
}

void AHist_SI::SetLineProperties(const QString &HistName, int LineColor, int LineStyle, int LineWidth)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
        abort("Histogram " + HistName + " not found!");
    else
        r->SetLineProperties(LineColor, LineStyle, LineWidth);
}

void AHist_SI::SetMarkerProperties(const QString &HistName, int MarkerColor, int MarkerStyle, double MarkerSize)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
        abort("Histogram " + HistName + " not found!");
    else
        r->SetMarkerProperties(MarkerColor, MarkerStyle, MarkerSize);
}

void AHist_SI::SetFillColor(const QString &HistName, int Color)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r) abort("Histogram " + HistName + " not found!");
    else    r->SetFillColor(Color);
}

void AHist_SI::SetMaximum(const QString &HistName, double max)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r) abort("Histogram " + HistName + " not found!");
    else    r->SetMax(max);
}

void AHist_SI::SetMinimum(const QString &HistName, double min)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r) abort("Histogram " + HistName + " not found!");
    else    r->SetMin(min);
}

void AHist_SI::SetXDivisions(const QString &HistName, int primary, int secondary, int tertiary, bool canOptimize)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r) abort("Histogram " + HistName + " not found!");
    else    r->SetXDivisions(primary, secondary, tertiary, canOptimize);
}

void AHist_SI::SetYDivisions(const QString &HistName, int primary, int secondary, int tertiary, bool canOptimize)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r) abort("Histogram " + HistName + " not found!");
    else    r->SetYDivisions(primary, secondary, tertiary, canOptimize);
}

void AHist_SI::SetXLabelProperties(const QString &HistName, double size, double offset)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r) abort("Histogram " + HistName + " not found!");
    else    r->SetXLabelProperties(size, offset);
}

void AHist_SI::SetYLabelProperties(const QString &HistName, double size, double offset)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r) abort("Histogram " + HistName + " not found!");
    else    r->SetYLabelProperties(size, offset);
}

void AHist_SI::fill(QString HistName, double val, double weight)
{
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r || r->getType() != "TH1D")
        abort("1D histogram " + HistName + " not found!");
    else
        r->Fill(val, weight);
}

void AHist_SI::fill(QString HistName, double x, double y, double weight)
{
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r || r->getType() != "TH2D")
        abort("2D histogram " + HistName + " not found!");
    else
        r->Fill2D(x, y, weight);
}

void AHist_SI::fill(QString HistName, double x, double y, double z, double weight)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r || r->getType() != "TH3D")
        abort("3D histogram " + HistName + " not found!");
    else
        r->Fill3D(x, y, z, weight);
}

void AHist_SI::Smooth(const QString &HistName, int times)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
        abort("Histogram " + HistName + " not found!");
    else
    {
        r->Smooth(times);
        if (bGuiThread) emit RequestDraw(0, "", true); //to update
    }
}

void AHist_SI::Smear(const QString &HistName, double sigma)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
        abort("Histogram " + HistName + " not found!");
    else
    {
        if (r->getType() != "TH1D")
        {
            abort("Smear is implemented only for TH1D");
            return;
        }
        r->Smear(sigma);
        if (bGuiThread) emit RequestDraw(0, "", true); //to update
    }
}

void AHist_SI::ApplyMedianFilter(const QString &HistName, int span)
{
    ApplyMedianFilter(HistName, span, -1);
}

void AHist_SI::ApplyMedianFilter(const QString &HistName, int spanLeft, int spanRight)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
        abort("Histogram " + HistName + " not found!");
    else
    {
        bool bOK = r->MedianFilter(spanLeft, spanRight);
        if (!bOK) abort("Failed - Median filter is currently implemented only for 1D histograms (TH1)");
    }
}

void AHist_SI::FillArr(const QString &HistName, const QVariant XY_Array)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
        abort("Histogram " + HistName + " not found!");
    else
    {
        QVariantList VarList = XY_Array.toList();
        if (VarList.isEmpty())
        {
            abort("Array (or array of arrays) is expected as the second argument in hist.FillArr()");
            return;
        }

        QVector<double> val(VarList.size()), weight(VarList.size());
        bool bOK1, bOK2;
        for (int i=0; i<VarList.size(); i++)
        {
            QVariantList element = VarList.at(i).toList();
            switch (element.size())
            {
            case 2:
                {
                    double v = element.at(0).toDouble(&bOK1);
                    double w = element.at(1).toDouble(&bOK2);
                    if (bOK1 && bOK2)
                    {
                        val[i] = v;
                        weight[i] = w;
                        continue;  // NEXT
                    }
                    break;
                }
            case 0:
                {
                    double v = VarList.at(i).toDouble(&bOK1);
                    if (bOK1)
                    {
                        val[i] = v;
                        weight[i] = 1.0;
                        continue;  // NEXT
                    }
                    break;
                }
            default:
                break;
            }

            abort("hist.FillArr(): the second argument has to be array of values (then weight=1) or arrays of [val, weight]");
            return;
        }

        r->FillArr(val, weight);
    }
}

void AHist_SI::FillArr(const QString &HistName, const QVariantList X_Array, const QVariantList Y_Array)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));

    if (!r)
    {
        abort("Histogram " + HistName + " not found!");
        return;
    }

    const int size = X_Array.size();
    if (size == 0) return;
    if (size != Y_Array.size())
    {
        abort("Mismatch in array sizes in FillArr");
        return;
    }

    QVector<double> val, weight;
    bool bOK1, bOK2;
    for (int i=0; i < size; i++)
    {
        double v = X_Array.at(i).toDouble(&bOK1);
        double w = Y_Array.at(i).toDouble(&bOK2);
        if (!bOK1 || !bOK2)
        {
            abort("FillArr: Error in conversion to number");
            return;
        }

        val << v;
        weight << w;
        continue;
    }

    r->FillArr(val, weight);
}

void AHist_SI::Fill2DArr(const QString &HistName, const QVariant Array)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
        abort("Histogram " + HistName + " not found!");
    else
    {
        QVariantList VarList = Array.toList();
        if (VarList.isEmpty())
        {
            abort("Array of arrays is expected as the second argument in hist.Fill2DArr()");
            return;
        }

        QVector<double> xx(VarList.size()), yy(VarList.size()), weight(VarList.size());
        bool bOK1, bOK2, bOK3;
        for (int i=0; i<VarList.size(); i++)
        {
            QVariantList element = VarList.at(i).toList();
            switch (element.size())
            {
            case 2:
                {
                    double x = element.at(0).toDouble(&bOK1);
                    double y = element.at(1).toDouble(&bOK2);
                    if (bOK1 && bOK2)
                    {
                        xx[i] = x;
                        yy[i] = y;
                        weight[i] = 1.0;
                        continue;  // NEXT
                    }
                    break;
                }
            case 3:
                {
                    double x = element.at(0).toDouble(&bOK1);
                    double y = element.at(1).toDouble(&bOK2);
                    double w = element.at(2).toDouble(&bOK3);
                    if (bOK1 && bOK2 && bOK3)
                    {
                        xx[i] = x;
                        yy[i] = y;
                        weight[i] = w;
                        continue;  // NEXT
                    }
                    break;
                }
            default:
                break;
            }

            abort("hist.Fill2DArr(): the second argument has to be array of arrays: [x, y] (then weight is 1) or [x, y, weight]");
            return;
        }

        r->Fill2DArr(xx, yy, weight);
    }
}

void AHist_SI::Divide(const QString &HistName, const QString &HistToDivideWith)
{
    ARootHistRecord* r1 = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    ARootHistRecord* r2 = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistToDivideWith));
    if (!r1)
    {
        abort("Histogram " + HistName + " not found!");
        return;
    }
    if (!r2)
    {
        abort("Histogram " + HistToDivideWith + " not found!");
        return;
    }
    else
    {
        bool bOK = r1->Divide(r2);
        if (!bOK) abort("Histogram division failed: " + HistName + " by " + HistToDivideWith);
    }
}

QVariant ReturnNanArray(int num)
{
    QJsonArray ar;
    for (int i=0; i<num; i++) ar << std::numeric_limits<double>::quiet_NaN();
    QJsonValue jv = ar;
    QVariant res = jv.toVariant();
    return res;
}

const QVariant AHist_SI::FitGauss(const QString &HistName, const QString options)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
    {
        abort("Histogram " + HistName + " not found!");
        return ReturnNanArray(6);
    }
    else
    {
        const QVector<double> vec = r->FitGauss(options);
        if (vec.isEmpty()) return ReturnNanArray(6);

        if (bGuiThread) emit RequestDraw(0, "", true); //to update

        QJsonArray ar;
        for (int i=0; i<6; i++) ar << vec.at(i);

        QJsonValue jv = ar;
        QVariant res = jv.toVariant();
        return res;
    }
}

const QVariant AHist_SI::FitGaussWithInit(const QString &HistName, const QVariant InitialParValues, const QString options)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
    {
        abort("Histogram " + HistName + " not found!");
        return ReturnNanArray(6);
    }

    QString type = InitialParValues.typeName();
    if (type != "QVariantList")
    {
        abort("InitialParValues has to be an array of three numeric values");
        return ReturnNanArray(6);
    }

    QVariantList vl = InitialParValues.toList();
    QVector<double> in(3);
    if (vl.size() != 3)
    {
        abort("InitialParValues has to be an array of three numeric values");
        return ReturnNanArray(6);
    }
    bool bOK;
    for (int i=0; i<3; i++)
    {
        double d = vl.at(i).toDouble(&bOK);
        if (!bOK)
        {
            abort("InitialParValues has to be an array of three numeric values");
            return ReturnNanArray(6);
        }
        in[i] = d;
    }

    const QVector<double> vec = r->FitGaussWithInit(in, options);
    if (vec.isEmpty()) return ReturnNanArray(6);

    if (bGuiThread) emit RequestDraw(0, "", true); //to update

    QJsonArray ar;
    for (int i=0; i<6; i++) ar << vec.at(i);

    QJsonValue jv = ar;
    QVariant res = jv.toVariant();
    return res;
}

QVariantList AHist_SI::findPeaks(const QString &HistName, double sigma, double threshold)
{
    QVariantList vl;

    if (threshold <= 0 || threshold >= 1.0)
    {
        abort("hist.FindPeaks() : Threshold should be larger than 0 and less than 1.0");
        return vl;
    }
    if (sigma <= 0)
    {
        abort("hist.FindPeaks() : Sigma should be positive");
        return vl;
    }
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
    {
        abort("Histogram " + HistName + " not found!");
        return vl;
    }

    const std::vector<double> vec = r->findPeaks(sigma, threshold);
    for (const double& d : vec) vl << d;
    return vl;
}

int AHist_SI::GetNumberOfEntries(const QString &HistName)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
    {
        abort("Histogram " + HistName + " not found!");
        return 1.0;
    }
    else
        return r->GetEntries();
}

void AHist_SI::SetNumberOfEntries(const QString &HistName, int numEntries)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r) abort("Histogram " + HistName + " not found!");
    else r->SetEntries(numEntries);
}

double AHist_SI::GetIntegral(const QString &HistName, bool MultiplyByBinWidth)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
    {
        abort("Histogram " + HistName + " not found!");
        return 1.0;
    }
    else
        return r->GetIntegral(MultiplyByBinWidth);
}

double AHist_SI::GetMaximum(const QString &HistName)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
    {
        abort("Histogram " + HistName + " not found!");
        return 0;
    }

    return r->GetMaximum();
}

double AHist_SI::GetRandom(const QString &HistName)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
    {
        abort("Histogram " + HistName + " not found!");
        return 1.0;
    }

    return r->GetRandom();
}

QVariantList AHist_SI::GetRandomMultiple(const QString &HistName, int numRandoms)
{
    QVariantList vl;
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
    {
        abort("Histogram " + HistName + " not found!");
        return vl;
    }

    QVector<double> vec = r->GetRandomMultiple(numRandoms);
    for (const double & d : vec) vl.append(d);
    return vl;
}

QVariantList AHist_SI::GetStatistics(const QString & HistName)
{
    QVariantList vl;
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
        abort("Histogram " + HistName + " not found!");
    else
    {
        int num;
        std::vector<double> mean = {0, 0};
        std::vector<double> std = {0, 0};
        r->GetStatistics(num, mean, std);
        if (r->getType() == "TH1D")
            vl << num << mean[0] << std[0];
        else
        {
            vl << num;
            QVariantList m; m << mean[0] << mean[1]; vl.push_back(m);
            QVariantList s; s << std[0]  << std[1];  vl.push_back(s);
        }
    }
    return vl;
}

void AHist_SI::Scale(const QString& HistName, double ScaleIntegralTo, bool DividedByBinWidth)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
        abort("Histogram " + HistName + " not found!");
    else
        r->Scale(ScaleIntegralTo, DividedByBinWidth);
}

void AHist_SI::Save(const QString &HistName, const QString& fileName)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r) abort("Histogram " + HistName + " not found!");
    else    r->Save(fileName);
}

void AHist_SI::Load(const QString &HistName, const QString &fileName, const QString histNameInFile)
{
    if (!bGuiThread)
    {
        abort("Threads cannot load histograms!");
        return;
    }

    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (r && bAbortIfExists)
    {
        abort("Histogram " + HistName + " already exists!");
        return;
    }

    TFile* f = new TFile(fileName.toLatin1());
    if (!f)
    {
        abort("File not found or cannot be opened: " + fileName);
        return;
    }

    const int numKeys = f->GetListOfKeys()->GetEntries();
    qDebug() << "File contains" << numKeys << "TKeys";

    ARootHistRecord * rec = nullptr;
    bool bFound = false;
    for (int i=0; i<numKeys; i++)
    {
        TKey *key = (TKey*)f->GetListOfKeys()->At(i);
        QString Type = key->GetClassName();
        QString Name = key->GetName();
        qDebug() << i << Type << Name;

        if (!histNameInFile.isEmpty() && Name != histNameInFile) continue;
        bFound = true;

        if (Type == "TH1D")
        {
            TH1D * hist = (TH1D*)key->ReadObj();
            rec = new ARootHistRecord(hist, HistName, "TH1D");
            hist->GetYaxis()->SetTitleOffset(1.30f);
            break;
        }
        //else if (Type=="TProfile") p = (TProfile*)key->ReadObj();
        //else if (Type=="TProfile2D") p = (TProfile2D*)key->ReadObj();
        else if (Type == "TH2D")
        {
            TH2D * hist = (TH2D*)key->ReadObj();
            rec = new ARootHistRecord(hist, HistName, "TH2D");
            //hist->GetYaxis()->SetTitleOffset(1.30f);
            break;
        }
    }
    f->Close();
    delete f;

    if (!rec)
    {
        if (!histNameInFile.isEmpty() && !bFound)
            abort("Histogram with name " + histNameInFile + " not found in file " + fileName);
        else
            abort("Error loading histogram.\nNote that currently supported histogram types are TH1D and TH2D");
    }
    else
    {
        bool bOK = TmpHub.Hists.append(HistName, rec, false);
        if (!bOK)
        {
            delete rec;
            abort("Load histogram from file " + fileName + " failed!");
        }
    }
}

bool AHist_SI::Delete(const QString &HistName)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw histograms!");
        return false;
    }

    return TmpHub.Hists.remove(HistName);
}

void AHist_SI::DeleteAllHist()
{
    if (!bGuiThread)
        abort("Threads cannot create/delete/draw histograms!");
    else
        TmpHub.Hists.clear();
}

void AHist_SI::draw(QString HistName, QString options)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw histograms!");
        return;
    }

    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r)
        abort("Histogram " + HistName + " not found!");
    else
    {
        if (options.isEmpty())
        {
            if      (r->getType() == "TH1D") options = "hist";
            else if (r->getType() == "TH2D") options = "colz";
            else                             options = ""; // !!!*** can be better?
        }

        TObject * copy = r->GetObject()->Clone(r->GetObject()->GetName());
        emit RequestDraw(copy, options, true);
    }
}

QVariantList AHist_SI::GetContent(const QString& HistName)
{
    QVariantList vl;
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r) abort("Histogram " + HistName + " not found!");
    else
    {
        if (r->is1D())
        {
            QVector<double> x, y;
            const bool bOK = r->GetContent(x, y);
            if (bOK)
            {
                for (int i=0; i<x.size(); i++)
                {
                    QVariantList el;
                    el << x.at(i) << y.at(i);
                    vl.push_back(el);
                }
            }
        }
        else if (r->is2D())
        {
            QVector<double> x, y, z;
            const bool bOK = r->GetContent2D(x, y, z);
            if (bOK)
            {
                for (int i=0; i<x.size(); i++)
                {
                    QVariantList el;
                    el << x.at(i) << y.at(i) << z.at(i);
                    vl.push_back(el);
                }
            }
        }
        else if (r->is3D())
        {
            QVector<double> x, y, z, val;
            const bool bOK = r->GetContent3D(x, y, z, val);
            if (bOK)
            {
                for (int i=0; i<x.size(); i++)
                {
                    QVariantList el;
                    el << x.at(i) << y.at(i) << z.at(i) << val.at(i);
                    vl.push_back(el);
                }
            }
        }
        else abort("GetContent method is currently implemented only for TH1D, TH2D and TH3D histograms");
    }
    return vl;
}

double AHist_SI::GetUnderflowBin(const QString& HistName)
{
    double val = 0;
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r) abort("Histogram " + HistName + " not found!");
    else
    {
        if (!r->GetUnderflow(val)) abort("Failed to get undeflow - the method is curretly implemented only for TH1");
    }
    return val;
}

double AHist_SI::GetOverflowBin(const QString& HistName)
{
    double val = 0;
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(TmpHub.Hists.getRecord(HistName));
    if (!r) abort("Histogram " + HistName + " not found!");
    else
    {
        if (!r->GetOverflow(val)) abort("Failed to get overflow - the method is curretly implemented only for TH1");
    }
    return val;
}

