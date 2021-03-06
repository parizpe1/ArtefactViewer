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

#include "avglwidget.h"

#include <QtGui>
#include <QtOpenGL>
#include <QDebug>
#include <QDesktopWidget>
#include <QWheelEvent>
#include <QStatusBar>
#include <QToolBar>
#define PI 3.14159265
#include "math.h"
#include "AVKinector.h"
#include "avmodel.h"
#include "avmainwindow.h"
#include "avtrackball.h"
#include "avpqreader.h"
#include "avtouchpoint.h"
#include "avpointframe.h"
#include "iostream"
#include "kinect.h"

using namespace std;
int lesstext=0;

AVGLWidget::AVGLWidget(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    m_model = AVModel::instance();
    m_pMatrix.setToIdentity();
    m_model->setupLightGeometry();
    AVStatus=AVMainWindow::instance()->statusBar();
    AVStatus->showMessage(
                "To start the Kinect, hold both hands open,facing the Kinect camera and away from your body.",0);
    QGLFormat stereoFormat;
    stereoFormat.setSampleBuffers(true);
    stereoFormat.setStereo(true);
    this->setFormat(stereoFormat);
    this->setCursor(Qt::BlankCursor);
    initialize();
    setAutoFillBackground(false);
    setAutoBufferSwap(false);
    m_trackball = new AVTrackBall(AVTrackBall::Plane);
    RotScaling=false;
    rotating=false;
    zooming=false;
    moving=false;
    spinning=false;
    tIdle=false;
    m_kLTr=false;
    m_kRTr=false;
    m_kRot=false;
    m_kScale=false;
    lRotInit=false;
    rRotInit=false;
    lScaleInit=false;
    rScaleInit=false;
    m_kLUC=0;
    m_kLNTC=0;
    m_kRUC=0;
    m_kRNTC=0;
    m_status="";
    resetRightKinectCounts();
    resetLeftKinectCounts();
    m_kinectIsWatching=false;
}

AVGLWidget::~AVGLWidget()
{
    delete m_trackball;
}

//! Initializes the AVGLWidget so the user gets a standard view of the artefact in an opened file
/*! On application start and in case a new file is opened several variables need
 * to be (re-)initialised with reasonable default values. This function handles the
 * part of the 3D viewport and corresponds to the initialization function of the mainwindow
 * that handles the (re-)initialization of the user interface.
 * Since 2016 modifications, this also manages the stereographic visualization.
 */

void AVGLWidget::initialize()
{
    QThread::currentThread()->setPriority(QThread::HighestPriority);
    m_MatrixLights.setToIdentity();
    m_vMatrix.setToIdentity();
    m_vMatrixCurrent.setToIdentity();
    m_leftVMatrix.setToIdentity();
    m_rightVMatrix.setToIdentity();
    m_camRotateMatrix.setToIdentity();
    m_MatrixArtefact.setToIdentity();
    m_lighting = true;
    m_lightsAreVisible = true;
    m_vertexColors = false;
    m_enhancement = 0;
    //strength                  //threshold                //smooth
    m_enhance_param[0][0] =  0; m_enhance_param[0][1] =  0; m_enhance_param[0][2] = 0;//no enhancement
    m_enhance_param[1][0] = 10; m_enhance_param[1][1] = 25; m_enhance_param[1][2] = 0;//dark edges
    m_enhance_param[2][0] = 10; m_enhance_param[2][1] = 25; m_enhance_param[2][2] = 0;//bright edges
    m_enhance_param[3][0] = 10; m_enhance_param[3][1] =  0; m_enhance_param[3][2] = 0;//dark valleys
    m_paintAnnotations = false;
    //change to header as constants
    m_camDistanceToOrigin = 200.0;
    m_eyeSeparation = 10.0;
    m_camOrigin = QVector3D(0,0,0);
    m_backgroundColor1 = QVector3D(0.0,0.0,0.0);
    m_backgroundColor2 = QVector3D(0.0,0.0,0.0);
    m_currentAnnotation = -1;
    m_shiftDown = false;
    m_draggedPoint = -1;
    m_selectedPoint = -1;
    m_model->m_flatColor = QVector3D(1.0,0.0,0.0);
    m_lights.clear();
    // Light 1
    AVLight light1(true, 200, 330, -25, 100);
    m_lights.append(light1);
    // Light 2
    AVLight light2(true, 200, 150, 25, 100);
    m_lights.append(light2);
    // Light 3
    AVLight light3(false, 200, 60, 0, 100);
    m_lights.append(light3);
    // Light 4
    AVLight light4(false, 200, 240, 0, 100);
    m_lights.append(light4);
    emit glWidgetInitialised();
}

QMatrix4x4 AVGLWidget::getMatrixArtefact() const
{
    return m_MatrixArtefact;
}

void AVGLWidget::setMatrixArtefact(const QMatrix4x4 &MatrixArtefact)
{
    m_MatrixArtefact = MatrixArtefact;
}

QMatrix4x4 AVGLWidget::getLeftVMatrix() const
{
    return m_leftVMatrix;
}

QMatrix4x4 AVGLWidget::getRightVMatrix() const
{
    return m_rightVMatrix;
}

QMatrix4x4 AVGLWidget::getViewMatrix() const
{
    return m_vMatrix;
}
QMatrix4x4 AVGLWidget::getVRMatrix() const
{
    return m_vMatrix;
}

void AVGLWidget::setVRMatrix(const QMatrix4x4 &vMatrix)
{
    m_camRotateMatrix = QMatrix4x4(vMatrix.normalMatrix()).inverted();
}

void AVGLWidget::resetVMatrix()
{
    m_camRotateMatrix.setToIdentity();
    m_camDistanceToOrigin = 200.0;
    m_camOrigin=QVector3D(0,0,0);
    updateGL();
}

void AVGLWidget::resetMatrixArtefact()
{
    m_MatrixArtefact.setToIdentity();
    m_MatrixArtefact.translate(-m_model->m_centerPoint);
    updateGL();
}
/*! 
 * Since 2016 modifications, this manages the quad buffer system * 
 * 
 */
QImage AVGLWidget::renderToOffscreenBuffer(int width, int height)
{
    qDebug() << "renderToOffscreenBuffer: " << width << " x " << height << "  max: " << GL_MAX_TEXTURE_SIZE;
    QGLFramebufferObjectFormat fboFormat;
    fboFormat.setAttachment(QGLFramebufferObject::Depth);
    fboFormat.setSamples(4);
    int numSamples = fboFormat.samples();
    qDebug() << "renderToOffscreenBuffer:: FBO has actually " << numSamples << " samples.";
    QGLFramebufferObject fbo(width, height, fboFormat);

    m_pMatrix.setToIdentity();
    m_pMatrix.frustum(-(float) width /2,(float) width /2, -(float) height/2,(float) height/2, 0.001f, 100000.0f);
    glViewport(0, 0, fbo.size().width(), fbo.size().height());


    fbo.bind();
    // Select back left buffer
    glDrawBuffer(GL_BACK_LEFT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawBackground();
    m_vMatrixCurrent=m_leftVMatrix;
    setupCamera(true);
    if (m_lighting) // Shading with lighting
    {
        drawModelShaded(m_leftVMatrix);

        if (m_lightsAreVisible) //Draw light representations
        {
            drawLights(m_leftVMatrix);
        }
    }
    else // Lighting off
    {
        drawModelNoShading(m_leftVMatrix);
    }
    drawOverlays(&fbo, true, width);
    fbo.release();

    //bind again
    fbo.bind();
    // Select back right buffer
    glDrawBuffer(GL_BACK_RIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawBackground();
    m_vMatrixCurrent=m_rightVMatrix;
    setupCamera(false);
    if (m_lighting) // Shading with lighting
    {
        drawModelShaded(m_rightVMatrix);

        if (m_lightsAreVisible) //Draw light representations
        {
            drawLights(m_rightVMatrix);
        }
    }
    else // Lighting off
    {
        drawModelNoShading(m_rightVMatrix);
    }
    drawOverlays(&fbo, true, width);


    fbo.release();
    resizeGL(this->width(), this->height());
    updateGL();

    return fbo.toImage();
}

QVector3D AVGLWidget::getBackgroundColor2() const
{
    return m_backgroundColor2;
}
void AVGLWidget::setBackgroundColor2(const QVector3D &backgroundColor2)
{
    m_backgroundColor2 = backgroundColor2;
}

QVector3D AVGLWidget::getBackgroundColor1() const
{
    return m_backgroundColor1;
}
void AVGLWidget::setBackgroundColor1(const QVector3D &backgroundColor1)
{
    m_backgroundColor1 = backgroundColor1;
}

double AVGLWidget::getCamDistanceToOrigin() const
{
    return m_camDistanceToOrigin;
}
void AVGLWidget::setCamDistanceToOrigin(double camDistanceToOrigin)
{
    m_camDistanceToOrigin = camDistanceToOrigin;
}

QPoint AVGLWidget::getLastMousePosition() const
{
    return m_lastMousePosition;
}
void AVGLWidget::setLastMousePosition(QPoint lastMousePosition)
{
    m_lastMousePosition = lastMousePosition;
}

QVector3D AVGLWidget::getCamOrigin() const
{
    return m_camOrigin;
}
void AVGLWidget::setCamOrigin(const QVector3D &camOrigin)
{
    m_camOrigin = camOrigin;
}

bool AVGLWidget::getPaintAnnotations() const
{
    return m_paintAnnotations;
}
void AVGLWidget::setPaintAnnotations(bool paintAnnotations)
{
    m_paintAnnotations = paintAnnotations;
}

bool AVGLWidget::getUseVertexColors() const
{
    return m_vertexColors;
}
void AVGLWidget::setVertexColors(bool vertexColors)
{
    m_vertexColors = vertexColors;
}

bool AVGLWidget::getUseLighting() const
{
    return m_lighting;
}
void AVGLWidget::setLighting(bool lighting)
{
    m_lighting = lighting;
}

bool AVGLWidget::getLightsAreVisible() const
{
    return m_lightsAreVisible;
}
void AVGLWidget::setLightsAreVisible(bool lightsAreVisible)
{
    m_lightsAreVisible = lightsAreVisible;
}

bool AVGLWidget::isShiftDown() const
{
    return m_shiftDown;
}
void AVGLWidget::setShiftDown(bool shiftDown)
{
    m_shiftDown = shiftDown;
}

int AVGLWidget::getEnhancementParam(int i, int j) const
{
    if(i > 3 || j > 2) return -1;
    return m_enhance_param[i][j];
}
void AVGLWidget::setEnhancementParam(int i, int j, int value)
{
    if(i > 3 || j > 2) return;
    m_enhance_param[i][j] = value;
}

int AVGLWidget::getEnhancement() const
{
    return m_enhancement;
}

void AVGLWidget::setEnhancement(int enhancement)
{
    m_enhancement = enhancement;
}

int AVGLWidget::getCurrentAnnotation() const
{
    return m_currentAnnotation;
}
void AVGLWidget::setCurrentAnnotation(int currentAnnotation)
{
    m_currentAnnotation = currentAnnotation;
}

int AVGLWidget::getDraggedPoint() const
{
    return m_draggedPoint;
}
void AVGLWidget::setDraggedPoint(int draggedPoint)
{
    m_draggedPoint = draggedPoint;
}

int AVGLWidget::getSelectedPoint() const
{
    return m_selectedPoint;
}
void AVGLWidget::setSelectedPoint(int selectedPoint)
{
    m_selectedPoint = selectedPoint;
}

//! Sets the zoom level. Can be given as a relative value (currently unused)
void AVGLWidget::setZoomLevel(double level, bool relative)
{
    if (relative) m_camDistanceToOrigin *= level;
    else m_camDistanceToOrigin = level; //absolute (0) or relative (1) zoom
    updateGL();
}

//! Gives a hint about the minimum size
QSize AVGLWidget::minimumSizeHint() const
{
    return QSize(200, 200);
}

//! Gives a hint about the maximum sizes and causes maximization in most cases
QSize AVGLWidget::sizeHint() const

{
    return QSize(2000, 2000);
}

//! Does some initialization for openGL
/*! Three shaders are prepared for usage:
 * 1) A phong shader for four light sources that needs (varying) vertex colors, used for the artefact
 * 2) A shader that uses varying colors but not lighting, used for the artefact and the lights
 * 3) A shader for uniform colors and no lighting, used for some overlay objects
 */
void AVGLWidget::initializeGL()
{
    initializeGLFunctions();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    m_lightingShaderP.addShaderFromSourceFile(QGLShader::Vertex, ":/lightingVertexShader.vsh");
    m_lightingShaderP.addShaderFromSourceFile(QGLShader::Fragment, ":/lightingFragmentShader.fsh");
    m_lightingShaderP.link();
    m_coloringShaderP.addShaderFromSourceFile(QGLShader::Vertex, ":/coloringVertexShader.vsh");
    m_coloringShaderP.addShaderFromSourceFile(QGLShader::Fragment, ":/coloringFragmentShader.fsh");
    m_coloringShaderP.link();
    m_singleColorShaderP.addShaderFromSourceFile(QGLShader::Vertex, ":/singleColorVertexShader.vsh");
    m_singleColorShaderP.addShaderFromSourceFile(QGLShader::Fragment, ":/singleColorFragmentShader.fsh");
    m_singleColorShaderP.link();
    fillBuffers();
}

//! Fills the buffers of the graphics hardware
/*! All the vertices, colors, normals and triangle informations for the artefact
 * along with the vertices and on/off colors for the lights are uploaded to the
 * graphics hardware. For the artefact colors there is an additional check for
 * flat coloring. In case that flag is set, the whole buffer is filled with a
 * single color.
 */
void AVGLWidget::fillBuffers()
{
    qDebug() << "AVGLWidget::fillBuffers";

    if (m_vertexBuffer.isCreated()) m_vertexBuffer.destroy();
    m_vertexBuffer.create();
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(m_model->m_vertices.size() * 3 * sizeof(GLfloat));
    m_vertexBuffer.write(0, m_model->m_vertices.constData(), m_model->m_vertices.size() * 3 * sizeof(GLfloat));
    m_vertexBuffer.release();

    if (m_colorBuffer.isCreated()) m_colorBuffer.destroy();
    m_colorBuffer.create();
    m_colorBuffer.bind();
    m_colorBuffer.allocate(m_model->m_colors.size() * 3 * sizeof(GLfloat));
    for (int i=0; i < m_model->m_colors.size(); i++)
    {
        QVector3D myColor(0,0,0);
        if (m_vertexColors)   myColor = m_model->m_colors[i] * m_model->m_shadowMap[i];
        else                myColor = m_model->m_flatColor * m_model->m_shadowMap[i];
        m_colorBuffer.write(i*12, &myColor, 3 * sizeof(GLfloat));
    }
    m_colorBuffer.release();

    if (m_normalBuffer.isCreated()) m_normalBuffer.destroy();
    m_normalBuffer.create();
    m_normalBuffer.bind();
    m_normalBuffer.allocate(m_model->m_normals.size() * 3 * sizeof(GLfloat));
    m_normalBuffer.write(0, m_model->m_normals.constData(), m_model->m_normals.size() * 3 * sizeof(GLfloat));
    m_normalBuffer.release();

    if (m_indexBuffer.isCreated()) m_indexBuffer.destroy();
    m_indexBuffer = QGLBuffer(QGLBuffer::IndexBuffer);
    m_indexBuffer.create();
    m_indexBuffer.bind();
    m_indexBuffer.allocate(m_model->m_triangles.size() * sizeof (GLint));
    m_indexBuffer.write(0, m_model->m_triangles.constData(), m_model->m_triangles.size() * sizeof(GLint));
    m_indexBuffer.release();

    if (m_lightVertexBuffer.isCreated()) m_lightVertexBuffer.destroy();
    m_lightVertexBuffer.create();
    m_lightVertexBuffer.bind();
    m_lightVertexBuffer.allocate(m_model->m_lightVertices.size() * 3 * sizeof(GLfloat));
    m_lightVertexBuffer.write(0, m_model->m_lightVertices.constData(), m_model->m_lightVertices.size() * 3 * sizeof(GLfloat));
    m_lightVertexBuffer.release();

    if (m_lightOnColorBuffer.isCreated()) m_lightOnColorBuffer.destroy();
    m_lightOnColorBuffer.create();
    m_lightOnColorBuffer.bind();
    m_lightOnColorBuffer.allocate(m_model->m_lightOnColors.size() * 3 * sizeof(GLfloat));
    m_lightOnColorBuffer.write(0, m_model->m_lightOnColors.constData(), m_model->m_lightOnColors.size() * 3 * sizeof(GLfloat));
    m_lightOnColorBuffer.release();

    if (m_lightOffColorBuffer.isCreated()) m_lightOffColorBuffer.destroy();
    m_lightOffColorBuffer.create();
    m_lightOffColorBuffer.bind();
    m_lightOffColorBuffer.allocate(m_model->m_lightOffColors.size() * 3 * sizeof(GLfloat));
    m_lightOffColorBuffer.write(0, m_model->m_lightOffColors.constData(), m_model->m_lightOffColors.size() * 3 * sizeof(GLfloat));
    m_lightOffColorBuffer.release();

    if (m_lightCircleVertexBuffer.isCreated()) m_lightCircleVertexBuffer.destroy();
    m_lightCircleVertexBuffer.create();
    m_lightCircleVertexBuffer.bind();
    m_lightCircleVertexBuffer.allocate(m_model->m_lightCircleVertices.size() * 3 * sizeof(GLfloat));
    m_lightCircleVertexBuffer.write(0,m_model->m_lightCircleVertices.constData(), m_model->m_lightCircleVertices.size() * 3 * sizeof(GLfloat));
    m_lightCircleVertexBuffer.release();

    if( m_lightCircleColorBuffer.isCreated()) m_lightCircleColorBuffer.destroy();
    m_lightCircleColorBuffer.create();
    m_lightCircleColorBuffer.bind();
    m_lightCircleColorBuffer.allocate(m_model->m_lightCircleColors.size() * 3 * sizeof(GLfloat));
    m_lightCircleColorBuffer.write(0,m_model->m_lightCircleColors.constData(), m_model->m_lightCircleColors.size() * 3 * sizeof(GLfloat));
    m_lightCircleColorBuffer.release();
}


//! Does all the painting inside the AVGLWidget
/*! paintGL is called after every glUpdate call and handles all the drawing
 * inside the AVGLWidget, which includes
 * 1) painting the background in the colors chosen by the user
 * 2) painting the projection of the artefact with or without vertex colors and lighting
 * 3) painting the representations of the light sources according to their on/off status
 * 4) painting the annotation texts and the lines pointing to the artefact as overlay
 * 
 * Since 2016 modifications, this has to be done twice, once for the left eye and once
 * for the right eye to achieve stereography
 * 
 * 
 */
void AVGLWidget::paintGL()
{
    // Select back left buffer
    glDrawBuffer(GL_BACK_LEFT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //    glEnable(GL_FASTEST);

    // Draw Background
    drawBackground();

    // Set up the camera/view left
    setupCamera(true);

    glEnable(GL_DEPTH_TEST);
    if (m_lighting) // Shading with lighting
    {
        drawModelShaded(m_leftVMatrix);

        if (m_lightsAreVisible) //Draw light representations
        {
            drawLights(m_leftVMatrix);
        }
    }
    else // Lighting off
    {
        drawModelNoShading(m_leftVMatrix);
    }

    // Paint overlay objects
    drawOverlays(this);



    // Select back right buffer
    glDrawBuffer(GL_BACK_RIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_FASTEST);

    // Draw Background
    drawBackground();

    // Set up the camera/view
    setupCamera(false);

    glEnable(GL_DEPTH_TEST);
    if (m_lighting) // Shading with lighting
    {
        drawModelShaded(m_rightVMatrix);

        if (m_lightsAreVisible) //Draw light representations
        {
            drawLights(m_rightVMatrix);
        }
    }
    else // Lighting off
    {
        drawModelNoShading(m_rightVMatrix);
    }

    // Paint overlay objects
    drawOverlays(this);


    swapBuffers();
}



//! Is called when the widget's size (and thus the window's size) changes

void AVGLWidget::resizeGL(int width, int height)
{
    if (height == 0) {
        height = 1;
    }
    m_pMatrix.setToIdentity();
    m_pMatrix.perspective(60.0f, (float) width / (float) height, 0.001f, 100000.0f);
    glViewport(0, 0, width, height);
}


//! Handles mouse wheel events
/*! wheelEvent is called when the mouse's wheel is rotated either up or down
*  and adjusts the distance of the camera to the camera origin
*/
void AVGLWidget::wheelEvent(QWheelEvent *event)
{
    if (event->delta() > 0) setZoomLevel(0.909090909, true);
    else setZoomLevel(1.1, true);
}

void AVGLWidget::drawBackground()
{
    glDisable(GL_DEPTH_TEST);
    QVector<QVector2D> backgroundVertices;
    backgroundVertices.append(QVector2D(-1.0,-1.0));
    backgroundVertices.append(QVector2D(1.0,-1.0));
    backgroundVertices.append(QVector2D(1.0,1.0));
    backgroundVertices.append(QVector2D(-1.0,1.0));
    QVector<QVector3D> backgroundColors;
    backgroundColors.append(m_backgroundColor1);
    backgroundColors.append(m_backgroundColor1);
    backgroundColors.append(m_backgroundColor2);
    backgroundColors.append(m_backgroundColor2);
    QMatrix4x4 mMatrix; mMatrix.setToIdentity();
    QMatrix4x4 vMat; vMat.setToIdentity();
    QMatrix4x4 pMat; pMat.setToIdentity();
    m_coloringShaderP.bind();
    m_coloringShaderP.setUniformValue("mvpMatrix", pMat * vMat * mMatrix);
    m_coloringShaderP.setAttributeArray("vertex", backgroundVertices.constData());
    m_coloringShaderP.enableAttributeArray("vertex");
    m_coloringShaderP.setAttributeArray("color", backgroundColors.constData());
    m_coloringShaderP.enableAttributeArray("color");
    glDrawArrays(GL_QUADS, 0, 4);
    m_coloringShaderP.disableAttributeArray("vertex");
    m_coloringShaderP.disableAttributeArray("color");
    m_coloringShaderP.release();
    glEnable(GL_DEPTH_TEST);
}
/*
 * 2016 modifications: set one camera for the right eye and one of the left one
 */
void AVGLWidget::setupCamera(boolean eyeIsLeft)
{
    m_camPosition = m_camRotateMatrix.mapVector(QVector3D(0, 0, m_camDistanceToOrigin));
    m_camPosition +=m_camOrigin;
    //        calculate right and up direction with cross product
    m_camUpDirection=m_camRotateMatrix.mapVector(QVector3D(0,1,0));
    m_camRightDirection=
            m_camRotateMatrix.mapVector
            (QVector3D(1,0,0));
    if(eyeIsLeft)
    {
        m_LeftCamPosition=m_camPosition-(m_eyeSeparation/2)*m_camRightDirection;

        m_leftVMatrix.setToIdentity();
        m_leftVMatrix.lookAt(m_LeftCamPosition, m_camOrigin, m_camUpDirection.normalized());
    }
    else{
        m_RightCamPosition=m_camPosition+(m_eyeSeparation/2)*m_camRightDirection;
        m_rightVMatrix.setToIdentity();
        m_rightVMatrix.lookAt(m_RightCamPosition, m_camOrigin, m_camUpDirection.normalized());
    }
    m_vMatrix.setToIdentity();
    m_vMatrix.lookAt(m_camPosition, m_camOrigin, m_camUpDirection.normalized());

}

void AVGLWidget::drawModelShaded(QMatrix4x4 l_vMatrix)
{
    m_lightingShaderP.bind();
    m_lightingShaderP.setUniformValue("mvpMatrix", m_pMatrix * l_vMatrix * m_MatrixArtefact);
    m_lightingShaderP.setUniformValue("mvMatrix", l_vMatrix * m_MatrixArtefact);
    m_lightingShaderP.setUniformValue("normalMatrix", (l_vMatrix * m_MatrixArtefact).normalMatrix());
    m_lightingShaderP.setUniformValue("light0Position", l_vMatrix * m_lights[0].getPosition());
    m_lightingShaderP.setUniformValue("light1Position", l_vMatrix * m_lights[1].getPosition());
    m_lightingShaderP.setUniformValue("light2Position", l_vMatrix * m_lights[2].getPosition());
    m_lightingShaderP.setUniformValue("light3Position", l_vMatrix * m_lights[3].getPosition());
    m_lightingShaderP.setUniformValue("ambientColor", QColor(30, 30, 30));
    m_lightingShaderP.setUniformValue("diffuse0Color", QColor(m_lights[0].getIsOn()?m_lights[0].getIntensity():0, m_lights[0].getIsOn()?m_lights[0].getIntensity():0, m_lights[0].getIsOn()?m_lights[0].getIntensity():0));
    m_lightingShaderP.setUniformValue("specular0Color", QColor(m_lights[0].getIsOn()?m_lights[0].getIntensity():0, m_lights[0].getIsOn()?m_lights[0].getIntensity():0, m_lights[0].getIsOn()?m_lights[0].getIntensity():0));
    m_lightingShaderP.setUniformValue("diffuse1Color", QColor(m_lights[1].getIsOn()?m_lights[1].getIntensity():0, m_lights[1].getIsOn()?m_lights[1].getIntensity():0, m_lights[1].getIsOn()?m_lights[1].getIntensity():0));
    m_lightingShaderP.setUniformValue("specular1Color", QColor(m_lights[1].getIsOn()?m_lights[1].getIntensity():0, m_lights[1].getIsOn()?m_lights[1].getIntensity():0, m_lights[1].getIsOn()?m_lights[1].getIntensity():0));
    m_lightingShaderP.setUniformValue("diffuse2Color", QColor(m_lights[2].getIsOn()?m_lights[2].getIntensity():0, m_lights[2].getIsOn()?m_lights[2].getIntensity():0, m_lights[2].getIsOn()?m_lights[2].getIntensity():0));
    m_lightingShaderP.setUniformValue("specular2Color", QColor(m_lights[2].getIsOn()?m_lights[2].getIntensity():0, m_lights[2].getIsOn()?m_lights[2].getIntensity():0, m_lights[2].getIsOn()?m_lights[2].getIntensity():0));
    m_lightingShaderP.setUniformValue("diffuse3Color", QColor(m_lights[3].getIsOn()?m_lights[3].getIntensity():0, m_lights[3].getIsOn()?m_lights[3].getIntensity():0, m_lights[3].getIsOn()?m_lights[3].getIntensity():0));
    m_lightingShaderP.setUniformValue("specular3Color", QColor(m_lights[3].getIsOn()?m_lights[3].getIntensity():0, m_lights[3].getIsOn()?m_lights[3].getIntensity():0, m_lights[3].getIsOn()?m_lights[3].getIntensity():0));
    m_lightingShaderP.setUniformValue("ambientReflection", (GLfloat) 1.0);
    m_lightingShaderP.setUniformValue("diffuseReflection", (GLfloat) 1.0);
    m_lightingShaderP.setUniformValue("specularReflection", (GLfloat) 0.0);
    m_lightingShaderP.setUniformValue("shininess", (GLfloat) 2000.0);
    // Use the buffer for vertices, colors and normals
    m_vertexBuffer.bind();
    m_lightingShaderP.setAttributeBuffer("vertex", GL_FLOAT, 0, 3, 0);
    m_lightingShaderP.enableAttributeArray("vertex");
    m_vertexBuffer.release();
    m_colorBuffer.bind();
    m_lightingShaderP.setAttributeBuffer("color", GL_FLOAT, 0, 3, 0);
    m_lightingShaderP.enableAttributeArray("color");
    m_colorBuffer.release();
    m_normalBuffer.bind();
    m_lightingShaderP.setAttributeBuffer("normal", GL_FLOAT, 0, 3, 0);
    m_lightingShaderP.enableAttributeArray("normal");
    m_normalBuffer.release();
    m_indexBuffer.bind();
    glDrawElements(GL_TRIANGLES, m_model->m_triangles.size(), GL_UNSIGNED_INT, 0);
    m_indexBuffer.release();
    m_lightingShaderP.disableAttributeArray("normal");
    m_lightingShaderP.disableAttributeArray("color");
    m_lightingShaderP.disableAttributeArray("vertex");
    m_lightingShaderP.release();
}

void AVGLWidget::drawModelNoShading(QMatrix4x4 l_vMatrix)
{
    // Shading without lighting
    m_coloringShaderP.bind();
    m_coloringShaderP.setUniformValue("mvpMatrix", m_pMatrix * l_vMatrix * m_MatrixArtefact);
    m_vertexBuffer.bind();
    m_coloringShaderP.setAttributeBuffer("vertex", GL_FLOAT, 0, 3, 0);
    m_coloringShaderP.enableAttributeArray("vertex");
    m_vertexBuffer.release();
    m_colorBuffer.bind();
    m_coloringShaderP.setAttributeBuffer("color", GL_FLOAT, 0, 3, 0);
    m_coloringShaderP.enableAttributeArray("color");
    m_colorBuffer.release();
    m_indexBuffer.bind();
    glDrawElements(GL_TRIANGLES, m_model->m_triangles.size(), GL_UNSIGNED_INT, 0);
    m_indexBuffer.release();
    m_coloringShaderP.disableAttributeArray("color");
    m_coloringShaderP.disableAttributeArray("vertex");
    m_coloringShaderP.release();
}

void AVGLWidget::drawLights(QMatrix4x4 l_vMatrix)
{
    for (int i=0; i<m_lights.size(); i++)
    {
        m_MatrixLights.setToIdentity();
        m_MatrixLights = m_lights[i].getTransformation();
        m_coloringShaderP.bind();
        m_coloringShaderP.setUniformValue("mvpMatrix", m_pMatrix * l_vMatrix * m_MatrixLights);
        m_lightVertexBuffer.bind();
        m_coloringShaderP.setAttributeBuffer("vertex", GL_FLOAT, 0, 3, 0);
        m_coloringShaderP.enableAttributeArray("vertex");
        m_lightVertexBuffer.release();
        if (m_lights[i].getIsOn()) m_lightOnColorBuffer.bind();
        else m_lightOffColorBuffer.bind();
        m_coloringShaderP.setAttributeBuffer("color", GL_FLOAT, 0, 3, 0);
        m_coloringShaderP.enableAttributeArray("color");
        if (m_lights[i].getIsOn()) m_lightOnColorBuffer.release();
        else m_lightOffColorBuffer.release();
        glDrawArrays(GL_TRIANGLES, 0, m_model->m_lightVertices.size());
        m_coloringShaderP.disableAttributeArray("vertex");
        m_coloringShaderP.disableAttributeArray("color");
        m_coloringShaderP.release();
    }
    drawLightCircles(l_vMatrix);
}

void AVGLWidget::drawLightCircles(QMatrix4x4 l_vMatrix)
{
    //Draw circles around active light

    m_coloringShaderP.bind();

    m_lightCircleVertexBuffer.bind();
    m_coloringShaderP.setAttributeBuffer("vertex", GL_FLOAT, 0, 3, 0);
    m_coloringShaderP.enableAttributeArray("vertex");
    m_lightCircleVertexBuffer.release();

    QVector<QVector3D> lightCircleColors;
    for(int i = 0; i < m_model->m_lightCircleVertices.size(); i++) lightCircleColors.append(QVector3D(0,1,0));
    m_lightCircleColorBuffer.bind();
    m_lightCircleColorBuffer.write(0,lightCircleColors.constData(),lightCircleColors.size() * 3 * sizeof(GLfloat));
    m_lightCircleColorBuffer.release();

    m_lightCircleColorBuffer.bind();
    m_coloringShaderP.setAttributeBuffer("color", GL_FLOAT, 0, 3, 0);
    m_coloringShaderP.enableAttributeArray("color");
    m_lightCircleColorBuffer.release();

    m_MatrixLightCircle.setToIdentity();
    int currentLightIndex = AVMainWindow::instance()->getCurrentLightIndex();
    float xVal = m_lights[currentLightIndex].getPosition().x();
    float yVal = m_lights[currentLightIndex].getPosition().y();
    float zVal = m_lights[currentLightIndex].getPosition().z();
    m_MatrixLightCircle.translate(QVector3D(0, yVal, 0));
    m_MatrixLightCircle.scale(QVector3D(xVal, 0, zVal).length());
    m_coloringShaderP.setUniformValue("mvpMatrix", m_pMatrix * l_vMatrix * m_MatrixLightCircle);

    glDrawArrays(GL_LINE_LOOP, 0, m_model->m_lightCircleVertices.size());


    lightCircleColors.clear();
    for(int i = 0; i < m_model->m_lightCircleVertices.size(); i++) lightCircleColors.append(QVector3D(0,0,1));
    m_lightCircleColorBuffer.bind();
    m_lightCircleColorBuffer.write(0,lightCircleColors.constData(),lightCircleColors.size() * 3 * sizeof(GLfloat));
    m_lightCircleColorBuffer.release();

    m_MatrixLightCircle.setToIdentity();
    m_MatrixLightCircle.rotate(90, 1,0,0);
    m_MatrixLightCircle.rotate(-m_lights[currentLightIndex].getHRotation() + 90, 0,0,1);
    m_MatrixLightCircle.scale(m_lights[currentLightIndex].getDistanceToOrigin());
    m_coloringShaderP.setUniformValue("mvpMatrix", m_pMatrix * l_vMatrix * m_MatrixLightCircle);

    glDrawArrays(GL_LINE_LOOP, 0, m_model->m_lightCircleVertices.size());

    m_coloringShaderP.disableAttributeArray("vertex");
    m_coloringShaderP.disableAttributeArray("color");
    m_coloringShaderP.release();
}

void AVGLWidget::drawOverlays(QPaintDevice *device, bool offscreen, int fboWidth)
{
    QPainter painter(device);
    painter.setRenderHint(QPainter::Antialiasing);
    QFont font;
    QPen pen;

    font.setBold(true);
    font.setPixelSize(25);
    pen.setColor(QColor(255,0,0,200));
    pen.setWidth(3);

    painter.setFont(font);
    painter.setPen(pen);

    float offscreenFactor = 1.0f;
    if(offscreen) offscreenFactor = (float)fboWidth / (float)this->width();

    //    // Light Numbering was removed in 2016 to avoid ghosting.
    //    if (m_lightsAreVisible && m_lighting)
    //    {
    //        for (int i = 0; i < 4; i++)
    //        {
    //            QVector3D lightPos((m_pMatrix * m_vMatrixCurrent) * (m_lights[i].getTransformation() * QVector3D(0,0,0)));
    //            QPoint p(QVector3DUnnormalizedToQPoint(lightPos));
    //            painter.drawText(QRectF(p.x()-10, p.y()-45, 30, 30), QString::number(i+1));
    //        }
    //    }


    if (m_paintAnnotations)
    {
        //Draw Annotation Text
        for (int i=0; i < m_model->m_listOfPointClouds.size(); i++)
        {
            // Text
            pen.setColor(QColor(200,200,200,255));
            painter.setPen(pen);

            if(offscreen)
            {
                font.setPixelSize((int)(offscreenFactor * 12));
                painter.setFont(font);
                painter.drawText(QRectF( (int)(offscreenFactor * 10), (int)(offscreenFactor * (10+i*135)), (int)(offscreenFactor * 120), (int)(offscreenFactor * 120) ),
                                 m_model->m_listOfPointClouds[i].text);
            }
            else
            {
                font.setPixelSize(12);
                painter.setFont(font);
                painter.drawText(QRectF(10,10+i*135,120,120),
                                 m_model->m_listOfPointClouds[i].text);
            }
        }

        //The rest is painted with native OpenGL
        painter.beginNativePainting();

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        glLineWidth(offscreenFactor * 2.0f);
        glPointSize(offscreenFactor * 8.0f);

        //Init Shader for drawing Overlays
        m_singleColorShaderP.bind();
        QMatrix4x4 identityMvpMatrix;
        identityMvpMatrix.setToIdentity();
        m_singleColorShaderP.setUniformValue("mvpMatrix",identityMvpMatrix);

        // Draw annotations
        for (int i=0; i < m_model->m_listOfPointClouds.size()-1; i++)
        {
            //Draw lines from annotation to annotated feature
            QPoint start(140,40+i*135);
            QVector3D end3D = (m_pMatrix * m_vMatrixCurrent * m_MatrixArtefact) * m_model->m_listOfPointClouds[i].points[0];

            QVector<QVector3D> points;
            points.push_back(QPointNormalizedToVector3D(start));
            points.push_back(end3D);

            QVector4D color(0.8f,0.0f,0.0f,0.5f);
            m_singleColorShaderP.setUniformValue("color", color);
            m_singleColorShaderP.setAttributeArray("vertex", points.constData());
            m_singleColorShaderP.enableAttributeArray("vertex");
            glDrawArrays(GL_LINE_STRIP, 0, points.size());
            m_singleColorShaderP.disableAttributeArray("vertex");

            //Draw Rectangles around annotations
            QPoint upperLeft(   5,  5+i*135);
            QPoint upperRight(135,  5+i*135);
            QPoint lowerRight(135,135+i*135);
            QPoint lowerLeft(   5,135+i*135);

            points.clear();
            points.push_back(QPointNormalizedToVector3D(upperLeft));
            points.push_back(QPointNormalizedToVector3D(upperRight));
            points.push_back(QPointNormalizedToVector3D(lowerRight));
            points.push_back(QPointNormalizedToVector3D(lowerLeft));

            color = QVector4D(0.8f,0.8f,0.8f,1.0f);
            m_singleColorShaderP.setUniformValue("color", color);
            m_singleColorShaderP.setAttributeArray("vertex", points.constData());
            m_singleColorShaderP.enableAttributeArray("vertex");
            glDrawArrays(GL_LINE_LOOP, 0, points.size());
            m_singleColorShaderP.disableAttributeArray("vertex");

        }

        // Annotated features (Points, Distances, Angles, Areas)
        for (int i=0; i < m_model->m_listOfPointClouds.size(); i++)
        {
            m_singleColorShaderP.setUniformValue("mvpMatrix", m_pMatrix * m_vMatrixCurrent * m_MatrixArtefact);
            m_singleColorShaderP.setAttributeArray("vertex", m_model->m_listOfPointClouds[i].points.constData());
            m_singleColorShaderP.enableAttributeArray("vertex");
            QVector3D temp;
            QVector4D color;
            switch (m_model->m_listOfPointClouds[i].typeId)
            {
            case 0: //Point
                color = QVector4D(m_model->m_listOfPointClouds[i].color.redF(), m_model->m_listOfPointClouds[i].color.greenF(), m_model->m_listOfPointClouds[i].color.blueF(), 0.7f);
                m_singleColorShaderP.setUniformValue("color", color);

                glDrawArrays(GL_POINTS, 0, m_model->m_listOfPointClouds[i].points.size());
                break;
            case 1: //Distance
                color = QVector4D(m_model->m_listOfPointClouds[i].color.redF(), m_model->m_listOfPointClouds[i].color.greenF(), m_model->m_listOfPointClouds[i].color.blueF(), 1.0f);
                m_singleColorShaderP.setUniformValue("color", color);
                glDrawArrays(GL_LINE_STRIP, 0, m_model->m_listOfPointClouds[i].points.size());
                break;
            case 2: //Angle
                color = QVector4D(m_model->m_listOfPointClouds[i].color.redF(), m_model->m_listOfPointClouds[i].color.greenF(), m_model->m_listOfPointClouds[i].color.blueF(), 1.0f);
                m_singleColorShaderP.setUniformValue("color", color);
                temp = m_model->m_listOfPointClouds[i].points[0];
                m_model->m_listOfPointClouds[i].points[0] = m_model->m_listOfPointClouds[i].points[1];
                m_model->m_listOfPointClouds[i].points[1] = temp;
                glDrawArrays(GL_LINE_STRIP, 0, 3);
                m_model->m_listOfPointClouds[i].points[1] = m_model->m_listOfPointClouds[i].points[0];
                m_model->m_listOfPointClouds[i].points[0] = temp;
                glDrawArrays(GL_LINE_STRIP, 3, 5);
                break;
            case 3: //Area
                color = QVector4D(m_model->m_listOfPointClouds[i].color.redF(), m_model->m_listOfPointClouds[i].color.greenF(), m_model->m_listOfPointClouds[i].color.blueF(), 0.35f);
                m_singleColorShaderP.setUniformValue("color", color);
                glDrawArrays(GL_TRIANGLE_FAN, 0, m_model->m_listOfPointClouds[i].points.size());
                break;
            }
            m_singleColorShaderP.disableAttributeArray("vertex");
        }

        // Paint a small rectangle around the selected point (if there is one)
        if (m_selectedPoint != -1)
        {
            QPoint whiteSelect(QVector3DUnnormalizedToQPoint((m_pMatrix *m_vMatrixCurrent * m_MatrixArtefact) * m_model->m_listOfPointClouds.last().points[m_selectedPoint]));

            //Draw Rectangles around selected point
            int rectSize = 4;
            QPoint upperLeft =  whiteSelect + QPoint(-rectSize,-rectSize);
            QPoint upperRight = whiteSelect + QPoint( rectSize,-rectSize);
            QPoint lowerRight = whiteSelect + QPoint( rectSize, rectSize);
            QPoint lowerLeft =  whiteSelect + QPoint(-rectSize, rectSize);

            QVector<QVector3D> points;
            points.push_back(QPointNormalizedToVector3D(upperLeft));
            points.push_back(QPointNormalizedToVector3D(upperRight));
            points.push_back(QPointNormalizedToVector3D(lowerRight));
            points.push_back(QPointNormalizedToVector3D(lowerLeft));

            m_singleColorShaderP.setUniformValue("mvpMatrix",identityMvpMatrix);
            QVector4D color(1.0f,1.0f,1.0f,1.0f);
            m_singleColorShaderP.setUniformValue("color", color);
            m_singleColorShaderP.setAttributeArray("vertex", points.constData());
            m_singleColorShaderP.enableAttributeArray("vertex");
            glDrawArrays(GL_LINE_LOOP, 0, points.size());
            m_singleColorShaderP.disableAttributeArray("vertex");
        }

        m_singleColorShaderP.release();

        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        painter.endNativePainting();
    }
}


/////////////////////////////////////////////////////////////////
///////////////////////////KINECT EVENTS//////////////////////////
/////////////////////////////////////////////////////////////////

//! 2016: This method listens to the events sent by the kinect
/*! this method recieves an AVHand object from the Kinect Sensor and
 * processes it by detecting the state of the hand and setting the
 * message for the user.
 */
void AVGLWidget::catchKP(AVHand mano)
{
    m_kIndex=mano.index;
    if(!m_kinectIsWatching&&tIdle)
        m_status="Please hold both hands open,facing the Kinect camera and away from your body";
    if(m_kLOC>0&&m_kROC>0)
        m_status="Left Hand is Open Right Hand is Open";
    if(m_kLLC>0&&m_kROC>0)
        m_status="Left Hand is Lasso Right Hand is Open";
    if(m_kLOC>0&&m_kRCC>0)
        m_status="Left Hand is Open Right Hand is Closed";
    if(m_kLCC>0&&m_kRCC>0)
        m_status="Left Hand is Closed Right Hand is Closed";
    if(m_kLLC>0&&m_kRCC>0)
        m_status="Left Hand is Lasso Right Hand is Closed";
    if(m_kLOC>0&&m_kRLC>0)
        m_status="Left Hand is Open Right Hand is Lasso";
    if(m_kLCC>0&&m_kRLC>0)
        m_status="Left Hand is Closed Right Hand is Lasso";
    if(m_kLLC>0&&m_kRLC>0)
        m_status="Left Hand is Lasso Right Hand is Lasso";
    if((m_kLNTC>0||m_kLUC>0)&&m_kinectIsWatching)
        m_status="Left Hand (Body "+QString::number(m_kIndex)+ ") is Lost";
    if((m_kRNTC>0||m_kRUC>0)&&m_kinectIsWatching)
        m_status="Right Hand (Body "+QString::number(m_kIndex)+ ")is Lost";
    //!if both hands have been open for more than 3 seconds, kinect starts watching

    if(m_kLOC>5&&m_kROC>5&&!m_kinectIsWatching){
        //"Started reading"
        m_kinectIsWatching=true;
        return;
    }

    //!if translation is off and kinect is watching and the signal is from the left hand, we begin to translate with left.

    if( !m_kLTr&&(m_kinectIsWatching&&(m_kLCC==2&&m_kROC>2))&&mano.isLeft)
    {
        m_kLTr=true;
        //!For horizontal screen, y and z coordinates are swapped
        m_kInitialTrPos= QVector4D((qreal)-mano.x,(qreal)mano.z,(qreal)-mano.y, 1);
    }

    //!if kinect is watching and left (scenario) translation is on and the signal is from the left hand, we translate with left.

    if(m_kinectIsWatching&&m_kLTr&&mano.isLeft)
    {
        //!For horizontal screen, y and z coordinates are swapped
        m_kNewTrPos= QVector4D((qreal)-mano.x,(qreal)mano.z,(qreal)-mano.y, 1);

        m_kResTr=m_camDistanceToOrigin/5*(m_kInitialTrPos-m_kNewTrPos);
        m_status="Left Hand is Closed Right Hand is Open, Scenario translation is active";

        if(abs(m_kResTr.x())<100&&abs(m_kResTr.y())<100&&abs(m_kResTr.z())<100)
        {
            m_camOrigin=m_camOrigin-m_camRotateMatrix.mapVector(m_kResTr.toVector3D());
        }
        m_kInitialTrPos=m_kNewTrPos;
        if(m_kROC==0||m_kLCC==0)
        {
            m_kLTr=false;
        }
        updateGL();
    }

    //!if translation is off and kinect is watching and the signal is from the right hand, we begin to translate with right.

    if( !m_kRTr&&(m_kinectIsWatching&&((m_kLOC>2&&m_kRCC==2)&&!mano.isLeft)))
    {
        m_kRTr=true;

        //!For horizontal screen, y and z coordinates are swapped
        m_kInitialTrPos= QVector4D((qreal)-mano.x,(qreal)mano.z,(qreal)-mano.y, 1);
    }

    //!if kinect is watching and right (artefact) translation is on and the signal is from the right hand, we translate with right.

    if(m_kinectIsWatching&&m_kRTr&&!mano.isLeft)
    {
        //! For horizontal screen, y and z coordinates are swapped
        m_kNewTrPos= QVector4D((qreal)-mano.x,(qreal)mano.z,(qreal)-mano.y, 1);
        m_kResTr=m_camDistanceToOrigin/5*(m_kInitialTrPos-m_kNewTrPos);
        m_axis = m_MatrixArtefact.inverted().mapVector(
                    m_camRotateMatrix.mapVector(
                    m_kResTr.toVector3D()) );
        m_status+=" Object translation is active";


        if(abs(m_kResTr.x())<100&&abs(m_kResTr.y())<100&&abs(m_kResTr.z())<100)
        {
            m_MatrixArtefact.translate(m_axis);
        }

        m_kInitialTrPos=m_kNewTrPos;
        if(m_kLOC==0||m_kRCC==0)
        {
            m_kRTr=false;
        }
        updateGL();
    }

    //!3DObject Rotation Scaling with kinect
    if(!m_kRot&&m_kinectIsWatching
            &&((m_kLCC>2&&m_kRCC==2)||(m_kLCC==2&&m_kRCC>2)))
    {
        m_kRTr=false;
        m_kLTr=false;
        if(mano.isLeft&&!lRotInit){
            //save position for the left hand
            m_kNewLeftRotPos=QVector3D((qreal)mano.x,(qreal)-mano.z,(qreal)mano.y);
            m_kOldLeftRotPos=m_kNewLeftRotPos;
            lRotInit=true;
            m_kRCC=1;
            //"Prepared left 3DORotScaling"
        }
        if(!mano.isLeft&&!rRotInit){
            //save position for the right hand
            m_kNewRightRotPos=QVector3D((qreal)mano.x,(qreal)-mano.z,(qreal)mano.y);
            m_kOldRightRotPos=m_kNewRightRotPos;
            rRotInit=true;
            m_kLCC=1;
            //"Prepared right 3DORotScaling"
        }
    }
    //once the left and right rotation initial positions have been saved
    if(lRotInit&&rRotInit&&!m_kRot)
    {
        m_kRot=true;
        //draw the old line
        m_kOldRotVec=m_kOldRightRotPos-m_kOldLeftRotPos;
        lRotInit=false;
        rRotInit=false;
        //"Initialized 3D RotScale"
    }

    if(m_kRot){
        m_status+=" Object Rotating and Scaling is active";
        //refresh hand positions
        if(!mano.isLeft)
        {
            m_kNewRightRotPos=QVector3D((qreal)mano.x,(qreal)-mano.z,(qreal)mano.y);
        }
        if(mano.isLeft)
        {
            m_kNewLeftRotPos=QVector3D((qreal)mano.x,(qreal)-mano.z,(qreal)mano.y);
        }
        //draw new rotation line
        m_kNewRotVec=m_kNewRightRotPos-m_kNewLeftRotPos;
        //calculate axis and angle
        m_kRotAngle=acos(QVector3D::dotProduct(m_kOldRotVec.normalized(),m_kNewRotVec.normalized()))* 180.0 / PI;
        m_axis = QVector3D::crossProduct(m_kOldRotVec.normalized(),m_kNewRotVec.normalized());
        m_axis = m_MatrixArtefact.inverted().mapVector(
                    m_camRotateMatrix.mapVector(
                        m_axis) );
        //rotate by axis and angle
        if(m_kRotAngle<30||m_kRotAngle>330)
        {
            m_MatrixArtefact.translate(m_model->m_centerPoint);
            m_MatrixArtefact.rotate(m_kRotAngle,m_axis.normalized());
            if(m_kNewRotVec.length()/m_kOldRotVec.length()<1.25f &&
                    m_kNewRotVec.length()/m_kOldRotVec.length()>0.8f)
                m_MatrixArtefact.scale(m_kNewRotVec.length()/m_kOldRotVec.length());
            m_MatrixArtefact.translate(-m_model->m_centerPoint);
        }

        updateGL();
        //update hand positions and connection vector
        m_kOldLeftRotPos=m_kNewLeftRotPos;
        m_kOldRightRotPos=m_kNewRightRotPos;
        m_kOldRotVec=m_kNewRotVec;

        if(m_kLCC==0||m_kRCC==0)
        {
            m_kRot=false;
            m_kRTr=false;
            m_kLTr=false;
            lRotInit=false;
            rRotInit=false;
        }

    }

    //!Lasso moves
    if(!m_kScale&&m_kinectIsWatching
            &&((m_kLLC>2&&m_kRLC==2)||(m_kLLC==2&&m_kRLC>2)))
    {
        if(mano.isLeft&&!lScaleInit){
            //save position for the left hand
            m_kNewLeftScalePos=QVector3D((qreal)mano.x,(qreal)-mano.z,(qreal)mano.y);
            m_kOldLeftScalePos=m_kNewLeftScalePos;
            lScaleInit=true;
            m_kRLC=1;
            //"Prepared left 3DSRotScaling"
        }
        if(!mano.isLeft&&!rScaleInit){
            //save position for the right hand
            m_kNewRightScalePos=QVector3D((qreal)mano.x,(qreal)-mano.z,(qreal)mano.y);
            m_kOldRightScalePos=m_kNewRightScalePos;
            rScaleInit=true;
            m_kLLC=1;
            //"Prepared right 3DSRotScaling"
        }
    }
    if(lScaleInit&&rScaleInit&&!m_kScale)
    {
        m_kScale=true;
        //draw the old line
        m_kOldScaleDist=m_kOldRightScalePos.distanceToPoint(m_kOldLeftScalePos);
        m_kOldRotVec=m_kOldRightScalePos-m_kOldLeftScalePos;
        lScaleInit=false;
        rScaleInit=false;
        //"Initialized 3D Scenario RotScale"
    }

    if(m_kScale){
        m_status+=" Scenario Rotate and Scale is active";
        //refresh hand positions
        if(!mano.isLeft)
        {
            m_kNewRightScalePos=QVector3D((qreal)mano.x,(qreal)-mano.z,(qreal)mano.y);
        }
        if(mano.isLeft)
        {
            m_kNewLeftScalePos=QVector3D((qreal)mano.x,(qreal)-mano.z,(qreal)mano.y);
        }
        //draw new Scalation line
        m_kNewScaleDist=m_kNewRightScalePos.distanceToPoint(m_kNewLeftScalePos);

        //draw new rotation line
        m_kNewRotVec=m_kNewRightScalePos-m_kNewLeftScalePos;
        //calculate axis and angle
        m_kRotAngle=acos(QVector3D::dotProduct(m_kOldRotVec.normalized(),m_kNewRotVec.normalized()))* 180.0 / PI;
        m_axis = QVector3D::crossProduct(m_kOldRotVec.normalized(),m_kNewRotVec.normalized());
        if(m_kOldScaleDist/m_kNewScaleDist<1.25f &&
                m_kOldScaleDist/m_kNewScaleDist>0.8f)
            m_camDistanceToOrigin*=m_kOldScaleDist/m_kNewScaleDist;
        if(m_kRotAngle<30||m_kRotAngle>330)
            m_camRotateMatrix.rotate(-m_kRotAngle,m_axis.normalized());
        updateGL();
        //update hand positions and connection vector
        m_kOldLeftScalePos=m_kNewLeftScalePos;
        m_kOldRightScalePos=m_kNewRightScalePos;
        m_kOldScaleDist=m_kNewScaleDist;
        m_kOldRotVec=m_kNewRotVec;

        if(m_kLLC==0||m_kRLC==0)
        {
            m_kScale=false;
            m_kRTr=false;
            m_kLTr=false;
            lScaleInit=false;
            rScaleInit=false;
            //"stopped 3D Scaling"
        }

    }

    if(m_kLNTC>5&&m_kRNTC>5){
        m_status=" Stopped reading, both hands untracked for more than 5s";
        m_kinectIsWatching=false;
        m_kLNTC=0;
        m_kRNTC=0;
    }
    if(m_kLUC>50||m_kRUC>50){
        m_status=" Stopped reading, one or more hands were unrecognized or unknown for about 10s";
        m_kinectIsWatching=false;
        m_kLNTC=0;
        m_kRNTC=0;
        m_kLUC=0;
        m_kRUC=0;
    }
    AVGLWidget::kinectCount(mano);
    AVStatus->showMessage(m_status,0);
}

/*! This method switches smoothly between hand states to allow for a smooth interaction
 */
void AVGLWidget::kinectCount(AVHand mano){

    if(mano.hState==0&&mano.isLeft==true){
        //        cout<<"Left hand state is unknown since "<<m_kLUC<<endl;
        m_kLUC++;

    }
    if(mano.hState==1&&mano.isLeft==true){
        //        cout<<"Left hand not tracked since "<<m_kLNTC<<endl;
        resetLeftKinectCounts();
        m_kLNTC++;
    }
    if(mano.hState==2&&mano.isLeft==true){
        //        cout<<"Left hand is open since "<<m_kLOC<<endl;
        m_kLOC++;
        m_kLCC=0;
        m_kLUC=0;
        m_kLNTC=0;
        m_kLLC=0;
    }
    if(mano.hState==3&&mano.isLeft==true){
        //        cout<<"Left hand is closed since "<<m_kLCC<<endl;
        m_kLCC++;
        m_kLOC=0;
        m_kLUC=0;
        m_kLNTC=0;
        m_kLLC=0;
    }
    if(mano.hState==4&&mano.isLeft==true){
        //        cout<<"Left Hand is Lasso since "<<m_kLLC<<endl;
        m_kLLC++;
    }
    if(mano.hState==0&&mano.isLeft==false){
        //        cout<<"Right hand state is unknown since "<<m_kRUC<<endl;
        m_kRUC++;
    }
    if(mano.hState==1&&mano.isLeft==false){
        //        cout<<"Right hand not tracked since "<<m_kRNTC<<endl;
        resetRightKinectCounts();
        m_kRNTC++;
    }
    if(mano.hState==2&&mano.isLeft==false){
        //        cout<<"Right hand is open since "<<m_kROC<<endl;
        m_kROC++;
        m_kRCC=0;
        m_kRUC=0;
        m_kRNTC=0;
        m_kRLC=0;

    }
    if(mano.hState==3&&mano.isLeft==false){
        //        cout<<"Right hand is closed since "<<m_kRCC<<endl;
        m_kRCC++;
        m_kROC=0;
        m_kRUC=0;
        m_kRNTC=0;
        m_kRLC=0;
    }
    if(mano.hState==4&&mano.isLeft==false){
        //        cout<<"Right Hand is Lasso since "<<m_kRLC<<endl;
        m_kRLC++;
    }



}

void AVGLWidget::resetLeftKinectCounts(){
    m_kLOC=0;
    m_kLCC=0;
    m_kLLC=0;
}

void AVGLWidget::resetRightKinectCounts(){
    m_kROC=0;
    m_kRCC=0;
    m_kRLC=0;
}


/////////////////////////////////////////////////////////////////
///////////////////////////TOUCH EVENTS//////////////////////////
/////////////////////////////////////////////////////////////////

//! 2016: This function is called if at any time there are signals from the touchscreen.
void AVGLWidget::catchPF(AVPointFrame pFrame)
{   QPointF screenPosCam;
    QPointF localPosCam;
    //!if there is one point, translation movement is called
    //initialization and assignment of the finger position and mapping from global coordinates

    screenPosCam=QPointF((qreal)pFrame.pf_moving_point_array[0].x,
            (qreal)pFrame.pf_moving_point_array[0].y);
    localPosCam=this->mapFromGlobal(screenPosCam.toPoint());
    //For TESTING: this corrects the dislocation if the program thinks 
    //that it is on a different screen than the touchpoint
    //        if(QApplication::desktop()->screenNumber(this)!=QApplication::desktop()->screenNumber(screenPosCam.toPoint()))
    //        {
    //            QPoint displasia;displasia.setX(QApplication::desktop()->screenGeometry(this).width());
    //            localPosCam=displasia+localPosCam;
    //        }
    //        uncomment to tune this dislocation patch
    //        cout << "local pos cam: " << localPosCam.x() << " " << localPosCam.y()
    //                  << " Widget dimensions" << height() << " " << width()
    //                  << " Widget Position" <<this->geometry().topLeft().x()<<","<<this->geometry().topLeft().y()
    //                  << endl;
    if(localPosCam.x()>0&&localPosCam.x()<width() &&localPosCam.y()>0&&localPosCam.y()<height())
    {
        tIdle=false;
        if(pFrame.pf_moving_point_count==1)
        {
            //!test if the touch is in the widget

            //!on touch start initialize finger position
            if(pFrame.pf_moving_point_array[0].point_event==TP_DOWN)
            {
                m_iFP.setX((int)pFrame.pf_moving_point_array[0].x);
                m_iFP.setY((int)pFrame.pf_moving_point_array[0].y);
            }
            else
            {
                //!on finger movement calculate the distance to initial finger position (iFP)
                if(pFrame.pf_moving_point_array[0].point_event==TP_MOVE)
                {
                    double deltaX = pFrame.pf_moving_point_array[0].x - m_iFP.x();
                    double deltaY = pFrame.pf_moving_point_array[0].y - m_iFP.y();

                    //filter jumpy movements that are too big (bigger than m_jumpSize)
                    if(abs(deltaX)<m_jumpSize&&abs(deltaY)<m_jumpSize)
                    {
                        if(abs(deltaX)<m_jumpSize&&abs(deltaY)<m_jumpSize)
                        {
                            m_axis = m_MatrixArtefact.inverted().mapVector(
                                        m_camRotateMatrix.mapVector(
                                            QVector3D(deltaX,-deltaY,0)
                                            ));
                            m_MatrixArtefact.translate(m_axis*m_camDistanceToOrigin/1000);
                        }
                    }
                    //reset "initial" finger position
                    m_iFP.setX((int)pFrame.pf_moving_point_array[0].x);
                    m_iFP.setY((int)pFrame.pf_moving_point_array[0].y);

                }

                if(pFrame.pf_moving_point_array[0].point_event==TP_UP)
                {
                    tIdle=true;
                    RotScaling=false;
                    rotating=false;
                    zooming=false;
                    moving=false;
                    spinning=false;
                }
            }

        }


        else //(if more than one finger)
        {
            //! With 2 fingers: Scale and rotate
            if(pFrame.pf_moving_point_count==2)
            {
                //!Scaling and rotation start if the second finger starts to touch
                if(!RotScaling&&
                        (pFrame.pf_moving_point_array[0].point_event==TP_DOWN||
                         pFrame.pf_moving_point_array[1].point_event==TP_DOWN||
                         (pFrame.pf_moving_point_array[0].point_event==TP_MOVE&&
                          pFrame.pf_moving_point_array[1].point_event==TP_MOVE)))
                {
                    RotScaling=true;
                    rotating=false;
                    zooming=false;
                    moving=false;
                    spinning=false;
                    //get the initial scaling separation
                    m_iZS.setX((qreal)(pFrame.pf_moving_point_array[0].x-pFrame.pf_moving_point_array[1].x));
                    m_iZS.setY((qreal)(pFrame.pf_moving_point_array[0].y-pFrame.pf_moving_point_array[1].y));
                    m_nZS=m_iZS;

                    //save both touch //map them to screen coordinates
                    m_tRot1=QPointF(mapFromGlobal(QPoint(
                                                      (qreal)(pFrame.pf_moving_point_array[0].x),
                                                  (qreal) (pFrame.pf_moving_point_array[0].y))));
                    m_tRot2=QPointF(mapFromGlobal(QPoint(
                                                      (qreal)(pFrame.pf_moving_point_array[1].x),
                                                  (qreal) (pFrame.pf_moving_point_array[1].y))));
                    //draw a line between them
                    m_tRotL1=QLineF(m_tRot1,m_tRot2);
                    //initialize both rotation lines as equal
                    m_tRotL2=m_tRotL1;
                }

                else //(no finger is starting the touch)
                {
                    //acquire new zoom separation
                    m_nZS=QPointF((qreal)(pFrame.pf_moving_point_array[0].x-pFrame.pf_moving_point_array[1].x),
                            (qreal)(pFrame.pf_moving_point_array[0].y-pFrame.pf_moving_point_array[1].y));
                    //!scaling and rotation take place if one of the fingers is moving and RotScaling is on:
                    if(RotScaling&&
                            pFrame.pf_moving_point_array[0].point_event==TP_MOVE&&
                            pFrame.pf_moving_point_array[1].point_event==TP_MOVE)
                    {
                        //!on pinch zoom out, on split zoom in
                        //if the distance between fingers changes enough but the quotient not too big nor too small
                        if(abs(m_nZS.manhattanLength()-m_iZS.manhattanLength())>1
                                &&m_nZS.manhattanLength()/m_iZS.manhattanLength()<1.5f
                                &&m_nZS.manhattanLength()/m_iZS.manhattanLength()>0.75f)
                        {
                            m_MatrixArtefact.translate(m_model->m_centerPoint);
                            //                        m_MatrixArtefact.translate(m_camOrigin);
                            m_MatrixArtefact.scale(m_nZS.manhattanLength()/m_iZS.manhattanLength());
                            //                        m_MatrixArtefact.translate(-m_camOrigin);
                            m_MatrixArtefact.translate(-m_model->m_centerPoint);

                        }
                        m_iZS=m_nZS;

                        //save both touch //map them to screen coordinates
                        m_tRot1=QPointF(mapFromGlobal(QPoint(
                                                          (qreal)(pFrame.pf_moving_point_array[0].x),
                                                      (qreal) (pFrame.pf_moving_point_array[0].y))));
                        m_tRot2=QPointF(mapFromGlobal(QPoint(
                                                          (qreal)(pFrame.pf_moving_point_array[1].x),
                                                      (qreal) (pFrame.pf_moving_point_array[1].y))));
                        //draw a line between both fingers and save to new touch Rotation line
                        m_tRotL2=QLineF(m_tRot1,m_tRot2);
                        newAngleCR=m_tRotL2.angleTo(m_tRotL1);
                        newAngleSign=-(newAngleCR-180)/abs(newAngleCR-180);
                        m_tRotAngle=(-abs(newAngleCR-180)+180)*newAngleSign;
                        //rotate
                        if(abs(m_tRotAngle)<10)
                        {
                            m_MatrixArtefact.translate(m_model->m_centerPoint);
                            //                        m_MatrixArtefact.translate(m_camOrigin);
                            m_MatrixArtefact.rotate(-m_tRotAngle,
                                                    m_MatrixArtefact.inverted().mapVector(
                                                        m_camRotateMatrix.mapVector(QVector3D(0,0,1)) ));
                            //                        m_MatrixArtefact.translate(-m_camOrigin);
                            m_MatrixArtefact.translate(-m_model->m_centerPoint);
                        }
                        //refresh touch rotation line
                        m_tRotL1=m_tRotL2;
                    }

                    //!scaling ends if one of both fingers go up
                    if(RotScaling&&(
                                pFrame.pf_moving_point_array[0].point_event==TP_UP||
                                pFrame.pf_moving_point_array[1].point_event==TP_UP))
                    {
                        RotScaling=false;
                        rotating=false;
                        zooming=false;
                        moving=false;
                        spinning=false;
                    }
                }

            }
            else //(with more than 2 fingers)
            {
                if(pFrame.pf_moving_point_count==3){
                    //!on third finger down, initialize trackball movement at the first finger and stop scaling
                    if(!rotating&&
                            (pFrame.pf_moving_point_array[0].point_event==TP_DOWN||
                             pFrame.pf_moving_point_array[1].point_event==TP_DOWN||
                             pFrame.pf_moving_point_array[2].point_event==TP_DOWN||
                             (pFrame.pf_moving_point_array[0].point_event==TP_MOVE&&
                              pFrame.pf_moving_point_array[1].point_event==TP_MOVE&&
                              pFrame.pf_moving_point_array[2].point_event==TP_MOVE)) )
                    {
                        RotScaling=false;
                        rotating=true;
                        zooming=false;
                        moving=false;
                        spinning=false;
                        m_iFP.setX((int)pFrame.pf_moving_point_array[0].x);
                        m_iFP.setY((int)pFrame.pf_moving_point_array[0].y);
                        m_nFP=m_iFP;
                    }
                    else
                    {
                        if(rotating&&(
                                    pFrame.pf_moving_point_array[0].point_event==TP_MOVE||
                                    pFrame.pf_moving_point_array[1].point_event==TP_MOVE||
                                    pFrame.pf_moving_point_array[2].point_event==TP_MOVE))
                        {
                            m_nFP.setX((qreal)-pFrame.pf_moving_point_array[0].x);
                            m_nFP.setY((qreal)pFrame.pf_moving_point_array[0].y);
                            QLineF delta(pixelPosToViewPos(m_iFP),pixelPosToViewPos(m_nFP));
                            m_axis = QVector3D(delta.dy(), delta.dx(), 0.0f).normalized();
                            m_axis = m_MatrixArtefact.inverted().mapVector(
                                        m_camRotateMatrix.mapVector(
                                            m_axis));
                            m_axis.normalize();
                            m_tRotAngle=-180 / PI * delta.length();
                            m_MatrixArtefact.translate(m_model->m_centerPoint);
                            if(abs(m_tRotAngle)<10)
                                m_MatrixArtefact.rotate(-180 / PI * delta.length(),m_axis);
                            m_MatrixArtefact.translate(-m_model->m_centerPoint);
                            m_iFP=m_nFP;
                        }
                        else{
                            if(rotating&&(
                                        pFrame.pf_moving_point_array[0].point_event==TP_UP||
                                        pFrame.pf_moving_point_array[1].point_event==TP_UP||
                                        pFrame.pf_moving_point_array[2].point_event==TP_UP))
                            {
                                RotScaling=true;
                                rotating=false;
                                zooming=false;
                                moving=false;
                                spinning=false;
                            }
                        }
                    }
                }
                else
                {
                    if(pFrame.pf_moving_point_count==4)
                    {
                        //!Zooming starts if the fourth finger starts to touch
                        if(!zooming&&(
                                    pFrame.pf_moving_point_array[0].point_event==TP_DOWN||
                                    pFrame.pf_moving_point_array[1].point_event==TP_DOWN||
                                    pFrame.pf_moving_point_array[2].point_event==TP_DOWN||
                                    pFrame.pf_moving_point_array[3].point_event==TP_DOWN||
                                    (pFrame.pf_moving_point_array[0].point_event==TP_MOVE&&
                                     pFrame.pf_moving_point_array[1].point_event==TP_MOVE&&
                                     pFrame.pf_moving_point_array[2].point_event==TP_MOVE&&
                                     pFrame.pf_moving_point_array[3].point_event==TP_MOVE)))
                        {
                            zooming=true;
                            RotScaling=false;
                            rotating=false;
                            moving=false;
                            spinning=false;
                            //get the initial zooming lines
                            m_OldLeftZoomer=QLine(pFrame.pf_moving_point_array[0].x,pFrame.pf_moving_point_array[0].y,
                                    pFrame.pf_moving_point_array[1].x,pFrame.pf_moving_point_array[1].y);
                            m_OldRightZoomer=QLine(pFrame.pf_moving_point_array[2].x,pFrame.pf_moving_point_array[2].y,
                                    pFrame.pf_moving_point_array[3].x,pFrame.pf_moving_point_array[3].y);
                            m_NewRightZoomer=m_OldRightZoomer;
                            m_NewLeftZoomer=m_OldLeftZoomer;
                        }
                        else //(no finger is starting the touch)
                        {
                            //acquire new zoom lines
                            m_NewLeftZoomer=QLine(pFrame.pf_moving_point_array[0].x,pFrame.pf_moving_point_array[0].y,
                                    pFrame.pf_moving_point_array[1].x,pFrame.pf_moving_point_array[1].y);
                            m_NewRightZoomer=QLine(pFrame.pf_moving_point_array[2].x,pFrame.pf_moving_point_array[2].y,
                                    pFrame.pf_moving_point_array[3].x,pFrame.pf_moving_point_array[3].y);
                            //!zooming takes place if all the fingers is moving and zooming is on:
                            if(zooming&&(pFrame.pf_moving_point_array[0].point_event==TP_MOVE||
                                         pFrame.pf_moving_point_array[1].point_event==TP_MOVE||
                                         pFrame.pf_moving_point_array[2].point_event==TP_MOVE||
                                         pFrame.pf_moving_point_array[3].point_event==TP_MOVE))
                            {
                                //!on pinch zoom out, on split zoom in
                                m_zoomFactor=(m_OldLeftZoomer.length()+m_OldRightZoomer.length())/
                                        (m_NewLeftZoomer.length()+m_NewRightZoomer.length());
                                //if the distance between fingers changes enough but the quotient not too big nor too small
                                if(m_zoomFactor>0.95&&m_zoomFactor<1.1)
                                {
                                    m_camDistanceToOrigin*=m_zoomFactor;
                                }
                                m_OldRightZoomer=m_NewRightZoomer;
                                m_OldLeftZoomer=m_NewLeftZoomer;
                            }
                            //!zooming ends if one finger goes up
                            if(zooming&&(
                                        pFrame.pf_moving_point_array[0].point_event==TP_UP||
                                        pFrame.pf_moving_point_array[1].point_event==TP_UP||
                                        pFrame.pf_moving_point_array[2].point_event==TP_UP||
                                        pFrame.pf_moving_point_array[3].point_event==TP_UP))
                            {
                                RotScaling=false;
                                rotating=true;
                                zooming=false;
                                moving=false;
                                spinning=false;
                            }
                        }
                    }
                    else
                    {
                        if(pFrame.pf_moving_point_count==5)
                        {
                            if(     !moving&&(
                                        pFrame.pf_moving_point_array[0].point_event==TP_DOWN||
                                        pFrame.pf_moving_point_array[1].point_event==TP_DOWN||
                                        pFrame.pf_moving_point_array[2].point_event==TP_DOWN||
                                        pFrame.pf_moving_point_array[3].point_event==TP_DOWN||
                                        pFrame.pf_moving_point_array[4].point_event==TP_DOWN||
                                        (pFrame.pf_moving_point_array[0].point_event==TP_MOVE&&
                                         pFrame.pf_moving_point_array[1].point_event==TP_MOVE&&
                                         pFrame.pf_moving_point_array[2].point_event==TP_MOVE&&
                                         pFrame.pf_moving_point_array[3].point_event==TP_MOVE&&
                                         pFrame.pf_moving_point_array[4].point_event==TP_MOVE)))
                            {
                                m_iFP.setX((qreal)-pFrame.pf_moving_point_array[4].x);
                                m_iFP.setY((qreal)pFrame.pf_moving_point_array[4].y);
                                m_nFP=m_iFP;
                                moving=true;
                                RotScaling=false;
                                rotating=false;
                                zooming=false;
                                spinning=false;
                            }
                            else
                            {
                                //!on finger movement calculate the distance to initial finger position (iFP)
                                if(     moving&&(
                                            pFrame.pf_moving_point_array[0].point_event==TP_MOVE||
                                            pFrame.pf_moving_point_array[1].point_event==TP_MOVE||
                                            pFrame.pf_moving_point_array[2].point_event==TP_MOVE||
                                            pFrame.pf_moving_point_array[3].point_event==TP_MOVE||
                                            pFrame.pf_moving_point_array[4].point_event==TP_MOVE))
                                {
                                    m_nFP.setX((qreal)-pFrame.pf_moving_point_array[4].x);
                                    m_nFP.setY((qreal)pFrame.pf_moving_point_array[4].y);

                                    QVector3D delta;
                                    delta= QVector3D(m_nFP- m_iFP);
                                    //filter jumpy movements that are too big (bigger than m_jumpSize)
                                    if(abs(delta.x())<m_jumpSize&&abs(delta.y())<m_jumpSize)
                                    {
                                        //move the camera.
                                        m_camOrigin+=m_camRotateMatrix.mapVector(delta)* m_camDistanceToOrigin/1000;
                                    }
                                    //reset "initial" finger position
                                    m_iFP=m_nFP;
                                }
                                else{
                                    if(moving&&(
                                                pFrame.pf_moving_point_array[0].point_event==TP_UP||
                                                pFrame.pf_moving_point_array[1].point_event==TP_UP||
                                                pFrame.pf_moving_point_array[2].point_event==TP_UP||
                                                pFrame.pf_moving_point_array[3].point_event==TP_UP||
                                                pFrame.pf_moving_point_array[4].point_event==TP_UP))
                                    {
                                        RotScaling=false;
                                        rotating=false;
                                        zooming=true;
                                        moving=false;
                                        spinning=false;
                                    }
                                }
                            }
                        }
                        else//we have 6 fingers
                        {
                            if(     !spinning&&(
                                        pFrame.pf_moving_point_array[0].point_event==TP_DOWN||
                                        pFrame.pf_moving_point_array[1].point_event==TP_DOWN||
                                        pFrame.pf_moving_point_array[2].point_event==TP_DOWN||
                                        pFrame.pf_moving_point_array[3].point_event==TP_DOWN||
                                        pFrame.pf_moving_point_array[4].point_event==TP_DOWN||
                                        pFrame.pf_moving_point_array[5].point_event==TP_DOWN||
                                        (pFrame.pf_moving_point_array[0].point_event==TP_MOVE&&
                                         pFrame.pf_moving_point_array[1].point_event==TP_MOVE&&
                                         pFrame.pf_moving_point_array[2].point_event==TP_MOVE&&
                                         pFrame.pf_moving_point_array[3].point_event==TP_MOVE&&
                                         pFrame.pf_moving_point_array[4].point_event==TP_MOVE&&
                                         pFrame.pf_moving_point_array[5].point_event==TP_MOVE)))
                            {
                                spinning=true;
                                RotScaling=false;
                                rotating=false;
                                zooming=false;
                                moving=false;
                                m_oldAverageSpinnerLeft.setX(0);m_oldAverageSpinnerLeft.setY(0);
                                for(int i = 0; i <5; ++ i)
                                {
                                    spinning=true;
                                    m_oldAverageSpinnerLeft.operator +=( QPointF((qreal)pFrame.pf_moving_point_array[i].x/5,
                                                                                 pFrame.pf_moving_point_array[i].y/5)
                                                                         );
                                }
                                m_oldAverageSpinnerRight=QPointF((qreal)pFrame.pf_moving_point_array[5].x,
                                        pFrame.pf_moving_point_array[5].y);

                                m_newAverageSpinnerLeft=m_oldAverageSpinnerLeft;
                                m_newAverageSpinnerRight=m_oldAverageSpinnerRight;

                            }
                            else //(no finger is starting the touch)
                            {
                                //acquire new zoom lines
                                m_newAverageSpinnerLeft.setX(0);m_newAverageSpinnerLeft.setY(0);
                                for(int i = 0; i <5; ++ i)
                                {
                                    m_newAverageSpinnerLeft.operator +=( QPointF((qreal)pFrame.pf_moving_point_array[i].x/5,
                                                                                 (qreal)pFrame.pf_moving_point_array[i].y/5)
                                                                         );
                                }

                                m_newAverageSpinnerRight=QPointF((qreal)pFrame.pf_moving_point_array[5].x,
                                        (qreal)pFrame.pf_moving_point_array[5].y);
                                //!spinning takes place if all the fingers are moving and spinning is on:
                                if(spinning&&(
                                            pFrame.pf_moving_point_array[0].point_event==TP_MOVE||
                                            pFrame.pf_moving_point_array[1].point_event==TP_MOVE||
                                            pFrame.pf_moving_point_array[2].point_event==TP_MOVE||
                                            pFrame.pf_moving_point_array[3].point_event==TP_MOVE||
                                            pFrame.pf_moving_point_array[4].point_event==TP_MOVE||
                                            pFrame.pf_moving_point_array[5].point_event==TP_MOVE
                                            ))
                                {
                                    m_OldCenterSpinner=QLine(m_oldAverageSpinnerLeft.toPoint(),m_oldAverageSpinnerRight.toPoint());
                                    m_NewCenterSpinner=QLine(m_newAverageSpinnerLeft.toPoint(),m_newAverageSpinnerRight.toPoint());
                                    //!we turn the camera up direction by the average change in angles
                                    newAngleCR=m_NewCenterSpinner.angleTo(m_OldCenterSpinner);
                                    newAngleSign=-(newAngleCR-180)/abs(newAngleCR-180);
                                    resultingAngle=(-abs((newAngleCR-180))+180)*newAngleSign;
                                    if(abs(resultingAngle)<30)
                                        m_camRotateMatrix.rotate(resultingAngle,
                                                                 m_camRotateMatrix.mapVector(QVector3D(0,0,1)));
                                    m_oldAverageSpinnerLeft=m_newAverageSpinnerLeft;
                                    m_oldAverageSpinnerRight=m_newAverageSpinnerRight;
                                }
                            }
                            if(spinning&&(
                                        pFrame.pf_moving_point_array[0].point_event==TP_UP||
                                        pFrame.pf_moving_point_array[1].point_event==TP_UP||
                                        pFrame.pf_moving_point_array[2].point_event==TP_UP||
                                        pFrame.pf_moving_point_array[3].point_event==TP_UP||
                                        pFrame.pf_moving_point_array[4].point_event==TP_UP||
                                        pFrame.pf_moving_point_array[5].point_event==TP_UP))
                            {
                                RotScaling=false;
                                rotating=false;
                                zooming=false;
                                moving=true;
                                spinning=false;
                            }
                        }
                    }
                }
            }
        }
    }
    m_status="Touchscreen registered "+QString::number(pFrame.pf_moving_point_count)+" finger(s).";
    if(pFrame.pf_moving_point_count>6)
        m_status="Touchscreen registered more than 6 fingers";
    AVStatus->showMessage(m_status,2000);
    updateGL();
    if(pFrame.pf_moving_point_array[0].point_event==TP_UP)
    {
        tIdle=true;
        RotScaling=false;
        rotating=false;
        zooming=false;
        moving=false;
        spinning=false;
    }
}


//! Handles mouse button press events
/*! mousePressEvent is called on every frame that is drawn with a mouse button down and handles different actions
*  - left mouse button enables free camera rotation and lateral movement
*  - right mouse button has several functions like
*   - selecting an annotation
*   - creating a new point
*   - selecting and dragging a point
*/
void AVGLWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        if(!m_shiftDown) m_trackball->push(pixelPosToViewPos(event->localPos()), QQuaternion());
        m_lastMousePosition = event->pos();
        //        cout << "That was the mouse at:"<<event->localPos().x() << endl;
    }
    else if (event->buttons() & Qt::RightButton)
    {
        bool clickFound = false;

        // Check for the annotations
        if(!clickFound && m_paintAnnotations)
        {
            for (int i=0; i < m_model->m_listOfPointClouds.size()-1; i++)
            {
                if (event->pos().x() > 4 && event->pos().x() < 136 && event->pos().y() > (4+i*135) && event->pos().y() < (134+i*135))
                {
                    m_currentAnnotation = i;
                    clickFound = true;
                    emit annotationClicked();
                    updateGL();
                }
            }
        }

        // Check for dragging of a point
        if(!clickFound && m_paintAnnotations)
        {
            for (int i=0; i < m_model->m_listOfPointClouds.last().points.size(); i++)
            {
                QVector3D projectedPoint(m_pMatrix*m_vMatrixCurrent*m_MatrixArtefact*m_model->m_listOfPointClouds.last().points[i]);
                int unNormalizedX = (projectedPoint.x() + 1) / 2 * this->width();
                int unNormalizedY = (-projectedPoint.y()+ 1) / 2 * this->height();
                float distance = 0.0;
                distance = qSqrt( (event->pos().x()-unNormalizedX) * (event->pos().x()-unNormalizedX) +
                                  (event->pos().y()-unNormalizedY) * (event->pos().y()-unNormalizedY) );
                if (distance < 14)
                {
                    clickFound = true;
                    m_draggedPoint = m_selectedPoint = i;
                    emit pointSelected();
                    QVector3D intersectionPoint;
                    getIntersectionPoint(QPoint(event->x(), event->y()), &intersectionPoint,m_vMatrixCurrent);
                    if (!intersectionPoint.isNull())
                    {
                        m_model->m_listOfPointClouds.last().points[m_draggedPoint] = (m_MatrixArtefact.inverted() * intersectionPoint);
                        updateGL();
                    }
                }
            }
        }

        //check for clicking on a light
        if(!clickFound && m_lightsAreVisible)
        {
            int lightClickedIndex = -1;
            bool lightClicked = getClickedLight(QPoint(event->x(), event->y()), &lightClickedIndex,m_vMatrixCurrent);
            if(lightClicked){
                qDebug() << "found click on light: " << lightClickedIndex;
                AVMainWindow* mainWindow = AVMainWindow::instance();
                mainWindow->setCurrentLightIndex(lightClickedIndex);
                mainWindow->updateLightComboBox();
            }

        }

        // Check for a click on the artefact
        if (!clickFound && m_paintAnnotations)
        {
            //TODO: Speed up with Bounding Box Test
            QVector3D intersectionPoint;

            getIntersectionPoint(QPoint(event->x(), event->y()), &intersectionPoint, m_vMatrixCurrent);
            if (!intersectionPoint.isNull())
            {
                m_model->m_listOfPointClouds.last().points.append(m_MatrixArtefact.inverted() * intersectionPoint);
                emit pointAdded();
                updateGL();
            }
        }

    }
    event->accept();
}


//! Disables dragging of points
void AVGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_trackball->release(pixelPosToViewPos(event->localPos()), m_MatrixArtefact);

    (void*) event;
    m_draggedPoint = -1;
}


//! Handles events from mouse movement
/*! wheelMoveEvent is called on every frame and looks for movement of the mouse to handle different actions
*  - while a point is dragged with left button, its representation is drawn on the artefact
*  - while right mouse button is pressed, mouse movement adjusts the camera rotation
*  - right button and shift key results in lateral camera movement
* Since 2016, this can also handle events from a single finger in a touchscreen
*/
void AVGLWidget::mouseMoveEvent(QMouseEvent *event)
{


    double deltaX = event->x() - m_lastMousePosition.x();
    double deltaY = event->y() - m_lastMousePosition.y();

    if (event->buttons() & Qt::RightButton) {
        if (m_draggedPoint != -1)
        {
            QVector3D intersectionPoint;
            getIntersectionPoint(QPoint(event->x(), event->y()), &intersectionPoint,m_vMatrixCurrent);
            if (!intersectionPoint.isNull())
            {
                m_model->m_listOfPointClouds.last().points[m_draggedPoint] = (m_MatrixArtefact.inverted() * intersectionPoint);
                updateGL();
            }
        }
    }
    else if (event->buttons() & Qt::LeftButton && !m_shiftDown)
    {
        QQuaternion rotation = m_trackball->move(pixelPosToViewPos(event->localPos()), m_MatrixArtefact);
        m_MatrixArtefact.translate(m_model->m_centerPoint);
        m_MatrixArtefact.rotate(rotation);
        m_MatrixArtefact.translate(-m_model->m_centerPoint);
    }
    else if (event->buttons() & Qt::LeftButton && m_shiftDown)
    {
        //calculate the vectors in which the camera should move "down" and "left" starting from world ccordinates
        QVector3D camUpDirection(0,1,0), camRightDirection(1,0,0);
        m_camOrigin+=m_camRotateMatrix.mapVector(camRightDirection) * -deltaX/6.3f * m_camDistanceToOrigin/150.0f;
        m_camOrigin+=m_camRotateMatrix.mapVector(camUpDirection) * deltaY/6.3f * m_camDistanceToOrigin/150.0f;
    }
    m_lastMousePosition = event->pos();
    event->accept();
    updateGL();
}


void AVGLWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::RightButton && m_paintAnnotations == true)
    {
        // Check for the annotations
        bool clickFound = false;

        for (int i=0; i < m_model->m_listOfPointClouds.size()-1; i++)
        {
            if (event->pos().x() > 4 && event->pos().x() < 136 && event->pos().y() > (4+i*135) && event->pos().y() < (134+i*135))
            {
                m_currentAnnotation = i;
                clickFound = true;
                emit annotationClicked();
                emit annotationDoubleClicked();
                updateGL();
            }
        }
    }
}


//! normalizeAngle180 limits all input angles to the interval between -90 and 90. Everything beyond 90 is cut off.
double AVGLWidget::normalizeAngle180(double angle)
{
    while (angle < -90.0) angle = -90.0;
    while (angle > 90.0) angle = 90.0;
    return angle;
}


//! normalizeAngle360 limits all input angles to the interval between 0 and 360.
/*! Values below 0 get 360 degrees added until above 0. Values above 360 get 360 subtracted until below 360
*/
double AVGLWidget::normalizeAngle360(double angle)
{
    while (angle < 0.0) angle += 360.0;
    while (angle >= 360.0) angle -= 360.0;
    return angle;
}


//! Calculates an intersection point for a mouse cursor point on the artefact
/*! Calculates a ray from the camera through the mouse click point on the view plane. The determines for all
 * triangles whether they could be hit by that ray by checking whether their corresponding plane is hit.
 * For all hit planes a calculation is done that determines, whether the hit was actually inside the triangle.
 * For all those points the point closest to the camera is returned.
 */
bool AVGLWidget::getIntersectionPoint(QPoint point, QVector3D *desiredPoint, QMatrix4x4 l_vMatrix)
{
    //TODO: Speed up with bounding boxes
    QMatrix4x4 unviewMatrix;
    QVector3D rayDir;
    QVector3D normalizedPoint(QPointNormalizedToVector3D(point));
    unviewMatrix = (m_pMatrix * l_vMatrix).inverted();

    rayDir = ((unviewMatrix * normalizedPoint) - m_camPosition).normalized();

    QVector<QVector3D> possiblePoints;
    // For every triangle
    QVector3D a,b,c,planeNormal;
    for (int i=0; i < m_model->m_triangles.size(); i+=3)
    {
        a = m_MatrixArtefact*m_model->m_vertices[m_model->m_triangles[i]];
        b = m_MatrixArtefact*m_model->m_vertices[m_model->m_triangles[i+1]];
        c = m_MatrixArtefact*m_model->m_vertices[m_model->m_triangles[i+2]];
        planeNormal = QVector3D::normal(a,b,c);

        // Get the intersection point with its plane
        float lambda = -QVector3D::dotProduct(a, planeNormal);
        QVector3D intersection = m_camPosition+(-(QVector3D::dotProduct(m_camPosition, planeNormal) + lambda) / QVector3D::dotProduct(rayDir, planeNormal))*rayDir;

        // Check inside/outside of Triangle and add to list
        if (m_model->isInTriangle(a,b,c,planeNormal,intersection)) possiblePoints.append(intersection);

    }

    // Determine the point closest to the camera
    if (possiblePoints.size() == 0) return false; // User didn't hit the artefact. Bad aiming?

    QVector3D closestPoint(possiblePoints[0]);
    for (int i=1; i < possiblePoints.size(); i++)
    {
        if((possiblePoints[i]-m_camPosition).length() < (closestPoint-m_camPosition).length()) closestPoint = possiblePoints[i];
    }

    *desiredPoint = closestPoint;
    updateGL();
    return true;
}

bool AVGLWidget::getClickedLight(QPoint point, int *light, QMatrix4x4 l_vMatrix)
{
    QVector3D normalizedPoint(QPointNormalizedToVector3D(point));
    QMatrix4x4 unviewMatrix = (m_pMatrix * l_vMatrix).inverted();
    QVector3D rayDir = ((unviewMatrix * normalizedPoint) - m_camPosition).normalized();

    //For every  light and every triangle
    QVector3D a,b,c,planeNormal;
    int lightHitIndex = -1;
    for(int i = 0; i < m_lights.size(); i++)
    {
        for(int j = 0; j < m_model->m_lightVertices.size(); j+=3)
        {
            a = m_lights[i].getTransformation() * m_model->m_lightVertices[j];
            b = m_lights[i].getTransformation() * m_model->m_lightVertices[j+1];
            c = m_lights[i].getTransformation() * m_model->m_lightVertices[j+2];
            planeNormal = QVector3D::normal(a,b,c);

            // Get the intersection point with its plane
            float lambda = -QVector3D::dotProduct(a, planeNormal);
            QVector3D intersection = m_camPosition+(-(QVector3D::dotProduct(m_camPosition, planeNormal) + lambda) / QVector3D::dotProduct(rayDir, planeNormal))*rayDir;

            // Check inside/outside of Triangle and add to list
            if (m_model->isInTriangle(a,b,c,planeNormal,intersection)) lightHitIndex = i;
        }
    }

    if(lightHitIndex != -1)
    {
        *light = lightHitIndex;
        return true;
    }

    return false;

}


//! Converts a point in openGL coordinates to widget corrdinates
QPoint AVGLWidget::QVector3DUnnormalizedToQPoint(QVector3D vector)
{
    return QPoint((vector.x() + 1) / 2 * this->width(),(-vector.y()+ 1) / 2 * this->height());
}


//! Converts a point in widget coordinates to openGL coordinates and returns a QVector3D with z=0
QVector3D AVGLWidget::QPointNormalizedToVector3D(QPoint point)
{
    return QVector3D((2.0 * point.x() / this->width() - 1.0),
                     (1.0 - 2.0 * point.y() / this->height()), 0);
}


//! Converts a point in widget coordinates to openGL coordinates and returns a QPointF
QPointF AVGLWidget::pixelPosToViewPos(const QPointF& p)
{
    return QPointF(2.0 * float(p.x()) / this->width() - 1.0,
                   1.0 - 2.0 * float(p.y()) / this->height());
}


//! Returns the current mvpMatrix
QMatrix4x4 AVGLWidget::getMvpMatrix()
{
    return m_pMatrix * m_vMatrixCurrent * m_MatrixArtefact;
}

//! Returns the current MvMatrix
QMatrix4x4 AVGLWidget::getCurrentMvMatrix()
{
    return m_vMatrix * m_MatrixArtefact;
}



double AVGLWidget::evaluateMVAngle(QQuaternion ideal)
{
    double r_angle;
    QQuaternion res;
    res=QuaternionFromMatrix(m_vMatrix*m_MatrixArtefact);
    r_angle=1-QVector4D::dotProduct(res.toVector4D(),ideal.toVector4D());
    return r_angle;
}
double AVGLWidget::evaluateMVDistance(QVector4D ideal)
{
    double r_distance;
    QVector4D res;
    res=(m_vMatrix*m_MatrixArtefact).column(3);
    r_distance=(ideal-res).toVector3D().length();
    return r_distance;
}
QQuaternion AVGLWidget::QuaternionFromMatrix(QMatrix4x4 m) {
    // Adapted from: http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
    QQuaternion q;
    q=QQuaternion(sqrt(abs( 1 + m(0,0) + m(1,1) + m(2,2)) )/ 2,
                  sqrt(abs( 1 + m(0,0) - m(1,1) - m(2,2) )) / 2,
                  sqrt(abs( 1 - m(0,0) + m(1,1) - m(2,2) )) / 2,
                  sqrt(abs( 1 - m(0,0) - m(1,1) + m(2,2) )) / 2);
    if(( q.x() * ( m(2,1) - m(1,2) ) )!=0){
        q.setX(q.x()* abs( q.x() * ( m(2,1) - m(1,2) ) )/
               ( q.x() * ( m(2,1) - m(1,2) ) ));
        q.setY(q.y()* abs( q.y() * ( m(0,2) - m(2,0) ) )/
               ( q.x() * ( m(2,1) - m(1,2) ) ));
        q.setZ(q.z()* abs( q.z() * ( m(1,0) - m(0,1) ) )/
               ( q.x() * ( m(2,1) - m(1,2) ) ));
    }
    return q;
}
