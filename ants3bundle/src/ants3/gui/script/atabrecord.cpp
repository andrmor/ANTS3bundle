#include "atabrecord.h"
#include "atextedit.h"
#include "ahighlighters.h"
#include "ajsontools.h"

#include <QCompleter>
#include <QStringList>
#include <QStringListModel>
#include <QMenu>
#include <QAction>
#include <QFileInfo>

ATabRecord::ATabRecord(const QStringList & functions, EScriptLanguage language) :
    Functions(functions)
{
    TextEdit = new ATextEdit(language);
    TextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

    Completer = new QCompleter(this);
    CompletitionModel = new QStringListModel(functions, this);
    Completer->setModel(CompletitionModel);
    Completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    //completer->setCompletionMode(QCompleter::PopupCompletion);
    Completer->setFilterMode(Qt::MatchContains);
    //completer->setFilterMode(Qt::MatchStartsWith);
    Completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    Completer->setCaseSensitivity(Qt::CaseSensitive);
    Completer->setWrapAround(false);
    TextEdit->setCompleter(Completer);

    if (language == EScriptLanguage::JavaScript)
        Highlighter = new AHighlighterJS(TextEdit->document());
    else
        Highlighter = new AHighlighterPython(TextEdit->document());

    TextEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(TextEdit, &ATextEdit::customContextMenuRequested, this, &ATabRecord::onCustomContextMenuRequested);
    connect(TextEdit, &ATextEdit::lineNumberChanged,          this, &ATabRecord::onLineNumberChanged);
    connect(TextEdit, &ATextEdit::textChanged,                this, &ATabRecord::onTextChanged);
}

ATabRecord::~ATabRecord()
{
    delete TextEdit;
}

void ATabRecord::updateHighlight()
{
    bool bWasMod = wasModified();
    Highlighter->rehighlight();
    setModifiedStatus(bWasMod);
}

void ATabRecord::writeToJson(QJsonObject & json) const
{
    json["FileName"]         = FileName;
    json["TabName"]          = TabName;
    json["bExplicitlyNamed"] = bExplicitlyNamed;
    json["Script"]           = (TextEdit ? TextEdit->document()->toPlainText() : "");
}

void ATabRecord::readFromJson(const QJsonObject & json)
{
    if (!TextEdit) return;

    jstools::parseJson(json, "FileName", FileName);
    jstools::parseJson(json, "TabName", TabName);
    jstools::parseJson(json, "bExplicitlyNamed", bExplicitlyNamed);

    QString Script = json["Script"].toString();
    TextEdit->clear();
    TextEdit->appendPlainText(Script);
    TextEdit->document()->clearUndoRedoStacks();

    bExplicitlyNamed = false; // !!!*** why it is forced here?
}

/*
QTextEdit holds a QTextDocument object which can be retrieved using the document() method.
You can also set your own document object using setDocument(). QTextDocument emits a textChanged() signal
 if the text changes and it also provides a isModified() function which will return true if the text has been modified
 since it was either loaded or since the last call to setModified with false as argument.
 In addition it provides methods for undo and redo.
*/

bool ATabRecord::wasModified() const
{
    return TextEdit->document()->isModified();
}

void ATabRecord::setModifiedStatus(bool flag)
{
    TextEdit->document()->setModified(flag);
}

void ATabRecord::goBack()
{
    if (IndexVisitedLines >= 1 && IndexVisitedLines < VisitedLines.size())
    {
        IndexVisitedLines--;
        int goTo = VisitedLines.at(IndexVisitedLines);
        QTextCursor tc = TextEdit->textCursor();
        int nowAt = tc.blockNumber();
        if (nowAt == goTo) return;
        else if (nowAt < goTo) tc.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, goTo-nowAt );
        else if (nowAt > goTo) tc.movePosition(QTextCursor::Up,   QTextCursor::MoveAnchor, nowAt-goTo );
        TextEdit->setTextCursorSilently(tc);
        TextEdit->ensureCursorVisible();
    }
}

void ATabRecord::goForward()
{
    if (IndexVisitedLines >= 0 && IndexVisitedLines < VisitedLines.size()-1)
    {
        IndexVisitedLines++;
        int goTo = VisitedLines.at(IndexVisitedLines);
        QTextCursor tc = TextEdit->textCursor();
        int nowAt = tc.blockNumber();
        if (nowAt == goTo) return;
        else if (nowAt < goTo) tc.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, goTo-nowAt );
        else if (nowAt > goTo) tc.movePosition(QTextCursor::Up,   QTextCursor::MoveAnchor, nowAt-goTo );
        TextEdit->setTextCursorSilently(tc);
        TextEdit->ensureCursorVisible();
    }
}

void ATabRecord::onCustomContextMenuRequested(const QPoint& pos)
{
    QMenu menu;

    QAction* paste = menu.addAction("Paste"); paste->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_V));
    QAction* copy  = menu.addAction("Copy");   copy->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_C));
    QAction* cut   = menu.addAction("Cut");     cut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_X));
    menu.addSeparator();
    QAction* findSel =    menu.addAction("Find selected text");      findSel->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));
    QAction* replaceSel = menu.addAction("Replace selected text");replaceSel->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    menu.addSeparator();
    QAction* findFunct = menu.addAction("Find function definition");      findFunct->setShortcut(QKeySequence(Qt::Key_F2));
    QAction* findVar =   menu.addAction("Find variable definition (F3)");   findVar->setShortcut(QKeySequence(Qt::Key_F3));
    menu.addSeparator();
    QAction* shiftBack = menu.addAction("Go back");          shiftBack->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Left));
    QAction* shiftForward = menu.addAction("Go forward"); shiftForward->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Right));
    menu.addSeparator();
    QAction* alignText = menu.addAction("Align selected text"); alignText->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));

    QAction* selectedItem = menu.exec(TextEdit->mapToGlobal(pos));
    if (!selectedItem) return; //nothing was selected

    if (selectedItem == findSel)         emit requestFindText();
    if (selectedItem == replaceSel)      emit requestReplaceText();
    else if (selectedItem == findFunct)  emit requestFindFunction();
    else if (selectedItem == findVar)    emit requestFindVariable();
    else if (selectedItem == replaceSel) emit requestReplaceText();

    else if (selectedItem == shiftBack)    goBack();
    else if (selectedItem == shiftForward) goForward();

    else if (selectedItem == alignText) TextEdit->align();
    else if (selectedItem == cut)       TextEdit->cut();
    else if (selectedItem == copy)      TextEdit->copy();
    else if (selectedItem == paste)     TextEdit->paste();
}

void ATabRecord::onLineNumberChanged(int lineNumber)
{
    if (!VisitedLines.isEmpty())
        if (VisitedLines.last() == lineNumber) return;

    VisitedLines.append(lineNumber);
    if (VisitedLines.size() > MaxLineNumbers) VisitedLines.removeFirst();

    IndexVisitedLines = VisitedLines.size() - 1;
}

void ATabRecord::onTextChanged()
{
    //qDebug() << "Text changed!";
    QTextDocument* d = TextEdit->document();
    QRegularExpression re("(?<=var)\\s+\\w+\\b");

    QStringList Variables;
    QTextCursor tc = d->find(re, 0);//, flags);
    while (!tc.isNull())
    {
        Variables << tc.selectedText().trimmed();
        tc = d->find(re, tc);//, flags);
    }

    Variables.append(Functions);
    CompletitionModel->setStringList(Variables);
}
