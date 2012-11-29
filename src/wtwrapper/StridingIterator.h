#ifndef WTW_STRIDING_ITERATOR_H
#define WTW_STRIDING_ITERATOR_H


#include <wtwrapper/WtWrapper.h>

#pragma warning (push)
#pragma warning (disable: 4251) //("vector needs  dll-interface")
#pragma warning (disable: 4275) //("non dll-interface class used as base for dll-interface")
#include <Wt/WGLWidget>


#include <vector>

namespace WTW
{
    // I do not want to copy the data from the client side arrays, so I provide 
    // an Iterator which knows how to handle stride (used by WT)
    template <typename T>
    class StridingIterator
    {

    public:
        StridingIterator(const GLvoid *pointer, GLsizei stride = 0)
            :m_pointer(pointer), m_stride(stride)
        {}
        StridingIterator & operator++()
        {
            // increment by one position in terms of T
            // 1. convert pointer to T* 2. add one 3. convert back to void*
            m_pointer = ((GLvoid*)(( (T*)m_pointer )+1));
            //m_pointer += sizeof(T);

            // increment by the stride given in bytes
            // 1. convert pointer to byte* 2. add stride 3. convert back to void*
            m_pointer = ((GLvoid*)(( (GLbyte*)m_pointer )+m_stride));

            return *this;
        }

        T operator*() const
        {
            return *(T*)m_pointer;
        }

        T* pointer()
        {
            return (T*)m_pointer;
        }

        // add value is given in num of elements (not num bytes)
        StridingIterator operator+(const int & addValue) const
        {
            return StridingIterator(((T*)m_pointer)+addValue, m_stride);
        }

        bool operator==(StridingIterator & other) const
        {
            return m_pointer == other.pointer();
        }

        bool operator!=(StridingIterator & other) const
        {
            return m_pointer != other.pointer();
        }


    private:
        const GLvoid * m_pointer;
        GLsizei m_stride;
    };
}

#pragma warning (pop) // warning disabling

#endif //WTW_STRIDING_ITERATOR_H
