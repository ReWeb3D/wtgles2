# set the name of current project
SET(app_name osgViewerWt.wt)

# find 3rd party dependencies
FIND_PACKAGE(OpenSceneGraph REQUIRED osgDB osgUtil osgViewer osgGA osgTerrain osgText)
FIND_PACKAGE(Wt)
FIND_PACKAGE(Boost REQUIRED system signals)


SET( target_src
    osgviewerWt.cpp
)

INCLUDE_DIRECTORIES(${OPENSCENEGRAPH_INCLUDE_DIRS} ${Wt_INCLUDE_DIR} ${Boost_INCLUDE_DIR} )

LINK_DIRECTORIES(${Boost_LIBRARY_DIRS} )

ADD_EXECUTABLE( ${app_name} ${target_src})

SET_PROPERTY(TARGET ${app_name} PROPERTY FOLDER "wtgles2/examples")

IF (CONNECTOR_FCGI)
	SET (CONNECTOR_LIBRARY debug ${Wt_FCGI_DEBUG_LIBRARY} ${Wt_FCGI_LIBRARY})
ELSE (CONNECTOR_FCGI)
	SET (CONNECTOR_LIBRARY debug ${Wt_HTTP_DEBUG_LIBRARY} optimized ${Wt_HTTP_LIBRARY})
ENDIF (CONNECTOR_FCGI)

IF(WIN32)
TARGET_LINK_LIBRARIES( ${app_name} 
	${OPENSCENEGRAPH_LIBRARIES}
	${Boost_LIBRARIES}	
	# TG: Wt does not provide all libs with debug/optimized flag, need to add manually
	debug ${Wt_DEBUG_LIBRARY} optimized ${Wt_LIBRARY}
	${CONNECTOR_LIBRARY}
	wtwrapper
	wtwOSG
	# needed by wt in windows
	ws2_32.lib 
	mswsock.lib
	winmm.lib
)
ELSE(WIN32)
TARGET_LINK_LIBRARIES( ${app_name} 
	${OPENSCENEGRAPH_LIBRARIES}
	${Boost_LIBRARIES}
	# TG: Wt does not provide all libs with debug/optimized flag, need to add manually
	debug ${Wt_DEBUG_LIBRARY} optimized ${Wt_LIBRARY}
	${CONNECTOR_LIBRARY}
	wtwrapper
	wtwOSG
)
ENDIF(WIN32)
