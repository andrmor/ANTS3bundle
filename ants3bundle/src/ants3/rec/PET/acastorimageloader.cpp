#include "acastorimageloader.h"
#include "afiletools.h"

#include <QFileInfo>
#include <QStringList>
#include <QDebug>

#include <fstream>

ACastorImageLoader::ACastorImageLoader() {}

QString ACastorImageLoader::loadImage(const QString & fileName)
{
    QFileInfo fi(fileName);
    if (fi.suffix() != "hdr") return "File name should have suffix 'hdr'";

    QString header;
    bool ok = ftools::loadTextFromFile(header, fileName);
    if (!ok || header.isEmpty()) return "Could not read image header file or it is empty";

    const QStringList sl = header.split('\n', Qt::SkipEmptyParts);

    for (const QString & line : sl)
    {
        QString txt = line.simplified();
        if (txt.isEmpty()) continue;

        qDebug() << txt;
        QStringList fields = txt.split(":=", Qt::SkipEmptyParts);
        if (fields.size() != 2) continue;

        const QString key = fields.front();
        bool ok;
        if (key.contains("!matrix size"))
        {
            int num = fields[1].toInt(&ok);
            if (!ok || num < 1) return "Format error in the image header";
            if      (key.contains('1')) NumBinsX = num;
            else if (key.contains('2')) NumBinsY = num;
            else if (key.contains('3')) NumBinsZ = num;
            else return "Format error in the image header";
            continue;
        }

        if (key.contains("scaling factor (mm/pixel)"))
        {
            double val = fields[1].toDouble(&ok);
            if (!ok) return "Format error in the image header";
            if      (key.contains('1')) mmPerPixelX = val;
            else if (key.contains('2')) mmPerPixelY = val;
            else if (key.contains('3')) mmPerPixelZ = val;
            else return "Format error in the image header";
            continue;
        }

        if (key.contains("first pixel offset (mm)"))
        {
            double val = fields[1].toDouble(&ok);
            if (!ok) return "Format error in the image header";
            if      (key.contains('1')) OffsetX = val;
            else if (key.contains('2')) OffsetY = val;
            else if (key.contains('3')) OffsetZ = val;
            else return "Format error in the image header";
            continue;
        }
    }

    qDebug() << "---> Num bins:" << NumBinsX << NumBinsY << NumBinsZ;
    qDebug() << "---> mm/pixel:" << mmPerPixelX << mmPerPixelY << mmPerPixelZ;
    qDebug() << "---> Offset:" << OffsetX << OffsetY << OffsetZ;

    Data.resize(NumBinsX);
    for (int ix = 0; ix < NumBinsX; ix++)
    {
        Data[ix].resize(NumBinsY);
        for (int iy = 0; iy < NumBinsY; iy++)
            Data[ix][iy].resize(NumBinsZ, 0);
    }

    QString binFileName = fileName;
    binFileName.replace(".hdr", ".img");

    std::ifstream inStream;
    inStream.open(binFileName.toLatin1().data(), std::ios::in | std::ios::binary);

    if (!inStream.is_open() || inStream.fail() || inStream.bad()) return "Cannot open image file: " + binFileName;

    float buffer;
    for (int iz = 0; iz < NumBinsZ; iz++)
        for (int iy = 0; iy < NumBinsY; iy++)
            for (int ix = 0; ix < NumBinsX; ix++)
            {
                inStream.read((char*)&buffer, sizeof(float));
                Data[ix][iy][iz] = buffer;
            }

    // !!!*** file read error control

    return "";
}
