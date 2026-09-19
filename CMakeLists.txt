cmake_minimum_required(VERSION 3.24)
SET(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(WordCount LANGUAGES CXX)

enable_testing()

add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(tests)
