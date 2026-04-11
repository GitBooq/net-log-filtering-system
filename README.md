# IPv4 Log Filter

A streaming log processing system with IPv4 address filtering capabilities.

## Features
- **IPv4 parsing** from logs in `IP - message` format
- **Filter types:**
  - CIDR subnets (e.g., `192.168.0.0/24`)
  - IP ranges (e.g., `10.0.0.1-10.0.0.255`)
  - Combined filters(up to 20) using logical `OR`
- **Streaming processing** with buffering up to 1000 records

## Usage Example

```cpp
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "netlogger/netlogger.hpp"

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
      CreateFilter({{"type", "subnet", "value", "10.0.0.0/24"},
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
```

## Test input
```
# Valid IPv4
192.168.1.25 - GET /index.html
10.0.0.100 - POST /login

# Edge cases
0.0.0.0 - DEFAULT_ROUTE
255.255.255.255 - BROADCAST
192.168.1.0 - NETWORK_ADDR
192.168.1.255 - BROADCAST_ADDR

# Invalid data
192.168.1.256 - INVALID_OCTET
192.168.1 - INCOMPLETE
invalid.ip - CORRUPTED
 - EMPTY_LINE

# Subnet 192.168.1.0/24 tests
192.168.1.1 - SUBNET_VALID
192.168.2.1 - SUBNET_INVALID

# Range 192.168.1.10-192.168.1.20 tests
192.168.1.10 - RANGE_LOWER
192.168.1.15 - RANGE_MIDDLE
192.168.1.20 - RANGE_UPPER
192.168.1.9 - RANGE_BELOW
192.168.1.21 - RANGE_ABOVE

# Composite AND tests (192.168.1.0/24 AND 192.168.1.10-192.168.1.30)
192.168.1.25 - COMPOSITE_VALID
192.168.1.30 - COMPOSITE_UPPER
192.168.1.5 - COMPOSITE_INVALID_SUBNET_YES_RANGE_NO
192.168.2.15 - COMPOSITE_INVALID_SUBNET_NO_RANGE_YES
```

## Test Output
```
192.168.1.25 - GET /index.html
192.168.1.0 - NETWORK_ADDR
192.168.1.255 - BROADCAST_ADDR
192.168.1.1 - SUBNET_VALID
192.168.1.10 - RANGE_LOWER
192.168.1.15 - RANGE_MIDDLE
192.168.1.20 - RANGE_UPPER
192.168.1.9 - RANGE_BELOW
192.168.1.21 - RANGE_ABOVE
192.168.1.25 - COMPOSITE_VALID
192.168.1.30 - COMPOSITE_UPPER
192.168.1.5 - COMPOSITE_INVALID_SUBNET_YES_RANGE_NO
////////////////////////////////////////
10.0.0.100 - POST /login
////////////////////////////////////////
192.168.1.10 - RANGE_LOWER
192.168.1.15 - RANGE_MIDDLE
192.168.1.20 - RANGE_UPPER
////////////////////////////////////////
192.168.1.25 - GET /index.html
192.168.1.10 - RANGE_LOWER
192.168.1.15 - RANGE_MIDDLE
192.168.1.20 - RANGE_UPPER
192.168.1.21 - RANGE_ABOVE
192.168.1.25 - COMPOSITE_VALID
192.168.1.30 - COMPOSITE_UPPER
```

## Build & Run

```bash
clone repo
run ./buildAndRun.sh