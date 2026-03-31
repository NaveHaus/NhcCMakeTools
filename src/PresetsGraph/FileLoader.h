/// @file FileLoader.h
/// @brief Defines an abstraction for loading preset files.

#pragma once

#include <string>

namespace nhc::preset_graph {

/// Represents the result of a file load operation.
struct FileLoadResult
{
  bool Success = false;
  bool FileDoesNotExist = false;
  std::string Contents;
};

/// Abstraction for loading file contents.
class FileLoader
{
  public:
  virtual ~FileLoader() = default;

  /// Loads the file at the provided path.
  virtual FileLoadResult LoadFile(const std::string& path) const = 0;
};

}  // namespace nhc::preset_graph
