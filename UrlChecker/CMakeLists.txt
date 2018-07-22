cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 11)

project(UrlChecker)

set(SOURCES
    main.cpp
    urlwrapper.cpp)
set(HEADERS
    urlwrapper.h)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

find_package(Boost 1.58.0 COMPONENTS program_options)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif()
