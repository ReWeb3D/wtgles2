#include <wtwrapper/WtOSGWidget.h>
#include <wtwrapper/WtWrapper.h>

#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WImage>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WTextArea>
#include <Wt/WEnvironment>

#include <osg/TexEnvCombine>
#include <osg/TexEnv>
#include <osg/Vec3>
#include <osg/io_utils>
#include <osg/PositionAttitudeTransform>

#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>

#include <osgUtil/Optimizer>
#include <osgUtil/ShaderGen>
#include <osgUtil/Simplifier>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgGA/StateSetManipulator>

#include <iostream>

#include <Wt/WGLWidget>
#include <Wt/WMatrix4x4>

using namespace Wt;

// This application is an example for the generic model viewer using ReWeb3D.
// Per default, it loads a cow model "CowChecker.osgb" and applies a default 
// shader.
//
// If you use the built-in server (and not the FCGI connection on an apache)
// you start it by running
//  osgViewerWt.wt --http-address=0.0.0.0 --http-port=8080 --deploy-path=/viewer --docroot=.
// After starting, you can navigate with your WebGL-browser to 
//  http://localhost:8080/viewer
// and should see the cow model. If the model cannot be found, two quads
// with the osg logo are displayed.
//
// Other parameters can be added in the url using ? and separated by
// &, for example http://localhost:8080/viewer?dataset=glider.osg&debug
// - dataset=<filename> : loads filename
// - debug : inserts error checks after each webgl call 
//      (if Wt/WGLWidget has been compiled with #define WT_WGLWIDGET_DEBUG 1)
// - updateTime=<time in milliseconds> : set the update interval (default 500)
// - buffers=<string|binary|aggregatedBinary> : set the transfer method of geometry
//      buffers to string (embedded in js), binary (one file per buffer) or 
//      aggregated (default, files of 256KB size)
// 
// In the browser, lighting can be toggled ('l') and texture boundaries ('t').

class OSGWTApp : public WApplication
{
public:
    OSGWTApp(const WEnvironment& env)
        :WApplication(env)
    {
    };
};


std::string defaultFragmentShader = 
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"\n"
"uniform sampler2D osg_Sampler0;"
"uniform bool showTiles; \n"
"varying vec3 vLightWeighting;\n"
"varying vec4 vFrontColor;\n"
"varying vec2 vTexCoord0;\n"
"\n"
"void main(void) {\n"
"  vec4 matColor = vec4(0.878, 0.768, 0.353, 1.0);\n"
"  vec4 texColor = texture2D(osg_Sampler0, vec2(vTexCoord0.s, vTexCoord0.t));"
"  vec4 finalColor;\n"
" if (vTexCoord0.s<0.00001 && vTexCoord0.t<0.00001 && texColor != vec4(0.0)){ "
"   finalColor = vec4(vFrontColor.rgb * vLightWeighting, vFrontColor.a); \n"
" }else{ \n"
"	finalColor = vec4(vec3(texColor.rgb * vLightWeighting) ,texColor.a);\n"
" }\n"
// TG: debug: show tex boundaries
//" finalColor=mix(vec4(vTexCoord0.st, finalColor.b, finalColor.a), finalColor, 0.8);\n"
" const float lineBoundary = 0.95;\n"
" float maxEdge = max(vTexCoord0.s,vTexCoord0.t);\n"
" if (showTiles && maxEdge>lineBoundary) finalColor=mix(finalColor, vec4(1.0,0.2,0.2,1.0), 0.2*smoothstep(lineBoundary,1.0,maxEdge));\n"
"  gl_FragColor = finalColor;\n"
"}\n";

std::string defaultVertexShader =
"attribute vec3 osg_Vertex;\n"
"attribute vec3 osg_Normal;\n"
"attribute vec4 osg_Color;\n"
"attribute vec2 osg_MultiTexCoord0;\n"
"\n"

"uniform mat4 osg_ModelViewMatrix; // [M]odel[V]iew matrix\n"
"uniform mat4 client_ViewMatrix; \n"
"uniform bool enableLighting; \n"
"uniform mat4 osg_ProjectionMatrix;  // Perspective [P]rojection matrix\n"
"uniform mat3 osg_NormalMatrix;  // [N]ormal transformation\n"
"// uNMatrix is the transpose of the inverse of uCMatrix * uMVMatrix\n"
"\n"
"varying vec3 vLightWeighting;\n"
"varying vec4 vFrontColor;\n"
"varying vec2 vTexCoord0;\n"
"\n"
"void main(void) {\n"
// TG: the vertex is transformed in addition with the client view matrix, 
// to update the view independent and directly from the client
"  gl_Position = osg_ProjectionMatrix * client_ViewMatrix * osg_ModelViewMatrix * vec4(osg_Vertex, 1.0);\n"
"\n"
"  // Phong shading\n"
// TG: this is a poor man's normal matrix, which will only work in case there is no nonuniform scale transform
// using the osg_NormalMatrix we get the "right" matrix, but it is only updated when the server resends it
//"  vec3 transformedNormal = normalize(osg_NormalMatrix * normalize(osg_Normal));\n"
"  vec3 transformedNormal = normalize(client_ViewMatrix*osg_ModelViewMatrix * vec4(osg_Normal, 0.0)).xyz;\n"

"  vec3 lightingDirection = -1.0*normalize(vec3(-1, -1, -1));\n"
"  float directionalLightWeighting = max(dot(transformedNormal, lightingDirection), 0.0);\n"
"  vec3 uAmbientLightColor = vec3(0.2, 0.2, 0.2);\n"
"  vec3 uDirectionalColor = vec3(0.8, 0.8, 0.8);\n"
"  vLightWeighting = uAmbientLightColor + uDirectionalColor * directionalLightWeighting;\n"
"  if (enableLighting) vLightWeighting = vec3(1.0);\n"
"  vFrontColor = osg_Color;\n"
"  vTexCoord0 = osg_MultiTexCoord0;\n"
"}\n";

 osg::ref_ptr<osg::Uniform> s_showTiles;
 osg::ref_ptr<osg::Uniform> s_enableLighting;

 osgWt::WtOSGWidget * s_viewWidget = 0;

// testQuad is created if no model was loaded
osg::ref_ptr<osg::Geode> createTestQuad()
{
    osg::ref_ptr<osg::Geode> retGeode = new osg::Geode;
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
    
    // create & set vertices
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array();
    vertices->push_back(osg::Vec3(-0.5,-0.5, -0.2));
    vertices->push_back(osg::Vec3(0.5,-0.5, 0.0));
    vertices->push_back(osg::Vec3(0.5,0.5, 0.2));
    vertices->push_back(osg::Vec3(-0.5,0.5, 0.4));

    geom->setVertexArray(vertices);

    // create & set normals
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array();
    normals->push_back(osg::Vec3(0.0,0.0, 1.0));
    normals->push_back(osg::Vec3(0.0,0.0, 1.0));
    normals->push_back(osg::Vec3(0.0,0.0, 1.0));
    normals->push_back(osg::Vec3(0.0,0.0, 1.0));

    geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    geom->setNormalArray(normals);

    // create & set colors
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array();
    colors->push_back(osg::Vec4(1.0,0.0,0.0, 1.0));
    colors->push_back(osg::Vec4(1.0,1.0,0.0, 1.0));
    colors->push_back(osg::Vec4(0.0,1.0,0.0, 1.0));
    colors->push_back(osg::Vec4(0.0,0.0,1.0, 1.0));

    geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    geom->setColorArray(colors);

    // create & set texture coordinates
    osg::ref_ptr<osg::Vec2Array> texCoords = new osg::Vec2Array();
    texCoords->push_back(osg::Vec2(0.0,0.0));
    texCoords->push_back(osg::Vec2(1.0,0.0));
    texCoords->push_back(osg::Vec2(1.0,1.0));
    texCoords->push_back(osg::Vec2(0.0,1.0));

    geom->setTexCoordArray(0, texCoords);

    // create & set indices
    osg::ref_ptr<osg::DrawElementsUShort> indices = 
        new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP,4);
    indices->push_back(3);
    indices->push_back(0);
    indices->push_back(2);
    indices->push_back(1);

    geom->addPrimitiveSet(indices); 

    // load & set texture
    std::string textureFileName = "datasets/osg64.png";   // found in OSG example data 
    osg::ref_ptr<osg::Image> img = osgDB::readImageFile(textureFileName);

    if (img)
    {
        osg::ref_ptr<osg::StateSet> stateSet = geom->getOrCreateStateSet();
        
        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(img);
        stateSet->setTextureAttribute(0, texture);
        stateSet->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);

        // add sampler uniform
        osg::ref_ptr<osg::Uniform> uSampler0 = new osg::Uniform(osg::Uniform::SAMPLER_2D, "osg_Sampler0");
        uSampler0->set(0);
        stateSet->addUniform(uSampler0, osg::StateAttribute::ON);
    }

    // for fast path enable VBOs
    geom->setUseVertexBufferObjects(true);

    retGeode->addDrawable(geom);

    return retGeode;
}

void keyWentUp(const WKeyEvent& event)
{
    static bool lightOn = true;
    static bool textureBoundsOn = false;

    switch (event.key())
    {
    case Key_L:
        lightOn = !lightOn;
        s_enableLighting->set(lightOn);
        std::cout << "toggle lighting" << std::endl;
    	break;
    case Key_T:
        textureBoundsOn = !textureBoundsOn;
        s_showTiles->set(textureBoundsOn);
        std::cout << "toggle tiles" << std::endl;
        break;
    default:
        ;
    }

    s_viewWidget->repaintGL(WGLWidget::PAINT_GL);
}

// createApplication is the former main function. It is called by the
// Wt server whenever a new query arrives.
//
// Note that the internal server is not working (well) for multiple
// parallel queries, as there is just one static wtwrapper object.
WApplication *createApplication(const WEnvironment& env)
{
    OSGWTApp * w = new OSGWTApp (env);

    // set up some html UI elements
    w->setTitle("osgViewer in WebGL");

    w->root()->addWidget(new WText("Below you see the OSG scene rendered using WebGL."));
    w->root()->addWidget(new WBreak());
    w->root()->addWidget(new WText("<b>'l'</b> toggles lighting, <b>'t'</b> toggles showing of texture boundaries"));
    w->root()->addWidget(new WBreak());
    WText * fileNameLabel = new WText(w->root());
    unsigned int screenWidth = 1024, screenHeight = 768;
    
    // catch key events and send them to keyWentUp
    w->globalKeyWentUp().connect(&keyWentUp);
    
    // demo model
    std::string modelFileName = "CowChecker.osgb";     

    // use datasets in search path
    osgDB::Registry::instance()->getDataFilePathList().push_front("datasets");

    //////////////////////////////////////////////////////////////////////////
    // evaluate url parameters
    // check if a name was given in the URL
    Http::ParameterValues pv = env.getParameterValues("dataset");
    if (!pv.empty())
    {
        const std::string datasetDir = "datasets/";
        modelFileName = datasetDir + pv.at(0);
    }

    {
        Http::ParameterValues pv2 = env.getParameterValues("debug");    
        Wt::WGLWidget::enableClientErrorChecks(!pv2.empty());
    }

    {
        Http::ParameterValues pv2 = env.getParameterValues("buffer"); 
        if (!pv2.empty())
        {
            if (pv2.at(0) == "string")
            {
                WTW::WtWrapper::instance()->m_bufferTransferImplementation = 
                    WTW::WtWrapper::STRING_BUFFER;
            }else if (pv2.at(0) == "binary")
            {
                WTW::WtWrapper::instance()->m_bufferTransferImplementation = 
                    WTW::WtWrapper::BINARY_BUFFER;
            }else if(pv2.at(0) == "aggregatedBinary")
            {
                WTW::WtWrapper::instance()->m_bufferTransferImplementation = 
                    WTW::WtWrapper::AGGREGATED_BINARY_BUFFER;
            }else
            {
                // default is aggregated binary
                WTW::WtWrapper::instance()->m_bufferTransferImplementation = 
                    WTW::WtWrapper::AGGREGATED_BINARY_BUFFER;
            }
        }
    }

    unsigned int updateTimeIntervalInMilliseconds = 500;
    {
        Http::ParameterValues pv2 = env.getParameterValues("updateTime");    
        if (!pv2.empty())
        {
            updateTimeIntervalInMilliseconds = boost::lexical_cast<unsigned int>(pv2.at(0));
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // construct scene

    osg::ref_ptr<osg::Node> n = 0;
    modelFileName = osgDB::Registry::instance()->findDataFile(modelFileName, NULL, osgDB::CASE_INSENSITIVE);
    n = osgDB::readNodeFile(modelFileName);

    if (n == NULL)
    {
        std::cout << "model not found: " << modelFileName << std::endl;
        std::cout << "creating default quad. "<< std::endl;
        fileNameLabel->setText(modelFileName + " not found, showing default quad");
        
        n = createTestQuad();
    }else
    {
        fileNameLabel->setText(modelFileName);
    }

    // use vertex buffer objects only (no client side arrays are supported by webgl)
    osgUtil::GLObjectsVisitor glov(osgUtil::GLObjectsVisitor::SWITCH_ON_VERTEX_BUFFER_OBJECTS);
    n->accept(glov);

    // add default shader to OSG scenegraph
    {
        osg::ref_ptr<osg::Shader> vShader = new osg::Shader(
            osg::Shader::VERTEX,
            defaultVertexShader);

        osg::ref_ptr<osg::Shader> fShader = new osg::Shader(
            osg::Shader::FRAGMENT,
            defaultFragmentShader);

        osg::ref_ptr<osg::Program> program = new osg::Program();
        program->addShader(vShader);
        program->addShader(fShader);
        n->getOrCreateStateSet()->setAttribute(program);

        // Uniform for lighting and texture tiles showing
        if (!s_enableLighting) s_enableLighting= new osg::Uniform("enableLighting",true);
        if (!s_showTiles) s_showTiles= new osg::Uniform("showTiles",false);
        n->getOrCreateStateSet()->addUniform(s_enableLighting);
        n->getOrCreateStateSet()->addUniform(s_showTiles);
    }

    // print out some details of the scenegraph
    {
        std::cout << "\nScene center: " << n->getBound().center() << " radius: " << n->getBound().radius() << "\n" << std::endl;
    }

    osgViewer::Viewer * viewer = new osgViewer::Viewer;

    s_viewWidget = new osgWt::WtOSGWidget(screenWidth, screenHeight, w->root());

    s_viewWidget->setFrameUpdateTimeout(updateTimeIntervalInMilliseconds);

    // IMPORTANT
    s_viewWidget->setViewer(viewer);

    // IMPORTANT
    // set as embedded to avoid EGL creation and context management
    viewer->setUpViewerAsEmbeddedInWindow(100,100,screenWidth, screenHeight);

    viewer->setSceneData(n);

    // the camera manipulator is not really used, but we need a manipulator
    viewer->setCameraManipulator(new osgGA::TrackballManipulator);
    
    // these are not used, but could be controlled using key events
    //viewer->addEventHandler(new osgViewer::StatsHandler);
    //viewer->addEventHandler( new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()) );
    //viewer->addEventHandler(new osgViewer::LODScaleHandler);
    return w;
}

int main( int argc, char** argv )
{
    return WRun(argc, argv, &createApplication);     
}
