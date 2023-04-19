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

#ifndef SRC_TINT_AST_WORKGROUP_ATTRIBUTE_H_
#define SRC_TINT_AST_WORKGROUP_ATTRIBUTE_H_

#include <array>
#include <string>

#include "src/tint/ast/attribute.h"

// Forward declarations
namespace tint::ast {
class Expression;
}  // namespace tint::ast

namespace tint::ast {

/// A workgroup attribute
class WorkgroupAttribute final : public utils::Castable<WorkgroupAttribute, Attribute> {
  public:
    /// constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param x the workgroup x dimension expression
    /// @param y the optional workgroup y dimension expression
    /// @param z the optional workgroup z dimension expression
    WorkgroupAttribute(ProgramID pid,
                       NodeID nid,
                       const Source& src,
                       const Expression* x,
                       const Expression* y = nullptr,
                       const Expression* z = nullptr);

    ~WorkgroupAttribute() override;

    /// @returns the workgroup dimensions
    std::array<const Expression*, 3> Values() const { return {x, y, z}; }

    /// @returns the WGSL name for the attribute
    std::string Name() const override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const WorkgroupAttribute* Clone(CloneContext* ctx) const override;

    /// The workgroup x dimension.
    const Expression* const x;
    /// The optional workgroup y dimension. May be null.
    const Expression* const y = nullptr;
    /// The optional workgroup z dimension. May be null.
    const Expression* const z = nullptr;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_WORKGROUP_ATTRIBUTE_H_
