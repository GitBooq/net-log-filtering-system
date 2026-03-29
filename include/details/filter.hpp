/**
 * @file filter.hpp
 * @brief Filters
 */

#pragma once

#include <algorithm>    // for clamp
#include <concepts>     // for same_as
#include <cstdint>      // for uint32_t
#include <optional>     // for optional
#include <string_view>  // for string_view
#include <utility>      // for forward, pair
#include <variant>      // for variant
#include <vector>       // for vector

#include "ipaddr.hpp"  // for IPv4Address

namespace net::details {

/**
 * @brief CRTP base for all filters
 */
template <typename Derived>
struct Filter {
  /**
   * @brief Check if ip matches the filter
   *
   * @param ip ip address
   * @return true
   * @return false
   */
  bool matches(const IPv4Address& ip) const {
    return static_cast<const Derived*>(this)->matches_impl(ip);
  }

 protected:
  ~Filter() = default;
};

/**
 * @brief Concept to constrain T to filters
 */
template <typename T>
concept FilterType = requires(const T& t, const IPv4Address& ip) {
  { t.matches(ip) } -> std::same_as<bool>;
};

/**
 * @brief Subnet filter
 *  CIDR filtering
 */
class SubnetFilter : public Filter<SubnetFilter> {
 private:
  uint32_t network_;  ///< network address
  uint32_t mask_;     ///< network mask

  SubnetFilter(uint32_t network, uint32_t mask) noexcept
      : network_(network), mask_(mask) {}

 public:
  /**
   * @brief Creates filter from CIDR string
   *
   * @param cidr CIDR string
   * @return std::optional<SubnetFilter>
   */
  static std::optional<SubnetFilter> create(std::string_view cidr);

  /**
   * @brief Check if ip matches filter
   *
   * @param ip ip to check
   * @return true
   * @return false
   */
  bool matches_impl(const IPv4Address& ip) const noexcept {
    return (ip.to_uint32() & mask_) == network_;
  }
};

/**
 * @brief IP Range filter
 *
 */
class RangeFilter : public Filter<RangeFilter> {
 private:
  std::pair<IPv4Address, IPv4Address> ip_range;

  RangeFilter(const IPv4Address& first, const IPv4Address& last)
      : ip_range{first, last} {}

 public:
  /**
   * @brief Creates filter from range string
   *
   * @param range range string
   * @return std::optional<RangeFilter>
   */
  static std::optional<RangeFilter> create(std::string_view range);

  /**
   * @brief Creates filter from two strings
   *
   * @param left lo bound
   * @param right hi bound
   * @return std::optional<RangeFilter>
   */
  static std::optional<RangeFilter> create(std::string_view left,
                                           std::string_view right);

  /**
   * @brief Check if ip matches filter
   *
   * @param ip ip to check
   * @return true
   * @return false
   */
  bool matches_impl(const IPv4Address& ip) const noexcept {
    const auto& [left, right] = ip_range;
    return ip == std::clamp(ip, left, right);
  }
};

/**
 * @brief Creates 32bit network mask from prefix
 *
 * @param prefix subnet prefix
 * @return constexpr uint32_t
 */
constexpr uint32_t create32BitMask(uint32_t prefix);

/**
 * @brief Check if range [left, right] is valid
 *
 * @param left lo
 * @param right hi
 * @return true
 * @return false
 */
constexpr bool isValidRange(uint32_t left, uint32_t right);

/**
 * @brief Class to compose and store all filters
 *
 */
class CompositeFilter {
  using VarFilter = std::variant<SubnetFilter, RangeFilter>;  ///< fixed filters

 private:
  std::vector<VarFilter> filters_;

 public:
  /**
   * @brief Adds filter
   *
   * @tparam T Filter
   * @param filter filter to add
   */
  template <FilterType T>
  void add(T&& filter) {
    filters_.push_back(std::forward<T>(filter));
  }

  /**
   * @brief Check if ip mathes all stored filters
   *
   * @param ip ip to check
   * @return true
   * @return false
   */
  bool matches(const IPv4Address& ip) const;
};
}  // namespace net::details

#include "details/filter.tpp"