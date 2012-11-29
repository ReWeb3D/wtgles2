#ifndef WTW_SHADER_H
#define WTW_SHADER_H


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

    class WTW_API Shader
    {
    public:
        Shader(const Wt::WGLWidget::Shader & wtShader, const Wt::WGLWidget::GLenum & type)
            :m_wtShader(wtShader), m_type(type)
        {};

        ~Shader()
        {
        }

        void parseShaderSource(std::string & shaderSource);

        const Wt::WGLWidget::Shader m_wtShader;
        const Wt::WGLWidget::GLenum m_type;

        std::vector<Attribute> m_attributes;
        std::vector<Uniform> m_uniforms;

    };


    struct WTW_API Attribute
    {
        Attribute():m_type(0), m_size(0){}
        Attribute (const std::string & name, const int & GL_type, const int & size)
            :m_name(name), m_type (GL_type), m_size(size){}

        std::string m_name;
        int m_type;
        int m_size;
    };

    struct WTW_API Uniform
    {
        Uniform():m_type(0), m_size(0){}
        Uniform (const std::string & name, const int & GL_type, const int & size)
            :m_name(name), m_type (GL_type), m_size(size){}

        std::string m_name;
        int m_type;
        int m_size;
    };
}

#pragma warning (pop) // warning disabling

#endif //WTW_SHADER_H