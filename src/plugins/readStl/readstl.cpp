#include "readstl.h"
#include <QFile>
#include <iostream>


bool ReadStl::init()
{
    return true;
}

QStringList ReadStl::fileTypes()
{
    return QStringList("stl");
}

void ReadStl::setFilename(const QString &filename)
{
    m_fileName = filename;
}

QString ReadStl::getFilename()
{
    return m_fileName;
}

int ReadStl::readFile(QVector<QVector3D> &vertices, QVector<QVector3D> &colors, QVector<unsigned int> &triangles)
{

    /*
    vertices.clear();
    colors.clear();
    triangles.clear();
    //normals.clear();

    int triangleAmount = 0;
    qint8 tempByte = 0;
    short tempShort = 0;
    float vectorX = 0.0;
    float vectorY = 0.0;
    float vectorZ = 0.0;
    float normX = 0.0;
    float normY = 0.0;
    float normZ = 0.0;

    QFile stlFile(m_fileName);
    if (!stlFile.open(QIODevice::ReadOnly)) return 1;
    QDataStream stream(&stlFile);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    for (int i=0; i<80; i++) stream >> tempByte;
    stream >> triangleAmount;

    for (int i=0; i < triangleAmount; i++)
    {
        stream >> normX;
        stream >> normY;
        stream >> normZ;
        stream >> vectorX;
        stream >> vectorY;
        stream >> vectorZ;

        vertices.append(QVector3D(vectorX, vectorY, vectorZ));
        triangles.append(vertices.size()-1);

        stream >> vectorX;
        stream >> vectorY;
        stream >> vectorZ;
        vertices.append(QVector3D(vectorX, vectorY, vectorZ));
        triangles.append(vertices.size()-1);

        stream >> vectorX;
        stream >> vectorY;
        stream >> vectorZ;
        vertices.append(QVector3D(vectorX, vectorY, vectorZ));
        triangles.append(vertices.size()-1);

        stream >> tempShort;
    }

    for (int i=0; i < vertices.size(); i++) colors.append(QVector3D(1,1,1));

    stlFile.close();

    return 0;
    */

    //make sure the arrays for the geometry are empty
    vertices.clear();
    colors.clear();
    triangles.clear();

    //some variables
    quint32 triangleAmount = 0;
    quint8 tempByte;
    quint16 tempShort;

    float vectorX;
    float vectorY;
    float vectorZ;
    float normX;
    float normY;
    float normZ;

    bool readOptocadColor = false;

    //open the file
    QFile stlFile(m_fileName);
    if (!stlFile.open(QIODevice::ReadOnly)) return 1;

    //stream used to read from file
    QDataStream stream(&stlFile);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    //read the header (80 bytes)
    QByteArray bytes;
    for (int i=0; i<80; i++)
    {
        stream >> tempByte;
        bytes.append((char)tempByte);
    }

    //check if the file was generated with OPTOCAD (such files have certain color info)
    if(QString(bytes).startsWith(QString("File generated by OPTOCAT")))
    {
        std::cout << "OPTOCAD File" << std::endl;
        readOptocadColor = true;
    }
    else
    {
        std::cout << "Format unrecognized" << std::endl;
        readOptocadColor = false;
    }

    //get number of triangles in file
    stream >> triangleAmount;
    std::cout << "stl num triangles : " << triangleAmount << std::endl;

    //for each triangle
    for (unsigned int i=0; i < triangleAmount; i++)
    {
        //stream normal and forget it
        stream >> normX;
        stream >> normY;
        stream >> normZ;

        //stream 1st vertex and store it
        stream >> vectorX;
        stream >> vectorY;
        stream >> vectorZ;

        vertices.append(QVector3D(vectorX, vectorY, vectorZ));
        triangles.append(vertices.size()-1);

        //stream 2nd vertex and store it
        stream >> vectorX;
        stream >> vectorY;
        stream >> vectorZ;

        vertices.append(QVector3D(vectorX, vectorY, vectorZ));
        triangles.append(vertices.size()-1);

        //stream 3rd vertex and store it
        stream >> vectorX;
        stream >> vectorY;
        stream >> vectorZ;

        vertices.append(QVector3D(vectorX, vectorY, vectorZ));
        triangles.append(vertices.size()-1);

        //stream 16bit short with optional color info
        stream >> tempShort;

        //if short has color info read and store it.
        if(readOptocadColor)
        {
            unsigned int blue;
            blue = tempShort & 31;
            tempShort = tempShort >> 5;

            unsigned int green;
            green = tempShort & 31;
            tempShort = tempShort >> 5;

            unsigned int red;
            red = tempShort & 31;
            tempShort = tempShort >> 5;

            QVector3D optoColor(blue / 32.0f, green / 32.0f , red / 32.0f);
            colors.append(optoColor);
            colors.append(optoColor);
            colors.append(optoColor);
        }
        else //otherwise make everything kinda greyish
        {
            QVector3D unicolor(0.8f, 0.8f, 0.8f);
            colors.append(unicolor);
            colors.append(unicolor);
            colors.append(unicolor);
        }

    }
    return 0;
}

