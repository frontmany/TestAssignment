cmake_minimum_required(VERSION 3.20)

project(TestAssignment)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "src/*.cpp" "src/*.h")
source_group("Source Files" FILES ${SOURCES})

add_executable(${PROJECT_NAME} ${SOURCES})