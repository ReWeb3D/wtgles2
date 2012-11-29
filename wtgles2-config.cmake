get_filename_component(THIS_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(THIS_LIB "${CMAKE_CURRENT_LIST_FILE}" NAME)
string(REGEX REPLACE "-.*$" "" THIS_LIB ${THIS_LIB})

set(${THIS_LIB}_INCLUDE_DIR ${THIS_DIR}/src)

if(NOT TARGET ${THIS_LIB})
  # Only do this once globally
  add_subdirectory(${THIS_DIR} ${THIS_LIB})
endif(NOT TARGET ${THIS_LIB})
