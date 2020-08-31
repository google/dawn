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

#ifndef SRC_AST_INTRINSIC_H_
#define SRC_AST_INTRINSIC_H_

#include <string>

namespace tint {
namespace ast {
namespace intrinsic {

/// Determines if the given |name| is a coarse derivative
/// @param name the name to check
/// @returns true if the given derivative is coarse.
bool IsCoarseDerivative(const std::string& name);

/// Determines if the given |name| is a fine derivative
/// @param name the name to check
/// @returns true if the given derivative is fine.
bool IsFineDerivative(const std::string& name);

/// Determine if the given |name| is a derivative intrinsic
/// @param name the name to check
/// @returns true if the given |name| is a derivative intrinsic
bool IsDerivative(const std::string& name);

/// Determines if the given |name| is a float classification intrinsic
/// @param name the name to check
/// @returns true if the given |name| is a float intrinsic
bool IsFloatClassificationIntrinsic(const std::string& name);

/// Determines if the given |name| is a texture operation intrinsic
/// @param name the name to check
/// @returns true if the given |name| is a texture operation intrinsic
bool IsTextureOperationIntrinsic(const std::string& name);

/// Determines if the given |name| is an intrinsic
/// @param name the name to check
/// @returns true if the given |name| is an intrinsic
bool IsIntrinsic(const std::string& name);

}  // namespace intrinsic
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_INTRINSIC_H_
