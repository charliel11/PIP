cmake_minimum_required( VERSION 3.16 )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

set(CMAKE_MAKE_PROGRAM ${CMAKE_CURRENT_SOURCE_DIR}/external/install/Release/bin/ninja.exe CACHE FILEPATH "" FORCE)

project( PIP LANGUAGES C CXX )

if( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
    # using Clang
    if( CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC" )
        # using clang with clang-cl front end
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -openmp" )
    elseif( CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU" )
        # using clang with regular front end
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math -march=native -fexceptions -fopenmp" )
    endif()
elseif( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
    # using GCC
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math -march=native -fexceptions -fopenmp" )
elseif( CMAKE_CXX_COMPILER_ID STREQUAL "Intel" )
# using Intel C++
elseif( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -openmp" )
    # using Visual Studio C++
endif()

if( NOT CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE Release )
endif()

set( TBB_DIR "external/install/${CMAKE_BUILD_TYPE}/lib/cmake/TBB" )
set( benchmark_DIR "external/install/${CMAKE_BUILD_TYPE}/lib/cmake/benchmark" )

# set(GTest_DIR "external/install/Release/lib/cmake/GTest")
# set(CMAKE_PREFIX_PATH "external/install/Release")
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE} )

find_package( TBB CONFIG REQUIRED )
find_package( benchmark CONFIG REQUIRED )
find_package( OpenMP REQUIRED )

# find_package(GTest CONFIG REQUIRED)
if( NOT TBB_FOUND )
    message( FATAL_ERROR " TBB Not Found! " )
endif( NOT TBB_FOUND )

if( PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR )
    message( WARNING " The binary directory of CMake cannot be the same as source directory! " )
endif()

if( WIN32 )
    add_definitions( -DNOMINMAX -D_USE_MATH_DEFINES )
endif()

add_subdirectory( external )
add_subdirectory( benchmark )
add_subdirectory( pip )

add_executable( ${PROJECT_NAME} main.cpp )

target_link_libraries( ${PROJECT_NAME} benchmark::benchmark )
target_link_libraries( ${PROJECT_NAME} OpenMP::OpenMP_CXX )
target_link_libraries( ${PROJECT_NAME} TBB::tbb )
target_link_libraries( ${PROJECT_NAME} pip )
target_compile_definitions( ${PROJECT_NAME} PRIVATE PROJECTDIR="${PROJECT_SOURCE_DIR}" )

# get_target_property(TBB_DLL TBB::tbb IMPORTED_LOCATION_DEBUG)
add_custom_command( TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:TBB::tbb>
    $<TARGET_FILE_DIR:${PROJECT_NAME}> )
