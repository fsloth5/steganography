add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE "${PROJECT_NAME}::core")
add_subdirectory(core)
