#include <wtwrapper/Shader.h>
#include <wtwrapper/Program.h>
#include <catch.hpp>    // using commit 89d2a3f911cd87a6bef96aceb1702e65eee73163
// see http://www.levelofindirection.com/journal/2010/12/28/unit-testing-in-c-and-objective-c-just-got-easier.html


std::string testFragmentShader = 
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

std::string testVertexShader =
"attribute vec3 osg_Vertex;\n"
"attribute vec3 osg_Normal;\n"
"attribute vec4 osg_Color;\n"
"attribute vec2 osg_MultiTexCoord0;\n"
"\n"
"uniform mat4 osg_ModelViewMatrix; " // Put in a line without line break
"uniform mat4 client_ViewMatrix; // [V^-1]View matrix from the client \n"
"uniform mat4 osg_ProjectionMatrix;  // Perspective [P]rojection matrix" // Put in a line without line break
"uniform CrazyType DonotReadThis;\n" // but behind a comment. This should not be read
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
//"  vLightWeighting = uAmbientLightColor + uDirectionalColor * directionalLightWeighting;\n"
"  vLightWeighting = vec3(0.5) + uAmbientLightColor + uDirectionalColor * directionalLightWeighting;\n" // for paper
"  vFrontColor = osg_Color;\n"
"  vTexCoord0 = osg_MultiTexCoord0;\n"
"}\n";


// tests the use of structs and arrays
std::string structShader = 
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
"struct glMaterialSourceParameters {   " // Put in a line without line break
"vec4 ambient;              \n"
"vec4 diffuse;              \n"
"vec4 specular;             \n"
"float shiness;             \n"
"vec4 emission;           // Derived: Hi   \n"
"vec3 colorIndices ;\n"
"};\n"
"struct UnusedStruct \n"
"{\n"       // Put opening bracket on a single line
"vec4 ambient;              \n"
"vec4 diffuse;              \n"
"vec4 specular;             \n"
"float shiness;             \n"
"vec4 emission;           // Derived: Hi   \n"
"vec3 colorIndices ;};" // put closing bracket on the same line as member
"      \n"  // line only with white spaces
"uniform glLightSourceParameters glLightSource0;\n"
"uniform glLightSourceParameters glLightSource1;\n"
"uniform glMaterialSourceParameters glMaterial;\n"
"uniform mat4 osg_ModelViewMatrix; // [M]odel[V]iew matrix\n"
"uniform mat4 client_ViewMatrix; // [V^-1]View matrix from the client\n"
"\n"
"uniform sampler2D osg_Sampler0;"
"uniform bool testArray[3];"
"uniform bool testDegeneratedArray[1];"
"uniform glMaterialSourceParameters glMaterialArrays[2];"
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


// tests the use of structs
std::string matrixShader = 
"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"mat4 rotation = mat4("
"                     vec4( cos(timer),  0.0,  sin(timer),0.0),"
"                     vec4(0.0,  1.0,  0.0,0.0),"
"                     vec4(-sin(timer),         0.0,  cos(timer),0.0)"
"                     vec4(0.0,         0.0,  0.0,1.0)"
"                     );";


TEST_CASE( "testShader", "testing the correct parsing of a shader" )
{
    // provide failing tests to see the behavior in case of bugs
    // uncomment to test
    // CHECK: the program continues to test the other cases
    //CHECK(false); 

    // REQUIRE: the program stops here
    //REQUIRE(false); 

    // test working of buffer functions
    WTW::Shader shader("", (Wt::WGLWidget::GLenum)0);
    CHECK(shader.m_uniforms.size() == 0);
    CHECK(shader.m_attributes.size() == 0);

    //////////////////////////////////////////////////////////////////////////
    // check vertex shader
    shader.parseShaderSource(testVertexShader);
    
    // check the uniforms
    CHECK(shader.m_uniforms.size() == 4);
    
    CHECK(shader.m_uniforms[0].m_name == "osg_ModelViewMatrix");
    CHECK(shader.m_uniforms[0].m_type == GL_FLOAT_MAT4);
    CHECK(shader.m_uniforms[0].m_size == 1);

    CHECK(shader.m_uniforms[1].m_name == "client_ViewMatrix");
    CHECK(shader.m_uniforms[1].m_type == GL_FLOAT_MAT4);
    CHECK(shader.m_uniforms[1].m_size == 1);

    CHECK(shader.m_uniforms[2].m_name == "osg_ProjectionMatrix");
    CHECK(shader.m_uniforms[2].m_type == GL_FLOAT_MAT4);
    CHECK(shader.m_uniforms[2].m_size == 1);

    CHECK(shader.m_uniforms[3].m_name == "osg_NormalMatrix");
    CHECK(shader.m_uniforms[3].m_type == GL_FLOAT_MAT3);
    CHECK(shader.m_uniforms[3].m_size == 1);

    // check the attributes

    CHECK(shader.m_attributes.size() == 4);

    CHECK(shader.m_attributes[0].m_name == "osg_Vertex");
    CHECK(shader.m_attributes[0].m_type == GL_FLOAT_VEC3);
    CHECK(shader.m_attributes[0].m_size == 1);

    CHECK(shader.m_attributes[1].m_name == "osg_Normal");
    CHECK(shader.m_attributes[1].m_type == GL_FLOAT_VEC3);
    CHECK(shader.m_attributes[1].m_size == 1);

    CHECK(shader.m_attributes[2].m_name == "osg_Color");
    CHECK(shader.m_attributes[2].m_type == GL_FLOAT_VEC4);
    CHECK(shader.m_attributes[2].m_size == 1);

    CHECK(shader.m_attributes[3].m_name == "osg_MultiTexCoord0");
    CHECK(shader.m_attributes[3].m_type == GL_FLOAT_VEC2);
    CHECK(shader.m_attributes[3].m_size == 1);


    //////////////////////////////////////////////////////////////////////////
    // check fragment shader
    shader.parseShaderSource(testFragmentShader);

    // check the uniforms
    CHECK(shader.m_uniforms.size() == 1);
    CHECK(shader.m_uniforms[0].m_name == "osg_Sampler0");
    CHECK(shader.m_uniforms[0].m_type == GL_SAMPLER_2D);
    CHECK(shader.m_uniforms[0].m_size == 1);

    // check the attributes
    CHECK(shader.m_attributes.size() == 0);


    //////////////////////////////////////////////////////////////////////////
    // check structs in the shader
    shader.parseShaderSource(structShader);

    // to ease copy and pasting
    int uniformCounter= 0;

    // TODO check num uniforms

    // check glLightSource0 variables
    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource0.ambient");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_VEC4);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);
    
    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource0.diffuse");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_VEC4);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);
    
    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource0.specular");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_VEC4);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource0.position");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_VEC4);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource0.halfVector");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_VEC4);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource0.spotDirection");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_VEC3);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource0.spotExponent");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource0.spotCutoff");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource0.spotCosCutoff");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource0.constantAttenuation");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource0.linearAttenuation");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource0.quadraticAttenuation");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    // check glLightSource1 variables
    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource1.ambient");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_VEC4);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource1.diffuse");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_VEC4);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource1.specular");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_VEC4);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource1.position");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_VEC4);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource1.halfVector");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_VEC4);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource1.spotDirection");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_VEC3);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource1.spotExponent");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource1.spotCutoff");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource1.spotCosCutoff");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource1.constantAttenuation");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource1.linearAttenuation");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glLightSource1.quadraticAttenuation");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    // check glMaterial
    CHECK(shader.m_uniforms[uniformCounter].m_name == "glMaterial.ambient");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_VEC4);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glMaterial.diffuse");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_VEC4);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glMaterial.specular");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_VEC4);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glMaterial.shiness");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glMaterial.emission");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_VEC4);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "glMaterial.colorIndices");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_VEC3);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    // rest
    CHECK(shader.m_uniforms[uniformCounter].m_name == "osg_ModelViewMatrix");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_MAT4);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "client_ViewMatrix");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_MAT4);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "osg_Sampler0");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_SAMPLER_2D);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    // check arrays
    CHECK(shader.m_uniforms[uniformCounter].m_name == "testArray");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_BOOL);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 3);

    CHECK(shader.m_uniforms[uniformCounter].m_name == "testDegeneratedArray");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_BOOL);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    // check array of struct
    CHECK(shader.m_uniforms[uniformCounter].m_name == "glMaterialArrays[0].ambient");
    CHECK(shader.m_uniforms[uniformCounter].m_type == GL_FLOAT_VEC4);
    CHECK(shader.m_uniforms[uniformCounter++].m_size == 1);

    //////////////////////////////////////////////////////////////////////////
    // check matrix in the shader
    shader.parseShaderSource(matrixShader);
    // TODO   
 

}

TEST_CASE( "testProgram", "testing the correct aggregtion of program" )
{
    // provide failing tests to see the behavior in case of bugs
    // uncomment to test
    // CHECK: the program continues to test the other cases
    //CHECK(false); 

    // REQUIRE: the program stops here
    //REQUIRE(false); 

    // test working of buffer functions
    WTW::Shader vShader("", (Wt::WGLWidget::GLenum)0);
    CHECK(vShader.m_uniforms.size() == 0);
    CHECK(vShader.m_attributes.size() == 0);

    // check vertex shader
    vShader.parseShaderSource(testVertexShader);

    WTW::Shader fShader("", (Wt::WGLWidget::GLenum)0);
    CHECK(fShader.m_uniforms.size() == 0);
    CHECK(fShader.m_attributes.size() == 0);

    fShader.parseShaderSource(structShader);

    WTW::Program program ("");
    CHECK(program.getNumAttachedShaders() == 0);
    program.attachShader(&vShader);
    CHECK(program.getNumAttachedShaders() == 1);
    program.attachShader(&fShader);
    CHECK(program.getNumAttachedShaders() == 2);
    program.computeActiveUniformsAndAttributes();

    // 4 + 24 (2*lightsourceStruct) + 6 (material source) + 2 + 12(2*material source) +1
    CHECK(program.getNumActiveUniforms() == 49);

    CHECK(program.getNumActiveAttributes() == 4);
    
    // TODO: check each attribute and uniform
    //void getActiveAttrib(GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
    //void getActiveUniform(GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);

}
