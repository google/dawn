// Copyright 2020 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_WRITER_HLSL_NAMER_H_
#define SRC_WRITER_HLSL_NAMER_H_

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace tint {
namespace writer {
namespace hlsl {

/// Remaps maps names to avoid reserved words and collisions for HLSL.
class Namer {
 public:
  /// Constructor
  Namer();
  ~Namer();

  /// Returns a sanitized version of |name|
  /// @param name the name to sanitize
  /// @returns the sanitized version of |name|
  std::string NameFor(const std::string& name);

  /// Registers a remapped name.
  /// @param name the name to register
  void RegisterRemappedName(const std::string& name);

  /// Returns if the given name has been mapped alread
  /// @param name the name to check
  /// @returns true if the name has been mapped
  bool IsMapped(const std::string& name);

 private:
  /// Map of original name to new name. The two names may be the same.
  std::unordered_map<std::string, std::string> name_map_;
  // The list of names taken by the remapper
  std::unordered_set<std::string> remapped_names_;
};

}  // namespace hlsl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_HLSL_NAMER_H_
