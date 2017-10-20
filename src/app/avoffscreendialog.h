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

#ifndef AVOFFSCREENDIALOG_H
#define AVOFFSCREENDIALOG_H

#include <QDialog>

namespace Ui {
class AVOffscreenDialog;
}

class AVOffscreenDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AVOffscreenDialog(int glWidgetWidth, int glWidgetHeight, QWidget *parent = 0);
    void getOptions(int* hSize, int* vSize, int* quality);
    ~AVOffscreenDialog();

private slots:
    void onLineEditHorizonalTextEdited(const QString &text);
    void onLineEditVerticalTextEdited(const QString &text);

    void onPushButtonLowPressed();
    void onPushButtonMediumPressed();
    void onPushButtonHighPressed();

private:
    Ui::AVOffscreenDialog *ui;
    int m_glWidgetWidth;
    int m_glWidgetHeight;

    void setQualityPreset(int preset);
};

#endif // AVOFFSCREENDIALOG_H
