
SET(test_name TestWtWrapper)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src ${Wt_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${TEST_CATCH_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS} )

# add multiple tests
SET(TEST_SOURCES
	main.cpp
	TestBuffer.cpp
	TestShader.cpp
	)

ADD_EXECUTABLE( ${test_name} ${TEST_SOURCES})
SET_PROPERTY(TARGET ${test_name} PROPERTY FOLDER "wtgles2/tests/wtwrapper")

TARGET_LINK_LIBRARIES( ${test_name} 
	wtwrapper
)

ADD_TEST(${test_name} ${CMAKE_CURRENT_BINARY_DIR}/${test_name})
