cmake_minimum_required(VERSION 3.1)

# ---- Project ----
project(Lars)
set(LARS_LIBRARY TRUE CACHE INTERNAL "")

# ---- Configuration variables ----
set(LARS_BUILD_EXAMPLES FALSE CACHE BOOL "build the examples included in the lars libraries")
set(LARS_BUILD_TESTS FALSE CACHE BOOL "build the tests included in the lars libraries")

# ---- Init lists ----

FILE(GLOB lars_headers "${CMAKE_CURRENT_SOURCE_DIR}/include/lars/*.h")
list(APPEND lars_include_dirs "${CMAKE_CURRENT_SOURCE_DIR}/include")

# ---- Add modules ----

SET(modules Visitor Event Generator NDArray Parser Expresso Glue )

FOREACH(module ${modules})
  ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/modules/${module})
ENDFOREACH()

add_library(Lars EXCLUDE_FROM_ALL ${lars_headers})
SET_TARGET_PROPERTIES(Lars PROPERTIES LINKER_LANGUAGE CXX)

set(Lars_INCLUDE_DIRS ${lars_include_dirs} CACHE INTERNAL "")
set(Lars_LIBRARIES ${lars_libraries} CACHE INTERNAL "")

IF(LARS_BUILD_EXAMPLES)
  set(CMAKE_CXX_STANDARD 11)
  file( GLOB example_sources ${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp )
  foreach( examplesourcefile ${example_sources} )
    get_filename_component(filename ${examplesourcefile} NAME)
    string( REPLACE ".cpp" "" examplename ${filename} )
    add_executable( ${examplename} ${examplesourcefile})
    include_directories(${Lars_INCLUDE_DIRS})
    target_link_libraries(${examplename} ${Lars_LIBRARIES})
  endforeach()
ENDIF()


