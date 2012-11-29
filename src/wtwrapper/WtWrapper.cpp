#include <wtwrapper/WtWrapper.h>

// disable warnings
#pragma warning (push)
#pragma warning (disable: 4251) //("vector needs  dll-interface")
#pragma warning (disable: 4275) //("non dll-interface class used as base for dll-interface")
#include <Wt/WGLWidget>
#include <Wt/WImage>
#include <Wt/WMemoryResource>
#include <Wt/WLink>
#include <Wt/WRasterImage>
#include <Wt/WColor>
#pragma warning (pop) 

#include <iostream>
#include <fstream>
#include <iterator>
#include <limits>

#include "Buffer.h"
#include "Program.h"
#include "Shader.h"
#include "StridingIterator.h"

using namespace WTW;

WtWrapper * WtWrapper::s_instance = 0;

namespace WTW{

    // Utilities used by wtwrapper only.
    Wt::WGLWidget::AttribLocation wtAttrLoc(GLuint loc)
    {
        // just direct mapping
        Wt::WGLWidget::AttribLocation wtLoc = boost::lexical_cast<std::string>(loc);
        return wtLoc;
    }

    struct ClientPointer{

        ClientPointer(GLenum target, GLenum usage, GLenum type, const GLvoid* pointer)
            :
        m_numElements(0), 
            m_wtBuffer("null"), 
            m_target(target), 
            m_pointer(pointer),
            m_stride(0),
            m_type(type),
            m_usage(usage)
        {
        }

        GLenum m_type;
        GLuint index;
        GLint size;
        GLboolean normalized;
        GLsizei m_stride;
        const GLvoid *m_pointer;
        bool isActive;

        GLenum m_usage;

        GLenum m_target;

        // number of elements (e.g. vertices)
        GLsizei m_numElements;
        Wt::WGLWidget::Buffer m_wtBuffer;

        void setData(GLuint index_, GLint size_, GLenum type_, GLboolean normalized_, GLsizei stride_, const GLvoid *pointer_)
        {
            this->index = index_;
            this->m_type=type_;
            this->size=size_;
            this->normalized=normalized_;
            this->m_stride = stride_;
            this->isActive=true;
            this->m_pointer= (GLvoid*)pointer_;
        }
    };

    struct Texture
    {
        Texture(const Wt::WGLWidget::Texture & wtTex):m_wtTexture(wtTex){}

        Wt::WGLWidget::Texture m_wtTexture;
    };
}

void setWGLBufferSubData(Wt::WGLWidget* widget, GLenum target, unsigned offset, unsigned size, Buffer * data);
void setWGLBufferData(Wt::WGLWidget* widget, GLenum target, GLenum type, Buffer * data, GLenum usage);

WtWrapper * WtWrapper::instance()
{
    if (WtWrapper::s_instance == 0)
    {
        s_instance = new WtWrapper();
    }
    return s_instance;
}


WtWrapper::WtWrapper()
:m_wglwidget(0),
m_activeArrayBuffer(0), 
m_activeElementBuffer(0),
m_renderPhase(INIT), 
m_enableDebugging(false),
// TODO: find out exactly at initialization time from the current client
m_maxNumVertexAttributes (16),
m_clientSideFloat2Uniform("", ""),
m_bufferTransferImplementation(AGGREGATED_BINARY_BUFFER)
{
    reset();
}

Wt::WGLWidget::Buffer createBindSetData(
                                        Wt::WGLWidget * wglWidget,
                                        GLenum target,
                                        GLenum usage,
                                        GLsizei size,               // size in bytes
                                        GLenum type,                
                                        GLsizei numElements,        // number of elements
                                        const GLvoid * pointer,
                                        GLsizei offset = 0,
                                        GLsizei stride = 0
                                        )
{
    // create a new buffer and set in CSA
    Wt::WGLWidget::Buffer b = wglWidget->createBuffer();

    // bind it
    wglWidget->bindBuffer(
        (Wt::WGLWidget::GLenum)target,
        b);


    switch (type)
    {
    case GL_UNSIGNED_SHORT :
        {
            StridingIterator<GLushort> sit (pointer, stride);
            wglWidget->bufferDataiv(
                (Wt::WGLWidget::GLenum)target,
                sit + (offset * size),
                sit+((numElements+offset) * size),
                (Wt::WGLWidget::GLenum)usage,
                (Wt::WGLWidget::GLenum)type
                );
        }
        break;
    case GL_UNSIGNED_BYTE :
        {
            StridingIterator<GLubyte> sit (pointer, stride);
            wglWidget->bufferDataiv(
                (Wt::WGLWidget::GLenum)target,
                sit + (offset * size),
                sit+(numElements * size),
                (Wt::WGLWidget::GLenum)usage,
                (Wt::WGLWidget::GLenum)type
                );
        }
        break;
    case GL_UNSIGNED_INT :
        {
            // TG: not supported. Revert to short and cross fingers
            if (numElements> 2<<16)
            {
                std::cout << "UNSIGNED INT not supported for buffer data usage, reverting to UNSIGNED_SHORT." << std::endl;
                std::cout << "Warning: too many elements for short" << std::endl;
            }

            StridingIterator<GLuint> sit (pointer, stride);

            wglWidget->bufferDataiv(
                (Wt::WGLWidget::GLenum)target,
                sit + (offset * size),
                sit+(numElements * size),
                (Wt::WGLWidget::GLenum)usage,
                (Wt::WGLWidget::GLenum)GL_UNSIGNED_SHORT
                );
        }
        break;
    case GL_FLOAT :
        {
            StridingIterator<GLfloat> sit (pointer, stride);
            wglWidget->bufferDatafv(
                (Wt::WGLWidget::GLenum)target,
                sit + (offset * size),
                sit+(numElements * size),
                (Wt::WGLWidget::GLenum)usage
                );
        }
        break;
    default:
        // TODO: check the other types ?
        std::cout << "dont support " << type << std::endl;
    }

    return b;
}

void WtWrapper::initClientSideViewTransformUniform(
    const std::string & csViewTfUniformName,   
    const std::string & csViewTf
    )
{
    if (m_wglwidget)
    {
        m_vMatrixJSUniform = csViewTfUniformName;
        m_vMatrixJS = csViewTf;

        for (size_t i = 1, iEnd = m_glProgramList.size(); i<iEnd; i++)
        {
            Program* p = m_glProgramList[i];
            // this creates the client side variable for the location
            int loc = glGetUniformLocation(static_cast<GLuint>(i), csViewTfUniformName.c_str());
        }
    }
}

void WtWrapper::renderClientSideViewTransformUniform(Program* p)
{
    if (m_wglwidget && m_renderPhase == PAINT)
    {       
        int glID = p->getGLIDbyUniformName(m_vMatrixJSUniform);
        if (glID == Program::UNDEFINED)
        {
            std::cout << "client side view tf needs to be initialized before calling renderClientSideViewTransformUniform" << std::endl;
            return;
        }

        m_wglwidget->uniformMatrix4(
            p->getUniformLocation(glID),
            Wt::WGLWidget::JavaScriptMatrix4x4(m_vMatrixJS));
    }
}

void WtWrapper::glBindBuffer(GLenum target, GLuint buffer)
{
    if (m_wglwidget)
    {
        switch (target)
        {
        case GL_ELEMENT_ARRAY_BUFFER:
            m_activeElementBuffer = buffer;
            break;
        case GL_ARRAY_BUFFER:
        default:
            m_activeArrayBuffer = buffer;
            break;
        }

        m_wglwidget->bindBuffer(
            (Wt::WGLWidget::GLenum)target,
            m_bufferList[buffer]->getWtBuffer());
    }
}

void WtWrapper::glBindFramebuffer(GLenum target, GLuint framebuffer)
{
    if (m_wglwidget)
    {
        m_wglwidget->bindFramebuffer(
            (Wt::WGLWidget::GLenum)target,
            m_framebufferList[framebuffer]->wtFramebuffer);
    }
}

void WtWrapper::glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
    if (m_wglwidget)
    {
        m_wglwidget->bindRenderbuffer(
            (Wt::WGLWidget::GLenum)target,
            m_renderbufferList[renderbuffer]->wtFramebuffer);
    }
}

void WtWrapper::glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    //http://www.khronos.org/opengles/sdk/docs/man/xhtml/glRenderbufferStorage.xml

    // internal format may only be one of the following:
    // Must be one of the following symbolic constants: GL_RGBA4, GL_RGB565, GL_RGB5_A1, GL_DEPTH_COMPONENT16, or GL_STENCIL_INDEX8.

    // downgrade
    GLenum myInternalformat;

    switch (internalformat)
    {
    case 0x081A6: //GL_DEPTH_COMPONENT24
    	myInternalformat = GL_DEPTH_COMPONENT16;
        break;
     case GL_RGBA4:
     case GL_RGB565:
     case GL_RGB5_A1:
     case GL_DEPTH_COMPONENT16:
     case GL_STENCIL_INDEX8:
         // directly supported by GLES2 :  do not change
         myInternalformat = internalformat;
         break;
    default:
        std::cout << "Renderbufferstorage: internal format not supported: " << internalformat << std::endl;
    }

    if (m_wglwidget)
    {
        m_wglwidget->renderbufferStorage(
            (Wt::WGLWidget::GLenum) target, 
            (Wt::WGLWidget::GLenum) myInternalformat, 
            width, height);
    }
}

void WtWrapper::glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbufferTarget, GLuint renderbuffer)
{
    // http://www.khronos.org/opengles/sdk/docs/man/xhtml/glFramebufferRenderbuffer.xml
    if (m_wglwidget)
    {
        m_wglwidget->framebufferRenderbuffer(
            (Wt::WGLWidget::GLenum) target, 
            (Wt::WGLWidget::GLenum) attachment, 
            (Wt::WGLWidget::GLenum) renderbufferTarget, 
            m_renderbufferList[renderbuffer]->wtFramebuffer);
    }
}

void WtWrapper::glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLuint level)
{
    if (m_wglwidget)
    {
        m_wglwidget->framebufferTexture2D(
            (Wt::WGLWidget::GLenum) target, 
            (Wt::WGLWidget::GLenum) attachment, 
            (Wt::WGLWidget::GLenum) textarget,
            m_glTextureList[texture]->m_wtTexture,
            level
            );
    }
}

void WtWrapper::glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
    if (m_wglwidget)
    {
        // if not existing in map, it will be created on first reference
        switch (target)
        {
        case GL_ELEMENT_ARRAY_BUFFER:
            m_bufferList[m_activeElementBuffer]->setData(target, size, data, usage);
            setWGLBufferData(
                target,
                GL_UNSIGNED_BYTE,
                m_bufferList[m_activeElementBuffer],
                usage
                );
            break;
        case GL_ARRAY_BUFFER:
        default:
            m_bufferList[m_activeArrayBuffer]->setData(target, size, data, usage);
            setWGLBufferData(
                target,
                GL_UNSIGNED_BYTE,
                m_bufferList[m_activeArrayBuffer],
                usage
                );
            break;
        }
    }
}

void WtWrapper::glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)
{
    if (m_wglwidget)
    {
        // assuming an active buffer
        switch (target)
        {
        case GL_ELEMENT_ARRAY_BUFFER:
            m_bufferList[m_activeElementBuffer]->setSubData(offset, size, data);
            // apply directly
            setWGLBufferSubData(
                target,
                offset,
                size,
                m_bufferList[m_activeElementBuffer]
            );
            break;
        case GL_ARRAY_BUFFER:
        default:
            m_bufferList[m_activeArrayBuffer]->setSubData(offset, size, data);
            // apply directly
            setWGLBufferSubData(
                target,
                offset,
                size,
                m_bufferList[m_activeArrayBuffer]
            );
            break;
        }
    }
}


void WtWrapper::glDeleteBuffers(GLsizei n, const GLuint *buffer)
{
    if (m_wglwidget && m_renderPhase != PAINT)
    {
        for (GLuint i = 0, iEnd = n; i<iEnd; i++)
        {
            // spec: glDeleteBuffers silently ignores 0's and names that do not correspond to existing buffer objects.
            if (m_bufferList.count(buffer[i]) == 0 || buffer[i] == 0)
            {
                continue;
            }

            m_wglwidget->deleteBuffer(m_bufferList[buffer[i]]->getWtBuffer());
            delete m_bufferList[buffer[i]]; // delete object
            m_bufferList.erase(buffer[i]);  // delete entry in list
        }
    }
}

void WtWrapper::glDeleteTextures(GLsizei n, const GLuint* textures)
{
    if (m_wglwidget && m_renderPhase != PAINT)
    {
        for (GLuint i = 0, iEnd = n; i<iEnd; i++)
        {

            m_wglwidget->deleteTexture(m_glTextureList[textures[i]]->m_wtTexture);
            delete m_glTextureList[textures[i]]; // delete object

            // TG: we cannot remove an entry without invalidating all the 
            // following entry indices. hence we just set it to zero
            m_glTextureList[textures[i]] = (Texture*)NULL;  // delete entry in list
        }
    }
}


void WtWrapper::glGenBuffers(GLsizei n, GLuint *buffer)
{

    if (m_wglwidget)
    {
        for (GLuint i = 0, iEnd = n, counter=0; i<iEnd; i++, counter++)
        {
            unsigned int currentBufferID = m_nextBufferID++;
            //std::cout << "\tgenBuffer (" << m_lastBufferID <<")" << std::endl;
            const std::string b = m_wglwidget->createBuffer();
            m_bufferList[currentBufferID] = new Buffer(b);
            buffer[counter] = currentBufferID;
        }
    }
}

void WtWrapper::glGenFramebuffers(GLsizei n, GLuint* framebuffers)
{
    if (m_wglwidget)
    {
        for (GLuint i = 0, iEnd = n, counter=0; i<iEnd; i++, counter++)
        {
            unsigned int currentFramebufferID = m_nextFramebufferID++;
            const std::string b = m_wglwidget->createFramebuffer();
            m_framebufferList[currentFramebufferID] = new Framebuffer(b);
            framebuffers[counter] = currentFramebufferID;
        }
    }
}

void WtWrapper::glGenRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
    if (m_wglwidget)
    {
        for (GLuint i = 0, iEnd = n, counter=0; i<iEnd; i++, counter++)
        {
            unsigned int currentRenderbufferID = m_nextRenderbufferID++;
            const std::string b = m_wglwidget->createRenderbuffer();
            m_renderbufferList[currentRenderbufferID] = new Framebuffer(b);
            renderbuffers[counter] = currentRenderbufferID;
        }
    }
}

void WtWrapper::glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    if (m_wglwidget)
    {
        m_wglwidget->clearColor(red, green, blue, alpha);
    }

}

void WtWrapper::glEnable(GLenum cap)
{
    if (m_wglwidget)
    {
        {
            switch (cap)
            {
            case GL_TEXTURE_2D:
                break; // not necessary and not supported by WebGL
            case GL_CULL_FACE:
            case GL_BLEND:
            case GL_DITHER:
            case GL_STENCIL_TEST:
            case GL_DEPTH_TEST:
            case GL_SCISSOR_TEST:
            case GL_POLYGON_OFFSET_FILL:
            case GL_SAMPLE_ALPHA_TO_COVERAGE:
            case GL_SAMPLE_COVERAGE:
                m_wglwidget->enable((Wt::WGLWidget::GLenum)cap);
                break;
            default:
                std::cout << "Not supported for glEnable: " << cap << std::endl;

            }
        }
    }
}

void WtWrapper::glClearDepthf(GLclampf depth)
{
    if (m_wglwidget)
    {
        m_wglwidget->clearDepth(depth);
    }
}

void WtWrapper::glDepthFunc(GLenum func)
{
    if (m_wglwidget)
    {
        m_wglwidget->depthFunc((Wt::WGLWidget::GLenum)func);
    }
}

void WtWrapper::setupDrawCSARender(unsigned int minIndex, unsigned int maxIndex)
{
    //////////////////////////////////////////////////////////////////////////
    // workaround for client side arrays:
    // - for all active Attributes (in list, set on)
    //     ○ in the CSA
    //     ○ create buffer, bind, 
    //     o setvertexattribpointer

    for (size_t i = 0, iEnd = m_activeClientSideAttributes.size(); i<iEnd; i++)
    {
        // is it active and valid?
        if (m_activeClientSideAttributes[i].second && m_activeClientSideAttributes[i].first != NULL)
        {
            static bool warnedOnce = false;
            if (!warnedOnce)
            {
                std::cout << "Performance Warning: Using slow client side arrays implementation!" << std::endl;
                warnedOnce = true;
            }


            ClientPointer * csa = m_activeClientSideAttributes[i].first;

            // create buffer
            Wt::WGLWidget::Buffer b = createBindSetData(
                m_wglwidget,
                csa->m_target,
                csa->m_usage,
                csa->size,
                csa->m_type,
                maxIndex+1,//maxIndex-minIndex+1,    // num elements
                csa->m_pointer,
                0,//minIndex,
                csa->m_stride);
            csa->m_wtBuffer = b;

            m_wglwidget->vertexAttribPointer(
                wtAttrLoc(static_cast<GLuint>(i)),
                csa->size,
                (Wt::WGLWidget::GLenum)csa->m_type,
                csa->normalized == 1,
                0,//csa->stride, // set to zero as we recreated a new buffer just for this attribute
                0);

            //unbind
            m_wglwidget->bindBuffer(
                Wt::WGLWidget::ARRAY_BUFFER,
                "null");
        }
    }
}


void WtWrapper::deleteDrawCSARender()
{
    //////////////////////////////////////////////////////////////////////////
    // workaround for client side arrays:
    // - for all active Attributes (in list, set on)
    //     ○ in the CSA
    //     ○ bind, delete buffer

    for (size_t i = 0, iEnd = m_activeClientSideAttributes.size(); i<iEnd; i++)
    {
        // is it active and valid?
        if (m_activeClientSideAttributes[i].second && m_activeClientSideAttributes[i].first != NULL)
        {
            ClientPointer * csa = m_activeClientSideAttributes[i].first;

            // delete buffer
            m_wglwidget->deleteBuffer(csa->m_wtBuffer);
        }
    }
}

void WtWrapper::glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    static bool warnedOnce = false;
    if (!warnedOnce && (mode == 9 || mode == 7 || mode == 8 ))
    {
        warnedOnce = true;
        std::cout << "WebGL does not support GL_POLYGON, GL_QUADS, or GL_QUAD_STRIP types" << std::endl;
    }

    if (m_wglwidget)
    {
        if (m_renderPhase != PAINT)
        {
            //setupDrawCSAInit(first, count);
        }

        {
            setupDrawCSARender(first, first + count-1);
            m_wglwidget->drawArrays((Wt::WGLWidget::GLenum)mode, first, count);
            deleteDrawCSARender();
        }
    }
}

void WtWrapper::setWGLBufferData(GLenum target, GLenum type, Buffer * data, GLenum usage)
{

    // handle buffer allocations without data given (i.e. NULL passed as data)
    if (data->dataIsNull())
    {
        GLenum target2, usage2;
        GLsizeiptr size;
        data->getData(target2,size,0,usage2);
        m_wglwidget->bufferData(
            (Wt::WGLWidget::GLenum)target,
            size,
            (Wt::WGLWidget::GLenum)usage);
    }else 
    {
        //////////////////////////////////////////////////////////////////////////
        std::vector<unsigned char> & mem = (std::vector<unsigned char> &)(data->getRawData());

        if (m_bufferTransferImplementation == AGGREGATED_BINARY_BUFFER)
        {
            //////////////////////////////////////////////////////////////////////////
            // this code collects buffers in a binary file until a size was exceeded

            std::string url;

            unsigned int aggOffset = 0;

            // check if a memory resource exists AND we can squeeze the next bufferresource into the current MemoryResource
            if (m_currentMemResource && m_currentAggregationBuffer.size() + mem.size() <MaxBufferAggregationSize)
            {
                aggOffset = static_cast<unsigned int>(m_currentAggregationBuffer.size());
                url = m_currentMemResource->url();
            }else
            {
                // and create a new MemoryResource
                m_currentMemResource = new Wt::WMemoryResource("application/octet-stream", m_wglwidget);

                m_currentAggregationBuffer.clear();
                m_currentAggregationBuffer.reserve(MaxBufferAggregationSize);
                url = m_currentMemResource->generateUrl();

                // setup for preloading and get a JS identifier
                m_currentClientBufferResource = m_wglwidget->createAndLoadBufferResource(url);
            }
            m_currentAggregationBuffer.insert(
                m_currentAggregationBuffer.begin() + aggOffset , mem.begin(), mem.end());

            // TG: test: set the data everytime (probably too many copy operations)
            m_currentMemResource->setData(m_currentAggregationBuffer);

            m_wglwidget->bufferData(
                (Wt::WGLWidget::GLenum)target,
                m_currentClientBufferResource,
                aggOffset, static_cast<unsigned int>(mem.size()),
                (Wt::WGLWidget::GLenum)usage);
        }
        else if (m_bufferTransferImplementation == BINARY_BUFFER)
        {
            //////////////////////////////////////////////////////////////////////////
            // this code transfers each buffer as an individual binary file
            Wt::WMemoryResource * memRes = new Wt::WMemoryResource("application/octet-stream", m_wglwidget);
            // buffer is the full buffer, so I need to use the offset here
            memRes->setData(&(mem[0]), static_cast<unsigned int>(mem.size()));
            std::string memUrl = memRes->generateUrl();

            Wt::WGLWidget::BufferResource bufRes = m_wglwidget->createAndLoadBufferResource(memRes->generateUrl());

            m_wglwidget->bufferData(
                (Wt::WGLWidget::GLenum)target,bufRes,(Wt::WGLWidget::GLenum)usage);
        }else //m_bufferTransfer == STRING_BUFFER
        {
            //////////////////////////////////////////////////////////////////////////
            // this code generates the string for the array in place of the JS
            m_wglwidget->bufferDataiv(
                (Wt::WGLWidget::GLenum)target,
                data->first<GLubyte>(),
                data->last<GLubyte>(),
                (Wt::WGLWidget::GLenum)usage,
                (Wt::WGLWidget::GLenum)type);
        }
        return;
    }
}

void WtWrapper::setWGLBufferSubData(GLenum target, unsigned offset, unsigned size, Buffer * data)
{
    std::vector<unsigned char> & mem = (std::vector<unsigned char> &)(data->getRawData());

    if (m_bufferTransferImplementation == AGGREGATED_BINARY_BUFFER)
    {
        //////////////////////////////////////////////////////////////////////////
        // this code collects buffers in a binary file until a size was exceeded
        
        std::string url;

        unsigned int aggOffset = 0;

        // check if a memory resource exists AND we can squeeze the next bufferresource into the current MemoryResource
        if (m_currentMemResource && m_currentAggregationBuffer.size() + size <MaxBufferAggregationSize)
        {
            aggOffset = static_cast<unsigned int>(m_currentAggregationBuffer.size());
            url = m_currentMemResource->url();
        }else
        {
            // and create a new MemoryResource
            m_currentMemResource = new Wt::WMemoryResource("application/octet-stream", m_wglwidget);

            m_currentAggregationBuffer.clear();
            m_currentAggregationBuffer.reserve(MaxBufferAggregationSize);
            url = m_currentMemResource->generateUrl();

            // setup for preloading and get a JS identifier
            m_currentClientBufferResource = m_wglwidget->createAndLoadBufferResource(url);
        }
        m_currentAggregationBuffer.insert(
            m_currentAggregationBuffer.begin() + aggOffset , mem.begin() + offset, mem.begin() + offset + size);
            
        // TG: set the data everytime (probably too many copy operations)
        m_currentMemResource->setData(m_currentAggregationBuffer);

        m_wglwidget->bufferSubData(
            (Wt::WGLWidget::GLenum)target,
            offset,
            m_currentClientBufferResource,
            aggOffset,
            size);
    }
    else if (m_bufferTransferImplementation == BINARY_BUFFER)
    {
        //////////////////////////////////////////////////////////////////////////
        // this code transfers each buffer as an individual binary file
        Wt::WMemoryResource * memRes = new Wt::WMemoryResource("application/octet-stream", m_wglwidget);
        // buffer is the full buffer, so I need to use the offset here
        memRes->setData(&(mem[0]) + offset, size);
        std::string memUrl = memRes->generateUrl();

        Wt::WGLWidget::BufferResource bufRes = m_wglwidget->createAndLoadBufferResource(memRes->generateUrl());

        m_wglwidget->bufferSubData(
            (Wt::WGLWidget::GLenum)target,
            offset,
            bufRes);
    }else //m_bufferTransfer == STRING_BUFFER
    {
        //////////////////////////////////////////////////////////////////////////
        // this code generates the string for the array in place of the JS
        m_wglwidget->bufferSubDataiv(
            (Wt::WGLWidget::GLenum)target,
            offset,
            data->first<GLubyte>()+offset,
            data->first<GLubyte>()+offset+size,
            (Wt::WGLWidget::GLenum)GL_UNSIGNED_BYTE);
    }


    return;
}

template<typename T>
const T* lastElement(const GLvoid * data, GLsizei sizeInBytes)
{
    // get the first element
    GLbyte * first = (GLbyte*)data;
    // get pointer just behind the last element
    GLbyte * last = &first[sizeInBytes];
    // cast to target
    const T* lastCasted = reinterpret_cast<const T*>(last);

    return lastCasted;
}

template<typename ITERATOR>
void minMax(ITERATOR &it,ITERATOR & end, unsigned int & minIndex, unsigned int & maxIndex )
{
    while (it!= end)
    {
        minIndex = std::min(minIndex, (unsigned int)*it);
        maxIndex = std::max(maxIndex, (unsigned int)*it);
        ++it;
    }
}

void getMinMax(GLsizei count, GLenum type, const GLvoid *indices, unsigned int & minIndex, unsigned int & maxIndex)
{

    switch (type)
    {
    case GL_UNSIGNED_SHORT :
    case GL_UNSIGNED_BYTE :
    case GL_UNSIGNED_INT :
        {
            StridingIterator<GLushort> sit (indices);
            StridingIterator<GLushort> sitEnd =  sit+(count);
            minMax(sit,sitEnd,minIndex, maxIndex);
        }
        break;
    default:
        std::cout << "dont support " << type << std::endl;
    }
}

void WtWrapper::glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    // warn if Quads, Quadstrips or polygons are used
    static bool warnedOnce = false;
    if (mode == 9 || mode == 7 || mode == 8 )
    {
        if (!warnedOnce )
        {
            warnedOnce = true;
            std::cout << "WebGL does not support GL_POLYGON, GL_QUADS, or GL_QUAD_STRIP types, reverting to TRIANGLES" << std::endl;
        }
        if (mode == 9) mode = GL_TRIANGLE_FAN; // convert polygons to triangles
        else if (mode == 7) mode = GL_TRIANGLES;
        else if (mode == 8) mode = GL_TRIANGLES;
    }

    // possible usage:
    // - indices point to array. Not supported by WebGL (client side arrays), need to be worked around
    // - indices are an offset into index buffer bound before

    // In init phase, we check for the first case and store the data in a buffer
    // in render phase, in the first case, we bind this buffer, in the second case we just assume a bound buffer

    if (m_wglwidget)
    {
        unsigned int minIndex = std::numeric_limits<unsigned int>::max(), maxIndex = 0;

        if (m_renderPhase != PAINT)
        {        
            // if no buffer is bound as ElementArray, assume pointer to data
            if (m_activeElementBuffer == 0)
            {
            }else
            {                 
                if (!m_bufferList[m_activeElementBuffer]->isWtBufferSetup())
                {
                    // find out min max
                    getMinMax(
                        count,
                        type,
                        m_bufferList[m_activeElementBuffer]->first<unsigned char*>(),
                        minIndex,
                        maxIndex);
                }
            }
        }

        // TG: This is a bit of a problem: some osg mechanisms have to draw exactly once, e.g. the
        // overlay node renders once into a texture. the overlay subgraph is not traversed a second
        // time, so this gets lost.
        // on the other hand, the Client Side Arrays workaround requires the minimum / maximum
        // index of all vertex attributes to be known... 
        // TODO: remove the separation of init and paint stages for CSA here and in drawArrays, always 
        // apply the buffer with the currently needed range
        if(m_renderPhase == PAINT)
        {
            GLenum typeShortened = type;
            if (type == GL_UNSIGNED_INT)
            {
                // TG: Unsigned int is not supported. Revert to short and cross fingers
                typeShortened = GL_UNSIGNED_SHORT;
            }

            // client side array workaround for elements
            if (m_activeElementBuffer == 0)
            {

                // TG:  WebGL supports no client side arrays.
                //std::cout << "No support for client side arrays" << std::endl;

                // find out min max
                getMinMax(
                    count,
                    type,
                    indices,
                    minIndex,
                    maxIndex);
                setupDrawCSARender(minIndex, maxIndex);

                // create element buffer just for this draw call
                Wt::WGLWidget::Buffer b = createBindSetData(
                    m_wglwidget,
                    GL_ELEMENT_ARRAY_BUFFER,
                    GL_STATIC_DRAW,
                    1,
                    type,
                    count,
                    indices);

                // is still bound 

                // do drawing with zero offset
                m_wglwidget->drawElements((Wt::WGLWidget::GLenum)mode, count, (Wt::WGLWidget::GLenum)typeShortened, (GLsizeiptr)0);

                //unbind and delete element buffer
                m_wglwidget->bindBuffer(
                    Wt::WGLWidget::ELEMENT_ARRAY_BUFFER,
                    "null");

                m_wglwidget->deleteBuffer(b);
            }else
            {

                // find out min max
                getMinMax(
                    count,
                    type,
                    m_bufferList[m_activeElementBuffer]->first<unsigned char*>(),
                    minIndex,
                    maxIndex);

                // client side array workaround for vertex attributes
                setupDrawCSARender(minIndex, maxIndex);

                // do usual drawing
                m_wglwidget->drawElements((Wt::WGLWidget::GLenum)mode, count, (Wt::WGLWidget::GLenum)typeShortened, (GLsizeiptr)indices);
            }
            deleteDrawCSARender();
        }
    }

}

void WtWrapper::glPixelStorei(GLenum pname, GLint param)
{
    if (m_wglwidget)
    {
        if (m_renderPhase != PAINT){
            m_wglwidget->pixelStorei((Wt::WGLWidget::GLenum)pname, (Wt::WGLWidget::GLenum)param);
        }
    }
}

void WtWrapper::glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    if (m_wglwidget)
    {
        m_wglwidget->blendFunc((Wt::WGLWidget::GLenum)sfactor, (Wt::WGLWidget::GLenum)dfactor);
    }
}

void WtWrapper::glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    if (m_wglwidget)
    {
        m_wglwidget->colorMask(red==GL_TRUE,green==GL_TRUE,blue==GL_TRUE,alpha==GL_TRUE);
    }
}

void WtWrapper::glDepthMask(GLboolean flag)
{
    if (m_wglwidget)
    {
        m_wglwidget->depthMask(flag == GL_TRUE);
    }
}

void WtWrapper::glDisable(GLenum cap)
{
    // TG: only allow the following (taken from  http://www.khronos.org/opengles/sdk/docs/man/xhtml/glEnable.xml)
    if (m_wglwidget)
    {
        {
            switch (cap)
            {
            case GL_TEXTURE_2D:
                break; // not necessary and not supported by WebGL
            case GL_BLEND:
            case GL_CULL_FACE:
            case GL_DEPTH_TEST:
            case GL_DITHER:
            case GL_POLYGON_OFFSET_FILL:
            case GL_SAMPLE_ALPHA_TO_COVERAGE:
            case GL_SAMPLE_COVERAGE:
            case GL_SCISSOR_TEST:
            case GL_STENCIL_TEST:
                m_wglwidget->disable((Wt::WGLWidget::GLenum)cap);
                break;
            default:
                std::cout << "Not supported for glDisable: " << cap << std::endl;
            }
        }
    }
}

void WtWrapper::glDisableVertexAttribArray(GLuint index)
{
    if (m_wglwidget)
    {
        m_wglwidget->disableVertexAttribArray(wtAttrLoc(index));
        m_activeClientSideAttributes[index].second = false;
    }
}

void WtWrapper::glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    if (m_wglwidget)
    {
        {
            m_wglwidget->scissor(x,y,  static_cast<unsigned int>(width), static_cast<unsigned int>(height));
        }
    }
}

void WtWrapper::glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    if (m_wglwidget)
    {
        {
            m_wglwidget->viewport(x,y,static_cast<unsigned int>(width), static_cast<unsigned int>(height));
        }
    }
}

void WtWrapper::glVertexAttrib1f(GLuint index, GLfloat x){
    if (m_wglwidget)
    {
        m_wglwidget->vertexAttrib1f(wtAttrLoc(index), x);
    }
}

void WtWrapper::glVertexAttrib1fv(GLuint index, const GLfloat* values){
    if (m_wglwidget)
    {
        m_wglwidget->vertexAttrib1fv(wtAttrLoc(index), values);
    }
}

void WtWrapper::glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y){
    if (m_wglwidget)
    {
        m_wglwidget->vertexAttrib2f(wtAttrLoc(index), x, y);
    }
}

void WtWrapper::glVertexAttrib2fv(GLuint index, const GLfloat* values){
    if (m_wglwidget)
    {
        m_wglwidget->vertexAttrib2fv(wtAttrLoc(index), values);
    }
}

void WtWrapper::glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z){
    if (m_wglwidget)
    {
        m_wglwidget->vertexAttrib3f(wtAttrLoc(index), x, y, z);
    }
}

void WtWrapper::glVertexAttrib3fv(GLuint index, const GLfloat* values){
    if (m_wglwidget)
    {
        m_wglwidget->vertexAttrib3fv(wtAttrLoc(index), values);
    }
}

void WtWrapper::glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w){
    if (m_wglwidget)
    {
        m_wglwidget->vertexAttrib4f(wtAttrLoc(index), x, y, z, w);
    }
}

void WtWrapper::glVertexAttrib4fv(GLuint index, const GLfloat* values){
    if (m_wglwidget)
    {
        m_wglwidget->vertexAttrib4fv(wtAttrLoc(index), values);
    }
}

bool WtWrapper::checkAndSetClientSideBoundVariables(const GLint & location)
{
    if (m_clientSideFloat2Uniform.first != "" && m_glProgramList[m_activeProgram]->getGLIDbyUniformName(m_clientSideFloat2Uniform.second)==location)
    {   
        std::string customSetUniformCode;

        customSetUniformCode = " //TG custom code \n";
        customSetUniformCode += "ctx.uniform1f(";
        customSetUniformCode += m_glProgramList[m_activeProgram]->getUniformLocation(location) + ",";
        customSetUniformCode += m_clientSideFloat2Uniform.first + ");";
        m_wglwidget->injectJS(customSetUniformCode);
        return true;
    }else return false;
}

void WtWrapper::glUniform1f(GLint location, GLfloat x)
{
    if (m_wglwidget)
    {
        // if clientside variable has been bound to uniform
        // check if the bound uniform name is the same as the current location
        if (!checkAndSetClientSideBoundVariables(location))
        {   
            m_wglwidget->uniform1f(
                m_glProgramList[m_activeProgram]->getUniformLocation(location),
                x);
        }
    }
}

void WtWrapper::glUniform1fv(GLint location, GLsizei count,const GLfloat *v)
{
    if (m_wglwidget)
    {        if (!checkAndSetClientSideBoundVariables(location))
        {
            m_wglwidget->uniform1fv(
                m_glProgramList[m_activeProgram]->getUniformLocation(location),
                count,
                v);
        }
    }
}

void WtWrapper::glUniform1i(GLint location, GLint x)
{
    if (m_wglwidget)
    {

        m_wglwidget->uniform1i(
            m_glProgramList[m_activeProgram]->getUniformLocation(location),
            x);
    }
}

void WtWrapper::glUniform1iv(GLint location,GLsizei count,const GLint *v)
{
    if (m_wglwidget)
    {
        m_wglwidget->uniform1iv(
            m_glProgramList[m_activeProgram]->getUniformLocation(location),
            count,
            v);
    }
}

void WtWrapper::glUniform2f(GLint location, GLfloat x, GLfloat y)
{
    if (m_wglwidget)
    {

        m_wglwidget->uniform2f(
            m_glProgramList[m_activeProgram]->getUniformLocation(location),
            x, y);

    }
}

void WtWrapper::glUniform2fv(GLint location, GLsizei count,const GLfloat *v)
{
    if (m_wglwidget)
    {
        m_wglwidget->uniform2fv(
            m_glProgramList[m_activeProgram]->getUniformLocation(location),
            count,
            v);
    }
}

void WtWrapper::glUniform2i(GLint location, GLint x, GLint y)
{
    if (m_wglwidget)
    {
        m_wglwidget->uniform2i(
            m_glProgramList[m_activeProgram]->getUniformLocation(location),
            x, y);
    }
}

void WtWrapper::glUniform2iv(GLint location,GLsizei count,const GLint *v)
{
    if (m_wglwidget)
    {
        m_wglwidget->uniform2iv(
            m_glProgramList[m_activeProgram]->getUniformLocation(location),
            count,
            v);
    }
}

//

void WtWrapper::glUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    if (m_wglwidget)
    {
        m_wglwidget->uniform3f(
            m_glProgramList[m_activeProgram]->getUniformLocation(location),
            x, y, z);
    }
}

void WtWrapper::glUniform3fv(GLint location, GLsizei count,const GLfloat *v)
{
    if (m_wglwidget)
    {
        m_wglwidget->uniform3fv(
            m_glProgramList[m_activeProgram]->getUniformLocation(location),
            count,
            v);
    }
}

void WtWrapper::glUniform3i(GLint location, GLint x, GLint y, GLint z)
{
    if (m_wglwidget)
    {
        m_wglwidget->uniform3i(
            m_glProgramList[m_activeProgram]->getUniformLocation(location),
            x, y, z);
    }
}

void WtWrapper::glUniform3iv(GLint location,GLsizei count,const GLint *v)
{
    if (m_wglwidget)
    {
        m_wglwidget->uniform3iv(
            m_glProgramList[m_activeProgram]->getUniformLocation(location),
            count,
            v);
    }
}

void WtWrapper::glUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    if (m_wglwidget)
    {
        m_wglwidget->uniform4f(
            m_glProgramList[m_activeProgram]->getUniformLocation(location),
            x, y, z, w);
    }
}

void WtWrapper::glUniform4fv(GLint location, GLsizei count,const GLfloat *v)
{
    if (m_wglwidget)
    {
        m_wglwidget->uniform4fv(
            m_glProgramList[m_activeProgram]->getUniformLocation(location),
            count,
            v);
    }
}

void WtWrapper::glUniform4i(GLint location, GLint x, GLint y, GLint z, GLint w)
{
    if (m_wglwidget)
    {
        m_wglwidget->uniform4i(
            m_glProgramList[m_activeProgram]->getUniformLocation(location),
            x, y, z, w);
    }
}

void WtWrapper::glUniform4iv(GLint location,GLsizei count,const GLint *v)
{
    if (m_wglwidget)
    {
        m_wglwidget->uniform4iv(
            m_glProgramList[m_activeProgram]->getUniformLocation(location),
            count,
            v);
    }
}

void WtWrapper::glBlendColor(GLclampf red,GLclampf green,GLclampf blue,GLclampf alpha)
{
    if (m_wglwidget)
    {
        m_wglwidget->blendColor(red,green,blue,alpha);
    }
}

void WtWrapper::glBlendEquation(GLenum mode)
{
    if (m_wglwidget)
    {
        m_wglwidget->blendEquation((Wt::WGLWidget::GLenum)mode);
    }
}

void WtWrapper::glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
    if (m_wglwidget)
    {
        m_wglwidget->blendEquationSeparate((Wt::WGLWidget::GLenum)modeRGB,(Wt::WGLWidget::GLenum)modeAlpha);
    }
}

void WtWrapper::glClearStencil(GLint s)
{
    if (m_wglwidget)
    {
        m_wglwidget->clearStencil(s);
    }
}


void WtWrapper::glCullFace(GLenum mode)
{
    if (m_wglwidget)
    {
        m_wglwidget->cullFace((Wt::WGLWidget::GLenum) mode);
    }
}

void WtWrapper::glDepthRange(GLclampf zNear, GLclampf zFar)
{
    if (m_wglwidget)
    {
        m_wglwidget->depthRange(zNear,zFar);
    }
}

void WtWrapper::glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer)
{
    //
    if (m_wglwidget)
    {
        // if no buffer is bound, assume pointer to data
        if (m_activeArrayBuffer == 0)
        {
            // TG:  WebGL supports no client side arrays. 
            // workaround: store pointer and store it as buffer in WT when drawingit
            // reason ist that we dont know the number of elements
            if (m_renderPhase == PAINT)
            {
                ClientPointer* csp = new ClientPointer(
                    GL_ARRAY_BUFFER, 
                    GL_STATIC_DRAW, 
                    type,
                    pointer);
                csp->setData(index, size, type, normalized, stride, pointer);

                // delete existing clientpointer
                if (m_activeClientSideAttributes[index].first != NULL)
                {
                    delete m_activeClientSideAttributes[index].first;
                }

                // set as active attribute
                m_activeClientSideAttributes[index].first = csp;
                m_activeClientSideAttributes[index].second = true;
            }
        }else{

            // else assume pointer giving just the offset, 
            // deactivate potential client side arrays for this position
            m_activeClientSideAttributes[index].second = false;

            // TG: need to call it also in the init stage because OSG remembers
            // the state. If I would not set it here, it would not be set
            // in the next call of frame()
            {
                // now do the real call of glVertexAttribPointer
                m_wglwidget->vertexAttribPointer(
                    wtAttrLoc(index),
                    size,
                    (Wt::WGLWidget::GLenum)type,
                    normalized == 1,    // to get rid of compiler performance warning forcing GLboolean to bool
                    stride,
                    (GLsizeiptr)pointer);
            }
        }
    }
}


void WtWrapper::free()
{
    // TODO: free wrasterimages and buffers


    // free buffers
    for (size_t i = 0, iEnd = m_bufferList.size(); i<iEnd; i++)
    {
        delete m_bufferList[static_cast<const unsigned int>(i)];
    }
    m_bufferList.clear();
    m_nextBufferID = 0;

    // free framebuffers and render buffers
    for (size_t i = 0, iEnd = m_framebufferList.size(); i<iEnd; i++)
    {
        delete m_framebufferList[static_cast<const unsigned int>(i)];
    }
    m_framebufferList.clear();
    m_nextFramebufferID = 0;

    for (size_t i = 0, iEnd = m_renderbufferList.size(); i<iEnd; i++)
    {
        delete m_renderbufferList[static_cast<const unsigned int>(i)];
    }
    m_renderbufferList.clear();
    m_nextRenderbufferID = 0;

    // free programs
    for (size_t i = 0, iEnd = m_glProgramList.size(); i<iEnd; i++)
    {
        delete m_glProgramList[i];
    }
    m_glProgramList.clear();

    // free shaders
    for (size_t i = 0, iEnd = m_glShader2WtID.size(); i<iEnd; i++)
    {
        delete m_glShader2WtID[i];
    }
    m_glShader2WtID.clear();

    // free textures
    for (size_t i = 0, iEnd = m_glTextureList.size(); i<iEnd; i++)
    {
        delete m_glTextureList[i];
    }
    m_glTextureList.clear();
}

void WtWrapper::reset()
{
    free();

    m_renderPhase = INIT;

    // initialize buffers
    m_activeArrayBuffer = 0;
    m_activeElementBuffer = 0;

    m_bufferList[0] = new Buffer("null");
    m_nextBufferID++;

    // initialize frame buffer
    m_activeFramebuffer = 0;
    m_framebufferList[0] = new Framebuffer("null");

    m_nextFramebufferID++;

    m_activeRenderbuffer= 0;
    m_renderbufferList[0] = new Framebuffer("null");
    m_nextRenderbufferID++;

    // initialize shaders mapping
    m_glShader2WtID.push_back(new Shader("null",Wt::WGLWidget::VERTEX_SHADER));

    // initialize program mapping
    m_glProgramList.push_back(new Program("null"));

    m_glTextureList.push_back(new Texture("null"));

    // initialize client side arrays structure
    m_activeClientSideAttributes.resize(
        m_maxNumVertexAttributes,
        ActivateableAttribute((WTW::ClientPointer*)NULL,false));

    m_currentAggregationBuffer.clear();
    m_currentAggregationBuffer.reserve(MaxBufferAggregationSize);
    m_currentMemResource = NULL;
}

GLuint WtWrapper::glCreateShader(GLenum type)
{
    if (m_wglwidget)
    {
        const Wt::WGLWidget::Shader s = m_wglwidget->createShader((Wt::WGLWidget::GLenum)type);
        GLuint retVal = static_cast<GLuint>(m_glShader2WtID.size());

        m_glShader2WtID.push_back(new Shader(s,(Wt::WGLWidget::GLenum)type));
        return retVal;
    }
    else return 0;
}

void WtWrapper::glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
{
    if (m_wglwidget)
    {
        // quoting the gles spec:
        // glShaderSource sets the source code in shader to the source code
        // in the array of strings specified by string. Any source code 
        // previously stored in the shader object is completely replaced. 
        // The number of strings in the array is specified by count. If 
        // length is NULL, each string is assumed to be null terminated. If
        // length is a value other than NULL, it points to an array 
        // containing a string length for each of the corresponding 
        // elements of string. Each element in the length array may contain
        // the length of the corresponding string (the null character is 
        // not counted as part of the string length) or a value less than 0
        // to indicate that the string is null terminated. The source code
        // strings are not scanned or parsed at this time; they are simply
        // copied into the specified shader object.

        std::string shdSrc;
        if (length == NULL)
        {
            // each string is null terminated, just add
            for (unsigned int i = 0, iEnd = count; i<iEnd; i++)
            {
                shdSrc += string[i];
            }

        }else
        {
            // not supported at the moment
            std::cout << "Does not support shader source without null terminated strings" << std::endl;
        }

        // parse for active uniforms and attributes
        m_glShader2WtID[shader]->parseShaderSource(shdSrc);

        m_wglwidget->shaderSource(
            m_glShader2WtID[shader]->m_wtShader, 
            shdSrc);
    }
}

void WtWrapper::glCompileShader(GLuint shader)
{
    if (m_wglwidget)
    {
        m_wglwidget->compileShader(m_glShader2WtID[shader]->m_wtShader);
    }
}

void WtWrapper::glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{

}

GLuint WtWrapper::glCreateProgram(void)
{
    if (m_wglwidget)
    {
        const std::string s = m_wglwidget->createProgram();
        GLuint retVal = static_cast<GLuint>(m_glProgramList.size());
        m_glProgramList.push_back(new Program(s));
        return retVal;
    }
    else return 0;
}

void WtWrapper::glAttachShader(GLuint program, GLuint shader)
{
    if (m_wglwidget)
    {
        // attach shader to internal program
        m_glProgramList[program]->attachShader(m_glShader2WtID[shader]);

        // call webgl function
        m_wglwidget->attachShader(
            m_glProgramList[program]->m_wtProgram,
            m_glShader2WtID[shader]->m_wtShader);
    }
}


void WtWrapper::glLinkProgram(GLuint program)
{
    if (m_wglwidget)
    {
        // compute active uniforms and shaders on the server
        m_glProgramList[program]->computeActiveUniformsAndAttributes();

        m_wglwidget->linkProgram(m_glProgramList[program]->m_wtProgram);
    }
}


// TODO: implement relay in libGLES2v2.cpp
// void WtWrapper::glDeleteProgram(GLuint program)
// {
//    if (m_wglwidget)
//    {
//		if (!m_renderPhase)
//        {
//            m_wglwidget->deleteProgram(m_glProgramList[program]->m_wtProgram);
//		}
//	}
// }

void WtWrapper::glDeleteShader(GLuint shader)
{
    // TODO
}

int WtWrapper::glGetAttribLocation(GLuint program, const GLchar* name)
{
    int retVal = -1;
    if (m_wglwidget && m_renderPhase!=PAINT)
    {

        int glID = m_glProgramList[program]->getGLIDbyAttributeName(name);
        // see if it was bound before
        if (glID==Program::UNDEFINED)
            //if (m_glProgramList[program]->m_glUniformName2glIndex.count(name)==0)
        {

            // not queried before -> create and save to mappings
            Wt::WGLWidget::AttribLocation attribLocation = m_wglwidget->getAttribLocation(
                m_glProgramList[program]->m_wtProgram,
                name);

            m_glProgramList[program]->addAttributeLocation(attribLocation, name);

        }else
        {
            retVal = m_glProgramList[program]->getGLIDbyAttributeName(name);
        }
    }
    return retVal;
}

void WtWrapper::glBindAttribLocation(GLuint program, GLuint index, const GLchar* name)
{
    if (m_wglwidget)
    {

        // TG: dont know if this is necessary...try without

        m_glProgramList[program]->addAttributeLocation(index, name);

        //std::string attribName;
        m_wglwidget->bindAttribLocation(
            m_glProgramList[program]->m_wtProgram,
            index, 
            name);
    }
}

void WtWrapper::glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    // use attributes computed on the server
    m_glProgramList[program]->getActiveAttrib(index, bufsize, length, size, type, name);

}

void WtWrapper::glGetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    // use uniforms computed on the server
    m_glProgramList[program]->getActiveUniform(index, bufsize, length, size, type, name);
}

void WtWrapper::glUseProgram(GLuint program)
{
    if (m_wglwidget)
    {
        m_activeProgram = program;
        m_wglwidget->useProgram(m_glProgramList[program]->m_wtProgram);

        // update uniform for client side view transform
        if (program != 0)renderClientSideViewTransformUniform(m_glProgramList[program]);
    }
}

int WtWrapper::glGetUniformLocation(GLuint program, const GLchar* name)
{
    int retVal = -1;
    if (m_wglwidget && m_renderPhase!=PAINT)
    {
        if (m_glProgramList[program]->getGLIDbyUniformName(name) == Program::UNDEFINED)
        {
            // careful: witty creates a new location for repeated calls even with the same name
            // therefore need to store it internally
            Wt::WGLWidget::UniformLocation u = m_wglwidget->getUniformLocation(
                m_glProgramList[program]->m_wtProgram,
                name);

            // store in internal mapping
            retVal = m_glProgramList[program]->createUniformLocation(u, name);
        }else
        {
            retVal = m_glProgramList[program]->getGLIDbyUniformName(name);
        }
    }
    return retVal;
}

void WtWrapper::glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (m_wglwidget)
    {
        m_wglwidget->uniformMatrix2fv(
            m_glProgramList[m_activeProgram]->getUniformLocation(location),
            transpose == 1,  // to get rid of compiler performance warning forcing GLboolean to bool
            value);
    }
}

void WtWrapper::glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (m_wglwidget)
    {
        m_wglwidget->uniformMatrix3fv(
            m_glProgramList[m_activeProgram]->getUniformLocation(location),
            transpose == 1,  // to get rid of compiler performance warning forcing GLboolean to bool
            value);
    }
}

void WtWrapper::glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (m_wglwidget)
    {
        m_wglwidget->uniformMatrix4fv(
            m_glProgramList[m_activeProgram]->getUniformLocation(location),
            transpose ==1,  // to get rid of compiler performance warning forcing GLboolean to bool
            value);
    }
};

void WtWrapper::glGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
    if (m_wglwidget)
    {
        // not implemented by wtwrapper, may need to do it.
        // some of the queries are done by parsing the shader source before
       
        switch (pname)
        {
        case GL_DELETE_STATUS:
            //params returns GL_TRUE if program is currently flagged for deletion, and GL_FALSE otherwise.
            {
                *params = GL_FALSE;
                break;
            }
        case GL_LINK_STATUS:
            // params returns GL_TRUE if the last link operation on program was successful, and GL_FALSE otherwise.
            {
                *params = GL_TRUE;
                break;
            }
        case GL_VALIDATE_STATUS:
            //params returns GL_TRUE or if the last validation operation on program was successful, and GL_FALSE otherwise.
            {
                *params = GL_TRUE;
                break;
            }
        case GL_INFO_LOG_LENGTH:
            //params returns the number of characters in the information log for program including the null termination character (i.e., the size of the character buffer required to store the information log). If program has no information log, a value of 0 is returned.
            {
                *params = 0;
                break;
            }
        case GL_ATTACHED_SHADERS:
            // params returns the number of shader objects attached to program.
            {
                *params = m_glProgramList[program]->getNumAttachedShaders();

                // dont know
                //*params = 2;
                break;
            }
        case GL_ACTIVE_UNIFORMS:
            // params returns the number of active uniform variables for program.
            {
                *params = m_glProgramList[program]->getNumActiveUniforms();
                break;
            }
        case GL_ACTIVE_UNIFORM_MAX_LENGTH:
            // params returns the length of the longest active uniform variable name for program, including the null termination character (i.e., the size of the character buffer required to store the longest uniform variable name). If no active uniform variables exist, 0 is returned.
            {
                // TODO find out exactly
                *params = 100; // should be safe for most shaders 
                break;
            }
        case GL_ACTIVE_ATTRIBUTES:
            //params returns the number of active attribute variables for program.
            *params = m_glProgramList[program]->getNumActiveAttributes();
            break;
        case GL_ACTIVE_ATTRIBUTE_MAX_LENGTH:
            // TODO find out exactly
            *params = 100; // should be safe for most shaders 
            break;
        default:
            std::cout << "No support for glGetProgramiv with pname= " << pname << std::endl;
            *params = -1;
        }
    }
}

void WtWrapper::glEnableVertexAttribArray(GLuint index)
{
    if (m_wglwidget)
    {
        m_wglwidget->enableVertexAttribArray(wtAttrLoc(index));

        m_activeClientSideAttributes[index].second = true;
    }
}

// Needed by osgEarth
// not defined in gles2 - so it is an error of osgEarth?
#ifndef GL_MAX_TEXTURE_COORDS
#define GL_MAX_TEXTURE_COORDS             0x8871
#endif

#ifndef GL_MAX_TEXTURE_COORDS_ARB
#define GL_MAX_TEXTURE_COORDS_ARB GL_MAX_TEXTURE_COORDS
#endif

#ifndef GL_MAX_TEXTURE_IMAGE_UNITS_ARB
#define GL_MAX_TEXTURE_IMAGE_UNITS_ARB             GL_MAX_TEXTURE_IMAGE_UNITS
#endif

#if !defined( GL_MAX_TEXTURE_UNITS )
#define GL_MAX_TEXTURE_UNITS              0x84E2
#endif

void WtWrapper::glGetIntegerv(GLenum pname, GLint* params)
{

    // TODO: query everything once from the client and store.
    // until then implement the things needed by OSG hardcoded
    switch (pname)
    {
    case GL_MAX_TEXTURE_SIZE:
        {
            // TODO
            *params = 2048; // galaxy tab limitation
            //*params = 4096; // ipad 1 limitation
            std::cout << "Warning: returning hardcoded constant: GL_MAX_TEXTURE_SIZE=" << *params << std::endl;

            break;
        }
    case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
        {
            *params = 8; // made up
            std::cout << "Warning: returning hardcoded constant: GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS=" << *params << std::endl;
            break;
        }
    case GL_MAX_TEXTURE_COORDS:
        {
            *params = 8; // made up
            std::cout << "Warning: returning hardcoded constant: GL_MAX_TEXTURE_COORDS=" << *params << std::endl;
            break;
        }
    case GL_MAX_TEXTURE_UNITS:
        {
            *params = 8; // made up
            std::cout << "Warning: returning hardcoded constant: GL_MAX_TEXTURE_UNITS=" << *params << std::endl;
            break;
        }        
    case GL_MAX_TEXTURE_IMAGE_UNITS_ARB:
        {
            *params = 8; // made up
            std::cout << "Warning: returning hardcoded constant: GL_MAX_TEXTURE_IMAGE_UNITS_ARB=" << *params << std::endl;
            break;
        }
    case GL_DEPTH_BITS:
        {
            *params = 8; // made up
            std::cout << "Warning: returning hardcoded constant: GL_DEPTH_BITS=" << *params << std::endl;
            break;
        }
    case GL_MAX_VERTEX_ATTRIBS:
        {
            *params = 8; // made up
            std::cout << "Warning: returning hardcoded constant: GL_MAX_VERTEX_ATTRIBS=" << *params << std::endl;
            break;
        }

    default:
        {
            std::cout << "glGetIntegerv not mapped:" << pname << std::endl;
        }
    }
}

void WtWrapper::glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
    int bufferID = 0;
    switch (target)
    {

    case GL_ELEMENT_ARRAY_BUFFER:
        bufferID = m_activeElementBuffer;
        break;
    case GL_ARRAY_BUFFER:
        bufferID = m_activeArrayBuffer;
        break;
    default: 
        ;
    }

    if (bufferID = 0)
    {
        std::cout << "GL_INVALID_OPERATION no buffer bound " << std::endl;
        return;
    }

    GLenum target2, usage2;
    GLsizeiptr size;
    m_bufferList[bufferID]->getData(target2,size,0,usage2);

    switch (pname)
    {
    case GL_BUFFER_SIZE: 
        *params = size;
        break;
    case GL_BUFFER_USAGE: 
        *params = usage2;
        break;
    default:
        std::cout << "GL_INVALID_ENUM " << std::endl;
    }
    return;
}

void WtWrapper::glClear(GLbitfield mask)
{
    if (m_wglwidget)
    {
        m_wglwidget->clear(Wt::WFlags<Wt::WGLWidget::GLenum>((Wt::WGLWidget::GLenum)mask));
    }
}

void WtWrapper::glLineWidth(GLfloat width)
{
    if (m_wglwidget)
    {
        m_wglwidget->lineWidth(width);
    }
}

void WtWrapper::glGenTextures(GLsizei n, GLuint* textures)
{

    if (m_wglwidget)
    {
        GLsizei currentSize = static_cast<GLsizei>(m_glTextureList.size());
        for (unsigned int i = 0, iEnd = n; i<iEnd; i++)
        {
            const std::string t = m_wglwidget->createTexture();
            m_glTextureList.push_back(new Texture(t));
            textures[i] = static_cast<GLuint>(m_glTextureList.size()-1);
        }
    }
}

void WtWrapper::glBindTexture(GLenum target, GLuint texture)
{
    if (m_wglwidget)
    {
        // TODO: no handling of textures other than 2d textures
        m_activeTexture = texture;

        m_wglwidget->bindTexture(
            (Wt::WGLWidget::GLenum)target,
            m_glTextureList[texture]->m_wtTexture);
    }
}

// 10. Complement and Compare
// This function is another one-liner that can be found on the Web. At it’s core, it is similar to the Decrement and Compare method.
// http://www.exploringbinary.com/ten-ways-to-check-if-an-integer-is-a-power-of-two-in-c/
int isPowerOfTwo (unsigned int x)
{
    return ((x != 0) && ((x & (~x + 1)) == x));
}

Wt::WRasterImage * createWRasterImage(unsigned int width, unsigned int height, GLenum format, const unsigned char * pixels_int, Wt::WGLWidget * parent)
{
    // Check the format. only support RGB and RGBA (now)
    // TODO: support other formats, e.g. GL_ALPHA
    // TODO: Create subclassed WRasterImage to support images with less channels
    // TODO: subclass WRasterImage to take the whole data with one call (faster)

    // ownership goes to parent, so no need to delete image
    Wt::WRasterImage *tmpImage = 0;
    if (format == GL_RGB) tmpImage = new Wt::WRasterImage("jpg",width,height,parent);
    else tmpImage = new Wt::WRasterImage("png",width,height,parent);

    //OpenGL stores left to right, bottom to up:
    //http://www.opengl.org/sdk/docs/man/ :
    //The first element corresponds to the lower left corner of the 
    //texture image. Subsequent elements progress left-to-right 
    //through the remaining texels in the lowest row of the texture 
    //image, and then in successively higher rows of the texture 
    //image. The final element corresponds to the upper right corner 
    //of the texture image.

    for(unsigned int y = 0; y < height; y++)
    {
        for(unsigned int x = 0 ; x < width ; x++ )
        {
            // TODO: might be slow
            switch (format)
            {
            case GL_ALPHA:   // 20121105 - Quick Hack to enable Textures "alpha"
                {
                    // flip y: y=height-y-1
                    const unsigned int k = (height-y-1)*width*1 + x*1;  // copy-pasted from RGBA (4 components -> 1 component)
                    // use pixel pending an set finish later
                    tmpImage->setPixel(x,y,Wt::WColor(
                        pixels_int[k],
                        pixels_int[k],
                        pixels_int[k],
                        pixels_int[k]
                    ));
                }
                break;
            case GL_RGBA:
                {
                    // flip y: y=height-y-1
                    const unsigned int k = (height-y-1)*width*4 + x*4;
                    // use pixel pending an set finish later
                    tmpImage->setPixel(x,y,Wt::WColor(
                        pixels_int[k],
                        pixels_int[k+1],
                        pixels_int[k+2],
                        pixels_int[k+3]
                    ));
                    // alternative Wt version: tmpImage->setPixelPending(x,y,Wt::WColor(
                }
                break;
            case GL_RGB:
                {
                    const unsigned int k = (height-y-1)*width*3 + x*3;
                    // use pixel pending an set finish later
                    tmpImage->setPixel(x,y,Wt::WColor(
                        pixels_int[k],
                        pixels_int[k+1],
                        pixels_int[k+2],
                        255
                        ));
                    // alternative Wt version: setPixelPending->setPixel(x,y,Wt::WColor(
                }
                break;
            default:
                std::cout << "Do not support texture format " << format << std::endl;
            }
        }
    }
    // synchronizes all changes
    //tmpImage->setPixelFinish();
    // old version: no need to finish

    //////////////////////////////////////////////////////////////////////////
    // TG: this alternative requires a hacked Wt (or them including my patch)
    // it performs about 10x (Release) to 100x (Debug) faster
    // TODO: need to handle flipped Y: if this code is used, dont use Webgl flip
    // pixelStorei(UNPACK_FLIP_Y_WEBGL,false);

    //char* channelMap;
    //switch (format)
    //{
    //    case GL_RGBA:
    //        channelMap = "RGBA";
    //        break;
    //    case GL_RGB:
    //    default:
    //        channelMap = "RGB";
    //}
    //Wt::WRasterImage *tmpImage = new Wt::WRasterImage("png",width,height,pixels_int,channelMap,m_wglwidget);

    return tmpImage;
}


void WtWrapper::glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
                             GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    // This is a hack: for all textures sent here MIP textures are created
    // on the client to minimize the traffic. 
    // Therefore we do not accept provided MIP maps here...
    // TODO: make automatic MIP map creation optional
    if (level>0)
        return;

    if (m_wglwidget && m_renderPhase!=PAINT)
    {

        const unsigned char * pixels_int = (unsigned char *)pixels;

        if (pixels_int == NULL)
        {
            // setup without data, just read
            m_wglwidget->texImage2D(
                (Wt::WGLWidget::GLenum)target, 
                level, 
                (Wt::WGLWidget::GLenum)internalformat, 
                width,
                height,
                border,
                (Wt::WGLWidget::GLenum)format
                );
        }else
        {
           // TODO: Experiment with raw buffers to transfer images (less time for compression and reencoding vs higher bandwidth)

            // image will exist until widget dies
            Wt::WRasterImage *tmpImage = 
                createWRasterImage(width, height, format, pixels_int, m_wglwidget);
            
            m_wglwidget->texImage2DFromURL(
                (Wt::WGLWidget::GLenum)target, 
                level, 
                (Wt::WGLWidget::GLenum)internalformat, 
                (Wt::WGLWidget::GLenum)format,
                (Wt::WGLWidget::GLenum)type,
                m_wglwidget->resolveRelativeUrl(tmpImage->url()));

            // TG: create Mip Map levels on browser side if power of two texture
            if (isPowerOfTwo(height) && isPowerOfTwo(width))
                m_wglwidget->generateMipmap((Wt::WGLWidget::GLenum)target);

        }
            
    }

}

void WtWrapper::glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
                     GLenum format, GLenum type, const GLvoid* pixels)
{

    if (m_wglwidget && m_renderPhase!=PAINT)
    {
        //////////////////////////////////////////////////////////////////////////
        // images approach
  
        const unsigned char * pixels_int = (unsigned char *)pixels;

        Wt::WRasterImage *tmpImage = 
            createWRasterImage(width, height, format, pixels_int, m_wglwidget);

        // this is the binary buffer variant
        m_wglwidget->texSubImage2D(
            (Wt::WGLWidget::GLenum)target,
            level, xoffset, yoffset,
            (Wt::WGLWidget::GLenum)format, 
            (Wt::WGLWidget::GLenum)type,
           m_wglwidget->resolveRelativeUrl(tmpImage->url()));
    }
}

void WtWrapper::glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    if (m_wglwidget)
    {
        m_wglwidget->texParameteri(
            (Wt::WGLWidget::GLenum)target, 
            (Wt::WGLWidget::GLenum)pname, 
            (Wt::WGLWidget::GLenum)param);
    }
}


void WtWrapper::glGenerateMipmap(GLenum target)
{
    if (m_wglwidget && m_renderPhase!=PAINT)
    {
        m_wglwidget->generateMipmap(
            (Wt::WGLWidget::GLenum)target);
    }
}

void WtWrapper::glActiveTexture(GLenum texture)
{
    if (m_wglwidget)
    {
        m_wglwidget->activeTexture(
            (Wt::WGLWidget::GLenum)texture);
    }
}

void WtWrapper::setPhase(const RenderPhase & renderPhase)
{ 
    m_renderPhase= renderPhase;
    if(renderPhase == INIT) reset();
    else if (renderPhase == UPDATE)
    {
        // TODO: free Wrasterimages and buffers before an update
        // e.g. like this:
        //    for (unsigned int i = 0, iEnd = m_wglwidget->children().size(); i<iEnd; i++)
        //    {
        //        Wt::WRasterImage* img = dynamic_cast<Wt::WRasterImage*> (m_wglwidget->children().at(i));
        //        if (img)
        //        {
        //            m_wglwidget->removeChild((Wt::WObject*)img)
        //            delete img;
        //        }
        //    }

        // start with new buffer for each update
        m_currentMemResource = 0;
    }
}
