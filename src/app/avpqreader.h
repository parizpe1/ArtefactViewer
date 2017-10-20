/*
 * Copyright (c) 2016, Pedro Fernando Arizpe Gomez

//  PQLabs.
//
//  Copyright (c) PQLabs.  All rights reserved.
//
//  File:       PQMTClient.h
//
//  Contents:   SDK APIs for MultiTouch Server.
//
//  Date:		2008-12-19

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PQ_MULTITOUCH_CLIENT_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PQMT_CLIENT_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
* 
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

#ifndef AVPQREADER_H
#define AVPQREADER_H
#define NOMINMAX
#include "PQMTClient.h"
#include "Kinect.h"
#include <QMutex>
#include <QPoint>
#include <QPointF>
#include "avglwidget.h"
#include "avpointframe.h"

using namespace PQ_SDK_MultiTouch;

class AVPQReader : public QObject
{
    Q_OBJECT

public:

    static AVPQReader* instance()
    {
        static QMutex mutex;
        if(!m_instance)
        {
            mutex.lock();
            if(!m_instance) m_instance = new AVPQReader;
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
        DisconnectServer();
    }

    int Init();
//    void setGLWidget(AVGLWidget* glWidget);
    AVTouchPoint e;
    AVPointFrame pf;
    static const int moveResolution=10;

signals:
//    void throwEvent(AVTouchPoint event);
    void throwPF(AVPointFrame pFrame);
private:
    explicit AVPQReader(QObject* parent = 0);
    // we leave just the declarations, so the compiler will warn us
    // if we try to use those two functions by accident
    AVPQReader(const AVPQReader &); //hide copy constructor
    AVPQReader& operator=(const AVPQReader &); //hide assign op
    ~AVPQReader();
    AVGLWidget*         m_glWidget;
    static AVPQReader* m_instance;

    //////////////////////call back functions///////////////////////
    // OnReceivePointFrame: function to handle when recieve touch point frame
    //	the unmoving touch point won't be sent from server. The new touch point with its pointevent is TP_DOWN
    //	and the leaving touch point with its pointevent will be always sent from server;
    static void OnReceivePointFrame(int frame_id,int time_stamp,int moving_point_count,const TouchPoint * moving_point_array, void * call_back_object);

//    // OnReceiveGesture: function to handle when recieve a gesture
    static void OnReceiveGesture(const TouchGesture & ges, void * call_back_object);

    // OnServerBreak: function to handle when server break(disconnect or network error)
    static void OnServerBreak(void * param, void * call_back_object);

    // OnReceiveError: function to handle when some errors occur on the process of receiving touch datas.
    static void OnReceiveError(int err_code,void * call_back_object);

    // OnGetServerResolution: function to get the resolution of the server system.attention: not the resolution of touch screen.
    static void OnGetServerResolution(int x, int y, void * call_back_object);

    // OnGetDeviceInfo: function to get the information of the touch device.
    static void OnGetDeviceInfo(const TouchDeviceInfo & device_info, void * call_back_object);

//////////////////////call back functions end ///////////////////////

    // functions to handle TouchGestures, attention the means of the params
    void InitFuncOnTG();

    // set the call back functions while reciving touch data;
    void SetFuncsOnReceiveProc();

    // OnTouchPoint: function to handle TouchPoint
    void OnTouchPoint(const AVPointFrame & pf);

    // OnTouchGesture: function to handle TouchGesture
//    void OnTouchGesture(const TouchGesture & tg);
    //

    //here use function pointer table to handle the different gesture types;
    typedef void (*PFuncOnTouchGesture)(const TouchGesture & tg,void * call_object);

    //    static void DefaultOnTG(const TouchGesture & tg,void * call_object); // just show the gesture

    //    static void OnTG_TouchStart(const TouchGesture & tg,void * call_object);
    //    static void OnTG_Down(const TouchGesture & tg,void * call_object);
    //    static void OnTG_Move(const TouchGesture & tg,void * call_object);
    //    static void OnTG_Up(const TouchGesture & tg,void * call_object);

//    //
//    static void OnTG_SecondDown(const TouchGesture & tg,void * call_object);
//    static void OnTG_SecondUp(const TouchGesture & tg,void * call_object);

//    //
//    static void OnTG_SplitStart(const TouchGesture & tg,void * call_object);
//    static void OnTG_SplitApart(const TouchGesture & tg,void * call_object);
//    static void OnTG_SplitClose(const TouchGesture & tg,void * call_object);
//    static void OnTG_SplitEnd(const TouchGesture & tg,void * call_object);

// OnTG_TouchEnd: to clear what need to clear;
//    static void OnTG_TouchEnd(const TouchGesture & tg,void * call_object);

private:
    PFuncOnTouchGesture m_pf_on_tges[TG_TOUCH_END + 1];
};

#endif // AVPQREADER_H
