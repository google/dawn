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

#ifndef SRC_TINT_AST_FUNCTION_H_
#define SRC_TINT_AST_FUNCTION_H_

#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "src/tint/ast/attribute.h"
#include "src/tint/ast/binding_attribute.h"
#include "src/tint/ast/block_statement.h"
#include "src/tint/ast/builtin_attribute.h"
#include "src/tint/ast/group_attribute.h"
#include "src/tint/ast/location_attribute.h"
#include "src/tint/ast/parameter.h"
#include "src/tint/ast/pipeline_stage.h"

// Forward declarations
namespace tint::ast {
class Identifier;
class IdentifierExpression;
}  // namespace tint::ast

namespace tint::ast {

/// A Function statement.
class Function final : public utils::Castable<Function, Node> {
  public:
    /// Create a function
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param source the variable source
    /// @param name the function name
    /// @param params the function parameters
    /// @param return_type the return type
    /// @param body the function body
    /// @param attributes the function attributes
    /// @param return_type_attributes the return type attributes
    Function(ProgramID pid,
             NodeID nid,
             const Source& source,
             const Identifier* name,
             utils::VectorRef<const Parameter*> params,
             Type return_type,
             const BlockStatement* body,
             utils::VectorRef<const Attribute*> attributes,
             utils::VectorRef<const Attribute*> return_type_attributes);

    /// Destructor
    ~Function() override;

    /// @returns the functions pipeline stage or None if not set
    ast::PipelineStage PipelineStage() const;

    /// @returns true if this function is an entry point
    bool IsEntryPoint() const { return PipelineStage() != ast::PipelineStage::kNone; }

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const Function* Clone(CloneContext* ctx) const override;

    /// The function name
    const Identifier* const name;

    /// The function params
    const utils::Vector<const Parameter*, 8> params;

    /// The function return type
    const Type return_type;

    /// The function body
    const BlockStatement* const body;

    /// The attributes attached to this function
    const utils::Vector<const Attribute*, 2> attributes;

    /// The attributes attached to the function return type.
    const utils::Vector<const Attribute*, 2> return_type_attributes;
};

/// A list of functions
class FunctionList : public utils::Vector<const Function*, 8> {
  public:
    /// Appends f to the end of the list
    /// @param f the function to append to this list
    void Add(const Function* f) { this->Push(f); }

    /// Returns the function with the given name
    /// @param sym the function symbol to search for
    /// @returns the associated function or nullptr if none exists
    const Function* Find(Symbol sym) const;

    /// Returns the function with the given name
    /// @param sym the function symbol to search for
    /// @param stage the pipeline stage
    /// @returns the associated function or nullptr if none exists
    const Function* Find(Symbol sym, ast::PipelineStage stage) const;

    /// @param stage the pipeline stage
    /// @returns true if the Builder contains an entrypoint function with
    /// the given stage
    bool HasStage(ast::PipelineStage stage) const;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_FUNCTION_H_
