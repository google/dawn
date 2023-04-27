// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_RESOLVER_BUILTIN_STRUCTS_H_
#define SRC_TINT_RESOLVER_BUILTIN_STRUCTS_H_

// Forward declarations
namespace tint {
class ProgramBuilder;
}  // namespace tint
namespace tint::type {
class Struct;
class Type;
}  // namespace tint::type

namespace tint::resolver {

/**
 * @param ty the type of the `fract` and `whole` struct members.
 * @return the builtin struct type for a modf() builtin call.
 */
type::Struct* CreateModfResult(ProgramBuilder& b, const type::Type* ty);

/**
 * @param fract the type of the `fract` struct member.
 * @return the builtin struct type for a frexp() builtin call.
 */
type::Struct* CreateFrexpResult(ProgramBuilder& b, const type::Type* fract);

/**
 * @param ty the type of the `old_value` struct member.
 * @return the builtin struct type for a atomic_compare_exchange() builtin call.
 */
type::Struct* CreateAtomicCompareExchangeResult(ProgramBuilder& b, const type::Type* ty);

}  // namespace tint::resolver

#endif  // SRC_TINT_RESOLVER_BUILTIN_STRUCTS_H_
