#ifndef ALINEEDITWITHESCAPE_H
#define ALINEEDITWITHESCAPE_H

#include <QLineEdit>

class ALineEditWithEscape : public QLineEdit
{
    Q_OBJECT
public:
    ALineEditWithEscape(const QString & text, QWidget * parent) : QLineEdit(text, parent){}

protected:
    void keyPressEvent(QKeyEvent * event);

signals:
    void escapePressed();
};

#endif // ALINEEDITWITHESCAPE_H
