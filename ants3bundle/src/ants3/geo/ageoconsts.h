#ifndef AGEOCONSTS_H
#define AGEOCONSTS_H

#include <QVector>
#include <vector>
#include <QString>
#include <QRegularExpression>

class QJsonArray;
class AGeoObject;

struct AGeoConstRecord
{
    AGeoConstRecord(){}
    AGeoConstRecord(const QString & Name) : Name(Name) {}

    QString Name;
    QString Expression;
    QString Comment;

    //runtime
    QRegularExpression RegExp;
    QString Index;
};

class AGeoConsts final
{
public:
    static       AGeoConsts & getInstance();
    static const AGeoConsts & getConstInstance();

    void    clearConstants();
    void    updateFromExpressions(); // !!!*** add error handling!

    QString addNewConstant(const QString & name, double value, int index = -1);
    void    addNoNameConstant(int index);
    void    removeConstant(int index);

    bool    rename(int index, const QString & newName, AGeoObject * world, QString & errorStr);
    QString isNameValid(int index, const QString & newName);
    bool    setNewValue(int index, double newValue);
    QString setNewExpression(int index, const QString & newExpression);
    void    setNewComment(int index, const QString & txt);
    bool    isIndexValid(int index);

    QString checkifValidAndGetDoublefromExpression(int index);
    QString isGeoConstsBelowInUse(int index) const;

    QString isGeoConstInUse(const QRegularExpression & nameRegExp, int index) const;
    void    replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName, int index);

    QString getName(int index) const;
    double  getValue(int index) const;
    QString getExpression(int index) const;
    QString getComment(int index) const;

    int     countConstants() const {return Records.size();}
    bool    evaluateConstExpression(int index);
    bool    isGeoConstInUseGlobal(const QRegularExpression & nameRegExp, const AGeoObject * obj) const;

    QString exportToScript(const AGeoObject * obj, const QString &CommentStr, const QString &VarStr) const;
    void    formulaToScript(QString & str, bool usePython) const;

    void    writeToJsonArr(QJsonArray & ar) const;
    void    readFromJsonArr(const QJsonArray & json);

    bool    evaluateFormula(QString str, double & returnValue, int to = -1) const;
    bool    updateDoubleParameter(QString & errorStr, QString & str, double & returnValue, bool bForbidZero = true, bool bForbidNegative = true, bool bMakeHalf = true) const;
    bool    updateIntParameter(QString & errorStr, QString & str, int & returnValue, bool bForbidZero = true, bool bForbidNegative = true) const;

    const std::vector<QString> & getTFormulaReservedWords() const {return FormulaReservedWords;}

public:
    const QString placeholderStr = "______";

private:
    AGeoConsts();

    AGeoConsts(const AGeoConsts&) = delete;
    AGeoConsts(AGeoConsts&&) = delete;
    AGeoConsts& operator=(const AGeoConsts&) = delete;
    AGeoConsts& operator=(AGeoConsts&) = delete;

    QVector<AGeoConstRecord> Records;  // !!!*** std::vector
    QVector<double> GeoConstValues;    // has to be always synchronized with Records !  GeoConstValues.data() is used by TFormula

    //misc
    std::vector<QString> FunctionsToJS;
    std::vector<QString> FormulaReservedWords;
    std::vector<QRegularExpression> ForbiddenVarsRExp;

    void updateRunTimeProperties();
};

#endif // AGEOCONSTS_H
