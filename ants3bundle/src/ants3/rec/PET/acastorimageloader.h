#ifndef ACASTORIMAGELOADER_H
#define ACASTORIMAGELOADER_H

#include <QString>

#include <vector>

class ACastorImageLoader
{
public:
    ACastorImageLoader();

    QString loadImage(const QString & fileName);

    int NumBinsX = -1;
    int NumBinsY = -1;
    int NumBinsZ = -1;

    double mmPerPixelX = 0;
    double mmPerPixelY = 0;
    double mmPerPixelZ = 0;

    double OffsetX = 1e10;
    double OffsetY = 1e10;
    double OffsetZ = 1e10;

    std::vector<std::vector<std::vector<double>>> Data; // Data[ix][iy][iz]

};

#endif // ACASTORIMAGELOADER_H
