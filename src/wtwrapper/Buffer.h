#ifndef WTW_BUFFER_H
#define WTW_BUFFER_H


#include <wtwrapper/WtWrapper.h>

#pragma warning (push)
#pragma warning (disable: 4251) //("vector needs  dll-interface")
#pragma warning (disable: 4275) //("non dll-interface class used as base for dll-interface")
#include <Wt/WGLWidget>


#include <vector>

namespace WTW
{
// Buffer helper
class WTW_API Buffer
{
public:
    Buffer(Wt::WGLWidget::Buffer wtBuffer):m_wtBuffer(wtBuffer),m_wtBufferIsSetup(false){};

    // data is null, if buffer contains only size, target and usage
    inline bool dataIsNull()
    {
        return m_isNull;
    }

    // returns first element of array in given type
    template<typename T>
    inline T* first()
    {
        return reinterpret_cast<T*>(&m_data[0]);
    }
   
    // returns last element of array in given type
    template<typename T>
    inline T* last()
    {
        // get the first element
        char * first = &m_data[0];
        // get pointer just behind the last element
        char * last = &first[m_data.size()];
        // cast to target
        T* lastCasted = reinterpret_cast<T*>(last);

        return lastCasted;
    }

    // use this, if the size in bytes is known
    // need to copy everything locally, because after client is allowed
    // to discard data afterwards
    void setData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
    

    // use this, if the number of elements and its type is known
    void setData(GLenum target, GLsizei count, GLenum type, const GLvoid *data, GLenum usage);

    void getData(GLenum &target, GLsizeiptr &size, GLvoid *data, GLenum & usage);

    std::vector<char> & getRawData() {return m_data;}

    void setSubData( GLintptr offset, GLsizeiptr size, const GLvoid *data);

    inline std::string getWtBuffer() const {return m_wtBuffer;}

    inline bool isWtBufferSetup()const {return m_wtBufferIsSetup;}

private:
    Wt::WGLWidget::Buffer m_wtBuffer;


    // set to true if it was used first (and WT data structure was set up)
    bool m_wtBufferIsSetup;

    unsigned int m_size; // specified size
    GLenum m_target;
    GLenum m_usage;
    bool m_isNull; // if the data is initialized with NULL
    std::vector<char> m_data;
};
}

#pragma warning (pop) // warning disabling

#endif 