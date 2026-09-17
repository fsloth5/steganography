#include <core/megan.hpp>
#include <iostream>

namespace megan {
void encrypt() { std::cout << "Encrypting...\n"; }
bool host_uses_crlf() {
  return (platform_name_v == "DOS" || platform_name_v == "OS2" ||
          platform_name_v == "Windows" || platform_name_v == "WindowsCE" ||
          platform_name_v == "WindowsPhone" ||
          platform_name_v == "WindowsStore");
}

} // namespace megan
