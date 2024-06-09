#include "ascriptwindow.h"
#include "ui_ascriptwindow.h"
#include "atabrecord.h"
#include "ahighlighters.h"
#include "atextedit.h"
#include "ascriptinterface.h"
#include "guitools.h"
#include "ascriptexampleexplorer.h"
#include "ajsontools.h"
#include "afiletools.h"
#include "a3global.h"
#include "ascripthub.h"
#include "ajscriptmanager.h"
#include "atextoutputwindow.h"
#include "amsg_si.h"
#include "avirtualscriptmanager.h"

#ifdef ANTS3_PYTHON
    #include "apythonscriptmanager.h"
#endif

#include <QDebug>
#include <QList>
#include <QSplitter>
#include <QFileDialog>
#include <QMetaMethod>
#include <QPainter>
#include <QPlainTextEdit>
#include <QShortcut>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLineEdit>
#include <QMenu>
#include <QClipboard>
#include <QJsonParseError>
#include <QThread>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QDesktopServices>
#include <QInputDialog>
#include <QHeaderView>
#include <QRegularExpression>
#include <QTextBlock>

AScriptWindow::AScriptWindow(EScriptLanguage lang, QWidget * parent) :
    AGuiWindow( (lang == EScriptLanguage::JavaScript ? "JScript" : "Python"), parent),
    ScriptLanguage(lang),
    ScriptHub(AScriptHub::getInstance()), GlobSet(A3Global::getInstance()),
    ui(new Ui::AScriptWindow)
{
    ui->setupUi(this);

#ifdef ANTS3_PYTHON
    if (lang == EScriptLanguage::JavaScript)
    {
#endif
        ScriptManager = &ScriptHub.getJScriptManager();
        setWindowTitle("JavaScript");
#ifdef ANTS3_PYTHON
    }
    else
    {
        ScriptManager = &ScriptHub.getPythonManager();
        setWindowTitle("Python");
    }
#endif

    ui->pbStop->setVisible(false);
    ui->prbProgress->setValue(0);
    ui->prbProgress->setVisible(false);
    ui->cbActivateTextReplace->setChecked(false);
    ui->frFindReplace->setVisible(false);

    QPixmap rm(16, 16);
    rm.fill(Qt::transparent);
    QPainter b(&rm);
    b.setBrush(QBrush(Qt::red));
    b.drawEllipse(0, 0, 14, 14);
    RedIcon = new QIcon(rm);

    twBooks = new QTabWidget();
    twBooks->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(twBooks, &QTabWidget::customContextMenuRequested, this, &AScriptWindow::twBooks_customContextMenuRequested);
    connect(twBooks, &QTabWidget::currentChanged, this, &AScriptWindow::twBooks_currentChanged);
    connect(twBooks->tabBar(), &QTabBar::tabMoved, this, &AScriptWindow::onBookTabMoved);
    twBooks->setMovable(true);
    twBooks->setTabShape(QTabWidget::Triangular);
    twBooks->setMinimumHeight(25);
    addNewBook();

    createGuiElements();

    //shortcuts
    QShortcut* Run = new QShortcut(QKeySequence("Ctrl+Return"), this);
    connect(Run, &QShortcut::activated, this, &AScriptWindow::on_pbRunScript_clicked);
    QShortcut* Find = new QShortcut(QKeySequence("Ctrl+f"), this);
    connect(Find, &QShortcut::activated, this, &AScriptWindow::on_actionShow_Find_Replace_triggered);
    QShortcut* Replace = new QShortcut(QKeySequence("Ctrl+r"), this);
    connect(Replace, &QShortcut::activated, this, &AScriptWindow::on_actionReplace_widget_Ctr_r_triggered);
    QShortcut* FindFunction = new QShortcut(QKeySequence("F2"), this);
    connect(FindFunction, &QShortcut::activated, this, &AScriptWindow::onFindFunction);
    QShortcut* FindVariable = new QShortcut(QKeySequence("F3"), this);
    connect(FindVariable, &QShortcut::activated, this, &AScriptWindow::onFindVariable);
    QShortcut* GoBack = new QShortcut(QKeySequence("Alt+Left"), this);
    connect(GoBack, &QShortcut::activated, this, &AScriptWindow::onBack);
    QShortcut* GoForward = new QShortcut(QKeySequence("Alt+Right"), this);
    connect(GoForward, &QShortcut::activated, this, &AScriptWindow::onForward);
    QShortcut* DoAlign = new QShortcut(QKeySequence("Ctrl+I"), this);
    connect(DoAlign, &QShortcut::activated, this, [&](){getTab()->TextEdit->align();});
    //QShortcut* DoPaste = new QShortcut(QKeySequence("Ctrl+V"), this);
    //connect(DoPaste, &QShortcut::activated, [&](){getTab()->TextEdit->paste();});

    ATextOutputWindow * SMW = new ATextOutputWindow( (lang == EScriptLanguage::JavaScript ? "JS_Msg" : "Python_Msg"), this );
    SMW->setWindowTitle( lang == EScriptLanguage::JavaScript ? "JS msg window" : "Python msg window");
    ScriptMsgWin = SMW;
    ScriptManager->registerInterface(new AMsg_SI(SMW), "msg");

    //read open script tabs
    ReadFromJson();

    //read database with script examples
    QString recordsFileName = GlobSet.ExamplesDir + "/";
    QString pathToExamples  = GlobSet.ExamplesDir + "/";
    if (ScriptLanguage == EScriptLanguage::JavaScript)
    {
        recordsFileName += "JSExamples.txt";
        pathToExamples  += "/scripts/js/";
    }
    else
    {
        recordsFileName += "PythonExamples.txt";
        pathToExamples  += "/scripts/python/";
    }
    QFile file(recordsFileName);
    if (!file.open(QIODevice::ReadOnly))
        guitools::message("Failed to open file with script example database:\n" + recordsFileName, this);
    else
    {
        //create example explorer
        ExampleExplorer = new AScriptExampleExplorer(recordsFileName, pathToExamples, this);
        Qt::WindowFlags windowFlags = (Qt::Window | Qt::CustomizeWindowHint);
        windowFlags |= Qt::WindowCloseButtonHint;
        ExampleExplorer->setWindowFlags( windowFlags );
        ExampleExplorer->setWindowModality(Qt::WindowModal);
        QObject::connect(ExampleExplorer, &AScriptExampleExplorer::requestLoadScript, this, &AScriptWindow::onLoadRequested);
    }
}

AScriptWindow::~AScriptWindow()
{
    bShutDown = true;

    delete ui;
    delete RedIcon;

    for (auto & sb : ScriptBooks)
    {
        for (ATabRecord * r : sb.Tabs) delete r;
        sb.Tabs.clear();
    }
    ScriptBooks.clear();
}

void AScriptWindow::createGuiElements()
{
    splMain = new QSplitter();  // upper + output with buttons
    splMain->setOrientation(Qt::Vertical);
    splMain->setChildrenCollapsible(false);

    QSplitter* hor = new QSplitter(); //all upper widgets are here
    hor->setContentsMargins(0,0,0,0);
    hor->addWidget(twBooks);

    splHelp = new QSplitter();
    splHelp->setOrientation(Qt::Horizontal);
    splHelp->setChildrenCollapsible(false);
    splHelp->setContentsMargins(0,0,0,0);

    frHelper = new QFrame();
    frHelper->setContentsMargins(1,1,1,1);
    frHelper->setFrameShape(QFrame::NoFrame);
    QVBoxLayout* vb1 = new QVBoxLayout();
    vb1->setContentsMargins(0,0,0,0);

    QSplitter* sh = new QSplitter();
    sh->setOrientation(Qt::Vertical);
    sh->setChildrenCollapsible(false);
    sh->setContentsMargins(0,0,0,0);

    trwHelp = new QTreeWidget();
    trwHelp->setContextMenuPolicy(Qt::CustomContextMenu);
    trwHelp->setColumnCount(1);
    trwHelp->setHeaderLabel("Unit.Function");
    QObject::connect(trwHelp, &QTreeWidget::itemClicked,                this, &AScriptWindow::onFunctionClicked);
    QObject::connect(trwHelp, &QTreeWidget::customContextMenuRequested, this, &AScriptWindow::onContextMenuRequestedByHelp);
    sh->addWidget(trwHelp);

    pteHelp = new QPlainTextEdit();
    pteHelp->setReadOnly(true);
    pteHelp->setMinimumHeight(20);
    sh->addWidget(pteHelp);
    QList<int> sizes;
    sizes << 800 << 250;
    sh->setSizes(sizes);

    vb1->addWidget(sh);

    leFind = new QLineEdit("Find");
    leFind->setMinimumHeight(20);
    leFind->setMaximumHeight(20);
    connect(leFind, &QLineEdit::textChanged, this, &AScriptWindow::onFindTextChanged);
    vb1->addWidget(leFind);

    frHelper->setLayout(vb1);
    splHelp->addWidget(frHelper);
    frHelper->setVisible(false);

    frJsonBrowser = new QFrame();
    frJsonBrowser->setContentsMargins(0,0,0,0);
    frJsonBrowser->setFrameShape(QFrame::NoFrame);
    QVBoxLayout* vbl = new QVBoxLayout();
    vbl->setContentsMargins(0,0,0,0);

    trwJson = new QTreeWidget();
    trwJson->setColumnCount(2);
    trwJson->setMinimumHeight(30);
    trwJson->setMinimumWidth(100);
    QStringList strl;
    strl << "Key" << "Value or type";
    trwJson->setHeaderLabels(strl);
    trwJson->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(trwJson, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(onKeyDoubleClicked(QTreeWidgetItem*,int)));
    QObject::connect(trwJson, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(onKeyClicked(QTreeWidgetItem*,int)));
    QObject::connect(trwJson, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequestedByJsonTree(QPoint)));
    QObject::connect(trwJson, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(onJsonTWExpanded(QTreeWidgetItem*)));
    QObject::connect(trwJson, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(onJsonTWCollapsed(QTreeWidgetItem*)));

    vbl->addWidget(trwJson);

    leFindJ = new QLineEdit("Find");
    leFindJ->setMinimumHeight(20);
    leFindJ->setMaximumHeight(20);
    QObject::connect(leFindJ, &QLineEdit::textChanged, this, &AScriptWindow::onFindTextJsonChanged);
    vbl->addWidget(leFindJ);
    frJsonBrowser->setLayout(vbl);

    splHelp->addWidget(frJsonBrowser);

    sizes.clear();
    sizes << 500 << 500 << 500;
    splHelp->setSizes(sizes);
    frJsonBrowser->setVisible(false);

    hor->addWidget(splHelp);
    hor->setMinimumHeight(60);
    splMain->addWidget(hor);
    //
    pteOut = new QPlainTextEdit();
    pteOut->setMinimumHeight(50);
    pteOut->setReadOnly(true);
    QPalette p = pteOut->palette();
    //p.setColor(QPalette::Active, QPalette::Base, QColor(240,240,240));
    //p.setColor(QPalette::Inactive, QPalette::Base, QColor(240,240,240));
    p.setColor(QPalette::Active, QPalette::Base, p.color(QPalette::AlternateBase));
    p.setColor(QPalette::Inactive, QPalette::Base, p.color(QPalette::AlternateBase));
    pteOut->setPalette(p);
    pteHelp->setPalette(p);
    hor->setSizes(sizes);  // sizes of Script / Help / Config

    splMain->addWidget(pteOut);
    ui->centralwidget->layout()->removeItem(ui->horizontalLayout);
    ui->centralwidget->layout()->addWidget(splMain);
    ui->centralwidget->layout()->addItem(ui->horizontalLayout);

    trwJson->header()->resizeSection(0, 200);

    sizes.clear();
    sizes << 800 << 70;
    splMain->setSizes(sizes);
}

void AScriptWindow::registerInterfaces()
{
    updateMethodHelp();
    updateRemovedAndDeprecatedMethods();
    updateAutocompleterAndHeighlighter();
}

void AScriptWindow::updateMethodHelp()
{
    ListOfMethods.clear();
    trwHelp->clear();
    for (const AScriptInterface * inter : ScriptManager->getInterfaces())
        fillHelper(inter);
}

void AScriptWindow::on_aAlphabeticOrder_changed()
{
    updateMethodHelp();
}

void AScriptWindow::updateRemovedAndDeprecatedMethods()
{
    //DeprecatedOrRemovedMethods.clear();
    //ListOfDeprecatedOrRemovedMethods.clear();
    for (const AScriptInterface * inter : ScriptManager->getInterfaces())
        appendDeprecatedAndRemovedMethods(inter);
}

void AScriptWindow::updateAutocompleterAndHeighlighter()
{
    UnitNames.clear();
    Methods.clear();

    for (const AScriptInterface * inter : ScriptManager->getInterfaces())
    {
        const QString & name = inter->Name;
        UnitNames << name;
        QStringList methodsions = getListOfMethods(inter, name, false);
        for (int i = 0; i < methodsions.size(); i++) methodsions[i] += "()";
        Methods << methodsions;
    }
}

void AScriptWindow::updateGui()
{
    for (AScriptBook & book : ScriptBooks)
        for (ATabRecord * tab : book.Tabs)
            updateTab(tab);

    trwHelp->collapseAll();

    updateJsonTree();

    updateFileStatusIndication();
}

void AScriptWindow::reportError(QString error, int line)
{
    //error = "<font color=\"red\">Error:</font><br>" + error;
    error = "<font color=\"red\">" + error + "</font>";
    pteOut->appendHtml(error);
    qDebug() << "ln:" << line;
    highlightErrorLine(line);
}

void AScriptWindow::highlightErrorLine(int line)
{
    if (line < 0) return;
    line--;

    ATextEdit * te = getTab()->TextEdit;
    if (!te) return;

    QTextBlock block = te->document()->findBlockByLineNumber(line);
    int loc = block.position();
    QTextCursor cur = te->textCursor();
    cur.setPosition(loc);
    te->setTextCursor(cur);
    te->ensureCursorVisible();

    int length = block.text().split("\n").at(0).length();
    cur.movePosition(cur.Right, cur.KeepAnchor, length);

    QTextCharFormat tf = block.charFormat();
    tf.setBackground(QBrush(Qt::yellow));
    QTextEdit::ExtraSelection es;
    es.cursor = cur;
    es.format = tf;

    QList<QTextEdit::ExtraSelection> esList;
    esList << es;
    te->setExtraSelections(esList);
}

void AScriptWindow::WriteToJson()
{
    if (ScriptLanguage == EScriptLanguage::JavaScript) writeToJson(GlobSet.JavaScriptJson);
    else                                                   writeToJson(GlobSet.PythonJson);
}

void AScriptWindow::writeToJson(QJsonObject & json)
{
    json = QJsonObject(); //clear

    QJsonArray ar;
    for (const AScriptBook & b : ScriptBooks)
    {
        QJsonObject js;
        b.writeToJson(js);
        ar << js;
    }
    json["ScriptBooks"] = ar;
    json["CurrentBook"] = iCurrentBook;

    QJsonArray sar;
    for (int & i : splMain->sizes()) sar << i;
    json["Sizes"] = sar;
}

void AScriptWindow::ReadFromJson()
{
    if (ScriptLanguage == EScriptLanguage::JavaScript) readFromJson(GlobSet.JavaScriptJson);
    else                                               readFromJson(GlobSet.PythonJson);
}

void AScriptWindow::removeAllBooksExceptFirst()
{
    for (int i = (int)ScriptBooks.size() - 1; i > 0; i--) removeBook(i, false);
    ScriptBooks[0].removeAllTabs();
}

void AScriptWindow::readFromJson(QJsonObject & json)
{
    if (json.isEmpty()) return;

    removeAllBooksExceptFirst();

    QJsonArray ar = json["ScriptBooks"].toArray();
    for (int iBook = 0; iBook < ar.size(); iBook++)
    {
        if (iBook != 0) addNewBook();
        QJsonObject js = ar[iBook].toObject();
        loadBook(iBook, js);
    }

    int iCur = 0;
    jstools::parseJson(json, "CurrentBook", iCur);
    if (iCur < 0 || iCur >= (int)ScriptBooks.size()) iCur = 0;
    iCurrentBook = iCur;
    twBooks->setCurrentIndex(iCurrentBook);

    updateFileStatusIndication();

    if (json.contains("Sizes"))
    {
        QJsonArray sar = json["Sizes"].toArray();
        if (sar.size() == 2)
        {
            QList<int> sizes;
            sizes << sar.at(0).toInt(50) << sar.at(1).toInt(50);
            splMain->setSizes(sizes);
        }
    }
}

void AScriptWindow::onBusyOn()
{
    ui->pbRunScript->setEnabled(false);
}

void AScriptWindow::onBusyOff()
{
    ui->pbRunScript->setEnabled(true);
}

void AScriptWindow::outputHtml(QString text)
{
    pteOut->appendHtml(text);
    qApp->processEvents();
}

void AScriptWindow::outputText(QString text)
{
    pteOut->appendPlainText(text);
    qApp->processEvents();
}

void AScriptWindow::outputFromBuffer(std::vector<std::pair<bool, QString>> buffer)
{
    for (const auto & p : buffer)
    {
        if (p.first) pteOut->appendHtml(p.second);
        else         pteOut->appendPlainText(p.second);
    }
}

void AScriptWindow::outputAbortMessage(QString text)
{
    pteOut->appendHtml("<p style='color:red;'>Aborted: " + text + "</p>");
    qApp->processEvents();
}

void AScriptWindow::onRequestAddScript(const QString & script)
{
    showNormal();
    raise();
    activateWindow();

    onLoadRequested(script);
}

void AScriptWindow::clearOutput()
{
    pteOut->clear();
    qApp->processEvents();
}

#include "acore_si.h"
void AScriptWindow::on_pbRunScript_clicked()
{
    // save all tabs -> GlobSet
    WriteToJson();
    A3Global::getInstance().saveConfig();
    emit requestUpdateConfig();

    const QString Script = getTab()->TextEdit->document()->toPlainText();

    pteOut->clear();
    ui->pbStop->setVisible(true);
    ui->pbRunScript->setVisible(false);

    ScriptManager->evaluate(Script);
    do
    {
        QThread::msleep(50);
        qApp->processEvents();
    }
    while (ScriptManager->isRunning());

    ui->pbStop->setVisible(false);
    ui->pbRunScript->setVisible(true);

    if (ScriptManager->isError())
    {
        QString err = ScriptManager->getErrorDescription();
        //qDebug() << "->->->->-->" << err << ScriptManager->getErrorLineNumber() << ScriptManager->isAborted();
        if (!ScriptManager->isAborted())
            reportError(err, ScriptManager->getErrorLineNumber());
    }
    else
    {
        QString s;
        const QVariant res = ScriptManager->getResult();
        AVirtualScriptManager::addQVariantToString(res, s, ScriptLanguage);
        if (!s.simplified().isEmpty() && s != "undefined") outputText(s);
    }

    ScriptManager->collectGarbage();

    updateJsonTree();

    emit requestUpdateGui();
}

void AScriptWindow::onF1pressed(QString text)
{
    //qDebug() << "F1 requested for:"<<text;
    ui->pbHelp->setChecked(true);

    trwHelp->collapseAll();
    trwHelp->clearSelection();

    QList<QTreeWidgetItem*> list;
    list = trwHelp->findItems(text, Qt::MatchContains | Qt::MatchRecursive, 0);

    for (int i=0; i<list.size(); i++)
    {
        QTreeWidgetItem* item = list[i];
        do
        {
            trwHelp->expandItem(item);
            item = item->parent();
        }
        while (item);
        trwHelp->setCurrentItem(list[i], 0, QItemSelectionModel::Select);
        trwHelp->setCurrentItem(list[i], 1, QItemSelectionModel::Select);
    }

    if (list.size() == 1)
        emit trwHelp->itemClicked(list.first(), 0);
}

void AScriptWindow::on_pbStop_clicked()
{
    if (ScriptManager->isRunning())
    {
        //qDebug() << "Stop button pressed!";
        //AScriptHub::abort("<p style='color:red'>Aborting...</p>", ScriptLanguage);
        AScriptHub::abort("Stop script eval clicked", ScriptLanguage);
        qApp->processEvents();
    }
}

void AScriptWindow::on_pbLoad_clicked()
{
    QString starter = ""; // (GlobSet.LibScripts.isEmpty()) ? GlobSet.LastOpenDir : GlobSet.LibScripts;
    QString fileName = QFileDialog::getOpenFileName(this, "Load script", starter, "Text files (*.txt);;All files (*.*)");
    if (fileName.isEmpty()) return;

    QString Script;
    bool ok = ftools::loadTextFromFile(Script, fileName);
    if (!ok)
    {
        guitools::message("Could not open: " + fileName, this);
        return;
    }

    onLoadRequested(Script);

    getTab()->FileName = fileName;
    setTabName(QFileInfo(fileName).baseName(), getCurrentTabIndex(), iCurrentBook);
    updateFileStatusIndication();
}

void AScriptWindow::onLoadRequested(const QString & script)
{
    ATabRecord * tab = getTab();
    if (!tab->TextEdit->document()->isEmpty()) tab = &addNewTab();

    tab->TextEdit->clear();
    tab->TextEdit->appendPlainText(script);

    //for example load (triggered on signal from example explorer): do not register file name!
    tab->FileName.clear();
    setTabName( createNewTabName(iCurrentBook), getCurrentTabIndex(), iCurrentBook );
    updateFileStatusIndication();
}

void AScriptWindow::on_pbSave_clicked()
{
    QString FileName = getTab()->FileName;
    if (FileName.isEmpty())
    {
        on_pbSaveAs_clicked();
        return;
    }

    bool ok = ftools::saveTextToFile(getTab()->TextEdit->document()->toPlainText(), FileName);
    if(!ok)
    {
        guitools::message("Unable to open file " + FileName + " for writing!", this);
        return;
    }

    if (!getTab()->bExplicitlyNamed)
        setTabName(QFileInfo(FileName).baseName(), getCurrentTabIndex(), iCurrentBook);

    getTab()->setModifiedStatus(false);
    updateFileStatusIndication();
}

void AScriptWindow::on_pbSaveAs_clicked()
{
    QString starter = ""; //(GlobSet.LibScripts.isEmpty()) ? GlobSet.LastOpenDir : GlobSet.LibScripts;
    if (!getTab()->FileName.isEmpty()) starter = getTab()->FileName;
    QString fileName = QFileDialog::getSaveFileName(this,"Save script", starter, "Script files (*.txt *.js);;All files (*.*)");
    if (fileName.isEmpty()) return;

    QFileInfo fileInfo(fileName);
    if(fileInfo.suffix().isEmpty()) fileName += ".txt";

    getTab()->FileName = fileName;
    on_pbSave_clicked();
}

void AScriptWindow::on_pbExample_clicked()
{
    if (ExampleExplorer) ExampleExplorer->showNormal();
}

/*
void AScriptWindow::fillHelper(const AScriptInterface * io)
{
    const QString module = io->Name;

    QStringList functions = getListOfMethods(io, module, true);
    if (ui->aAlphabeticOrder->isChecked()) functions.sort();

    QTreeWidgetItem * objItem = new QTreeWidgetItem(trwHelp);
    objItem->setText(0, module);
    QFont f = objItem->font(0);
    f.setBold(true);
    objItem->setFont(0, f);
    objItem->setToolTip(0, io->Description);
    bool bAlreadyAdded = false; // TMP!!!
    for (int i = 0; i < functions.size(); i++)
    {
        QStringList sl = functions.at(i).split("_:_");
        QString Fshort = sl.first();
        QString Flong  = sl.last();
        functionList << Flong;

        QString methodName = QString(Fshort).remove(QRegularExpression("\\((.*)\\)"));
        methodName.remove(0, module.length() + 1); //remove module name and '.'

        // TMP!!! !!!***
        if (methodName == "print")
        {
            if (bAlreadyAdded) continue;
            Fshort = "core.print( m1, ... )";
            Flong  = "void core.print( QVariant m1, ... )";
            bAlreadyAdded = true;
        }

        QTreeWidgetItem * fItem = new QTreeWidgetItem(objItem);
        fItem->setText(0, Fshort);
        fItem->setText(1, Flong);

        const QString & str = io->getMethodHelp(methodName, -1);
        fItem->setToolTip(0, str);
    }
}
*/

void AScriptWindow::fillHelper(const AScriptInterface * io)
{
    const QString module = io->Name;

    std::vector<std::pair<QString,int>> methods = getListOfMethodsWithNumArgs(io);
    if (ui->aAlphabeticOrder->isChecked()) std::sort(methods.begin(), methods.end(), [](const auto & lhs, const auto & rhs){return (lhs.first < rhs.first);});

    QTreeWidgetItem * objItem = new QTreeWidgetItem(trwHelp);
    objItem->setText(0, module);
    QFont f = objItem->font(0);
    f.setBold(true);
    objItem->setFont(0, f);
    objItem->setToolTip(0, io->Description);
    bool bAlreadyAdded = false; // TMP!!!
    for (size_t iMet = 0; iMet < methods.size(); iMet++)
    {
        QStringList sl = methods[iMet].first.split("_:_");
        QString Fshort = sl.first();
        QString Flong  = sl.last();
        ListOfMethods.push_back( {Flong, methods[iMet].second} );

        QString methodName = QString(Fshort).remove(QRegularExpression("\\((.*)\\)"));
        methodName.remove(0, module.length() + 1); //remove module name and '.'

        if (methodName == "print")
        {
            if (bAlreadyAdded) continue;
            Fshort = "core.print( m1, ... )";
            Flong  = "void core.print( QVariant m1, ... )";
            bAlreadyAdded = true;
        }

        QTreeWidgetItem * fItem = new QTreeWidgetItem(objItem);
        fItem->setText(0, Fshort);
        fItem->setText(1, Flong);  // long is used when a method is clicked in GUI to show complete signature

        const QString & str = io->getMethodHelp(methodName, methods[iMet].second);
        fItem->setToolTip(0, str);
    }
}

void AScriptWindow::onJsonTWExpanded(QTreeWidgetItem *item)
{
    ExpandedItemsInJsonTW << item->text(0);
}

void AScriptWindow::onJsonTWCollapsed(QTreeWidgetItem *item)
{
    ExpandedItemsInJsonTW.remove(item->text(0));
}

#include "aconfig.h"
void AScriptWindow::updateJsonTree()
{
    AConfig & Config = AConfig::getInstance();
    Config.updateJSONfromConfig();

    trwJson->clear();

    const QJsonObject & json = AConfig::getConstInstance().JSON;
    QJsonObject::const_iterator it;
    for (it = json.begin(); it != json.end(); ++it)
    {
        QString key = it.key();
        QTreeWidgetItem *TopKey = new QTreeWidgetItem(trwJson);
        TopKey->setText(0, key);

        const QJsonValue &value = it.value();
        TopKey->setText(1, getDesc(value));

        if (value.isObject())
            fillSubObject(TopKey, value.toObject());
        else if (value.isArray())
            fillSubArray(TopKey, value.toArray());
    }

    //restoring expanded status
    QSet<QString> expanded = ExpandedItemsInJsonTW;
    ExpandedItemsInJsonTW.clear();
    foreach (QString s, expanded)
    {
        QList<QTreeWidgetItem*> l = trwJson->findItems(s, Qt::MatchExactly | Qt::MatchRecursive);
        foreach (QTreeWidgetItem* item, l)
            item->setExpanded(true);
    }
}

void AScriptWindow::fillSubObject(QTreeWidgetItem *parent, const QJsonObject &obj)
{
    QJsonObject::const_iterator it;
    for (it = obj.begin(); it != obj.end(); ++it)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, it.key());
        QJsonValue value = it.value();
        item->setText(1, getDesc(value));

        if (value.isObject())
            fillSubObject(item, value.toObject());
        else if (value.isArray())
            fillSubArray(item, value.toArray());
    }
}

void AScriptWindow::fillSubArray(QTreeWidgetItem *parent, const QJsonArray &arr)
{
    for (int i=0; i<arr.size(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        //QString str = "[" + QString::number(i)+"]";
        QString str = parent->text(0)+"[" + QString::number(i)+"]";
        item->setText(0, str);
        QJsonValue value = arr.at(i);
        item->setText(1, getDesc(value));

        if (value.isObject())
            fillSubObject(item, value.toObject());
        else if (value.isArray())
            fillSubArray(item, value.toArray());
    }
}

QString AScriptWindow::getDesc(const QJsonValue &ref)
{
    if (ref.isBool())
    {
        bool f = ref.toBool();
        return (f ? "true" : "false");
        //return "bool";
    }
    if (ref.isDouble())
    {
        double v = ref.toDouble();
        QString s = QString::number(v);
        return s;
        //return "number";
    }
    if (ref.isString())
    {
        QString s = ref.toString();
        s = "\"" + s + "\"";
        if (s.length()<100) return s;
        else return "string";
    }
    if (ref.isObject()) return "obj";
    if (ref.isArray())
    {
        int size = ref.toArray().size();
        QString ret = "array[" + QString::number(size)+"]";
        return ret;
    }
    return "undefined";
}

void AScriptWindow::onFunctionClicked(QTreeWidgetItem *item, int /*column*/)
{
    pteHelp->clear();
    //qDebug() << item->text(1);
    //QString returnType = getFunctionReturnType(item->text(0));
    //pteHelp->appendPlainText(returnType+ "  " +item->text(0)+":");

    //pteHelp->appendHtml("<b>" + item->text(1) + "</b>");
    //pteHelp->appendHtml("<p style=\"color:blue;\"> " + item->text(1) + "</p>");
    pteHelp->appendPlainText(item->text(1)+"\n");
    pteHelp->appendPlainText(item->toolTip(0));
}

void AScriptWindow::onKeyDoubleClicked(QTreeWidgetItem *item, int /*column*/)
{
    //QString str = getKeyPath(item);
    //cteScript->insertPlainText(str);
    if (!item) return;
    showContextMenuForJsonTree(item, trwJson->mapFromGlobal(cursor().pos()));
}

QString AScriptWindow::getKeyPath(QTreeWidgetItem *item)
{
    if (!item) return "";

    QString path;
    int SkipOnArray = 0;
    do
    {
        if (SkipOnArray != 0) SkipOnArray--;
        else
        {
            QString thisPart = item->text(0);
            SkipOnArray = thisPart.count('[');
            path = thisPart + "." + path;
        }
        item = item->parent();
    }
    while (item);

    path.chop(1);
    return path;
}

void AScriptWindow::onKeyClicked(QTreeWidgetItem* /*item*/, int /*column*/)
{
    //trwJson->resizeColumnToContents(column);
}

void AScriptWindow::onFindTextChanged(const QString &arg1)
{
    QTreeWidget* tw = trwHelp;

    tw->collapseAll();
    tw->clearSelection();
    if (arg1.length()<3) return;

    QList<QTreeWidgetItem*> list;
    list = tw->findItems(arg1, Qt::MatchContains | Qt::MatchRecursive, 0);

    for (int i=0; i<list.size(); i++)
    {
        QTreeWidgetItem* item = list[i];
        do
        {
            tw->expandItem(item);
            item = item->parent();
        }
        while (item);
        tw->setCurrentItem(list[i], 0, QItemSelectionModel::Select);
        tw->setCurrentItem(list[i], 1, QItemSelectionModel::Select);
    }
}

void AScriptWindow::onFindTextJsonChanged(const QString &arg1)
{
    QTreeWidget* tw = trwJson;

    tw->collapseAll();
    tw->clearSelection();
    if (arg1.length()<3) return;

    QList<QTreeWidgetItem*> list;
    list = tw->findItems(arg1, Qt::MatchContains | Qt::MatchRecursive, 0);

    for (int i=0; i<list.size(); i++)
    {
        QTreeWidgetItem* item = list[i];
        do
        {
            tw->expandItem(item);
            item = item->parent();
        }
        while (item);
        tw->setCurrentItem(list[i], 0, QItemSelectionModel::Select);
        tw->setCurrentItem(list[i], 1, QItemSelectionModel::Select);
    }
}

void AScriptWindow::onContextMenuRequestedByJsonTree(QPoint pos)
{
    QTreeWidgetItem *item = trwJson->itemAt(pos);
    if (!item) return;

    showContextMenuForJsonTree(item, pos);
}

void AScriptWindow::showContextMenuForJsonTree(QTreeWidgetItem *item, QPoint pos)
{
    QMenu menu;

    QAction* plainKey = menu.addAction("Add Key path to clipboard");
    menu.addSeparator();
    QAction* keyQuatation = menu.addAction("Add Key path to clipboard: in quatations");
    QAction* keyGet = menu.addAction("Add Key path to clipboard: get key value command");
    QAction* keySet = menu.addAction("Add Key path to clipboard: replace key value command");
    //menu.addSeparator();

    QAction* sa = menu.exec(trwJson->mapToGlobal(pos));
    if (!sa) return;

    QClipboard * clipboard = QApplication::clipboard();
    QString text = getKeyPath(item);
    if (sa == plainKey) ;
    else if (sa == keyQuatation) text = "\"" + text + "\"";
    else if (sa == keyGet) text = "config.getKeyValue(\"" + text + "\")";
    else if (sa == keySet) text = "config.replace(\"" + text + "\", newValue)";
    clipboard->setText(text);
}

void AScriptWindow::onContextMenuRequestedByHelp(QPoint pos)
{
    QTreeWidgetItem *item = trwHelp->itemAt(pos);
    if (!item) return;
    QString str = item->text(0);

    QMenu menu;
    QAction* toClipboard = menu.addAction("Add function to clipboard");

    QAction* selectedItem = menu.exec(trwHelp->mapToGlobal(pos));
    if (!selectedItem) return; //nothing was selected

    if (selectedItem == toClipboard)
    {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(str);
    }
}

//void AScriptWindow::onFunctionDoubleClicked(QTreeWidgetItem *item, int /*column*/)
//{
//  QString text = item->text(0);
//  getTab()->TextEdit->insertPlainText(text);
//}

/*
void AScriptWindow::closeEvent(QCloseEvent* e)
{
    QString Script = getTab()->TextEdit->document()->toPlainText();

    QMainWindow::closeEvent(e);
}
*/

bool AScriptWindow::event(QEvent *e)
{
    switch(e->type())
    {
    case QEvent::WindowActivate :
        // gained focus
        //qDebug() << "Focussed!";
        updateJsonTree();
        break;
    case QEvent::WindowDeactivate :
        // lost focus
        break;
/*
    case QEvent::Hide :
        //qDebug() << "Script window: hide event";
        ScriptManager->hideMsgDialogs();
        break;
    case QEvent::Show :
        //qDebug() << "Script window: show event";
        ScriptManager->restoreMsgDialogs();
        break;
*/
    default:;
    };

    return QMainWindow::event(e);
}

void AScriptWindow::receivedOnAbort()
{
    ui->prbProgress->setValue(0);
    ui->prbProgress->setVisible(false);
    emit onAbort();
}

void AScriptWindow::receivedOnSuccess(QString eval)
{
    ui->prbProgress->setValue(0);
    ui->prbProgress->setVisible(false);
    emit success(eval);
//    emit onFinish(ScriptManager->isUncaughtException());
}

void AScriptWindow::onDefaulFontSizeChanged(int size)
{
    GlobSet.SW_FontSize = size;
    for (ATabRecord* tab : getScriptTabs())
        tab->TextEdit->SetFontSize(size);
}

void AScriptWindow::onProgressChanged(int percent)
{
    ui->prbProgress->setValue(percent);
    ui->prbProgress->setVisible(true);
    qApp->processEvents();
}

QStringList AScriptWindow::getListOfMethods(const QObject * obj, QString ObjName, bool fWithArguments)
{
    QStringList methods;
    if (!obj) return methods;

    const int numMethods = obj->metaObject()->methodCount();
    for (int iMet = 0; iMet < numMethods; iMet++)
    {
        const QMetaMethod & m = obj->metaObject()->method(iMet);
        bool bSlot   = (m.methodType() == QMetaMethod::Slot);
        bool bPublic = (m.access() == QMetaMethod::Public);
        QString candidate, extra;
        if (bSlot && bPublic)
        {
            if (m.name() == "deleteLater") continue;
            if (m.name() == "help") continue;

            if (ObjName.isEmpty()) candidate = m.name();
            else                   candidate = ObjName + "." + m.name();

            if (fWithArguments)
            {
                candidate += "(";
                extra = candidate;

                int args = m.parameterCount();
                for (int i = 0; i < args; i++)
                {
                    QString typ = m.parameterTypes().at(i);
                    if (typ == "QString") typ = "string";
                    extra += " " + typ + " " + m.parameterNames().at(i);
                    candidate     += " " + m.parameterNames().at(i);
                    if (i != args-1)
                    {
                        candidate += ", ";
                        extra += ", ";
                    }
                }
                candidate += " )";
                extra += " )";
                extra = QString() + m.typeName() + " " + extra;

                candidate += "_:_" + extra;
            }

            if (methods.isEmpty() || methods.last() != candidate)
                methods << candidate;
        }
    }
    return methods;
}

std::vector<std::pair<QString, int>> AScriptWindow::getListOfMethodsWithNumArgs(const AScriptInterface * interface)
{
    std::vector<std::pair<QString, int>> vec;
    if (!interface) return vec;

    const int numMethods = interface->metaObject()->methodCount();
    for (int iMet = 0; iMet < numMethods; iMet++)
    {
        const QMetaMethod & m = interface->metaObject()->method(iMet);
        bool bSlot   = (m.methodType() == QMetaMethod::Slot);
        bool bPublic = (m.access() == QMetaMethod::Public);

        if (bSlot && bPublic)
        {
            if (m.name() == "deleteLater") continue;
            if (m.name() == "help") continue;

            QString candidate = interface->Name + "." + m.name();
            int args = m.parameterCount();

            if (true) // fWithArguments)
            {
                candidate += "(";
                QString extra = candidate;

                for (int i = 0; i < args; i++)
                {
                    QString typ = m.parameterTypes().at(i);
                    if (typ == "QString") typ = "string";
                    extra += " " + typ + " " + m.parameterNames().at(i);
                    candidate     += " " + m.parameterNames().at(i);
                    if (i != args-1)
                    {
                        candidate += ", ";
                        extra += ", ";
                    }
                }
                candidate += " )";
                extra += " )";
                extra = QString() + m.typeName() + " " + extra;

                candidate += "_:_" + extra;
            }

            if (vec.empty() || vec.back().first != candidate)
                vec.push_back( {candidate, args} );
        }
    }

    return vec;
}

void AScriptWindow::appendDeprecatedAndRemovedMethods(const AScriptInterface * obj)
{
/*
    const QString name = obj->Name;
    QHashIterator<QString, QString> iter(obj->getDeprecatedOrRemovedMethods());
    while (iter.hasNext())
    {
        iter.next();

        QString key = iter.key();
        if (!name.isEmpty()) key = name + "." + key;

        DeprecatedOrRemovedMethods[key] = iter.value();
        ListOfDeprecatedOrRemovedMethods << key;
    }
*/
}

// ------------------------

void AScriptWindow::onCurrentTabChanged(int tab)
{
    if (bShutDown || tab < 0) return;
    //qDebug() << "Current changed for visible!" << tab;
    updateFileStatusIndication();
    applyTextFindState();
}

QIcon makeIcon(int h)
{
    QPixmap pm(h-2, h-2);
    pm.fill(Qt::transparent);
    QPainter b(&pm);
    b.setBrush(QBrush(Qt::yellow));
    b.drawEllipse(0, 2, h-5, h-5);
    return QIcon(pm);
}

void AScriptWindow::updateFileStatusIndication()
{
    ATabRecord * tab = getTab();
    if (!tab) return;

    QString fileName  = tab->FileName;
    bool bWasModified = tab->wasModified();

    ui->labNotSaved->setVisible(fileName.isEmpty());

    QString s;
    if (fileName.isEmpty())
        ui->labWasModified->setVisible(false);
    else
    {
        ui->labWasModified->setVisible(bWasModified);
        s = fileName;
    }
    ui->pbFileName->setText(s);
}

void AScriptWindow::on_pbFileName_clicked()
{
    QString s = getTab()->FileName;
    QFileInfo fi(s);
    QString path = fi.path();
    pteOut->appendPlainText(path);
    QDesktopServices::openUrl(QUrl("file:///"+path, QUrl::TolerantMode));
}

void AScriptWindow::onRequestTabWidgetContextMenu(QPoint pos)
{
    if (pos.isNull()) return;

    QMenu menu;
    int tab = getTabWidget()->tabBar()->tabAt(pos);

    QAction* add = menu.addAction("Add new tab");
    menu.addSeparator();
    QAction* rename = (tab == -1 ? nullptr : menu.addAction("Rename tab") );
    menu.addSeparator();
    QAction* remove = (tab == -1 ? nullptr : menu.addAction("Close tab") );
    menu.addSeparator();
    QAction * markTabA = (tab == -1 ? nullptr : menu.addAction("Mark this tab for copy/move") );
    menu.addSeparator();
    QAction * copyTabA = menu.addAction("Copy marked tab to this book"); copyTabA->setEnabled(iMarkedTab != -1);

    QAction* selectedItem = menu.exec(getTabWidget()->mapToGlobal(pos));
    if (!selectedItem) return; //nothing was selected

    if (selectedItem == add)            addNewTab();
    else if (selectedItem == remove)    askRemoveTab(tab);
    else if (selectedItem == rename)    renameTab(tab);
    else if (selectedItem == markTabA)  markTab(tab);
    else if (selectedItem == copyTabA)  pasteMarkedTab();
}

void AScriptWindow::onScriptTabMoved(int from, int to)
{
    //qDebug() << "Form->to:"<<from<<to;
    std::vector<ATabRecord*> & vec = getScriptTabs();
    std::swap(vec[from], vec[to]);

    iMarkedTab = -1;
}

void AScriptWindow::updateTab(ATabRecord* tab)
{
    tab->Highlighter->setExternalRules(UnitNames, Methods, ListOfDeprecatedOrRemovedMethods);
    tab->updateHighlight();
    tab->TextEdit->ListOfMethods = &ListOfMethods;
    tab->TextEdit->DeprecatedOrRemovedMethods = &DeprecatedOrRemovedMethods;
}

ATabRecord & AScriptWindow::addNewTab(int iBook)
{
    ATabRecord * tab = new ATabRecord(Methods, ScriptLanguage);
    tab->TabName = createNewTabName(iBook);

    formatTab(tab);

    getScriptTabs(iBook).push_back(tab);
    getTabWidget(iBook)->addTab(tab->TextEdit, tab->TabName);

    int index = countTabs(iBook) - 1;
    setCurrentTabIndex(index, iBook);

    return *tab;
}

ATabRecord & AScriptWindow::addNewTab()
{
    return addNewTab(iCurrentBook);
}

void AScriptWindow::formatTab(ATabRecord * tab)
{
    updateTab(tab);

    if (GlobSet.SW_FontFamily.isEmpty())
        tab->TextEdit->SetFontSize(GlobSet.SW_FontSize);
    else
    {
        QFont font(GlobSet.SW_FontFamily, GlobSet.SW_FontSize, GlobSet.SW_FontWeight, GlobSet.SW_Italic);
        tab->TextEdit->setFont(font);
    }

    connect(tab->TextEdit, &ATextEdit::fontSizeChanged, this, &AScriptWindow::onDefaulFontSizeChanged);
    connect(tab->TextEdit, &ATextEdit::requestHelp, this, &AScriptWindow::onF1pressed);
    connect(tab->TextEdit->document(), &QTextDocument::modificationChanged, this, &AScriptWindow::updateFileStatusIndication);
    connect(tab, &ATabRecord::requestFindText, this, &AScriptWindow::onFindSelected);
    connect(tab, &ATabRecord::requestReplaceText, this, &AScriptWindow::onReplaceSelected);
    connect(tab, &ATabRecord::requestFindFunction, this, &AScriptWindow::onFindFunction);
    connect(tab, &ATabRecord::requestFindVariable, this, &AScriptWindow::onFindVariable);
}

QString AScriptWindow::createNewTabName(int iBook)
{
    int counter = 1;
    QString res;
    bool fFound;
    do
    {
        fFound = false;
        res = QString("new_%1").arg(counter);
        for (int i = 0; i < countTabs(iBook); i++)
            if ( getTabWidget(iBook)->tabText(i) == res )
            {
                fFound = true;
                counter++;
                break;
            }
    }
    while (fFound);
    return res;
}

QString AScriptWindow::createNewBookName()
{
    int counter = 1;
    QString res;
    bool fFound;
    do
    {
        fFound = false;
        res = QString("Book%1").arg(counter);
        for (const AScriptBook & b : ScriptBooks)
            if (b.Name == res)
            {
                fFound = true;
                counter++;
                break;
            }
    }
    while (fFound);
    return res;
}

void AScriptWindow::removeTab(int tab)
{
    ScriptBooks[iCurrentBook].removeTab(tab);

    if (countTabs() == 0) addNewTab();
    updateFileStatusIndication();

    iMarkedTab = -1;
}

void AScriptWindow::on_pbConfig_toggled(bool checked)
{
    frJsonBrowser->setVisible(checked);
}

void AScriptWindow::on_pbHelp_toggled(bool checked)
{
    frHelper->setVisible(checked);
}

void AScriptWindow::on_actionIncrease_font_size_triggered()
{
    onDefaulFontSizeChanged(++GlobSet.SW_FontSize);
}

void AScriptWindow::on_actionDecrease_font_size_triggered()
{
    if (GlobSet.SW_FontSize < 1) return;

    onDefaulFontSizeChanged(--GlobSet.SW_FontSize);
    //qDebug() << "New font size:"<<GlobSet.DefaultFontSize_ScriptWindow;
}

#include <QFontDialog>
void AScriptWindow::on_actionSelect_font_triggered()
{
    bool ok;
    QFont font = QFontDialog::getFont(
                &ok,
                QFont(GlobSet.SW_FontFamily,
                      GlobSet.SW_FontSize,
                      GlobSet.SW_FontWeight,
                      GlobSet.SW_Italic),
                this);
    if (!ok) return;

    GlobSet.SW_FontFamily = font.family();
    GlobSet.SW_FontSize   = font.pointSize();
    GlobSet.SW_FontWeight = font.weight();
    GlobSet.SW_Italic     = font.italic();
    GlobSet.saveConfig();

    for (ATabRecord* tab : getScriptTabs())
        tab->TextEdit->setFont(font);
}

/*
void AScriptWindow::on_actionShow_all_messenger_windows_triggered()
{
    ScriptManager->restoreMsgDialogs();
}

void AScriptWindow::on_actionHide_all_messenger_windows_triggered()
{
    ScriptManager->hideMsgDialogs();
}

void AScriptWindow::on_actionClear_unused_messenger_windows_triggered()
{
    AJavaScriptManager* JSM = dynamic_cast<AJavaScriptManager*>(ScriptManager);
    if (JSM) JSM->clearUnusedMsgDialogs();
}

void AScriptWindow::on_actionClose_all_messenger_windows_triggered()
{
    AJavaScriptManager* JSM = dynamic_cast<AJavaScriptManager*>(ScriptManager);
    if (JSM) JSM->closeAllMsgDialogs();
}
*/

void AScriptWindow::on_actionAdd_new_tab_triggered()
{
    addNewTab();
}

void AScriptWindow::askRemoveTab(int tab)
{
    if (tab < 0 || tab >= countTabs()) return;

    bool ok = guitools::confirm("Close tab " + getTab(tab)->TabName + "?");
    if (ok) removeTab(tab);
}

void AScriptWindow::renameTab(int tab)
{
    ATabRecord * tr = getTab(tab);
    if (!tr) return;

    bool ok;
    QString text = QInputDialog::getText(this, "Input text",
                                         "New name for the tab:", QLineEdit::Normal,
                                         tr->TabName, &ok);
    if (ok && !text.isEmpty())
    {
        setTabName(text, tab, iCurrentBook);
        tr->bExplicitlyNamed = true;
    }
}

void AScriptWindow::markTab(int tab)
{
    iMarkedBook = iCurrentBook;
    iMarkedTab = tab;
}

void AScriptWindow::pasteMarkedTab()
{
    if (iMarkedTab == -1) return;

    QString script = ScriptBooks[iMarkedBook].getTab(iMarkedTab)->TextEdit->document()->toPlainText();
    QString newName = "CopyOf_" + ScriptBooks[iMarkedBook].getTab(iMarkedTab)->TabName;

    ATabRecord & tab = addNewTab(iCurrentBook);

    tab.TextEdit->appendPlainText(script);
    setTabName(newName, countTabs(iCurrentBook)-1, iCurrentBook);
}

void AScriptWindow::copyTab(int iBook)
{
    if (iMarkedTab == -1) return;

    QString script = ScriptBooks[iMarkedBook].getTab(iMarkedTab)->TextEdit->document()->toPlainText();
    QString newName = "CopyOf_" + ScriptBooks[iMarkedBook].getTab(iMarkedTab)->TabName;

    ATabRecord & tab = addNewTab(iBook);

    tab.TextEdit->appendPlainText(script);
    setTabName(newName, countTabs(iBook)-1, iBook);
}

void AScriptWindow::moveTab(int iBook)
{
    if (iMarkedTab == -1) return;

    ATabRecord * tab = getTab(iMarkedTab, iMarkedBook);
    ScriptBooks[iMarkedBook].removeTabNoCleanup(iMarkedTab);
    if (countTabs(iMarkedBook) == 0) addNewTab(iMarkedBook);

    ScriptBooks[iBook].Tabs.push_back(tab);
    ScriptBooks[iBook].TabWidget->addTab(tab->TextEdit, tab->TabName);

    iMarkedTab = -1;
}

void AScriptWindow::on_actionRemove_current_tab_triggered()
{
    askRemoveTab(getCurrentTabIndex());
}

void AScriptWindow::on_pbCloseFindReplaceFrame_clicked()
{
    ui->frFindReplace->hide();
    applyTextFindState();
}

void AScriptWindow::on_actionShow_Find_Replace_triggered()
{
    if (ui->frFindReplace->isVisible())
    {
        if (ui->cbActivateTextReplace->isChecked())
            ui->cbActivateTextReplace->setChecked(false);
        else
        {
            if (getTab()->TextEdit->textCursor().selectedText() == ui->leFind->text())
            {
                ui->frFindReplace->setVisible(false);
                return;
            }
        }
    }
    else ui->frFindReplace->setVisible(true);

    onFindSelected();
}

void AScriptWindow::onFindSelected()
{
    ui->frFindReplace->setVisible(true);
    ui->cbActivateTextReplace->setChecked(false);

    QTextCursor tc = getTab()->TextEdit->textCursor();
    QString sel = tc.selectedText();
    //if (!sel.isEmpty())
    //    ui->leFind->setText(sel);

    if (sel.isEmpty())
    {
        tc.select(QTextCursor::WordUnderCursor);
        sel = tc.selectedText();
    }

    ui->leFind->setText(sel);

    ui->leFind->setFocus();
    ui->leFind->selectAll();

    applyTextFindState();
}

void AScriptWindow::on_actionReplace_widget_Ctr_r_triggered()
{
    if (ui->frFindReplace->isVisible())
    {
        if (!ui->cbActivateTextReplace->isChecked())
            ui->cbActivateTextReplace->setChecked(true);
        else
        {
            if (getTab()->TextEdit->textCursor().selectedText() == ui->leFind->text())
            {
                ui->frFindReplace->setVisible(false);
                return;
            }
        }
    }
    else ui->frFindReplace->setVisible(true);

    onReplaceSelected();
}

void AScriptWindow::onReplaceSelected()
{
    ui->frFindReplace->setVisible(true);
    ui->cbActivateTextReplace->setChecked(true);

    QTextCursor tc = getTab()->TextEdit->textCursor();
    QString sel = tc.selectedText();
    //if (sel.isEmpty())
    //{
    //    ui->leFind->setFocus();
    //    ui->leFind->selectAll();
    //}
    //else
    //{

    if (sel.isEmpty())
    {
        tc.select(QTextCursor::WordUnderCursor);
        sel = tc.selectedText();
    }

    ui->leFind->setText(sel);
    ui->leReplace->setFocus();
    ui->leReplace->selectAll();
    //}

    applyTextFindState();
}

void AScriptWindow::onFindFunction()
{
    ATextEdit* te = getTab()->TextEdit;
    QTextDocument* d = te->document();
    QTextCursor tc = te->textCursor();
    QString name = tc.selectedText();
    if (name.isEmpty())
    {
        tc.select(QTextCursor::WordUnderCursor);
        name = tc.selectedText();
    }

    QStringList sl = name.split("(");
    if (sl.size() > 0) name = sl.first();
    QRegularExpression sp("\\bfunction\\s+" + name + "\\s*" + "\\(");
    //qDebug() << "Looking for:"<<sp;

    QTextDocument::FindFlags flags = QTextDocument::FindCaseSensitively;

    tc = d->find(sp, 0, flags);

    if (tc.isNull())
    {
        guitools::message("Function definition for " + name + " not found", this);
        return;
    }

    QTextCursor tc_copy = QTextCursor(tc);
    tc_copy.setPosition(tc_copy.anchor(), QTextCursor::MoveAnchor); //position
    te->setTextCursor(tc_copy);
    te->ensureCursorVisible();

    QTextCharFormat tf;
    tf.setBackground(Qt::blue);
    tf.setForeground(Qt::white);
    tf.setUnderlineStyle(QTextCharFormat::SingleUnderline);

    QTextEdit::ExtraSelection es;
    es.cursor = tc;
    es.format = tf;

    QList<QTextEdit::ExtraSelection> esList = te->extraSelections();
    esList << es;
    te->setExtraSelections(esList);
}

void AScriptWindow::onFindVariable()
{
    ATextEdit* te = getTab()->TextEdit;
    QTextDocument* d = te->document();
    QTextCursor tc = te->textCursor();
    QString name = tc.selectedText();
    if (name.isEmpty())
    {
        tc.select(QTextCursor::WordUnderCursor);
        name = tc.selectedText();
    }

    QStringList sl = name.split("(");
    if (sl.size() > 0) name = sl.first();
    QRegularExpression sp("\\bvar\\s+" + name + "\\b");
    //qDebug() << "Looking for:"<<sp;

    QTextDocument::FindFlags flags = QTextDocument::FindCaseSensitively;

    tc = d->find(sp, 0, flags);

    if (tc.isNull())
    {
        guitools::message("Variable definition for " + name + " not found", this);
        return;
    }

    QTextCursor tc_copy = QTextCursor(tc);
    tc_copy.setPosition(tc_copy.anchor(), QTextCursor::MoveAnchor); //position
    te->setTextCursor(tc_copy);
    te->ensureCursorVisible();

    QTextCharFormat tf;
    tf.setBackground(Qt::blue);
    tf.setForeground(Qt::white);
    tf.setUnderlineStyle(QTextCharFormat::SingleUnderline);

    QTextEdit::ExtraSelection es;
    es.cursor = tc;
    es.format = tf;

    QList<QTextEdit::ExtraSelection> esList = te->extraSelections();
    esList << es;
    te->setExtraSelections(esList);
}

void AScriptWindow::onBack()
{
    getTab()->goBack();
}

void AScriptWindow::onForward()
{
    getTab()->goForward();
}

void AScriptWindow::on_pbFindNext_clicked()
{
    findText(true);
}

void AScriptWindow::on_pbFindPrevious_clicked()
{
    findText(false);
}

void AScriptWindow::findText(bool bForward)
{
    ATextEdit* te = getTab()->TextEdit;
    QTextDocument* d = te->document();

    QString textToFind = ui->leFind->text();
    const int oldPos = te->textCursor().anchor();
    QTextDocument::FindFlags flags;
    if (!bForward)
        flags = flags | QTextDocument::FindBackward;
    if (ui->cbFindTextCaseSensitive->isChecked())
        flags = flags | QTextDocument::FindCaseSensitively;
    if (ui->cbFindTextWholeWords->isChecked())
        flags = flags | QTextDocument::FindWholeWords;

    QTextCursor tc = d->find(textToFind, te->textCursor(), flags);

    if (!tc.isNull())
        if (oldPos == tc.anchor())
        {
            //just because the cursor was already on the start of the searched string
            tc = d->find(textToFind, tc, flags);
        }

    if (tc.isNull())
    {
        if (bForward)
            tc = d->find(textToFind, 0, flags);
        else
            tc = d->find(textToFind, d->characterCount()-1, flags);

        if (tc.isNull())
        {
            guitools::message("No matches found", this);
            return;
        }
    }

    QTextCursor tc_copy = QTextCursor(tc);
    tc_copy.setPosition(tc_copy.anchor(), QTextCursor::MoveAnchor); //position
    te->setTextCursor(tc_copy);
    te->ensureCursorVisible();

    QTextCharFormat tf;
    tf.setBackground(Qt::blue);
    tf.setForeground(Qt::white);
    tf.setUnderlineStyle(QTextCharFormat::SingleUnderline);

    QTextEdit::ExtraSelection es;
    es.cursor = tc;
    es.format = tf;

    QList<QTextEdit::ExtraSelection> esList = te->extraSelections();
    esList << es;
    te->setExtraSelections(esList);
}

void AScriptWindow::on_leFind_textChanged(const QString & /*arg1*/)
{
    applyTextFindState();
}

void AScriptWindow::applyTextFindState()
{
    bool bActive = ui->frFindReplace->isVisible();
    QString Text = (bActive ? ui->leFind->text() : "");

    ATextEdit * te = getTab()->TextEdit;
    te->FindString = Text;
    te->RefreshExtraHighlight();
}

void AScriptWindow::on_pbReplaceOne_clicked()
{
    ATextEdit* te = getTab()->TextEdit;
    QTextDocument* d = te->document();

    QString textToFind = ui->leFind->text();
    QString textReplacement = ui->leReplace->text();
    const int oldPos = te->textCursor().anchor();
    QTextDocument::FindFlags flags;
    if (ui->cbFindTextCaseSensitive->isChecked())
        flags = flags | QTextDocument::FindCaseSensitively;
    if (ui->cbFindTextWholeWords->isChecked())
        flags = flags | QTextDocument::FindWholeWords;

    QTextCursor tc = d->find(textToFind, te->textCursor(), flags);
    if (tc.isNull() || oldPos != tc.anchor())
    {
        guitools::message("Not found or cursor is not in front of the match pattern. Use find buttons above", this);
        return;
    }

    tc.insertText(textReplacement);
    te->setTextCursor(tc);
}

void AScriptWindow::on_pbReplaceAll_clicked()
{
    ATextEdit* te = getTab()->TextEdit;
    QTextDocument* d = te->document();

    QString textToFind = ui->leFind->text();
    QString textReplacement = ui->leReplace->text();

    QTextDocument::FindFlags flags;
    if (ui->cbFindTextCaseSensitive->isChecked())
        flags = flags | QTextDocument::FindCaseSensitively;
    if (ui->cbFindTextWholeWords->isChecked())
        flags = flags | QTextDocument::FindWholeWords;

    int numReplacements = 0;
    QTextCursor tc = d->find(textToFind, 0, flags);
    while (!tc.isNull())
    {
        tc.insertText(textReplacement);
        numReplacements++;
        tc = d->find(textToFind, tc, flags);
    }
    guitools::message("Replacements performed: " + QString::number(numReplacements), this);
}

void AScriptWindow::on_actionShortcuts_triggered()
{
    QString s = "For the current line:\n"
                "Ctrl + Alt + Del\tDelete line\n"
                "Ctrl + Alt + Down\tDublicate line\n"
                "Ctrl + Shift + Up\tShift line up\n"
                "Ctrl + Shift + Down\tShift line down\n"
                "\n"
                "For selected text:\n"
                "Ctrl + i\t\tAuto-align JavaScript\n"
                "\n"
                "Insert boilerplate code:\n"
                "Ctrl + Alt + R\tSet ROOT stats\n"
                "Ctrl + Alt + F  \t'For' cycle\n"
                "Ctrl + Alt + G\t1D graph\n"
                "Ctrl + Alt + Shift + G\t1D graph with more code\n"
                "Ctrl + Alt + H\t1D histogram\n"
                "Ctrl + Alt + Shift + H\t1D histogram with more code\n"
                "Ctrl + Alt + 2 followed by a hist or graph shortcut -> 2D hist or graph\n";

    guitools::message(s, this);
}

// ----------------------

void AScriptWindow::addNewBook()
{
    //qDebug() << "Add book triggered";
    size_t iNewBook = ScriptBooks.size();

    ScriptBooks.resize(iNewBook + 1);
    twBooks->addTab(getTabWidget(iNewBook), "");

    renameBook(iNewBook, createNewBookName());

    QTabWidget * twScriptTabs = getTabWidget(iNewBook);

    twScriptTabs->isVisible();

    //connect(twScriptTabs, &QTabWidget::currentChanged, this, &AScriptWindow::onCurrentTabChanged);
    connect(twScriptTabs, &QTabWidget::currentChanged, this, [this, twScriptTabs](int index)
    {
        if (twScriptTabs->isVisible()) onCurrentTabChanged(index);
    });
    connect(twScriptTabs, &QTabWidget::customContextMenuRequested, this, &AScriptWindow::onRequestTabWidgetContextMenu);
    connect(twScriptTabs->tabBar(), &QTabBar::tabMoved, this, &AScriptWindow::onScriptTabMoved);
}

void AScriptWindow::removeBook(int iBook, bool bConfirm)
{
    if (iBook < 0 || iBook >= (int)ScriptBooks.size()) return;
    if (ScriptBooks.size() == 1)
    {
        guitools::message("Cannot remove the last book", this);
        return;
    }

    if (bConfirm)
    {
        QString t = QString("Close book %1?\nUnsaved data will be lost").arg(ScriptBooks[iBook].Name);
        if ( !guitools::confirm(t, this) ) return;
    }

    twBooks->removeTab(iBook);
    ScriptBooks[iBook].removeAllTabs();
    delete ScriptBooks[iBook].TabWidget;
    ScriptBooks.erase(ScriptBooks.begin() + iBook);

    if (iCurrentBook >= (int)ScriptBooks.size()) iCurrentBook = (int)ScriptBooks.size() - 1;

    if (countTabs() == 0) addNewTab();
    updateFileStatusIndication();

    iMarkedTab = -1;
}

void AScriptWindow::saveBook(int iBook)
{
    if (iBook < 0 || iBook >= (int)ScriptBooks.size()) return;

    QString fileName = guitools::dialogSaveFile(this, "Save book", "json files (*.json);;All files (*.*)");
    if (fileName.isEmpty()) return;

    QFileInfo fileInfo(fileName);
    if(fileInfo.suffix().isEmpty()) fileName += ".json";

    QJsonObject json;
    ScriptBooks[iCurrentBook].writeToJson(json);
    bool bOK = jstools::saveJsonToFile(json, fileName);
    if (!bOK) guitools::message("Failed to save json to file: " + fileName, this);
}

void AScriptWindow::loadBook(int iBook, const QString & fileName)
{
    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, fileName);
    if (!ok)
    {
        guitools::message("Cannot open file: " + fileName, this);
        return;
    }

    if (json.isEmpty() || !json.contains("ScriptTabs"))
    {
        guitools::message("Json format error", this);
        return;
    }

    loadBook(iBook, json);
}

void AScriptWindow::loadBook(int iBook, const QJsonObject & json)
{
    QJsonArray ar = json["ScriptTabs"].toArray();
    ScriptBooks[iBook].removeAllTabs();
    for (int iTab = 0; iTab < ar.size(); iTab++)
    {
        QJsonObject js = ar[iTab].toObject();
        ATabRecord & tab = addNewTab(iBook);
        tab.readFromJson(js);

        if (tab.TabName.isEmpty()) tab.TabName = createNewTabName(iBook);
        setTabName(tab.TabName, iTab, iBook);

        if (!tab.FileName.isEmpty())
        {
            QString ScriptInFile;
            if ( ftools::loadTextFromFile(ScriptInFile, tab.FileName) )
            {
                QPlainTextEdit te;
                te.appendPlainText(ScriptInFile);
                bool bWasModified = ( te.document()->toPlainText() != tab.TextEdit->document()->toPlainText() );
                tab.setModifiedStatus(bWasModified);
            }
        }
    }
    if (countTabs(iBook) == 0) addNewTab(iBook);

    int iCurTab = -1;
    jstools::parseJson(json, "CurrentTab", iCurTab);
    if (iCurTab < 0 || iCurTab >= countTabs(iBook)) iCurTab = 0;
    ScriptBooks[iBook].setCurrentTabIndex(iCurTab);

    QString Name = ScriptBooks[iBook].Name;
    jstools::parseJson(json, "Name", Name);
    renameBook(iBook, Name);
}

void AScriptWindow::renameBook(int iBook, const QString & newName)
{
    if (iBook < 0 || iBook >= (int)ScriptBooks.size()) return;

    ScriptBooks[iBook].Name = newName;
    twBooks->setTabText(iBook, " " + newName + " ");
}

void AScriptWindow::renameBookRequested(int iBook)
{
    bool ok;
    QString text = QInputDialog::getText(this, "Input text",
                                         "New name for the tab:", QLineEdit::Normal,
                                         ScriptBooks[iBook].Name, &ok);
    if (ok && !text.isEmpty())
        renameBook(iBook, text);
}

bool AScriptWindow::isUntouchedBook(int iBook) const
{
    if (iBook < 0 || iBook >= (int)ScriptBooks.size()) return false;

    int numTabs = countTabs(iBook);
    if (numTabs == 0) return true;

    if (numTabs == 1) return getTab(0, iBook)->TextEdit->document()->isEmpty();
    else return false;
}

std::vector<ATabRecord*> & AScriptWindow::getScriptTabs()
{
    return ScriptBooks[iCurrentBook].Tabs;
}

std::vector<ATabRecord*> &AScriptWindow::getScriptTabs(int iBook)
{
    return ScriptBooks[iBook].Tabs;
}

ATabRecord * AScriptWindow::getTab()
{
    return ScriptBooks[iCurrentBook].getCurrentTab();
}

ATabRecord *AScriptWindow::getTab(int index)
{
    return ScriptBooks[iCurrentBook].getTab(index);
}

ATabRecord *AScriptWindow::getTab(int index, int iBook)
{
    if (iBook < 0 || iBook >= (int)ScriptBooks.size()) return nullptr;
    return ScriptBooks[iBook].getTab(index);
}

const ATabRecord *AScriptWindow::getTab(int index, int iBook) const
{
    if (iBook < 0 || iBook >= (int)ScriptBooks.size()) return nullptr;
    return ScriptBooks[iBook].getTab(index);
}

QTabWidget *AScriptWindow::getTabWidget()
{
    return ScriptBooks[iCurrentBook].getTabWidget();
}

QTabWidget *AScriptWindow::getTabWidget(int iBook)
{
    if (iBook < 0 || iBook >= (int)ScriptBooks.size()) return nullptr;
    return ScriptBooks[iBook].getTabWidget();
}

int AScriptWindow::getCurrentTabIndex()
{
    return ScriptBooks[iCurrentBook].getCurrentTabIndex();
}

void AScriptWindow::setCurrentTabIndex(int index)
{
    ScriptBooks[iCurrentBook].setCurrentTabIndex(index);
}

void AScriptWindow::setCurrentTabIndex(int index, int iBook)
{
    ScriptBooks[iBook].setCurrentTabIndex(index);
}

int AScriptWindow::countTabs(int iBook) const
{
    if (iBook < 0 || iBook >= (int)ScriptBooks.size()) return 0;
    return ScriptBooks[iBook].Tabs.size();
}

int AScriptWindow::countTabs() const
{
    return ScriptBooks[iCurrentBook].Tabs.size();
}

void AScriptWindow::setTabName(const QString & name, int index, int iBook)
{
    if (iBook < 0 || iBook >= (int)ScriptBooks.size()) return;
    ScriptBooks[iBook].setTabName(name, index);
}

void AScriptWindow::twBooks_customContextMenuRequested(const QPoint & pos)
{
    QMenu menu;

    int iBook = twBooks->tabBar()->tabAt(pos);
    bool bBook = (iBook != -1);

    QAction * add = menu.addAction("Add new book");
    menu.addSeparator();
    QAction * rename = menu.addAction("Rename");             rename->setEnabled(bBook);
    menu.addSeparator();
    QAction * remove = menu.addAction("Close");              remove->setEnabled(bBook);
    menu.addSeparator();
    QAction * save = menu.addAction("Save");                 save->setEnabled(bBook);
    menu.addSeparator();
    QAction * copy = menu.addAction("Copy marked tab here"); copy->setEnabled(bBook && iMarkedTab != -1);
    QAction * move = menu.addAction("Move marked tab here"); move->setEnabled(bBook && iMarkedTab != -1);

    QAction* selectedItem = menu.exec(twBooks->mapToGlobal(pos));
    if (!selectedItem) return;

    if      (selectedItem == rename) renameBookRequested(iBook);
    else if (selectedItem == add)    addNewBook();
    else if (selectedItem == copy)   copyTab(iBook);
    else if (selectedItem == move)   moveTab(iBook);
    else if (selectedItem == remove) removeBook(iBook);
    else if (selectedItem == save)   saveBook(iBook);
}

void AScriptWindow::twBooks_currentChanged(int index)
{
    //qDebug() << "book index changed to" << index;
    iCurrentBook = index;

    if (ScriptBooks[index].Tabs.empty()) addNewTab();

    updateFileStatusIndication();
}

void AScriptWindow::onBookTabMoved(int from, int to)
{
    //qDebug() << "Book from->to:"<<from<<to;
    std::swap(ScriptBooks[from], ScriptBooks[to]);
    iMarkedTab = -1;
}

void AScriptWindow::on_actionAdd_new_book_triggered()
{
    addNewBook();
}

void AScriptWindow::on_actionLoad_book_triggered()
{
    if ( !isUntouchedBook(iCurrentBook) )
    {
        addNewBook();
        iCurrentBook = (int)ScriptBooks.size() - 1;
        twBooks->setCurrentIndex(iCurrentBook);
    }

    QString fileName = guitools::dialogLoadFile(this, "Load script book", "json files (*.json);;All files (*.*)");
    if (fileName.isEmpty()) return;

    loadBook(iCurrentBook, fileName);

    updateFileStatusIndication();
}

void AScriptWindow::on_actionClose_book_triggered()
{
    removeBook(iCurrentBook);
}

void AScriptWindow::on_actionSave_book_triggered()
{
    saveBook(iCurrentBook);
}

void AScriptWindow::on_actionSave_all_triggered()
{
    if (ScriptBooks.size() == 0) return;
    QString fileName = guitools::dialogSaveFile(this, "Save script books to a file", "json files (*.json);;All files (*.*)");
    if (fileName.isEmpty()) return;

    QFileInfo fileInfo(fileName);
    if(fileInfo.suffix().isEmpty()) fileName += ".json";

    QJsonObject json;
    writeToJson(json);
    bool bOK = jstools::saveJsonToFile(json, fileName);
    if (!bOK) guitools::message("Failed to save json to file: "+fileName, this);
}

void AScriptWindow::on_actionload_session_triggered()
{
    bool bSingleUntouched = (ScriptBooks.size() == 1 && isUntouchedBook(0));
    if (!bSingleUntouched)
    {
        bool ok = guitools::confirm("All books will be closed and unsaved data will be lost.\nContinue?", this);
        if (!ok) return;
    }

    QString fileName = guitools::dialogLoadFile(this, "Load script books", "json files (*.json);;All files (*.*)");
    if (fileName.isEmpty()) return;

    QJsonObject json;
    bool bOK = jstools::loadJsonFromFile(json, fileName);
    if (!bOK) guitools::message("Cannot open file: "+fileName, this);
    else readFromJson(json);
}

void AScriptWindow::on_actionClose_all_books_triggered()
{
    QString t = "Close all books?\nUnsaved data will be lost";
    if ( !guitools::confirm(t, this) ) return;
    removeAllBooksExceptFirst();
    addNewTab();
}

#include <QClipboard>
void AScriptWindow::on_pbFileName_customContextMenuRequested(const QPoint & pos)
{
    QString fn = getTab()->FileName;
    if (fn.isEmpty()) return;

    QMenu menu;
    QAction * copy   = menu.addAction("Copy file name to the clipboard");
    QAction * copyIn = menu.addAction("Copy file name to the clipboard to \"include\" that script in another one");

    QAction * sel = menu.exec(ui->pbFileName->mapToGlobal(pos));

    if (sel == copy)
    {
        QClipboard * clipboard = QApplication::clipboard();
        clipboard->setText(fn);
    }
    else if (sel == copyIn)
    {
        QClipboard * clipboard = QApplication::clipboard();
        QString txt;
        if (ScriptLanguage == EScriptLanguage::JavaScript)
            txt = QString("var loaded_script = core.loadText(\"%0\"); eval(loaded_script)").arg(fn);
        else
            txt = QString("loaded_script = core.loadText(\"%0\"); exec(loaded_script)").arg(fn);
        clipboard->setText(txt);
    }
}
