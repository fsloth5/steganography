#pragma once

#include <string_view>

#include <core/scanner.hpp>
#include <core/utils.hpp>

namespace megan {

enum class status_t : utils::u8 {
  OK,
  IMAGE_TOO_SMALL,
};

void hide();

status_t hide_in_ppm(std::string_view message, scanner::ppm_file_t &file);

} // namespace megan
