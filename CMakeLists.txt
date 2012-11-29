cmake_minimum_required(VERSION 2.8) 

# include custom cmake modules
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

SET(3RD_PARTY_DIR "${CMAKE_SOURCE_DIR}/../3rdparty_wtgles2" CACHE PATH "Enter the path to the 3rd party directory, if present (containing OSG, Wt, ..., in folders of bin, include, lib ")

if (IS_DIRECTORY ${3RD_PARTY_DIR})
	message ( "Using ${3RD_PARTY_DIR} to search for 3rd party libraries")
	SET(CMAKE_PREFIX_PATH "${3RD_PARTY_DIR};${CMAKE_PREFIX_PATH}")
endif (IS_DIRECTORY ${3RD_PARTY_DIR})

# enable support of folders in visual studio
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS On)

# set solution name 
project(wtgles2)

######################################################################
# copy all binaries to the same dir, skipping debug and release directories
# eases development with examples, when opened as part of one solution:
# no need to run install after every change in wtwrapper/libgles2~
#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE TYPE STRING)
if (MSVC_IDE)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()
######################################################################

SET (OSG_DIR CACHE PATH "Set the path to the OSG installation directory.")
SET (CMAKE_PREFIX_PATH "${OSG_DIR};${CMAKE_PREFIX_PATH}")
if(MSVC10)
	GET_FILENAME_COMPONENT( MSVC_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\10.0;InstallDir]" ABSOLUTE )
endif(MSVC10)
if(MSVC90)
	GET_FILENAME_COMPONENT( MSVC_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\9.0;InstallDir]" ABSOLUTE )
endif(MSVC90)
configure_file(setPath.bat.in setPath.bat)

SET (CONNECTOR_FCGI CACHE BOOL "Enable FCGI configuration.") 

# check what platform we're on (64-bit or 32-bit)
if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(PLATFORM x64)
    MESSAGE(STATUS "Detected 64-bit platform")
else(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(PLATFORM win32)
    MESSAGE(STATUS "Detected 32-bit platform")
endif(CMAKE_SIZEOF_VOID_P MATCHES 8)

set(CMAKE_DEBUG_POSTFIX d)

# Turn on testing support. This will generate the RUN_TESTS project in VS
ENABLE_TESTING()
INCLUDE(CTest)
IF(BUILD_TESTING)
  SET(BUILDNAME "${BUILDNAME}" CACHE STRING "Name of build on the dashboard.")
  MARK_AS_ADVANCED(BUILDNAME)
# add test that runs the install target before going on. Visual Studio only
add_test(TestINSTALL ${CMAKE_BUILD_TOOL} wtgles2.sln /build Release /project INSTALL) 
ENDIF(BUILD_TESTING)

SET(CPACK_GENERATOR "ZIP")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Vicomtech") #required
 INCLUDE(CPack)

# suppress messages about found wt libs
SET(Wt_FIND_QUIETLY 1)

OPTION(WTGLES2_WITH_OSG "Enable build of osg specific lib and examples. Only enable AFTER osg compiled to wtgles2 was installed" OFF)


IF(WIN32)
  OPTION(BOOST_DYNAMIC "Link to boost DLLs (OFF means static link)" OFF)
ENDIF(WIN32)
# http://cboard.cprogramming.com/cplusplus-programming/97390-boost-auto-linking.html
# taken from Wt (CMakeLists.txt)
IF(WIN32)
  IF(BOOST_DYNAMIC)
    ADD_DEFINITIONS(-DBOOST_ALL_DYN_LINK)
  ELSE(BOOST_DYNAMIC)
    # You could expect that this is the default when BOOST_ALL_DYN_LINK
    # is not set, but this is problematic for cygwin
    ADD_DEFINITIONS(-DBOOST_THREAD_USE_LIB)
  ENDIF(BOOST_DYNAMIC)
  ADD_DEFINITIONS(
      -D_CRT_SECURE_NO_WARNINGS
      -Dinline=__inline
      -D_SCL_SECURE_NO_WARNINGS
      )
ENDIF(WIN32)


include_directories(include)

add_subdirectory(src)

add_subdirectory(examples)
