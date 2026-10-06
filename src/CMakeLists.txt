add_executable(${PROJECT_NAME} main.cpp)

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE TEST_FILE="${ROOT_DIR}/tests/multiple_per_line_p3_big_endian.ppm")

target_link_libraries(${PROJECT_NAME} PRIVATE "${PROJECT_NAME}::core")

add_subdirectory(core)
