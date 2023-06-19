#include "ainspector.h"

#include <QDebug>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        qDebug() << "The 1st argument should be the dir for file exchange and the 2nd file name (no dir, should be in the exchange dir)";
        exit(1);
    }

    AInspector inspector(argv[1], argv[2]);
    inspector.start();

    return 0;
}
