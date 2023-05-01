file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)
add_library(filter STATIC ${srcs})
target_include_directories(filter PUBLIC include)