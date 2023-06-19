#include "aonelinetextedit.h"

#include <QtGui>
#include <QCompleter>
#include <QAbstractItemView>
#include <QScrollBar>

AOneLineTextEdit::AOneLineTextEdit(const QString & txt, QWidget * parent) : QPlainTextEdit(parent)
{
    setText(txt);

    setTabChangesFocus(true);
    setWordWrapMode(QTextOption::NoWrap);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFixedHeight(sizeHint().height());
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    //setContentsMargins(0,0,0,0);
    document()->setDocumentMargin(2); // !!!*** hard coded!

    setAcceptDrops(false);
    setCenterOnScroll(true);
    setTabChangesFocus(false);

    QPalette p = palette();
    //p.setColor(QPalette::Disabled, QPalette::Base, QColor(235,235,235));
    p.setColor(QPalette::Disabled, QPalette::Base, p.color(QPalette::AlternateBase));
    setPalette(p);

    connect(this, &AOneLineTextEdit::textChanged, this, &AOneLineTextEdit::clearTooltip);
}

#include "ageoconsts.h"
void AOneLineTextEdit::setText(const QString & text)
{
    clear();
    appendPlainText(text);

    if (text.isEmpty()) return;

    bool ok;
    double val = text.toDouble(&ok);
    if (!ok)
    {
        QString errorStr;
        bool ok = AGeoConsts::getConstInstance().evaluateFormula(errorStr, text, val);

        QString toolTip;
        if (ok)
        {
            if (bIntegerTooltip) toolTip = QString::number((int)val);
            else                 toolTip = QString::number(val);
        }
        else                     toolTip = errorStr;

        setToolTip(toolTip);
        setToolTipDuration(1000);
    }
}

QString AOneLineTextEdit::text() const
{
    return document()->toPlainText();
}

void AOneLineTextEdit::setFrame(bool flag)
{
    setFrameShape( flag ? QFrame::Panel : QFrame::NoFrame );
}

void AOneLineTextEdit::insertCompletion(const QString &completion)
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    tc.removeSelectedText();
    tc.insertText(completion);
}

void AOneLineTextEdit::keyPressEvent(QKeyEvent * e)
{
    const bool bComplVisible = Completer && Completer->popup()->isVisible();
    if (!bComplVisible && e->key() == Qt::Key_Tab)
    {
        e->ignore();  //equivavlent of setTabChangesFocus(true);
        return;
    }

    //qDebug() << "Key pressed:" << e->text();
    if ( !Completer || !bComplVisible )
    {
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
        {
            e->accept();
            emit enterPressed();
            emit editingFinished();
            return;
        }
        if (e->key() == Qt::Key_Escape)
        {
            e->accept();
            emit escapePressed();
            return;
        }
    }

    if (!Completer)
    {
        QPlainTextEdit::keyPressEvent(e);
        return;
    }

    if (bComplVisible)
    {
        // The following keys are forwarded by the completer to the widget
        switch (e->key())
        {
        case Qt::Key_Tab: // Tab is not intercepted !
        {
            qDebug() << "Tab pressed when completer is active, filling in common part of the completer";
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

    QPlainTextEdit::keyPressEvent(e);

    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);//textUnderCursor();
    QString completionPrefix = tc.selectedText();

    if (/*e->text().isEmpty() || */ completionPrefix.length() < 3)
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

void AOneLineTextEdit::focusOutEvent(QFocusEvent *event)
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
    QPlainTextEdit::focusOutEvent(event);
    emit editingFinished();
}

void AOneLineTextEdit::clearTooltip()
{
    setToolTip("");
}

#include <QFont>
#include <QFontMetrics>
#include <QStyleOptionFrame>
#include <QStyle>
#include <QApplication>

QSize AOneLineTextEdit::sizeHint() const
{
    ensurePolished();
    QFontMetrics fm(font());
    const int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize, 0, this);
    //const QMargins tm = d->effectiveTextMargins();
    int h = qMax(fm.height(), qMax(14, iconSize - 2));// + 2 * QLineEditPrivate::verticalMargin + tm.top() + tm.bottom() + d->topmargin + d->bottommargin;
    //int w = fm.width(QLatin1Char('x')) * 17 + 4;//fm.horizontalAdvance(QLatin1Char('x')) * 17 + 2 * QLineEditPrivate::horizontalMargin + tm.left() + tm.right() + d->leftmargin + d->rightmargin; // "some"
    int w = fm.horizontalAdvance(QLatin1Char('x')) * 17 + 4;//fm.horizontalAdvance(QLatin1Char('x')) * 17 + 2 * QLineEditPrivate::horizontalMargin + tm.left() + tm.right() + d->leftmargin + d->rightmargin; // "some"
    QStyleOptionFrame opt;
    initStyleOption(&opt);
    // !!!***
    //return (style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(w, h).expandedTo(QApplication::globalStrut()), this));
    return (style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(w, h).expandedTo(QSize(10, 10)), this));
}

// ------------------

void ABasicHighlighter::highlightBlock(const QString & text)
{
    /*
    for (const AHighlightingRule & rule : qAsConst(HighlightingRules))
    {
        const QRegularExpression & expression = rule.pattern;
        int index = rule.pattern.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    */
    for (const AHighlightingRule & rule : qAsConst(HighlightingRules))
    {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
            while (matchIterator.hasNext())
            {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
    }
}
