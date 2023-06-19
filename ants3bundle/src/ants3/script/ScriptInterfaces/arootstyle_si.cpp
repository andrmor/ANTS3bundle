#include "arootstyle_si.h"

#include "TStyle.h"

ARootStyle_SI::ARootStyle_SI() : AScriptInterface()
{
    Description = "See //https://root.cern.ch/root/html/TStyle.html - essentially all Set... methods are implemented";

    QString str = "The parameter mode can be any combination of kKsSiourRmMen.\n"
                "k : kurtosis\n"
                "K : kurtosis and kurtosis error\n"
                "s : skewness\n"
                "S : skewness and skewness error\n"
                "i : integral of bins\n"
                "I : integral of bins with option \"width\"\n"
                "o : number of overflows\n"
                "u : number of underflows\n"
                "r : rms\n"
                "R : rms and rms error\n"
                "m : mean value\n"
                "M : mean value mean error values\n"
                "e : number of entries\n"
                "n : name of histogram";
    Help["SetOptStat"] = str;
}

void ARootStyle_SI::SetAxisColor(int color, QString axis) {gStyle->SetAxisColor(color, axis.toLocal8Bit());}

void ARootStyle_SI::SetBarOffset(float baroff) {gStyle->SetBarOffset(baroff);}

void ARootStyle_SI::SetBarWidth(float barwidth) {gStyle->SetBarWidth(barwidth);}

void ARootStyle_SI::SetCanvasBorderMode(int mode) {gStyle->SetCanvasBorderMode(mode);}

void ARootStyle_SI::SetCanvasBorderSize(int size) {gStyle->SetCanvasBorderSize(size);}

void ARootStyle_SI::SetCanvasColor(int color) {gStyle->SetCanvasColor(color);}

void ARootStyle_SI::SetCanvasDefH(int h) {gStyle->SetCanvasDefH(h);}

void ARootStyle_SI::SetCanvasDefW(int w) {gStyle->SetCanvasDefW(w);}

void ARootStyle_SI::SetCanvasDefX(int topx) {gStyle->SetCanvasDefX(topx);}

void ARootStyle_SI::SetCanvasDefY(int topy) {gStyle->SetCanvasDefY(topy);}

void ARootStyle_SI::SetCanvasPreferGL(bool prefer) {gStyle->SetCanvasPreferGL(prefer);}

void ARootStyle_SI::SetColorModelPS(int c) {gStyle->SetColorModelPS(c);}

void ARootStyle_SI::SetDateX(float x) {gStyle->SetDateX(x);}

void ARootStyle_SI::SetDateY(float y) {gStyle->SetDateY(y);}

void ARootStyle_SI::SetDrawBorder(int drawborder) {gStyle->SetDrawBorder(drawborder);}

void ARootStyle_SI::SetEndErrorSize(float np) {gStyle->SetEndErrorSize(np);}

void ARootStyle_SI::SetErrorX(float errorx) {gStyle->SetErrorX(errorx);}

void ARootStyle_SI::SetFitFormat(QString format) {gStyle->SetFitFormat(format.toLocal8Bit());}

void ARootStyle_SI::SetFrameBorderMode(int mode) {gStyle->SetFrameBorderMode(mode);}

void ARootStyle_SI::SetFrameBorderSize(int size) {gStyle->SetFrameBorderSize(size);}

void ARootStyle_SI::SetFrameFillColor(int color) {gStyle->SetFrameFillColor(color);}

void ARootStyle_SI::SetFrameFillStyle(int style) {gStyle->SetFrameFillStyle(style);}

void ARootStyle_SI::SetFrameLineColor(int color) {gStyle->SetFrameLineColor(color);}

void ARootStyle_SI::SetFrameLineStyle(int style) {gStyle->SetFrameLineStyle(style);}

void ARootStyle_SI::SetFrameLineWidth(int width) {gStyle->SetFrameLineWidth(width);}

void ARootStyle_SI::SetFuncColor(int color) {gStyle->SetFuncColor(color);}

void ARootStyle_SI::SetFuncStyle(int style) {gStyle->SetFuncStyle(style);}

void ARootStyle_SI::SetFuncWidth(int width) {gStyle->SetFuncWidth(width);}

void ARootStyle_SI::SetGridColor(int color) {gStyle->SetGridColor(color);}

void ARootStyle_SI::SetGridStyle(int style) {gStyle->SetGridStyle(style);}

void ARootStyle_SI::SetGridWidth(int width) {gStyle->SetGridWidth(width);}

void ARootStyle_SI::SetHatchesLineWidth(int l) {gStyle->SetHatchesLineWidth(l);}

void ARootStyle_SI::SetHatchesSpacing(double h) {gStyle->SetHatchesSpacing(h);}

void ARootStyle_SI::SetHeaderPS(QString header) {gStyle->SetHeaderPS(header.toLocal8Bit());}

void ARootStyle_SI::SetHistFillColor(int color) {gStyle->SetHistFillColor(color);}

void ARootStyle_SI::SetHistFillStyle(int style) {gStyle->SetHistFillStyle(style);}

void ARootStyle_SI::SetHistLineColor(int color) {gStyle->SetHistLineColor(color);}

void ARootStyle_SI::SetHistLineStyle(int style) {gStyle->SetHistLineStyle(style);}

void ARootStyle_SI::SetHistLineWidth(int width) {gStyle->SetHistLineWidth(width);}

void ARootStyle_SI::SetHistMinimumZero(bool zero) {gStyle->SetHistMinimumZero(zero);}

void ARootStyle_SI::SetHistTopMargin(double hmax) {gStyle->SetHistTopMargin(hmax);}

void ARootStyle_SI::SetIsReading(bool reading) {gStyle->SetIsReading(reading);}

void ARootStyle_SI::SetLabelColor(int color, QString axis) {gStyle->SetLabelColor(color, axis.toLocal8Bit());}

void ARootStyle_SI::SetLabelFont(int font, QString axis) {gStyle->SetLabelFont(font, axis.toLocal8Bit());}

void ARootStyle_SI::SetLabelOffset(float offset, QString axis) {gStyle->SetLabelOffset(offset, axis.toLocal8Bit());}

void ARootStyle_SI::SetLabelSize(float size, QString axis) {gStyle->SetLabelSize(size, axis.toLocal8Bit());}

void ARootStyle_SI::SetLegendBorderSize(int size) {gStyle->SetLegendBorderSize(size);}

void ARootStyle_SI::SetLegendFillColor(int color){gStyle->SetLegendFillColor(color);}

void ARootStyle_SI::SetLegendFont(int font) {gStyle->SetLegendFont(font);}

void ARootStyle_SI::SetLegoInnerR(float rad){gStyle->SetLegoInnerR(rad);}

void ARootStyle_SI::SetLineScalePS(float scale) {gStyle->SetLineScalePS(scale);}

void ARootStyle_SI::SetLineStyleString(int i, QString text){gStyle->SetLineStyleString(i, text.toLocal8Bit());}

void ARootStyle_SI::SetNdivisions(int n, QString axis) {gStyle->SetNdivisions(n, axis.toLocal8Bit());}

void ARootStyle_SI::SetNumberContours(int number)  {gStyle->SetNumberContours(number);}

void ARootStyle_SI::SetOptDate(int datefl) {gStyle->SetOptDate(datefl);}

void ARootStyle_SI::SetOptFile(int file) {gStyle->SetOptFile(file);}

void ARootStyle_SI::SetOptFit(int fit) {gStyle->SetOptFit(fit);}

void ARootStyle_SI::SetOptLogx(int logx) {gStyle->SetOptLogx(logx);}

void ARootStyle_SI::SetOptLogy(int logy) {gStyle->SetOptLogy(logy);}

void ARootStyle_SI::SetOptLogz(int logz) {gStyle->SetOptLogz(logz);}

//void ARootStyle_SI::SetOptStat(int stat) {gStyle->SetOptStat(stat);}
//void ARootStyle_SI::SetOptStat(QString stat) {gStyle->SetOptStat(stat.toLocal8Bit().constData());}
void ARootStyle_SI::SetOptStat(QVariant stat)
{
    int i = 0;
    bool ok;
    i = stat.toInt(&ok);
    if (ok) gStyle->SetOptStat(i);
    else
    {
        QString str = stat.toString();
        gStyle->SetOptStat(str.toLocal8Bit().constData());
    }
}

void ARootStyle_SI::SetOptTitle(int tit) {gStyle->SetOptTitle(tit);}

void ARootStyle_SI::SetPadBorderMode(int mode) {gStyle->SetPadBorderMode(mode);}

void ARootStyle_SI::SetPadBorderSize(int size) {gStyle->SetPadBorderSize(size);}

void ARootStyle_SI::SetPadBottomMargin(float margin) {gStyle->SetPadBottomMargin(margin);}

void ARootStyle_SI::SetPadColor(int color) {gStyle->SetPadColor(color);}

void ARootStyle_SI::SetPadGridX(bool gridx) {gStyle->SetPadGridX(gridx);}

void ARootStyle_SI::SetPadGridY(bool gridy) {gStyle->SetPadGridY(gridy);}

void ARootStyle_SI::SetPadLeftMargin(float margin) {gStyle->SetPadLeftMargin(margin);}

void ARootStyle_SI::SetPadRightMargin(float margin) {gStyle->SetPadRightMargin(margin);}

void ARootStyle_SI::SetPadTickX(int tickx) {gStyle->SetPadTickX(tickx);}

void ARootStyle_SI::SetPadTickY(int ticky) {gStyle->SetPadTickY(ticky);}

void ARootStyle_SI::SetPadTopMargin(float margin) {gStyle->SetPadTopMargin(margin);}

void ARootStyle_SI::SetPaintTextFormat(QString format) {gStyle->SetPaintTextFormat(format.toLocal8Bit());}

void ARootStyle_SI::SetPalette(int scheme) {gStyle->SetPalette(scheme);}

void ARootStyle_SI::SetPaperSize(float xsize, float ysize) {gStyle->SetPaperSize(xsize, ysize);}

void ARootStyle_SI::SetScreenFactor(float factor) {gStyle->SetScreenFactor(factor);}

void ARootStyle_SI::SetStatBorderSize(int size) {gStyle->SetStatBorderSize(size);}

void ARootStyle_SI::SetStatColor(int color) {gStyle->SetStatColor(color);}

void ARootStyle_SI::SetStatFont(int font) {gStyle->SetStatFont(font);}

void ARootStyle_SI::SetStatFontSize(float size) {gStyle->SetStatFontSize(size);}

void ARootStyle_SI::SetStatFormat(QString format) {gStyle->SetStatFormat(format.toLocal8Bit());}

void ARootStyle_SI::SetStatH(float h) {gStyle->SetStatH(h);}

void ARootStyle_SI::SetStatStyle(int style) {gStyle->SetStatStyle(style);}

void ARootStyle_SI::SetStatTextColor(int color) {gStyle->SetStatTextColor(color);}

void ARootStyle_SI::SetStatW(float w) {gStyle->SetStatW(w);}

void ARootStyle_SI::SetStatX(float x) {gStyle->SetStatX(x);}

void ARootStyle_SI::SetStatY(float y) {gStyle->SetStatY(y);}

void ARootStyle_SI::SetStripDecimals(bool strip) {gStyle->SetStripDecimals(strip);}

void ARootStyle_SI::SetTickLength(float length, QString axis) {gStyle->SetTickLength(length, axis.toLocal8Bit());}

void ARootStyle_SI::SetTimeOffset(double toffset) {gStyle->SetTimeOffset(toffset);}

void ARootStyle_SI::SetTitleAlign(int a) {gStyle->SetTitleAlign(a);}

void ARootStyle_SI::SetTitleBorderSize(int size) {gStyle->SetTitleBorderSize(size);}

void ARootStyle_SI::SetTitleColor(int color, QString axis) {gStyle->SetTitleColor(color, axis.toLocal8Bit());}

void ARootStyle_SI::SetTitleFillColor(int color) {gStyle->SetTitleFillColor(color);}

void ARootStyle_SI::SetTitleFont(int font, QString axis) {gStyle->SetTitleFont(font, axis.toLocal8Bit());}

void ARootStyle_SI::SetTitleFontSize(float size) {gStyle->SetTitleFontSize(size);}

void ARootStyle_SI::SetTitleH(float h) {gStyle->SetTitleH(h);}

void ARootStyle_SI::SetTitleOffset(float offset, QString axis) {gStyle->SetTitleOffset(offset, axis.toLocal8Bit());}

void ARootStyle_SI::SetTitlePS(QString pstitle) {gStyle->SetTitlePS(pstitle.toLocal8Bit());}

void ARootStyle_SI::SetTitleSize(float size, QString axis) {gStyle->SetTitleSize(size, axis.toLocal8Bit());}

void ARootStyle_SI::SetTitleStyle(int style) {gStyle->SetTitleStyle(style);}

void ARootStyle_SI::SetTitleTextColor(int color) {gStyle->SetTitleTextColor(color);}

void ARootStyle_SI::SetTitleW(float w) {gStyle->SetTitleW(w);}

void ARootStyle_SI::SetTitleX(float x) {gStyle->SetTitleX(x);}

void ARootStyle_SI::SetTitleXOffset(float offset) {gStyle->SetTitleXOffset(offset);}

void ARootStyle_SI::SetTitleXSize(float size) {gStyle->SetTitleXSize(size);}

void ARootStyle_SI::SetTitleY(float y) {gStyle->SetTitleY(y);}

void ARootStyle_SI::SetTitleYOffset(float offset) {gStyle->SetTitleYOffset(offset);}

void ARootStyle_SI::SetTitleYSize(float size) {gStyle->SetTitleYSize(size);}


