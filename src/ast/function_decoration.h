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

#ifndef SRC_AST_FUNCTION_DECORATION_H_
#define SRC_AST_FUNCTION_DECORATION_H_

#include <memory>
#include <ostream>
#include <vector>

namespace tint {
namespace ast {

class WorkgroupDecoration;

/// A decoration attached to a function
class FunctionDecoration {
 public:
  virtual ~FunctionDecoration();

  /// @returns true if this is a workgroup decoration
  virtual bool IsWorkgroup() const;

  /// @returns the decoration as a workgroup decoration
  const WorkgroupDecoration* AsWorkgroup() const;

  /// Outputs the function decoration to the given stream
  /// @param out the stream to output too
  virtual void to_str(std::ostream& out) const = 0;

 protected:
  FunctionDecoration();
};

/// A list of unique function decorations
using FunctionDecorationList = std::vector<std::unique_ptr<FunctionDecoration>>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_FUNCTION_DECORATION_H_
