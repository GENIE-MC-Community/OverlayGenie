cmake_minimum_required(VERSION 3.00)
project (OverlayGenie)

if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Debug CACHE STRING
       "Choose the type of build, options are: None Debug Release RelWithDebInfo
MinSizeRel."
       FORCE )
endif()

if( CMAKE_INSTALL_PREFIX STREQUAL "/usr/local" )
  set( CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}" )
endif()

# write this variable to cache
set( CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE PATH "Where to install ${PROJECT_NAME}" FORCE )

set( CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib" )
mark_as_advanced( CMAKE_INSTALL_RPATH )

#append link pathes to rpath list
set( CMAKE_INSTALL_RPATH_USE_LINK_PATH 1 )
mark_as_advanced( CMAKE_INSTALL_RPATH_USE_LINK_PATH )

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

find_package(ROOT REQUIRED COMPONENTS Core RIO Hist Tree Rint XMLIO Geom Physics EGPythia6 GenVector)
include(${ROOT_USE_FILE})

if(GENIE STREQUAL "")
  cmessage(FATAL_ERROR "Variable GENIE is not defined. ")
endif()

execute_process (COMMAND genie-config --topsrcdir OUTPUT_VARIABLE GENIE_INC_DIR)
execute_process (COMMAND genie-config --libs OUTPUT_VARIABLE GENIE_LIBS_DIR_WITH_LFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process (COMMAND genie-config --libdir OUTPUT_VARIABLE GENIE_LIBS_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
include_directories(${GENIE_INC_DIR})

message(STATUS "GENIE inc : ${GENIE_INC_DIR}")
message(STATUS "GENIE libdir  : ${GENIE_LIBS_DIR}")
message(STATUS "GENIE libs    : ${GENIE_LIBS_DIR_WITH_LFLAGS}")

find_library(libxml2 NAMES libxml2 xml2 PATHS $ENV{LIBXML2_FQ_DIR} PATH_SUFFIXES lib lib64 NO_DEFAULT_PATH)
include_directories("$ENV{LIBXML2_INC}")

find_library(pythia6 NAMES Pythia6 pythia6 PATHS $ENV{PYTHIA6_LIBRARY} PATH_SUFFIXES lib lib64 NO_DEFAULT_PATH)
include_directories("$ENV{PYTHIA_INC}")

find_library(log4cpp NAMES log4cpp PATHS $ENV{LOG4CPP_LIB} PATH_SUFFIXES lib lib64 NO_DEFAULT_PATH)
include_directories("$ENV{LOG4CPP_INC}")

include_directories( BEFORE include )

add_subdirectory(src)
