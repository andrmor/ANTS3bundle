#ifndef ADEMO_SI_H
#define ADEMO_SI_H

#include "ascriptinterface.h"

class ADemo_SI : public AScriptInterface
{
    Q_OBJECT

public:
    ADemo_SI();

    bool beforeRun() override;  // just as an example
    bool afterRun()  override;  // just as an example

public slots:
    bool run(int numLocalProc = -1);
};

#endif // ADEMO_SI_H
