#include <iostream>
#include <vector>

#include <core/megan.hpp>
#include <core/platform.hpp>
#include <core/scanner.hpp>
#include <core/utils.hpp>

namespace megan {

void encrypt() { std::cout << "Encrypting...\n"; }

struct ppm_file_t final {
  scanner::ppm_file_header_t header{};
  std::vector<utils::u16> pixels{};
};

} // namespace megan
