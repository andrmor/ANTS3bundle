#ifndef AINTERFACETOGUISCRIPT_H
#define AINTERFACETOGUISCRIPT_H

#include "ascriptinterface.h"

//#include <functional>

#include <QString>
#include <QVariant>
#include <QMap>

class AGuiFromScrWin;
class QLayout;
class QVBoxLayout;

// abstract base class: JS and Python have different function call implementation --> see below
class AGui_SI : public AScriptInterface
{
    Q_OBJECT
public:
    AGui_SI(AGuiFromScrWin * parent);

    bool beforeRun() override;
    void abortRun() override;

public slots:
    void buttonNew(QString name, QString addTo, QString text = "");
    void buttonSetText(QString name, QString text, bool bold = false);
    //void buttonOnRightClick --> see below
    //void buttonOnClick --> see below

    void labelNew(QString name, QString addTo, QString text);
    void labelSetText(QString name, QString labelText);

    void editNew(QString name, QString addTo, QString text = "");
    void editSetText(QString name, QString text);
    QString editGetText(QString name); // !!!***
    void editSetIntValidator(QString name, int min, int max);
    void editSetDoubleValidator(QString name, double min, double max, int decimals);
    void editSetCompleter(QString name, QVariant arrayOfStrings);
    //void editOnTextChanged --> see below

    void comboboxNew(QString name, QString addTo, bool editable = false);
    void comboboxAppend(QString name, QVariant entries);
    void comboboxClear(QString name);
    QString comboboxGetSelected(QString name); // !!!***
    //void comboboxOnTextChanged --> see below

    void textNew(QString name, QString addTo, QString text = "");
    void textClear(QString name);
    void textAppendPlainText(QString name, QString text);
    void textAppendHtml(QString name, QString text);
    QString textGet(QString name); // !!!***

    void checkboxNew(QString name, QString addTo, QString text = "", bool checked = false);
    void checkboxSetText(QString name, QString text);
    void checkboxSetChecked(QString name, bool checked);
    bool checkboxIsChecked(QString name);  // !!!***
    //void checkboxOnClick --> see below

    void setMinimumWidth(QString name, int min);
    void setMinimumHeight(QString name, int min);
    void setMaximumWidth(QString name, int max);
    void setMaximumHeight(QString name, int max);

    void setToolTip(QString name, QString text);

    void addStretch(QString addTo);
    void addHoizontalLine(QString addTo);
    void addVerticalLine(QString addTo);

    void verticalLayout(QString name, QString addTo);
    void horizontalLayout(QString name, QString addTo);

    void messageBox(QString text);

    void show();
    void hide();
    void setWidgetTitle(QString title);
    void resize(int width, int height);

    void setEnabled(QString name, bool flag);
    void setVisible(QString name, bool flag);

protected:
    AGuiFromScrWin * Win = nullptr;
    QMap<QString, QWidget*> Widgets;
    QMap<QString, QLayout*> Layouts;

public slots:
    void init();
    void doShow();
    void doHide();

signals:
    void requestInit();
    void requestShow();
    void requestHide();
};

// ---- JavaScript ----

#include <QJSValue>
class AGui_JS_SI : public AGui_SI
{
    Q_OBJECT
public:
    AGui_JS_SI(AGuiFromScrWin * win);

    AScriptInterface * cloneBase() const override {return new AGui_JS_SI(Win);}

public slots:
    void buttonOnClick(QString name, QJSValue scriptFunction);
    void buttonOnRightClick(QString name, QVariant scriptFunction); // !!!***

    void editOnTextChanged(QString name, QVariant scriptFunction); // !!!***

    void comboboxOnTextChanged(QString name, QVariant scriptFunction); // !!!***

    void checkboxOnClick(QString name, const QVariant scriptFunction); // !!!***

private:
    std::function<void(void)> getCallable(const QVariant & function);
};
#endif // AINTERFACETOGUISCRIPT_H
