/**
 * @file filter.tpp
 * @brief Filters impl
 */
#pragma once

#include <charconv>      // for from_chars, from_chars_result
#include <cstdint>       // for uint32_t
#include <optional>      // for optional, nullopt
#include <string_view>   // for string_view, basic_string_view
#include <system_error>  // for errc
#include <variant>       // for visit, variant
#include <vector>        // for vector

#include "ipaddr.hpp"  // for IPv4Address

namespace net::details {
// Forward dcls
class RangeFilter;
class SubnetFilter;

constexpr uint32_t Create32BitMask(uint32_t prefix) {
  return (prefix == 0) ? 0 : (0xFFFFFFFF << (32 - prefix));
}
constexpr bool IsValidRange(uint32_t left, uint32_t right) {
  return left <= right;
}

inline std::optional<SubnetFilter> SubnetFilter::Create(std::string_view cidr) {
  auto slash_pos = cidr.find('/');
  if (slash_pos == std::string_view::npos) return std::nullopt;

  auto ip_str = cidr.substr(0, slash_pos);
  auto ip = IPv4Address::FromString(ip_str);
  if (!ip) return std::nullopt;

  auto prefix_str = cidr.substr(slash_pos + 1);
  int prefix;
  auto [ptr, ec] = std::from_chars(
      prefix_str.data(), prefix_str.data() + prefix_str.size(), prefix);

  if (ec != std::errc() || prefix < 0 || prefix > 32) {
    return std::nullopt;
  }

  uint32_t mask = Create32BitMask(prefix);
  uint32_t network = ip->ToUint32() & mask;

  return SubnetFilter(network, mask);
}

inline std::optional<RangeFilter> RangeFilter::Create(std::string_view range) {
  char sep = '-';
  auto dash_pos = range.find(sep);
  if (dash_pos == std::string_view::npos) return std::nullopt;

  auto left_str = range.substr(0, dash_pos);
  auto right_str = range.substr(dash_pos + 1);

  auto left = IPv4Address::FromString(left_str);
  auto right = IPv4Address::FromString(right_str);

  if (!left || !right) return std::nullopt;
  if (!IsValidRange(left->ToUint32(), right->ToUint32())) return std::nullopt;

  return RangeFilter{left.value(), right.value()};
}

inline std::optional<RangeFilter> RangeFilter::Create(std::string_view left,
                                                      std::string_view right) {
  auto ip_left = IPv4Address::FromString(left);
  auto ip_right = IPv4Address::FromString(right);

  if (!ip_left || !ip_right) return std::nullopt;
  if (!IsValidRange(ip_left->ToUint32(), ip_right->ToUint32()))
    return std::nullopt;

  return RangeFilter{ip_left.value(), ip_right.value()};
}

inline bool CompositeFilter::Matches(const IPv4Address& ip) const {
  for (const auto& f : filters_) {
    if (std::visit([&ip](const auto& filter) { return filter.Matches(ip); }, f))
      return true;
  }
  return false;
}
}  // namespace net::details
