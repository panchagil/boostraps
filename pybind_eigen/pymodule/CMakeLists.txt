cmake_minimum_required(VERSION 2.8.12)
project(mymodule)

### Compilation flags: adapt to your needs ###
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /bigobj /w") ### Enable parallel compilation
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR} )
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR} )
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2") #### Libigl requires a modern C++ compiler that supports c++11
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-unused-parameter -Wno-deprecated-register -Wno-return-type-c-linkage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations -Wno-unused-parameter -Wno-deprecated-register -Wno-return-type-c-linkage")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Os")
endif()

### Include python
set(Python_ADDITIONAL_VERSIONS 3.4 3.5 3.6)
find_package(PythonLibs REQUIRED)
find_package(PythonInterp REQUIRED)
include_directories(${PYTHON_INCLUDE_DIR} include)

# Include pybind11
set( PYBIND11_INCLUDE_DIR "$ENV{PYBIND11_INCLUDE_DIR}" )
IF( NOT PYBIND11_INCLUDE_DIR )
    message( FATAL_ERROR "Please point the environment variable PYBIND11_INCLUDE_DIR to the include directory of your pybind11 installation.")
ENDIF()
include_directories(${PYBIND11_INCLUDE_DIR})


# Include Eigen
SET( EIGEN3_INCLUDE_DIR "$ENV{EIGEN3_INCLUDE_DIR}" )
IF( NOT EIGEN3_INCLUDE_DIR )
    MESSAGE( FATAL_ERROR "Please point the environment variable EIGEN3_INCLUDE_DIR to the include directory of your Eigen3 installation.")
ENDIF()
INCLUDE_DIRECTORIES ( "${EIGEN3_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES(${EIGEN3_INCLUDE_DIR}/unsupported)
ADD_DEFINITIONS(-DEIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET)


# Base project
set(CPROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/..)

# Include my headers
include_directories(${PROJECT_SOURCE_DIR}/)
include_directories(${CPROJECT_SOURCE_DIR}/)

# Enumerate source files
file(GLOB SOURCE_FILES
        "${CPROJECT_SOURCE_DIR}/mymodule/*.cpp"
        "${PROJECT_SOURCE_DIR}/*.cpp"
        )
file(GLOB INCLUDE_FILES
        "${CPROJECT_SOURCE_DIR}/mymodule/*.h"
        "${PROJECT_SOURCE_DIR}/*.h"
        )

message("${SOURCE_FILES}")
## Prepare the python library
set(pymodule ${PROJECT_NAME})

add_library(${pymodule} SHARED
        ${SOURCE_FILES}
        ${INCLUDE_FILES}
        )

#
set_target_properties(${pymodule} PROPERTIES PREFIX "")
set_target_properties(${pymodule} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})


if (WIN32)
    if (MSVC)
        # Enforce size-based optimization and link time code generation on MSVC (~30% smaller binaries in experiments)
        set_target_properties(${pymodule} PROPERTIES COMPILE_FLAGS "/Os /GL")
        set_target_properties(${pymodule} PROPERTIES LINK_FLAGS "/LTCG")
    endif()

    # .PYD file extension on Windows
    set_target_properties(${pymodule} PROPERTIES SUFFIX ".pyd")

    # Link against the Python shared library
    # message(FATAL_ERROR ${PYTHON_LIBRARY})
    target_link_libraries(${pymodule} ${PYTHON_LIBRARIES})

elseif (UNIX)
    # It's quite common to have multiple copies of the same Python version
    # installed on one's system. E.g.: one copy from the OS and another copy
    # that's statically linked into an application like Blender or Maya.
    # If we link our plugin library against the OS Python here and import it
    # into Blender or Maya later on, this will cause segfaults when multiple
    # conflicting Python instances are active at the same time.

    # Windows does not seem to be affected by this issue. The solution for Linux
    # and Mac OS is simple: we just don't link against the Python library. The
    # resulting shared library will have missing symbols, but that's perfectly
    # fine -- they will be resolved at import time.

    # .SO file extension on Linux/Mac OS
    set_target_properties(${pymodule} PROPERTIES SUFFIX ".so")

    #Enable flag if undefined symbols appear on ${pymodule} module import to get notified about the missing symbols at link time
    option(CHECK_UNDEFINED        "Check for undefined symbols"    OFF)

    # Strip unnecessary sections of the binary on Linux/Mac OS
    if(APPLE)
        set_target_properties(${pymodule} PROPERTIES MACOSX_RPATH ".")

        if (NOT CHECK_UNDEFINED)
            set_target_properties(${pymodule} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup -dead_strip")
        endif()

        if (NOT ${U_CMAKE_BUILD_TYPE} MATCHES DEBUG)
            add_custom_command(TARGET ${pymodule} POST_BUILD COMMAND strip -u -r ${CMAKE_CURRENT_BINARY_DIR}/../${pymodule}.so)
        endif()
    else()

        if (CHECK_UNDEFINED)
            target_link_libraries(${pymodule} ${PYTHON_LIBRARIES})
            set_target_properties(${pymodule} PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
        endif()

        if (NOT ${U_CMAKE_BUILD_TYPE} MATCHES DEBUG)
            add_custom_command(TARGET ${pymodule} POST_BUILD COMMAND strip ${CMAKE_CURRENT_BINARY_DIR}/../${pymodule}.so)
        endif()
    endif()
endif()