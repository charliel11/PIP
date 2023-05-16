file(GLOB_RECURSE srcs CONFIGURE_DEPENDS *.cpp)

project(benchmark CXX)

add_executable(${PROJECT_NAME} ${srcs})
target_link_libraries(${PROJECT_NAME} benchmark::benchmark)
target_link_libraries(${PROJECT_NAME} TBB::tbb)
target_link_libraries(${PROJECT_NAME} pip)
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECTDIR="${PROJECT_SOURCE_DIR}")

add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:TBB::tbb>
    $<TARGET_FILE_DIR:${PROJECT_NAME}>)
