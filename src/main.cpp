#include <exception>  // for exception
#include <fstream>    // for basic_ostream, char_traits, opera...
#include <iostream>   // for cerr, cout
#include <string>     // for allocator, basic_string, operator<<
#include <vector>     // for vector

#include "details/streamproc.hpp"   // for StreamProcessor
#include "netlogger/netlogger.hpp"  // for FilterConfig, create_filter, proc...

using namespace net::logger;

int main() try {
  StreamProcessor sp;
  auto filter =
      create_filter({{"type", "subnet", "value", "0.0.0.0/0"},
                     {"type", "range", "value", "0.0.0.0-255.255.255.255"}});

  std::ifstream input("test.log");

  if (!input.is_open()) {
    std::cerr << "Can't open file\n";
    return 1;
  }

  std::cout << std::string(40, '/') << std::endl;
  process_stream(input, std::cout, filter);
  std::cout << std::string(40, '\\') << std::endl;

} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
} catch (...) {
  std::cerr << "Unknown exception caught" << std::endl;
}