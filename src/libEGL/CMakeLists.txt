cmake_minimum_required(VERSION 2.8) 

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}../include )

LINK_DIRECTORIES(${Boost_LIBRARY_DIRS} 
	)

add_library (libEGL SHARED 
	libEGL.cpp
	libEGL.def
) 

TARGET_LINK_LIBRARIES (	libEGL
	wtwrapper 
)

# install header files
install (FILES ../../include/EGL/egl.h DESTINATION include/EGL)
install (FILES ../../include/EGL/eglext.h DESTINATION include/EGL)
install (FILES ../../include/EGL/eglplatform.h DESTINATION include/EGL)
install (FILES ../../include/KHR/khrplatform.h DESTINATION include/KHR)

SET_PROPERTY(TARGET libEGL PROPERTY FOLDER "wtgles2/core")

INSTALL(TARGETS libEGL
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib	#Do not install lib to avoid relinking
    LIBRARY DESTINATION lib	#Do not install lib to avoid relinking
    )
