/*
 * Copyright (c) 2014, Dominic Michael Laurentius
 * Copyright (c) 2016, Pedro Fernando Arizpe Gomez


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
#ifndef AVCONTROLLER_H
#define AVCONTROLLER_H

#include <QMutex>
#include <QString>
#include <QColor>

class AVKinector;
class AVMainWindow;
class AVPluginManager;
class AVModel;
class AVGLWidget;
class AVPQReader;

class AVController
{
public:

    static AVController* instance()
    {
        static QMutex mutex;
        if(!m_instance)
        {
            mutex.lock();
            if(!m_instance) m_instance = new AVController;
            mutex.unlock();
        }
        return m_instance;
    }

    static void destroy()
    {
        static QMutex mutex;
        mutex.lock();
        delete m_instance;
        m_instance = 0;
        mutex.unlock();
    }

    int readFile(QString filename);
    void saveXmlFile();

private:

    AVController();
    AVController(const AVController &); //hide copy constructor
    AVController& operator=(const AVController &); //hide assign op

    ~AVController();

    static AVController*    m_instance;

    AVMainWindow*           m_mainWindow;
    AVPluginManager*        m_pluginManager;
    AVModel*                m_model;
    AVGLWidget*             m_glWidget;
    AVPQReader*             m_pqReader;
    AVKinector*             m_Kinector;

    QString                 m_currentlyOpenFile;
    bool                    m_xmlFileAlreadyExists;

public:

    static QColor      QVector3DToQColor(QVector3D vector);
    static QString     QVector4DToQString(QVector4D vector);
    static QString     QMatrix4x4ToQString(QMatrix4x4 matrix);
    static QVector4D   QStringToQVector4D(QString string);
    static QMatrix4x4  QStringToQMatrix4x4(QString string);
};

#endif // AVCONTROLLER_H
