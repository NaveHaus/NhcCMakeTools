/// @file TestMain.cpp
/// @brief Provides the main entry point for all test executables in this repository.

#include <catch2/catch_session.hpp>

/// Main entry point for test executables.
///
/// Initializes and runs the Catch2 test session with command-line arguments.
///
/// @param argc The number of command-line arguments
/// @param argv The command-line argument values
/// @return Exit code: 0 on success, non-zero on failure
int
main(int argc, char* argv[])
{
  return Catch::Session().run(argc, argv);
}
