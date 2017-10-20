/*
 * Copyright (c) 2014, Dominic Michael Laurentius


All rights reserved.


Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PLUGINFILEREADERINTERFACE_H
#define PLUGINFILEREADERINTERFACE_H

#include <QtPlugin>

QT_BEGIN_NAMESPACE
class QString;
class QStringList;
//class QVector;
//class QVector3D;
QT_END_NAMESPACE

class AVModel;

class ReaderInterface
{
public:
    virtual ~ReaderInterface() {}

    virtual bool init() = 0;

    virtual QStringList fileTypes() = 0;
    virtual void setFilename(const QString& filename) = 0;
    virtual QString getFilename() = 0;

    virtual int readFile(QVector<QVector3D>& vertices, QVector<QVector3D>& colors, QVector<unsigned int>& triangles) = 0;
};


QT_BEGIN_NAMESPACE
#define ReaderInterface_iid "av.reader.0.1"
Q_DECLARE_INTERFACE(ReaderInterface, ReaderInterface_iid)
QT_END_NAMESPACE

#endif // PLUGINFILEREADERINTERFACE_H
