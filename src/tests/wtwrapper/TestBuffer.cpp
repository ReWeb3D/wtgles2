#include <wtwrapper/Buffer.h>
#include <wtwrapper/StridingIterator.h>

#include <catch.hpp>    // using commit 89d2a3f911cd87a6bef96aceb1702e65eee73163
// see http://www.levelofindirection.com/journal/2010/12/28/unit-testing-in-c-and-objective-c-just-got-easier.html



TEST_CASE( "testBuffer", "first and iterator access" )
{

    // provide failing tests to see the behavior in case of bugs
    // uncomment to test
    // CHECK: the program continues to test the other cases
    //CHECK(false); 

    // REQUIRE: the program stops here
    //REQUIRE(false); 

    // test working of buffer functions
    WTW::Buffer testBuffer("NameDoesNotMatter");

    void * ptr = NULL;

    //////////////////////////////////////////////////////////////////////////
    // test float 
    //////////////////////////////////////////////////////////////////////////
    // initialize a float array
    {
        
        float floatArray [] = {1.3f, -3.1f, 0.22f, 234.0f, 533.42f, 34.0f};

        int size = sizeof (floatArray);
        //// set as data and try to access it
        ptr = &floatArray;
        testBuffer.setData(
            0,                      // does not matter
            size,
            ptr,
            0);                     // does not matter

 
        size_t i = 0;
        for (float * it = testBuffer.first<float>(), *itEnd = testBuffer.last<float>();
            it!=itEnd; it++)
        {
            CHECK(*it == floatArray[i]);
            i++;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // test char 
    //////////////////////////////////////////////////////////////////////////
    {
        char charArray [] = {10, -2, 24, 56, 42, 11};

        int size = sizeof (charArray);
        //// set as data and try to access it
        ptr = &charArray;
        testBuffer.setData(
            0,                      // does not matter
            size,
            ptr,
            0);                     // does not matter


        size_t i = 0;
        for (char * it = testBuffer.first<char>(), *itEnd = testBuffer.last<char>();
            it!=itEnd; it++)
        {
            CHECK(*it == charArray[i]);
            i++;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // test double 
    //////////////////////////////////////////////////////////////////////////
    // initialize a double array
    {
        double doubleArray [] = {10.1, 125542.5454684770, -0.111114, 23456.0050045, 42.1, 11};

        int size = sizeof (doubleArray);
        //// set as data and try to access it
        ptr = &doubleArray;
        testBuffer.setData(
            0,                      // does not matter
            size,
            ptr,
            0);                     // does not matter

        size_t i = 0;
        for (double * it = testBuffer.first<double>(), *itEnd = testBuffer.last<double>();
            it!=itEnd; it++)
        {
            CHECK(*it == doubleArray[i]);
            i++;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // test unsigned int and subdata
    //////////////////////////////////////////////////////////////////////////
    {
        unsigned int uintArray [] = {10, 55662, 21234, 56001, 42, 111551};

        int size = sizeof (uintArray);
        //// set as data and try to access it
        ptr = &uintArray;
        // initialize
        testBuffer.setData(
            0,                      
            size,
            0,
            0);

        // use subdata for the start
        testBuffer.setSubData(
            0,          // offset
            2*sizeof(unsigned int),          // size
            ptr);       // data

        // use subdata for the end
        testBuffer.setSubData(
            3*sizeof(unsigned int),          // offset
            3*sizeof(unsigned int),          // size
            (const GLvoid*)( &((unsigned int*)ptr)[3] ));       // data

        // use subdata for the center
        testBuffer.setSubData(
            2*sizeof(unsigned int),          // offset
            1*sizeof(unsigned int),          // size
            (const GLvoid*)( &((unsigned int*)ptr)[2] ));       // data


        size_t i = 0;
        for (unsigned int * it = testBuffer.first<unsigned int>(), *itEnd = testBuffer.last<unsigned int>();
            it!=itEnd; it++)
        {
            CHECK(*it == uintArray[i]);
            i++;
        }
    }

}

TEST_CASE("testStridingIterator", "StridingIterator test")
{


    float floatArray [] = {1.3f, -3.1f, 0.22f, 234.0f, 533.42f, 34.0f};

    int size = sizeof (floatArray);
    //// set as data and try to access it

    WTW::Buffer testBuffer("NameDoesNotMatter");
    testBuffer.setData(
        0,                      // does not matter
        size,
        &floatArray[0],
        0);    

    {
        // test stride == 0
        WTW::StridingIterator<float> sit(testBuffer.first<float>(), 0);
        WTW::StridingIterator<float> sitEnd(testBuffer.last<float>(), 0);

        size_t i = 0;
        for (;sit!=sitEnd; ++sit)
        {
            CHECK(*sit == floatArray[i]);
            i++;
            
            // check that the for cycle stops
            REQUIRE (i<7);
        }
    }
    {
        // test stride == 1,
        char stride = 1;
        WTW::StridingIterator<float> sit(testBuffer.first<float>(), sizeof(float)*stride);
        WTW::StridingIterator<float> sitEnd(testBuffer.last<float>(), sizeof(float)*stride);

        size_t i = 0;
        for (;sit!=sitEnd; ++sit)
        {
            
            CHECK(*sit == floatArray[i]);
            i+=1+stride;
            
            // check that the for cycle stops
            REQUIRE (i<7);
        }
    }
    {
        // test stride == 1,
        char stride = 2;
        WTW::StridingIterator<float> sit(testBuffer.first<float>(), sizeof(float)*stride);
        WTW::StridingIterator<float> sitEnd(testBuffer.last<float>(), sizeof(float)*stride);

        size_t i = 0;
        for (;sit!=sitEnd; ++sit)
        {

            CHECK(*sit == floatArray[i]);
            i+=1+stride;

            // check that the for cycle stops
            REQUIRE (i<10);
        }
    }
}
