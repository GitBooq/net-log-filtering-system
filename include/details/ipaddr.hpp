#pragma once

#include <array>        // for array
#include <compare>      // for strong_ordering, operator==
#include <cstdint>      // for uint32_t
#include <optional>     // for optional
#include <string_view>  // for string_view

namespace net::details {

struct IPv4Address {
 private:
  using oct_arr_t = std::array<uint32_t, 4>;

  uint32_t addr;

  explicit IPv4Address(oct_arr_t octets) noexcept
      : addr{(octets[0] << 24) | (octets[1] << 16) | (octets[2] << 8) |
             octets[3]} {}

 public:
  // IPv4Address() = delete;
  IPv4Address() : addr{0} {}
  IPv4Address(const IPv4Address&) = default;
  IPv4Address(IPv4Address&&) = default;
  ~IPv4Address() noexcept = default;

  IPv4Address& operator=(const IPv4Address&) = default;
  IPv4Address& operator=(IPv4Address&&) = default;

  static std::optional<IPv4Address> from_bytes(oct_arr_t octets);
  static std::optional<IPv4Address> from_string(std::string_view str_view);

  uint32_t to_uint32() const noexcept { return addr; }

  auto operator<=>(const IPv4Address&) const noexcept = default;
};

inline bool containValidIPSymbols(std::string_view ip);
}  // namespace net::details

#include "details/ipaddr.tpp"