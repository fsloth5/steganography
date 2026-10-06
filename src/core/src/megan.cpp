#include <cmath>
#include <iostream>
#include <vector>

#include <core/megan.hpp>
#include <core/platform.hpp>
#include <core/scanner.hpp>
#include <core/utils.hpp>

namespace megan {

void hide() { std::cout << "Encrypting...\n"; }

status_t hide_in_ppm(std::string_view message, scanner::ppm_file_t &file) {
  using namespace utils;

  const auto channel_count = file.header.channel_count;

  const auto message_size =
      static_cast<usize>(std::ceil(message.size() * 8 / channel_count));

  const auto image_size =
      static_cast<usize>(std::ceil(file.pixels.size() / channel_count));

  if (image_size < message_size) {
    return status_t::IMAGE_TOO_SMALL;
  }

  auto &pixels = file.pixels;

  for (usize stride{}, message_index{}, message_size = 8 * message.length();
       stride < message_size; stride += 8, ++message_index) {

    for (u8 bit{}; bit < 8; ++bit) {
      // Builds a mask by shifting the bits of the current character in the
      // message to the right and selects the right-most
      const auto mask = (message[message_index] >> bit) & 1;
      // Resets the right most bit of the current byte (if the current
      // character requires it) then joins the mask--if needed
      pixels[stride + bit] = (pixels[stride + bit] & 0xFE) | mask;
    }
  }

  return status_t::OK;
}

} // namespace megan
