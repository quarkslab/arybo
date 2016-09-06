cmake_minimum_required(VERSION 2.8)
project(petanque)

enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CMakeCompilers.txt)
include(CMakeRequiredLibraries.txt)

configure_file(${CMAKE_SOURCE_DIR}/include/pa/config.h.in ${CMAKE_BINARY_DIR}/include/pa/config.h @ONLY)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/include)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/third-party)

add_subdirectory(src)
add_subdirectory(bindings)
if (NOT (DEFINED NO_TESTS OR NO_TESTS))
	add_subdirectory(tests)
endif()
if (NOT (DEFINED NO_TOOLS OR NO_TOOLS))
	add_subdirectory(tools)
endif()
