#ifndef WTWRAPPER_H
#define WTWRAPPER_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>

#include <map>
#include <vector>
#include <string>


#if defined (_WIN32) 
#if defined(wtwrapper_EXPORTS)
#define  WTW_API __declspec(dllexport)
#else
#define  WTW_API __declspec(dllimport)
#endif /* WTW_API */
#else /* defined (_WIN32) */
#define WTW_API
#endif


namespace Wt
{
    class WGLWidget;
    class WMemoryResource;
}


#include <vector>

namespace WTW
{

    // needed for data that makes up the state
    class Buffer;
    class Program;
    class Shader;
    struct Texture;
    
    struct ClientPointer;

    // used for render buffers and framebuffers
    struct Framebuffer
    {
        Framebuffer(std::string framebuffer)
            :wtFramebuffer(framebuffer)
        {
        }
        std::string wtFramebuffer;
    };

    // WtWrapper wraps a Wt WGLWidget and relays opengles2 calls to 
    // Javascript calls.
    class WTW_API WtWrapper {
    public:

        // enum encapsulating the current rendering phase.
        // Before traversing the scene, the phase has to be set.
        // conceptionally: in INIT and UPDATE the draw calls are not relayed,
        // while in PAINT buffers, textures, shaders are not set.
        // The phases are translated to the WebGL JavaScript client,
        // which executes the INIT code once, the UPDATE code some times
        // (at regular update intervals), and the PAINT code for each frame.
        enum RenderPhase{INIT=0, PAINT=1, UPDATE=2};

        // enum encapsulating the method of buffer transfer to the client
        // STRING_BUFFER will embed the buffer directly in the javascript
        // BINARY_BUFFER will open a WMemoryResource for each call to bufferData or 
        // bufferSubData, that is preloaded just like images
        // AGGREGATED_BINARY_BUFFER collects buffer data and aggregates them in 
        // BufferResources with the size of MaxBufferAggregationSize
        enum BufferTransferImplementation {
            AGGREGATED_BINARY_BUFFER = 0, 
            BINARY_BUFFER = 1, 
            STRING_BUFFER = 2};
        
        // default buffer implementation is AGGREGATED_BINARY_BUFFER
        BufferTransferImplementation m_bufferTransferImplementation;

        // Static method returning the singleton instance of WtWrapper
        // because of the singleton implementation, only one client may
        // exist per process. (with FCGI on apache, more clients are supported)
        static WtWrapper * instance();

        // important: need to set current WGLWidget where the gl calls are forwarded to
        void setWGLWidget(Wt::WGLWidget * widget){m_wglwidget = widget;}

        // Call this for a new scene when WebGL data must be set new, calls free
        void reset();

        // frees all allocated ressources
        void free();

        // set init phase (setup of buffers, shaders,...) or render phase (draw calls)
        // if a new frame is setup, reset is called.
        void setPhase(const RenderPhase & renderPhase);

        // Needs to be called just before the init phase is completed
        // to init for each program the uniform location for the client side view matrix
        // stores the name of the JS variable to set in each frame
        void initClientSideViewTransformUniform(
            const std::string & csViewTfUniformName, 
            const std::string & csViewTf
            );

        // Binds a client-side float variable to the given uniform
        // Everytime the application tries to set this variable, instead
        // it will be set to the content of the givenclient side variable.
        // use this for example to replace osg_FrameTime by a client side timer.
        // (see WtOSGWidget.cpp)
        // Currently, only one variable is supported.
        // TODO: extend to a mapping of any uniforms, which should be
        // replaced by client variables.
        void bindClientVariableToUniform1f(
            const std::string & clientSideVariable,
            const std::string & uniformName)
        {
            m_clientSideFloat2Uniform = std::pair<std::string, std::string>(clientSideVariable, uniformName);
        }

        // setup workaround for client side attributes: for all active csas, 
        // create buffer, bind, and setvertexattribpointer
        void setupDrawCSARender(unsigned int minIndex, unsigned int maxIndex);
        
        // setup workaround for client side attributes: for all active csas, 
        // remove buffer, bind, and setvertexattribpointer to zero
        void deleteDrawCSARender();

        ///////////////////////
        // GLES 2 interface
        ///////////////////////

        void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

        void glEnable(GLenum cap);

        void glClearDepthf(GLclampf depth);

        void glDepthFunc(GLenum func);

        void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);

        void glBindBuffer(GLenum target, GLuint buffer);

        void glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);

        void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);

        void glDeleteBuffers(GLsizei n, const GLuint *buffer);

        void glGenBuffers(GLsizei n, GLuint *buffer);

        void glDrawArrays(GLenum mode, GLint first, GLsizei count);

        void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);

        GLuint glCreateShader(GLenum type);

        void glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);

        void glCompileShader(GLuint shader);

        void glGetShaderiv(GLuint shader, GLenum pname, GLint* params);

        GLuint glCreateProgram(void);

        void glAttachShader(GLuint program, GLuint shader);

        void glLinkProgram(GLuint program);

        void glDeleteProgram(GLuint program);

        void glDeleteShader(GLuint shader);

        int glGetAttribLocation(GLuint program, const GLchar* name);

        void glBindAttribLocation(GLuint program, GLuint index, const GLchar* name);

        void glUseProgram(GLuint program);

        int glGetUniformLocation(GLuint program, const GLchar* name);

        void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

        void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

        void glGetProgramiv(GLuint program, GLenum pname, GLint* params);

        void glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params);

        void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);

        void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);

        void glEnableVertexAttribArray(GLuint index);

        void glClear(GLbitfield mask);

        void glGetIntegerv(GLenum pname, GLint* params);

        // Texture functions
        void glGenTextures(GLsizei n, GLuint* textures);

        void glBindTexture(GLenum target, GLuint texture);

        void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
            GLint border, GLenum format, GLenum type, const GLvoid* pixels);

        void glTexParameteri(GLenum target, GLenum pname, GLint param);

        void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
            GLenum format, GLenum type, const GLvoid* pixels);

        void glGenerateMipmap(GLenum target);

        void glActiveTexture(GLenum texture);

        void glVertexAttrib1f(GLuint index, GLfloat x);

        void glVertexAttrib1fv(GLuint index, const GLfloat* values);

        void glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y);

        void glVertexAttrib2fv(GLuint index, const GLfloat* values);

        void glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z);

        void glVertexAttrib3fv(GLuint index, const GLfloat* values);

        void glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

        void glVertexAttrib4fv(GLuint index, const GLfloat* values);

        void glPixelStorei(GLenum pname, GLint param);

        void glBlendFunc(GLenum sfactor, GLenum dfactor);

        void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);

        //void glDeleteProgram(GLuint program);

        void glDepthMask(GLboolean flag);

        void glDisable(GLenum cap);

        void glDisableVertexAttribArray(GLuint index);

        void glScissor(GLint x, GLint y, GLsizei width, GLsizei height);    

        void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

        void glDeleteTextures(GLsizei n, const GLuint* textures);

        void glUniform1f(GLint location, GLfloat x);

        void glUniform1fv(GLint location, GLsizei count,const GLfloat *v);

        void glUniform1i(GLint location, GLint x);

        void glUniform1iv(GLint location,GLsizei count,const GLint *v);


        void glUniform2f(GLint location, GLfloat x, GLfloat y);

        void glUniform2fv(GLint location, GLsizei count,const GLfloat *v);

        void glUniform2i(GLint location, GLint x, GLint y);

        void glUniform2iv(GLint location, GLsizei count,const GLint *v);


        void glUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z);

        void glUniform3fv(GLint location, GLsizei count,const GLfloat *v);

        void glUniform3i(GLint location, GLint x, GLint y, GLint z);

        void glUniform3iv(GLint location,GLsizei count,const GLint *v);


        void glUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

        void glUniform4fv(GLint location, GLsizei count,const GLfloat *v);

        void glUniform4i(GLint location, GLint x, GLint y, GLint z, GLint w);

        void glUniform4iv(GLint location,GLsizei count,const GLint *v);


        void glBlendColor(GLclampf red,GLclampf green,GLclampf blue,GLclampf alpha);

        void glBlendEquation(GLenum mode);

        void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha);

        void glClearStencil(GLint s);

        void glCullFace(GLenum mode);

        void glDepthRange(GLclampf zNear, GLclampf zFar);

        void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

        void glLineWidth(GLfloat width);

        // frame buffer / render buffer functions
        void glGenFramebuffers(GLsizei n, GLuint* framebuffers);

        void glGenRenderbuffers(GLsizei n, GLuint* renderbuffers);

        void glBindFramebuffer(GLenum target, GLuint framebuffer);

        void glBindRenderbuffer(GLenum target, GLuint renderbuffer);
        
        void glDeleteFramebuffers(GLsizei n, const GLuint* renderbuffers);
        void glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers);

        void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
        
        void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbufferTarget, GLuint renderbuffer);

        void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLuint level);

        //void setModelViewJSPlaceHolder(const std::string & matrix)
        //{
        //    m_mvMatrixJS = matrix;
        //}
        //void setProjectionJSPlaceHolder(const std::string & matrix)
        //{
        //    m_projMatrixJS = matrix;   
        //};

        //void setNormalJSPlaceHolder(const std::string & matrix)
        //{
        //    m_normalMatrixJS = matrix;   
        //};

    private:

        // private constructor, use instance() to get the singleton
        WtWrapper();

        // called in useProgram to update the uniform for the client side transform
        void renderClientSideViewTransformUniform(Program * p);


        // Called within uniform1x to check whether a clientside variable bound before
        // (bindClientVariableToUniform1f) should be set
        // if yes, it is set and true is returned, else false
        // see bindClientVariableToUniform1f()
        // needed for example for client side timer
        bool checkAndSetClientSideBoundVariables(const GLint & location);

        void setWGLBufferData(GLenum target, GLenum type, Buffer * data, GLenum usage);
        
        void setWGLBufferSubData(GLenum target, unsigned offset, unsigned size, Buffer * data);

        // singleton instance
        static WtWrapper * s_instance;

        // Wt widget to which the gl calls are forwarded
        Wt::WGLWidget * m_wglwidget;

        RenderPhase m_renderPhase;

        // disable warning
#pragma warning (push)
#pragma warning (disable: 4251)  


        // buffer storage need to store since we dont know the data type until usage
        typedef std::map<GLuint, Buffer*> BufferList;
        BufferList m_bufferList;
        // global counter of used buffers
        // TODO: issues: if a buffer is deleted, it wont be reused. May flip to zero in long running applications 
        GLuint m_nextBufferID; 


        // buffer storage need to store since we dont know the data type until usage
        typedef std::map<GLuint, Framebuffer*> FramebufferList;
        FramebufferList m_framebufferList;
        // global counter of used buffers
        GLuint m_nextFramebufferID; 

        FramebufferList m_renderbufferList;
        // global counter of used buffers
        GLuint m_nextRenderbufferID; 


        // mapping of shaders from GL ids to WT ids
        typedef std::vector<Shader*> ShaderList;
        // TODO: use a map here: typedef std::map<GLuint, Shader*> ShaderList;
        ShaderList m_glShader2WtID;

        // mapping of programs from GL ids to WT ids
        typedef std::vector<Program*> ProgramList;
        ProgramList m_glProgramList;

        // mapping of textures from GL ids to WT ids
        typedef std::vector<Texture*> TextureList;
        TextureList m_glTextureList;

        //////////////////////////////////////////////////////////////////////////
        // client side arrays workaround

        // active client side pointers list for each vertex attribute
        typedef std::pair<ClientPointer *, bool> ActivateableAttribute;
        typedef std::vector<ActivateableAttribute> ActiveClientSideAttributes;
        ActiveClientSideAttributes m_activeClientSideAttributes;
        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        // optimized transfer of binary buffers: do not transfer single buffers
        // but wait until a certain size is reached
        // storage space for binary buffers
        std::vector<unsigned char> m_currentAggregationBuffer;
        static const unsigned int MaxBufferAggregationSize = 512 * 1024; // 512 KB
        
        // wmemoryresource to be filled when the MaxAggregationSize is reached
        Wt::WMemoryResource * m_currentMemResource;
        std::string m_currentClientBufferResource;
        //////////////////////////////////////////////////////////////////////////

        // currently bound buffer
        GLuint m_activeArrayBuffer;
        GLuint m_activeElementBuffer;

        // currently bound frame buffer
        GLuint m_activeFramebuffer;
        GLuint m_activeRenderbuffer;

        // currently bound shader program
        GLuint m_activeProgram;

        // currently bound texture
        GLuint m_activeTexture;

        // for hack
        std::string m_mvMatrixJS;
        std::string m_projMatrixJS;
        std::string m_normalMatrixJS;

        std::string m_vMatrixJS;
        std::string m_vMatrixJSUniform;

        // if enabled, stores values 
        bool m_enableDebugging;

        const unsigned int m_maxNumVertexAttributes;

        std::pair<std::string, std::string> m_clientSideFloat2Uniform;


#pragma warning (pop)  
    };

} // WTW namespace

#endif // WTWRAPPER_H