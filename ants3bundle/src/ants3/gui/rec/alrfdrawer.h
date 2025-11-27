#ifndef ALRFDRAWER_H
#define ALRFDRAWER_H

#include <QObject>
#include <QString>

class LRModel;
class TObject;

class ALrfDrawer : public QObject
{
    Q_OBJECT

public:
    ALrfDrawer(LRModel * model);

    QString drawRadial(int iSens, bool showNodes); // returns error
    QString drawXY(int iSens); // returns error

private:
    LRModel * Model = nullptr;

    size_t NumPointsInRadialGraph = 100;
    size_t NumPointsInXYGraph = 100;

signals:
    void requestDraw(TObject * obj, QString options, bool fFocus);
};

#endif // ALRFDRAWER_H
