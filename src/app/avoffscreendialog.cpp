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

#include "avoffscreendialog.h"
#include "ui_avoffscreendialog.h"

#include "QDebug"

enum presetEnum {LOW = 0, MEDIUM = 1, HIGH = 2};

AVOffscreenDialog::AVOffscreenDialog(int glWidgetWidth, int glWidgetheight, QWidget *parent) :
    m_glWidgetWidth(glWidgetWidth),
    m_glWidgetHeight(glWidgetheight),
    QDialog(parent),
    ui(new Ui::AVOffscreenDialog)
{
    ui->setupUi(this);

    /*
    QValidator *validator = new QIntValidator(1,10000,this);
    ui->lineEditHorizontalSize->setValidator(validator);
    ui->lineEditVerticalSize->setValidator(validator);
    */

    QObject::connect(ui->lineEditHorizontalSize, &QLineEdit::textEdited, this, &AVOffscreenDialog::onLineEditHorizonalTextEdited);
    QObject::connect(ui->lineEditVerticalSize,   &QLineEdit::textEdited, this, &AVOffscreenDialog::onLineEditVerticalTextEdited);

    QObject::connect(ui->pushButtonLow,    &QPushButton::clicked, this, &AVOffscreenDialog::onPushButtonLowPressed);
    QObject::connect(ui->pushButtonMedium, &QPushButton::clicked, this, &AVOffscreenDialog::onPushButtonMediumPressed);
    QObject::connect(ui->pushButtonHigh,   &QPushButton::clicked, this, &AVOffscreenDialog::onPushButtonHighPressed);

    setQualityPreset(MEDIUM);
}

void AVOffscreenDialog::getOptions(int *hSize, int *vSize, int *quality)
{
    *hSize = ui->lineEditHorizontalSize->text().toInt();
    *vSize = ui->lineEditVerticalSize->text().toInt();
    *quality = ui->spinBoxQuality->value();
}

AVOffscreenDialog::~AVOffscreenDialog()
{
    delete ui;
}

void AVOffscreenDialog::onLineEditHorizonalTextEdited(const QString &text)
{
    //qDebug() << "onLineEditHorizonalTextEdited: " << text;

    float width = text.toFloat();
    float height = width / (float)m_glWidgetWidth * (float)m_glWidgetHeight;
    ui->lineEditVerticalSize->setText(QString::number((int)height));
}

void AVOffscreenDialog::onLineEditVerticalTextEdited(const QString &text)
{
    //qDebug() << "onLineEditVerticalTextEdited: " << text;

    float height = text.toFloat();
    float width = height / (float)m_glWidgetHeight * (float)m_glWidgetWidth;
    ui->lineEditHorizontalSize->setText(QString::number((int)width));
}

void AVOffscreenDialog::onPushButtonLowPressed()
{
    setQualityPreset(LOW);
}

void AVOffscreenDialog::onPushButtonMediumPressed()
{
    setQualityPreset(MEDIUM);
}

void AVOffscreenDialog::onPushButtonHighPressed()
{
    setQualityPreset(HIGH);
}

void AVOffscreenDialog::setQualityPreset(int preset)
{
    int size = 0;
    int quality = 0;

    switch (preset)
    {
        case LOW :
            size = 2048;
            quality = 60;
            break;
        case MEDIUM :
            size = 4096;
            quality = 80;
            break;
        case HIGH :
            size = 8192;
            quality = 100;
            break;
        default:
            size = 2048;
            quality = 60;
            break;
    }

    if( m_glWidgetWidth >= m_glWidgetHeight)
    {
        ui->lineEditHorizontalSize->setText(QString::number(size));
        ui->lineEditVerticalSize->setText(QString::number((int)((float)size / (float)m_glWidgetWidth * (float)m_glWidgetHeight)));
        ui->spinBoxQuality->setValue(quality);
    }
    else
    {
        ui->lineEditVerticalSize->setText(QString::number(size));
        ui->lineEditHorizontalSize->setText(QString::number((int)((float)size / (float)m_glWidgetHeight * (float)m_glWidgetWidth)));
        ui->spinBoxQuality->setValue(quality);
    }

}
