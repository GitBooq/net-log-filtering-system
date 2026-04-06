/**
 * @file streamproc.tpp
 * @brief Stream Processor impl
 */

#pragma once

#include <algorithm>    // for __copy_fn, copy
#include <cstddef>      // for size_t
#include <istream>      // for char_traits, basic_istream, operator<<
#include <iterator>     // for ostream_iterator
#include <optional>     // for optional, nullopt
#include <ranges>       // for transform_view, all_t, _Partial, _Tran...
#include <string>       // for basic_string, string, getline
#include <string_view>  // for string_view
#include <vector>       // for vector

#include "details/filter.hpp"  // for FilterType
#include "details/ipaddr.hpp"  // for IPv4Address

namespace net::details {

template <FilterType T>
void StreamProcessor::Process(std::istream& input, std::ostream& output,
                              const T& filter) {
  std::string line;
  size_t line_number = 0;

  while (std::getline(input, line)) {
    ++line_number;

    auto ip = ParseIPFromLine(line);

    if (ip && filter.Matches(*ip)) {
      buffer_.push_back({*ip, line, line_number});
    }

    if (buffer_.size() >= batch_size_) {
      FlushBuffer(output);
    }
  }

  if (!buffer_.empty()) FlushBuffer(output);
}

inline std::optional<IPv4Address> StreamProcessor::ParseIPFromLine(
    const std::string& line) const {
  size_t end = line.find(' ');
  if (end == std::string::npos) {
    return std::nullopt;
  }

  std::string_view ip_str(line.data(), end);
  return IPv4Address::FromString(ip_str);
}

// Output buffer to ostream and clear
inline void StreamProcessor::FlushBuffer(std::ostream& output) {
  if (buffer_.empty()) return;

  std::ranges::copy(buffer_ | std::views::transform(&LogEntry::line),
                    std::ostream_iterator<std::string>(output, "\n"));

  buffer_.clear();
  output.flush();
}
}  // namespace net::details