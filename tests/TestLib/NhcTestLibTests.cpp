/// @file NhcTestLibTests.cpp
/// @brief Smoke test to verify NhcTestLib provides proper Catch2 integration.

#include <catch2/catch_test_macros.hpp>

SCENARIO("NhcTestLib provides a working test harness")
{
  GIVEN("A basic test case")
  {
    int value = 42;

    THEN("The test harness is functional")
    {
      REQUIRE(value == 42);
    }

    WHEN("a simple calculation is performed")
    {
      value = value * 2;

      THEN("the result is correct")
      {
        REQUIRE(value == 84);
      }
    }
  }
}

TEST_CASE("Basic assertions work", "[smoke]")
{
  REQUIRE(true);
  REQUIRE(1 + 1 == 2);
  REQUIRE_FALSE(false);
}
