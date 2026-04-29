/// @file FileLoaderTests.cpp
/// @brief Tests for filesystem preset loading behavior.

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

#include "FileLoader.h"

namespace {

using nhc::preset_graph::FilesystemFileLoader;

}  // namespace

SCENARIO("FilesystemFileLoader loads a file by absolute path")
{
  GIVEN("A readable file")
  {
    const auto path =
      std::filesystem::temp_directory_path() / "nhc-file-loader-test.json";
    {
      auto stream = std::ofstream{path};
      stream << R"({"version":10})";
    }

    WHEN("The file is loaded by absolute path")
    {
      const auto result = FilesystemFileLoader{}.LoadFile(path.string());

      THEN("The complete contents are returned")
      {
        REQUIRE(result.Success);
        REQUIRE(result.Contents == R"({"version":10})");
      }
    }

    std::filesystem::remove(path);
  }
}

SCENARIO("FilesystemFileLoader reports missing files")
{
  GIVEN("A path that does not exist")
  {
    const auto path =
      std::filesystem::temp_directory_path() / "nhc-file-loader-missing.json";
    std::filesystem::remove(path);

    WHEN("The file is loaded")
    {
      const auto result = FilesystemFileLoader{}.LoadFile(path.string());

      THEN("The failure reports that the file does not exist")
      {
        REQUIRE_FALSE(result.Success);
        REQUIRE(result.FileDoesNotExist);
      }
    }
  }
}
