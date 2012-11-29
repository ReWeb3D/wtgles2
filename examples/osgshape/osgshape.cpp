/* OpenSceneGraph example, osgshape.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

// The example is slightly adapted to run in WebGL using ReWeb3D.
// Note that the shape implementations use client side arrays which are 
// supported but are not very efficient, as WebGL does not support them.
// The workaround creates, sets, and deletes the buffers for each frame.
// 
// In Chrome 23.0.1271.64 m, the client side arrays cause a WebGL error:
// drawElements: attributes not setup correctly...Rendering is fine otherwise
// and Firefox has no problems.

///////////////// ReWeb3D /////////////////////
#include <wtwrapper/WtWrapper.h>
#include <wtwrapper/WtOSGWidget.h>

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WImage>

#include <osg/io_utils>
#include <osgGA/TrackballManipulator> 
#include <osgViewer/Viewer>

///////////////////////////////////////////////


#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/Texture2D>
#include <osgUtil/ShaderGen>

#include <osgViewer/Viewer>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osg/Math>

// for the grid data..
#include "../osghangglide/terrain_coords.h"

///////////////////////// ReWeb3D //////////////////
// ReWeb3D: Added the shaders
std::string defaultFragmentShader = 
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"\n"
"uniform sampler2D osg_Sampler0;"
"varying vec3 vLightWeighting;\n"
"varying vec4 vFrontColor;\n"
"varying vec2 vTexCoord0;\n"
"\n"
"void main(void) {\n"
"	vec4 matColor = vec4(0.878, 0.768, 0.353, 1.0);\n"
"	vec4 texColor = texture2D(osg_Sampler0, vec2(vTexCoord0.s, 1.0-vTexCoord0.t));"
"	vec4 finalColor;"
" if ((texColor.r < 0.999 && texColor.g < 0.999 && texColor.b < 0.999)  ){ "
"	finalColor = vec4(vec3(texColor.rgb * vLightWeighting) ,matColor.a);\n"
" }else{ \n"
"   finalColor = vec4(vFrontColor.rgb * vLightWeighting, vFrontColor.a); \n"
" }\n"
" gl_FragColor = finalColor;\n"
"}\n";

std::string defaultVertexShader =
"attribute vec3 osg_Vertex;\n"
"attribute vec3 osg_Normal;\n"
"attribute vec4 osg_Color;\n"
"attribute vec2 osg_MultiTexCoord0;\n"
"\n"
"uniform mat4 osg_ModelViewMatrix; // [M]odel[V]iew matrix\n"
"uniform mat4 client_ViewMatrix; // [V^-1]View matrix from the client\n"
"uniform mat4 osg_ProjectionMatrix;  // Perspective [P]rojection matrix\n"
"uniform mat3 osg_NormalMatrix;  // [N]ormal transformation\n"
"// uNMatrix is the transpose of the inverse of uCMatrix * uMVMatrix\n"
"\n"
"varying vec3 vLightWeighting;\n"
"varying vec4 vFrontColor;\n"
"varying vec2 vTexCoord0;\n"
"varying vec4 vColor;\n"
"\n"
"void main(void) {\n"
"  // Calculate the position of this vertex\n"
"  gl_Position = osg_ProjectionMatrix * client_ViewMatrix * osg_ModelViewMatrix * vec4(osg_Vertex, 1.0);\n"
"\n"
"  // Phong shading\n"
"  vec3 transformedNormal = normalize(osg_NormalMatrix * osg_Normal);\n"
"  vec3 lightingDirection = -1.0*normalize(vec3(0.2, -0.5, -1.0));\n"
"  float directionalLightWeighting = max(dot(transformedNormal, lightingDirection), 0.0);\n"
"  vec3 uAmbientLightColor = vec3(0.2, 0.2, 0.2);\n"
"  vec3 uDirectionalColor = vec3(0.8, 0.8, 0.8);\n"
"  vLightWeighting = uAmbientLightColor + uDirectionalColor * directionalLightWeighting;\n"
"  vFrontColor = osg_Color;\n"
"  vTexCoord0 = osg_MultiTexCoord0;\n"
"}\n";
///////////////////////// ReWeb3D //////////////////


osg::Geode* createShapes()
{
    osg::Geode* geode = new osg::Geode();

    
    // ---------------------------------------
    // Set up a StateSet to texture the objects
    // ---------------------------------------
    osg::StateSet* stateset = new osg::StateSet();

    osg::Image* image = osgDB::readImageFile( "Images/lz.rgb" );
    if (image)
    {
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);
        texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
        stateset->setTextureAttributeAndModes(0,texture, osg::StateAttribute::ON);
    }
    
    stateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    
    geode->setStateSet( stateset );

    
    float radius = 0.8f;
    float height = 1.0f;
    
    osg::TessellationHints* hints = new osg::TessellationHints;
    hints->setDetailRatio(0.5f);
    osg::Drawable *sphereGeom = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f,0.0f,0.0f),radius),hints);
    geode->addDrawable(sphereGeom); // reuses vertex arrays

    
    geode->addDrawable(new osg::ShapeDrawable(new osg::Box(osg::Vec3(2.0f,0.0f,0.0f),2*radius),hints));
    geode->addDrawable(new osg::ShapeDrawable(new osg::Cone(osg::Vec3(4.0f,0.0f,0.0f),radius,height),hints));
    geode->addDrawable(new osg::ShapeDrawable(new osg::Cylinder(osg::Vec3(6.0f,0.0f,0.0f),radius,height),hints));
    geode->addDrawable(new osg::ShapeDrawable(new osg::Capsule(osg::Vec3(8.0f,0.0f,0.0f),radius,height),hints));
    
    unsigned int numX = 38, numY = 39;
    //int numX = 9, numY = 10;
    osg::HeightField* grid = new osg::HeightField;
    grid->allocate(numX,numY);
    grid->setXInterval(0.28f);
    grid->setYInterval(0.28f);
    
    for(unsigned int r=0;r<numY;++r)
    {
        for(unsigned int c=0;c<numX;++c)
        {
            grid->setHeight(c,r,vertex[r+c*numY][2]);
        }
    }
    // reuses quads
    geode->addDrawable(new osg::ShapeDrawable(grid));
    
    osg::ConvexHull* mesh = new osg::ConvexHull;
    osg::Vec3Array* vertices = new osg::Vec3Array(4);
    (*vertices)[0].set(9.0+0.0f,-1.0f+2.0f,-1.0f+0.0f);
    (*vertices)[1].set(9.0+1.0f,-1.0f+0.0f,-1.0f+0.0f);
    (*vertices)[2].set(9.0+2.0f,-1.0f+2.0f,-1.0f+0.0f);
    (*vertices)[3].set(9.0+1.0f,-1.0f+1.0f,-1.0f+2.0f);
    osg::UByteArray* indices = new osg::UByteArray(12);
    (*indices)[0]=0;
    (*indices)[1]=2;
    (*indices)[2]=1;
    (*indices)[3]=0;
    (*indices)[4]=1;
    (*indices)[5]=3;
    (*indices)[6]=1;
    (*indices)[7]=2;
    (*indices)[8]=3;
    (*indices)[9]=2;
    (*indices)[10]=0;
    (*indices)[11]=3;
    mesh->setVertices(vertices);
    mesh->setIndices(indices);
    geode->addDrawable(new osg::ShapeDrawable(mesh));

    return geode;
}

//////////////// ReWeb3D
// ReWeb3D : Wt create application function.
class OSGWTApp : public Wt::WApplication
{
public:
	OSGWTApp(const Wt::WEnvironment& env)
		:Wt::WApplication(env)
    {

    };
};
Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
    // uncomment to get error checking (slow)
    //Wt::WGLWidget::enableClientErrorChecks(true);

    OSGWTApp * w = new OSGWTApp (env);
        
    w->setTitle("osgShape");

    unsigned int screenWidth = 1024, screenHeight = 1024;
	Wt::WContainerWidget* glContainer = new Wt::WContainerWidget(w->root());
    glContainer->resize(screenWidth, screenHeight);
    glContainer->setInline(false);
    
	osg::ref_ptr<osg::Group> g = new osg::Group();
	
    g->addChild(createShapes());

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
        g->getOrCreateStateSet()->setAttribute(program);
    }

    osgViewer::Viewer * viewer = new osgViewer::Viewer;

    osgWt::WtOSGWidget * viewWidget = new osgWt::WtOSGWidget(screenHeight, screenWidth, glContainer);

    // IMPORTANT
    viewWidget->setViewer(viewer);

    // IMPORTANT
    viewer->setUpViewerAsEmbeddedInWindow(100,100,screenWidth,screenHeight);
    
    viewer->setSceneData(g);

    viewer->setCameraManipulator(new osgGA::TrackballManipulator);

    viewer->getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
    viewer->getCamera()->setProjectionMatrixAsPerspective(45,((double)screenWidth)/screenHeight, 0.5, 100.0);
    return w;

	////////////// ADDED
}


// ReWeb3D: Added argc and argv
int main(int argc, char **argv)
{
    /*
    // ReWeb3D - moved to createApplication
    // create the model
    // construct the viewer.
    osgViewer::Viewer viewer;

    // add model to viewer.
    viewer.setSceneData( createShapes() );

    return viewer.run();*/

    return Wt::WRun(argc, argv, &createApplication);
}
