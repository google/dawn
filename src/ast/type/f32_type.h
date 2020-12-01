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

#ifndef SRC_AST_TYPE_F32_TYPE_H_
#define SRC_AST_TYPE_F32_TYPE_H_

#include <string>

#include "src/ast/type/type.h"

namespace tint {
namespace ast {
namespace type {

/// A float 32 type
class F32 : public Castable<F32, Type> {
 public:
  /// Constructor
  F32();
  /// Move constructor
  F32(F32&&);
  ~F32() override;

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
  F32* Clone(CloneContext* ctx) const override;
};

}  // namespace type
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_F32_TYPE_H_
