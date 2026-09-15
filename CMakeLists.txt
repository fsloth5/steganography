cmake_minimum_required(VERSION 4.4.2)

set(project_name "megan")
project(
  ${project_name}
  VERSION "0.0.1"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "-d")

add_subdirectory(src)

option(BUILD_TESTS "Builds ${project_name} tests" ON)
if(BUILD_TESTS)
  include(FetchContent)
  if(NOT TARGET Catch2::Catch2WithMain)
    FetchContent_Declare(
      catch2
      GIT_REPOSITORY https://github.com/catchorg/catch2
      GIT_TAG v3.16.0)
    FetchContent_MakeAvailable(Catch2)
  endif()
  enable_testing()
  add_subdirectory(tests)
endif()
