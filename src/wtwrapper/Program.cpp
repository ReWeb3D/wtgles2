#include "Program.h"

#include "Shader.h"

namespace{
    void copyString(const std::string & source, char* destination)
    {
        // get rid of C4996 warning 
        //("Function call with parameters that may be unsafe - this call relies on the caller to check that the passed values are correct. To disable this warning, use -D_SCL_SECURE_NO_WARNINGS. See documentation on how to use Visual C++ 'Checked Iterators'")
#pragma warning (push)
#pragma warning (disable: 4996) 
        source.copy(destination, source.size());
        // add a 0 termination
        destination[source.size()] = 0;
#pragma warning (pop)
    }
}

namespace WTW
{

    int Program::getNumActiveUniforms()
    {
        //// have to add all sizes as well to take care of arrays
        //unsigned int size = 0;
        ////std::map<std::string, Uniform>::iterator it = 
        //for (std::map<std::string, Uniform>::iterator it = m_activeUniforms.begin(),
        //    itEnd = it = m_activeUniforms.end(); it!= itEnd; it++)
        //{
        //    size += it->second.m_size;
        //}

        return static_cast<int>(m_activeUniforms.size());
    }

    void Program::computeActiveUniformsAndAttributes()
    {
        // reset
        m_activeUniforms.clear();
        m_activeAttributes.clear();

        // need to aggregate uniforms and attributes from all attached shaders
        // just check for same name
        for( size_t i = 0, iEnd = m_attachedShaders.size(); i<iEnd; i++)
        {
            const Shader* currentShader = m_attachedShaders[i];
            for (size_t j = 0, jEnd = currentShader->m_attributes.size(); j<jEnd; j++)
            {
                const Attribute & currentAttribute = currentShader->m_attributes[j];
                if (m_activeAttributes.count(currentAttribute.m_name) == 0)
                {
                    m_activeAttributes[currentAttribute.m_name] = currentAttribute;
                }
            }
            for (size_t j = 0, jEnd = currentShader->m_uniforms.size(); j<jEnd; j++)
            {
                const Uniform & currentUniform = currentShader->m_uniforms[j];
                if (m_activeUniforms.count(currentUniform.m_name) == 0)
                {
                    m_activeUniforms[currentUniform.m_name] = currentUniform;
                }
            }
        }
    }

    void Program::getActiveAttrib(GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
    {
        // get attribute at index
        unsigned int i = 0;
        std::map<std::string, Attribute>::iterator it = m_activeAttributes.begin(), 
            itEnd = m_activeAttributes.end();
        for (; it!=itEnd; it++, i++)
        {
            if (i==index) break;
        }
        Attribute& attribute = it->second;


        *size = attribute.m_size;
        *type = (GLenum)attribute.m_type;        

        if (length) *length = static_cast<GLsizei>(attribute.m_name.size());
        copyString(attribute.m_name, name);
    }

    void Program::getActiveUniform(GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
    {
        // get attribute at index
        unsigned int i = 0;
        std::map<std::string, Uniform>::iterator it = m_activeUniforms.begin(), 
            itEnd = m_activeUniforms.end();
        for (; it!=itEnd; it++, i++)
        {
            if (i==index) break;
        }
        Uniform& uniform = it->second;


        *size = uniform.m_size;
        *type = (GLenum)uniform.m_type;        

        if (length) *length = static_cast<GLsizei>(uniform.m_name.size());
        copyString(uniform.m_name, name);

    }

}

