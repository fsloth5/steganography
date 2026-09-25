#include <charconv>
#include <string>

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
  auto const on_magic_number_parsed =
      [](ppm_file_header_t &header, ppm_phase_t &ppm_phase,
         const std::string &line, scanner::scanner_t &scanner) {
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
      if (is_whitespace_char(scanner.peek())) {
        ppm_phase = ppm_phase_t::PIXELS;
      } else {
        scanner.err_message = "End of header cannot be determined";
      }
      break;
    }

    if (scanner.is_at_end()) {
      std::getline(source, line);

      scanner.set_source_line(line);
    }

    skip_whitespace(scanner);

    if (scanner.skipped_line()) {
      continue;
    }

    scanner.start = scanner.current;

    if (ppm_phase == ppm_phase_t::MAGIC_NUMBER) {
      if (auto version = line[1];
          std::isalpha(line[0]) && (version == '3' || version == '6')) {
        on_magic_number_parsed(header, ppm_phase, line, scanner);
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

  return scanner.err_message.empty() && ppm_phase == ppm_phase_t::PIXELS
             ? std::optional{header}
             : std::optional<scanner::ppm_file_header_t>{};
}

std::optional<ppm_file_t> parse_ppm_file(std::ifstream &source,
                                         scanner_t &scanner) {
  auto parsed_header = parse_ppm_file_header(source, scanner);

  if (!parsed_header) {
    return {};
  }

  auto header = *parsed_header;

  auto pixels = std::vector<utils::u16>{};

  pixels.reserve(3 * header.image_width * header.image_height);

  std::string line{};

  auto on_p3_format = [](scanner::scanner_t &scanner, std::ifstream &source,
                         std::string &line, std::vector<utils::u16> pixels) {
    while (std::getline(source, line)) {
      scanner.set_source_line(line);

      while (!scanner.is_at_end()) {
        scanner.start = scanner.current;

        skip_whitespace(scanner);

        auto value = number(scanner);

        if (!value) {
          break;
        }

        pixels.push_back(static_cast<utils::u16>(*value));
      }

      scanner.advance_line();
    }
  };

  auto const on_p6_format = [](scanner::scanner_t &scanner,
                               std::ifstream &source, std::string &line,
                               std::vector<utils::u16> pixels) {};

  if (header.magic_number.second == '3') {
    on_p3_format(scanner, source, line, pixels);
  } else {
    on_p6_format(scanner, source, line, pixels);
  }

  return std::optional<ppm_file_t>{
      ppm_file_t{.header = header, .pixels = std::move(pixels)}};
}
} // namespace megan::scanner
