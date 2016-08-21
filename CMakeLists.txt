cmake_minimum_required(VERSION 3.1)

# ---- Macros ----

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${child})
      IF(NOT ${child} STREQUAL ${curdir})
        LIST(APPEND dirlist ${child})
      ENDIF()
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

# ---- Configuration variables ----

set(LARS_BUILD_EXAMPLES FALSE CACHE BOOL "build the examples included in the lars libraries")

# ---- Init lists ----

list(APPEND lars_headers "")
list(APPEND lars_include_dirs "")

# ---- Add modules ----

SUBDIRLIST(module_directories "${CMAKE_CURRENT_SOURCE_DIR}/modules")
FOREACH(module_directory ${module_directories})
  ADD_SUBDIRECTORY(${module_directory})
ENDFOREACH()

add_library(Lars EXCLUDE_FROM_ALL ${lars_headers})
SET_TARGET_PROPERTIES(Lars PROPERTIES LINKER_LANGUAGE CXX)

set(Lars_INCLUDE_DIRS ${lars_include_dirs} CACHE STRING "Lars include dirs")

