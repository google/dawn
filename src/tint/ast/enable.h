// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_AST_ENABLE_H_
#define SRC_TINT_AST_ENABLE_H_

#include <string>
#include <unordered_set>
#include <utility>

#include "src/tint/ast/access.h"
#include "src/tint/ast/expression.h"

namespace tint::ast {

/// An instance of this class represents one extension mentioned in a
/// "enable" derictive. Example:
///       // Enable an extension named "f16"
///       enable f16;
class Enable : public Castable<Enable, Node> {
  public:
    ///  The enum class identifing each supported WGSL extension
    enum class ExtensionKind {
        /// An internal reserved extension for test, named
        /// "InternalExtensionForTesting"
        kInternalExtensionForTesting = -2,
        kNotAnExtension = -1,
    };

    /// Convert a string of extension name into one of ExtensionKind enum value,
    /// the result will be ExtensionKind::kNotAnExtension if the name is not a
    /// known extension name. A extension node of kind kNotAnExtension must not
    /// exist in the AST tree, and using a unknown extension name in WGSL code
    /// should result in a shader-creation error.
    /// @param name string of the extension name
    /// @return the ExtensionKind enum value for the extension of given name, or
    /// kNotAnExtension if no known extension has the given name
    static ExtensionKind NameToKind(const std::string& name);

    /// Convert the ExtensionKind enum value to corresponding extension name
    /// string. If the given enum value is kNotAnExtension or don't have a known
    /// name, return an empty string instead.
    /// @param kind the ExtensionKind enum value
    /// @return string of the extension name corresponding to the given kind, or
    /// an empty string if the given enum value is kNotAnExtension or don't have a
    /// known corresponding name
    static std::string KindToName(ExtensionKind kind);

    /// Create a extension
    /// @param pid the identifier of the program that owns this node
    /// @param src the source of this node
    /// @param name the name of extension
    Enable(ProgramID pid, const Source& src, const std::string& name);
    /// Move constructor
    Enable(Enable&&);

    ~Enable() override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const Enable* Clone(CloneContext* ctx) const override;

    /// The extension name
    const std::string name;

    /// The extension kind
    const ExtensionKind kind;
};

///  A set of extension kinds
using ExtensionSet = std::unordered_set<Enable::ExtensionKind>;

}  // namespace tint::ast

#endif  // SRC_TINT_AST_ENABLE_H_
