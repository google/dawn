// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#ifndef SRC_TINT_SEM_STRUCT_H_
#define SRC_TINT_SEM_STRUCT_H_

#include <optional>

#include "src/tint/ast/struct.h"
#include "src/tint/builtin/address_space.h"
#include "src/tint/symbol.h"
#include "src/tint/type/struct.h"
#include "src/tint/type/type.h"
#include "src/tint/utils/vector.h"

// Forward declarations
namespace tint::ast {
class StructMember;
}  // namespace tint::ast
namespace tint::sem {
class StructMember;
}  // namespace tint::sem
namespace tint::type {
class StructMember;
}  // namespace tint::type

namespace tint::sem {

/// Struct holds the semantic information for structures.
/// Unlike type::Struct, sem::Struct has an AST declaration node.
class Struct final : public utils::Castable<Struct, type::Struct> {
  public:
    /// Constructor
    /// @param declaration the AST structure declaration
    /// @param name the name of the structure
    /// @param members the structure members
    /// @param align the byte alignment of the structure
    /// @param size the byte size of the structure
    /// @param size_no_padding size of the members without the end of structure alignment padding
    Struct(const ast::Struct* declaration,
           Symbol name,
           utils::VectorRef<const StructMember*> members,
           uint32_t align,
           uint32_t size,
           uint32_t size_no_padding);

    /// Destructor
    ~Struct() override;

    /// @returns the struct
    const ast::Struct* Declaration() const { return declaration_; }

    /// @returns the members of the structure
    utils::VectorRef<const StructMember*> Members() const {
        return Base::Members().ReinterpretCast<const StructMember*>();
    }

  private:
    ast::Struct const* const declaration_;
};

/// StructMember holds the semantic information for structure members.
/// Unlike type::StructMember, sem::StructMember has an AST declaration node.
class StructMember final : public utils::Castable<StructMember, type::StructMember> {
  public:
    /// Constructor
    /// @param declaration the AST declaration node
    /// @param name the name of the structure member
    /// @param type the type of the member
    /// @param index the index of the member in the structure
    /// @param offset the byte offset from the base of the structure
    /// @param align the byte alignment of the member
    /// @param size the byte size of the member
    /// @param attributes the optional attributes
    StructMember(const ast::StructMember* declaration,
                 Symbol name,
                 const type::Type* type,
                 uint32_t index,
                 uint32_t offset,
                 uint32_t align,
                 uint32_t size,
                 const type::StructMemberAttributes& attributes);

    /// Destructor
    ~StructMember() override;

    /// @returns the AST declaration node
    const ast::StructMember* Declaration() const { return declaration_; }

    /// @returns the structure that owns this member
    const sem::Struct* Struct() const { return static_cast<const sem::Struct*>(Base::Struct()); }

  private:
    const ast::StructMember* const declaration_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_STRUCT_H_
