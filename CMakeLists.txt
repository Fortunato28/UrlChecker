cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 11)

project(UrlChecker)
include_directories(headers)

set(SOURCES
    src/main.cpp
    src/url_wrapper.cpp
    src/url_parser.cpp
    src/controller.cpp)
set(HEADERS
    headers/url_wrapper.hpp
    headers/url_parser.hpp
    headers/controller.hpp)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

find_package(Boost 1.58.0 COMPONENTS program_options)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif()

find_package (Threads)
target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
