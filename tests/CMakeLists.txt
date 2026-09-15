list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

function(make_test test_name)
  add_executable(${test_name} "${test_name}.cpp")
  target_link_libraries(${test_name} PRIVATE "${PROJECT_NAME}::core"
                                             Catch2::Catch2WithMain)
  catch_discover_tests(${test_name})
endfunction()

make_test(dummy)
