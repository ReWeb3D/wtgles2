cmake_minimum_required(VERSION 2.8) 

FIND_PACKAGE(Wt REQUIRED)
FIND_PACKAGE(Boost REQUIRED COMPONENTS signals)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} 
	${Wt_INCLUDE_DIR} 
	${Boost_INCLUDE_DIR} 
	)

LINK_DIRECTORIES(${Boost_LIBRARY_DIRS} 
	)
	
ADD_LIBRARY (wtwrapper SHARED
	# public interface
	WtWrapper.cpp ../../include/wtwrapper/WtWrapper.h 
	Buffer.cpp Buffer.h
	Program.cpp Program.h
	Shader.h Shader.cpp
	StridingIterator.h
	) 
set_property(TARGET wtwrapper PROPERTY FOLDER "wtgles2/core")

IF(WIN32)		
TARGET_LINK_LIBRARIES (wtwrapper 
	debug ${Wt_DEBUG_LIBRARY} optimized ${Wt_LIBRARY};
	# needed by Wt in windows	
	ws2_32.lib mswsock.lib winmm.lib
	)
ELSE(WIN32)

TARGET_LINK_LIBRARIES (wtwrapper 
	debug ${Wt_DEBUG_LIBRARY} optimized ${Wt_LIBRARY}
	)
ENDIF(WIN32)

install (FILES ../../include/wtwrapper/WtWrapper.h DESTINATION include/wtwrapper)

INSTALL(TARGETS wtwrapper
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib   
	)
