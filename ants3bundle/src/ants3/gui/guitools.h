#ifndef GUITOOLS_H
#define GUITOOLS_H

#include <QString>
#include <QIcon>

#include <vector>

class QWidget;
class QJsonObject;
class QLineEdit;
class QCheckBox;
class QSpinBox;
class QComboBox;

namespace guitools
{
    void message(QString text, QWidget* parent = nullptr);
    void message1(const QString & text, const QString &title, QWidget * parent = nullptr);
    void message1notModal(const QString & text, const QString &title, QWidget * parent = nullptr);
    bool confirm(const QString &text, QWidget* parent = nullptr);

    void inputInteger(const QString & text, int & input, int min, int max, QWidget * parent = nullptr);
    void inputString(const QString & label, QString & input, QWidget * parent = nullptr);

    bool assureWidgetIsWithinVisibleArea(QWidget* w);

    QIcon   createColorCircleIcon(QSize size, Qt::GlobalColor color);
    QPixmap createColorCirclePixmap(QSize size, Qt::GlobalColor color);

    QString dialogSaveFile(QWidget * parent, const QString & text, const QString & filePattern);
    QString dialogLoadFile(QWidget * parent, const QString & text, const QString & filePattern);

    QString dialogDirectory(QWidget * parent, const QString & text, const QString & initialDir, bool DefaultRead = false, bool DefaultWrite = false);

    bool    extractNumbersFromQString(const QString & input, std::vector<int> & extracted);

    bool isDarkTheme();

    void parseJsonToQLineEdit(const QJsonObject & json, const QString & name, QLineEdit * le);
    void parseJsonToQCheckBox(const QJsonObject & json, const QString & name, QCheckBox * cb);
    void parseJsonToQSpinBox(const QJsonObject & json, const QString & name, QSpinBox * sb);
    void parseJsonToQComboBox(const QJsonObject & json, const QString & name, QComboBox * cob);

}

#endif // GUITOOLS_H
