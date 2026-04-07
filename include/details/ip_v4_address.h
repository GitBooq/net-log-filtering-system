/**
 * @file ip_v4_address.h
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
class IPv4Address {
 private:
  using OctArr = std::array<uint32_t, 4>;

  uint32_t addr_;  ///< BigEndian ip representation

  /**
   * @brief Construct a new IPv4Address object from 4 bytes array
   * IP stored in network byte order (BigEndian)
   * @param octets 4 bytes array to construct from
   */
  explicit IPv4Address(OctArr octets) noexcept
      : addr_{(octets[0] << 24) | (octets[1] << 16) | (octets[2] << 8) |
              octets[3]} {}

 public:
  /**
   * @brief Construct a new empty IPv4Address object
   *
   */
  IPv4Address() : addr_{0} {}
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
  static std::optional<IPv4Address> FromBytes(OctArr octets);
  /**
   * @brief Create IP from string
   * Perform validity check
   * @param str_view string to create from
   * @return std::optional<IPv4Address>
   */
  static std::optional<IPv4Address> FromString(std::string_view str_view);

  /**
   * @brief Get IP in bytes(network order)
   *
   * @return uint32_t
   */
  uint32_t ToUint32() const noexcept { return addr_; }

  auto operator<=>(const IPv4Address&) const noexcept = default;
};

/**
 * @brief Check if string contains valid symbols for ip
 *
 * @param ip
 * @return true
 * @return false
 */
inline bool ContainValidIPSymbols(std::string_view ip);
}  // namespace net::details

#include "details/ip_v4_address.inc"