include(FetchContent)

FetchContent_Declare(
  libspng
  GIT_REPOSITORY https://github.com/randy408/libspng
  GIT_TAG v0.7.4)
FetchContent_MakeAvailable(libspng)

set(LIB_NAME "${PROJECT_NAME}_lib")

add_library(${LIB_NAME} STATIC "include/core/megan.hpp" "src/megan.cpp")
add_library(megan::core ALIAS ${LIB_NAME})

target_compile_features(${LIB_NAME} PUBLIC cxx_std_20)
target_include_directories(${LIB_NAME} PUBLIC "include")
target_link_libraries(${LIB_NAME} PRIVATE spng_static)
