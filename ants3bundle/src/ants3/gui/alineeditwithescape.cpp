#include "alineeditwithescape.h"

#include <QKeyEvent>

void ALineEditWithEscape::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_Escape)
    {
        event->accept();
        emit escapePressed();
    }
    else QLineEdit::keyPressEvent(event);
}
