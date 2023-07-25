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
    bool afterRun() override;
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
    QString editGetText(QString name); // use only inside action functions
    void editSetIntValidator(QString name, int min, int max);
    void editSetDoubleValidator(QString name, double min, double max, int decimals);
    void editSetCompleter(QString name, QVariant arrayOfStrings);
    //void editOnTextChanged --> see below

    void comboboxNew(QString name, QString addTo);
    void comboboxAppend(QString name, QVariant entries);
    void comboboxClear(QString name);
    QString comboboxGetSelected(QString name); // use only inside action functions
    //void comboboxOnTextChanged --> see below

    void textNew(QString name, QString addTo, QString text = "");
    void textClear(QString name);
    void textAppendPlainText(QString name, QString text);
    void textAppendHtml(QString name, QString text);
    QString textGet(QString name); // use only inside action functions

    void checkboxNew(QString name, QString addTo, QString text = "", bool checked = false);
    void checkboxSetText(QString name, QString text);
    void checkboxSetChecked(QString name, bool checked);
    bool checkboxIsChecked(QString name);  // use only inside action functions
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
    void resize(int width, int height);
    void move(int x, int y);
    void setAlwaysOnTop();
    void setWidgetTitle(QString title);

    void setEnabled(QString name, bool flag);
    void setVisible(QString name, bool flag);

protected:
    AGuiFromScrWin * Win = nullptr;
    QMap<QString, QWidget*> Widgets;
    QMap<QString, QLayout*> Layouts;

    bool  KeepLoop = false;

private slots:
    void init();
    void doShow();
    void doHide();
    void doResize(int w, int h);
    void doMove(int x, int y);
    void doSetOnTop();
    void onWindowClosed();

signals:
    void requestInit();
    void requestShow();
    void requestHide();
    void requestResize(int w, int h);
    void requestMove(int x, int y);
    void requestOnTop();
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
    void buttonOnRightClick(QString name, QJSValue scriptFunction);

    void editOnTextModified(QString name, QJSValue scriptFunction);

    void comboboxOnSelectionChanged(QString name, QJSValue scriptFunction);

    void checkboxOnClick(QString name, QJSValue scriptFunction);

};

#ifdef ANTS3_PYTHON
// ---- Python ----

class AGui_Py_SI : public AGui_SI
{
    Q_OBJECT
public:
    AGui_Py_SI(AGuiFromScrWin * win);

    AScriptInterface * cloneBase() const override {return new AGui_Py_SI(Win);}

public slots:
    void buttonOnClick(QString name, QString scriptFunctionName);
    void buttonOnRightClick(QString name, QString scriptFunctionName);

    void editOnTextModified(QString name, QString scriptFunctionName);

    void comboboxOnSelectionChanged(QString name, QString scriptFunctionName);

    void checkboxOnClick(QString name, QString scriptFunctionName);
};
#endif

#endif // AINTERFACETOGUISCRIPT_H
