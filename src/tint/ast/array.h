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

#ifndef SRC_TINT_AST_ARRAY_H_
#define SRC_TINT_AST_ARRAY_H_

#include <string>

#include "src/tint/ast/attribute.h"
#include "src/tint/ast/type.h"

// Forward declarations
namespace tint::ast {
class Expression;
}  // namespace tint::ast

namespace tint::ast {

/// An array type. If size is zero then it is a runtime array.
class Array final : public Castable<Array, Type> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param src the source of this node
    /// @param subtype the type of the array elements
    /// @param count the number of elements in the array. nullptr represents a
    /// runtime-sized array.
    /// @param attributes the array attributes
    Array(ProgramID pid,
          const Source& src,
          const Type* subtype,
          const Expression* count,
          AttributeList attributes);
    /// Move constructor
    Array(Array&&);
    ~Array() override;

    /// @returns true if this is a runtime array.
    /// i.e. the size is determined at runtime
    bool IsRuntimeArray() const { return count == nullptr; }

    /// @param symbols the program's symbol table
    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName(const SymbolTable& symbols) const override;

    /// Clones this type and all transitive types using the `CloneContext` `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned type
    const Array* Clone(CloneContext* ctx) const override;

    /// the array element type
    const Type* const type;

    /// the array size in elements, or nullptr for a runtime array
    const Expression* const count;

    /// the array attributes
    const AttributeList attributes;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_ARRAY_H_
