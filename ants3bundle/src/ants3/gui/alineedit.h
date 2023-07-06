#ifndef ALINEEDIT_H
#define ALINEEDIT_H

#include <QLineEdit>
#include <QString>

class QKeyEvent;
class QCompleter;

class ALineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit ALineEdit(QWidget * parent = nullptr) : QLineEdit(parent) {}
    explicit ALineEdit(const QString & text, QWidget * parent = nullptr) : QLineEdit(text, parent) {}

    void setCompleter(QCompleter * completer);
    QCompleter * completer() const {return c;}

protected:
    void keyPressEvent(QKeyEvent * e) override;

private:
    QString cursorWord(const QString & sentence) const;

private slots:
    void insertCompletion(QString arg);

private:
    QCompleter * c = nullptr;
};

#endif // ALINEEDIT_H
