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

#ifndef SRC_TYPE_STRUCT_TYPE_H_
#define SRC_TYPE_STRUCT_TYPE_H_

#include <string>

#include "src/ast/struct.h"
#include "src/type/type.h"

namespace tint {
namespace type {

/// A structure type
class Struct : public Castable<Struct, Type> {
 public:
  /// Constructor
  /// @param sym the symbol representing the struct
  /// @param impl the struct data
  Struct(const Symbol& sym, ast::Struct* impl);
  /// Move constructor
  Struct(Struct&&);
  ~Struct() override;

  /// @returns the struct symbol
  const Symbol& symbol() const { return symbol_; }

  /// @returns true if the struct has a block decoration
  bool IsBlockDecorated() const { return struct_->IsBlockDecorated(); }

  /// @returns the struct
  ast::Struct* impl() const { return struct_; }

  /// @returns the name for the type
  std::string type_name() const override;

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  std::string FriendlyName(const SymbolTable& symbols) const override;

  /// Clones this type and all transitive types using the `CloneContext` `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned type
  Struct* Clone(CloneContext* ctx) const override;

 private:
  Symbol const symbol_;
  ast::Struct* const struct_;

  uint64_t LargestMemberBaseAlignment(MemoryLayout mem_layout) const;
};

}  // namespace type
}  // namespace tint

#endif  // SRC_TYPE_STRUCT_TYPE_H_
