#include "atextedit.h"
#include "guitools.h"
#include "a3global.h"
#include "aargumentcounter.h"

#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QDebug>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>
#include <QToolTip>
#include <QWidget>
#include <QTextEdit>
#include <QPainter>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include <QClipboard>
#include <QRegularExpression>

ATextEdit::ATextEdit(EScriptLanguage lang, QLabel * labelHelpTooltip, QWidget * parent) :
    QPlainTextEdit(parent),
    ScriptLanguage(lang), lHelp(labelHelpTooltip), TabInSpaces(A3Global::getInstance().TabInSpaces)
{
    LeftField = new ALeftField(*this);
    connect(this, &ATextEdit::blockCountChanged, this, &ATextEdit::updateLineNumberAreaWidth);
    connect(this, &ATextEdit::updateRequest, this, &ATextEdit::updateLineNumberArea);
    updateLineNumberAreaWidth();

    connect(this, &ATextEdit::cursorPositionChanged, this, &ATextEdit::onCursorPositionChanged);

    setMouseTracking(true);
}

void ATextEdit::setCompleter(QCompleter *completer)
{
    if (Completer) QObject::disconnect(Completer, 0, this, 0);
    Completer = completer;
    if (!Completer) return;

    Completer->setWidget(this);
    Completer->setCompletionMode(QCompleter::PopupCompletion);
    Completer->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(Completer, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
}

void ATextEdit::showMethodHelpForCursor()
{
    //QString text = selectObjFunctUnderCursor();
    //emit requestHelp(text);

    std::vector<std::pair<QString,int>> matchingMethods;
    bool cursorIsInArguments = false;
    int functionEndPosition = findMathcingMethodsForCursor(textCursor(), matchingMethods, cursorIsInArguments);
    if (matchingMethods.empty()) return;

    std::sort(matchingMethods.begin(), matchingMethods.end(), [](const auto & lhs, const auto & rhs){return (lhs.second < rhs.second);});
    int selectedMethod = 0;
    if (ForcedMethodTooltipSelection)
        selectedMethod = SelectedMethodInTooltip;
    else
    {
        int numNow = computeIntroducedNumberOfArguments(textCursor(), functionEndPosition);
        //qDebug() << "Detected num arguments:" << numNow;
        selectedMethod = 0;
        for (const auto & pair : matchingMethods)
        {
            if (numNow == pair.second) break;
            selectedMethod++;
        }
    }

    if (matchingMethods.front().first.startsWith("void core.print(")) selectedMethod = 0;

    if (selectedMethod >= matchingMethods.size())
    {
        qDebug() << "On F1 pressed: Bad method index!";
        selectedMethod = 0;
    }
    emit requestHelpWithArgs(matchingMethods[selectedMethod]);
}

#include "afiletools.h"
bool ATextEdit::saveTextToFile(const QString & fileName) const
{
     return ftools::saveTextToFile(document()->toPlainText(), fileName);
}

void ATextEdit::keyPressEvent(QKeyEvent * e)
{
    const int key = e->key();

    if (key == Qt::Key_Shift) return;

    const bool controlPressed   = ( (e->modifiers() & Qt::ControlModifier) );
    const bool altPressed       = ( (e->modifiers() & Qt::AltModifier) );
    const bool shiftPressed     = ( (e->modifiers() & Qt::ShiftModifier) );
    const bool completerVisible = (Completer && Completer->popup()->isVisible());
    // !!!*** introduce these flags below

    if (controlPressed && altPressed)
    {
        bool ok = onKeyPressed_interceptShortcut(key, shiftPressed);
        if (ok) return;
    }

    Pressed_2 = false;

    QTextCursor tc = textCursor();

    switch (key)
    {
    case Qt::Key_V :
      {
        if (controlPressed)
        {
            paste();
            return;
        }
        break;
      }
    case Qt::Key_Tab :
      {
        if (e->modifiers() == 0 && !completerVisible)
        {
            int posInBlock = tc.positionInBlock();
            int timesInsert = TabInSpaces - posInBlock % TabInSpaces;
            if (timesInsert == 0) timesInsert += TabInSpaces;
            insertPlainText(QString(" ").repeated(timesInsert));
            return;
        }
        break;
      }
    case Qt::Key_Backspace :
      {
        if (shiftPressed)
        {
            int posInBlock = tc.positionInBlock();
            int timesDelete = posInBlock % TabInSpaces;
            if (timesDelete == 0) timesDelete = TabInSpaces;
            if (timesDelete <= posInBlock)
            {
                tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, timesDelete);
                QString s = tc.selectedText().simplified();
                if (s.isEmpty()) tc.removeSelectedText();
            }
            return;
        }
        break;
      }
    case Qt::Key_Escape :
      {
        //QToolTip::hideText();
        if (lHelp) lHelp->hide();
        break;
      }
    case Qt::Key_F1 :
      {
        showMethodHelpForCursor();
        return;
      }
    case Qt::Key_Delete :
      {
        if (shiftPressed)                               // Delete line
        {
            tc.select(QTextCursor::LineUnderCursor);
            tc.removeSelectedText();
            tc.deleteChar();
            onCursorPositionChanged();
            return;
        }
        QPlainTextEdit::keyPressEvent(e);
        onCursorPositionChanged();
        return;
      }
    case Qt::Key_Down :
      {
        if (controlPressed && altPressed)               // Copy line
        {
            tc.select(QTextCursor::LineUnderCursor);
            QString line = tc.selectedText();
            tc.movePosition(QTextCursor::EndOfLine);
            setTextCursor(tc);
            insertPlainText("\n" + line);
            onCursorPositionChanged();
            return;
        }

        if ( controlPressed && shiftPressed)            // Shift line down
        {
            tc.select(QTextCursor::LineUnderCursor);
            QString line = tc.selectedText();
            tc.removeSelectedText();
            tc.deleteChar();
            setTextCursor(tc);
            tc = textCursor();
            tc.movePosition(QTextCursor::Down);
            setTextCursor(tc);
            insertPlainText(line + "\n");
            tc.movePosition(QTextCursor::Up);
            setTextCursor(tc);
            onCursorPositionChanged();
            return;
        }

        if ( MethodTooltipVisible && shiftPressed)      // Choose another signarure in help tooltip
        {
            if (SelectedMethodInTooltip == 0) SelectedMethodInTooltip = NumberOfMethodsInTooltip - 1;
            else SelectedMethodInTooltip--;
            ForcedMethodTooltipSelection = true;
            tryShowFunctionTooltip(tc);
            return;
        }
        break;
      }
    case Qt::Key_Up :
      {
        if (controlPressed && shiftPressed)             // Shift line up
        {
            tc.select(QTextCursor::LineUnderCursor);
            QString line = tc.selectedText();
            tc.removeSelectedText();
            tc.deleteChar();
            setTextCursor(tc);
            tc = textCursor();
            tc.movePosition(QTextCursor::Up);
            setTextCursor(tc);
            insertPlainText(line + "\n");
            tc.movePosition(QTextCursor::Up);
            setTextCursor(tc);
            onCursorPositionChanged();
            return;
        }

        if (MethodTooltipVisible && shiftPressed)       // Choose another signarure in help tooltip
        {
            SelectedMethodInTooltip++;
            if (SelectedMethodInTooltip >= NumberOfMethodsInTooltip) SelectedMethodInTooltip = 0;
            ForcedMethodTooltipSelection = true;
            tryShowFunctionTooltip(tc);
            return;
        }
        break;
      }
    case Qt::Key_Plus :
      {
        if (controlPressed)                             // font size +
        {
            int size = font().pointSize();
            setFontSizeAndEmitSignal(++size);
            return;
        }
        break;
      }
    case Qt::Key_Minus :
      {
        if (controlPressed)                             // font size -
        {
            int size = font().pointSize();
            setFontSizeAndEmitSignal(--size); //check is there: cannot go < 1
            return;
        }
        break;
      }
    case Qt::Key_BraceRight :
      { // } as only one char in the line
        tc.select(QTextCursor::LineUnderCursor);
        QString line = tc.selectedText();
        if (line.simplified().isEmpty())
        {
            QTextCursor tc1 = textCursor();
            tc1.movePosition(QTextCursor::Up);
            tc1.movePosition(QTextCursor::EndOfLine);

            int iLevel = 0;
            while (!tc1.atStart())
            {
                tc1.clearSelection();
                tc1.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
                const QString ch = tc1.selectedText();
                if (ch == "}")
                {
                    iLevel++;
                    continue;
                }
                if (ch != "{") continue;
                iLevel--;
                if (iLevel < 0)
                {
                    tc1.select(QTextCursor::LineUnderCursor);
                    QString start = tc1.selectedText();
                    if (start.simplified() == "{")
                    {
                        int indent = getIndent(start);
                        tc.insertText(QString(" ").repeated(indent) + "}");
                        setTextCursor(tc);
                        return;
                    }
                    break;
                }
            }
        }
        break;
      }
    default:
        break;
    }

    /*  BUGS!
    if (e->key() == Qt::Key_Delete && (e->modifiers()==0))
    {
        QTextCursor tc = this->textCursor();
        int pos = tc.position();
        tc.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
        if (tc.selectedText().simplified().isEmpty())
        {
            //tc.insertText("");
            tc.removeSelectedText();
            tc.movePosition(QTextCursor::Down);
            tc.movePosition(QTextCursor::StartOfLine);
            tc.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
            tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            QString s = tc.selectedText().trimmed();
            tc.removeSelectedText();

            tc.setPosition(pos);
            tc.insertText(s);
            tc.setPosition(pos);
            setTextCursor(tc);
            return;
        }
        else
        {
            QPlainTextEdit::keyPressEvent(e);
            return;
        }
    }
    */

    if (key == Qt::Key_Return  && !(Completer && Completer->popup()->isVisible()))
    { //enter is pressed but completer popup is not visible
        tc.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
        QString onRight = tc.selectedText();

        //leading spaces?
        tc.select(QTextCursor::LineUnderCursor);
        QString line = tc.selectedText();
        int startingSpaces = 0;
        for (int i=0; i<line.size(); i++)
          {
            if (line.at(i) != QChar::Space) break;
            else startingSpaces++;
          }
        QString spacer;
        spacer.fill(QChar::Space, startingSpaces);  //leading spaces

        if (onRight.simplified() == "")
        {
            QString insert = "\n";
            tc = textCursor();
            tc.movePosition(QTextCursor::EndOfLine);
            tc.select(QTextCursor::WordUnderCursor);

            bool fUp = false;

            //auto-insert "}"
            if (SelectTextToLeft(this->textCursor(), 1) == "{")
            {
                //do it only if this last "{" is inside closed brackets section (or no brackets)
                if (InsertClosingBracket())
                {
                    insert += spacer + QString(" ").repeated(TabInSpaces) + "\n" + spacer + "}";
                    fUp = true;
                }
            }

            //auto-insert "{" and "}"//
//            QString lastWord = tc.selectedText();
//            if ( line.simplified().endsWith("{") )
//              {
//                 //if the next curly bracket is "{" (or do not found), add "}"
//                 QTextCursor tmpc = this->textCursor();
//                 bool fDoIt = true;
//                 do
//                   {
//                     tmpc.clearSelection();
//                     tmpc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
//                     QString st = tmpc.selectedText();
//                     if (st == "}")
//                       {
//                         fDoIt = false;
//                         break;
//                       }
//                     if (st == "{") break; //still true
//                   }
//                 while (!tmpc.atEnd());
//                 if (fDoIt)
//                   {
//                     insert += spacer+"    \n"+
//                               spacer+"}";
//                     fUp = true;
//                   }
//                 else insert += spacer+"  ";
//              }

//            else if (line.simplified().startsWith("for") || lastWord == "while" || lastWord == "do")
//              {
//                 insert += spacer +"{\n"+
//                           spacer+"   \n"+
//                           spacer+"}";
//                 fUp = true;
//              }
//            else
            insert += spacer;

            tc.movePosition(QTextCursor::EndOfLine);
            tc.insertText(insert);
            if (fUp)
            {
                tc.movePosition(QTextCursor::Up);
                tc.movePosition(QTextCursor::EndOfLine);
            }
            setTextCursor(tc);
            return;
        }
        else
        {
            tc = textCursor();
            tc.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
            tc.insertText("\n" + spacer + onRight);
            tc.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
            tc.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, startingSpaces);
            setTextCursor(tc);
            return;
        }
    }

    // ----- completer -----
    if (Completer && Completer->popup()->isVisible())
    {
        // The following keys are forwarded by the completer to the widget
        switch (key)
        {
        case Qt::Key_Tab:
          {
            //qDebug() << "Tab pressed when completer is active";
            //QString startsWith = c->completionPrefix();
            int i = 0;
            QAbstractItemModel * m = Completer->completionModel();
            QStringList sl;
            while (m->hasIndex(i, 0)) sl << m->data(m->index(i++, 0)).toString();
            if (sl.size() < 2)
            {
                e->ignore(); // let the completer do default behavior
                return;
            }
            QString root = sl.first();
            for (int isl=1; isl<sl.size(); isl++)
            {
                const QString & item = sl.at(isl);
                if (root.length() > item.length())
                    root.truncate(item.length());
                for (int i = 0; i < root.length(); ++i)
                {
                    if (root[i] != item[i])
                    {
                        root.truncate(i);
                        break;
                    }
                }
            }
            //qDebug() << root;
            if (root.isEmpty())
            {
                //do nothing
            }
            else
            {
                insertCompletion(root);
                Completer->popup()->setCurrentIndex(Completer->completionModel()->index(0, 0));
            }
            return;
          }
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Backtab:
            e->ignore(); // let the completer do default behavior
            return;
        default:
            break;
        }
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && key == Qt::Key_E); // CTRL+E
    if (!Completer || !isShortcut) // do not process the shortcut when we have a completer
        QPlainTextEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!Completer || (ctrlOrShift && e->text().isEmpty()))
        return;

    //static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    static QString eow("~!@#$%^&*()+{}|:\"<>?,/;'[]\\-="); // end of word  //no dot
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (completionPrefix == "else")
    {
        Completer->popup()->hide();
        return;
    }

    //qDebug() <<completionPrefix<< e->text().right(1)<< "hasModifier:"<<hasModifier << (e->modifiers() != Qt::NoModifier) << !ctrlOrShift;
    if (e->text().right(1) == "." || e->text().right(1) == "_") hasModifier = false; //my fix for dot

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 3 || eow.contains(e->text().right(1))))
    {
        //qDebug() << "Hiding!";
        Completer->popup()->hide();
        return;
    }

    if (completionPrefix != Completer->completionPrefix())
    {
        Completer->setCompletionPrefix(completionPrefix);
        Completer->popup()->setCurrentIndex(Completer->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(Completer->popup()->sizeHintForColumn(0)
                + Completer->popup()->verticalScrollBar()->sizeHint().width());
    Completer->complete(cr); // popup it up!
}

bool ATextEdit::onKeyPressed_interceptShortcut(int key, bool shift)
{
    //qDebug() << key << shift;
    QString text;
    switch (key)
    {
    case Qt::Key_2 :
        Pressed_2 = true;
        return true;
    case Qt::Key_QuoteDbl :
        Pressed_2 = true;
        return true;
    case Qt::Key_F :
        if (ScriptLanguage == EScriptLanguage::JavaScript)
        {
            text = "for (var ii = 0; ii < iiMax; ii++)\n"
                   "{\n"
                   "\n"
                   "}";
        }
        else text = "for ii in range(iiFrom, iiTo, iiStep):";
        pasteText(text);
        return true;
    case Qt::Key_G :
        if (Pressed_2)
        {
            text  = "graphName = \"g\"\n"
                    "graph.new2D(graphName)\n"
                    "graph.addPoints(graphName, ArrayOfArraysOfXYZ)\n";
            if (shift)
            {
                text += "graph.setAxisTitles(graphName, \"X_axis_title\", \"Y_axis_title\", \"Z_axis_title\")\n"
                        "graph.setTitle(graphName, \"Title\")\n";
            }
            text += "graph.draw(graphName, \"lego\");";
        }
        else
        {
            text  = "graphName = \"g\"\n"
                    "graph.new1D(graphName)\n"
                    "graph.addPoints(graphName, ArrayOfArrysOfXY)\n";
            if (shift)
            {
                text += "graph.setAxisTitles(graphName, \"X_axis_title\", \"Y_axis_title\")\n"
                        "graph.setLineProperties(graphName, lineColor, lineStyle, lineWidth)\n"
                        "graph.setMarkerProperties(graphName, markerColor, markerStyle, markerSize)\n"
                        "graph.setTitle(graphName, \"Title\")\n";
            }
            text += "graph.draw(graphName, \"APL\")";
        }
        pasteText(text);
        return true;
    case Qt::Key_H :
        if (Pressed_2)
        {
            text  = "histName = \"h\"\n"
                   "hist.new2D(histName, xBins, xFrom, xTo,  yBins, yFrom, yTo)\n"
                   "hist.fillArr(histName, arrayOfArrysOfXYZ)\n";
            if (shift)
            {
                text += "hist.setAxisTitles(histName, \"X_axis_title\", \"Y_axis_title\", \"Z_axis_title\")\n"
                        "hist.setTitle(histName, \"Title\")\n";
            }
            text += "hist.draw(histName, \"colz\")";
        }
        else
        {
            text  = "histName = \"h\"\n"
                    "hist.new1D(histName, Bins, RangeFrom, RangeTo)\n"
                    "hist.fillArr(histName, arrayOfArrayOfXY)\n";
            if (shift)
            {
                text += "hist.setAxisTitles(histName, \"X_axis_title\", \"Y_axis_title\")\n"
                        "hist.setLineProperties(histName, lineColor, lineStyle, lineWidth)\n"
                        "hist.setTitle(histName, \"Title\")\n";
            }
            text += "hist.draw(histName, \"hist\")";
        }
        pasteText(text);
        return true;
    case Qt::Key_R :
        text = "root.SetOptStat( \"rmeoui\" )";
        pasteText(text);
        return true;
    }

    return false;
}

void ATextEdit::focusInEvent(QFocusEvent *e)
{
    //if (c) c->setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}

void ATextEdit::wheelEvent(QWheelEvent *e)
{
    if (lHelp) lHelp->hide();

    if (e->modifiers().testFlag(Qt::ControlModifier))
    {
        int size = font().pointSize();
        if (e->angleDelta().y() > 0) setFontSizeAndEmitSignal(++size);
        else setFontSizeAndEmitSignal(--size); //check is there: cannot go < 1
    }
    else
    {
        //scroll
        int delta = e->angleDelta().y();
        if (delta != 0) //paranoic :)
        {
            int was = this->verticalScrollBar()->value();
            delta /= abs(delta);
            this->verticalScrollBar()->setValue(was - 2*delta);
        }
    }
}

void ATextEdit::setFontSizeAndEmitSignal(int size)
{
    QFont f = font();
    if (size<1) size = 1;
    f.setPointSize(size);
    setFont(f);
    emit fontSizeChanged(size);
}

void ATextEdit::paintLeftField(QPaintEvent *event)
{
    QPainter painter(LeftField);
    //QColor color = QColor(Qt::gray).lighter(152);
    QColor color = palette().color(QPalette::AlternateBase);
    painter.fillRect(event->rect(), color);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>( blockBoundingGeometry(block).translated(contentOffset()).top() );
    int bottom = top + static_cast<int>( blockBoundingRect(block).height() );

    QFont font = painter.font();
    font.setPointSize(font.pointSize() * 0.8);
    painter.setFont(font);

    int currentLine = textCursor().block().firstLineNumber();

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(blockNumber + 1);
            if (guitools::isDarkTheme())
                painter.setPen( currentLine == blockNumber ? Qt::white : Qt::gray);
            else
                painter.setPen( currentLine == blockNumber ? Qt::black : Qt::gray);
            painter.drawText(0, top, LeftField->width(), fontMetrics().height(), Qt::AlignCenter, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>( blockBoundingRect(block).height() );
        ++blockNumber;
    }
}

int ATextEdit::getWidthLeftField() const
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10)
    {
        max /= 10;
        ++digits;
    }

    int width = 20 + fontMetrics().horizontalAdvance(QLatin1Char('0')) * digits;
    return width;
}

void ATextEdit::mouseDoubleClickEvent(QMouseEvent* /*e*/)
{
  QTextCursor tc = textCursor();
  tc.select(QTextCursor::WordUnderCursor);
  setTextCursor(tc);

  QList<QTextEdit::ExtraSelection> extraSelections;
  setExtraSelections(extraSelections);
}

void ATextEdit::focusOutEvent(QFocusEvent *e)
{
    emit editingFinished();
    if (lHelp) lHelp->hide();
    QPlainTextEdit::focusOutEvent(e);
}

void ATextEdit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    LeftField->setGeometry( QRect(cr.left(), cr.top(), getWidthLeftField(), cr.height()) );
}

void ATextEdit::insertCompletion(const QString &completion)
{
    if (Completer->widget() != this) return;

    QTextCursor tc = textCursor();
    //tc.select(QTextCursor::WordUnderCursor);
    while (tc.position() != 0)
      {
       tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
       QString selected = tc.selectedText();
       //qDebug() << "<-" <<selected << selected.left(1).contains(QRegExp("[A-Za-z0-9.]"));
       if ( !selected.left(1).contains(QRegularExpression("[A-Za-z0-9._]")) )
         {
           tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
           break;
         }
      }    
    //qDebug() << "reached left, selection:"<<tc.selectedText();
    tc.removeSelectedText();


    /*
    QString OnRight;
    while (tc.position() != document()->characterCount()-1)
      {
        tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        QString selected = tc.selectedText();
        qDebug() << "->"<< selected << selected.right(1).contains(QRegExp("[A-Za-z0-9.]"));
        OnRight = selected.right(1);
        if ( !OnRight.contains(QRegExp("[A-Za-z0-9._]")) )
          {
            tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
            break;
          }
      }
    if (!tc.selectedText().isEmpty()) tc.removeSelectedText();
    */
    tc.insertText(completion);


    //if inserted empty brackets at the end of the functuion, and the next character is "(", remove "()"
    tc.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 2);
    tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 3);
    if (tc.selectedText() == "()(")
    {
        tc.removeSelectedText();
        tc.insertText("(");
    }
}

void ATextEdit::findMatchingMethods(const QString & text, std::vector<std::pair<QString,int>> & pairs) const
{
    /*
    if (DeprecatedOrRemovedMethods && DeprecatedOrRemovedMethods->contains(text))
    {
        pairs = DeprecatedOrRemovedMethods->value(text);
        return true;
    }
    */

    for (const auto & p : *ListOfMethods)
    {
        QString tmp = p.first;
        QString returnArgument = tmp.section(' ', 0, 0) + " ";
        tmp.remove(returnArgument);
        if (tmp.remove(text).startsWith("("))
        {
            pairs.push_back(p);

            // !!!*** optimize by not looking at the other units once one is found
        }
    }
}

void ATextEdit::onCursorPositionChanged()
{
  //    qDebug() << "--> On cursor change";

  QList<QTextEdit::ExtraSelection> extraSelections;

  // extraSelections introduced later override previous if both match!
  QTextEdit::ExtraSelection extra;

  //lowest priority: highlight line where the cursor is
  QColor color = (guitools::isDarkTheme() ? QColor(Qt::darkGray).darker(250) : QColor(Qt::gray).lighter(150) );
  extra.format.setBackground(color);
  extra.format.setProperty(QTextFormat::FullWidthSelection, true);
  extra.cursor = textCursor();
  extra.cursor.clearSelection();
  extraSelections.append(extra);

  /*
  //higher priority: show with trailing spaces
  QTextCursor tc = textCursor();
  tc.select(QTextCursor::LineUnderCursor);
  QColor color = (guitools::isDarkTheme() ? QColor(Qt::gray).darker(140) : QColor(Qt::gray).lighter(140) );
  extra.format.setBackground(color);
  extra.cursor = tc;
  extraSelections.append(extra);
  */

  //checking for '}', ')' or ']' on the left
  QColor colorBrackets = ( guitools::isDarkTheme() ? QColor(Qt::darkGray).darker(170) : QColor(Qt::green).lighter(170) );
  checkBracketsOnLeft(extraSelections, colorBrackets);
  //checking for '{', '(' or '[' on the right
  checkBracketsOnRight(extraSelections, colorBrackets);

  if (!FindString.isEmpty())
  {
      QColor col = Qt::yellow;
      QTextCursor cursor = document()->find(FindString, 0, QTextDocument::FindCaseSensitively);
      while(cursor.hasSelection())
        {
          QTextEdit::ExtraSelection extra;
          extra.format.setBackground(col);
          extra.cursor = cursor;
          extraSelections.append(extra);
          cursor = document()->find(FindString, cursor, QTextDocument::FindCaseSensitively);
        }
  }
  else
  {
      // highlight same text if not in Exclude list
      QTextCursor tc = textCursor();
      tc.select(QTextCursor::WordUnderCursor);
      QString selection = tc.selectedText();
      //    qDebug() << "-->"<<selection;
      QColor color = ( guitools::isDarkTheme() ? QColor(Qt::darkGray).darker(150) : QColor(Qt::green).lighter(170) );
      QRegularExpression exl("[0-9 (){}\\[\\]=+\\-*/\\|~^.,:;\"'<>\\#\\$\\&\\?]");
      QString test = selection.simplified();
      test.remove(exl);
      //    qDebug() << "rem-->"<<test;
      if (test.isEmpty())
      {
          //could be after the var name and followed by bracket
          tc = textCursor();
          tc.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
          tc.select(QTextCursor::WordUnderCursor);
          selection = tc.selectedText();
          test = selection.simplified();
          test.remove(exl);
          //    qDebug() << "New test:>>>"<<test;
      }

      if (!test.isEmpty())
      {
          QRegularExpression pat("\\b"+selection+"\\b");
          QTextCursor cursor = document()->find(pat, 0, QTextDocument::FindCaseSensitively);
          while(cursor.hasSelection())
          {
              QTextEdit::ExtraSelection extra;
              extra.format.setBackground(color);
              extra.cursor = cursor;
              extraSelections.append(extra);
              cursor = document()->find(pat, cursor, QTextDocument::FindCaseSensitively);
          }
/*
          //variable highlight test  // !!!*** temporary disabled
          QRegularExpression patvar("\\bvar\\s+"+selection+"\\b"); // !!!*** let and const and Python aware!
          QTextCursor cursor1 = document()->find(patvar, tc, QTextDocument::FindCaseSensitively | QTextDocument::FindBackward);
          if (cursor1.hasSelection())// && cursor1 != tc)
          {
              QTextEdit::ExtraSelection extra;
              extra.format.setBackground(Qt::black);
              extra.format.setForeground(Qt::white);
                //extra.format.setFontUnderline(true);
              extra.format.setUnderlineColor(Qt::black);
              extra.cursor = cursor1;
              extraSelections.append(extra);

              QTextEdit::ExtraSelection extra1;
              extra1.format.setBackground(Qt::white);
              extra1.format.setFontUnderline(true);
              extra1.format.setUnderlineColor(Qt::blue);
              //extra1.format.setForeground(color);
              extra1.cursor = tc;
              extraSelections.append(extra1);
          }
*/
      }
  }

    //all extra selections defined, applying now
    setExtraSelections(extraSelections);

    // tooltip for known functions
    tryShowFunctionTooltip(textCursor());

    if (bMonitorLineChange)
    {
        int currentLine = textCursor().blockNumber();
        if ( currentLine != previousLineNumber)
        {
            previousLineNumber = currentLine;
            emit lineNumberChanged(currentLine);
        }
    }
}

int ATextEdit::findMathcingMethodsForCursor(const QTextCursor & cursor, std::vector<std::pair<QString,int>> & matchingMethods, bool & cursorIsInArguments)
{
    int functionEndPosition = -1;
    const QString functionCandidateText = selectObjFunctUnderCursor(cursor, functionEndPosition);

    // start with the case when the cursor is on one of the defined methods directly
    findMatchingMethods(functionCandidateText, matchingMethods);
    if (matchingMethods.empty())
    {
        AArgumentCounter counter(cursor, 0, ScriptLanguage);
        QTextCursor tc = cursor;
        bool ok = counter.moveCursorBeforeArguments(tc);
        if (!ok) return -1;
        tc.setPosition(tc.position(), QTextCursor::MoveAnchor);
        //qDebug() << SelectObjFunctUnderCursor(&tc);
        findMatchingMethods(selectObjFunctUnderCursor(tc, functionEndPosition), matchingMethods);
        cursorIsInArguments = true;

        /*
        QTextCursor tc = cursor;
        while (tc.position() != 0)
        {
            tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
            QString selected = tc.selectedText();
            //qDebug() << selected << selected.left(1).contains(QRegExp("[A-Za-z0-9.]"));
            if ( selected.left(1) == ")" ) break;
            if ( selected.left(1) == "\n" ) break;
            if ( selected.left(1) == "(")
            {
                // second case: the cursor is in the bracket section with the arguments
                tc.setPosition(tc.position(), QTextCursor::MoveAnchor);
                //qDebug() << SelectObjFunctUnderCursor(&tc);
                findMatchingMethods(selectObjFunctUnderCursor(tc, functionEndPosition), matchingMethods);
                cursorIsInArguments = true;
                break;
            }
        }
        */
    }
    return functionEndPosition;
}

bool ATextEdit::tryShowFunctionTooltip(const QTextCursor & cursor)
{
    std::vector<std::pair<QString,int>> matchingMethods;
    bool cursorIsInArguments = false;

    int functionEndPosition = findMathcingMethodsForCursor(cursor, matchingMethods, cursorIsInArguments);
    if (matchingMethods.empty())
    {
        ForcedMethodTooltipSelection = false;
        MethodTooltipVisible = false;
        SelectedMethodInTooltip = 0;
        //QToolTip::hideText();
        if (lHelp) lHelp->hide();
        return false;
    }

    std::sort(matchingMethods.begin(), matchingMethods.end(), [](const auto & lhs, const auto & rhs){return (lhs.second < rhs.second);});

    MethodTooltipVisible = true;
    NumberOfMethodsInTooltip = matchingMethods.size();

    int selectedMethod = 0;
    if (ForcedMethodTooltipSelection)
        selectedMethod = SelectedMethodInTooltip;
    else
    {
        int numNow = computeIntroducedNumberOfArguments(cursor, functionEndPosition);
        //qDebug() << "Computed number of arguments:" << numNow;

        selectedMethod = 0;
        for (const auto & pair : matchingMethods)
        {
            if (numNow == pair.second) break;
            selectedMethod++;
        }
    }

    QString tooltipText;
    int numArguments;
    if (matchingMethods.size() == 1)
    {
        tooltipText = "<p style='white-space:pre'>" + matchingMethods.front().first + "</p>";
        numArguments = matchingMethods.front().second;
    }
    else
    {
        if (selectedMethod >= matchingMethods.size()) selectedMethod = 0;
        tooltipText = QString("<p style='white-space:pre'><b>%0</b> of %1 [shift+\u2195]:    %2</p>")
                          .arg(selectedMethod + 1)
                          .arg(NumberOfMethodsInTooltip)
                          .arg(matchingMethods[selectedMethod].first);
        numArguments = matchingMethods[selectedMethod].second;
    }

    if (numArguments > 1 && cursorIsInArguments)
    {
        int currentArgument = computeCurrentArgument(cursor, functionEndPosition);
        //qDebug() << ">>>>>>>>>>" << currentArgument;
        if (currentArgument != -1)
        {
            int iPos = 0;
            for (; iPos < tooltipText.size(); iPos++)
                if (tooltipText[iPos] == '(') break;
            iPos++;
            int start = -1;
            if (currentArgument == 0) start = iPos;
            else
            {
                for (; iPos < tooltipText.size(); iPos++)
                    if (tooltipText[iPos] == ',')
                    {
                        currentArgument--;
                        if (currentArgument == 0)
                        {
                            iPos++;
                            start = iPos;
                            break;
                        }
                    }
            }
            int stop  = -1;
            for (; iPos < tooltipText.size(); iPos++)
                if (tooltipText[iPos] == ',' || tooltipText[iPos] == ')')
                {
                    stop = iPos - 1;
                    break;
                }

            if (start > 0 && start < tooltipText.size() && stop > 0 && stop < tooltipText.size())
            {
                //qDebug() << start << " - " << stop;
                QString toBold = tooltipText.mid(start, stop - start + 1);
                //qDebug() << "--->" << toBold;
                toBold = "<b>" + toBold + "</b>";
                tooltipText.replace(start, stop - start + 1, toBold);
            }
        }
    }

    const int fontHeight = fontMetrics().height();
    /*
        QToolTip::showText( mapToGlobal( QPoint(cursorRect(*cursor).topRight().x(), cursorRect(*cursor).topRight().y() -3.0*fh )),
                                    tooltipText,
                                    this,
                                    QRect(),
                                    1000000);
        */

    //lHelp->move(mapToGlobal( QPoint(cursorRect(*cursor).topRight().x(), cursorRect(*cursor).topRight().y() -3.0*fh)));

    int yPosition = cursorRect(cursor).topRight().y();
    //qDebug() << yPosition << fh;
    if (yPosition < 2*fontHeight)
        yPosition += 2 * fontHeight;
    else
        yPosition -= 2 * fontHeight;

    if (lHelp)
    {
        lHelp->move(mapToGlobal( QPoint( int(0.01*cursorRect(cursor).topRight().x())*100, yPosition)));
        lHelp->setText(tooltipText);
        lHelp->showNormal();
        lHelp->adjustSize();
    }

    return true;
}

int ATextEdit::computeIntroducedNumberOfArguments(const QTextCursor & cursor, int functionEndPosition)
{
    //qDebug() << cursor.position() << functionEndPosition;
    AArgumentCounter counter(cursor, functionEndPosition, ScriptLanguage);
    return counter.countArguments();
}

int ATextEdit::computeCurrentArgument(const QTextCursor & cursor, int functionEndPosition)
{
    AArgumentCounter counter(cursor, functionEndPosition, ScriptLanguage);
    return counter.getCurrentArgument();
}

#include <QTimer>
bool ATextEdit::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip)
    {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
        QTextCursor cursor = cursorForPosition(helpEvent->pos());

        return tryShowFunctionTooltip(cursor);
    }
    if (event->type() == QEvent::FocusIn && lHelp)
    {
        //QTimer::singleShot(0, this, [=](){lHelp->hide();});
        lHelp->hide();
    }
    if (event->type() == QEvent::FocusOut)
    {
        if (lHelp) lHelp->hide();
    }
    //qDebug() << event->type();
    return QPlainTextEdit::event(event);
}

void ATextEdit::leaveEvent(QEvent *)
{
    if (!rect().contains(mapFromGlobal(QCursor::pos())))
        if (lHelp) lHelp->hide();
}

void ATextEdit::mouseReleaseEvent(QMouseEvent *e)
{
    ForcedMethodTooltipSelection = false;

    QTextCursor cursor = cursorForPosition(e->pos());
    tryShowFunctionTooltip(cursor);
    QPlainTextEdit::mouseReleaseEvent(e);
}

void ATextEdit::checkBracketsOnLeft(QList<QTextEdit::ExtraSelection>& extraSelections, const QColor& color)
{
    //checking for '}', ')' or ']' on the left
    QTextCursor tc = textCursor();
    tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
    const QString selText = tc.selectedText();
    if ( selText == "}" || selText == ")" || selText == "]")
    {
        QString same, inverse;
        if (selText == "}")
        {
            same = "}"; inverse = "{";
        }
        else if (selText == ")")
        {
            same = ")"; inverse = "(";
        }
        else if (selText == "]")
        {
            same = "]"; inverse = "[";
        }

        int depth = 0;
        QString selected;
        do
        {
            tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
            selected = tc.selectedText();
            //qDebug() << selected << selected.left(1).contains(QRegExp("[A-Za-z0-9.]"));
            QString s = selected.left(1);
            if ( s == same )
            {
                depth++;
                continue;
            }
            if (s == inverse)
            {
                if (depth>0)
                {
                    depth--;
                    continue;
                }
                else break;
            }
        }
        while (tc.position() != 0);

        QTextEdit::ExtraSelection extra;
        extra.format.setBackground(color);
        extra.cursor = tc;
        extraSelections.append(extra);
    }
}

void ATextEdit::checkBracketsOnRight(QList<QTextEdit::ExtraSelection> &extraSelections, const QColor &color)
{
    //checking for '{', '(' or '[' on the right
    QTextCursor tc = textCursor();
    tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    const QString selText = tc.selectedText();
    if ( selText == "{" || selText == "(" || selText == "[" )
    {
        QString same, inverse;
        if (selText == "{")
        {
            same = "{"; inverse = "}";
        }
        else if (selText == "(")
        {
            same = "("; inverse = ")";
        }
        else if (selText == "[")
        {
            same = "["; inverse = "]";
        }
        int depth = 0;
        QString selected;
        do
        {
            tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            selected = tc.selectedText();
            //qDebug() << selected << selected.left(1).contains(QRegExp("[A-Za-z0-9.]"));
            QString s = selected.right(1);
            if ( s == same )
            {
                depth++;
                continue;
            }
            if (s == inverse)
            {
                if (depth>0)
                {
                    depth--;
                    continue;
                }
                else break;
            }
        }
        while (tc.position() < document()->characterCount()-1);

        QTextEdit::ExtraSelection extra;
        extra.format.setBackground(color);
        extra.cursor = tc;
        extraSelections.append(extra);
    }
}

void ATextEdit::updateLineNumberAreaWidth()
{
    setViewportMargins(getWidthLeftField(), 0, 0, 0);
}

void ATextEdit::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy) LeftField->scroll(0, dy);
    else LeftField->update(0, rect.y(), LeftField->width(), rect.height());

    if (rect.contains(viewport()->rect())) updateLineNumberAreaWidth();
}

void ATextEdit::setFontSize(int size)
{
    if (size<1) size = 1;
    QFont f = font();
    f.setPointSize(size);
    setFont(f);
}

void ATextEdit::refreshExtraHighlight()
{
    onCursorPositionChanged();
}

void ATextEdit::setTextCursorSilently(const QTextCursor &tc)
{
    bMonitorLineChange = false;
    setTextCursor(tc);
    bMonitorLineChange = true;
}

int ATextEdit::getIndent(const QString& line) const
{
    int indent = 0;
    if (!line.isEmpty())
    {
        for (indent = 0; indent<line.size(); indent++)
            if (line.at(indent) != ' ') break;

        if (indent == line.size()) indent = -1;
    }
    return indent;
}

void ATextEdit::setIndent(QString &line, int indent)
{
    line = line.trimmed();

    const QString spaces = QString(indent, ' ');
    line.insert(0, spaces);
}

void ATextEdit::convertTabToSpaces(QString& line)
{
    for (int i=line.size()-1; i>-1; i--)
        if (line.at(i) == '\t')
        {
            line.remove(i, 1);
            const QString spaces = QString(ATextEdit::TabInSpaces, ' ');
            line.insert(i, spaces);
        }
}

int ATextEdit::getSectionCounterChange(const QString& line) const
{
    int counter = 0;
    int bComment = false;
    for (int i=0; i<line.size(); i++)
    {
        const QChar & ch = line.at(i);
        // ***!!! add ignore commented inside /* */
        if      (ch == '{' ) counter++;
        else if (ch == '}' ) counter--;
        else if (ch == '/')
        {
            if (bComment) break;
            else bComment = true;
        }
        else bComment = false;
    }
    return counter;
}

void ATextEdit::paste()
{
    QClipboard *clipboard = QApplication::clipboard();
    QString text = clipboard->text();
    QStringList lines = text.split('\n');
    if (text.isEmpty() || lines.isEmpty())
    {
        QPlainTextEdit::paste(); //just in case, but most likely it is empty
        return;
    }

    QTextCursor tc = textCursor();
    tc.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    QString s = tc.selectedText();
    s.remove(' ');
    if (s.isEmpty())
    {
        //  qDebug() << "Adjusting ident";
        tc = textCursor();
        int newIdent = textCursor().positionInBlock();

        QString toInsert;
        for (int i=0; i<lines.size(); i++)
        {
            QString modLine = lines.at(i);
            if (i != lines.size()-1) modLine += "\n";
            if (i != 0) modLine = QString(newIdent, ' ') + modLine;
            toInsert += modLine;
        }
        tc.insertText( toInsert );
    }
    else
    {
        //  qDebug() << "Not empty, using regular paste";
        QPlainTextEdit::paste(); //just in case, but most likely it is empty
    }
}

void ATextEdit::pasteText(const QString & text)
{
    QStringList lines = text.split('\n');
    lines.push_back("");

    QTextCursor tc = textCursor();
    tc.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);

    //  qDebug() << "Adjusting ident";
    tc = textCursor();
    int newIdent = textCursor().positionInBlock();

    QString toInsert;
    for (int i=0; i<lines.size(); i++)
    {
        QString modLine = lines.at(i);
        if (i != lines.size()-1) modLine += "\n";
        if (i != 0) modLine = QString(newIdent, ' ') + modLine;
        toInsert += modLine;
    }
    tc.insertText( toInsert );
}

void ATextEdit::align()
{
    QTextCursor tc = textCursor();
    int start = tc.anchor();
    int stop = tc.position();
    if (start > stop) std::swap(start, stop);

    tc.setPosition(stop, QTextCursor::MoveAnchor);
    tc.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
    tc.setPosition(start, QTextCursor::KeepAnchor);
    tc.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);

    QString text = tc.selection().toPlainText();
    if (text.isEmpty()) return;

    QStringList list = text.split('\n');
    if (list.size() <= 1) return;

    for (QString& s : list) convertTabToSpaces(s);

    int currentIndent = getIndent(list.first());
    const int size = list.size();
    for (int i = 1; i <= size; i++)
    {
        int deltaSections = getSectionCounterChange(list.at(i-1));

        if (deltaSections >= 0)
        {
            currentIndent += deltaSections * TabInSpaces;
            //no need to adjust the previous line
        }
        else
        {
            currentIndent += deltaSections * TabInSpaces;
            if (currentIndent < 0) currentIndent = 0;
            setIndent(list[i-1], currentIndent);
        }
        if (i != size)
            setIndent(list[i], currentIndent);
    }

    QString res = list.join('\n');
    tc.insertText(res);
}

QString ATextEdit::textUnderCursor() const
{    
    QTextCursor tc = textCursor();
/*
    tc.select(QTextCursor::WordUnderCursor);
    //tc.select(QTextCursor::LineUnderCursor);
    return tc.selectedText();
*/
    QString selected;
    do
      {
       tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
       selected = tc.selectedText();
       //qDebug() << selected << selected.left(1).contains(QRegExp("[A-Za-z0-9.]"));
       if ( !selected.left(1).contains(QRegularExpression("[A-Za-z0-9._]")) ) return selected.remove(0,1);
      }
    while (tc.position() != 0);

    return selected;
}

QString ATextEdit::selectObjFunctUnderCursor(const QTextCursor & cursor, int & functEndPosition) const
{
    QString sel;
    QTextCursor tc = cursor;
    while (tc.position() != 0)
    {
        tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
        QString selected = tc.selectedText();
        //qDebug() << "<-" <<selected << selected.left(1).contains(QRegExp("[A-Za-z0-9.]"));
        if ( !selected.left(1).contains(QRegularExpression("[A-Za-z0-9._]")) )
        {
            tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            break;
        }
    }
    sel = tc.selectedText();

    while (tc.position() != document()->characterCount()-1)
    {
        tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        QString selected = tc.selectedText();
        if (selected.isEmpty()) continue;
        //qDebug() << "->"<< selected << selected.right(1).contains(QRegExp("[A-Za-z0-9.]"));
        if ( !selected.right(1).contains(QRegularExpression("[A-Za-z0-9._]")) )
        {
            tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
            break;
        }
    }
    sel += tc.selectedText();
    functEndPosition = tc.position();
    return sel;
}

QString ATextEdit::SelectTextToLeft(QTextCursor cursor, int num) const
{
    while ( num>0 && cursor.position() != 0)
    {
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
        num--;
    }
    return cursor.selectedText();
}

bool ATextEdit::InsertClosingBracket() const
{
    //to the left
    QTextCursor tc = this->textCursor();
    int depthL = 0;
    do
    {
        QString s = SelectTextToLeft(tc, 1);
        tc.movePosition(QTextCursor::Left);
        if (s == "{") depthL++;
        else if (s == "}") depthL--;
    }
    while (tc.position() != 0 );

    //to the right
    tc = this->textCursor();
    int depthR = 0;
    while (tc.position() != document()->characterCount()-1)
      {
        tc.movePosition(QTextCursor::Right);
        QString s = SelectTextToLeft(tc, 1);
        if (s == "{") depthR++;
        if (s == "}") depthR--;
      }

    if ( (depthL + depthR) == 0) return false;
    if ( (depthL + depthR) < 0 ) return false;
    return true;
}
