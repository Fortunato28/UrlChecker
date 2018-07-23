cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 11)

project(UrlChecker)

set(SOURCES
    src/main.cpp
    src/urlwrapper.cpp
    src/url_parser.cpp)
set(HEADERS
    headers/urlwrapper.hpp
    headers/url_parser.hpp)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

find_package(Boost 1.58.0 COMPONENTS program_options)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif()

include_directories(headers)
