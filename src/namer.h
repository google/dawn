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

#ifndef SRC_NAMER_H_
#define SRC_NAMER_H_

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace tint {

/// Base class for the namers.
class Namer {
 public:
  /// Constructor
  Namer();
  virtual ~Namer();

  /// Returns a sanitized version of |name|
  /// @param name the name to sanitize
  /// @returns the sanitized version of |name|
  virtual std::string NameFor(const std::string& name) = 0;

  /// Returns if the given name has been mapped already
  /// @param name the name to check
  /// @returns true if the name has been mapped
  bool IsMapped(const std::string& name);

 protected:
  /// Map of original name to new name.
  std::unordered_map<std::string, std::string> name_map_;
};

/// A namer class which hashes the name
class HashingNamer : public Namer {
 public:
  HashingNamer();
  ~HashingNamer() override;

  /// Returns a sanitized version of |name|
  /// @param name the name to sanitize
  /// @returns the sanitized version of |name|
  std::string NameFor(const std::string& name) override;
};

/// A namer which just returns the provided string
class NoopNamer : public Namer {
 public:
  NoopNamer();
  ~NoopNamer() override;

  /// Returns |name|
  /// @param name the name
  /// @returns |name|
  std::string NameFor(const std::string& name) override;
};

}  // namespace tint

#endif  // SRC_NAMER_H_
