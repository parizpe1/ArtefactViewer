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

#include "avlight.h"

AVLight::AVLight()
{
    m_isOn = false;
    m_intensity = 0;
    m_hRotation = 0.0;
    m_vRotation = 0.0;
    m_distanceToOrigin = 10.0;

    calculateTransformation();
}

AVLight::AVLight(bool isOn, int intensity, float hRot, float vRot, float distToOrigin)
    : m_isOn(isOn),
      m_intensity(intensity),
      m_hRotation(hRot),
      m_vRotation(vRot),
      m_distanceToOrigin(distToOrigin)
{
    calculateTransformation();
}

bool AVLight::getIsOn() const
{
    return m_isOn;
}
void AVLight::setIsOn(bool value)
{
    m_isOn = value;
}

int AVLight::getIntensity() const
{
    return m_intensity;
}
void AVLight::setIntensity(int value)
{
    m_intensity = value;
}

float AVLight::getVRotation() const
{
    return m_vRotation;
}
void AVLight::setVRotation(float value)
{
    m_vRotation = value;
    calculateTransformation();
}

float AVLight::getHRotation() const
{
    return m_hRotation;
}
void AVLight::setHRotation(float value)
{
    m_hRotation = value;
    calculateTransformation();
}

float AVLight::getDistanceToOrigin() const
{
    return m_distanceToOrigin;
}
void AVLight::setDistanceToOrigin(float value)
{
    m_distanceToOrigin = value;
    calculateTransformation();
}

QVector3D AVLight::getPosition() const
{
    return m_position;
}
QMatrix4x4 AVLight::getTransformation() const
{
    return m_transformation;
}

void AVLight::calculateTransformation()
{
    m_transformation.setToIdentity();
    m_transformation.rotate(m_hRotation, 0, 1, 0);
    m_transformation.rotate(m_vRotation, 1, 0, 0);
    m_transformation.translate(QVector3D(0, 0, m_distanceToOrigin));
    m_transformation.scale(5);
    m_transformation.rotate(90, 1, 0, 0);
    m_position = m_transformation * QVector3D(0, 0, 0);
}








