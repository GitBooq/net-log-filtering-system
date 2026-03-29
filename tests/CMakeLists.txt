add_executable(tests_all
    test_ipaddr.cpp
    test_filter.cpp
    test_streamproc.cpp
)

target_link_libraries(tests_all
    ipaddr
    filter
    GTest::gtest_main
)

target_compile_options(tests_all PRIVATE
    -O0
    -g
)

include(GoogleTest)
gtest_discover_tests(tests_all)
