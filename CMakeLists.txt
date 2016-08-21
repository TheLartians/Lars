cmake_minimum_required(VERSION 3.1)

# ---- Configuration variables ----

set(LARS_BUILD_EXAMPLES FALSE CACHE BOOL "build the examples included in the lars libraries")

# ---- Init lists ----

FILE(GLOB lars_headers "${CMAKE_CURRENT_SOURCE_DIR}/include/lars/*.h")
list(APPEND lars_include_dirs "${CMAKE_CURRENT_SOURCE_DIR}/include")

# ---- Add modules ----

SET(modules Event Generator NDArray)

FOREACH(module ${modules})
  ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/modules/${module})
ENDFOREACH()

add_library(Lars EXCLUDE_FROM_ALL ${lars_headers})
SET_TARGET_PROPERTIES(Lars PROPERTIES LINKER_LANGUAGE CXX)

set(Lars_INCLUDE_DIRS ${lars_include_dirs} CACHE STRING "Lars include dirs")

