#include "aargumentcounter.h"

AArgumentCounter::AArgumentCounter(const QTextCursor & tc, int functionEndPosition, EScriptLanguage scriptLanguage) :
    Cursor(tc), FunctionEndPosition(functionEndPosition), ScriptLanguage(scriptLanguage) {}

int AArgumentCounter::getCurrentArgument()
{
    QTextCursor tc = Cursor;
    tc.setPosition(Cursor.position(), QTextCursor::MoveAnchor);
    tc.setPosition(FunctionEndPosition, QTextCursor::KeepAnchor);
    QString txt = tc.selectedText();
    if (txt.isEmpty()) return -1;

    txt.remove(0,1);
    //qDebug() << txt;

    int numArgs = 0;
    for (int iCh = 0; iCh < txt.size(); iCh++)
    {
        const QChar ch = txt[iCh];

        if (InApostrophe)
        {
            if (ch == '\'') InApostrophe = false;
            continue;
        }
        if (InQuatation)
        {
            if (ch == '\"') InQuatation  = false;
            continue;
        }

        if      (ch == '\'') InApostrophe = true;
        else if (ch == '\"') InQuatation  = true;
        else if (ch == '(')
            InBracketsLevel++;
        else if (ch == ')')
        {
            InBracketsLevel--;
            if (InBracketsLevel < 0) return -1;
        }
        else if (ch == '[')
            InSquareBracketsLevel++;
        else if (ch == ']')
        {
            InSquareBracketsLevel--;
            if (InSquareBracketsLevel < 0) return -1;
        }
        else if (ch == '{')
            InFigureBracketsLevel++;
        else if (ch == '}')
        {
            InFigureBracketsLevel--;
            if (InFigureBracketsLevel < 0) return -1;
        }
        else if (ch == ',')
        {
            if (InBracketsLevel == 0 &&
                InSquareBracketsLevel == 0 &&
                InFigureBracketsLevel == 0 &&
                !InApostrophe &&
                !InQuatation)
            {
                numArgs++;
            }
        }
    }

    return numArgs;
}
