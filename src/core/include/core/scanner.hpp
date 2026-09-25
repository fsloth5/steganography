#pragma once

#include <fstream>
#include <optional>
#include <string_view>
#include <vector>

#include <core/utils.hpp>

namespace megan::scanner {

struct scanner_t final {
  utils::usize start{};
  utils::usize current{};
  utils::usize current_line{};
  utils::usize last_line{};
  std::string_view source_line{};
  std::string_view err_message{};

  void advance() { ++current; }

  void advance_line() { last_line = current_line++; }

  [[nodiscard]] char peek() const { return source_line[current]; }

  [[nodiscard]] bool is_at_end() const {
    return current == source_line.length();
  }

  [[nodiscard]] bool skipped_line() const { return last_line != current_line; }

  void set_source_line(std::string_view line) {
    source_line = line;
    current = 0;
  }
};

struct ppm_file_header_t final {
  std::pair<char, utils::u8> magic_number{};
  utils::u32 image_width{};
  utils::u32 image_height{};
  utils::u16 max_color_val{};
  [[nodiscard]] utils::u8 version() const { return magic_number.second; }
};

struct ppm_file_t final {
  ppm_file_header_t header{};
  std::vector<utils::u16> pixels{};
};

std::optional<scanner::ppm_file_header_t>
parse_ppm_file_header(std::ifstream &source, scanner::scanner_t &scanner);

std::optional<ppm_file_t> parse_ppm_file(std::ifstream &source,
                                         scanner_t &scanner);

} // namespace megan::scanner
