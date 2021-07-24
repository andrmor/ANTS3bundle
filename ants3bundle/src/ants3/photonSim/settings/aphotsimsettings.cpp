#include "aphotsimsettings.h"

#include <cmath>

APhotSimSettings::APhotSimSettings()
{

}

int AWaveResSettings::countNodes() const
{
    if (Step == 0) return 1;
    return (To - From) / Step + 1;
}

double AWaveResSettings::getWavelength(int index) const
{
    return From + Step * index;
}

int AWaveResSettings::getIndex(double wavelength) const
{
    if (!Enabled) return -1;

    int iwave = round( (wavelength - From) / Step );

    if (iwave < 0) iwave = 0;
    const int numNodes = countNodes();
    if (iwave >= numNodes) iwave = numNodes - 1;
    return iwave;
}
