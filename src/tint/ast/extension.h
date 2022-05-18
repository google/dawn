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

#ifndef SRC_TINT_AST_EXTENSION_H_
#define SRC_TINT_AST_EXTENSION_H_

#include <sstream>
#include <string>

#include "src/tint/utils/unique_vector.h"

namespace tint::ast {

/// An enumerator of WGSL extensions
enum class Extension {
    /// WGSL Extension "f16"
    kF16,

    /// An extension for the experimental feature
    /// "chromium_experimental_dp4a".
    /// See crbug.com/tint/1497 for more details
    kChromiumExperimentalDP4a,
    /// A Chromium-specific extension for disabling uniformity analysis.
    kChromiumDisableUniformityAnalysis,

    /// Reserved for representing "No extension required" or "Not a valid extension".
    kNone,
};

/// Convert a string of extension name into one of Extension enum value, the result will be
/// Extension::kNone if the name is not a known extension name. A extension node of kind
/// kNone must not exist in the AST tree, and using a unknown extension name in WGSL code
/// should result in a shader-creation error.
/// @param name string of the extension name
/// @return the Extension enum value for the extension of given name, or kNone if no known extension
/// has the given name
Extension ParseExtension(const std::string& name);

/// Convert the Extension enum value to corresponding extension name string.
/// @param ext the Extension enum value
/// @return string of the extension name corresponding to the given kind, or
/// an empty string if the given enum value is kNone or don't have a
/// known corresponding name
const char* ExtensionName(Extension ext);

/// @returns the name of the extension.
const char* str(Extension i);

/// Emits the name of the extension type.
std::ostream& operator<<(std::ostream& out, Extension i);

// A unique vector of extensions
using Extensions = utils::UniqueVector<Extension>;

}  // namespace tint::ast

#endif  // SRC_TINT_AST_EXTENSION_H_
