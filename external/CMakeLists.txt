file(GLOB srcs CONFIGURE_DEPENDS stb/*.h)

project(stb LANGUAGES C CXX)

add_library(stb INTERFACE ${srcs})
target_include_directories(stb INTERFACE stb/)