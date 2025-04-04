#ifndef AROOTTEXTCONFIGURATOR_H
#define AROOTTEXTCONFIGURATOR_H

#include <QDialog>

#include <vector>

namespace Ui {
class ARootTextConfigurator;
}

class ARootTextConfigurator : public QDialog
{
    Q_OBJECT

public:
    ARootTextConfigurator(int & color, int & align, int & font, float & size, QWidget * parent = nullptr);
    virtual ~ARootTextConfigurator();

protected:
  void paintEvent(QPaintEvent * e);
  void mousePressEvent(QMouseEvent * e);

protected:
    Ui::ARootTextConfigurator * ui = nullptr;

    int SquareSize = 30;
    std::vector<int> BaseColors;

    int   & Color;
    int   & Align;
    int   & Font;
    float & Size;

protected:
    void setupAlignmentControls();
    virtual int readAlignment();

private slots:
    void PaintColorRow(QPainter * p, int row, int colorBase);

private slots:
    void updateColorFrame();
    void previewColor();
    void onUserAction();

    void on_pbAccept_clicked();
    void on_pbCancel_clicked();

    void on_cobHorizontalAlignment_activated(int index);
    void on_cobVerticalAlignment_activated(int index);
    void on_ledSize_editingFinished();
    void on_cobFontType_activated(int index);

signals:
    void propertiesChanged(int color, int align, int font, float size);

};

class ARootAxisTitleTextConfigurator : public ARootTextConfigurator
{
    Q_OBJECT

public:
    explicit ARootAxisTitleTextConfigurator(int & color, int & align, int & font, float & size, QWidget * parent = nullptr);

protected:
    void setupAlignmentControls();
    int readAlignment() override;

};

class ARootAxisLabelTextConfigurator : public ARootTextConfigurator
{
    Q_OBJECT

public:
    explicit ARootAxisLabelTextConfigurator(int & color, int & align, int & font, float & size, QWidget * parent = nullptr);

protected:
    void setupAlignmentControls();
    int readAlignment() override;

};


#endif // AROOTTEXTCONFIGURATOR_H
