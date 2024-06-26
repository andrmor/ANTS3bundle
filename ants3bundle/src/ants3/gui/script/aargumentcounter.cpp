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
                InFigureBracketsLevel == 0)
            {
                numArgs++;
            }
        }
    }

    return numArgs;
}

int AArgumentCounter::countArguments()
{
    QTextCursor tc = Cursor;
    tc.setPosition(FunctionEndPosition + 1, QTextCursor::MoveAnchor);

    int numArgs = 1;
    while (tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor))
    {
        const QString selected = tc.selectedText();
        if (selected.isEmpty()) continue;
        const QChar ch = selected.back();

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
            if (InBracketsLevel < 0)
            {
                // normal exit as the opening bracket was removed on start
                QString txt = tc.selectedText();
                txt.chop(1);
                txt = txt.simplified();
                //qDebug() << "------>" << txt;
                if (txt.isEmpty()) return 0;
                else return numArgs;
            }
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
                InFigureBracketsLevel == 0)
            {
                numArgs++;
            }
        }
    }

    return -1;
}

bool AArgumentCounter::moveCursorBeforeArguments(QTextCursor & tc)
{
    //int maxLines = 10;
    while (tc.position() != 0)
    {
        tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
        const QString sel = tc.selectedText();
        if (sel.isEmpty()) continue;
        const QChar ch = sel.front();
        //qDebug() << ch;

        if (ch == '\n' || ch == QChar(0x2029) )
        {
            return false;
            //if (ScriptLanguage == EScriptLanguage::Python) return false;
            // too many side cases in JS with multiline...
            //maxLines--;
            //if (maxLines == 0) return false;
        }

        if (ch == '(')
        {
            bool ok = tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
            if (!ok) return false;
            QString selection = tc.selectedText();
            if (selection.isEmpty()) return false;
            QChar chBefore = selection.front();
            if (chBefore.isLetterOrNumber()) return true;
            tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor); // back, it could be methodeName((
        }
    }
    return false;
}
