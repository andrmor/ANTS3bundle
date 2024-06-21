#ifndef AARGUMENTCOUNTER_H
#define AARGUMENTCOUNTER_H

#include "escriptlanguage.h"

#include <QTextCursor>

class AArgumentCounter
{
public:
    AArgumentCounter(const QTextCursor & tc, int functionEndPosition, EScriptLanguage scriptLanguage);

    int getCurrentArgument();

    const QTextCursor Cursor;
    int FunctionEndPosition = 0;
    EScriptLanguage ScriptLanguage = EScriptLanguage::JavaScript;

protected:
    int  InBracketsLevel = 0;
    int  InSquareBracketsLevel = 0;
    int  InFigureBracketsLevel = 0;

    bool InApostrophe = false;
    bool InQuatation  = false;
};

#endif // AARGUMENTCOUNTER_H
