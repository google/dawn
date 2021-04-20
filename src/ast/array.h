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

#ifndef SRC_AST_ARRAY_H_
#define SRC_AST_ARRAY_H_

#include <string>

#include "src/ast/decoration.h"
#include "src/ast/type.h"

namespace tint {
namespace ast {

/// An array type. If size is zero then it is a runtime array.
class Array : public Castable<Array, Type> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source of this node
  /// @param subtype the type of the array elements
  /// @param size the number of elements in the array. `0` represents a
  /// runtime-sized array.
  /// @param decorations the array decorations
  Array(ProgramID program_id,
        const Source& source,
        Type* subtype,
        uint32_t size,
        ast::DecorationList decorations);
  /// Move constructor
  Array(Array&&);
  ~Array() override;

  /// @returns true if this is a runtime array.
  /// i.e. the size is determined at runtime
  bool IsRuntimeArray() const { return size_ == 0; }

  /// @returns the array decorations
  const ast::DecorationList& decorations() const { return decos_; }

  /// @returns the array type
  Type* type() const { return subtype_; }
  /// @returns the array size. Size is 0 for a runtime array
  uint32_t size() const { return size_; }

  /// @returns the name for the type
  std::string type_name() const override;

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  std::string FriendlyName(const SymbolTable& symbols) const override;

  /// Clones this type and all transitive types using the `CloneContext` `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned type
  Array* Clone(CloneContext* ctx) const override;

 private:
  Type* const subtype_;
  uint32_t const size_;
  ast::DecorationList const decos_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_ARRAY_H_
