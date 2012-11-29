/* -*-c++-*- OpenSceneGraph - Copyright (C) 2009 Wang Rui
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include <osg/DeleteHandler>
#include "WtOSGWidget.h"

#include <osgViewer/Viewer>

#include <Wt/WEvent>
#include <Wt/WMatrix4x4>

#include <wtwrapper/wtwrapper.h>

#include <osgGA/TrackballManipulator>

using namespace osgWt;
using namespace Wt;
using namespace WTW;

WtOSGWidget::WtOSGWidget (WContainerWidget *parent)
:WGLWidget(parent)
{
    // TODO_
    setWidth(500);setHeight(500);
    // TODO do this everytime make current is called?
    WtWrapper::instance()->setWGLWidget(this);
}

WtOSGWidget::~WtOSGWidget()
{
    // close GraphicsWindowQt and remove the reference to us
    WtWrapper::instance()->setWGLWidget(0);
}


//////////////////////////////////////////////////////////////////////////
// The shaders, in plain text format

std::string fragmentShader_ = 
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"\n"
"varying vec3 vLightWeighting;\n"
"\n"
"void main(void) {\n"
"  vec4 matColor = vec4(0.278, 0.768, 0.353, 1.0);\n"
"  gl_FragColor = vec4(matColor.rgb * vLightWeighting, matColor.a);\n"
"}\n";

std::string vertexShader_ =
"attribute vec3 aVertexPosition;\n"
"attribute vec3 aVertexNormal;\n"
"\n"
"uniform mat4 uMVMatrix; // [M]odel[V]iew matrix\n"
"uniform mat4 uCMatrix;  // Client-side manipulated [C]amera matrix\n"
"uniform mat4 uPMatrix;  // Perspective [P]rojection matrix\n"
"uniform mat4 uNMatrix;  // [N]ormal transformation\n"
"// uNMatrix is the transpose of the inverse of uCMatrix * uMVMatrix\n"
"\n"
"varying vec3 vLightWeighting;\n"
"\n"
"void main(void) {\n"
"  // Calculate the position of this vertex\n"
"  gl_Position = uPMatrix * uCMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);\n"
"\n"
"  // Phong shading\n"
"  vec3 transformedNormal = normalize((uNMatrix * vec4(normalize(aVertexNormal), 0)).xyz);\n"
"  vec3 lightingDirection = normalize(vec3(1, 1, 1));\n"
"  float directionalLightWeighting = max(dot(transformedNormal, lightingDirection), 0.0);\n"
"  vec3 uAmbientLightColor = vec3(0.2, 0.2, 0.2);\n"
"  vec3 uDirectionalColor = vec3(0.8, 0.8, 0.8);\n"
"  vLightWeighting = uAmbientLightColor + uDirectionalColor * directionalLightWeighting;\n"
"}\n";

// Program and related variables
Wt::WGLWidget::Program shaderProgram_;
Wt::WGLWidget::AttribLocation vertexPositionAttribute_;
Wt::WGLWidget::AttribLocation vertexNormalAttribute_;
Wt::WGLWidget::UniformLocation pMatrixUniform_;
Wt::WGLWidget::UniformLocation cMatrixUniform_;
Wt::WGLWidget::UniformLocation mvMatrixUniform_;
Wt::WGLWidget::UniformLocation nMatrixUniform_;
Wt::WGLWidget::UniformLocation redValueUniform_;


// A client-side JavaScript matrix variable for model view
Wt::WGLWidget::JavaScriptMatrix4x4 jsMatrixMV_;

// A client-side JavaScript matrix variable for perspective
Wt::WGLWidget::JavaScriptMatrix4x4 jsMatrixP_;

// A client-side JavaScript matrix variable for normals
Wt::WGLWidget::JavaScriptMatrix4x4 jsMatrixN_;


// Specialization of WGLWidgeT::intializeGL()
void WtOSGWidget::initializeGL()
{

    //////////////////////////////////////////////////////////////////////////
    //// Start OSG stuff
    WtWrapper::instance()->setWGLWidget(this);
    WtWrapper::instance()->setPhase(false);

    {
        jsMatrixMV_ = createJavaScriptMatrix4();
        jsMatrixP_ = createJavaScriptMatrix4();
        jsMatrixN_ = createJavaScriptMatrix4();

        WtWrapper::instance()->setModelViewJSPlaceHolder(jsMatrixMV_.jsRef());
        WtWrapper::instance()->setProjectionJSPlaceHolder(jsMatrixP_.jsRef());
        std::string jsInvertedTransposed = jsMatrixMV_.inverted().transposed().jsRef();
        WtWrapper::instance()->setNormalJSPlaceHolder(jsInvertedTransposed);
    }

    m_viewer->frame();
    // End OSG stuff
    //////////////////////////////////////////////////////////////////////////

    {
        
        double cx = 0, cy = 0, cz = 0;
        // TG: just set up some lookat.

        osg::Vec3d from, to, up;
        from[2] = 20.0;
        up = osg::Vec3d(0,0,1);

        // TG: Read initial parameters from OSG
        std::vector<osg::Camera*> cameras;
        m_viewer->getCameras(cameras);
        if (cameras.size()>0)
        {
            // use the first camera and do not care about more cameras
            osg::Camera * c = cameras[0];
            c->getViewMatrixAsLookAt(from,to, up);
        }

        // Transform the world so that we look at the centerpoint of the scene
        Wt::WMatrix4x4 worldTransform;
        worldTransform.lookAt(
            from[0], from[1], from[2],  // camera position
            to[0], to[1], to[2],        // looking at
            up[0], up[1], up[2]);       // 'up' vector

        // We want to be able to change the camera position client-side. In
        // order to do so, the world transformation matrix must be stored in
        // a matrix that can be manipulated from JavaScript.
        
        setJavaScriptMatrix4(jsMatrixMV_, worldTransform);


        // TG: Read initial parameters from OSG manipulator
        osg::Vec3d manipulatorEye, manipulatorCenter, manipulatorUp;
        m_viewer->getCameraManipulator()->getHomePosition(manipulatorEye,manipulatorCenter, manipulatorUp);

        //// This installs a client-side mouse handler that modifies the
        //// world transformation matrix. Like WMatrix4x4::lookAt, this works
        //// by specifying a center point and an up direction; mouse movements
        //// will allow the camera to be moved around the center point.
        setClientSideLookAtHandler(jsMatrixMV_, // the name of the JS matrix
            manipulatorCenter[0],manipulatorCenter[1],manipulatorCenter[2], // the center point
            manipulatorUp[0], manipulatorUp[1], manipulatorUp[2],           // the up direction
            0.005, 0.005);                      // 'speed' factors
    }

    // TODO
    enable(DEPTH_TEST);

}


// Specialization of WGLWidgeT::paintGL()
void WtOSGWidget::paintGL()
{


    //////////////////////////////////////////////////////////////////////////
    // Start OSG stuff
    WtWrapper::instance()->setWGLWidget(this);
    WtWrapper::instance()->setPhase(true);
    //TODO: call frame
    m_viewer->frame();
    // End OSG stuff
    //////////////////////////////////////////////////////////////////////////

}

// Specialization of WGLWidgeT::resizeGL()
void WtOSGWidget::resizeGL(int width, int height)
{
    // TODO
    // adapt viewport etc.
    // Set the viewport size.
    viewport(0, 0, width, height);

    // Set projection matrix to some fixed values
    WMatrix4x4 proj;
    //proj.perspective(45, ((double)width)/height, 1, 40);
    proj.perspective(60, 1.0, 0.01, 60);

    setJavaScriptMatrix4(jsMatrixP_, proj);


}