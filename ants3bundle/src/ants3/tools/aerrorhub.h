#ifndef AERRORHUB_H
#define AERRORHUB_H

#include <string>

class AErrorHub
{
public:
    static void clear();
    static void addError(const std::string & ErrorLine);
    static bool isError();
    static const std::string & getError();

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
