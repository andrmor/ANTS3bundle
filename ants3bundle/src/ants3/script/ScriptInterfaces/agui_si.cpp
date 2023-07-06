#include "agui_si.h"
#include "ascripthub.h"
#include "ajscriptmanager.h"
#include "guitools.h"
#include "alineedit.h"
#include "aguifromscrwin.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSpacerItem>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QFont>
#include <QFrame>
#include <QDebug>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QCompleter>
#include <QTimer>


AGui_SI::AGui_SI(AGuiFromScrWin * win) :
    AScriptInterface(), Win(win)
{
    //Wid->setWindowFlags(Qt::WindowStaysOnTopHint);

    init();

    connect(this, &AGui_SI::requestInit, this, &AGui_SI::init, Qt::QueuedConnection);
    connect(this, &AGui_SI::requestShow, this, &AGui_SI::doShow, Qt::QueuedConnection);
    connect(this, &AGui_SI::requestHide, this, &AGui_SI::doHide, Qt::QueuedConnection);
}

void AGui_SI::init()
{
    Widgets.clear(); //items owned by Wid!
    Layouts.clear(); //items owned by Wid!

    QLayout * lay = Win->resetLayout();
    Layouts.insert("", lay);

    Win->hide();
}

void AGui_SI::doShow()
{
    Win->restoreGeomStatus();
    Win->showNormal();
    Win->activateWindow();
}

void AGui_SI::doHide()
{
    Win->storeGeomStatus();
    Win->hide();
}

bool AGui_SI::beforeRun()
{
    emit requestInit();
    return true;
}

void AGui_SI::abortRun()
{
    emit requestHide();
}

void AGui_SI::buttonNew(QString name, QString addTo, QString text)
{
    QTimer::singleShot(0, Win, [this, name, addTo, text]()
                       {
                           if (Widgets.contains(name))
                           {
                               abort("Widget " + name + " already exists");
                               return;
                           }
                           QLayout * lay = Layouts.value(addTo, 0);
                           if (!lay)
                           {
                               abort("Layout " + addTo + " does not exist");
                               return;
                           }
                           QPushButton * b = new QPushButton(text);
                           b->setContextMenuPolicy(Qt::CustomContextMenu);
                           Widgets.insert(name, b);
                           lay->addWidget(b);
                       });
}

void AGui_SI::buttonSetText(QString name, QString text, bool bold)
{
    QTimer::singleShot(0, Win, [this, name, text, bold]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           QPushButton * b = dynamic_cast<QPushButton*>(w);
                           if (!b) abort("Button " + name + " does not exist");
                           else
                           {
                               b->setText(text);
                               QFont fo = b->font();
                               fo.setBold(bold);
                               b->setFont(fo);
                           }
                       });
}

void AGui_SI::labelNew(QString name, QString addTo, QString text)
{
    QTimer::singleShot(0, Win, [this, name, addTo, text]()
                       {
                           if (Widgets.contains(name))
                           {
                               abort("Widget " + name + " already exists");
                               return;
                           }
                           QLayout * lay = Layouts.value(addTo, 0);
                           if (!lay)
                           {
                               abort("Layout " + addTo + " does not exist");
                               return;
                           }
                           QLabel * l = new QLabel(text);
                           Widgets.insert(name, l);
                           lay->addWidget(l);
    });
}

void AGui_SI::labelSetText(QString name, QString labelText)
{
    QTimer::singleShot(0, Win, [this, name, labelText]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           QLabel * l = dynamic_cast<QLabel*>(w);
                           if (!l)
                           {
                               abort("Label " + name + " does not exist");
                               return;
                           }
                           l->setText(labelText);
    });
}

void AGui_SI::editNew(QString name, QString addTo, QString text)
{
    QTimer::singleShot(0, Win, [this, name, addTo, text]()
                       {
                           if (Widgets.contains(name))
                           {
                               abort("Widget " + name + " already exists");
                               return;
                           }
                           QLayout * lay = Layouts.value(addTo, 0);
                           if (!lay)
                           {
                               abort("Layout " + addTo + " does not exist");
                               return;
                           }
                           ALineEdit * e = new ALineEdit(text);
                           Widgets.insert(name, e);
                           lay->addWidget(e);
                       });
}

void AGui_SI::editSetText(QString name, QString text)
{
    QTimer::singleShot(0, Win, [this, name, text]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           ALineEdit * e = dynamic_cast<ALineEdit*>(w);
                           if (!e) abort("Edit box " + name + " does not exist");
                           else e->setText(text);
                       });
}

QString AGui_SI::editGetText(QString name)
{
    QWidget* w = Widgets.value(name, 0);
    ALineEdit* e = dynamic_cast<ALineEdit*>(w);
    if (!e)
    {
        abort("Edit box " + name + " does not exist");
        return "";
    }
    return e->text();
}

void AGui_SI::editSetIntValidator(QString name, int min, int max)
{
    QTimer::singleShot(0, Win, [this, name, min, max]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           ALineEdit * e = dynamic_cast<ALineEdit*>(w);
                           if (!e) abort("Edit box " + name + " does not exist");
                           else
                           {
                               const QValidator * old = e->validator();
                               if (old) delete old;

                               QValidator * validator = new QIntValidator(min, max, e);
                               e->setValidator(validator);
                           }
                       });
}

void AGui_SI::editSetDoubleValidator(QString name, double min, double max, int decimals)
{
    QTimer::singleShot(0, Win, [this, name, min, max, decimals]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           ALineEdit * e = dynamic_cast<ALineEdit*>(w);
                           if (!e) abort("Edit box " + name + " does not exist");
                           else
                           {
                               const QValidator * old = e->validator();
                               if (old) delete old;

                               QValidator * validator = new QDoubleValidator(min, max, decimals, e);
                               e->setValidator(validator);
                           }
                       });
}

void AGui_SI::editSetCompleter(QString name, QVariant arrayOfStrings)
{
    QTimer::singleShot(0, Win, [this, name, arrayOfStrings]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           ALineEdit * e = dynamic_cast<ALineEdit*>(w);
                           if (!e) abort("Edit box " + name + " does not exist");
                           else
                           {
                               const QCompleter * old = e->completer();
                               if (old) delete old;

                               QVariantList vl = arrayOfStrings.toList();
                               QStringList sl;
                               for (int i=0; i<vl.size(); i++) sl << vl.at(i).toString();

                               //qDebug() << "possible:" << sl;

                               QCompleter * completer = new QCompleter(sl, e);
                               completer->setCaseSensitivity(Qt::CaseInsensitive); //Qt::CaseSensitive
                               completer->setCompletionMode(QCompleter::PopupCompletion);
                               //completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
                               completer->setFilterMode(Qt::MatchContains);
                               completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
                               completer->setWrapAround(false);
                               e->setCompleter(completer);
                           }
                       });
}

void AGui_SI::comboboxNew(QString name, QString addTo)
{
    QTimer::singleShot(0, Win, [this, name, addTo]()
                       {
                           if (Widgets.contains(name))
                           {
                               abort("Widget " + name + " already exists");
                               return;
                           }
                           QLayout * lay = Layouts.value(addTo, 0);
                           if (!lay)
                           {
                               abort("Layout " + addTo + " does not exist");
                               return;
                           }
                           QComboBox * e = new QComboBox();
                           //e->setEditable(editable);
                           Widgets.insert(name, e);
                           lay->addWidget(e);
                       });
}

void AGui_SI::comboboxAppend(QString name, QVariant entries)
{
    QTimer::singleShot(0, Win, [this, name, entries]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           QComboBox * e = dynamic_cast<QComboBox*>(w);
                           if (!e) abort("Combobox " + name + " does not exist");
                           else
                           {
                               QVariantList vl = entries.toList();
                               QStringList list;
                               for (QVariant & v : vl) list << v.toString();
                               e->addItems(list);
                           }
                       });
}

void AGui_SI::comboboxClear(QString name)
{
    QTimer::singleShot(0, Win, [this, name]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           QComboBox * e = dynamic_cast<QComboBox*>(w);
                           if (!e) abort("Combobox " + name + " does not exist");
                           else e->clear();
                       });
}

QString AGui_SI::comboboxGetSelected(QString name)
{
    QWidget* w = Widgets.value(name, 0);
    QComboBox* e = dynamic_cast<QComboBox*>(w);
    if (!e)
    {
        abort("Combobox " + name + " does not exist");
        return "";
    }
    return e->currentText();
}

void AGui_SI::textNew(QString name, QString addTo, QString text)
{
    QTimer::singleShot(0, Win, [this, name, addTo, text]()
                       {
                           if (Widgets.contains(name))
                           {
                               abort("Widget " + name + " already exists");
                               return;
                           }
                           QLayout * lay = Layouts.value(addTo, 0);
                           if (!lay)
                           {
                               abort("Layout " + addTo + " does not exist");
                               return;
                           }
                           QPlainTextEdit * t = new QPlainTextEdit(text);
                           Widgets.insert(name, t);
                           lay->addWidget(t);
    });
}

void AGui_SI::textClear(QString name)
{
    QTimer::singleShot(0, Win, [this, name]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           QPlainTextEdit * t = dynamic_cast<QPlainTextEdit*>(w);
                           if (!t)
                           {
                               abort("Text box " + name + " does not exist");
                               return;
                           }
                           t->clear();
                       });
}

void AGui_SI::textAppendPlainText(QString name, QString text)
{
    QTimer::singleShot(0, Win, [this, name, text]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           QPlainTextEdit * t = dynamic_cast<QPlainTextEdit*>(w);
                           if (!t)
                           {
                               abort("Text box " + name + " does not exist");
                               return;
                           }
                           t->appendPlainText(text);
    });
}

void AGui_SI::textAppendHtml(QString name, QString text)
{
    QTimer::singleShot(0, Win, [this, name, text]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           QPlainTextEdit * t = dynamic_cast<QPlainTextEdit*>(w);
                           if (!t)
                           {
                               abort("Text box " + name + " does not exist");
                               return;
                           }
                           t->appendHtml(text);
                       });
}

QString AGui_SI::textGet(QString name)
{
    QWidget* w = Widgets.value(name, 0);
    QPlainTextEdit* t = dynamic_cast<QPlainTextEdit*>(w);
    if (!t)
    {
        abort("Text box " + name + " does not exist");
        return "";
    }
    return t->document()->toPlainText();
}

void AGui_SI::checkboxNew(QString name, QString addTo, QString text, bool checked)
{
    QTimer::singleShot(0, Win, [this, name, addTo, text, checked]()
                       {
                           if (Widgets.contains(name))
                           {
                               abort("Widget " + name + " already exists");
                               return;
                           }
                           QLayout * lay = Layouts.value(addTo, 0);
                           if (!lay)
                           {
                               abort("Layout " + addTo + " does not exist");
                               return;
                           }
                           QCheckBox * cb = new QCheckBox(text);
                           cb->setChecked(checked);
                           Widgets.insert(name, cb);
                           lay->addWidget(cb);
    });
}

void AGui_SI::checkboxSetText(QString name, QString text)
{
    QTimer::singleShot(0, Win, [this, name, text]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           QCheckBox * cb = dynamic_cast<QCheckBox*>(w);
                           if (!cb) abort("Checkbox " + name + " does not exist");
                           else cb->setText(text);
                       });
}

void AGui_SI::checkboxSetChecked(QString name, bool checked)
{
    QTimer::singleShot(0, Win, [this, name, checked]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           QCheckBox * cb = dynamic_cast<QCheckBox*>(w);
                           if (!cb) abort("Checkbox " + name + " does not exist");
                           else cb->setChecked(checked);
                       });
}

bool AGui_SI::checkboxIsChecked(QString name)
{
    QWidget* w = Widgets.value(name, 0);
    QCheckBox* cb = dynamic_cast<QCheckBox*>(w);
    if (!cb)
    {
        abort("Checkbox " + name + " does not exist");
        return false;
    }
    return cb->isChecked();
}

void AGui_SI::setMinimumWidth(QString name, int min)
{
    QTimer::singleShot(0, Win, [this, name, min]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           if (!w) abort("Widget " + name + " does not exist");
                           else w->setMinimumWidth(min);
                       });
}

void AGui_SI::setMinimumHeight(QString name, int min)
{
    QTimer::singleShot(0, Win, [this, name, min]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           if (!w) abort("Widget " + name + " does not exist");
                           else w->setMinimumHeight(min);
                       });
}

void AGui_SI::setMaximumWidth(QString name, int max)
{
    QTimer::singleShot(0, Win, [this, name, max]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           if (!w) abort("Widget " + name + " does not exist");
                           else w->setMaximumWidth(max);
                       });
}

void AGui_SI::setMaximumHeight(QString name, int max)
{
    QTimer::singleShot(0, Win, [this, name, max]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           if (!w) abort("Widget " + name + " does not exist");
                           else w->setMaximumHeight(max);
                       });
}

void AGui_SI::setToolTip(QString name, QString text)
{
    QTimer::singleShot(0, Win, [this, name, text]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           if (!w) abort("Widget " + name + " does not exist");
                           else w->setToolTip(text);
                       });
}

void AGui_SI::addStretch(QString addTo)
{
    QTimer::singleShot(0, Win, [this, addTo]()
                       {
                           QLayout * lay = Layouts.value(addTo, 0);
                           if (!lay)
                           {
                               abort("Layout " + addTo + " does not exist");
                               return;
                           }
                           QVBoxLayout * v = dynamic_cast<QVBoxLayout*>(lay);
                           if (v) v->addStretch();
                           else
                           {
                               QHBoxLayout * h = dynamic_cast<QHBoxLayout*>(lay);
                               if (h) h->addStretch();
                           }
                       });
}

void AGui_SI::addHoizontalLine(QString addTo)
{
    QTimer::singleShot(0, Win, [this, addTo]()
                       {
                           QLayout * lay = Layouts.value(addTo, 0);
                           if (!lay)
                           {
                               abort("Layout " + addTo + " does not exist");
                               return;
                           }
                           QFrame * e = new QFrame;
                           e->setFrameShape(QFrame::HLine);
                           e->setFrameShadow(QFrame::Raised);
                           lay->addWidget(e);
                       });
}

void AGui_SI::addVerticalLine(QString addTo)
{
    QTimer::singleShot(0, Win, [this, addTo]()
                       {
                           QLayout * lay = Layouts.value(addTo, 0);
                           if (!lay)
                           {
                               abort("Layout " + addTo + " does not exist");
                               return;
                           }
                           QFrame * e = new QFrame;
                           e->setFrameShape(QFrame::VLine);
                           e->setFrameShadow(QFrame::Raised);
                           lay->addWidget(e);
                       });
}

void AGui_SI::verticalLayout(QString name, QString addTo)
{
    QTimer::singleShot(0, Win, [this, name, addTo]()
                       {
                           if (Layouts.contains(name))
                           {
                               abort("Layout " + name + " already exists");
                               return;
                           }
                           QLayout * lay = Layouts.value(addTo, 0);
                           if (!lay)
                           {
                               abort("Layout " + addTo + " does not exist");
                               return;
                           }
                           QVBoxLayout * l = new QVBoxLayout();
                           Layouts.insert(name, l);

                           QVBoxLayout * v = dynamic_cast<QVBoxLayout*>(lay);
                           if (v) v->addLayout(l);
                           else
                           {
                               QHBoxLayout * h = dynamic_cast<QHBoxLayout*>(lay);
                               if (h) h->addLayout(l);
                           }
                       });
}

void AGui_SI::horizontalLayout(QString name, QString addTo)
{
    QTimer::singleShot(0, Win, [this, name, addTo]()
                       {
                           if (Layouts.contains(name))
                           {
                               abort("Layout " + name + " already exists");
                               return;
                           }
                           QLayout * lay = Layouts.value(addTo, 0);
                           if (!lay)
                           {
                               abort("Layout " + addTo + " does not exist");
                               return;
                           }
                           QHBoxLayout * l = new QHBoxLayout();
                           Layouts.insert(name, l);

                           QVBoxLayout * v = dynamic_cast<QVBoxLayout*>(lay);
                           if (v) v->addLayout(l);
                           else
                           {
                               QHBoxLayout * h = dynamic_cast<QHBoxLayout*>(lay);
                               if (h) h->addLayout(l);
                           }
                       });
}

void AGui_SI::messageBox(QString text)
{
    QTimer::singleShot(0, Win, [this, text]()
                       {
                           guitools::message(text, Win);
                       });
}

void AGui_SI::show()
{
    emit requestShow();
}

void AGui_SI::hide()
{
    emit requestHide();
}

void AGui_SI::setWidgetTitle(QString title)
{
    QTimer::singleShot(0, Win, [this, title]()
                       {
                           Win->setWindowTitle(title);
                       });
}

void AGui_SI::resize(int width, int height)
{
    QTimer::singleShot(0, Win, [this, width, height]()
                       {
                           Win->resize(width, height);
                       });
}

void AGui_SI::setEnabled(QString name, bool flag)
{
    QTimer::singleShot(0, Win, [this, name, flag]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           if (!w) abort("Widget " + name + " does not exist");
                           else w->setEnabled(flag);
                       });
}

void AGui_SI::setVisible(QString name, bool flag)
{
    QTimer::singleShot(0, Win, [this, name, flag]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           if (!w) abort("Widget " + name + " does not exist");
                           else w->setVisible(flag);
                       });
}

// ---- JavaScript ----

//std::function<int (int)> retFun() {return [](int x) { return x; };}

AGui_JS_SI::AGui_JS_SI(AGuiFromScrWin * win) :
    AGui_SI(win) {}

void AGui_JS_SI::buttonOnClick(QString name, QJSValue scriptFunction)
{
    QTimer::singleShot(0, Win, [this, name, scriptFunction]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           QPushButton * b = dynamic_cast<QPushButton*>(w);
                           if (!b)
                           {
                               abort("Button " + name + " does not exist");
                               return;
                           }
                           if (scriptFunction.isCallable())
                           {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                               abort("Direct call is not supported for Qt5, use function name (e.g. \"doSomething\")");
                               return;
#else
                               connect(b, &QPushButton::clicked, Win, [scriptFunction]()
                                       {
                                           bool err = scriptFunction.call().isError();
                                           if (err) AScriptHub::getInstance().abort("Error while executing function call", EScriptLanguage::JavaScript);
                                       });
#endif
                           }
                           else
                           {
                               QString functionName = scriptFunction.toString();
                               connect(b, &QPushButton::clicked, Win, [functionName]()
                                       {
                                           AScriptHub & hub = AScriptHub::getInstance();
                                           bool ok = hub.getJScriptManager().callFunctionNoArguments(functionName);
                                           if (!ok) AScriptHub::getInstance().abort("Error while executing function call", EScriptLanguage::JavaScript);
                                       });
                           }
    } );
}

void AGui_JS_SI::buttonOnRightClick(QString name, QJSValue scriptFunction)
{
    QTimer::singleShot(0, Win, [this, name, scriptFunction]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           QPushButton * b = dynamic_cast<QPushButton*>(w);
                           if (!b)
                           {
                               abort("Button " + name + " does not exist");
                               return;
                           }
                           if (scriptFunction.isCallable())
                           {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                               abort("Direct call is not supported for Qt5, use function name (e.g. \"doSomething\")");
                               return;
#else
                connect(b, &QPushButton::customContextMenuRequested, Win, [scriptFunction]()
                        {
                            bool err = scriptFunction.call().isError();
                            if (err) AScriptHub::getInstance().abort("Error while executing function call", EScriptLanguage::JavaScript);
                        });
#endif
                           }
                           else
                           {
                               QString functionName = scriptFunction.toString();
                               connect(b, &QPushButton::customContextMenuRequested, Win, [functionName]()
                                       {
                                           AScriptHub & hub = AScriptHub::getInstance();
                                           bool ok = hub.getJScriptManager().callFunctionNoArguments(functionName);
                                           if (!ok) AScriptHub::getInstance().abort("Error while executing function call", EScriptLanguage::JavaScript);
                                       });
                           }
                       } );
}

void AGui_JS_SI::editOnTextChanged(QString name, QJSValue scriptFunction)
{
    QTimer::singleShot(0, Win, [this, name, scriptFunction]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           ALineEdit * e = dynamic_cast<ALineEdit*>(w);
                           if (!e)
                           {
                               abort("Edit box " + name + " does not exist");
                               return;
                           }
                           if (scriptFunction.isCallable())
                           {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                               abort("Direct call is not supported for Qt5, use function name (e.g. \"doSomething\")");
                               return;
#else
                connect(e, &ALineEdit::editingFinished, Win, [scriptFunction]()
                        {
                            bool err = scriptFunction.call().isError();
                            if (err) AScriptHub::getInstance().abort("Error while executing function call", EScriptLanguage::JavaScript);
                        });
#endif
                           }
                           else
                           {
                               QString functionName = scriptFunction.toString();
                               connect(e, &ALineEdit::textChanged, Win, [functionName]()
                                       {
                                           AScriptHub & hub = AScriptHub::getInstance();
                                           bool ok = hub.getJScriptManager().callFunctionNoArguments(functionName);
                                           if (!ok) AScriptHub::getInstance().abort("Error while executing function call", EScriptLanguage::JavaScript);
                                       });
                           }
                       } );
}

void AGui_JS_SI::comboboxOnSelectionChanged(QString name, QJSValue scriptFunction)
{
    QTimer::singleShot(0, Win, [this, name, scriptFunction]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           QComboBox * b = dynamic_cast<QComboBox*>(w);
                           if (!b)
                           {
                               abort("Combobox " + name + " does not exist");
                               return;
                           }
                           if (scriptFunction.isCallable())
                           {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                               abort("Direct call is not supported for Qt5, use function name (e.g. \"doSomething\")");
                               return;
#else
                connect(b, &QComboBox::currentIndexChanged, Win, [scriptFunction]()
                        {
                            bool err = scriptFunction.call().isError();
                            if (err) AScriptHub::getInstance().abort("Error while executing function call", EScriptLanguage::JavaScript);
                        });
#endif
                           }
                           else
                           {
                               QString functionName = scriptFunction.toString();
                               connect(b, &QComboBox::currentIndexChanged, Win, [functionName]()
                                       {
                                           AScriptHub & hub = AScriptHub::getInstance();
                                           bool ok = hub.getJScriptManager().callFunctionNoArguments(functionName);
                                           if (!ok) AScriptHub::getInstance().abort("Error while executing function call", EScriptLanguage::JavaScript);
                                       });
                           }
                       } );

//        abort("comboboxOnTextChanged() function requires function or its name as the second argument!");
}

void AGui_JS_SI::checkboxOnClick(QString name, QJSValue scriptFunction)
{
    QTimer::singleShot(0, Win, [this, name, scriptFunction]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           QCheckBox * cb = dynamic_cast<QCheckBox*>(w);
                           if (!cb)
                           {
                               abort("Checkbox " + name + " does not exist");
                               return;
                           }
                           if (scriptFunction.isCallable())
                           {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                               abort("Direct call is not supported for Qt5, use function name (e.g. \"doSomething\")");
                               return;
#else
                connect(cb, &QCheckBox::clicked, Win, [scriptFunction]()
                        {
                            bool err = scriptFunction.call().isError();
                            if (err) AScriptHub::getInstance().abort("Error while executing function call", EScriptLanguage::JavaScript);
                        });
#endif
                           }
                           else
                           {
                               QString functionName = scriptFunction.toString();
                               connect(cb, &QCheckBox::clicked, Win, [functionName]()
                                       {
                                           AScriptHub & hub = AScriptHub::getInstance();
                                           bool ok = hub.getJScriptManager().callFunctionNoArguments(functionName);
                                           if (!ok) AScriptHub::getInstance().abort("Error while executing function call", EScriptLanguage::JavaScript);
                                       });
                           }
                       } );

//        abort("checkboxOnClick() function requires function or its name as the second argument!");
}


#ifdef ANTS3_PYTHON
// ---- Python ----

#include "apythonscriptmanager.h"
AGui_Py_SI::AGui_Py_SI(AGuiFromScrWin * win) :
    AGui_SI(win) {}

void AGui_Py_SI::buttonOnClick(QString name, QString scriptFunctionName)
{
    QTimer::singleShot(0, Win, [this, name, scriptFunctionName]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           QPushButton * b = dynamic_cast<QPushButton*>(w);
                           if (!b)
                           {
                               abort("Button " + name + " does not exist");
                               return;
                           }

                           AScriptHub & hub = AScriptHub::getInstance();
                           if (!hub.getPythonManager().isCallable(scriptFunctionName))
                           {
                               abort("Not callable object!");
                               return;
                           }
                           connect(b, &QPushButton::clicked, Win, [scriptFunctionName]()
                                   {
                                       AScriptHub & hub = AScriptHub::getInstance();
                                       bool ok = hub.getPythonManager().callFunctionNoArguments(scriptFunctionName);
                                       if (!ok) AScriptHub::getInstance().abort("Error while executing function call", EScriptLanguage::Python);
                                   });
                       } );
}

void AGui_Py_SI::buttonOnRightClick(QString name, QString scriptFunctionName)
{
    QTimer::singleShot(0, Win, [this, name, scriptFunctionName]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           QPushButton * b = dynamic_cast<QPushButton*>(w);
                           if (!b)
                           {
                               abort("Button " + name + " does not exist");
                               return;
                           }

                           AScriptHub & hub = AScriptHub::getInstance();
                           if (!hub.getPythonManager().isCallable(scriptFunctionName))
                           {
                               abort("Not callable object!");
                               return;
                           }
                           connect(b, &QPushButton::customContextMenuRequested, Win, [scriptFunctionName]()
                                   {
                                       AScriptHub & hub = AScriptHub::getInstance();
                                       bool ok = hub.getPythonManager().callFunctionNoArguments(scriptFunctionName);
                                       if (!ok) AScriptHub::getInstance().abort("Error while executing function call", EScriptLanguage::Python);
                                   });
    } );
}

void AGui_Py_SI::editOnTextChanged(QString name, QString scriptFunctionName)
{
    QTimer::singleShot(0, Win, [this, name, scriptFunctionName]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           ALineEdit * e = dynamic_cast<ALineEdit*>(w);
                           if (!e)
                           {
                               abort("Edit box " + name + " does not exist");
                               return;
                           }

                           AScriptHub & hub = AScriptHub::getInstance();
                           if (!hub.getPythonManager().isCallable(scriptFunctionName))
                           {
                               abort("Not callable object!");
                               return;
                           }
                           connect(e, &ALineEdit::editingFinished, Win, [scriptFunctionName]()
                                   {
                                       AScriptHub & hub = AScriptHub::getInstance();
                                       bool ok = hub.getPythonManager().callFunctionNoArguments(scriptFunctionName);
                                       if (!ok) AScriptHub::getInstance().abort("Error while executing function call", EScriptLanguage::Python);
                                   });
    } );
}

void AGui_Py_SI::comboboxOnSelectionChanged(QString name, QString scriptFunctionName)
{
    QTimer::singleShot(0, Win, [this, name, scriptFunctionName]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           QComboBox * b = dynamic_cast<QComboBox*>(w);
                           if (!b)
                           {
                               abort("Combobox " + name + " does not exist");
                               return;
                           }

                           AScriptHub & hub = AScriptHub::getInstance();
                           if (!hub.getPythonManager().isCallable(scriptFunctionName))
                           {
                               abort("Not callable object!");
                               return;
                           }
                           connect(b, &QComboBox::currentIndexChanged, Win, [scriptFunctionName]()
                                   {
                                       AScriptHub & hub = AScriptHub::getInstance();
                                       bool ok = hub.getPythonManager().callFunctionNoArguments(scriptFunctionName);
                                       if (!ok) AScriptHub::getInstance().abort("Error while executing function call", EScriptLanguage::Python);
                                   });
    } );
}

void AGui_Py_SI::checkboxOnClick(QString name, QString scriptFunctionName)
{
    QTimer::singleShot(0, Win, [this, name, scriptFunctionName]()
                       {
                           QWidget * w = Widgets.value(name, 0);
                           QCheckBox * cb = dynamic_cast<QCheckBox*>(w);
                           if (!cb)
                           {
                               abort("Checkbox " + name + " does not exist");
                               return;
                           }

                           AScriptHub & hub = AScriptHub::getInstance();
                           if (!hub.getPythonManager().isCallable(scriptFunctionName))
                           {
                               abort("Not callable object!");
                               return;
                           }
                           connect(cb, &QCheckBox::clicked, Win, [scriptFunctionName]()
                                   {
                                       AScriptHub & hub = AScriptHub::getInstance();
                                       bool ok = hub.getPythonManager().callFunctionNoArguments(scriptFunctionName);
                                       if (!ok) AScriptHub::getInstance().abort("Error while executing function call", EScriptLanguage::Python);
                                   });
                       } );
}

#endif
