#ifndef APARTICLEANALYZERWIDGET_H
#define APARTICLEANALYZERWIDGET_H

#include <QWidget>

namespace Ui {
class AParticleAnalyzerWidget;
}

class AParticleAnalyzerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AParticleAnalyzerWidget(QWidget *parent = nullptr);
    ~AParticleAnalyzerWidget();

private:
    Ui::AParticleAnalyzerWidget *ui;
};

#endif // APARTICLEANALYZERWIDGET_H
