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

#include <type_traits>
#include <unordered_map>

#include "src/tint/debug.h"
#include "src/tint/sem/node.h"
#include "src/tint/sem/type_mappings.h"

// Forward declarations
namespace tint::sem {
class Module;
}  // namespace tint::sem

namespace tint::sem {

/// Info holds all the resolved semantic information for a Program.
class Info {
  public:
    /// Placeholder type used by Get() to provide a default value for EXPLICIT_SEM
    using InferFromAST = std::nullptr_t;

    /// Resolves to the return type of the Get() method given the desired sementic
    /// type and AST type.
    template <typename SEM, typename AST_OR_TYPE>
    using GetResultType = std::conditional_t<std::is_same<SEM, InferFromAST>::value,
                                             SemanticNodeTypeFor<AST_OR_TYPE>,
                                             SEM>;

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

    /// Get looks up the semantic information for the AST or type node `node`.
    /// @param node the AST or type node
    /// @returns a pointer to the semantic node if found, otherwise nullptr
    template <typename SEM = InferFromAST,
              typename AST_OR_TYPE = CastableBase,
              typename RESULT = GetResultType<SEM, AST_OR_TYPE>>
    const RESULT* Get(const AST_OR_TYPE* node) const {
        auto it = map_.find(node);
        if (it == map_.end()) {
            return nullptr;
        }
        return As<RESULT>(it->second);
    }

    /// Add registers the semantic node `sem_node` for the AST or type node
    /// `node`.
    /// @param node the AST or type node
    /// @param sem_node the semantic node
    template <typename AST_OR_TYPE>
    void Add(const AST_OR_TYPE* node, const SemanticNodeTypeFor<AST_OR_TYPE>* sem_node) {
        // Check there's no semantic info already existing for the node
        TINT_ASSERT(Semantic, Get(node) == nullptr);
        map_.emplace(node, sem_node);
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
        out.map_ = inner.map_;
        out.module_ = inner.module_;
        return out;
    }

    /// Assigns the semantic module.
    /// @param module the module to assign.
    void SetModule(sem::Module* module) { module_ = module; }

    /// @returns the semantic module.
    const sem::Module* Module() const { return module_; }

  private:
    // TODO(crbug.com/tint/724): Once finished, this map should be:
    // std::unordered_map<const ast::Node*, const sem::Node*>
    std::unordered_map<const CastableBase*, const CastableBase*> map_;
    // The semantic module
    sem::Module* module_ = nullptr;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_INFO_H_
