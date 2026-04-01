#include <sstream>
#include <string>

#include "details/streamproc.hpp"
#include "gtest/gtest.h"

using namespace net::details;

auto npos = std::string_view::npos;

std::string make_log_line(const std::string& ip, const std::string& message) {
  return ip + " - " + message;
}

TEST(StreamProcessorTest, EmptyInput) {
  std::stringstream input;
  std::stringstream output;

  auto filter = SubnetFilter::create("192.168.1.0/24").value();
  StreamProcessor processor;

  processor.process(input, output, filter);

  EXPECT_TRUE(output.str().empty());
}

TEST(StreamProcessorTest, SingleLineMatch) {
  std::stringstream input;
  input << make_log_line("192.168.1.1", "GET /index.html") << "\n";

  std::stringstream output;

  auto filter = SubnetFilter::create("192.168.1.0/24").value();
  StreamProcessor processor;

  processor.process(input, output, filter);

  EXPECT_EQ(output.str(), "192.168.1.1 - GET /index.html\n");
}

TEST(StreamProcessorTest, SingleLineNoMatch) {
  std::stringstream input;
  input << make_log_line("10.0.0.1", "GET /index.html") << "\n";

  std::stringstream output;

  auto filter = SubnetFilter::create("192.168.1.0/24").value();
  StreamProcessor processor;

  processor.process(input, output, filter);

  EXPECT_TRUE(output.str().empty());
}

TEST(StreamProcessorTest, MultipleLinesMixed) {
  std::stringstream input;
  input << make_log_line("192.168.1.1", "GET /page1") << "\n";
  input << make_log_line("10.0.0.1", "POST /login") << "\n";
  input << make_log_line("192.168.1.100", "GET /page2") << "\n";
  input << make_log_line("invalid.ip", "SKIP") << "\n";
  input << make_log_line("192.168.1.50", "GET /page3") << "\n";

  std::stringstream output;

  auto filter = SubnetFilter::create("192.168.1.0/24").value();
  StreamProcessor processor;

  processor.process(input, output, filter);

  auto result = output.view();
  EXPECT_NE(result.find("192.168.1.1 - GET /page1"), npos);
  EXPECT_EQ(result.find("10.0.0.1 - POST /login"), npos);
  EXPECT_NE(result.find("192.168.1.100 - GET /page2"), npos);
  EXPECT_EQ(result.find("invalid.ip - SKIP"), npos);
  EXPECT_NE(result.find("192.168.1.50 - GET /page3"), npos);
}

TEST(StreamProcessorTest, BatchOutputExact) {
  std::stringstream input;
  const int BATCH_SIZE = 5;

  for (int i = 0; i < BATCH_SIZE; ++i) {
    input << make_log_line("192.168.1.1", "MESSAGE " + std::to_string(i))
          << "\n";
  }

  std::stringstream output;

  auto filter = SubnetFilter::create("192.168.1.0/24").value();
  StreamProcessor processor(BATCH_SIZE);

  processor.process(input, output, filter);

  auto view = output.view();
  int lines = std::count(view.begin(), view.end(), '\n');

  EXPECT_EQ(lines, BATCH_SIZE);
}

TEST(StreamProcessorTest, BatchOutputPartial) {
  std::stringstream input;
  const int BATCH_SIZE = 1000;
  const int PARTIAL = 342;

  for (int i = 0; i < PARTIAL; ++i) {
    input << make_log_line("192.168.1.1", "MESSAGE " + std::to_string(i))
          << "\n";
  }

  std::stringstream output;

  auto filter = SubnetFilter::create("192.168.1.0/24").value();
  StreamProcessor processor(BATCH_SIZE);

  processor.process(input, output, filter);

  auto view = output.view();
  int lines = std::count(view.begin(), view.end(), '\n');
  EXPECT_EQ(lines, PARTIAL);
}

TEST(StreamProcessorTest, BatchOutputMultipleBatches) {
  std::stringstream input;
  const int TOTAL = 2500;
  const int BATCH_SIZE = 1000;

  for (int i = 0; i < TOTAL; ++i) {
    input << make_log_line("192.168.1.1", "MESSAGE " + std::to_string(i))
          << "\n";
  }

  std::stringstream output;

  auto filter = SubnetFilter::create("192.168.1.0/24").value();
  StreamProcessor processor(BATCH_SIZE);

  processor.process(input, output, filter);

  auto view = output.view();
  int lines = std::count(view.begin(), view.end(), '\n');
  EXPECT_EQ(lines, TOTAL);
}

TEST(StreamProcessorTest, WithRangeFilter) {
  std::stringstream input;
  input << make_log_line("10.0.0.1", "LOWER_BOUND") << "\n";
  input << make_log_line("10.0.0.50", "MIDDLE") << "\n";
  input << make_log_line("10.0.0.100", "UPPER_BOUND") << "\n";
  input << make_log_line("10.0.0.101", "OUT_OF_RANGE") << "\n";
  input << make_log_line("192.168.1.1", "DIFFERENT_SUBNET") << "\n";

  std::stringstream output;

  auto filter = RangeFilter::create("10.0.0.1", "10.0.0.100").value();
  StreamProcessor processor;

  processor.process(input, output, filter);

  auto result = output.view();
  EXPECT_NE(result.find("10.0.0.1 - LOWER_BOUND"), npos);
  EXPECT_NE(result.find("10.0.0.50 - MIDDLE"), npos);
  EXPECT_NE(result.find("10.0.0.100 - UPPER_BOUND"), npos);
  EXPECT_EQ(result.find("10.0.0.101 - OUT_OF_RANGE"), npos);
  EXPECT_EQ(result.find("192.168.1.1 - DIFFERENT_SUBNET"), npos);
}

TEST(StreamProcessorTest, WithCompositeFilter) {
  std::stringstream input;
  input << make_log_line("192.168.1.15", "IN_BOTH") << "\n";
  input << make_log_line("192.168.1.5", "IN_SUBNET_OUT_RANGE") << "\n";
  input << make_log_line("10.0.0.50", "IN_RANGE_OUT_SUBNET") << "\n";
  input << make_log_line("192.168.2.1", "OUT_BOTH") << "\n";

  std::stringstream output;

  auto subnet = SubnetFilter::create("192.168.1.0/24").value();
  auto range = RangeFilter::create("192.168.1.10", "192.168.1.20").value();

  CompositeFilter composite;
  composite.add(std::move(subnet));
  composite.add(std::move(range));

  StreamProcessor processor;
  processor.process(input, output, composite);

  auto result = output.view();
  EXPECT_NE(result.find("192.168.1.15 - IN_BOTH"), npos);
  EXPECT_NE(result.find("192.168.1.5 - IN_SUBNET"), npos);
  EXPECT_EQ(result.find("10.0.0.50 - OUT_BOTH"), npos);
  EXPECT_EQ(result.find("192.168.2.1 - OUT_BOTH"), npos);
}

TEST(StreamProcessorTest, InvalidIpLinesIgnored) {
  std::stringstream input;
  input << make_log_line("192.168.1.1", "VALID") << "\n";
  input << "invalid.ip - CORRUPTED\n";
  input << "192.168.1.256 - INVALID_OCTET\n";
  input << "192.168.1 - INCOMPLETE\n";
  input << " - EMPTY_LINE\n";
  input << make_log_line("192.168.1.100", "VALID2") << "\n";

  std::stringstream output;

  auto filter = SubnetFilter::create("192.168.1.0/24").value();
  StreamProcessor processor;

  processor.process(input, output, filter);

  auto result = output.view();
  EXPECT_NE(result.find("192.168.1.1 - VALID"), npos);
  EXPECT_NE(result.find("192.168.1.100 - VALID2"), npos);
  EXPECT_EQ(result.find("invalid.ip - CORRUPTED"), npos);
  EXPECT_EQ(result.find("192.168.1.256 - INVALID_OCTET"), npos);
  EXPECT_EQ(result.find("192.168.1 - INCOMPLETE"), npos);
  EXPECT_EQ(result.find(" - EMPTY_LINE"), npos);
}

TEST(StreamProcessorTest, EdgeCaseZeroIP) {
  std::stringstream input;
  input << make_log_line("0.0.0.0", "DEFAULT_ROUTE") << "\n";
  input << make_log_line("255.255.255.255", "BROADCAST") << "\n";

  std::stringstream output;

  auto filter = RangeFilter::create("0.0.0.0", "255.255.255.255").value();
  StreamProcessor processor;

  processor.process(input, output, filter);

  auto result = output.view();
  EXPECT_NE(result.find("0.0.0.0 - DEFAULT_ROUTE"), npos);
  EXPECT_NE(result.find("255.255.255.255 - BROADCAST"), npos);
}

TEST(StreamProcessorTest, EdgeCaseNetworkAndBroadcast) {
  std::stringstream input;
  input << make_log_line("192.168.0.0", "NETWORK_ADDR") << "\n";
  input << make_log_line("192.168.0.255", "BROADCAST_ADDR") << "\n";

  std::stringstream output;

  auto filter = SubnetFilter::create("192.168.0.0/24").value();
  StreamProcessor processor;

  processor.process(input, output, filter);

  auto result = output.view();
  EXPECT_NE(result.find("192.168.0.0 - NETWORK_ADDR"), npos);
  EXPECT_NE(result.find("192.168.0.255 - BROADCAST_ADDR"), npos);
}

TEST(StreamProcessorTest, LargeInputNoMemoryLeak) {
  std::stringstream input;
  const int LARGE_COUNT = 100000;

  for (int i = 0; i < LARGE_COUNT; ++i) {
    input << make_log_line("192.168.1.1", "MESSAGE") << "\n";
  }

  std::stringstream output;

  auto filter = SubnetFilter::create("192.168.1.0/24").value();
  StreamProcessor processor(1000);

  processor.process(input, output, filter);

  auto view = output.view();
  int lines = std::count(view.begin(), view.end(), '\n');
  EXPECT_EQ(lines, LARGE_COUNT);
}