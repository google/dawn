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

#ifndef SRC_TINT_LANG_WGSL_RESOLVER_DEPENDENCY_GRAPH_H_
#define SRC_TINT_LANG_WGSL_RESOLVER_DEPENDENCY_GRAPH_H_

#include <string>
#include <vector>

#include "src/tint/lang/core/access.h"
#include "src/tint/lang/core/builtin.h"
#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/core/function.h"
#include "src/tint/lang/core/interpolation_sampling.h"
#include "src/tint/lang/core/interpolation_type.h"
#include "src/tint/lang/core/texel_format.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/diagnostic/diagnostic.h"

namespace tint::resolver {

/// UnresolvedIdentifier is the variant value used by ResolvedIdentifier
struct UnresolvedIdentifier {
    /// Name of the unresolved identifier
    std::string name;
};

/// ResolvedIdentifier holds the resolution of an ast::Identifier.
/// Can hold one of:
/// - UnresolvedIdentifier
/// - const ast::TypeDecl*  (as const ast::Node*)
/// - const ast::Variable*  (as const ast::Node*)
/// - const ast::Function*  (as const ast::Node*)
/// - core::Function
/// - core::Access
/// - core::AddressSpace
/// - core::Builtin
/// - core::BuiltinValue
/// - core::InterpolationSampling
/// - core::InterpolationType
/// - core::TexelFormat
class ResolvedIdentifier {
  public:
    /// Constructor
    /// @param value the resolved identifier value
    template <typename T>
    ResolvedIdentifier(T value) : value_(value) {}  // NOLINT(runtime/explicit)

    /// @return the UnresolvedIdentifier if the identifier was not resolved
    const UnresolvedIdentifier* Unresolved() const {
        if (auto n = std::get_if<UnresolvedIdentifier>(&value_)) {
            return n;
        }
        return nullptr;
    }

    /// @return the node pointer if the ResolvedIdentifier holds an AST node, otherwise nullptr
    const ast::Node* Node() const {
        if (auto n = std::get_if<const ast::Node*>(&value_)) {
            return *n;
        }
        return nullptr;
    }

    /// @return the builtin function if the ResolvedIdentifier holds core::Function, otherwise
    /// core::Function::kNone
    core::Function BuiltinFunction() const {
        if (auto n = std::get_if<core::Function>(&value_)) {
            return *n;
        }
        return core::Function::kNone;
    }

    /// @return the access if the ResolvedIdentifier holds core::Access, otherwise
    /// core::Access::kUndefined
    core::Access Access() const {
        if (auto n = std::get_if<core::Access>(&value_)) {
            return *n;
        }
        return core::Access::kUndefined;
    }

    /// @return the address space if the ResolvedIdentifier holds core::AddressSpace, otherwise
    /// core::AddressSpace::kUndefined
    core::AddressSpace AddressSpace() const {
        if (auto n = std::get_if<core::AddressSpace>(&value_)) {
            return *n;
        }
        return core::AddressSpace::kUndefined;
    }

    /// @return the builtin type if the ResolvedIdentifier holds core::Builtin, otherwise
    /// core::Builtin::kUndefined
    core::Builtin BuiltinType() const {
        if (auto n = std::get_if<core::Builtin>(&value_)) {
            return *n;
        }
        return core::Builtin::kUndefined;
    }

    /// @return the builtin value if the ResolvedIdentifier holds core::BuiltinValue, otherwise
    /// core::BuiltinValue::kUndefined
    core::BuiltinValue BuiltinValue() const {
        if (auto n = std::get_if<core::BuiltinValue>(&value_)) {
            return *n;
        }
        return core::BuiltinValue::kUndefined;
    }

    /// @return the texel format if the ResolvedIdentifier holds type::InterpolationSampling,
    /// otherwise type::InterpolationSampling::kUndefined
    core::InterpolationSampling InterpolationSampling() const {
        if (auto n = std::get_if<core::InterpolationSampling>(&value_)) {
            return *n;
        }
        return core::InterpolationSampling::kUndefined;
    }

    /// @return the texel format if the ResolvedIdentifier holds type::InterpolationType,
    /// otherwise type::InterpolationType::kUndefined
    core::InterpolationType InterpolationType() const {
        if (auto n = std::get_if<core::InterpolationType>(&value_)) {
            return *n;
        }
        return core::InterpolationType::kUndefined;
    }

    /// @return the texel format if the ResolvedIdentifier holds type::TexelFormat, otherwise
    /// type::TexelFormat::kUndefined
    core::TexelFormat TexelFormat() const {
        if (auto n = std::get_if<core::TexelFormat>(&value_)) {
            return *n;
        }
        return core::TexelFormat::kUndefined;
    }

    /// @param value the value to compare the ResolvedIdentifier to
    /// @return true if the ResolvedIdentifier is equal to @p value
    template <typename T>
    bool operator==(const T& value) const {
        if (auto n = std::get_if<T>(&value_)) {
            return *n == value;
        }
        return false;
    }

    /// @param other the other value to compare to this
    /// @return true if this ResolvedIdentifier and @p other are not equal
    template <typename T>
    bool operator!=(const T& other) const {
        return !(*this == other);
    }

    /// @return a description of the resolved symbol
    std::string String() const;

  private:
    std::variant<UnresolvedIdentifier,
                 const ast::Node*,
                 core::Function,
                 core::Access,
                 core::AddressSpace,
                 core::Builtin,
                 core::BuiltinValue,
                 core::InterpolationSampling,
                 core::InterpolationType,
                 core::TexelFormat>
        value_;
};

/// DependencyGraph holds information about module-scope declaration dependency
/// analysis and symbol resolutions.
struct DependencyGraph {
    /// Constructor
    DependencyGraph();
    /// Move-constructor
    DependencyGraph(DependencyGraph&&);
    /// Destructor
    ~DependencyGraph();

    /// Build() performs symbol resolution and dependency analysis on `module`,
    /// populating `output` with the resulting dependency graph.
    /// @param module the AST module to analyse
    /// @param diagnostics the diagnostic list to populate with errors / warnings
    /// @param output the resulting DependencyGraph
    /// @returns true on success, false on error
    static bool Build(const ast::Module& module, diag::List& diagnostics, DependencyGraph& output);

    /// All globals in dependency-sorted order.
    Vector<const ast::Node*, 32> ordered_globals;

    /// Map of ast::Identifier to a ResolvedIdentifier
    Hashmap<const ast::Identifier*, ResolvedIdentifier, 64> resolved_identifiers;

    /// Map of ast::Variable to a type, function, or variable that is shadowed by
    /// the variable key. A declaration (X) shadows another (Y) if X and Y use
    /// the same symbol, and X is declared in a sub-scope of the scope that
    /// declares Y.
    Hashmap<const ast::Variable*, const ast::Node*, 16> shadows;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_LANG_WGSL_RESOLVER_DEPENDENCY_GRAPH_H_
