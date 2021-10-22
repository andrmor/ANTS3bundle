#ifndef AERRORHUB_H
#define AERRORHUB_H

#include <string>

#ifdef QT
    #include <QString>
#endif
class AErrorHub
{
public:
    static void clear();

    static void addError(const std::string & ErrorLine);
#ifdef QT
    static void addQError(const QString & ErrorLine);
#endif

    static bool isError();

    static const std::string & getError();
 #ifdef QT
    static QString getQError();
#endif

private:
    AErrorHub(){}
    ~AErrorHub(){}

    static AErrorHub & getInstance();

    AErrorHub(const AErrorHub&)            = delete;
    AErrorHub(AErrorHub&&)                 = delete;
    AErrorHub& operator=(const AErrorHub&) = delete;
    AErrorHub& operator=(AErrorHub&&)      = delete;

    std::string Error;
};

#endif // AERRORHUB_H
