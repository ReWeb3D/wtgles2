#include <wtwrapper/WtOSGWidget.h>
#include <wtwrapper/WtWrapper.h>

#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WImage>
#include <Wt/WPushButton>
#include <Wt/WTabWidget>
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

#include <osg/ArgumentParser>
#include <osgUtil/Optimizer>
#include <osgUtil/Simplifier>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgEarthUtil/ExampleResources>
#include <osgEarth/ShaderFactory>
#include <osgEarth/Registry>

#include <iostream>

#include <Wt/WGLWidget>
#include <Wt/WMatrix4x4>

using namespace Wt;

//////////////////////////////////////////////////////////////////////////
#ifndef PRECISION_MEDIUMP_FLOAT
#   define PRECISION_MEDIUMP_FLOAT "precision mediump float;"
#endif

// TODO: refactor to WTWOSG
// create customized osgEarth shader factory that provides multiplication
// of the client view matrix, so we have interactive motion.
// adapted from osgEarth/ShaderFactory.cpp
using namespace osgEarth::ShaderComp;
class MyShaderFactory: public osgEarth::ShaderFactory
{
public:
    // override to customize vertex transformation
    virtual osg::Shader* createVertexShaderMain(
        const FunctionLocationMap& functions =FunctionLocationMap(),
        bool useLightingShaders =true ) const
    {
        FunctionLocationMap::const_iterator i = functions.find( LOCATION_VERTEX_PRE_TEXTURING );
        const OrderedFunctionMap* preTexture = i != functions.end() ? &i->second : 0L;

        FunctionLocationMap::const_iterator j = functions.find( LOCATION_VERTEX_PRE_LIGHTING );
        const OrderedFunctionMap* preLighting = j != functions.end() ? &j->second : 0L;

        FunctionLocationMap::const_iterator k = functions.find( LOCATION_VERTEX_POST_LIGHTING );
        const OrderedFunctionMap* postLighting = k != functions.end() ? &k->second : 0L;

        std::stringstream buf;
        buf << "#version " << GLSL_VERSION_STR << "\n"
            << PRECISION_MEDIUMP_FLOAT "\n"
            << "void osgearth_vert_setupColoring(); \n";

        if ( useLightingShaders )
            buf << "void osgearth_vert_setupLighting(); \n";

        if ( preTexture )
            for( OrderedFunctionMap::const_iterator i = preTexture->begin(); i != preTexture->end(); ++i )
                buf << "void " << i->second << "(); \n";

        if ( preLighting )
            for( OrderedFunctionMap::const_iterator i = preLighting->begin(); i != preLighting->end(); ++i )
                buf << "void " << i->second << "(); \n";

        if ( postLighting )
            for( OrderedFunctionMap::const_iterator i = postLighting->begin(); i != postLighting->end(); ++i )
                buf << "void " << i->second << "(); \n";

        // TG: adapted here!
        buf <<
            "uniform mat4 osg_ModelViewMatrix; // [M]odel[V]iew matrix\n"
            "uniform mat4 client_ViewMatrix; // [V^-1]View matrix from the client\n"
            "uniform mat4 osg_ProjectionMatrix;  // Perspective [P]rojection matrix\n";

        buf << "void main(void) \n"
            << "{ \n"
            << "    gl_Position = osg_ProjectionMatrix * client_ViewMatrix * osg_ModelViewMatrix * gl_Vertex; \n";

        // TG: end of change

        if ( preTexture )
            for( OrderedFunctionMap::const_iterator i = preTexture->begin(); i != preTexture->end(); ++i )
                buf << "    " << i->second << "(); \n";

        buf << "    osgearth_vert_setupColoring(); \n";

        if ( preLighting )
            for( OrderedFunctionMap::const_iterator i = preLighting->begin(); i != preLighting->end(); ++i )
                buf << "    " << i->second << "(); \n";

        if ( useLightingShaders )
            buf << "    osgearth_vert_setupLighting(); \n";

        if ( postLighting )
            for( OrderedFunctionMap::const_iterator i = postLighting->begin(); i != postLighting->end(); ++i )
                buf << "    " << i->second << "(); \n";

        buf << "} \n";

        std::string str;
        str = buf.str();
        osg::Shader* shader = new osg::Shader( osg::Shader::VERTEX, str );
        shader->setName( "main(vert)" );
        return shader;

    }
};

// This application is an example for an osgEarth application. 
// Per default, it loads openstreetmap.earth.
// Start instructions are similar as osgViewerWt.wt
// 
// Other models can be loaded using the dataset parameter, e.g.
// &, for example http://localhost:8080/viewer?dataset=readymap.earth
class OSGWTApp : public WApplication
{
public:
    OSGWTApp(const WEnvironment& env)
        :WApplication(env)
    {
    };

private:
};

// Creation of dummy shaders, these shaders are not used, they are purely 
// in the scenegraph to prevent
// OSG to generate a shader (which is provoking error messages in the browser
// as it is not GLES2 compatible)
// Instead, osgEarth is creating a GLES2 conform shader.
std::string dummyFragmentShader = 
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"\n"
"void main(void) {\n"
"  gl_FragColor = vec4(1.0,0.0,0.0,1.0);\n"
"}\n";

std::string dummyVertexShader =
"attribute vec3 osg_Vertex;\n"
"\n"
"void main(void) {\n"
"  // Calculate the position of this vertex\n"
"  gl_Position = vec4(osg_Vertex, 1.0);\n"
"}\n";

 osgWt::WtOSGWidget * s_viewWidget = 0;

WApplication *createApplication(const WEnvironment& env)
{
    // uncomment to add error checks after each drawcall
    // (if enabled in Wt/WGLWidget)
    //Wt::WGLWidget::enableClientErrorChecks();

    // Important for osgEarth apps:
    // setup new shaderfactory to create customized shader
    osgEarth::Registry::instance()->setShaderFactory(new MyShaderFactory());

    OSGWTApp * w = new OSGWTApp (env);
        
    w->setTitle("osgEarth Viewer");

    w->root()->addWidget(new WText("Below you see the osgEarth scene rendered using WebGL.",w->root()));
    w->root()->addWidget(new WBreak(w->root()));
    WText * fileNameLabel = new WText(w->root());
    w->root()->addWidget(new WBreak(w->root()));
    WContainerWidget* glContainer = new WContainerWidget(w->root());
    unsigned int screenWidth = 1024, screenHeight = 768;
    glContainer->resize(screenWidth, screenHeight);
    glContainer->setInline(false);

    osgViewer::Viewer * viewer = new osgViewer::Viewer;

    osgDB::Registry::instance()->getDataFilePathList().push_front("datasets");

    std::string modelFileName = "openstreetmap.earth";
    
    // check if a name was given in the URL
    Http::ParameterValues pv = env.getParameterValues("dataset");
    if (!pv.empty())
    {
        modelFileName = pv.at(0);
    }
    
    fileNameLabel->setText(modelFileName);

    modelFileName = osgDB::Registry::instance()->findDataFile(modelFileName, NULL, osgDB::CASE_INSENSITIVE);

    osg::ref_ptr<osg::Node> n = 0;
    
    std::vector<char*> argc;
    argc.push_back("osgEarthViewer");
    char* cstr = new char[modelFileName.size()+1];
    modelFileName.copy(cstr,modelFileName.size());
    cstr[modelFileName.size()] = 0;

    argc.push_back(cstr);
    int numArgs = argc.size();

    osg::ArgumentParser args(&numArgs,&argc[0]);
    // try to read osgEarth file
    n = osgEarth::Util::MapNodeHelper().load( args, viewer );

    // if not, fall back to osg
    if (n== NULL) n = osgDB::readNodeFile(modelFileName);
    
    if (n == NULL)
    {
        fileNameLabel->setText(fileNameLabel->text() + " not found");
        std::cout << "model not found: " << modelFileName << std::endl;
        return w;
    }

    // use vertex buffer objects only (no client side arrays are supported by webgl)
    osgUtil::GLObjectsVisitor glov(osgUtil::GLObjectsVisitor::SWITCH_ON_VERTEX_BUFFER_OBJECTS);
    n->accept(glov);

    // print out some details of the scenegraph
    {
        std::cout << "\nScene center: " << n->getBound().center() << " radius: " << n->getBound().radius() << "\n" << std::endl;
    }

    s_viewWidget = new osgWt::WtOSGWidget(screenWidth, screenHeight, glContainer);

    // IMPORTANT
    s_viewWidget->setViewer(viewer);

    // IMPORTANT
    viewer->setUpViewerAsEmbeddedInWindow(100,100,screenWidth, screenHeight);
    
 
    {
        // These shaders are not used, they are purely in the scenegraph to prevent
        // OSG to create a shader (which is provoking error messages in the browser
        // as it is not GLES2 compatible)
        // Instead, osgEarth is creating a GLES2 conform shader
        osg::ref_ptr<osg::Shader> vShader = new osg::Shader(
            osg::Shader::VERTEX,
            dummyVertexShader);

        osg::ref_ptr<osg::Shader> fShader = new osg::Shader(
            osg::Shader::FRAGMENT,
            dummyFragmentShader);

        osg::ref_ptr<osg::Program> program = new osg::Program();
        program->addShader(vShader);
        program->addShader(fShader);
        n->getOrCreateStateSet()->setAttribute(program);
    }

    viewer->setSceneData(n);
    viewer->setCameraManipulator(new osgGA::TrackballManipulator);
    return w;
}

int main( int argc, char** argv )
{
    return WRun(argc, argv, &createApplication);     
}
