#ifndef APARTICLEANALYZERWIDGET_H
#define APARTICLEANALYZERWIDGET_H

#include <QWidget>

namespace Ui {
class AParticleAnalyzerWidget;
}

class AGeoParticleAnalyzer;

class AParticleAnalyzerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AParticleAnalyzerWidget(QWidget * parent = nullptr);
    ~AParticleAnalyzerWidget();

    void updateGui(const AGeoParticleAnalyzer & pap);
    void updateObject(AGeoParticleAnalyzer & pa) const;

    QString check() const;

private:
    Ui::AParticleAnalyzerWidget * ui = nullptr;

private slots:
    void on_pbChanged_clicked();

signals:
    void contentChanged();
};

#endif // APARTICLEANALYZERWIDGET_H
