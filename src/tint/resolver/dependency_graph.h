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

#ifndef SRC_TINT_RESOLVER_DEPENDENCY_GRAPH_H_
#define SRC_TINT_RESOLVER_DEPENDENCY_GRAPH_H_

#include <string>
#include <vector>

#include "src/tint/ast/module.h"
#include "src/tint/builtin/access.h"
#include "src/tint/builtin/builtin.h"
#include "src/tint/builtin/builtin_value.h"
#include "src/tint/builtin/function.h"
#include "src/tint/builtin/interpolation_sampling.h"
#include "src/tint/builtin/interpolation_type.h"
#include "src/tint/builtin/texel_format.h"
#include "src/tint/diagnostic/diagnostic.h"
#include "src/tint/utils/hashmap.h"

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
/// - builtin::Function
/// - builtin::Access
/// - builtin::AddressSpace
/// - builtin::Builtin
/// - builtin::BuiltinValue
/// - builtin::InterpolationSampling
/// - builtin::InterpolationType
/// - builtin::TexelFormat
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

    /// @return the builtin function if the ResolvedIdentifier holds builtin::Function, otherwise
    /// builtin::Function::kNone
    builtin::Function BuiltinFunction() const {
        if (auto n = std::get_if<builtin::Function>(&value_)) {
            return *n;
        }
        return builtin::Function::kNone;
    }

    /// @return the access if the ResolvedIdentifier holds builtin::Access, otherwise
    /// builtin::Access::kUndefined
    builtin::Access Access() const {
        if (auto n = std::get_if<builtin::Access>(&value_)) {
            return *n;
        }
        return builtin::Access::kUndefined;
    }

    /// @return the address space if the ResolvedIdentifier holds builtin::AddressSpace, otherwise
    /// builtin::AddressSpace::kUndefined
    builtin::AddressSpace AddressSpace() const {
        if (auto n = std::get_if<builtin::AddressSpace>(&value_)) {
            return *n;
        }
        return builtin::AddressSpace::kUndefined;
    }

    /// @return the builtin type if the ResolvedIdentifier holds builtin::Builtin, otherwise
    /// builtin::Builtin::kUndefined
    builtin::Builtin BuiltinType() const {
        if (auto n = std::get_if<builtin::Builtin>(&value_)) {
            return *n;
        }
        return builtin::Builtin::kUndefined;
    }

    /// @return the builtin value if the ResolvedIdentifier holds builtin::BuiltinValue, otherwise
    /// builtin::BuiltinValue::kUndefined
    builtin::BuiltinValue BuiltinValue() const {
        if (auto n = std::get_if<builtin::BuiltinValue>(&value_)) {
            return *n;
        }
        return builtin::BuiltinValue::kUndefined;
    }

    /// @return the texel format if the ResolvedIdentifier holds type::InterpolationSampling,
    /// otherwise type::InterpolationSampling::kUndefined
    builtin::InterpolationSampling InterpolationSampling() const {
        if (auto n = std::get_if<builtin::InterpolationSampling>(&value_)) {
            return *n;
        }
        return builtin::InterpolationSampling::kUndefined;
    }

    /// @return the texel format if the ResolvedIdentifier holds type::InterpolationType,
    /// otherwise type::InterpolationType::kUndefined
    builtin::InterpolationType InterpolationType() const {
        if (auto n = std::get_if<builtin::InterpolationType>(&value_)) {
            return *n;
        }
        return builtin::InterpolationType::kUndefined;
    }

    /// @return the texel format if the ResolvedIdentifier holds type::TexelFormat, otherwise
    /// type::TexelFormat::kUndefined
    builtin::TexelFormat TexelFormat() const {
        if (auto n = std::get_if<builtin::TexelFormat>(&value_)) {
            return *n;
        }
        return builtin::TexelFormat::kUndefined;
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

    /// @param diagnostics diagnostics used to report ICEs
    /// @return a description of the resolved symbol
    std::string String(diag::List& diagnostics) const;

  private:
    std::variant<UnresolvedIdentifier,
                 const ast::Node*,
                 builtin::Function,
                 builtin::Access,
                 builtin::AddressSpace,
                 builtin::Builtin,
                 builtin::BuiltinValue,
                 builtin::InterpolationSampling,
                 builtin::InterpolationType,
                 builtin::TexelFormat>
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
    utils::Vector<const ast::Node*, 32> ordered_globals;

    /// Map of ast::Identifier to a ResolvedIdentifier
    utils::Hashmap<const ast::Identifier*, ResolvedIdentifier, 64> resolved_identifiers;

    /// Map of ast::Variable to a type, function, or variable that is shadowed by
    /// the variable key. A declaration (X) shadows another (Y) if X and Y use
    /// the same symbol, and X is declared in a sub-scope of the scope that
    /// declares Y.
    utils::Hashmap<const ast::Variable*, const ast::Node*, 16> shadows;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_RESOLVER_DEPENDENCY_GRAPH_H_
