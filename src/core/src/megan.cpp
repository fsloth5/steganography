#include <array>
#include <iostream>
#include <vector>

#include <core/megan.hpp>
#include <core/platform.hpp>
#include <core/utils.hpp>

namespace megan {

void encrypt() { std::cout << "Encrypting...\n"; }

struct ppm_file_t final {
  struct ppm_header_t final {
    std::array<char, 2> magic_number{};
    utils::u32 image_width{};
    utils::u32 image_height{};
    utils::u16 max_color_val{};
  } header;
  std::vector<utils::u16> pixels{};
};

bool is_ppm_whitespace(char value) {
  return (value == '\r' || value == '\n' || value == ' ' || value == '\t');
}

bool host_uses_crlf() {
  return (platform_name_v == "DOS" || platform_name_v == "OS2" ||
          platform_name_v == "Windows" || platform_name_v == "WindowsCE" ||
          platform_name_v == "WindowsPhone" ||
          platform_name_v == "WindowsStore");
}

} // namespace megan
