#include <bit>
#include <charconv>
#include <string>
#include <utility>

#include <core/scanner.hpp>
#include <core/utils.hpp>

namespace {

enum class ppm_phase_t : megan::utils::u8 {
  MAGIC_NUMBER,
  WIDTH,
  HEIGHT,
  MAX_COLOR_VAL,
  END_OF_HEADER,
  PIXELS,
};

[[nodiscard]] inline bool is_whitespace_char(const char c) {
  return c == ' ' || c == '#' || c == '\r' || c == '\t' || c == '\n' ||
         c == '\0';
}

void skip_whitespace(megan::scanner::scanner_t &scanner) {
  if (scanner.peek() == '#') {
    scanner.current = scanner.source_line.length();
    scanner.advance_line();
    return;
  }

  while (true) {
    switch (scanner.peek()) {
    case '\n':
      ++scanner.current;
      scanner.advance_line();
      break;

    case ' ':
    case '\r':
    case '\t':
      scanner.current += 1;
      break;

    default:
      if (scanner.skipped_line()) {
        scanner.last_line = scanner.current_line;
      }
      return;
    }
  }
}

std::optional<megan::utils::u32> number(megan::scanner::scanner_t &scanner) {
  while (std::isdigit(scanner.peek())) {
    scanner.advance();
  }

  megan::utils::u32 num{};

  auto [ptr, ec] = std::from_chars(
      scanner.source_line.data() + scanner.start,
      scanner.source_line.data() + (scanner.current - scanner.start), num);

  if (ec == std::errc()) {
    return std::optional{num};
  }

  scanner.err_message = ec == std::errc::invalid_argument
                            ? "Expected number"
                            : "Value exceeds storage";

  return std::optional<megan::utils::u32>{};
}

} // namespace

namespace megan::scanner {

std::optional<scanner::ppm_file_header_t>
parse_ppm_file_header(std::ifstream &source, scanner::scanner_t &scanner) {
  auto const found_magic_number = [](const std::string &line) {
    auto version = line[1];
    return std::isalpha(line[0]) && (version == '3' || version == '6');
  };

  auto const magic_number = [](ppm_file_header_t &header,
                               ppm_phase_t &ppm_phase, const std::string &line,
                               scanner::scanner_t &scanner) {
    header.magic_number = {line[0], static_cast<utils::u8>(line[1])};
    ppm_phase = ppm_phase_t::WIDTH;
    scanner.current += 2;
  };

  std::string line{};

  auto header = scanner::ppm_file_header_t{};

  auto ppm_phase = ppm_phase_t::MAGIC_NUMBER;

  std::getline(source, line);
  scanner.set_source_line(line);

  while (true) {
    if (ppm_phase == ppm_phase_t::END_OF_HEADER) {
      break;
    }

    if (scanner.is_at_line_end()) {
      std::getline(source, line);

      scanner.set_source_line(line);
    }

    skip_whitespace(scanner);

    if (scanner.skipped_line()) {
      continue;
    }

    scanner.start = scanner.current;

    if (ppm_phase == ppm_phase_t::MAGIC_NUMBER) {
      if (found_magic_number(line)) {
        magic_number(header, ppm_phase, line, scanner);
        continue;
      }

      scanner.err_message = "Magic number not found!";
      break;
    }

    auto value = number(scanner);

    if (!value) {
      break;
    }

    if (ppm_phase == ppm_phase_t::WIDTH) {
      header.image_width = *value;
      ppm_phase = ppm_phase_t::HEIGHT;
    } else if (ppm_phase == ppm_phase_t::HEIGHT) {
      header.image_height = *value;
      ppm_phase = ppm_phase_t::MAX_COLOR_VAL;
    } else if (ppm_phase == ppm_phase_t::MAX_COLOR_VAL) {
      header.max_color_val = static_cast<megan::utils::u16>(*value);
      ppm_phase = ppm_phase_t::END_OF_HEADER;
    }
  }

  if (is_whitespace_char(scanner.peek())) {
    ppm_phase = ppm_phase_t::PIXELS;
  } else {
    scanner.err_message = "End of header cannot be determined";
  }

  return scanner.err_message.empty() && ppm_phase == ppm_phase_t::PIXELS
             ? std::optional{header}
             : std::optional<scanner::ppm_file_header_t>{};
}

std::optional<ppm_file_t> parse_ppm_file(std::ifstream &source,
                                         scanner_t &scanner) {
  using namespace utils;

  auto parsed_header = parse_ppm_file_header(source, scanner);

  if (!parsed_header) {
    return {};
  }

  auto header = *parsed_header;

  const auto n =
      static_cast<usize>(3 * header.image_width * header.image_height);

  auto pixels = std::vector<u16>{};
  pixels.reserve(n);

  std::string line{};

  const bool uses_big_endian{header.max_color_val >= 256 &&
                             std::endian::native == std::endian::little};

  auto on_p3_format = [](scanner_t &scanner, std::ifstream &source,
                         std::string &line, std::vector<u16> &pixels) -> void {
    while (std::getline(source, line)) {
      scanner.set_source_line(line);

      while (!scanner.is_at_line_end()) {
        skip_whitespace(scanner);

        scanner.start = scanner.current;

        auto parsed = number(scanner);

        if (!parsed) {
          break;
        }

        auto value = static_cast<u16>(*parsed);

        pixels.push_back(value);
      }

      scanner.advance_line();
    }
  };

  auto const on_p6_format =
      [n, uses_big_endian](std::istream &source,
                           std::vector<u16> &pixels) -> void {
    auto const on_p6_parse =
        [n](auto data, std::istream &source, std::vector<u16> &pixels)
      requires(std::is_same_v<decltype(data), u8> ||
               std::is_same_v<decltype(data), u16>)
    {
      using data_t = decltype(data);

      std::vector<data_t> buffer{};

      buffer.reserve(n);

      auto ssize = static_cast<std::streamsize>(n);

      source.read(reinterpret_cast<char *>(buffer.data()), ssize);

      if (source.gcount() != ssize) {
        throw std::runtime_error("short read");
      }

      for (usize i{}, len = n; i < len; ++i) {
        if constexpr (std::is_same_v<data_t, u16>) {
          pixels[i] = std::rotl(buffer[i], 8);
        } else {
          pixels[i] = buffer[i];
        }
      }
    };

    if (uses_big_endian) {
      on_p6_parse(u16{}, source, pixels);
    } else {
      on_p6_parse(u8{}, source, pixels);
    }
  };

  scanner.start = scanner.current = 0;

  if (header.magic_number.second == '3') {
    on_p3_format(scanner, source, line, pixels);
  } else {
    on_p6_format(source, pixels);
  }

  return std::optional<ppm_file_t>{
      ppm_file_t{.header = header, .pixels = std::move(pixels)}};
}
} // namespace megan::scanner
