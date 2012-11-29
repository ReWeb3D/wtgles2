/* OpenSceneGraph example, osglight.
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
// The light state is not (yet) supported by OSG for GLES2, therefore
// we set all light parameters to custom uniforms and provide a shader
// to apply the lighting.
//
// Note that the animation is computed on the server. Only when an update
// occurs, the changed transforms are sent to the client to update the
// position of the glider.

//////////////  ReWeb3D  ///////////////////////
#include <wtwrapper/WtWrapper.h>
#include <wtwrapper/WtOSGWidget.h>

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WImage>


#include <osgGA/TrackballManipulator>

///////////////////////////////////////////////

#include <osgViewer/Viewer>

#include <osg/Group>
#include <osg/Node>

#include <osg/Light>
#include <osg/LightSource>
#include <osg/StateAttribute>
#include <osg/Geometry>
#include <osg/Point>

#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>
#include <osgUtil/SmoothingVisitor>

///////////////////////// ReWeb3D //////////////////
// ReWeb3D: Added the shaders
std::string defaultFragmentShader = 
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"#define PI 3.14159265 \n"
"struct glLightSourceParameters {   \n"
"vec4 ambient;              // Aclarri   \n"
"vec4 diffuse;              // Dcli   \n"
"vec4 specular;             // Scli   \n"
"vec4 position;             // Ppli   // VEC4 \n"
"vec4 halfVector;           // Derived: Hi \n"   
"vec3 spotDirection;        // Sdli    \n"
"float spotExponent;        // Srli    \n"
"float spotCutoff;          // Crli    \n"  
"                           // (range: [0.0,90.0], 180.0)   \n"
"float spotCosCutoff;       // Derived: cos(Crli)            \n"
"                           // (range: [1.0,0.0],-1.0)\n"
"float constantAttenuation; // K0  \n"
"float linearAttenuation;   // K1  \n"
"float quadraticAttenuation;// K2  \n"
"};\n"
"\n"
"struct glMaterialSourceParameters {   \n"
"vec4 ambient;              \n"
"vec4 diffuse;              \n"
"vec4 specular;             \n"
"float shiness;             \n"
"vec4 emission;           // Derived: Hi   \n"
"vec3 colorIndices ;\n"
"};\n"
"\n"
"uniform glLightSourceParameters glLightSource0;\n"
"uniform glLightSourceParameters glLightSource1;\n"
"uniform glMaterialSourceParameters glMaterial;\n"
"uniform mat4 osg_ModelViewMatrix; // [M]odel[V]iew matrix\n"
"uniform mat4 client_ViewMatrix; // [V^-1]View matrix from the client\n"
"\n"
"uniform sampler2D osg_Sampler0;"
"varying vec4 vFrontColor;\n"
"varying vec2 vTexCoord0;\n"
"varying vec3 vTransformedNormal;\n"
"varying vec4 vPosition;\n"
"\n"
"float calculateSpotLightEffect(vec3 vectorToVertex,vec3 spotDirection,float spotExponent,float cutOffAngle)\n"
"{\n"
"	// Check if the point is inside the range of the light.\n"
"	// Calculate the angle between the point<-spotCenter and the spotDirection.\n"
"	if (cutOffAngle == 180.0)\n"
"	{ \n"
"		return 1.0;\n"
"	} \n"
"	else \n"
"	{ \n"
"		float cosAngle = cos(cutOffAngle*PI/180.0); \n"
"		float dotProd = dot(normalize(vectorToVertex),normalize(spotDirection)); \n"
"		if (dotProd>=cosAngle)  // The point is INSIDE the cone \n"
"		{ \n"
"			return pow(max(dotProd,0.0),spotExponent); \n"
"		} \n"
"		else // The Point is outside the cone \n"
"		{ \n"
"			return 0.0; \n"
"		} \n"
"	} \n"
"} \n"
"\n"
"// Calculate Light Weighting Factor \n"
"vec3 calculateLightWeightFactor(glLightSourceParameters glLight){ \n"
"		// Calculate Light 0 \n"
"		vec3 vectorFromPointPositionToVertex = (vPosition.xyz - (client_ViewMatrix*osg_ModelViewMatrix*vec4(glLight.position.xyz,1.0)).xyz); \n"
"		vec3 vectorFromVertexToPointPosition = -1.0*vectorFromPointPositionToVertex; \n"
"		float distance = length(vectorFromPointPositionToVertex); \n"
"		// Attenuation Factor. \n"
"		float AF = 1.0/(glLight.constantAttenuation + distance * glLight.linearAttenuation + pow(distance,2.0) * glLight.quadraticAttenuation); //Attenuation Factor; \n"
"		// SpotLight Effect. \n"
"		float SLE = calculateSpotLightEffect(vectorFromPointPositionToVertex,(client_ViewMatrix*osg_ModelViewMatrix*vec4(glLight.spotDirection,0.0)).xyz,glLight.spotExponent,glLight.spotCutoff); \n"
"		// Ambient Term. \n"
"		vec4 AT = glLight.ambient*glMaterial.ambient; \n"
"		// Diffuse Term \n"
"		vec4 DT = max(dot(normalize(vectorFromVertexToPointPosition),vTransformedNormal),0.0)*glLight.diffuse*glMaterial.diffuse; \n"
"		// Specular Term .. - Assuming that the viewing point is  0.0 0.0 0.0  \n"
"		vec3 eyePos = normalize(-1.0*vPosition.xyz); \n"
"		vec3 reflectedLight = reflect(normalize(vectorFromPointPositionToVertex),vTransformedNormal); \n"
"		vec4 ST = pow(max(dot(normalize(reflectedLight),eyePos),0.0),glMaterial.shiness)*glLight.specular*glMaterial.specular; \n"
"		// Calculate Lighting Factor: \n"
"		return (AF * SLE * (AT + DT + ST)).rgb; \n"
"} \n"
" \n"
"void main(void) {\n"
"	vec4 matColor = vec4(0.878, 0.768, 0.353, 1.0);\n"
"	vec4 texColor = texture2D(osg_Sampler0, vec2(vTexCoord0.s, 1.0-vTexCoord0.t));"
"	vec4 finalColor;"
"   vec3 lightWeighting = calculateLightWeightFactor(glLightSource0)+calculateLightWeightFactor(glLightSource1); \n"
"   if ((texColor.r < 0.999 && texColor.g < 0.999 && texColor.b < 0.999) && (texColor.r > 0.001 && texColor.g > 0.001&& texColor.b > 0.001)){ "
"	    finalColor = vec4(texColor.rgb ,matColor.a);\n"
"   }else{ \n"
"       finalColor = vec4(vFrontColor.rgb, vFrontColor.a); \n"
"   }\n"
"   gl_FragColor = vec4(finalColor.rgb*lightWeighting,finalColor.a);\n"
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
"varying vec4 vFrontColor;\n"
"varying vec2 vTexCoord0;\n"
"varying vec4 vColor;\n"
"varying vec3 vTransformedNormal;\n"
"varying vec4 vPosition;\n"
"\n"
"void main(void) {\n"
"  // Calculate the position of this vertex\n"
"  vec4 position = client_ViewMatrix  * osg_ModelViewMatrix * vec4(osg_Vertex, 1.0);"
"  gl_Position = osg_ProjectionMatrix * position;\n"
"  \n"
"  // Phong shading\n"
"  vec3 transformedNormal = normalize((osg_NormalMatrix * normalize(osg_Normal)).xyz);\n"
"  vFrontColor = osg_Color;\n"
"  vPosition = position;"
"  vTransformedNormal = transformedNormal;\n"
"  vTexCoord0 = osg_MultiTexCoord0;\n"
"}\n";
///////////////////////// ReWeb3D //////////////////


// callback to make the loaded model oscilate up and down.
class ModelTransformCallback : public osg::NodeCallback
{
    public:

        ModelTransformCallback(const osg::BoundingSphere& bs)
        {
            _firstTime = 0.0;
            _period = 4.0f;
            _range = bs.radius()*0.5f;
        }
    
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            osg::PositionAttitudeTransform* pat = dynamic_cast<osg::PositionAttitudeTransform*>(node);
            const osg::FrameStamp* frameStamp = nv->getFrameStamp();
            if (pat && frameStamp)
            {
                if (_firstTime==0.0) 
                {
                    _firstTime = frameStamp->getSimulationTime();
                }
                
                double phase = (frameStamp->getSimulationTime()-_firstTime)/_period;
                phase -= floor(phase);
                phase *= (2.0 * osg::PI);
            
                osg::Quat rotation;
                rotation.makeRotate(phase,1.0f,1.0f,1.0f);
                
                pat->setAttitude(rotation); 
                
                pat->setPosition(osg::Vec3(0.0f,0.0f,sin(phase))*_range);
            }
        
            // must traverse the Node's subgraph            
            traverse(node,nv);
        }
        
        double _firstTime;
        double _period;
        double _range;

};
osg::Light* Light1;
osg::Light* Light0;
osg::Node* createLights(osg::BoundingBox& bb,osg::StateSet* rootStateSet)
{
    osg::Group* lightGroup = new osg::Group;
    
    float modelSize = bb.radius();

    // create a spot light.
    osg::Light* myLight1 = new osg::Light;
    myLight1->setLightNum(0);
    myLight1->setPosition(osg::Vec4(bb.corner(4),1.0f));
    myLight1->setAmbient(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
    myLight1->setDiffuse(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
    myLight1->setSpotCutoff(20.0f);
    myLight1->setSpotExponent(50.0f);
    myLight1->setDirection(osg::Vec3(1.0f,1.0f,-1.0f));
    Light0=myLight1;
    osg::LightSource* lightS1 = new osg::LightSource;    
    lightS1->setLight(myLight1);
    lightS1->setLocalStateSetModes(osg::StateAttribute::ON); 

    lightS1->setStateSetModes(*rootStateSet,osg::StateAttribute::ON);
    lightGroup->addChild(lightS1);
    

    // create a local light.
    osg::Light* myLight2 = new osg::Light;
    myLight2->setLightNum(1);
    myLight2->setPosition(osg::Vec4(0.0,0.0,0.0,1.0f));
    myLight2->setAmbient(osg::Vec4(0.0f,1.0f,1.0f,1.0f));
    myLight2->setDiffuse(osg::Vec4(0.0f,1.0f,1.0f,1.0f));
    myLight2->setConstantAttenuation(1.0f);
    myLight2->setLinearAttenuation(2.0f/modelSize);
    myLight2->setQuadraticAttenuation(2.0f/osg::square(modelSize));
    Light1 = myLight2;
    osg::LightSource* lightS2 = new osg::LightSource;    
    lightS2->setLight(myLight2);
    lightS2->setLocalStateSetModes(osg::StateAttribute::ON); 

    lightS2->setStateSetModes(*rootStateSet,osg::StateAttribute::ON);
    
    osg::MatrixTransform* mt = new osg::MatrixTransform();
    {
        // set up the animation path 
        osg::AnimationPath* animationPath = new osg::AnimationPath;
        animationPath->insert(0.0,osg::AnimationPath::ControlPoint(bb.corner(0)));
        animationPath->insert(1.0,osg::AnimationPath::ControlPoint(bb.corner(1)));
        animationPath->insert(2.0,osg::AnimationPath::ControlPoint(bb.corner(2)));
        animationPath->insert(3.0,osg::AnimationPath::ControlPoint(bb.corner(3)));
        animationPath->insert(4.0,osg::AnimationPath::ControlPoint(bb.corner(4)));
        animationPath->insert(5.0,osg::AnimationPath::ControlPoint(bb.corner(5)));
        animationPath->insert(6.0,osg::AnimationPath::ControlPoint(bb.corner(6)));
        animationPath->insert(7.0,osg::AnimationPath::ControlPoint(bb.corner(7)));
        animationPath->insert(8.0,osg::AnimationPath::ControlPoint(bb.corner(0)));
        animationPath->setLoopMode(osg::AnimationPath::SWING);
        
        mt->setUpdateCallback(new osg::AnimationPathCallback(animationPath));
    }
    
    // create marker for point light.
    osg::Geometry* marker = new osg::Geometry;
    osg::Vec3Array* vertices = new osg::Vec3Array;
    vertices->push_back(osg::Vec3(0.0,0.0,0.0));
    marker->setVertexArray(vertices);
    marker->addPrimitiveSet(new osg::DrawArrays(GL_POINTS,0,1));
    
    osg::StateSet* stateset = new osg::StateSet;
    osg::Point* point = new osg::Point;
    // ReWeb3D: point size (and line size) is supported but not implemented by Angle 
    point->setSize(4.0f);
    stateset->setAttribute(point);
    
    marker->setStateSet(stateset);
    
    osg::Geode* markerGeode = new osg::Geode;
    markerGeode->addDrawable(marker);
    
    mt->addChild(lightS2);
    mt->addChild(markerGeode);
    
    lightGroup->addChild(mt);

    return lightGroup;
}

osg::Geometry* createWall(const osg::Vec3& v1,const osg::Vec3& v2,const osg::Vec3& v3,osg::StateSet* stateset)
{

   // create a drawable for occluder.
    osg::Geometry* geom = new osg::Geometry;
    
    geom->setStateSet(stateset);

    unsigned int noXSteps = 100;
    unsigned int noYSteps = 100;
    
    osg::Vec3Array* coords = new osg::Vec3Array;
    coords->reserve(noXSteps*noYSteps);
    
    
    osg::Vec3 dx = (v2-v1)/((float)noXSteps-1.0f);
    osg::Vec3 dy = (v3-v1)/((float)noYSteps-1.0f);
    
    unsigned int row;
    osg::Vec3 vRowStart = v1;
    for(row=0;row<noYSteps;++row)
    {
        osg::Vec3 v = vRowStart;
        for(unsigned int col=0;col<noXSteps;++col)        
        {
            coords->push_back(v);
            v += dx;
        }
        vRowStart+=dy;
    }
    
    geom->setVertexArray(coords);
    
    osg::Vec4Array* colors = new osg::Vec4Array(1);
    (*colors)[0].set(1.0f,1.0f,1.0f,1.0f);
    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);
    
    
    for(row=0;row<noYSteps-1;++row)
    {
        osg::DrawElementsUShort* quadstrip = new osg::DrawElementsUShort(osg::PrimitiveSet::QUAD_STRIP);
        quadstrip->reserve(noXSteps*2);
        for(unsigned int col=0;col<noXSteps;++col)        
        {
            quadstrip->push_back((row+1)*noXSteps+col);
            quadstrip->push_back(row*noXSteps+col);
        }   
        geom->addPrimitiveSet(quadstrip);
    }
    
    // create the normals.    
    osgUtil::SmoothingVisitor::smooth(*geom);
    
    return geom;
 
}

osg::Node* createRoom(osg::Node* loadedModel)
{
    // default scale for this model.
    osg::BoundingSphere bs(osg::Vec3(0.0f,0.0f,0.0f),1.0f);

    osg::Group* root = new osg::Group;

    if (loadedModel)
    {
        const osg::BoundingSphere& loaded_bs = loadedModel->getBound();

        osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform();
        pat->setPivotPoint(loaded_bs.center());
        
        pat->setUpdateCallback(new ModelTransformCallback(loaded_bs));
        pat->addChild(loadedModel);
        
        bs = pat->getBound();
        
        root->addChild(pat);

    }

    bs.radius()*=1.5f;

    // create a bounding box, which we'll use to size the room.
    osg::BoundingBox bb;
    bb.expandBy(bs);


    // create statesets.
    osg::StateSet* rootStateSet = new osg::StateSet;
    root->setStateSet(rootStateSet);

    osg::StateSet* wall = new osg::StateSet;
    wall->setMode(GL_CULL_FACE,osg::StateAttribute::ON);
    
    osg::StateSet* floor = new osg::StateSet;
    floor->setMode(GL_CULL_FACE,osg::StateAttribute::ON);

    osg::StateSet* roof = new osg::StateSet;
    roof->setMode(GL_CULL_FACE,osg::StateAttribute::ON);

    osg::Geode* geode = new osg::Geode;
    
    // create front side.
    geode->addDrawable(createWall(bb.corner(0),
                                  bb.corner(4),
                                  bb.corner(1),
                                  wall));

    // right side
    geode->addDrawable(createWall(bb.corner(1),
                                  bb.corner(5),
                                  bb.corner(3),
                                  wall));

    // left side
    geode->addDrawable(createWall(bb.corner(2),
                                  bb.corner(6),
                                  bb.corner(0),
                                  wall));
    // back side
    geode->addDrawable(createWall(bb.corner(3),
                                  bb.corner(7),
                                  bb.corner(2),
                                  wall));

    // floor
    geode->addDrawable(createWall(bb.corner(0),
                                  bb.corner(1),
                                  bb.corner(2),
                                  floor));

    // roof
    geode->addDrawable(createWall(bb.corner(6),
                                  bb.corner(7),
                                  bb.corner(4),
                                  roof));

    root->addChild(geode);
    
    root->addChild(createLights(bb,rootStateSet));

    return root;
    
}    



//////////////// ADDED
// MAC : Wt create application function.
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
    //Wt::WGLWidget::enableClientErrorChecks();
    OSGWTApp * w = new OSGWTApp (env);
        
    w->setTitle("osgGeometry");

	Wt::WContainerWidget* glContainer = new Wt::WContainerWidget(w->root());
    glContainer->resize(500, 500);
    glContainer->setInline(false);
    
    osg::ref_ptr<osg::Group> g = new osg::Group();

    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile("glider.osgt");

    osg::ref_ptr<osg::Node> rootnode = createRoom(loadedModel);

    // use vertex buffer objects only (no client side arrays are supported by webgl)
    osgUtil::GLObjectsVisitor glov(osgUtil::GLObjectsVisitor::SWITCH_ON_VERTEX_BUFFER_OBJECTS);
    rootnode->accept(glov);
    
    // ReWeb3D: add uniforms for the lights!
    // best solution would be that osg::Light provides these
    {
        osg::Uniform* light0Ambient = new osg::Uniform( "glLightSource0.ambient",Light0->getAmbient() );
        osg::Uniform* light0Diffuse = new osg::Uniform( "glLightSource0.diffuse",Light0->getDiffuse());
        osg::Uniform* light0Specular = new osg::Uniform( "glLightSource0.specular", Light0->getSpecular());
        osg::Uniform* light0SpotExponent = new osg::Uniform( "glLightSource0.spotExponent", Light0->getSpotExponent());
        osg::Uniform* light0SpotCutoff = new osg::Uniform( "glLightSource0.spotCutoff", Light0->getSpotCutoff());
        osg::Uniform* light0linearAttenuation = new osg::Uniform( "glLightSource0.linearAttenuation",Light0->getLinearAttenuation() );
        osg::Uniform* light0constantAttenuation = new osg::Uniform( "glLightSource0.constantAttenuation",Light0->getConstantAttenuation() );
        osg::Uniform* light0quadraticAttenuation = new osg::Uniform( "glLightSource0.quadraticAttenuation",Light0->getQuadraticAttenuation() );
        osg::Uniform* light0spotDirection = new osg::Uniform( "glLightSource0.spotDirection", Light0->getDirection());
        osg::Uniform* light0position = new osg::Uniform( "glLightSource0.position", Light0->getPosition());
        
        osg::Uniform* light1Ambient = new osg::Uniform( "glLightSource1.ambient",Light1->getAmbient() );
        osg::Uniform* light1Diffuse = new osg::Uniform( "glLightSource1.diffuse",Light1->getDiffuse());
        osg::Uniform* light1Specular = new osg::Uniform( "glLightSource1.specular", Light1->getSpecular());
        osg::Uniform* light1SpotExponent = new osg::Uniform( "glLightSource1.spotExponent", Light1->getSpotExponent());
        osg::Uniform* light1SpotCutoff = new osg::Uniform( "glLightSource1.spotCutoff", Light1->getSpotCutoff());
        osg::Uniform* light1linearAttenuation = new osg::Uniform( "glLightSource1.linearAttenuation",Light1->getLinearAttenuation() );
        osg::Uniform* light1constantAttenuation = new osg::Uniform( "glLightSource1.constantAttenuation",Light1->getConstantAttenuation() );
        osg::Uniform* light1quadraticAttenuation = new osg::Uniform( "glLightSource1.quadraticAttenuation",Light1->getQuadraticAttenuation() );
        osg::Uniform* light1spotDirection = new osg::Uniform( "glLightSource1.spotDirection", Light1->getDirection());
        osg::Uniform* light1position = new osg::Uniform( "glLightSource1.position", Light1->getPosition());

        osg::Uniform* materialAmbient = new osg::Uniform( "glMaterial.ambient",osg::Vec4(1.0,1.0,1.0,1.0));
        osg::Uniform* materialDiffuse = new osg::Uniform( "glMaterial.diffuse",osg::Vec4(1.0,1.0,1.0,1.0));
        osg::Uniform* materialSpecular = new osg::Uniform( "glMaterial.specular", osg::Vec4(1.0,1.0,1.0,1.0));
        osg::Uniform* materialShiness = new osg::Uniform( "glMaterial.shiness",(float)32.0 );
        osg::Uniform* materialEmission = new osg::Uniform( "glMaterial.emission", osg::Vec4(0.0,0.0,0.0,0.0));
        
        osg::StateSet* ss = rootnode->getOrCreateStateSet();
        ss->addUniform(light0Ambient);
        ss->addUniform(light0Diffuse);
        ss->addUniform(light0Specular);
        ss->addUniform(light0SpotExponent);
        ss->addUniform(light0SpotCutoff);
        ss->addUniform(light0linearAttenuation);
        ss->addUniform(light0constantAttenuation); 
        ss->addUniform(light0quadraticAttenuation);
        ss->addUniform(light0spotDirection); 
        ss->addUniform(light0position);

        ss->addUniform(light1Ambient);
        ss->addUniform(light1Diffuse);
        ss->addUniform(light1Specular);
        ss->addUniform(light1SpotExponent);
        ss->addUniform(light1SpotCutoff);
        ss->addUniform(light1linearAttenuation);
        ss->addUniform(light1constantAttenuation); 
        ss->addUniform(light1quadraticAttenuation);
        ss->addUniform(light1spotDirection); 
        ss->addUniform(light1position);

        ss->addUniform(materialAmbient);
        ss->addUniform(materialDiffuse);
        ss->addUniform(materialSpecular);
        ss->addUniform(materialShiness);
        ss->addUniform(materialEmission);
    }

    // add shader to OSG scenegraph
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
        rootnode->getOrCreateStateSet()->setAttribute(program);
		
    }

    osgViewer::Viewer * viewer = new osgViewer::Viewer;

    osgWt::WtOSGWidget * viewWidget = new osgWt::WtOSGWidget(800,600, glContainer);

    // IMPORTANT
    viewWidget->setViewer(viewer);

    // IMPORTANT
    viewer->setUpViewerAsEmbeddedInWindow(100,100,800,600);
    
    viewer->setSceneData(rootnode);
    viewer->setCameraManipulator(new osgGA::TrackballManipulator);

    viewer->getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
    viewer->getCamera()->setProjectionMatrixAsPerspective(45,800.0/600.0, 0.5, 100.0);

    viewer->realize();   
    return w;

	////////////// ADDED
}

// MAC: Added argc and argv
int main(int argc, char **argv)
{
    // ReWeb3D - moved to createApplication
    /*// use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer;

    // load the nodes from the commandline arguments.
    osg::Node* loadedModel = osgDB::readNodeFiles(arguments);
    
    // if not loaded assume no arguments passed in, try use default mode instead.
    if (!loadedModel) loadedModel = osgDB::readNodeFile("glider.osgt");
    
    // create a room made of foor walls, a floor, a roof, and swinging light fitting.
    osg::Node* rootnode = createRoom(loadedModel);

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( rootnode );
    

    // create the windows and run the threads.
    viewer.realize();

    viewer.getCamera()->setCullingMode( viewer.getCamera()->getCullingMode() & ~osg::CullStack::SMALL_FEATURE_CULLING);

    return viewer.run();*/

    return Wt::WRun(argc, argv, &createApplication);
}
