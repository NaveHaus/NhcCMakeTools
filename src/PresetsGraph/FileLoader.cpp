/// @file FileLoader.cpp
/// @brief Implements filesystem-backed preset file loading.

#include "FileLoader.h"

#include <filesystem>
#include <fstream>
#include <iterator>

namespace nhc::preset_graph {

FileLoadResult
FilesystemFileLoader::LoadFile(const std::string& path) const
{
  auto stream = std::ifstream{path, std::ios::binary};
  if(!stream) {
    return FileLoadResult{
      .Success = false,
      .FileDoesNotExist = !std::filesystem::exists(path),
    };
  }

  return FileLoadResult{
    .Success = true,
    .Contents = std::string{std::istreambuf_iterator<char>{stream},
      std::istreambuf_iterator<char>{}},
  };
}

}  // namespace nhc::preset_graph
