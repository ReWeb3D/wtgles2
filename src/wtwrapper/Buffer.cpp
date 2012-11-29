#include "Buffer.h"


// Buffer helper

namespace WTW
{
    // use this, if the size in bytes is known
    // need to copy everything locally, because after client is allowed
    // to discard data afterwards
    void Buffer::setData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
    {
        m_target = target;
        m_usage = usage;
        m_size = size;

        m_data.resize(size);
        if (data == NULL)
        {   
            m_isNull = true;
        }else
        {
            setSubData(0, size, data);
            m_isNull = false;
        }
    }

    // use this, if the number of elements and its type is known
    void Buffer::setData(GLenum target, GLsizei count, GLenum type, const GLvoid *data, GLenum usage)
    {
        switch (type)
        {
        case GL_UNSIGNED_SHORT :
            setData(target, count*sizeof(GLushort), data, usage);
            break;
        case GL_UNSIGNED_BYTE :
            setData(target, count*sizeof(GLubyte), data, usage);
            break;
        case GL_UNSIGNED_INT :
            setData(target, count*sizeof(GLuint), data, usage);
            break;
        case GL_FLOAT :
            setData(target, count*sizeof(GLfloat), data, usage);
            break;
            // TODO: check the other types
        default:
            std::cout << "dont support " << type << std::endl;
        }
    }

    void Buffer::getData(GLenum &target, GLsizeiptr &size, GLvoid *data, GLenum & usage)
    {
        size = m_size; // this is the size to be allocated
        target = m_target;
        usage = m_usage;

        if (m_data.size()>0) data = &m_data[0];
        else data = 0;
    }

    void Buffer::setSubData( GLintptr offset, GLsizeiptr size, const GLvoid *data)
    {
        m_isNull = false;
        for(unsigned int i = 0, iEnd = size; i<iEnd; i++)
        {
            m_data[i + offset] = ((char*) data)[i];
        }
    }

}