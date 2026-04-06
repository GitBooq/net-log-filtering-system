/**
 * @file ipaddr.tpp
 * @brief IPv4Address class impl
 */
#pragma once

#include <charconv>      // for from_chars, from_chars_result
#include <cstddef>       // for size_t
#include <cstdint>       // for uint32_t
#include <iterator>      // for size
#include <optional>      // for optional, nullopt
#include <string_view>   // for string_view
#include <system_error>  // for errc

namespace net::details {

class IPv4Address;

inline std::optional<IPv4Address> IPv4Address::FromString(
    std::string_view str_view) {
  if (!ContainValidIPSymbols(str_view)) return std::nullopt;

  OctArr octets;
  const char* ptr = str_view.data();
  const char* end = ptr + str_view.size();
  int dot_count = 0;
  constexpr char dot = '.';

  for (std::size_t i = 0; i < std::size(octets); ++i) {
    // empty octet check
    if (ptr >= end || *ptr == dot) {
      return std::nullopt;
    }

    // Leading zero check
    if (*ptr == '0') {
      // next is not dot
      if (ptr + 1 < end && *(ptr + 1) != dot) {
        return std::nullopt;
      }
    }

    uint32_t value;

    auto [next, ec] = std::from_chars(ptr, end, value);

    if (ec != std::errc() || next == ptr) {
      return std::nullopt;
    }

    octets[i] = value;
    ptr = next;  // can point to '.' or end

    // xxx.xxx.xxx.xxx no dot at the end
    if (i < 3) {
      if (ptr >= end || *ptr != dot) return std::nullopt;
      ++ptr;  // skip '.'
      ++dot_count;
    }
  }

  // should not be any symbols after IP and exact 3 dots
  if (ptr != end || dot_count != 3) return std::nullopt;

  return FromBytes(octets);
}

inline std::optional<IPv4Address> IPv4Address::FromBytes(OctArr octets) {
  for (auto octet : octets) {
    if (octet > 255) {
      return std::nullopt;
    }
  }
  return IPv4Address(octets);
}

inline bool ContainValidIPSymbols(std::string_view ip) {
  for (char ch : ip) {
    if (!(ch >= '0' && ch <= '9') && ch != '.') {
      return false;
    }
  }

  return true;
}
}  // namespace net::details