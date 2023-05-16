cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math -march=native")

# set(TBB_INSTALL_PREFIX "external/oneTBB/install")
# set(BENCHMARK_INSTALL_PREFIX "external/benchmark/install")
# set(GTEST_INSTALL_PREFIX "external/googletest/install")

# set(CMAKE_PREFIX_PATH
# "${TBB_INSTALL_PREFIX}/lib/cmake/TBB"
# "${BENCHMARK_INSTALL_PREFIX}/lib/cmake/benchmark"
# "${GTEST_INSTALL_PREFIX}/lib/cmake/GTest")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Release")
    set(TBB_DIR "external/install/Release/lib/cmake/TBB")
    set(benchmark_DIR "external/install/Release/lib/cmake/benchmark")

    # set(GTest_DIR "external/install/Release/lib/cmake/GTest")
    # set(CMAKE_PREFIX_PATH "external/install/Release")
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(TBB_DIR "external/install/Debug/lib/cmake/TBB")
    set(benchmark_DIR "external/install/Debug/lib/cmake/benchmark")

    # set(GTest_DIR "external/install/Debug/lib/cmake/GTest")
    # set(CMAKE_PREFIX_PATH "external/install/Debug")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})

project(PIP LANGUAGES C CXX)

find_package(TBB CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)
find_package(OpenMP REQUIRED)

# find_package(GTest CONFIG REQUIRED)
if(NOT TBB_FOUND)
    message(FATAL_ERROR " TBB Not Found! ")
endif(NOT TBB_FOUND)

if(PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
    message(WARNING " The binary directory of CMake cannot be the same as source directory! ")
endif()

if(WIN32)
    add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
endif()

add_subdirectory(external)
add_subdirectory(benchmark)
add_subdirectory(pip)

# add_subdirectory(external/googletest)
# add_subdirectory(external/benchmark)
# add_subdirectory(external/oneTBB)
add_executable(${PROJECT_NAME} main.cpp)

# target_link_libraries(${PROJECT_NAME} GTest::gtest)
target_link_libraries(${PROJECT_NAME} benchmark::benchmark)
target_link_libraries(${PROJECT_NAME} OpenMP::OpenMP_CXX)
target_link_libraries(${PROJECT_NAME} TBB::tbb)
target_link_libraries(${PROJECT_NAME} pip)
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECTDIR="${PROJECT_SOURCE_DIR}")

# get_target_property(TBB_DLL TBB::tbb IMPORTED_LOCATION_DEBUG)
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:TBB::tbb>
    $<TARGET_FILE_DIR:${PROJECT_NAME}>)
