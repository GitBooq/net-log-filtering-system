#include <exception>  // for exception
#include <fstream>    // for basic_ostream, char_traits, opera...
#include <iostream>   // for cerr, cout
#include <sstream>    // for stringstream
#include <string>     // for allocator, basic_string, operator<<
#include <vector>     // for vector

#include "netlogger/netlogger.hpp"  // for FilterConfig, create_filter, proc...

using namespace net::logger;

int main() try {
  std::ifstream input("test.log");

  if (!input.is_open()) {
    std::cerr << "Can't open file\n";
    return 1;
  }

  std::stringstream buffer;
  buffer << input.rdbuf();

  auto subnet_filter1 =
      CreateFilter({{"type", "subnet", "value", "192.168.1.0/24"}});

  auto subnet_filter2 =
      CreateFilter({{"type", "subnet", "value", "10.0.0.0/24"}});

  auto range_filter =
      CreateFilter({{"type", "range", "value", "192.168.1.10-192.168.1.20"}});

  auto composite_filter =
      CreateFilter({{"type", "subnet", "value", "192.168.1.0/24"},
                    {"type", "range", "value", "192.168.1.10-192.168.1.30"}});

  ProcessStream(buffer, std::cout, subnet_filter1);
  buffer.clear();
  buffer.seekg(0, std::ios::beg);

  std::cout << std::string(40, '/') << std::endl;
  ProcessStream(buffer, std::cout, subnet_filter2);
  buffer.clear();
  buffer.seekg(0, std::ios::beg);

  std::cout << std::string(40, '/') << std::endl;
  ProcessStream(buffer, std::cout, range_filter);
  buffer.clear();
  buffer.seekg(0, std::ios::beg);

  std::cout << std::string(40, '/') << std::endl;
  ProcessStream(buffer, std::cout, composite_filter);

  return 0;
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "Unknown exception caught" << std::endl;
  return 1;
}