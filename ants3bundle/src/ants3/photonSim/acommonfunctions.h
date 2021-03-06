#ifndef ACOMMONFUNCTIONS_H
#define ACOMMONFUNCTIONS_H

//#include <QVector>
//#include <QtGui/QPolygon>
#include <QList>
#include <QString>

#include <vector>
#include <map>

class TH1;
class TRandom2;

double GetRandomFromHist(TH1* hist);    //integral must be already calculated to be thread-safe
int    GetRandomBinFromHist(TH1* hist); //integral must be already calculated to be thread-safe

bool NormalizeVector(double *arr);
void NormalizeVectorSilent(double *arr); //if failed, returns (0,0,1) vector

void UpdateMax(double &max, double value); //assume positive max and value!

//next two are now in APhotSimSettings
//double GetInterpolatedValue(double val, const QVector<double> *X, const QVector<double> *F, bool LogLog = false);
//void ConvertToStandardWavelengthes(const QVector<double> *sp_x, const QVector<double> *sp_y, double WaveFrom, double WaveStep, int WaveNodes, QVector<double>* y);

//double PolyInterpolation(double x, QVector<double> *xi, QVector<double> *yi, int order);


bool ExtractNumbersFromQString(QString input, QList<int> *ToAdd);  //parse number string e.g. like that: "2, 4-8, 7, 25-65"

//QPolygonF MakeClosedPolygon(double side, double angle, double x0, double y0);

template<typename Key, typename Val>
std::vector<Key> getMapKeys(const std::map<Key, Val> &map)
{
    std::vector<Key> list;
    list.reserve(map.size());
    for(const auto &key_val : map) list.push_back(key_val.first);
    return list;
}

#endif // ACOMMONFUNCTIONS_H
