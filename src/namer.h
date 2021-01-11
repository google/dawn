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

#include "src/ast/module.h"

namespace tint {

/// Base class for the namers.
class Namer {
 public:
  /// Constructor
  /// @param mod the module this namer works with
  explicit Namer(ast::Module* mod);
  /// Destructor
  virtual ~Namer();

  /// Returns the name for `sym`
  /// @param sym the symbol to retrieve the name for
  /// @returns the sanitized version of `name` or "" if not found
  virtual std::string NameFor(const Symbol& sym) = 0;

  /// Generates a unique name for `prefix`
  /// @param prefix the prefix name
  /// @returns the unique name string
  std::string GenerateName(const std::string& prefix);

  /// Resets the namer, removing all known symbols.
  void Reset();

 protected:
  /// Checks if `name` has been used
  /// @param name the name to check
  /// @returns true if `name` has already been used
  bool IsUsed(const std::string& name);

  /// The module storing the symbol table
  ast::Module* module_ = nullptr;

 private:
  // The list of names taken by the remapper
  std::unordered_set<std::string> used_;
};

/// A namer class which mangles the name
class MangleNamer : public Namer {
 public:
  /// Constructor
  /// @param mod the module to retrieve names from
  explicit MangleNamer(ast::Module* mod);
  /// Destructor
  ~MangleNamer() override;

  /// Returns a mangled name for `sym`
  /// @param sym the symbol to name
  /// @returns the name for `sym` or "" if not found
  std::string NameFor(const Symbol& sym) override;
};

/// A namer which returns the user provided name. This is unsafe in general as
/// it passes user provided data through to the backend compiler. It is useful
/// for development and debugging.
class UnsafeNamer : public Namer {
 public:
  /// Constructor
  /// @param mod the module to retrieve names from
  explicit UnsafeNamer(ast::Module* mod);
  /// Destructor
  ~UnsafeNamer() override;

  /// Returns `name`
  /// @param sym the symbol
  /// @returns `name` or "" if not found
  std::string NameFor(const Symbol& sym) override;
};

}  // namespace tint

#endif  // SRC_NAMER_H_
