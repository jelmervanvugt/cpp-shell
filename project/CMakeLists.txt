project(shell)
cmake_minimum_required(VERSION 3.0)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

SET(SRC_LIST shell.cpp)
add_library (${PROJECT_NAME}lib ${SRC_LIST})

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}lib)

add_subdirectory(ext/gtest)
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIRS})
set (test test.cpp)

FILE(GLOB_RECURSE UNITTESTS *.test.cpp)
add_executable (${PROJECT_NAME}test ${test} ${UNITTESTS})
target_link_libraries(${PROJECT_NAME}test ${PROJECT_NAME}lib)
add_dependencies(${PROJECT_NAME}test ${PROJECT_NAME} googletest)
target_link_libraries(${PROJECT_NAME}test ${GTEST_LIBS_DIR}/libgtest.a ${GTEST_LIBS_DIR}/libgtest_main.a)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(Threads)
    target_link_libraries(${PROJECT_NAME}test ${CMAKE_THREAD_LIBS_INIT})
ENDIF()

