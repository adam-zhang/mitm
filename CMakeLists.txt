project(mitm)
cmake_minimum_required(VERSION 2.8)
set(mitm_MAJOR 0)
set(mitm_MINOR 0)
set(mitm_PATCH 99)
set(mitm_VERSION "${mitm_MAJOR}.${mitm_MINOR}.${mitm_PATCH}")
set(mitm_STABLEVERSION "${mitm_MAJOR}.${mitm_MINOR}")

if (CMAKE_BUILD_TYPE STREQUAL "")
  set (CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build" FORCE)
endif ()

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"
    OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11")
endif ()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR})

### # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
## Check dependencies
#

find_package(PkgConfig)

macro (pkg_config_required_library LIBNAME pcfile)
  pkg_check_modules(${LIBNAME} REQUIRED ${pcfile})
  include_directories(${${LIBNAME}_INCLUDE_DIRS})
  link_directories(${${LIBNAME}_LIBRARY_DIRS})
  add_definitions(${${LIBNAME}_CFLAGS_OTHER})
  set(mitm_LINK_LIBRARIES ${mitm_LINK_LIBRARIES} ${${LIBNAME}_LIBRARIES})
  set(mitm_STATIC_LINK_LIBRARIES ${mitm_STATIC_LINK_LIBRARIES} ${${LIBNAME}_STATIC_LIBRARIES})
endmacro ()

pkg_config_required_library(LIBEIGEN3 eigen3)

message(STATUS "checking for a CUDA compiler")
find_package(CUDA)
if (NOT CUDA_FOUND)
  message(STATUS " failed to found a CUDA compiler. GPGPU is disabled.
  Check the FindCUDA.cmake for more details.")
else ()
  message(STATUS " CUDA support enabled")
endif ()

### # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
## Core library
#

set(libmitm_cxx_flags "")
set(libmitm_cxx_libs "")

if (CUDA_FOUND)
  set(libmitm_cxx_flags "${mitm_cxx_flags} -DMITM_HAVE_CUDA")
endif ()

set(mitm_library_sources_cpp
  src/cstream.cpp
  src/cstream.hpp
  src/heuristic-classic.cpp
  src/internal.hpp
  src/io.cpp
  src/mitm.cpp
  src/wedelin-default.cpp)

if (CUDA_FOUND)
  # Avoid std=c++11 in cuda's flags.
  set(CUDA_PROPAGATE_HOST_FLAGS OFF)

  if (BUILD_SHARED_LIBS)
    CUDA_COMPILE(heuristic-gpgpu-cuda src/heuristic-gpgpu.cu SHARED)
  else ()
    CUDA_COMPILE(heuristic-gpgpu-cuda src/heuristic-gpgpu.cu STATIC)
  endif ()
  list(APPEND mitm_library_sources_cpp ${heuristic-gpgpu-cuda})
  list(APPEND libmitm_cxx_libs ${CUDA_LIBRARIES})
endif ()

add_library(libmitm ${mitm_library_sources_cpp})

if (BUILD_SHARED_LIBS)
  list(APPEND libmitm_cxx_cflags "-DMITM_DLL")
endif ()

target_link_libraries(libmitm ${libmitm_cxx_libs})
set_target_properties(libmitm PROPERTIES
  VERSION 0
  OUTPUT_NAME "mitm-${mitm_STABLEVERSION}"
  COMPILE_FLAGS "${libmitm_cxx_flags}"
  LINKS_FLAGS "${libmitm_cxx_link_flags}"
  VERSION "${mitm_STABLEVERSION}")

install(DIRECTORY src/mitm DESTINATION include/mitm-${mitm_STABLEVERSION})
install(TARGETS libmitm DESTINATION lib)

set(mitm_PKGCONFIG_PREFIX_DIR "${CMAKE_INSTALL_PREFIX}")
configure_file(mitm.pc.in
  ${PROJECT_BINARY_DIR}/mitm-${mitm_STABLEVERSION}.pc @ONLY)

install(FILES ${PROJECT_BINARY_DIR}/mitm-${mitm_STABLEVERSION}.pc
  DESTINATION lib/pkgconfig)


### # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
## Core program
#

set(mitm_cxx_flags "")
set(mitm_cxx_libs "")

if (CUDA_FOUND)
  list(APPEND mitm_cxx_libs ${CUDA_LIBRARIES})
endif ()

set(mitm_sources_cpp src/main.cpp)

add_executable(mitm ${mitm_sources_cpp})
target_link_libraries(mitm ${mitm_cxx_libs};libmitm)
set_target_properties(mitm PROPERTIES
  VERSION ${mitm_STABLEVERSION}
  COMPILE_FLAGS " ${mitm_cxx_flags}")

install(TARGETS mitm DESTINATION bin)

### # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
## Testing
#

enable_testing()

message(STATUS "checking for 'catch.hpp'")
find_path(CATCH_INCLUDE_DIR catch.hpp PATHS /usr/include /usr/local/include
  ENV CATCH_INCLUDE_DIR)

macro (mitm_add_test_executable TEST_NAME TEST_SOURCE)
  add_executable(${TEST_NAME} ${TEST_SOURCE})
  target_link_libraries(${TEST_NAME} ${mitm_cxx_libs};libmitm)
  set_target_properties(${TEST_NAME} PROPERTIES
    COMPILE_DEFINITIONS EXAMPLES_DIR="${CMAKE_SOURCE_DIR}/tests")
endmacro ()

if (CATCH_INCLUDE_DIR)
  message(STATUS " found 'catch.hpp' in ${CATCH_INCLUDE_DIR}")
  include_directories(${CATCH_INCLUDE_DIR})

  mitm_add_test_executable(assigmement_problem tests/assignment_problem.cpp)
  add_test(NAME ap-01 COMMAND assignment_problem
    ${CMAKE_SOURCE_DIR}/tests/assignment_problem_input.conf)

  mitm_add_test_executable(n-queens-problem tests/n-queens-problem.cpp)
  add_test(NAME nqp-01 COMMAND assignment_problem -x 4)
else ()
  message(STATUS " not found catch.hpp. Unit test disabled")
endif ()
