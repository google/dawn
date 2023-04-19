// Copyright 2021 The Tint Authors.
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

#ifndef SRC_TINT_SEM_INFO_H_
#define SRC_TINT_SEM_INFO_H_

#include <algorithm>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "src/tint/ast/diagnostic_control.h"
#include "src/tint/ast/node.h"
#include "src/tint/debug.h"
#include "src/tint/sem/node.h"
#include "src/tint/sem/type_mappings.h"
#include "src/tint/utils/unique_vector.h"

// Forward declarations
namespace tint::sem {
class Module;
class ValueExpression;
}  // namespace tint::sem
namespace tint::type {
class Node;
class Type;
}  // namespace tint::type

namespace tint::sem {

/// Info holds all the resolved semantic information for a Program.
class Info {
  public:
    /// Placeholder type used by Get() to provide a default value for EXPLICIT_SEM
    using InferFromAST = std::nullptr_t;

    /// Resolves to the return type of the Get() method given the desired semantic
    /// type and AST type.
    template <typename SEM, typename AST>
    using GetResultType =
        std::conditional_t<std::is_same<SEM, InferFromAST>::value, SemanticNodeTypeFor<AST>, SEM>;

    /// Alias to a unique vector of transitively referenced global variables
    using TransitivelyReferenced = utils::UniqueVector<const GlobalVariable*, 4>;

    /// Constructor
    Info();

    /// Move constructor
    Info(Info&&);

    /// Destructor
    ~Info();

    /// Move assignment operator
    /// @param rhs the Program to move
    /// @return this Program
    Info& operator=(Info&& rhs);

    /// @param highest_node_id the last allocated (numerically highest) AST node identifier.
    void Reserve(ast::NodeID highest_node_id) {
        nodes_.resize(std::max(highest_node_id.value + 1, nodes_.size()));
    }

    /// Get looks up the semantic information for the AST node `ast_node`.
    /// @param ast_node the AST node
    /// @returns a pointer to the semantic node if found, otherwise nullptr
    template <typename SEM = InferFromAST,
              typename AST = utils::CastableBase,
              typename RESULT = GetResultType<SEM, AST>>
    const RESULT* Get(const AST* ast_node) const {
        static_assert(std::is_same_v<SEM, InferFromAST> ||
                          !utils::traits::IsTypeOrDerived<SemanticNodeTypeFor<AST>, SEM>,
                      "explicit template argument is unnecessary");
        if (ast_node && ast_node->node_id.value < nodes_.size()) {
            return As<RESULT>(nodes_[ast_node->node_id.value]);
        }
        return nullptr;
    }

    /// Convenience function that's an alias for Get<ValueExpression>()
    /// @param ast_node the AST node
    /// @returns a pointer to the semantic node if found, otherwise nullptr
    template <typename AST>
    const sem::ValueExpression* GetVal(const AST* ast_node) const {
        return Get<ValueExpression>(ast_node);
    }

    /// Add registers the semantic node `sem_node` for the AST node `ast_node`.
    /// @param ast_node the AST node
    /// @param sem_node the semantic node
    template <typename AST>
    void Add(const AST* ast_node, const SemanticNodeTypeFor<AST>* sem_node) {
        Reserve(ast_node->node_id);
        // Check there's no semantic info already existing for the AST node
        TINT_ASSERT(Semantic, nodes_[ast_node->node_id.value] == nullptr);
        nodes_[ast_node->node_id.value] = sem_node;
    }

    /// Replace replaces any existing semantic node `sem_node` for the AST node `ast_node`.
    /// @param ast_node the AST node
    /// @param sem_node the new semantic node
    template <typename AST>
    void Replace(const AST* ast_node, const SemanticNodeTypeFor<AST>* sem_node) {
        Reserve(ast_node->node_id);
        nodes_[ast_node->node_id.value] = sem_node;
    }

    /// Wrap returns a new Info created with the contents of `inner`.
    /// The Info returned by Wrap is intended to temporarily extend the contents
    /// of an existing immutable Info.
    /// As the copied contents are owned by `inner`, `inner` must not be
    /// destructed or assigned while using the returned Info.
    /// @param inner the immutable Info to extend
    /// @return the Info that wraps `inner`
    static Info Wrap(const Info& inner) {
        Info out;
        out.nodes_ = inner.nodes_;
        out.module_ = inner.module_;
        return out;
    }

    /// Assigns the semantic module.
    /// @param module the module to assign.
    void SetModule(sem::Module* module) { module_ = module; }

    /// @returns the semantic module.
    const sem::Module* Module() const { return module_; }

    /// Records that this variable (transitively) references the given override variable.
    /// @param from the item the variable is referenced from
    /// @param var the module-scope override variable
    void AddTransitivelyReferencedOverride(const utils::CastableBase* from,
                                           const GlobalVariable* var) {
        if (referenced_overrides_.count(from) == 0) {
            referenced_overrides_.insert({from, TransitivelyReferenced{}});
        }
        referenced_overrides_[from].Add(var);
    }

    /// @param from the key to look up
    /// @returns all transitively referenced override variables or nullptr if none set
    const TransitivelyReferenced* TransitivelyReferencedOverrides(
        const utils::CastableBase* from) const {
        if (referenced_overrides_.count(from) == 0) {
            return nullptr;
        }
        return &referenced_overrides_.at(from);
    }

    /// Determines the severity of a filterable diagnostic rule for the AST node `ast_node`.
    /// @param ast_node the AST node
    /// @param rule the diagnostic rule
    /// @returns the severity of the rule for that AST node
    builtin::DiagnosticSeverity DiagnosticSeverity(const ast::Node* ast_node,
                                                   builtin::DiagnosticRule rule) const;

  private:
    // AST node index to semantic node
    std::vector<const utils::CastableBase*> nodes_;
    // Lists transitively referenced overrides for the given item
    std::unordered_map<const utils::CastableBase*, TransitivelyReferenced> referenced_overrides_;
    // The semantic module
    sem::Module* module_ = nullptr;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_INFO_H_
