#include "aparticleanalyzerwidget.h"
#include "ui_aparticleanalyzerwidget.h"

AParticleAnalyzerWidget::AParticleAnalyzerWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AParticleAnalyzerWidget)
{
    ui->setupUi(this);
}

AParticleAnalyzerWidget::~AParticleAnalyzerWidget()
{
    delete ui;
}
