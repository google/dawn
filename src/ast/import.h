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

#ifndef SRC_AST_IMPORT_H_
#define SRC_AST_IMPORT_H_

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "src/ast/node.h"

namespace tint {
namespace ast {

/// An import statement.
class Import : public Node {
 public:
  /// Create a new empty import statement
  Import() = default;
  /// Create a new import statement
  /// @param path The import path e.g. GLSL.std.430
  /// @param name The import reference name e.g. std::
  Import(const std::string& path, const std::string& name);
  /// Create a new import statement
  /// @param source The input source for the import statement
  /// @param path The import path e.g. GLSL.std.430
  /// @param name The import reference name e.g. std::
  Import(const Source& source,
         const std::string& path,
         const std::string& name);
  /// Move constructor
  Import(Import&&) = default;

  ~Import() override;

  /// Sets the import path
  /// @param path the path to set
  void set_path(const std::string& path) { path_ = path; }
  /// @returns the import path
  const std::string& path() const { return path_; }
  /// Sets the import name
  /// @param name the name to set
  void set_name(const std::string& name) { name_ = name; }
  /// @returns the import name
  const std::string& name() const { return name_; }

  /// @returns true if the name and path are both present
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  Import(const Import&) = delete;

  std::string path_;
  std::string name_;
};

/// A list of unique imports
using ImportList = std::vector<std::unique_ptr<Import>>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_IMPORT_H_
