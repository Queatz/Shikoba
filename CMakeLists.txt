cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

project(Shikoba)
include_directories(/usr/include/freetype2/)
add_library(Shikoba SHARED Shikoba.cpp)
target_link_libraries(Shikoba freetype GL)

install(TARGETS Shikoba LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install_files(/include Shikoba.hpp)
