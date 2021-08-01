#include "abasicinterfacerule.h"
#include "aphoton.h"
#include "amaterial.h"
#include "amaterialhub.h"
#include "atracerstateful.h"
//#include "asimulationstatistics.h"
#include "ajsontools.h"

#include <QJsonObject>

#include "TMath.h"
#include "TRandom2.h"

#ifdef GUI
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleValidator>
#endif

ABasicInterfaceRule::ABasicInterfaceRule(int MatFrom, int MatTo)
    : AInterfaceRule(MatFrom, MatTo) {}

AInterfaceRule::OpticalOverrideResultEnum ABasicInterfaceRule::calculate(ATracerStateful &Resources, APhoton *Photon, const double *NormalVector)
{
    double rnd = Resources.RandGen->Rndm();

    // surface loss?
    rnd -= Abs;
    if (rnd < 0)
    {
        // qDebug()<<"Override: surface loss!";
        Status = Absorption;
        return Absorbed;
    }

    // specular reflection?
    rnd -= Spec;
    if (rnd<0)
    {
        // qDebug()<<"Override: specular reflection!";
        //rotating the vector: K = K - 2*(NK)*N
        double NK = NormalVector[0]*Photon->v[0]; NK += NormalVector[1]*Photon->v[1];  NK += NormalVector[2]*Photon->v[2];
        Photon->v[0] -= 2.0*NK*NormalVector[0]; Photon->v[1] -= 2.0*NK*NormalVector[1]; Photon->v[2] -= 2.0*NK*NormalVector[2];

        Status = SpikeReflection;
        return Back;
    }

    // scattering?
    rnd -= Scat;
    if (rnd<0)
    {
        // qDebug()<<"scattering triggered";

        switch (ScatterModel)
        {
        case 0: //4Pi scattering
            // qDebug()<<"4Pi scatter";
            Photon->RandomDir(Resources.RandGen);
            // qDebug()<<"New direction:"<<K[0]<<K[1]<<K[2];

            //enering new volume or backscattering?
            //normal is in the positive direction in respect to the original direction!
            if (Photon->v[0]*NormalVector[0] + Photon->v[1]*NormalVector[1] + Photon->v[2]*NormalVector[2] < 0)
            {
                // qDebug()<<"   scattering back";
                Status = LambertianReflection;
                return Back;
            }
            // qDebug()<<"   continuing to the next volume";
            Status = Transmission;
            return Forward;

        case 1: //2Pi lambertian, remaining in the same volume (back scattering)
        {
            // qDebug()<<"2Pi lambertian scattering backward";
            double norm2;
            do
            {
                Photon->RandomDir(Resources.RandGen);
                Photon->v[0] -= NormalVector[0]; Photon->v[1] -= NormalVector[1]; Photon->v[2] -= NormalVector[2];
                norm2 = Photon->v[0]*Photon->v[0] + Photon->v[1]*Photon->v[1] + Photon->v[2]*Photon->v[2];
            }
            while (norm2 < 0.000001);

            double normInverted = 1.0/TMath::Sqrt(norm2);
            Photon->v[0] *= normInverted; Photon->v[1] *= normInverted; Photon->v[2] *= normInverted;
            Status = LambertianReflection;
            return Back;
        }
        case 2: //2Pi lambertian, scattering to the next volume
        {
            // qDebug()<<"2Pi lambertian scattering forward";
            double norm2;
            do
            {
                Photon->RandomDir(Resources.RandGen);
                Photon->v[0] += NormalVector[0]; Photon->v[1] += NormalVector[1]; Photon->v[2] += NormalVector[2];
                norm2 = Photon->v[0]*Photon->v[0] + Photon->v[1]*Photon->v[1] + Photon->v[2]*Photon->v[2];
            }
            while (norm2 < 0.000001);

            double normInverted = 1.0/TMath::Sqrt(norm2);
            Photon->v[0] *= normInverted; Photon->v[1] *= normInverted; Photon->v[2] *= normInverted;
            Status = Transmission;
            return Forward;
        }
        }
    }

    // overrides NOT triggered - what is left is covered by Fresnel in the tracker code
    // qDebug()<<"Overrides did not trigger, using fresnel";
    Status = Transmission;
    return NotTriggered;
}

QString ABasicInterfaceRule::getReportLine() const
{
    double probFresnel = 1.0 - (Spec + Scat + Abs);
    QString s;
    if (Abs > 0) s += QString("Abs %1 +").arg(Abs);
    if (Spec > 0)  s += QString("Spec %1 +").arg(Spec);
    if (Scat > 0)
    {
        switch( ScatterModel )
        {
        case 0:
            s += "Iso ";
            break;
        case 1:
            s += "Lamb_B ";
            break;
        case 2:
            s += "Lamb_F ";
            break;
        }
        s += QString::number(Scat);
        s += " +";
    }
    if (probFresnel > 1e-10) s += QString("Fres %1 +").arg(probFresnel);
    s.chop(2);
    return s;
}

QString ABasicInterfaceRule::getLongReportLine() const
{
    QString s = "--> Simplistic <--\n";
    if (Abs > 0) s += QString("Absorption: %1%\n").arg(100.0 * Abs);
    if (Spec > 0)  s += QString("Specular reflection: %1%\n").arg(100.0 * Spec);
    if (Scat)
    {
        s += QString("Scattering: %1%").arg(100.0 * Scat);
        switch (ScatterModel)
        {
        case 0: s += " (isotropic)\n"; break;
        case 1: s += " (Lambertian, back)\n"; break;
        case 2: s += " (Lambertian, forward)\n"; break;
        }
    }
    double fres = 1.0 - Abs - Spec - Scat;
    if (fres > 0) s += QString("Fresnel: %1%").arg(100.0 * fres);
    return s;
}

void ABasicInterfaceRule::writeToJson(QJsonObject &json) const
{
    AInterfaceRule::writeToJson(json);

    json["Abs"]      = Abs;
    json["Spec"]     = Spec;
    json["Scat"]     = Scat;
    json["ScatMode"] = ScatterModel;
}

bool ABasicInterfaceRule::readFromJson(const QJsonObject & json)
{
    if ( !jstools::parseJson(json, "Abs",      Abs) )          return false;
    if ( !jstools::parseJson(json, "Spec",     Spec) )         return false;
    if ( !jstools::parseJson(json, "Scat",     Scat) )         return false;
    if ( !jstools::parseJson(json, "ScatMode", ScatterModel) ) return false;
    return true;
}

#ifdef GUI
QWidget *ABasicInterfaceRule::getEditWidget(QWidget*, GraphWindowClass *)
{
    QFrame* f = new QFrame();
    f->setFrameStyle(QFrame::Box);

    QHBoxLayout* hl = new QHBoxLayout(f);
    QVBoxLayout* l = new QVBoxLayout();
    QLabel* lab = new QLabel("Absorption:");
    l->addWidget(lab);
    lab = new QLabel("Specular reflection:");
    l->addWidget(lab);
    lab = new QLabel("Scattering:");
    l->addWidget(lab);
    hl->addLayout(l);
    l = new QVBoxLayout();
    QLineEdit* le = new QLineEdit(QString::number(Abs));
    QDoubleValidator* val = new QDoubleValidator(f);
    val->setNotation(QDoubleValidator::StandardNotation);
    val->setBottom(0);
    //val->setTop(1.0); //Qt(5.8.0) BUG: check does not work
    val->setDecimals(6);
    le->setValidator(val);
    QObject::connect(le, &QLineEdit::editingFinished, [le, this]() { this->Abs = le->text().toDouble(); } );
    l->addWidget(le);
    le = new QLineEdit(QString::number(Spec));
    le->setValidator(val);
    QObject::connect(le, &QLineEdit::editingFinished, [le, this]() { this->Spec = le->text().toDouble(); } );
    l->addWidget(le);
    le = new QLineEdit(QString::number(Scat));
    le->setValidator(val);
    QObject::connect(le, &QLineEdit::editingFinished, [le, this]() { this->Scat = le->text().toDouble(); } );
    l->addWidget(le);
    hl->addLayout(l);
    l = new QVBoxLayout();
    lab = new QLabel("");
    l->addWidget(lab);
    lab = new QLabel("");
    l->addWidget(lab);
    QComboBox* com = new QComboBox();
    com->addItem("Isotropic (4Pi)"); com->addItem("Lambertian, 2Pi back"); com->addItem("Lambertian, 2Pi forward");
    com->setCurrentIndex(ScatterModel);
    QObject::connect(com, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), [this](int index) { this->ScatterModel = index; } );
    l->addWidget(com);
    hl->addLayout(l);

    return f;
}
#endif

QString ABasicInterfaceRule::checkOverrideData()
{
    if (ScatterModel<0 || ScatterModel>2) return "Invalid scatter model";

    if (Abs  < 0 || Abs  > 1.0) return "Absorption probability should be within [0, 1.0]";
    if (Spec < 0 || Spec > 1.0) return "Reflection probability should be within [0, 1.0]";
    if (Scat < 0 || Scat > 1.0) return "Scattering probability should be within [0, 1.0]";

    if (Abs + Spec + Scat > 1.0) return "Sum of all process probabilities cannot exceed 1.0";

    return "";
}
