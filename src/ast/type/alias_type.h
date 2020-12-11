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

#ifndef SRC_AST_TYPE_ALIAS_TYPE_H_
#define SRC_AST_TYPE_ALIAS_TYPE_H_

#include <string>

#include "src/ast/type/type.h"
#include "src/symbol.h"

namespace tint {
namespace ast {
namespace type {

/// A type alias type. Holds a name and pointer to another type.
class Alias : public Castable<Alias, Type> {
 public:
  /// Constructor
  /// @param sym the symbol for the alias
  /// @param name the alias name
  /// @param subtype the alias'd type
  Alias(const Symbol& sym, const std::string& name, Type* subtype);
  /// Move constructor
  Alias(Alias&&);
  /// Destructor
  ~Alias() override;

  /// @returns the alias symbol
  Symbol symbol() const { return symbol_; }
  /// @returns the alias name
  const std::string& name() const { return name_; }
  /// @returns the alias type
  Type* type() const { return subtype_; }

  /// @returns the name for this type
  std::string type_name() const override;

  /// @param mem_layout type of memory layout to use in calculation.
  /// @returns minimum size required for this type, in bytes.
  ///          0 for non-host shareable types.
  uint64_t MinBufferBindingSize(MemoryLayout mem_layout) const override;

  /// @param mem_layout type of memory layout to use in calculation.
  /// @returns base alignment for the type, in bytes.
  ///          0 for non-host shareable types.
  uint64_t BaseAlignment(MemoryLayout mem_layout) const override;

  /// Clones this type and all transitive types using the `CloneContext` `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned type
  Alias* Clone(CloneContext* ctx) const override;

 private:
  Symbol symbol_;
  std::string name_;
  Type* subtype_ = nullptr;
};

}  // namespace type
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_ALIAS_TYPE_H_
