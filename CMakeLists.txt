cmake_minimum_required(VERSION 3.26)
project(fcg_new_project)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-function -g")

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES "src/*")
add_executable(main ${SOURCES})

file(GLOB LIBRARIES "${CMAKE_SOURCE_DIR}/lib-linux/*.a")
target_link_libraries(main ${LIBRARIES} rt m dl X11 pthread Xrandr Xinerama Xxf86vm Xcursor)
