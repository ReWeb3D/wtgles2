//
// This file was created starting from libGLESv2.cpp from the ANGLE project.
// it serves to relay the GL API calls to wtwrapper.
//
// libGLESv2.cpp: Implements the exported OpenGL ES 2.0 functions.
// adapted

#define GL_APICALL
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <limits>
#include <iostream>

#include <wtwrapper/WtWrapper.h>

#ifdef WIN32
	#define LIBGLES2_EXPORT __stdcall
#else
	#define LIBGLES2_EXPORT 
#endif


using namespace WTW;

extern "C"
{

    void LIBGLES2_EXPORT glActiveTexture(GLenum texture)
    {
        WtWrapper::instance()->glActiveTexture(texture);
    };

    void LIBGLES2_EXPORT glAttachShader(GLuint program, GLuint shader)
    {
        WtWrapper::instance()->glAttachShader(program, shader);
    };

    void LIBGLES2_EXPORT glBindAttribLocation(GLuint program, GLuint index, const GLchar* name)
    {
        WtWrapper::instance()->glBindAttribLocation(program, index, name);
    }; 

    void LIBGLES2_EXPORT glBindBuffer(GLenum target, GLuint buffer)
    {
        WtWrapper::instance()->glBindBuffer(target, buffer);
    };

    void LIBGLES2_EXPORT glBindFramebuffer(GLenum target, GLuint framebuffer)
    {
        WtWrapper::instance()->glBindFramebuffer(target, framebuffer);
    };

    void LIBGLES2_EXPORT glBindRenderbuffer(GLenum target, GLuint renderbuffer)
    {
        WtWrapper::instance()->glBindRenderbuffer(target, renderbuffer);
    };

    void LIBGLES2_EXPORT glBindTexture(GLenum target, GLuint texture)
    {
        WtWrapper::instance()->glBindTexture(target, texture);
    };

    void LIBGLES2_EXPORT glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
    {
        WtWrapper::instance()->glBlendColor(red,green,blue,alpha);
    };

    void LIBGLES2_EXPORT glBlendEquation(GLenum mode)
    {
        WtWrapper::instance()->glBlendEquation(mode);
    };

    void LIBGLES2_EXPORT glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
    {
        WtWrapper::instance()->glBlendEquationSeparate(modeRGB,modeAlpha);
    };

    void LIBGLES2_EXPORT glBlendFunc(GLenum sfactor, GLenum dfactor)
    {
        WtWrapper::instance()->glBlendFunc(sfactor,dfactor);
    };

    void LIBGLES2_EXPORT glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
    {
        std::cout << "glBlendFuncSeparate not implemented." << std::endl;
    };

    void LIBGLES2_EXPORT glBufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
    {
        WtWrapper::instance()->glBufferData(target, size, data, usage); 
    };

    void LIBGLES2_EXPORT glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
    {
        WtWrapper::instance()->glBufferSubData(target, offset, size, data);
    };

    GLenum LIBGLES2_EXPORT glCheckFramebufferStatus(GLenum target){
        // TG: always return success as no way to check atm
        return 0x8CD5;//GL_FRAMEBUFFER_COMPLETE_EXT;
    }

    void LIBGLES2_EXPORT glClear(GLbitfield mask)
    {
        WtWrapper::instance()->glClear(mask);
    };

    void LIBGLES2_EXPORT glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
    {
        WtWrapper::instance()->glClearColor(red, green, blue, alpha);
    };

    void LIBGLES2_EXPORT glClearDepthf(GLclampf depth)
    {
        WtWrapper::instance()->glClearDepthf(depth);
    };

    void LIBGLES2_EXPORT glClearStencil(GLint s){};

    void LIBGLES2_EXPORT glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
    {
        WtWrapper::instance()->glColorMask(red,green,blue,alpha);
    };

    void LIBGLES2_EXPORT glCompileShader(GLuint shader)
    {
        WtWrapper::instance()->glCompileShader(shader);
    };


    void LIBGLES2_EXPORT glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, 
        GLint border, GLsizei imageSize, const GLvoid* data)
    {
        // TG: not present in WebGL
        std::cout << "glCompressedTexImage2D not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
        GLenum format, GLsizei imageSize, const GLvoid* data)
    {
        // TG: not present in WebGL
        std::cout << "glCompressedTexSubImage2D not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
    {
        std::cout << "glCompressedTexSubImage2D not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
    {
        std::cout << "glCompressedTexSubImage2D not implemented" << std::endl;
    };

    GLuint LIBGLES2_EXPORT glCreateProgram(void)
    {
        return WtWrapper::instance()->glCreateProgram();
    }

    GLuint LIBGLES2_EXPORT glCreateShader(GLenum type)
    {
        return WtWrapper::instance()->glCreateShader(type);
    }

    void LIBGLES2_EXPORT glCullFace(GLenum mode)
    {
        WtWrapper::instance()->glCullFace(mode);
    };

    void LIBGLES2_EXPORT glDeleteBuffers(GLsizei n, const GLuint* buffers)
    {
        WtWrapper::instance()->glDeleteBuffers(n, buffers);
    };

    void LIBGLES2_EXPORT glDeleteFencesNV(GLsizei n, const GLuint* fences)
    {
        std::cout << "glDeleteFencesNV not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
    {
        std::cout << "glDeleteFramebuffers not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glDeleteProgram(GLuint program)
    {
        std::cout << "glDeleteProgram not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
    {
        std::cout << "glDeleteRenderbuffers not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glDeleteShader(GLuint shader)
    {
        std::cout << "glDeleteShader not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glDeleteTextures(GLsizei n, const GLuint* textures)
    {
        WtWrapper::instance()->glDeleteTextures(n,textures);
    };

    void LIBGLES2_EXPORT glDepthFunc(GLenum func)
    {
        WtWrapper::instance()->glDepthFunc(func);
    };

    void LIBGLES2_EXPORT glDepthMask(GLboolean flag)
    {
        WtWrapper::instance()->glDepthMask(flag);
    };

    void LIBGLES2_EXPORT glDepthRangef(GLclampf zNear, GLclampf zFar)
    {
        WtWrapper::instance()->glDepthRange(zNear, zFar);
    };

    void LIBGLES2_EXPORT glDetachShader(GLuint program, GLuint shader)
    {
        std::cout << "glDetachShader not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glDisable(GLenum cap)
    {
        if (cap!=0x0B50)
            WtWrapper::instance()->glDisable(cap);
    };

    // Not Implemented in Wt!
    void LIBGLES2_EXPORT glDisableVertexAttribArray(GLuint index)
    {
        WtWrapper::instance()->glDisableVertexAttribArray(index);
    };

    void LIBGLES2_EXPORT glDrawArrays(GLenum mode, GLint first, GLsizei count)
    {
        WtWrapper::instance()->glDrawArrays(mode, first, count);
    };

    void LIBGLES2_EXPORT glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
    {
        WtWrapper::instance()->glDrawElements(mode, count, type, indices);
    };

    void LIBGLES2_EXPORT glEnable(GLenum cap)
    {
        if (cap!=0x0B50)
            WtWrapper::instance()->glEnable(cap);
    };

    void LIBGLES2_EXPORT glEnableVertexAttribArray(GLuint index)
    {
        WtWrapper::instance()->glEnableVertexAttribArray(index);
    };

    void LIBGLES2_EXPORT glFinishFenceNV(GLuint fence)
    {

    };

    void LIBGLES2_EXPORT glFinish(void)
    {
        std::cout << "glFinish not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glFlush(void)
    {
        std::cout << "glFlush not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
    {
        WtWrapper::instance()->glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
    };

    void LIBGLES2_EXPORT glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
    {
        WtWrapper::instance()->glFramebufferTexture2D(target, attachment, textarget, texture, level);
    };

    void LIBGLES2_EXPORT glFrontFace(GLenum mode)
    {
        std::cout << "glFrontFace not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glGenBuffers(GLsizei n, GLuint* buffers)
    {
        WtWrapper::instance()->glGenBuffers(n, buffers);
    };

    void LIBGLES2_EXPORT glGenerateMipmap(GLenum target)
    {
        WtWrapper::instance()->glGenerateMipmap(target);
    };

    void LIBGLES2_EXPORT glGenFencesNV(GLsizei n, GLuint* fences)
    {
        std::cout << "glGenFencesNV not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glGenFramebuffers(GLsizei n, GLuint* framebuffers)
    {
        WtWrapper::instance()->glGenFramebuffers(n, framebuffers);
    };

    void LIBGLES2_EXPORT glGenRenderbuffers(GLsizei n, GLuint* renderbuffers)
    {
        WtWrapper::instance()->glGenRenderbuffers(n, renderbuffers);
    };

    void LIBGLES2_EXPORT glGenTextures(GLsizei n, GLuint* textures)
    {
        WtWrapper::instance()->glGenTextures(n,textures);
    };

    void LIBGLES2_EXPORT glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
    {
        WtWrapper::instance()->glGetActiveAttrib(program, index, bufsize, length, size, type, name);
    };

    void LIBGLES2_EXPORT glGetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
    {
        WtWrapper::instance()->glGetActiveUniform(program, index, bufsize, length, size, type, name);    
    };

    void LIBGLES2_EXPORT glGetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders)
    {
        std::cout << "glGetAttachedShaders not implemented" << std::endl;
    };

    int LIBGLES2_EXPORT glGetAttribLocation(GLuint program, const GLchar* name)
    {
        return WtWrapper::instance()->glGetAttribLocation(program, name);
    }

    void LIBGLES2_EXPORT glGetBooleanv(GLenum pname, GLboolean* params)
    {
        std::cout << "glGetBooleanv not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params)
    {
        WtWrapper::instance()->glGetBufferParameteriv(target, pname, params);
    };

    GLenum LIBGLES2_EXPORT glGetError(void)
    {
        std::cout << "glGetError not implemented" << std::endl;
        return 0; 
    }

    void LIBGLES2_EXPORT glGetFenceivNV(GLuint fence, GLenum pname, GLint *params)
    {
        std::cout << "glGetFenceivNV not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glGetFloatv(GLenum pname, GLfloat* params)
    {
        std::cout << "glGetFloatv not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params)
    {
        std::cout << "glGetFramebufferAttachmentParameteriv not implemented" << std::endl;
    };

    GLenum LIBGLES2_EXPORT glGetGraphicsResetStatusEXT(void)
    {
        std::cout << "glGetGraphicsResetStatusEXT not implemented" << std::endl;
        return 0; 
    }

    void LIBGLES2_EXPORT glGetIntegerv(GLenum pname, GLint* params)
    {
        WtWrapper::instance()->glGetIntegerv(pname, params);
    };

    void LIBGLES2_EXPORT glGetProgramiv(GLuint program, GLenum pname, GLint* params)
    {
        WtWrapper::instance()->glGetProgramiv(program, pname, params);
    };

    void LIBGLES2_EXPORT glGetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog)
    {
        std::cout << "glGetProgramInfoLog not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params)
    {
        std::cout << "glGetRenderbufferParameteriv not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
    {
        // queried by OSG
        //GL_COMPILE_STATUS
        //GL_INFO_LOG_LENGTH
        std::cout << "glGetShaderiv not implemented" << std::endl;
        *params = GL_TRUE;
    };

    void LIBGLES2_EXPORT glGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog)
    {
        std::cout << "glGetShaderInfoLog not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
    {
        std::cout << "glGetShaderPrecisionFormat not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glGetShaderSource(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source)
    {
        std::cout << "glGetShaderSource not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glGetTranslatedShaderSourceANGLE(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source)
    {
        std::cout << "glGetTranslatedShaderSourceANGLE not implemented" << std::endl;
    };

    const GLubyte* LIBGLES2_EXPORT glGetString(GLenum name)
    {

        std::cout << "glGetString only partly implemented" << std::endl;

        switch (name)
        {
        case GL_VENDOR:
            return (GLubyte*)"Vicomtech";
        case GL_RENDERER:
            return (GLubyte*)"WtGLES Renderer";
        case GL_VERSION:
            return (GLubyte*)"OpenGL ES 2.0";
        case GL_SHADING_LANGUAGE_VERSION:
            return (GLubyte*)"OpenGL ES GLSL ES 1.00";
        case GL_EXTENSIONS:
            return (GLubyte*)"";//((context != NULL) ? context->getExtensionString() : "");
        default:
            //return error(GL_INVALID_ENUM, (GLubyte*)NULL);
            return (GLubyte*)GL_INVALID_ENUM;
        }
    }

    void LIBGLES2_EXPORT glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params)
    {
        std::cout << "glGetTexParameterfv not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glGetTexParameteriv(GLenum target, GLenum pname, GLint* params)
    {
        std::cout << "glGetTexParameteriv not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glGetnUniformfvEXT(GLuint program, GLint location, GLsizei bufSize, GLfloat* params)
    {
        std::cout << "glGetnUniformfvEXT not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glGetUniformfv(GLuint program, GLint location, GLfloat* params)
    {
        std::cout << "glGetUniformfv not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glGetnUniformivEXT(GLuint program, GLint location, GLsizei bufSize, GLint* params)
    {
        std::cout << "glGetnUniformivEXT not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glGetUniformiv(GLuint program, GLint location, GLint* params)
    {
        std::cout << "glGetnUniformivEXT not implemented" << std::endl;
    };

    int LIBGLES2_EXPORT glGetUniformLocation(GLuint program, const GLchar* name)
    {
        return WtWrapper::instance()->glGetUniformLocation(program, name);
    }

    void LIBGLES2_EXPORT glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params)
    {
        std::cout << "glGetnUniformivEXT not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glGetVertexAttribiv(GLuint index, GLenum pname, GLint* params)
    {
        std::cout << "glGetVertexAttribiv not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid** pointer)
    {
        std::cout << "glGetVertexAttribPointerv not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glHint(GLenum target, GLenum mode)
    {
        std::cout << "glHint not implemented" << std::endl;
    };

    GLboolean LIBGLES2_EXPORT glIsBuffer(GLuint buffer)
    {
        std::cout << "glIsBuffer not implemented" << std::endl;
        return 0; 
    }

    GLboolean LIBGLES2_EXPORT glIsEnabled(GLenum cap)
    {
        std::cout << "glIsEnabled not implemented" << std::endl;
        return 0; 
    }

    GLboolean LIBGLES2_EXPORT glIsFenceNV(GLuint fence)
    {
        std::cout << "glIsFenceNV not implemented" << std::endl;
        return 0; 
    }

    GLboolean LIBGLES2_EXPORT glIsFramebuffer(GLuint framebuffer)
    {
        std::cout << "glIsFramebuffer not implemented" << std::endl;
        return 0; 
    }

    GLboolean LIBGLES2_EXPORT glIsProgram(GLuint program)
    {
        std::cout << "glIsProgram not implemented" << std::endl;
        return 0; 
    }

    GLboolean LIBGLES2_EXPORT glIsRenderbuffer(GLuint renderbuffer)
    {
        std::cout << "glIsRenderbuffer not implemented" << std::endl;
        return 0; 
    }

    GLboolean LIBGLES2_EXPORT glIsShader(GLuint shader)
    {
        std::cout << "glIsShader not implemented" << std::endl;
        return 0; 
    }

    GLboolean LIBGLES2_EXPORT glIsTexture(GLuint texture)
    {
        std::cout << "glIsTexture not implemented" << std::endl;
        return 0; 
    }

    void LIBGLES2_EXPORT glLineWidth(GLfloat width)
    {
        WtWrapper::instance()->glLineWidth(width);
    };

    void LIBGLES2_EXPORT glLinkProgram(GLuint program)
    {
        WtWrapper::instance()->glLinkProgram(program);
    };

    void LIBGLES2_EXPORT glPixelStorei(GLenum pname, GLint param)
    {
        WtWrapper::instance()->glPixelStorei(pname,param);
    };

    void LIBGLES2_EXPORT glPolygonOffset(GLfloat factor, GLfloat units)
    {
        std::cout << "glPolygonOffset not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glReadnPixelsEXT(GLint x, GLint y, GLsizei width, GLsizei height,
        GLenum format, GLenum type, GLsizei bufSize,
        GLvoid *data)
    {
        std::cout << "glReadnPixelsEXT not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height,
        GLenum format, GLenum type, GLvoid* pixels)
    {
        std::cout << "glReadPixels not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glReleaseShaderCompiler(void)
    {
        std::cout << "glReleaseShaderCompiler not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glRenderbufferStorageMultisampleANGLE(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
    {
        std::cout << "glRenderbufferStorageMultisampleANGLE not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
    {
        WtWrapper::instance()->glRenderbufferStorage(target, internalformat, width, height);
    };

    void LIBGLES2_EXPORT glSampleCoverage(GLclampf value, GLboolean invert)
    {
        std::cout << "glSampleCoverage not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glSetFenceNV(GLuint fence, GLenum condition)
    {
        std::cout << "glSetFenceNV not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
    {
        WtWrapper::instance()->glScissor(x,y,width,height);
    };

    void LIBGLES2_EXPORT glShaderBinary(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length)
    {
        std::cout << "glShaderBinary not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
    {
        WtWrapper::instance()->glShaderSource(shader,count, string, length);
    };

    void LIBGLES2_EXPORT glStencilFunc(GLenum func, GLint ref, GLuint mask)
    {
        std::cout << "glStencilFunc not implemented" << std::endl;
    };
    void LIBGLES2_EXPORT glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
    {
        std::cout << "glStencilFuncSeparate not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glStencilMask(GLuint mask)
    {
        std::cout << "glStencilMask not implemented" << std::endl;
    };
    void LIBGLES2_EXPORT glStencilMaskSeparate(GLenum face, GLuint mask)
    {
        std::cout << "glStencilMaskSeparate not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
    {
        std::cout << "glStencilOp not implemented" << std::endl;
    };
    void LIBGLES2_EXPORT glStencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
    {
        std::cout << "glStencilOpSeparate not implemented" << std::endl;
    };

    GLboolean LIBGLES2_EXPORT glTestFenceNV(GLuint fence)
    {
        std::cout << "glTestFenceNV not implemented" << std::endl;
        return 0; 
    }

    void LIBGLES2_EXPORT glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
        GLint border, GLenum format, GLenum type, const GLvoid* pixels)
    {
        WtWrapper::instance()->glTexImage2D(target,level,internalformat,width,height,border,format, type, pixels);
    };

    void LIBGLES2_EXPORT glTexParameterf(GLenum target, GLenum pname, GLfloat param)
    {
        std::cout << "glTexParameterf not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
    {
        std::cout << "glTexParameterfv not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glTexParameteri(GLenum target, GLenum pname, GLint param)
    {
        WtWrapper::instance()->glTexParameteri(target, pname, param);
    };

    void LIBGLES2_EXPORT glTexParameteriv(GLenum target, GLenum pname, const GLint* params)
    {
        std::cout << "glTexParameteriv not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glTexStorage2DEXT(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
    {
        std::cout << "glTexStorage2DEXT not implemented" << std::endl;
    };

    void LIBGLES2_EXPORT glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
        GLenum format, GLenum type, const GLvoid* pixels)
    {
        WtWrapper::instance()->glTexSubImage2D(target, level, xoffset, yoffset, 
            width, height,
            format, type, pixels);
    };

    void LIBGLES2_EXPORT glUniform1f(GLint location, GLfloat x)
    {
        WtWrapper::instance()->glUniform1f(location,x);
    };

    void LIBGLES2_EXPORT glUniform1fv(GLint location, GLsizei count, const GLfloat* v)
    {
        WtWrapper::instance()->glUniform1fv(location,count,v);
    };


    void LIBGLES2_EXPORT glUniform1i(GLint location, GLint x)
    {
        WtWrapper::instance()->glUniform1i(location,x);
    };

    void LIBGLES2_EXPORT glUniform1iv(GLint location, GLsizei count, const GLint* v)
    {
        WtWrapper::instance()->glUniform1iv(location,count,v);
    };

    void LIBGLES2_EXPORT glUniform2f(GLint location, GLfloat x, GLfloat y)
    {
        WtWrapper::instance()->glUniform2f(location,x,y);
    };

    void LIBGLES2_EXPORT glUniform2fv(GLint location, GLsizei count, const GLfloat* v)
    {
        WtWrapper::instance()->glUniform2fv(location,count,v);
    };

    void LIBGLES2_EXPORT glUniform2i(GLint location, GLint x, GLint y)
    {
        WtWrapper::instance()->glUniform2i(location,x,y);
    };

    void LIBGLES2_EXPORT glUniform2iv(GLint location, GLsizei count, const GLint* v)
    {
        WtWrapper::instance()->glUniform2iv(location,count,v);
    };

    void LIBGLES2_EXPORT glUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z)
    {
        WtWrapper::instance()->glUniform3f(location,x,y,z);
    };

    void LIBGLES2_EXPORT glUniform3fv(GLint location, GLsizei count, const GLfloat* v)
    {
        WtWrapper::instance()->glUniform3fv(location,count,v);
    };

    void LIBGLES2_EXPORT glUniform3i(GLint location, GLint x, GLint y, GLint z)
    {
        WtWrapper::instance()->glUniform3i(location,x,y,z);
    };

    void LIBGLES2_EXPORT glUniform3iv(GLint location, GLsizei count, const GLint* v)
    {
        WtWrapper::instance()->glUniform3iv(location,count,v);
    };

    void LIBGLES2_EXPORT glUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
    {
        WtWrapper::instance()->glUniform4f(location,x,y,z,w);
    };

    void LIBGLES2_EXPORT glUniform4fv(GLint location, GLsizei count, const GLfloat* v)
    {
        WtWrapper::instance()->glUniform4fv(location,count,v);
    };

    void LIBGLES2_EXPORT glUniform4i(GLint location, GLint x, GLint y, GLint z, GLint w)
    {
        WtWrapper::instance()->glUniform4i(location,x,y,z,w);
    };

    void LIBGLES2_EXPORT glUniform4iv(GLint location, GLsizei count, const GLint* v)
    {
        WtWrapper::instance()->glUniform4iv(location,count,v);
    };

    void LIBGLES2_EXPORT glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
    {
        WtWrapper::instance()->glUniformMatrix3fv(location, count, transpose, value);
    };

    void LIBGLES2_EXPORT glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
    {
        WtWrapper::instance()->glUniformMatrix3fv(location, count, transpose, value);
    };

    void LIBGLES2_EXPORT glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
    {
        return WtWrapper::instance()->glUniformMatrix4fv(location, count, transpose, value);
    };

    void LIBGLES2_EXPORT glUseProgram(GLuint program)
    {
        WtWrapper::instance()->glUseProgram(program);
    };

    void LIBGLES2_EXPORT glValidateProgram(GLuint program)
    {
        std::cout << "glValidateProgram not implemented." << std::endl;
    };

    void LIBGLES2_EXPORT glVertexAttrib1f(GLuint index, GLfloat x)
    {
        WtWrapper::instance()->glVertexAttrib1f( index, x);
    };

    void LIBGLES2_EXPORT glVertexAttrib1fv(GLuint index, const GLfloat* values)
    {
        WtWrapper::instance()->glVertexAttrib1fv(index,values);
    };

    void LIBGLES2_EXPORT glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
    {
        WtWrapper::instance()->glVertexAttrib2f( index, x, y);
    };

    void LIBGLES2_EXPORT glVertexAttrib2fv(GLuint index, const GLfloat* values)
    {
        WtWrapper::instance()->glVertexAttrib2fv(index,values);
    };

    void LIBGLES2_EXPORT glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
    {
        WtWrapper::instance()->glVertexAttrib3f( index, x, y, z);
    };

    void LIBGLES2_EXPORT glVertexAttrib3fv(GLuint index, const GLfloat* values)
    {
        WtWrapper::instance()->glVertexAttrib3fv(index,values);
    };

    void LIBGLES2_EXPORT glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
    {
        WtWrapper::instance()->glVertexAttrib4f( index, x, y, z, w);
    };

    void LIBGLES2_EXPORT glVertexAttrib4fv(GLuint index, const GLfloat* values)
    {
        WtWrapper::instance()->glVertexAttrib4fv(index,values);
    };

    void LIBGLES2_EXPORT glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr)
    {
        WtWrapper::instance()->glVertexAttribPointer(index, size, type, normalized, stride, ptr);
    };

    void LIBGLES2_EXPORT glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
    {
        WtWrapper::instance()->glViewport(x,y,width,height);
    };

    void LIBGLES2_EXPORT glTexImage3DOES(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth,
        GLint border, GLenum format, GLenum type, const GLvoid* pixels)
    {
        std::cout << "glTexImage3DOES not implemented." << std::endl;
    }
}
