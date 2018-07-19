cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 11)

project(UrlChecker)
add_executable(${PROJECT_NAME} "main.cpp")

find_package(Boost 1.58.0 COMPONENTS program_options)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif()
