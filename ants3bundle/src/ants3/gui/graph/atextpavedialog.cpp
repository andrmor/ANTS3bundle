#include "atextpavedialog.h"
#include "ui_atextpavedialog.h"
#include "aroottextconfigurator.h"
#include "arootlineconfigurator.h"

#include <QDebug>
#include <QDoubleValidator>

#include "TPaveText.h"
#include "TList.h"

// default properties
static bool   exportCreated = false;
static double export_x0, export_x1, export_y0, export_y1;
static int    export_text_color, export_text_align, export_text_font;
static float  export_text_size;
static int    export_border_color, export_border_width, export_border_style;


ATextPaveDialog::ATextPaveDialog(TPaveText &Pave, QWidget *parent) :
    QDialog(parent),
    Pave(Pave),
    ui(new Ui::ATextPaveDialog)
{
    ui->setupUi(this);

    ui->pbDummy->setDefault(true);
    ui->pbDummy->setVisible(false);

    TList * list = Pave.GetListOfLines();
    int numLines = list->GetEntries();
    for (int i=0; i<numLines; i++)
        ui->pte->appendPlainText(((TText*)list->At(i))->GetTitle());

    QDoubleValidator * dv = new QDoubleValidator(this);
    dv->setBottom(0);
    dv->setTop(1.0);
    ui->ledX0->setValidator(dv);
    ui->ledX1->setValidator(dv);
    ui->ledY0->setValidator(dv);
    ui->ledY1->setValidator(dv);

    connect(ui->pte, &QPlainTextEdit::blockCountChanged, this, &ATextPaveDialog::updatePave);

    ui->pbImport->setEnabled(exportCreated);

    updateGui();
}

ATextPaveDialog::~ATextPaveDialog()
{
    delete ui;
}

void ATextPaveDialog::updateGui()
{
    ui->ledX0->setText( QString::number(Pave.GetX1NDC(), 'g', 4));
    ui->ledX1->setText( QString::number(Pave.GetX2NDC(), 'g', 4));
    ui->ledY0->setText( QString::number(Pave.GetY1NDC(), 'g', 4));
    ui->ledY1->setText( QString::number(Pave.GetY2NDC(), 'g', 4));
}

void ATextPaveDialog::on_pbDummy_clicked()
{
    updatePave();
}

void ATextPaveDialog::on_pbConfirm_clicked()
{
    updatePave();
    accept();
}

void ATextPaveDialog::on_pbTextAttributes_clicked()
{
    int   color = Pave.GetTextColor();
    int   align = Pave.GetTextAlign();
    int   font  = Pave.GetTextFont();
    float size  = Pave.GetTextSize();

    ARootTextConfigurator D(color, align, font, size, this);
    connect(&D, &ARootTextConfigurator::propertiesChanged, this, [this](int color, int align, int font, float size)
            {
                Pave.SetTextColor(color);
                Pave.SetTextAlign(align);
                Pave.SetTextFont(font);
                Pave.SetTextSize(size);
                updatePave();
            });
    D.exec();

    Pave.SetTextColor(color);
    Pave.SetTextAlign(align);
    Pave.SetTextFont(font);
    Pave.SetTextSize(size);
    updatePave();
}

void ATextPaveDialog::on_pbConfigureFrame_clicked()
{
    int color = Pave.GetLineColor();
    int width = Pave.GetLineWidth();
    int style = Pave.GetLineStyle();

    ARootLineConfigurator RC(color, width, style, this);
    connect(&RC, &ARootLineConfigurator::propertiesChanged, this, [this](int color, int width, int style)
            {
                Pave.SetLineColor(color);
                Pave.SetLineWidth(width);
                Pave.SetLineStyle(style);
                updatePave();
            });

    RC.exec();
    Pave.SetLineColor(color);
    Pave.SetLineWidth(width);
    Pave.SetLineStyle(style);
    updatePave();
}

void ATextPaveDialog::updatePave()
{
    Pave.Clear();

    QString txt = ui->pte->document()->toPlainText();
    const QStringList sl = txt.split("\n");
    for (const QString & s : sl)
        Pave.AddText(s.toLatin1());

    emit requestRedraw();
}

void ATextPaveDialog::on_ledX0_editingFinished()
{
    Pave.SetX1NDC(ui->ledX0->text().toDouble());
    emit requestRedraw();
}

void ATextPaveDialog::on_ledX1_editingFinished()
{
    Pave.SetX2NDC(ui->ledX1->text().toDouble());
    emit requestRedraw();
}

void ATextPaveDialog::on_ledY0_editingFinished()
{
    Pave.SetY1NDC(ui->ledY0->text().toDouble());
    emit requestRedraw();
}

void ATextPaveDialog::on_ledY1_editingFinished()
{
    Pave.SetY2NDC(ui->ledY1->text().toDouble());
    emit requestRedraw();
}

void ATextPaveDialog::on_pbExport_clicked()
{
    exportCreated = true;
    ui->pbImport->setEnabled(true);

    export_x0 = Pave.GetX1NDC();
    export_x1 = Pave.GetX2NDC();
    export_y0 = Pave.GetY1NDC();
    export_y1 = Pave.GetY2NDC();

    export_text_color = Pave.GetTextColor();
    export_text_align = Pave.GetTextAlign();
    export_text_font  = Pave.GetTextFont();
    export_text_size  = Pave.GetTextSize();

    export_border_color = Pave.GetLineColor();
    export_border_width = Pave.GetLineWidth();
    export_border_style = Pave.GetLineStyle();
}

void ATextPaveDialog::on_pbImport_clicked()
{
    if (!exportCreated) return;

    Pave.SetX1NDC(export_x0);
    Pave.SetX2NDC(export_x1);
    Pave.SetY1NDC(export_y0);
    Pave.SetY2NDC(export_y1);

    Pave.SetTextColor(export_text_color);
    Pave.SetTextAlign(export_text_align);
    Pave.SetTextFont(export_text_font);
    Pave.SetTextSize(export_text_size);

    Pave.SetLineColor(export_border_color);
    Pave.SetLineWidth(export_border_width);
    Pave.SetLineStyle(export_border_style);

    updateGui();
    updatePave();
    emit requestRedraw();
}

