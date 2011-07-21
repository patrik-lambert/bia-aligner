cmake_minimum_required(VERSION 2.6)

PROJECT(bia)

#if you don't want the full compiler output, remove the following line
SET(CMAKE_VERBOSE_MAKEFILE ON)
#SET( CMAKE_CXX_FLAGS " -Wno-deprecated " CACHE STRING "Flags used by the C++ compiler" FORCE )

#add definitions, compiler switches, etc.
ADD_DEFINITIONS(-Wno-deprecated -O3 -static -g )
# -Wall -O2  -DINSTANTIATE_TEMPLATES
#list all source files here
# add_subdirectory(src)
add_subdirectory(training)

SET (bia_SRCS
${CMAKE_SOURCE_DIR}/bia.cpp ${CMAKE_SOURCE_DIR}/fertTable.cpp ${CMAKE_SOURCE_DIR}/iword.cpp ${CMAKE_SOURCE_DIR}/params.cpp ${CMAKE_SOURCE_DIR}/scoreTable.cpp ${CMAKE_SOURCE_DIR}/sentPair.cpp ${CMAKE_SOURCE_DIR}/state.cpp ${CMAKE_SOURCE_DIR}/linkCluster.cpp ${CMAKE_SOURCE_DIR}/phrase.cpp ${CMAKE_SOURCE_DIR}/wlink.cpp ${CMAKE_SOURCE_DIR}/alUnit.cpp
)

add_executable(bia ${bia_SRCS})

install(TARGETS bia DESTINATION ${CMAKE_SOURCE_DIR})
