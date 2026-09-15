#include <catch2/catch_test_macros.hpp>
#include <core/megan.hpp>
#include <cstdint>

namespace {
uint32_t factorial(const uint32_t number) {
  return number <= 1 ? number : factorial(number - 1) * number;
}
} // namespace

// NOLINTBEGIN
TEST_CASE("Factorials are computed", "[factorial]") {
  REQUIRE(factorial(1) == 1);
}
// NOLINTEND
