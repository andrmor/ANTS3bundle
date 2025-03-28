#include "ageoconsts.h"
#include "ageoobject.h"
#include "ajsontools.h"
#include "aerrorhub.h"

#include "TFormula.h"

#include <QDebug>

AGeoConsts::AGeoConsts()
{
    FunctionsToJS = {"abs", "acos", "asin", "atan", "ceil", "floor", "cos", "exp", "log", "pow", "sin", "sqrt", "tan"};

    FormulaReservedWords = {"sqrt2", "pi", "e", "ln10", "infinity",
                            "c", "r",
                            "pow", "sin", "cos", "sqrt", "exp", "ceil", "floor"};

    std::vector<QString> vec = {"g", "h", "t", "k", "x", "y", "z"};
    for (const QString & s : vec)
        ForbiddenVarsRExp.push_back(QRegularExpression("\\b" + s + "\\b"));
}

AGeoConsts &AGeoConsts::getInstance()
{
    static AGeoConsts instance;
    return instance;
}

const AGeoConsts &AGeoConsts::getConstInstance()
{
    return getInstance();
}

bool AGeoConsts::isGeoConstInUseGlobal(const QRegularExpression & nameRegExp, const AGeoObject * obj) const
{
    for (const AGeoConstRecord & r : Records)
        if (r.Expression.contains(nameRegExp)) return true;

    if (obj->isGeoConstInUseRecursive(nameRegExp)) return true;

    return false;
}

QString AGeoConsts::exportToScript(const AGeoObject * obj, const QString &CommentStr, const QString &VarStr) const
{
    QString GCScript;

    if (!Records.empty())
    {
        for (int i = 0; i < (int)Records.size(); i++)
        {
            const AGeoConstRecord & r = Records.at(i);
            QRegularExpression nameRegExp("\\b" + r.Name + "\\b");
            if (isGeoConstInUseGlobal(nameRegExp, obj))
            {
                GCScript += QString("%1%2 = %3")
                             .arg(VarStr)
                             .arg(r.Name)
                             .arg(r.Expression.isEmpty() ? QString::number(GeoConstValues[i]) : r.Expression);
                if (!r.Comment.isEmpty())
                    GCScript += QString("   %1 %2").arg(CommentStr).arg(r.Comment);
                GCScript += "\n";
            }
        }
        formulaToScript(GCScript, VarStr.isEmpty());    //VarStr is only empty for python
    }

    return GCScript;
}

void AGeoConsts::formulaToScript(QString & str, bool usePython) const
{
    QString mathStr;
    if (!usePython)
        mathStr = "Math.";
    else
        mathStr = "MATH.";

    for (const QString & s : FunctionsToJS)
    {
        QString pat(s + '(');
        str.replace(pat, mathStr + pat);
    }
}

void AGeoConsts::clearConstants()
{
    Records.clear();
    GeoConstValues.clear();
}

void AGeoConsts::updateFromExpressions()
{
    for (int i = 0; i < (int)Records.size(); i++)
        evaluateConstExpression(i);
}

void AGeoConsts::writeToJsonArr(QJsonArray & ar) const
{
    ar = QJsonArray();

    for (size_t i = 0; i < Records.size(); i++)
    {
        const AGeoConstRecord & r = Records.at(i);
        QJsonArray el;
            el << r.Name << GeoConstValues.at(i) << r.Expression << r.Comment;
        ar.push_back(el);
    }
}

void AGeoConsts::readFromJsonArr(const QJsonArray & ar)
{
    clearConstants();

    const int size = ar.size();
    Records.resize(size);
    GeoConstValues.resize(size);

    for (int i = 0; i < size; i++)
    {
        AGeoConstRecord & r = Records[i];
        QJsonArray el = ar[i].toArray();
        if (el.size() >= 2)
        {
            r.Name            = el[0].toString();
            GeoConstValues[i] = el[1].toDouble();
        }
        if (el.size() >= 3) r.Expression = el[2].toString();
        if (el.size() >= 4) r.Comment    = el[3].toString();
    }

    updateRunTimeProperties();

    for (int i = 0; i < size; i++)
    {
        bool ok = evaluateConstExpression(i);
        if (!ok) qWarning() << "Something went wrong with Geo Const test, error is reported in AErrorHub";
    }
}

#include "TFormula.h"
bool AGeoConsts::evaluateFormula(QString & error, QString str, double & returnValue, int to) const
{
    if (to == -1) to = Records.size();

    if (str.contains("ParentIndex")) str.replace("ParentIndex", "0");

    for (int i = 0; i < to; i++)
        str.replace(Records.at(i).RegExp, Records.at(i).Index);

    for (const QRegularExpression & fe : ForbiddenVarsRExp)
    {
        if (str.contains(fe))
        {
            error += QString("Geo constant (%0) contains invalid vars").arg(str);
            return false;
        }
    }

    TFormula * f = new TFormula("", str.toLocal8Bit().data());
    if (!f || !f->IsValid())
    {
        delete f;
        error += QString("String (%0) produces an invalid TFormula").arg(str);
        return false;
    }

    returnValue = f->EvalPar(nullptr, GeoConstValues.data());
    delete f;
    return true;
}

bool AGeoConsts::updateDoubleParameter(QString & errorStr, QString & str, double & returnValue, bool bForbidZero, bool bForbidNegative, bool bMakeHalf) const
{
    if (str.isEmpty()) return true;

    bool ok;
    returnValue = str.simplified().toDouble(&ok);
    if (ok) str.clear();
    else
    {
        ok = evaluateFormula(errorStr, str, returnValue);
        if (!ok) return false;
    }

    if (bForbidZero && returnValue == 0)
    {
        errorStr += "Invalid zero value";
        return false;
    }
    if (bForbidNegative && returnValue < 0)
    {
        errorStr = "Invalid negative value";
        return false;
    }

    if (bMakeHalf) returnValue *= 0.5;
    return true;
}

bool AGeoConsts::updateIntParameter(QString & errorStr, QString & str, int & returnValue, bool bForbidZero, bool bForbidNegative) const
{
    if (str.isEmpty()) return true;

    bool ok;
    returnValue = str.simplified().toInt(&ok);
    if (ok) str.clear();
    else
    {
        double dRetVal;
        ok = evaluateFormula(errorStr, str, dRetVal);
        if (!ok) return false;
        returnValue = dRetVal;
    }

    if (bForbidZero && returnValue == 0)
    {
        errorStr = "Invalid zero value";
        //if (!str.isEmpty()) errorStr += " in expression: " + str;
        return false;
    }
    if (bForbidNegative && returnValue < 0)
    {
        errorStr = "Invalid negative value";
        //if (!str.isEmpty()) errorStr += " in expression: " + str;
        //else errorStr += ": " + QString::number(returnValue);
        return false;
    }
    return true;
}

QString AGeoConsts::getName(int index) const
{
    if (index < 0 || index >= (int)Records.size()) return "";
    if (Records.at(index).Name == placeholderStr) return "";
    return Records.at(index).Name;
}

double AGeoConsts::getValue(int index) const
{
    if (index < 0 || index >= (int)GeoConstValues.size()) return 0;
    return GeoConstValues.at(index);
}

QString AGeoConsts::getExpression(int index) const
{
    if (index < 0 || index >= (int)Records.size()) return "";
    return Records.at(index).Expression;
}

QString AGeoConsts::getComment(int index) const
{
    if (index < 0 || index >= (int)Records.size()) return "";
    return Records.at(index).Comment;
}

bool AGeoConsts::evaluateConstExpression(int index)
{
    AGeoConstRecord & rec = Records[index];

    if (rec.Expression.isEmpty()) return true;
    QString strCopy = rec.Expression;

    bool ok;
    double val = strCopy.simplified().toDouble(&ok);
    if (ok)
        rec.Expression.clear();
    else
    {
        QString errorStr;
        ok = evaluateFormula(errorStr, strCopy, val, index);
        if (!ok)
        {
            AErrorHub::addQError("Error in Geo const expession " + rec.Name + ":\n  " + errorStr);
            return false;
        }
    }
    GeoConstValues[index] = val;
    return true;
}

bool AGeoConsts::rename(int index, const QString & newName, AGeoObject * world, QString & errorStr)
{
    if (index < 0 || index >= (int)Records.size()) return false;
    AGeoConstRecord & rec = Records[index];

    if (newName == rec.Name) return true;
    errorStr = isNameValid(index, newName);
    if (!errorStr.isEmpty()) return false;
    rec.Name = newName;

    replaceGeoConstName(rec.RegExp, newName, index);
    world->replaceGeoConstNameRecursive(rec.RegExp, newName);
    updateRunTimeProperties();
    return true;
}

QString AGeoConsts::isNameValid(int index, const QString & newName)
{
    if (newName.isEmpty())                return "Name cannot be empty";
    if (newName.at(0).isDigit())          return "Name cannot start with a digit";
    if (newName.contains(QRegularExpression("\\s"))) return "Name cannot contain whitespace charachters eg:\" \" or \"\\n\" ";
    if (newName.contains(QRegularExpression("\\W"))) return "Name can only contain word characters: [0-9], [A-Z], [a-z], _";

    for (int i = 0; i < (int)Records.size(); i++)
    {
        if (i == index) continue;
        if (newName == Records.at(i).Name) return "This name is already in use";
    }

    QRegularExpression reservedQRegularExpression;
    for (const QString & word : FormulaReservedWords)
    {
        reservedQRegularExpression = QRegularExpression("\\b" + word + "\\b");
        if (newName.contains(reservedQRegularExpression)) return QString("Name is a TFormula reserved word: %1").arg(word);
    }

    //for (const QRegularExpression & ex : ForbiddenVarsRExp)
    //    if (newName.contains(ex)) return QString("Name is a TFormula reserved expression: %1").arg(newName);

    return "";
}

bool AGeoConsts::setNewValue(int index, double newValue)
{
    if (index < 0 || index >= (int)Records.size()) return false;

    GeoConstValues[index] = newValue;
    Records[index].Expression.clear();
    return true;
}

QString AGeoConsts::setNewExpression(int index, const QString & newExpression)
{
    if (index < 0 || index >= (int)Records.size()) return "Wrong index";

    if (newExpression.contains('[') || newExpression.contains(']'))
    {
        QString err = QString("Expression can not contain square brackets: %0").arg(newExpression);
        AErrorHub::addQError(err);
        return err;
    }

    AGeoConstRecord & rec = Records[index];
    rec.Expression = newExpression;

    QString err = checkifValidAndGetDoublefromExpression(index);
    if (!err.isEmpty()) rec.Expression.clear();
    return err;
}

void AGeoConsts::setNewComment(int index, const QString & txt)
{
    if (index < 0 || index >= (int)Records.size()) return;
    Records[index].Comment = txt;
}

bool AGeoConsts::isIndexValid(int index)
{
    if (index < 0 || index >= (int)Records.size()) return false;
    return true;
}

QString AGeoConsts::checkifValidAndGetDoublefromExpression(int index)
{
    QString errorStr;
    const AGeoConstRecord & rec = Records[index];
    if (!rec.Expression.isEmpty())
    {
        QString constInUseBellow = isGeoConstsBelowInUse(index);
        if (!constInUseBellow.isEmpty())
            errorStr = QString("Expression not valid:\n"
                               "%1\n\n"
                               "Expression uses a geometry constant defined bellow:\n%2")
                               .arg(rec.Expression).arg(constInUseBellow);
        else
        {
            bool ok;
            ok = evaluateConstExpression(index);
            if (!ok) errorStr = QString("Expression not valid:\n\n%1\n\nSyntax error").arg(rec.Expression);
        }
    }
    return errorStr;
}

QString AGeoConsts::isGeoConstsBelowInUse(int index) const
{
    for (int i = index+1; i < (int)Records.size(); i++)
        if (Records.at(index).Expression.contains(Records.at(i).RegExp)) return Records.at(i).Name;
    return "";
}

QString AGeoConsts::isGeoConstInUse(const QRegularExpression & nameRegExp, int index) const
{
    for (int i = index; i < (int)Records.size(); i++)
        if (Records.at(i).Expression.contains(nameRegExp)) return Records.at(i).Name;
    return "";
}

void AGeoConsts::replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName, int index)
{
    for (int i = index; i < (int)Records.size(); i++)
        Records[i].Expression.replace(nameRegExp, newName);
}

QString AGeoConsts::addNewConstant(const QString & name, double value, int index)
{
    if (index < -1 || index > (int)Records.size()) return "Bad index for the new geo constant";

    QString errorStr;
    if (name != placeholderStr)
    {
        errorStr = isNameValid(index, name);
        if (!errorStr.isEmpty()) return errorStr;
    }
    if (index == -1) index = Records.size();

           Records.insert(Records.begin()        + index, AGeoConstRecord(name));
    GeoConstValues.insert(GeoConstValues.begin() + index, value);

    updateRunTimeProperties();

    return errorStr;
}

void AGeoConsts::addNoNameConstant(int index)
{
    addNewConstant(placeholderStr, 0, index);
}

void AGeoConsts::removeConstant(int index)
{
    if (index < 0 || index >= (int)Records.size()) return;

           Records.erase(Records.begin()        + index);
    GeoConstValues.erase(GeoConstValues.begin() + index);

    updateRunTimeProperties();
}

void AGeoConsts::updateRunTimeProperties()
{
    const int size = Records.size();

    for (int i = 0; i < size; i++)
    {
        Records[i].RegExp = QRegularExpression("\\b" + Records.at(i).Name + "\\b");
        Records[i].Index  = QString("[%1]").arg(i);
    }
}
