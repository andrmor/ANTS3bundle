#include "aphotsimsettings.h"

APhotSimSettings::APhotSimSettings()
{

}

int AWaveResSettings::countNodes()
{
    if (Step == 0) return 1;
    return (To - From) / Step + 1;
}
