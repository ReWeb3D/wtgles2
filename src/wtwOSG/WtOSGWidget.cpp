// disable warnings
#pragma warning (push)
#pragma warning (disable: 4251) //("vector needs  dll-interface")
#pragma warning (disable: 4275) //("non dll-interface class used as base for dll-interface")
#pragma warning (disable: 4996) // use of non safe version of std::string copy by boost
#include <wtwrapper/WtOSGWidget.h>
#include <wtwOSG/osgJS.aggregated.h>

#include <Wt/WEvent>
#include <Wt/WMatrix4x4>
#include <Wt/WGroupBox>
#include <Wt/WPushButton>
#include <Wt/WCheckBox>
#include <Wt/WApplication>

#include <boost/algorithm/string.hpp>

#include <osgViewer/Viewer>
#include <osgGA/CameraManipulator>
#include <osgDB/DatabasePager>

#include <osg/CullSettings>
#include <osg/GLObjects>

#pragma warning (pop)

#ifndef WT_DEBUG_JS
#include "js/WGLWidget.min.js"
#include "js/WtGlMatrix.min.js"
#endif


using namespace osgWt;
using namespace Wt;
using namespace WTW;

// create custom incremental compilation operation
class MyIncrementalCompileOperation : public osgUtil::IncrementalCompileOperation
{
public:

    MyIncrementalCompileOperation()
        :osgUtil::IncrementalCompileOperation()
    {
        m_compile = true;
    }

    bool m_compile;
    virtual void operator () (osg::GraphicsContext* context)
    {
        // compile always
        if (m_compile)
        {

            osg::NotifySeverity level = osg::INFO;

            //glFinish();
            //glFlush();

            double targetFrameRate = _targetFrameRate;
            double minimumTimeAvailableForGLCompileAndDeletePerFrame = _minimumTimeAvailableForGLCompileAndDeletePerFrame;

            double targetFrameTime = 1.0/targetFrameRate;

            const osg::FrameStamp* fs = context->getState()->getFrameStamp();
            double currentTime = fs ? fs->getReferenceTime() : 0.0;

            double currentElapsedFrameTime = context->getTimeSinceLastClear();

            OSG_NOTIFY(level)<<"IncrementalCompileOperation()"<<std::endl;
            OSG_NOTIFY(level)<<"    currentTime = "<<currentTime<<std::endl;
            OSG_NOTIFY(level)<<"    currentElapsedFrameTime = "<<currentElapsedFrameTime<<std::endl;

            double _flushTimeRatio(0.5);
            double _conservativeTimeRatio(0.5);

            double availableTime = std::max((targetFrameTime - currentElapsedFrameTime)*_conservativeTimeRatio,
                minimumTimeAvailableForGLCompileAndDeletePerFrame);

            double flushTime = availableTime * _flushTimeRatio;
            double compileTime = availableTime - flushTime;

#if 1
            OSG_NOTIFY(level)<<"    availableTime = "<<availableTime*1000.0<<std::endl;
            OSG_NOTIFY(level)<<"    flushTime     = "<<flushTime*1000.0<<std::endl;
            OSG_NOTIFY(level)<<"    compileTime   = "<<compileTime*1000.0<<std::endl;
#endif

            //level = osg::NOTICE;

            CompileInfo compileInfo(context, this);
            compileInfo.maxNumObjectsToCompile = _maximumNumOfObjectsToCompilePerFrame;
            compileInfo.allocatedTime = compileTime;
            compileInfo.compileAll = (_compileAllTillFrameNumber > _currentFrameNumber);

            CompileSets toCompileCopy;
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex>  toCompile_lock(_toCompileMutex);
                std::copy(_toCompile.begin(),_toCompile.end(),std::back_inserter<CompileSets>(toCompileCopy));
            }

            if (!toCompileCopy.empty())
            {
                compileSets(toCompileCopy, compileInfo);
            }

            osg::flushDeletedGLObjects(context->getState()->getContextID(), currentTime, flushTime);

            if (!toCompileCopy.empty() && compileInfo.maxNumObjectsToCompile>0)
            {
                compileInfo.allocatedTime += flushTime;

                // if any time left over from flush add on this remaining time to a second pass of compiling.
                if (compileInfo.okToCompile())
                {
                    OSG_NOTIFY(level)<<"    Passing on "<<flushTime<<" to second round of compileSets(..)"<<std::endl;
                    compileSets(toCompileCopy, compileInfo);
                }
            }

            //glFush();
            //glFinish();

            //////////////////////////////////////////////////////////////////////////
            //// taken from IncrementalCompileOperation.cpp
            //osgUtil::IncrementalCompileOperation::CompileInfo compileInfo(context, this);
            //compileInfo.maxNumObjectsToCompile = 2;//_maximumNumOfObjectsToCompilePerFrame;
            //compileInfo.allocatedTime = 1000; // does not matter if compileAll = true
            //compileInfo.compileAll = false;

            //CompileSets toCompileCopy;
            //{
            //    OpenThreads::ScopedLock<OpenThreads::Mutex>  toCompile_lock(_toCompileMutex);
            //    std::copy(_toCompile.begin(),_toCompile.end(),std::back_inserter<CompileSets>(toCompileCopy));
            //}

            //if (!toCompileCopy.empty())
            //{
            //    compileSets(toCompileCopy, compileInfo);
            //}

            ////TG: flush all osg::flushDeletedGLObjects(context->getState()->getContextID(), currentTime, flushTime);
            //osg::flushAllDeletedGLObjects(context->getState()->getContextID());


            //////////////////////////////////////////////////////////////////////////

        }else
        {
            // compile nothing

        }
    }
};


// create custom cull visitor with extended frustum
struct MyCulling : public osg::CullSettings::ClampProjectionMatrixCallback
{
    osg::ref_ptr<osgUtil::CullVisitor> m_customCulling;

    MyCulling()
    {
        m_customCulling = osgUtil::CullVisitor::create();
    }


    /** Clamp the projection float matrix to computed near and far values, use callback if it exists,
    * otherwise use default CullVisitor implementation.*/
    bool clampProjectionMatrixImplementation(osg::Matrixf& projection, double& znear, double& zfar) const
    {
        return _clampProjectionMatrix(projection, znear, zfar);
    }
    template<class matrix_type, class value_type>
    bool _clampProjectionMatrix(matrix_type& projection, value_type& znear, value_type& zfar) const
    {
        const double nearFarRatio = m_customCulling->getNearFarRatio();

        // copied from CullVisitor.cpp

        double epsilon = 1e-6;
        if (zfar<znear-epsilon)
        {
            if (zfar != -FLT_MAX || znear != FLT_MAX)
            {
                OSG_INFO<<"_clampProjectionMatrix not applied, invalid depth range, znear = "<<znear<<"  zfar = "<<zfar<<std::endl;
            }
            return false;
        }

        if (zfar<znear+epsilon)
        {
            // znear and zfar are too close together and could cause divide by zero problems
            // late on in the clamping code, so move the znear and zfar apart.
            double average = (znear+zfar)*0.5;
            znear = average-epsilon;
            zfar = average+epsilon;
            // OSG_INFO << "_clampProjectionMatrix widening znear and zfar to "<<znear<<" "<<zfar<<std::endl;
        }

        // TG: increase space between near and far
        znear *= 0.1;
        zfar *= 10.0;

        if (fabs(projection(0,3))<epsilon  && fabs(projection(1,3))<epsilon  && fabs(projection(2,3))<epsilon )
        {
            // OSG_INFO << "Orthographic matrix before clamping"<<projection<<std::endl;

            value_type delta_span = (zfar-znear)*0.02;
            if (delta_span<1.0) delta_span = 1.0;
            value_type desired_znear = znear - delta_span;
            value_type desired_zfar = zfar + delta_span;

            // assign the clamped values back to the computed values.
            znear = desired_znear;
            zfar = desired_zfar;

            projection(2,2)=static_cast<float>(-2.0f/(desired_zfar-desired_znear));
            projection(3,2)=static_cast<float>(-(desired_zfar+desired_znear)/(desired_zfar-desired_znear));

            // OSG_INFO << "Orthographic matrix after clamping "<<projection<<std::endl;
        }
        else
        {

            // OSG_INFO << "Perspective matrix before clamping"<<projection<<std::endl;

            //std::cout << "_computed_znear"<<_computed_znear<<std::endl;
            //std::cout << "_computed_zfar"<<_computed_zfar<<std::endl;

            value_type zfarPushRatio = 1.02;
            value_type znearPullRatio = 0.98;

            //znearPullRatio = 0.99;

            value_type desired_znear = znear * znearPullRatio;
            value_type desired_zfar = zfar * zfarPushRatio;

            // near plane clamping.
            double min_near_plane = zfar*nearFarRatio;
            if (desired_znear<min_near_plane) desired_znear=min_near_plane;

            // assign the clamped values back to the computed values.
            znear = desired_znear;
            zfar = desired_zfar;

            value_type trans_near_plane = (-desired_znear*projection(2,2)+projection(3,2))/(-desired_znear*projection(2,3)+projection(3,3));
            value_type trans_far_plane = (-desired_zfar*projection(2,2)+projection(3,2))/(-desired_zfar*projection(2,3)+projection(3,3));

            value_type ratio = fabs(2.0/(trans_near_plane-trans_far_plane));
            value_type center = -(trans_near_plane+trans_far_plane)/2.0;

            projection.postMult(osg::Matrix(1.0f,0.0f,0.0f,0.0f,
                0.0f,1.0f,0.0f,0.0f,
                0.0f,0.0f,ratio,0.0f,
                0.0f,0.0f,center*ratio,1.0f));

            // OSG_INFO << "Perspective matrix after clamping"<<projection<<std::endl;
        }
        return true;
    }

    bool clampProjectionMatrixImplementation(osg::Matrixd& projection, double& znear, double& zfar) const
    {
        return _clampProjectionMatrix(projection, znear, zfar);
    }
};


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


// A client-side JavaScript matrix variable for view transformation
Wt::WGLWidget::JavaScriptMatrix4x4 jsMatrixV_;
Wt::WGLWidget::JavaScriptMatrix4x4 jsMatrixVInv_;

// A client-side JavaScript matrix variable for perspective
Wt::WGLWidget::JavaScriptMatrix4x4 jsMatrixP_;

// A client-side JavaScript matrix variable for normals
Wt::WGLWidget::JavaScriptMatrix4x4 jsMatrixN_;

WtOSGWidget::WtOSGWidget (const unsigned int & width, const unsigned int & height, WContainerWidget *parent)
:WGLWidget(parent), jMVMatrixSignal_(NULL), m_frameUpdateTimeOut(500)
{
    jMVMatrixSignal_ = new Wt::JSignal<std::string>(this,"MVMatrixSlot");
    // Connect the signal with the readback function.
    jMVMatrixSignal_->connect(this,&WtOSGWidget::readCallbackMatrix);

    // setup event handling
    //this->globalKeyWentUp().connect(&this->keyWentUp);

    // TG: there are two ways to capture events: 
    // global events are caught no matter where the focus is:
    //WApplication::globalKeyWentUp().connect(this, &WtOSGWidget::keyUp);
    //WApplication::globalKeyWentDown().connect(this, &WtOSGWidget::keyDown);
    
    // local events are only caught, when the focus is at the canvas (using tab)
    // for it, I need to set the tabindex:
    this->setAttributeValue("tabindex", "0"); 
    //this->keyWentUp().connect(this, &WtOSGWidget::keyUp);
    //this->keyWentDown().connect(this, &WtOSGWidget::keyDown);
    this->keyPressed().connect(this, &WtOSGWidget::keyDown);

    setWidth(width);setHeight(height);
    // TODO do this everytime make current is called?
    WtWrapper::instance()->setWGLWidget(this);

    WGroupBox * group = new WGroupBox("Debug settings", parent);    

    m_checkBox = new WCheckBox("Enable automatic updates",group);
    m_checkBox ->changed().connect(this,&WtOSGWidget::callbackMatrix);
    m_checkBox ->setChecked(true);


    // Initialize the server-side ModelView matrix:
    mvMatrix.fill(0.0);

    m_ico = new MyIncrementalCompileOperation();

}

// Specialization of WGLWidgeT::intializeGL()
void WtOSGWidget::initializeGL()
{
    m_viewer->setIncrementalCompileOperation(m_ico);
    m_viewer->getCamera()->setClampProjectionMatrixCallback(new MyCulling());

    m_ico->m_compile = true;

    //////////////////////////////////////////////////////////////////////////
    //// Start OSG stuff
    WtWrapper::instance()->setWGLWidget(this);
    WtWrapper::instance()->setPhase(WtWrapper::INIT);

    // set database pager
    if (1){

        //m_viewer->getIncrementalCompileOperation();
        //s_ico = 
        //    new osgUtil::IncrementalCompileOperation();
        
        //m_viewer->setIncrementalCompileOperation(s_ico);

        //osgViewer::ViewerBase::Scenes scenes;
        //m_viewer->getScenes(scenes);
        //for(osgViewer::ViewerBase::Scenes::iterator itr = scenes.begin();
        //    itr != scenes.end();
        //    ++itr)
        //{
        //    osgViewer::Scene* scene = *itr;
        //    osgDB::DatabasePager* dp = scene->getDatabasePager();
        //    if (dp )//&& dp->isRunning())
        //    {
        //        // set a very low framerate to make sure everything was loaded
        //        //dp->getIncrementalCompileOperation()->setTargetFrameRate(1);
        //        dp->setUpThreads(4,1); // increase number of frames to read data
        //    }

        //    //setTargetFrameRate	(	double 	tfr	 )
        //    //    while( (osg::Timer::instance()->delta_m(a, osg::Timer::instance()->tick()) < m_waitLimit) && (dbp->getRequestsInProgress() || (dbp->getDataToCompileListSize() > 0)) ) {
        //    //        updateLoops++;
        //    //        m_viewer->updateTraversal();
        //    //        m_viewer->renderingTraversals();
        //    //    }
        //}
    }
    

    
    // TG: WebGL seems to interpret the image pixels top bottom vs OpenGL which interprets it bottom top
    this->pixelStorei(UNPACK_FLIP_Y_WEBGL,true);

    // WebGL pre-multiplies the ALPHA, which is not what we want http://games.greggman.com/game/webgl-and-alpha/
    this->pixelStorei(UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
    
    {
        jsMatrixV_ = createJavaScriptMatrix4();

        jsMatrixVInv_ = createJavaScriptMatrix4();

        jsMatrixP_ = createJavaScriptMatrix4();
        jsMatrixN_ = createJavaScriptMatrix4();
    }

    //s_ico->compileAllForNextFrame(2);
    m_viewer->frame();
    // End OSG stuff
    //////////////////////////////////////////////////////////////////////////

    // set current view parameters for initialization
    updateView((int)width().toPixels(), (int)height().toPixels());
    
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
        Wt::WMatrix4x4 viewTransform;
        viewTransform.lookAt(
            from[0]+0, from[1]+0, from[2]+0,  // camera position
            to[0], to[1], to[2],        // looking at
            up[0], up[1], up[2]);       // 'up' vector
            /*from[0], from[1], from[2],  // camera position
            to[0], to[1], to[2],        // looking at
            up[0], up[1], up[2]);       // 'up' vector*/

        // We want to be able to change the camera position client-side. In
        // order to do so, the world transformation matrix must be stored in
        // a matrix that can be manipulated from JavaScript.
        
        setJavaScriptMatrix4(jsMatrixV_, viewTransform);
        setJavaScriptMatrix4(jsMatrixVInv_, viewTransform.inverted());

        // TG: TEST
        
        Wt::WMatrix4x4 viewTransformInv = viewTransform.inverted();
        // this is differnt then the lookat of wt
        osg::Matrixd m = osg::Matrix::lookAt(from, to, up);
        osg::Matrixd mInv = osg::Matrix::inverse(m);


        // TG: Read initial parameters from OSG manipulator
        osg::Vec3d manipulatorEye, manipulatorCenter, manipulatorUp;
        m_viewer->getCameraManipulator()->getHomePosition(manipulatorEye,manipulatorCenter, manipulatorUp);

        // This installs a client-side mouse handler that modifies the
        // world transformation matrix. Like WMatrix4x4::lookAt, this works
        // by specifying a center point and an up direction; mouse movements
        // will allow the camera to be moved around the center point.
        //setClientSideLookAtHandler(jsMatrixV_, // the name of the JS matrix
        //    manipulatorCenter[0],manipulatorCenter[1],manipulatorCenter[2], // the center point
        //    manipulatorUp[0], manipulatorUp[1], manipulatorUp[2],           // the up direction
        //    0.005, 0.005);                      // 'speed' factors

        // TG: alternative using osgJS manipulator
        setClientSideLookAtHandler(jsMatrixV_, // the name of the JS matrix
            manipulatorEye[0],manipulatorEye[1],manipulatorEye[2],          // the eye point
            manipulatorCenter[0],manipulatorCenter[1],manipulatorCenter[2], // the center point
            manipulatorUp[0], manipulatorUp[1], manipulatorUp[2]           // the up direction
            );



        std::string jsMatrixStr;
        if (1) {
            // apply client side animation
            jsMatrixStr += std::string(WT_CLASS) + ".glMatrix.mat4.multiply(" + jsMatrixV_.jsRef() + ",";
            jsMatrixStr += jsMatrixVInv_.jsRef() + ",";
            jsMatrixStr += std::string(WT_CLASS) + ".glMatrix.mat4.create())";
        }else
        {
            // just apply identity matrix
            Wt::WMatrix4x4 identityMatrix;
            
            // We want to be able to change the camera position client-side. In
            // order to do so, the world transformation matrix must be stored in
            // a matrix that can be manipulated from JavaScript.

            JavaScriptMatrix4x4 identityMatrixJS = createJavaScriptMatrix4();
            setJavaScriptMatrix4(identityMatrixJS, identityMatrix);

            jsMatrixStr = identityMatrixJS.jsRef();
        }
        
        JavaScriptMatrix4x4 viewXviewOsgInverse(jsMatrixStr);

        const std::string csViewUniformName = "client_ViewMatrix";
        WtWrapper::instance()->initClientSideViewTransformUniform(
            csViewUniformName,
            //(jsMatrixV_ *jsMatrixVInv_).jsRef());
            (viewXviewOsgInverse).jsRef());


    }
    
    const bool EnableTimer = true;
    if (EnableTimer)
    {
        std::string enableAnimationString;
        
        const std::string clientSideTimeVariable = "clientSideTimeVariable";

        // add an update callback that is called every 1/60th s
        enableAnimationString += "// Definition of WTVicomGLAnim function \n";
        enableAnimationString += "ctx.WTVicomGLAnim=(function(){\n";
        enableAnimationString += "  if (!window.requestAnimationFrame) {\n";
        enableAnimationString += "      window.requestAnimationFrame = (function() {\n";
        enableAnimationString += "          return window.requestAnimationFrame ||\n";
        enableAnimationString += "          window.webkitRequestAnimationFrame ||\n";
        enableAnimationString += "          window.mozRequestAnimationFrame ||\n";
        enableAnimationString += "          window.oRequestAnimationFrame ||\n";
        enableAnimationString += "          function(/* function FrameRequestCallback */ callback, /* DOMElement Element */ element) {\n";
        enableAnimationString += "              window.setTimeout(callback, 1000/60);\n";  // timeout : 1/60 s
        enableAnimationString += "          };\n";
        enableAnimationString += "      })();}\n";

        // query the context
        enableAnimationString += "  var obj = (function(){\n";
        enableAnimationString += std::string("  var r = ") + jsRef() + ";\n";
        enableAnimationString += "  var o = r ? jQuery.data(r,'obj') : null;\n";
        enableAnimationString += "      return o ? o : {ctx: null};\n";
        enableAnimationString += "  })();";
        enableAnimationString += "  var ctx=obj.ctx;\n";
            
        // add a custom timer variable to the context
        enableAnimationString += "  if (typeof(ctx." + clientSideTimeVariable + ") =='undefined')\n";
        enableAnimationString +="{\n  ctx." + clientSideTimeVariable +" = 0.0;\n ctx.clientSideStartTime = new Date().getTime();}\n";
        enableAnimationString += "  else{ \n var currentTime = new Date().getTime(); \n ctx." + clientSideTimeVariable +" = (currentTime-ctx.clientSideStartTime)/1000.0;";

        enableAnimationString += "  obj.paintGL();}\n";
        enableAnimationString += "  window.requestAnimationFrame(ctx.WTVicomGLAnim);\n";
        enableAnimationString += "\n})\n\n";
        
        // execute function for the first time 
        enableAnimationString += "ctx.WTVicomGLAnim();";


        this->injectJS(enableAnimationString);

        const std::string osgFrameTimeUniformName = "osg_FrameTime";
        const std::string test =  std::string("ctx.") + clientSideTimeVariable;
        
        WtWrapper::instance()->bindClientVariableToUniform1f(
            test,
            osgFrameTimeUniformName);
    }

    const bool EnableAutoUpdates = true;
    if (EnableAutoUpdates)
    {
        std::string updateViewMatrixString;
        // add an function that sends the current modelview matrix to the server
        updateViewMatrixString += "// Definition of triggerMatrixUpdate function \n";
        updateViewMatrixString += "  if(typeof(ctx.triggerMatrixUpdate) =='undefined'){\n";
        updateViewMatrixString += "      ctx.triggerMatrixUpdate=(function(){\n";
        //updateViewMatrixString += "         debugger;";
        updateViewMatrixString += "          var m = " + jsMatrixV_.jsRef() + ";\n";
        updateViewMatrixString += "          var s = '';\n";
        updateViewMatrixString += "          for (var i =0;i<15;i++){ s += m[i]+','; }\n";
        updateViewMatrixString += "          s += m[15];\n";
        //updateViewMatrixString += "          console.log(\"Update...\");\n";
        updateViewMatrixString += "          " + jMVMatrixSignal_->createCall("s");
        updateViewMatrixString += "\n      });\n";
        updateViewMatrixString += "  }";
        this->injectJS(updateViewMatrixString);
    }

    // enable regular updates based on the checkbox setting
    if (m_checkBox->isChecked())
    {
        std::string regularUpdatesString;
        //regularUpdatesString+="debugger;";
        regularUpdatesString += "setTimeout(ctx.triggerMatrixUpdate," + boost::lexical_cast<std::string>(m_frameUpdateTimeOut) +");";
        injectJS(regularUpdatesString);
    }

}


// TG
void WtOSGWidget::setClientSideLookAtHandler(const JavaScriptMatrix4x4 &m,
                                           double fromX, double fromY, double fromZ,
                                           double toX, double toY, double toZ,
                                           double upX, double upY, double upZ)
{
    js_ <<
        "obj.setLookToFromUpParams("
        << m.jsRef()
        << ",[" << fromX << "," << fromY << "," << fromZ << "]"
        << ",[" << toX << "," << toY << "," << toZ << "]"
        << ",[" << upX << "," << upY << "," << upZ << "]"
        << ");";
}


// Specialization of WGLWidgeT::paintGL()
void WtOSGWidget::paintGL()
{
    m_ico->m_compile = false;
    // TODO inject javascript
    
    //////////////////////////////////////////////////////////////////////////
    // Start OSG stuff
    WtWrapper::instance()->setWGLWidget(this);
    WtWrapper::instance()->setPhase(WtWrapper::PAINT);

    //TODO: call frame
    m_viewer->frame();
    // End OSG stuff
    //////////////////////////////////////////////////////////////////////////
}

void WtOSGWidget::updateView(const unsigned int & width, const unsigned int & height)
{
    // TODO
    // adapt viewport etc.
    // Set the viewport size.
    m_viewer->getCamera()->setViewport(0,0,width, height);

    viewport(0, 0, width, height);

    // Set projection matrix to some fixed values
    WMatrix4x4 proj;

    // TODO: support orthographic perspective
    double near, far,fov, ratio;
    this->m_viewer->getCamera()->getProjectionMatrixAsPerspective(
        fov,
        ratio,
        near,
        far
        );

    proj.perspective(fov, ((double)width)/height, near, far);


    setJavaScriptMatrix4(jsMatrixP_, proj);
}

// Specialization of WGLWidgeT::resizeGL()
void WtOSGWidget::resizeGL(int width, int height)
{
    updateView(width, height);
}

void WtOSGWidget::callbackMatrix(void){
    std::stringstream j_;
    j_.str("");
    

    if (m_checkBox->isChecked())
    {
        j_ << "var obj=" << "(function(){"
            "var r = " << this->jsRef() << ";"
            <<"var o = r ? jQuery.data(r,'obj') : null;"
            <<"return o ? o : {ctx: null};"
            "})()" << ";\n"
            << "var ctx=obj.ctx;if (!ctx) return;\n"
            << "ctx.triggerMatrixUpdate();\n";
    }else
    {

    }
    
    doJavaScript(j_.str());
}

void WtOSGWidget::updateGL(void){

    m_ico->m_compile = true;

    osg::Vec3d from, to, up;

    // -Use the first camera and do not care about more cameras
    // -Also use the transposed version of the Wt matrix because
    // they are stored in the transposed way.
    osg::Matrix m(&mvMatrix.transposed().constData()[0]);
    
    m.getLookAt(from, to, up);
    //c->setViewMatrix(m);
    // TG: Read initial parameters from OSG manipulator
    
    m_viewer->getCameraManipulator()->setByInverseMatrix(m);

    // Transform the world so that we look at the centerpoint of the scene
    Wt::WMatrix4x4 viewTransform;
    viewTransform.lookAt(
        from[0]+0, from[1]+0, from[2]+0,  // camera position
        to[0], to[1], to[2],        // looking at
        up[0], up[1], up[2]);       // 'up' vector

    // We want to be able to change the camera position client-side. In
    // order to do so, the world transformation matrix must be stored in
    // a matrix that can be manipulated from JavaScript.
    
    setJavaScriptMatrix4(jsMatrixV_, viewTransform);
    setJavaScriptMatrix4(jsMatrixVInv_, viewTransform.inverted());

    WtWrapper::instance()->setWGLWidget(this);
    WtWrapper::instance()->setPhase(WtWrapper::UPDATE);

    //s_ico->compileAllForNextFrame(2);

    m_viewer->frame();

    // TG Test regular updates
    if (m_checkBox->isChecked())
    {
        std::string regularUpdatesString;

        regularUpdatesString += "setTimeout(ctx.triggerMatrixUpdate," + boost::lexical_cast<std::string>(m_frameUpdateTimeOut) +");";
        injectJS(regularUpdatesString);
    }
    //    // TODO: refactor: have a global callback method instead of always redefining this
    //    std::stringstream j_;
    //    j_.str("");

    //    // send current matrix to the server
    //    j_ << "var obj=" << "(function(){"
    //        "var r = " << this->jsRef() << ";"
    //        <<"var o = r ? jQuery.data(r,'obj') : null;"
    //        <<"return o ? o : {ctx: null};"
    //        "})()" << ";\n"
    //        << "var ctx=obj.ctx;if (!ctx) return;\n"
    //        "var m = " << jsMatrixV_.jsRef() << ";\n"
    //        "var s = '';\n"
    //        "for (var i =0;i<15;i++){ s += m[i]+','; }\n"
    //        "s += m[15];\n"
    //        << jMVMatrixSignal_->createCall("s") << "";


    //    // add an update callback that is called every 1/60th s
    //    regularUpdatesString += "if(typeof(ctx.WTTriggerUpdate) =='undefined'){\n";
    //    regularUpdatesString += "   ctx.WTTriggerUpdate=(function(){\n";
    //    regularUpdatesString += "       window.setTimeout(updateGL, 1000);"
    //    regularUpdatesString += "       if (!window.requestAnimationFrame) {\n";
    //    regularUpdatesString += "           window.requestAnimationFrame = (function() {\n";
    //    regularUpdatesString += "               return window.requestAnimationFrame ||\n";
    //    regularUpdatesString += "               window.webkitRequestAnimationFrame ||\n";
    //    regularUpdatesString += "               window.mozRequestAnimationFrame ||\n";
    //    regularUpdatesString += "               window.oRequestAnimationFrame ||\n";
    //    regularUpdatesString += "               function(/* function FrameRequestCallback */ callback, /* DOMElement Element */ element) {\n";
    //    regularUpdatesString += "                   window.setTimeout(callback, 1000);\n";  // timeout : 1 s
    //    regularUpdatesString += "               };\n";
    //    regularUpdatesString += "           })();\n";
    //    regularUpdatesString += "       }\n";
    //    regularUpdatesString += "       window.requestAnimationFrame(ctx.updateGL);\n";

    //    regularUpdatesString += "   })();\n";
    //    regularUpdatesString += "}\n";

    //    // call up the function with update
    //    Wt::JSignal triggerUpdate;
    //    triggerUpdate.connect(this, updateGL);
    //    triggerUpdate.emit()
    //    

    //    //// query the context
    //    //regularUpdatesString += "\t var obj = (function(){\n";
    //    //regularUpdatesString += std::string("\tvar r = ") + jsRef() + ";\n";
    //    //regularUpdatesString += "\tvar o = r ? jQuery.data(r,'obj') : null;\n";
    //    //regularUpdatesString += "\t\treturn o ? o : {ctx: null};\n";
    //    //regularUpdatesString += "})(); \tvar ctx=obj.ctx;\n";

    //    // add a custom timer variable to the context
    //    regularUpdatesString += "  window.requestAnimationFrame(ctx.updateGL);\n";
    //    regularUpdatesString += "\n})\n/**Vicomtech_Client_Side_Code*/\n";

    //    // execute function for the first time 
    //    regularUpdatesString += "ctx.WTVicomGLAnim();";
    //}

}

void WtOSGWidget::readCallbackMatrix(std::string a){
    // Split the sent string into the 16 numbers of the matrix.
    std::vector<std::string> strings;
    std::vector<float> values;
    boost::split(strings,a,boost::is_any_of(","));
    for(int i = 0;i<4;i++){
        for(int j = 0; j < 4; j++){
            mvMatrix(i,j) = boost::lexical_cast<double,std::string>(strings.front());
            strings.erase(strings.begin());
        }
    }
    mvMatrix = mvMatrix.transposed();

    // Trigger an update of the GL on the server, using UpdateGL.
    this->repaintGL(UPDATE_GL);

    // Trigger an update of the GL part on the client, rewriting the client's 
    // paintGL function 
    this ->repaintGL(PAINT_GL);
}

//////////////////////////////////////////////////////////////////////////
// event handling
//////////////////////////////////////////////////////////////////////////

void WtOSGWidget::keyUp(const Wt::WKeyEvent& event)
{
    // charcode is containing the char (upper / lower case)
    // but only in key pressed events
    m_viewer->getEventQueue()->keyRelease( event.charCode() );

    this->repaintGL(UPDATE_GL);
    this ->repaintGL(PAINT_GL);
}

void WtOSGWidget::keyDown(const Wt::WKeyEvent& event)
{

    // charcode is containing the char (upper / lower case)
    // but only in key pressed events
    //std::cout << "Key code (deprecated) " << event.keyCode()<< std::endl;
    //std::cout << "char code " << event.charCode()<< std::endl;
    //std::cout << "Key " << event.key() << std::endl;

    m_viewer->getEventQueue()->keyPress( event.charCode() );

    this->repaintGL(UPDATE_GL);
    this ->repaintGL(PAINT_GL);
}

// override to load custom javascript
void WtOSGWidget::render(WFlags<RenderFlag> flags)
{
    // copied from WGLWidget.C
    if (flags & RenderFull)
        defineJavaScript();
    WInteractWidget::render(flags);
}


// overwritten to add additional javascript library and load custom WtGlMatrix
void WtOSGWidget::defineJavaScript()
{
    WApplication *app = WApplication::instance();

    LOAD_JAVASCRIPT(app, "js/WtGlMatrix.js", "glMatrix", wtjs2);
    LOAD_JAVASCRIPT(app, "js/WGLWidget.js", "WGLWidget", wtjs1);
        
    app->doJavaScript(OSGJS_AGGREGATED_STRING, false);
    
    // ...or load all single resources for debugging
    // the following js files are loaded on startup and need to be found relative to the working dir
    // load the minified and combined version
    //app->require("js/osg.js");
    //app->require("js/Matrix.js");
    //app->require("js/Vec3.js");
    //app->require("js/osgGA.js");
    //app->require("js/Manipulator.js");
    //app->require("js/FirstPersonManipulator.js");
    //app->require("js/OrbitManipulator.js");
}

WtOSGWidget::~WtOSGWidget()
{
    // close GraphicsWindowQt and remove the reference to us
    WtWrapper::instance()->setWGLWidget(0);
}
