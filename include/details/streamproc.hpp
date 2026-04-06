/**
 * @file streamproc.hpp
 * @brief Stream Processor
 */
#pragma once

#include <cstddef>   // for size_t
#include <istream>   // for ostream, istream
#include <optional>  // for optional
#include <string>    // for string
#include <vector>    // for vector

#include "filter.hpp"  // for FilterType
#include "ipaddr.hpp"  // for IPv4Address

namespace net::details {

/**
 * @brief Parse input and output result after filtering.
 * Batch filtering.
 */
class StreamProcessor {
 public:
  /**
   * @brief Construct a new Stream Processor object.
   * Reserves batch_size bytes in output buffer.
   *
   * @param batch_size
   */
  explicit StreamProcessor(size_t batch_size = DEFAULT_BATCH_SIZE)
      : batch_size_(batch_size) {
    buffer_.reserve(batch_size);
  }

  /**
   * @brief Parse, filter, output.
   * Output when buffer is full, or input ends
   * @tparam T filter type
   * @param input input stream (logs)
   * @param output output stream (result)
   * @param filter filter to apply to input stream
   */
  template <FilterType T>
  void Process(std::istream& input, std::ostream& output, const T& filter);

 private:
  /**
   * @brief Input file line structure
   *
   */
  struct LogEntry {
    IPv4Address ip;
    std::string line;
    size_t line_number;
  };

  /**
   * @brief Parse IPAddr from string "IP - message"
   *
   * @param line string to parse
   * @return std::optional<IPv4Address>
   */
  std::optional<IPv4Address> ParseIPFromLine(const std::string& line) const;

  /**
   * @brief Output buffer and clear
   *
   * @param output output stream
   */
  void FlushBuffer(std::ostream& output);

  static constexpr size_t DEFAULT_BATCH_SIZE = 1000u;

  std::vector<LogEntry> buffer_;  ///< buffer for output
  size_t batch_size_;             ///< max buffer size
};

}  // namespace net::details

#include "details/streamproc.tpp"