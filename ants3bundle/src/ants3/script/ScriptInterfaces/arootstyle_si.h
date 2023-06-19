#ifndef AROOTSTYLE_SI_H
#define AROOTSTYLE_SI_H

#include "ascriptinterface.h"

#include <QObject>
#include <QString>
#include <QVariant>

class ARootStyle_SI : public AScriptInterface
{
    Q_OBJECT

public:
    ARootStyle_SI();

    AScriptInterface * cloneBase() const {return new ARootStyle_SI();}

public slots:
    //https://root.cern.ch/root/html/TStyle.html
    void SetAxisColor(int color, QString axis);
    void SetBarOffset(float baroff);
    void SetBarWidth(float barwidth);
    void SetCanvasBorderMode(int mode);
    void SetCanvasBorderSize(int size);
    void SetCanvasColor(int color);
    void SetCanvasDefH(int h);
    void SetCanvasDefW(int w );
    void SetCanvasDefX(int topx);
    void SetCanvasDefY(int topy);
    void SetCanvasPreferGL(bool prefer);
    void SetColorModelPS(int c);
    void SetDateX(float x);
    void SetDateY(float y);
    void SetDrawBorder(int drawborder);
    void SetEndErrorSize(float np);
    void SetErrorX(float errorx);
    void SetFitFormat(QString format);
    void SetFrameBorderMode(int mode);
    void SetFrameBorderSize(int size);
    void SetFrameFillColor(int color);
    void SetFrameFillStyle(int style);
    void SetFrameLineColor(int color);
    void SetFrameLineStyle(int style);
    void SetFrameLineWidth(int width);
    void SetFuncColor(int color);
    void SetFuncStyle(int style);
    void SetFuncWidth(int width);
    void SetGridColor(int color);
    void SetGridStyle(int style);
    void SetGridWidth(int width);
    void SetHatchesLineWidth(int l);
    void SetHatchesSpacing(double h);
    void SetHeaderPS(QString header);
    void SetHistFillColor(int color);
    void SetHistFillStyle(int style);
    void SetHistLineColor(int color);
    void SetHistLineStyle(int style);
    void SetHistLineWidth(int width);
    void SetHistMinimumZero(bool zero);
    void SetHistTopMargin(double hmax);
    void SetIsReading(bool reading);
    void SetLabelColor(int color, QString axis);
    void SetLabelFont(int font, QString axis);
    void SetLabelOffset(float offset, QString axis);
    void SetLabelSize(float size, QString axis);
    void SetLegendBorderSize(int size);
    void SetLegendFillColor(int color);
    void SetLegendFont(int font);
    void SetLegoInnerR(float rad);
    void SetLineScalePS(float scale);
    void SetLineStyleString(int i, QString text);
    void SetNdivisions(int n, QString axis);
    void SetNumberContours(int number);
    void SetOptDate(int datefl);
    void SetOptFile(int file);
    void SetOptFit(int fit);
    void SetOptLogx(int logx);
    void SetOptLogy(int logy);
    void SetOptLogz(int logz);
    //void SetOptStat(int stat);
    //void SetOptStat(QString stat);
    void SetOptStat(QVariant stat);
    void SetOptTitle(int tit);
    void SetPadBorderMode(int mode);
    void SetPadBorderSize(int size);
    void SetPadBottomMargin(float margin);
    void SetPadColor(int color);
    void SetPadGridX(bool gridx);
    void SetPadGridY(bool gridy);
    void SetPadLeftMargin(float margin);
    void SetPadRightMargin(float margin);
    void SetPadTickX(int tickx);
    void SetPadTickY(int ticky);
    void SetPadTopMargin(float margin);
    void SetPaintTextFormat(QString format);
    void SetPalette(int scheme); //51 - 56, default 1
    void SetPaperSize(float xsize, float ysize);
    void SetScreenFactor(float factor);
    void SetStatBorderSize(int size);
    void SetStatColor(int color);
    void SetStatFont(int font);
    void SetStatFontSize(float size);
    void SetStatFormat(QString format);
    void SetStatH(float h);
    void SetStatStyle(int style);
    void SetStatTextColor(int color);
    void SetStatW(float w);
    void SetStatX(float x);
    void SetStatY(float y);
    void SetStripDecimals(bool strip);
    void SetTickLength(float length, QString axis);
    void SetTimeOffset(double toffset);
    void SetTitleAlign(int a);
    void SetTitleBorderSize(int size);
    void SetTitleColor(int color, QString axis);
    void SetTitleFillColor(int color);
    void SetTitleFont(int font, QString axis);
    void SetTitleFontSize(float size);
    void SetTitleH(float h);
    void SetTitleOffset(float offset, QString axis);
    void SetTitlePS(QString pstitle);
    void SetTitleSize(float size, QString axis);
    void SetTitleStyle(int style);
    void SetTitleTextColor(int color);
    void SetTitleW(float w);
    void SetTitleX(float x);
    void SetTitleXOffset(float offset);
    void SetTitleXSize(float size);
    void SetTitleY(float y);
    void SetTitleYOffset(float offset);
    void SetTitleYSize(float size);
};

#endif // AROOTSTYLE_SI_H
