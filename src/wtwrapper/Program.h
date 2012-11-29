#ifndef WTW_PROGRAM_H
#define WTW_PROGRAM_H


#include <wtwrapper/WtWrapper.h>

#pragma warning (push)
#pragma warning (disable: 4251) //("vector needs  dll-interface")
#pragma warning (disable: 4275) //("non dll-interface class used as base for dll-interface")
#include <Wt/WGLWidget>


#include <vector>

namespace WTW
{
    struct Attribute;
    struct Uniform;

class WTW_API Program
{
public:
    Program(const Wt::WGLWidget::Program & wtProgram):m_wtProgram(wtProgram){};

    static const int UNDEFINED = -1;

    Wt::WGLWidget::Program m_wtProgram;

    // attach shader to t this program. 
    // needed to compute the active uniforms and attributes when linked
    void attachShader(Shader* shader) {m_attachedShaders.push_back(shader);}

    // returns active Attribute. 
    // note that it does not represent the real active attributes, just ones that are listed in the shader source
    // only valid after shaders have been attached and computeActiveUniformsAndAttributes() has been called.
    void getActiveAttrib(GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);

    // returns active uniforms
    // note that it does not represent the real uniform attributes, just ones that are listed in the shader source
    // only valid after shaders have been attached and computeActiveUniformsAndAttributes() has been called.
    void getActiveUniform(GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);

    // Returns the number of active uniforms.
    // only valid after shaders have been attached and computeActiveUniformsAndAttributes() has been called.
    int getNumActiveUniforms();
    
    // Returns the number of active attributes.
    // only valid after shaders have been attached and computeActiveUniformsAndAttributes() has been called.
    int getNumActiveAttributes(){return static_cast<int>(m_activeAttributes.size());}


    // Returns the number of attached shaders.
    // only valid after shaders have been attached
    int getNumAttachedShaders(){return static_cast<int>(m_attachedShaders.size());}




    // call this when the program is linked.
    // it will compute the active attributes and uniforms.
    void computeActiveUniformsAndAttributes();

    // Returns the GL identifier or UNDEFINED if none is there
    int getGLIDbyUniformLocation(const Wt::WGLWidget::UniformLocation & loc) const
    {
        if (m_uniformLocation2glIndex.count(loc) == 0)
        {
            return UNDEFINED;
        }else
        {
            return m_uniformLocation2glIndex.find(loc)->second;
        }
    }

    // Returns the GL identifier or UNDEFINED if none is there
    int getGLIDbyUniformName(const std::string & name) const
    {
        if (m_glUniformName2glIndex.count(name) == 0)
        {
            return UNDEFINED;
        }else
        {
            return m_glUniformName2glIndex.find(name)->second;
        }
    }


    // Returns the UniformLocation identifier or "" if none is there
    Wt::WGLWidget::UniformLocation getUniformLocation(const int & glID) const
    {
        if (m_glIndex2uniformLocation.count(glID) == 0)
        {
            return "";
        }else
        {
            return m_glIndex2uniformLocation.find(glID)->second;
        }
    }

    // creates a new Uniform entry in this program. Returns the GLID
    int createUniformLocation(const Wt::WGLWidget::UniformLocation & loc, const std::string & name)
    {
        int retVal = static_cast<int>(m_uniformLocation2glIndex.size());
        m_uniformLocation2glIndex[loc] = retVal;
        m_glIndex2uniformLocation[retVal] = loc; 

        m_glUniformName2glIndex[name] = retVal;
        return retVal;
    }


    //////////////////////////////////////////////////////////////////////////
    // attributes
    // Returns the GL identifier or UNDEFINED if none is there
    int getGLIDbyAttributeName(const std::string & name) const
    {
        if (m_glAttributeName2glIndex.count(name) == 0)
        {
            return UNDEFINED;
        }else
        {
            return m_glAttributeName2glIndex.find(name)->second;
        }
    }

    // creates a new Attribute entry in this program. Returns the GLID
    int addAttributeLocation(const Wt::WGLWidget::AttribLocation & loc, const std::string & name)
    {
        int retVal = static_cast<int>(m_attributeLocation2glIndex.size());
        m_attributeLocation2glIndex[loc] = retVal;
        m_glIndex2attributeLocation[retVal] = loc; 

        m_glAttributeName2glIndex[name] = retVal;
        return retVal;
    }

    // creates a new Attribute entry in this program. Returns the GLID
    // use this for storing the bindAttributeLocation 
    void addAttributeLocation(const GLuint & glIndex, const std::string & name)
    {
        Wt::WGLWidget::AttribLocation loc = boost::lexical_cast<std::string>(glIndex);
        m_attributeLocation2glIndex[loc] = glIndex;
        m_glIndex2attributeLocation[glIndex] = loc; 

        m_glAttributeName2glIndex[name] = glIndex;
    }


private:

    //////////////////////////////////////////////////////////////////////////
    // Uniforms
    // TODO: consider using bimap http://www.codeproject.com/KB/stl/bimap.aspx
    // UniformLocation -> GL index
    std::map<Wt::WGLWidget::UniformLocation, int> m_uniformLocation2glIndex;
    // GL index -> UniformLocation 
    std::map<int, Wt::WGLWidget::UniformLocation> m_glIndex2uniformLocation;

    // this is a hack to support WT returning a new identifier on each call
    std::map<std::string, int> m_glUniformName2glIndex;

    //////////////////////////////////////////////////////////////////////////
    // Attributes
    // AttributeLocation -> GL index
    std::map<Wt::WGLWidget::AttribLocation, int> m_attributeLocation2glIndex;
    std::map<int, Wt::WGLWidget::AttribLocation> m_glIndex2attributeLocation;
    // this is a hack to support WT returning a new identifier on each call
    std::map<std::string, int> m_glAttributeName2glIndex;

    std::vector<Shader*> m_attachedShaders;

    std::map<std::string, Attribute> m_activeAttributes;
    std::map<std::string, Uniform> m_activeUniforms;

};
}

#pragma warning (pop) // warning disabling

#endif // WTW_PROGRAM_H