cmake_minimum_required(VERSION 2.8) 

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}../include )

LINK_DIRECTORIES(${Boost_LIBRARY_DIRS} 
)

# always build a non _d filename, because OpenSceneGraph (as most
# other libraries) support only one GL lib
set(CMAKE_DEBUG_POSTFIX )

add_library (libGLESv2 SHARED 
	libGLESv2.cpp
	libGLESv2.def
	) 

TARGET_LINK_LIBRARIES (
	libGLESv2
	wtwrapper 
	)

# install header files
install (FILES ../../include/GLES2/gl2.h DESTINATION include/GLES2)
install (FILES ../../include/GLES2/gl2ext.h DESTINATION include/GLES2)
install (FILES ../../include/GLES2/gl2platform.h DESTINATION include/GLES2)
install (FILES ../../include/KHR/khrplatform.h DESTINATION include/KHR)

SET_PROPERTY(TARGET libGLESv2 PROPERTY FOLDER "wtgles2/core")

INSTALL(TARGETS libGLESv2
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib	#Do not install lib to avoid relinking
    LIBRARY DESTINATION lib	#Do not install lib to avoid relinking
    )
