#pragma once

#include <array>             // for array
#include <cstddef>           // for size_t
#include <exception>         // for exception
#include <initializer_list>  // for initializer_list
#include <iosfwd>            // for istream, ostream
#include <optional>          // for optional
#include <string>            // for allocator, operator==, char_traits
#include <utility>           // for move
#include <vector>            // for vector

#include "details/filter.hpp"      // for CompositeFilter, RangeFilter, Subn...
#include "details/streamproc.hpp"  // for StreamProcessor

namespace net::logger {

using net::details::CompositeFilter;
using net::details::RangeFilter;
using net::details::StreamProcessor;
using net::details::SubnetFilter;

class FilterConfigError final : public std::exception {
 public:
  FilterConfigError(std::string msg) noexcept : msg_(std::move(msg)) {}

  const char* what() const noexcept override { return msg_.c_str(); }

 private:
  std::string msg_;
};

struct FilterConfig {
  std::string type;
  std::string value;

  static constexpr size_t FIELDS = 4;

  FilterConfig(std::initializer_list<std::string> il) {
    if (il.size() != FIELDS) {
      throw FilterConfigError("FilterConfig expects 4 fields");
    }
    const std::string* arr = il.begin();
    auto [key1, val1, key2, val2] = std::tuple{arr[0], arr[1], arr[2], arr[3]};

    if (key1 != "type" || key2 != "value") {
      throw FilterConfigError(
          "Invalid format: expected 'type' and 'value' keys");
    }

    type = val1;
    value = val2;
  }
};

/**
 * Create filter with AND-semantics
 */
inline CompositeFilter create_filter(const std::vector<FilterConfig>& configs) {
  CompositeFilter filter;

  for (const auto& cfg : configs) {
    if (cfg.type == "subnet") {
      if (auto subnet = SubnetFilter::create(cfg.value)) {
        filter.add(std::move(*subnet));
      }
    } else if (cfg.type == "range") {
      if (auto range = RangeFilter::create(cfg.value))
        filter.add(std::move(*range));
    }
  }

  return filter;
}

inline void process_stream(std::istream& input, std::ostream& output,
                           const CompositeFilter& filter) {
  StreamProcessor processor;
  processor.process(input, output, filter);
}

}  // namespace net::logger