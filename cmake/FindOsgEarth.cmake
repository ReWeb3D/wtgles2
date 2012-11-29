# Locate OSGEarth
# This module defines
# OSGEARTH_LIBRARY
# OSGEARTH_FOUND, if false, do not try to link to OSG
# OSGEARTH_INCLUDE_DIR, where to find the headers
#
# $OSGEARTH_DIR is an environment variable that would
# correspond to the ./configure --prefix=$OSGEARTH_DIR
#
# Based on FindOSG.cmake Created by Robert Osfield
#   with revisions by the Delta3D team
# Modified by Th3flyboy
# Modified by Stefan Buschmann (added lib64 directories)

FIND_PATH(OSGEARTH_INCLUDE_DIR osgEarth/Map
    $ENV{OSG_DIR}/include
    $ENV{OSG_DIR}
    $ENV{OSGDIR}/include
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/include
    $ENV{OSGEARTH_DIR}/include
    $ENV{OSGEARTH_DIR}
    $ENV{OSGEARTHDIR}/include
    $ENV{OSGEARTHDIR}
    $ENV{OSGEARTH_ROOT}/include
    $ENV{CSPDEVPACK}/include
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include
    /usr/freeware/include
)

MACRO(FIND_OSGEARTH_LIBRARY MYLIBRARY MYLIBRARYNAME)

    FIND_LIBRARY(${MYLIBRARY}
        NAMES ${MYLIBRARYNAME}
        PATHS
        $ENV{OSG_DIR}/lib
        $ENV{OSG_DIR}/build/lib
        $ENV{OSG_DIR}
        $ENV{OSGDIR}/lib
        $ENV{OSGDIR}
        $ENV{OSG_ROOT}/lib
        $ENV{OSG_ROOT}/build/lib
        $ENV{OSGEARTH_DIR}/lib
        $ENV{OSGEARTH_DIR}/build/lib
        $ENV{OSGEARTH_DIR}
        $ENV{OSGEARTHDIR}/lib
        $ENV{OSGEARTHDIR}
        $ENV{OSGEARTH_ROOT}/lib
        $ENV{OSGEARTH_ROOT}/build/lib
		$ENV{CSPDEVPACK}/lib
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/local/lib64
        /usr/lib
        /usr/lib64
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
        /usr/freeware/lib64
    )

ENDMACRO(FIND_OSGEARTH_LIBRARY LIBRARY LIBRARYNAME)

############################################################
# MACRO add_library_for_config
#
# Add a library for a certain configuration, e.g. 'debug' or 'optimized'.
# The macro checks if $input is set, if yes, it adds it to the output variable
# with the given prefix. If $input is not set, but $fallback is set, $fallback is
# used instead, e.g. to allow for debug libraries to be used if the release version is
# not available or vice versa.
#
# example call: add_library_for_config(MY_LIBRARY optimized MY_LIBRARY_RELEASE MY_LIBRARY_DEBUG)
#
macro(add_library_for_config dest prefix input fallback)
  if(${input})
    set(${dest} ${${dest}} ${prefix} ${${input}})
  elseif(${fallback})
    set(${dest} ${${dest}} ${prefix} ${${fallback}})
  endif()
endmacro(add_library_for_config dest prefix input fallback)

# Find release (optimized) libs
FIND_OSGEARTH_LIBRARY(OSGEARTH_LIBRARY_RELEASE osgEarth)
FIND_OSGEARTH_LIBRARY(OSGEARTHSYMBOLOGY_LIBRARY_RELEASE osgEarthSymbology)
FIND_OSGEARTH_LIBRARY(OSGEARTHUTIL_LIBRARY_RELEASE osgEarthUtil)
FIND_OSGEARTH_LIBRARY(OSGEARTHFEATURES_LIBRARY_RELEASE osgEarthFeatures)

# Find debug libs
FIND_OSGEARTH_LIBRARY(OSGEARTH_LIBRARY_DEBUG osgEarthd)
FIND_OSGEARTH_LIBRARY(OSGEARTHSYMBOLOGY_LIBRARY_DEBUG osgEarthSymbologyd)
FIND_OSGEARTH_LIBRARY(OSGEARTHUTIL_LIBRARY_DEBUG osgEarthUtild)
FIND_OSGEARTH_LIBRARY(OSGEARTHFEATURES_LIBRARY_DEBUG osgEarthFeaturesd)

# Compose lists
SET(OSGEARTH_LIBRARY "")
add_library_for_config(OSGEARTH_LIBRARY optimized OSGEARTH_LIBRARY_RELEASE OSGEARTH_LIBRARY_DEBUG)
add_library_for_config(OSGEARTH_LIBRARY debug     OSGEARTH_LIBRARY_DEBUG   OSGEARTH_LIBRARY_RELEASE)
SET(OSGEARTHSYMBOLOGY_LIBRARY "")
add_library_for_config(OSGEARTHSYMBOLOGY_LIBRARY optimized OSGEARTHSYMBOLOGY_LIBRARY_RELEASE OSGEARTHSYMBOLOGY_LIBRARY_DEBUG)
add_library_for_config(OSGEARTHSYMBOLOGY_LIBRARY debug     OSGEARTHSYMBOLOGY_LIBRARY_DEBUG   OSGEARTHSYMBOLOGY_LIBRARY_RELEASE)
SET(OSGEARTHUTIL_LIBRARY "")
add_library_for_config(OSGEARTHUTIL_LIBRARY optimized OSGEARTHUTIL_LIBRARY_RELEASE OSGEARTHUTIL_LIBRARY_DEBUG)
add_library_for_config(OSGEARTHUTIL_LIBRARY debug     OSGEARTHUTIL_LIBRARY_DEBUG   OSGEARTHUTIL_LIBRARY_RELEASE)
SET(OSGEARTHFEATURES_LIBRARY "")
add_library_for_config(OSGEARTHFEATURES_LIBRARY optimized OSGEARTHFEATURES_LIBRARY_RELEASE OSGEARTHFEATURES_LIBRARY_DEBUG)
add_library_for_config(OSGEARTHFEATURES_LIBRARY debug     OSGEARTHFEATURES_LIBRARY_DEBUG   OSGEARTHFEATURES_LIBRARY_RELEASE)

# set up aggregated variable
SET(OSGEARTH_LIBRARIES ${OSGEARTH_LIBRARY} ${OSGEARTHSYMBOLOGY_LIBRARY} ${OSGEARTHUTIL_LIBRARY} ${OSGEARTHFEATURES_LIBRARY})

#MESSAGE(${OSGEARTH_LIBRARIES})

SET(OSGEARTH_FOUND "NO")
IF(OSGEARTH_LIBRARY AND OSGEARTH_INCLUDE_DIR)
    SET(OSGEARTH_FOUND "YES")
ENDIF(OSGEARTH_LIBRARY AND OSGEARTH_INCLUDE_DIR)
