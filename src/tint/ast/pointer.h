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

#ifndef SRC_TINT_AST_POINTER_H_
#define SRC_TINT_AST_POINTER_H_

#include <string>

#include "src/tint/ast/access.h"
#include "src/tint/ast/storage_class.h"
#include "src/tint/ast/type.h"

namespace tint::ast {

/// A pointer type.
class Pointer final : public Castable<Pointer, Type> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param src the source of this node
    /// @param subtype the pointee type
    /// @param storage_class the storage class of the pointer
    /// @param access the access control of the pointer
    Pointer(ProgramID pid,
            const Source& src,
            const Type* const subtype,
            ast::StorageClass storage_class,
            ast::Access access);
    /// Move constructor
    Pointer(Pointer&&);
    ~Pointer() override;

    /// @param symbols the program's symbol table
    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName(const SymbolTable& symbols) const override;

    /// Clones this type and all transitive types using the `CloneContext` `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned type
    const Pointer* Clone(CloneContext* ctx) const override;

    /// The pointee type
    const Type* const type;

    /// The storage class of the pointer
    ast::StorageClass const storage_class;

    /// The access control of the pointer
    ast::Access const access;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_POINTER_H_
