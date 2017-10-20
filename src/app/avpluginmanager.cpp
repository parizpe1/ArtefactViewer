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

#include "avpluginmanager.h"
#include "avmainwindow.h"
#include "avplugininterfaces.h"
#include "avcontroller.h"
#include "avpqreader.h"

#include <iostream>

#include <QPluginLoader>


AVPluginManager* AVPluginManager::m_instance = 0;

AVPluginManager::AVPluginManager(QObject *parent) :
    QObject(parent)
{
}

AVPluginManager::~AVPluginManager()
{

}


QStringList AVPluginManager::getReadableFileTypes()
{
    return m_readerPluginMap.uniqueKeys();
}


ReaderInterface *AVPluginManager::getReaderForFileType(QString filetype)
{
    return m_readerPluginMap.value(filetype.toLower());
}


void AVPluginManager::setPluginsDir(const QString &dirName)
{
    std::cout << "pluginsDir: " << dirName.toStdString().c_str() << std::endl;
    m_pluginsDir = QDir(dirName);
}


void AVPluginManager::loadPlugins()
{
    foreach(QString fileName, m_pluginsDir.entryList(QDir::Files))
    {
     //if clauses to shorten standard output
        if(fileName.toStdString()=="readStld.dll")
            std::cout << "checking file: " << fileName.toStdString().c_str() << std::endl;
        if(fileName.toStdString()=="readPlyd.dll")
            std::cout << "checking file: " << fileName.toStdString().c_str() << std::endl;

        QPluginLoader loader(m_pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        if(!plugin)
        {
          //if clauses to shorten standard output
            if(fileName.toStdString()=="readStld.dll")
            std::cout << "not a plugin: " << fileName.toStdString().c_str() << std::endl;
            if(fileName.toStdString()=="readPlyd.dll")
            std::cout << "not a plugin: " << fileName.toStdString().c_str() << std::endl;
            continue;
        }

        std::cout << "loaded plugin: " << fileName.toStdString().c_str() << " successfully" << std::endl;


        ReaderInterface* iReader = qobject_cast<ReaderInterface*>(plugin);
        if(iReader)
        {
            std::cout << "casted plugin to reader" << std::endl;

            if(!iReader->init())
            {
                std::cout << "Plugin init unsucessful" << std::endl;
                continue;
            }

            QStringList fileTypes = iReader->fileTypes();
            for(int i = 0; i < fileTypes.size(); i++)
            {
                m_readerPluginMap.insertMulti(fileTypes.at(i), iReader);
            }
        }
    }
}

