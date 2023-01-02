#ifndef AFILEMERGER_H
#define AFILEMERGER_H

#include <QString>

#include <vector>

class AFileMerger
{
public:
    void                 clear();

    void                 add(const QString & fileName);

    void                 mergeToFile(const QString & fileName, bool binary = false) const; // returns error, empty if all is ok

    const int            BufferSize = 1000;      // initially reserved max number of character in line
    std::vector<QString> FilesToMerge;
};

#endif // AFILEMERGER_H
