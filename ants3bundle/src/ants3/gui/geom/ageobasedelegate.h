#ifndef AGEOBASEDELEGATE_H
#define AGEOBASEDELEGATE_H

#include <QObject>
#include <QString>

class AGeoObject;
class QPushButton;
class QHBoxLayout;
class AOneLineTextEdit;
class QFrame;

class AGeoBaseDelegate : public QObject
{
    Q_OBJECT

public:
    AGeoBaseDelegate(QWidget * ParentWidget);
    virtual ~AGeoBaseDelegate(){}

    virtual QString getName() const = 0;
    virtual bool updateObject(AGeoObject * obj) const = 0;
    virtual void Update(const AGeoObject * obj) = 0;

    bool isLeEmpty(const QVector<AOneLineTextEdit*> & v) const;  // !!!***

    void postUpdate();

public:
    QWidget * Widget = nullptr;

protected:
    QFrame      * frBottomButtons = nullptr;
    QPushButton * pbShow = nullptr;
    QPushButton * pbChangeAtt = nullptr;
    QPushButton * pbScriptLine = nullptr;

    void createBottomButtons();

protected:
    QWidget * ParentWidget = nullptr;

signals:
    void contentChanged();
    void RequestShow();
    void RequestChangeVisAttributes();
    void RequestScriptToClipboard();
    void RequestScriptRecursiveToClipboard();

public slots:
    void onContentChangedBase();

public:
    static void configureHighligherAndCompleter(AOneLineTextEdit * edit, int iUntilIndex = -1); // -1 == all
    static bool processEditBox(AOneLineTextEdit * lineEdit, double & val, QString & str, QWidget * parent);
};

#endif // AGEOBASEDELEGATE_H
