#ifndef AGEOSCRIPTMAKER_H
#define AGEOSCRIPTMAKER_H

#include <QString>

class AGeoObject;

class AGeoScriptMaker
{
public:
    enum ELanguage {JavaScript, Python};

    AGeoScriptMaker(ELanguage lang = JavaScript) : Language(lang) {}

    void createScript(QString & script);

    void objectMembersToScript(AGeoObject * Master, QString & script, int ident, bool useStrings, bool bRecursive);
    void objectToScript(AGeoObject * obj, QString & script, int ident, bool useStrings, bool bRecursive);

protected:
    ELanguage Language = JavaScript;

    QString makeScriptString_basicObject(AGeoObject * obj, bool useStrings) const;
    QString makeScriptString_arrayObject(AGeoObject * obj) const;
    QString makeScriptString_instanceObject(AGeoObject * obj) const;
    QString makeScriptString_prototypeObject(AGeoObject * obj) const;
    QString makeScriptString_monitorBaseObject(const AGeoObject * obj) const;
    QString makeScriptString_monitorConfig(const AGeoObject * obj) const;
    QString makeScriptString_stackObjectStart(AGeoObject * obj) const;
    QString makeScriptString_stackObjectEnd(AGeoObject * obj) const;
    QString makeLinePropertiesString(AGeoObject * obj) const;
    QString makeScriptString_DisabledObject(AGeoObject * obj) const;

    void addLineProperties(QString & script, AGeoObject * obj, int ident);
    void addScaledIfApplicable(QString & script, AGeoObject * obj, int ident, bool useStrings);

    const QString getPythonGenerationString(const QString & javaGenString) const;
};

#endif // AGEOSCRIPTMAKER_H
