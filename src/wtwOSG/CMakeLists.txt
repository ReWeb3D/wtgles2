cmake_minimum_required(VERSION 2.8) 

IF(WTGLES2_WITH_OSG)
FIND_PACKAGE(Wt REQUIRED)
FIND_PACKAGE(Boost REQUIRED)

FIND_PACKAGE(OpenSceneGraph REQUIRED osgDB osgGA osgViewer osgUtil)



INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} 
	${OPENSCENEGRAPH_INCLUDE_DIRS} # test1 OSG for image storage
	${Wt_INCLUDE_DIR} 
	${Boost_INCLUDE_DIR} 
	..)


LINK_DIRECTORIES(${Boost_LIBRARY_DIRS} 
	)
	
#####################################################
# convert the osgJS  aggregated file to a string 
# taken from Wt
MACRO (FILE_TO_STRING infile outfile var)
ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${outfile}
	           COMMAND ${CMAKE_COMMAND}
                   ARGS -Dinfile:FILEPATH=${CMAKE_CURRENT_SOURCE_DIR}/${infile}
                        -Doutfile:FILEPATH=${CMAKE_CURRENT_SOURCE_DIR}/${outfile}
                        -Dvar:STRING=${var}
                        -P ${CMAKE_CURRENT_SOURCE_DIR}/filetostring.cmake
		   MAIN_DEPENDENCY
		        ${CMAKE_CURRENT_SOURCE_DIR}/${infile}
)
ENDMACRO (FILE_TO_STRING)

FILE_TO_STRING(js/osgJS.aggregated.js    osgJS.aggregated.h      OSGJS_AGGREGATED_STRING)
#####################################################

ADD_LIBRARY (wtwOSG SHARED	
	WtOSGWidget.cpp ../../include/wtwrapper/WtOSGWidget.h 
	WtContext.cpp WtContext.h 
	# internal
	osgJS.aggregated.h
	) 
set_property(TARGET wtwOSG PROPERTY FOLDER "wtgles2/core")

IF(WIN32)
TARGET_LINK_LIBRARIES (wtwOSG 
	wtwrapper
	${OPENSCENEGRAPH_LIBRARIES}# OSG for WtOSGWidget, image storage
	# TG: Wt does not provide all libs with debug/optimized flag, need to add manually
	debug ${Wt_DEBUG_LIBRARY} optimized ${Wt_LIBRARY}
	ws2_32.lib	# needed by Wt
	mswsock.lib
	winmm.lib
	)
ELSE(WIN32)
TARGET_LINK_LIBRARIES (wtwOSG 
	wtwrapper
	${OPENSCENEGRAPH_LIBRARIES}# OSG for WtOSGWidget, image storage
	# TG: Wt does not provide all libs with debug/optimized flag, need to add manually
	debug ${Wt_DEBUG_LIBRARY} optimized ${Wt_LIBRARY}
	)
ENDIF(WIN32)

install (FILES ../../include/wtwrapper/WtOSGWidget.h DESTINATION include/wtwrapper)

    INSTALL(TARGETS wtwOSG
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib   
        )
ENDIF(WTGLES2_WITH_OSG)
