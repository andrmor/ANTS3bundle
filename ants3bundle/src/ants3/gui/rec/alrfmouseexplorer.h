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
class QLabel;

class ALrfMouseExplorer : public QDialog
{
    Q_OBJECT

public:
    ALrfMouseExplorer(LRModel * lrmodel, double suggestedZ = 0, QWidget * parent = nullptr);
    ~ALrfMouseExplorer();

    void Start();

private:
    ALrfViewerObject * LRFviewObj = nullptr;
    ALrfGraphicsView * GrView     = nullptr;

    LRModel    * LRFs = nullptr;
    ASensorHub & SensHub;

    QLabel     * lInvalid = nullptr;
    QComboBox  * cobSG = nullptr;
    QLineEdit  * ledZ = nullptr;

    bool ModelValid = false;

public slots:
    void paintLRFonDialog(QPointF * pos);
    void onCobActivated(int);
    void checkModel();

};

#endif // ALRFMOUSEEXPLORER_H
