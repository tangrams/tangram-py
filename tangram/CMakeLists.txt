cmake_minimum_required(VERSION 2.8)
project(tangram)

# set(CMAKE_BUILD_TYPE Debug)

# helper functions
include(${PROJECT_SOURCE_DIR}/tangram-es/toolchains/utils.cmake)

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/tangram-es/external/alfons/CMakeLists.txt")
    message(SEND_ERROR "Missing submodules - Please run:\n 'git submodule update --init --recursive'")
    return()
endif()

# common compiler options
set(EXECUTABLE_NAME "tangram")

find_package(SWIG REQUIRED)
find_package(PythonLibs REQUIRED)

if(${PLATFORM_TARGET} MATCHES "rpi")
    message(STATUS "Building config for RaspberryPI")

    add_definitions(-DPLATFORM_RPI)

    # compiler options
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fpermissive -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_CXX_FLAGS} -L/opt/vc/lib/ -lGLESv2 -lEGL -lbcm_host -lvchiq_arm -lvcos -lrt -lpthread")
    set(CXX_FLAGS_DEBUG "-g -O0")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    
    include_directories(/opt/vc/include/)
    include_directories(/opt/vc/include/interface/vcos/pthreads)
    include_directories(/opt/vc/include/interface/vmcs_host/linux)
    include_directories(SYSTEM "/usr/include/fontconfig")

elseif(${PLATFORM_TARGET} MATCHES "linux")
    message(STATUS "Building config for Linux")

    find_package(OpenGL REQUIRED)

    # set for test in other cmake files
    set(PLATFORM_LINUX ON)
    add_definitions(-DPLATFORM_LINUX)
    set(CORE_COMPILE_DEFS PLATFORM_LINUX)
    
    include_directories(SYSTEM "/usr/include/fontconfig")

    # compiler options
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fpermissive -g -std=c++1y -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")
    set(CXX_FLAGS_DEBUG "-g -O0")

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-gnu-zero-variadic-macro-arguments")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  -lc++ -lc++abi")
    endif()

    if (CMAKE_COMPILER_IS_GNUCC)
        execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
        string(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${GCC_VERSION})
        list(GET GCC_VERSION_COMPONENTS 0 GCC_MAJOR)
        list(GET GCC_VERSION_COMPONENTS 1 GCC_MINOR)
        message(STATUS "Using gcc ${GCC_VERSION}")
        if (GCC_VERSION VERSION_GREATER 5.1)
            message(STATUS "USE CXX11_ABI")
            add_definitions("-D_GLIBCXX_USE_CXX11_ABI=1")
        endif()
    endif()

elseif(${PLATFORM_TARGET} MATCHES "osx")
    message(STATUS "Building config for Darwin OSX")

    find_package(OpenGL REQUIRED)
    
    # set for test in other cmake files
    set(PLATFORM_OSX ON)
    add_definitions(-DPLATFORM_OSX)

    # compiler options
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++1y ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wreturn-type -Wsign-compare")
    set(CXX_FLAGS_DEBUG "-g -O0")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/lib")

endif()

# add sources and include headers
add_subdirectory(${PROJECT_SOURCE_DIR}/tangram-es/external)
add_subdirectory(${PROJECT_SOURCE_DIR}/tangram-es/core)
include_directories(${CORE_INCLUDE_DIRS})
include_directories(${CORE_LIBRARIES_INCLUDE_DIRS} )

set(SOURCES
  ${PROJECT_SOURCE_DIR}/src/context.cpp
  ${PROJECT_SOURCE_DIR}/src/tangram-proxy.cpp
  ${PROJECT_SOURCE_DIR}/src/platform_posix.cpp
  ${PROJECT_SOURCE_DIR}/tangram-es/linux/src/urlWorker.cpp
  ${PROJECT_SOURCE_DIR}/tangram-es/core/common/platform_gl.cpp)

include(${SWIG_USE_FILE})
include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/tangram-es/core/)
include_directories(${PROJECT_SOURCE_DIR}/tangram-es/linux/src)
include_directories(${PROJECT_SOURCE_DIR}/tangram-es/core/common)

set(CMAKE_SWIG_FLAGS "")
set_source_files_properties(src/tangram.i PROPERTIES CPLUSPLUS ON)
#set_source_files_properties(src/tangram.i PROPERTIES SWIG_FLAGS "-includeall")

swig_add_module(${EXECUTABLE_NAME} python src/tangram.i ${SOURCES})
if(${PLATFORM_TARGET} MATCHES "linux")
    swig_link_libraries(${EXECUTABLE_NAME} ${CORE_LIBRARY} ${PYTHON_LIBRARIES} curl glfw ${OPENGL_LIBRARIES} fontconfig freetype pthread gcc_s gcc)
elseif(${PLATFORM_TARGET} MATCHES "rpi")
    swig_link_libraries(${EXECUTABLE_NAME} ${CORE_LIBRARY} ${PYTHON_LIBRARIES} curl fontconfig freetype gcc_s gcc)
elseif(${PLATFORM_TARGET} MATCHES "osx")
    swig_link_libraries(${EXECUTABLE_NAME} ${CORE_LIBRARY} ${PYTHON_LIBRARIES} curl glfw ${OPENGL_LIBRARIES})
endif()

execute_process(COMMAND python -c "from distutils.sysconfig import get_python_lib; print get_python_lib()" OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)
install(FILES _tangram DESTINATION ${PYTHON_SITE_PACKAGES})
install(FILES ${CMAKE_BINARY_DIR}/src/tangram.py DESTINATION ${PYTHON_SITE_PACKAGES})
message(STATUS "Python destination folder" ${PYTHON_SITE_PACKAGES})
