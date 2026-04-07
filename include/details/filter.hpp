/**
 * @file filter.hpp
 * @brief Filters
 */

#pragma once

#include <algorithm>    // for clamp
#include <concepts>     // for same_as
#include <cstdint>      // for uint32_t
#include <format>       // for format
#include <optional>     // for optional
#include <stdexcept>    // for logic_error
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
   * @return true if IP matches filter, false otherwise
   */
  bool Matches(const IPv4Address& ip) const {
    return static_cast<const Derived*>(this)->MatchesImpl(ip);
  }

 protected:
  ~Filter() = default;
};

/**
 * @brief Concept to constrain T to filters
 */
template <typename T>
concept FilterType = requires(const T& t, const IPv4Address& ip) {
  { t.Matches(ip) } -> std::same_as<bool>;
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
  static std::optional<SubnetFilter> Create(std::string_view cidr);

  /**
   * @brief Check if ip matches filter
   *
   * @param ip ip to check
   * @return true if IP matches filter, false otherwise
   */
  bool MatchesImpl(const IPv4Address& ip) const noexcept {
    return (ip.ToUint32() & mask_) == network_;
  }
};

/**
 * @brief IP Range filter
 *
 */
class RangeFilter : public Filter<RangeFilter> {
 private:
  std::pair<IPv4Address, IPv4Address> ip_range_;

  RangeFilter(const IPv4Address& first, const IPv4Address& last)
      : ip_range_{first, last} {}

 public:
  /**
   * @brief Creates filter from range string
   *
   * @param range range string
   * @return std::optional<RangeFilter>
   */
  static std::optional<RangeFilter> Create(std::string_view range);

  /**
   * @brief Creates filter from two strings
   *
   * @param left lo bound
   * @param right hi bound
   * @return std::optional<RangeFilter>
   */
  static std::optional<RangeFilter> Create(std::string_view left,
                                           std::string_view right);

  /**
   * @brief Check if ip matches filter
   *
   * @param ip ip to check
   * @return true if IP matches filter, false otherwise
   */
  bool MatchesImpl(const IPv4Address& ip) const noexcept {
    const auto& [left, right] = ip_range_;
    return ip == std::clamp(ip, left, right);
  }
};

/**
 * @brief Creates 32bit network mask from prefix
 *
 * @param prefix subnet prefix
 * @return constexpr uint32_t
 */
inline constexpr uint32_t Create32BitMask(uint32_t prefix);

/**
 * @brief Check if range [left, right] is valid
 *
 * @param left lo
 * @param right hi
 * @return true
 * @return false
 */
inline constexpr bool IsValidRange(uint32_t left, uint32_t right);

/**
 * @brief Check if prefix is valid
 * 
 * @param prefix CIDR prefix to check
 * @return true if prefix is valid (is in range [0, 32]), false otherwise
 */
inline constexpr bool IsValidCIDRPrefix(int prefix);

/**
 * @brief Class to compose and store all filters
 *
 */
class CompositeFilter {
  using VarFilter = std::variant<SubnetFilter, RangeFilter>;  ///< fixed filters

 private:
  std::vector<VarFilter> filters_;

 public:
  static constexpr std::size_t kMaxFilters = 20;

  /**
   * @brief Adds filter
   *
   * @tparam T Filter
   * @param filter filter to add
   * @throws std::length_error if exceeded kMaxFilters_
   */
  template <FilterType T>
  void Add(T&& filter);

  /**
   * @brief Check if ip matches all stored filters
   *
   * @param ip ip to check
   * @return true if IP matches all filters, false otherwise
   */
  bool Matches(const IPv4Address& ip) const;

  /**
   * @brief Get filters count
   *
   * @return constexpr std::size_t
   */
  constexpr std::size_t filter_count() const noexcept {
    return filters_.size();
  }
};
}  // namespace net::details

#include "details/filter.tpp"