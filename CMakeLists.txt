cmake_minimum_required(VERSION 3.0.0)
project(CalendarPuzzle VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(CalendarPuzzle main.cpp)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
