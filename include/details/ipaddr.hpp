/**
 * @file ipaddr.hpp
 * @brief IPv4Address class
 */
#pragma once

#include <array>        // for array
#include <compare>      // for strong_ordering, operator==
#include <cstdint>      // for uint32_t
#include <optional>     // for optional
#include <string_view>  // for string_view

namespace net::details {

/**
 * @brief IPv4 address representation
 *
 */
struct IPv4Address {
 private:
  using oct_arr_t = std::array<uint32_t, 4>;

  uint32_t addr;  ///< BigEndian ip representation

  /**
   * @brief Construct a new IPv4Address object from 4 bytes array
   * IP stored in network byte order (BigEndian)
   * @param octets 4 bytes array to construct from
   */
  explicit IPv4Address(oct_arr_t octets) noexcept
      : addr{(octets[0] << 24) | (octets[1] << 16) | (octets[2] << 8) |
             octets[3]} {}

 public:
  /**
   * @brief Construct a new empty IPv4Address object
   *
   */
  IPv4Address() : addr{0} {}
  IPv4Address(const IPv4Address&) = default;
  IPv4Address(IPv4Address&&) = default;
  ~IPv4Address() noexcept = default;

  IPv4Address& operator=(const IPv4Address&) = default;
  IPv4Address& operator=(IPv4Address&&) = default;

  /**
   * @brief Create IP from 4 bytes array
   * Perform validity check
   * @param octets 4 bytes array
   * @return std::optional<IPv4Address>
   */
  static std::optional<IPv4Address> from_bytes(oct_arr_t octets);
  /**
   * @brief Create IP from string
   * Perform validity check
   * @param str_view string to create from
   * @return std::optional<IPv4Address>
   */
  static std::optional<IPv4Address> from_string(std::string_view str_view);

  /**
   * @brief Get IP in bytes(network order)
   *
   * @return uint32_t
   */
  uint32_t to_uint32() const noexcept { return addr; }

  auto operator<=>(const IPv4Address&) const noexcept = default;
};

/**
 * @brief Check if string contains valid symbols for ip
 *
 * @param ip
 * @return true
 * @return false
 */
inline bool containValidIPSymbols(std::string_view ip);
}  // namespace net::details

#include "details/ipaddr.tpp"