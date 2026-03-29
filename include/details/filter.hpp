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

template <typename Derived>
struct Filter {
  bool matches(const IPv4Address& ip) const {
    return static_cast<const Derived*>(this)->matches_impl(ip);
  }

 protected:
  ~Filter() = default;
};

template <typename T>
concept FilterType = requires(const T& t, const IPv4Address& ip) {
  { t.matches(ip) } -> std::same_as<bool>;
};

class SubnetFilter : public Filter<SubnetFilter> {
 private:
  uint32_t network_;
  uint32_t mask_;

  SubnetFilter(uint32_t network, uint32_t mask) noexcept
      : network_(network), mask_(mask) {}

 public:
  static std::optional<SubnetFilter> create(std::string_view cidr);

  bool matches_impl(const IPv4Address& ip) const noexcept {
    return (ip.to_uint32() & mask_) == network_;
  }
};

class RangeFilter : public Filter<RangeFilter> {
 private:
  std::pair<IPv4Address, IPv4Address> ip_range;

  RangeFilter(const IPv4Address& first, const IPv4Address& last)
      : ip_range{first, last} {}

 public:
  static std::optional<RangeFilter> create(std::string_view range);
  static std::optional<RangeFilter> create(std::string_view left,
                                           std::string_view right);
  bool matches_impl(const IPv4Address& ip) const noexcept {
    const auto& [left, right] = ip_range;
    return ip == std::clamp(ip, left, right);
  }
};

constexpr uint32_t create32BitMask(uint32_t prefix);
constexpr bool isValidRange(uint32_t left, uint32_t right);

class CompositeFilter {
  using VarFilter = std::variant<SubnetFilter, RangeFilter>;

 private:
  std::vector<VarFilter> filters_;

 public:
  template <FilterType T>
  void add(T&& filter) {
    filters_.push_back(std::forward<T>(filter));
  }

  bool matches(const IPv4Address& ip) const;
};
}  // namespace net::details

#include "details/filter.tpp"