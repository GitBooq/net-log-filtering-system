# IPv4 Log Filter

A streaming log processing system with IPv4 address filtering capabilities.

## Features
- **IPv4 parsing** from logs in `IP - message` format
- **Filter types:**
  - CIDR subnets (e.g., `192.168.0.0/24`)
  - IP ranges (e.g., `10.0.0.1-10.0.0.255`)
  - Combined filters using logical `OR`
- **Streaming processing** with buffering up to 1000 records

## Usage Example

```cpp
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "netlogger/netlogger.hpp"

int main() {
  // Create filter: IP must satisfy at least one rule
  auto filter =
      create_filter({{"type", "subnet", "value", "192.168.0.0/24"},
                      {"type", "range", "value", "10.0.0.1-10.0.0.100"}});

  std::ifstream input("test.log");
  // Process stream and output to console
  process_stream(input, std::cout, filter);
}
```

## Build & Run

```bash
clone repo
run ./buildAndRun.sh