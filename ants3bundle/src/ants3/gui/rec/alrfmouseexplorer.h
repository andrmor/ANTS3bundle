#ifndef ALRFMOUSEEXPLORER_H
#define ALRFMOUSEEXPLORER_H

#include <QDialog>

class LRModel;
class ASensorHub;
class ALrfViewerObject;
class ALrfGraphicsView;
class QPointF;
class QComboBox;
class QLineEdit;

class ALrfMouseExplorer : public QDialog
{
    Q_OBJECT

public:
    ALrfMouseExplorer(LRModel * lrmodel, double SuggestedZ = 0, QWidget * parent = 0);
    ~ALrfMouseExplorer();

    void Start();

private:
    ALrfViewerObject * LRFviewObj = nullptr;
    ALrfGraphicsView * GrView     = nullptr;

    LRModel    * LRFs = nullptr;
    ASensorHub & SensHub;

    QComboBox  * cobSG = nullptr;
    QLineEdit  * ledZ = nullptr;

public slots:
    void paintLRFonDialog(QPointF * pos);
    void onCobActivated(int);

};

#endif // ALRFMOUSEEXPLORER_H
