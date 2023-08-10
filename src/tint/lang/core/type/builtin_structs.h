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

#ifndef SRC_TINT_LANG_CORE_TYPE_BUILTIN_STRUCTS_H_
#define SRC_TINT_LANG_CORE_TYPE_BUILTIN_STRUCTS_H_

// Forward declarations
namespace tint {
class SymbolTable;
}  // namespace tint
namespace tint::core::type {
class Manager;
class Struct;
class Type;
}  // namespace tint::core::type

namespace tint::core::type {

/// @param types the type manager
/// @param symbols the symbol table
/// @param ty the type of the `fract` and `whole` struct members.
/// @returns the builtin struct type for a modf() builtin call.
Struct* CreateModfResult(Manager& types, SymbolTable& symbols, const Type* ty);

/// @param types the type manager
/// @param symbols the symbol table
/// @param fract the type of the `fract` struct member.
/// @returns the builtin struct type for a frexp() builtin call.
Struct* CreateFrexpResult(Manager& types, SymbolTable& symbols, const Type* fract);

/// @param types the type manager
/// @param symbols the symbol table
/// @param ty the type of the `old_value` struct member.
/// @returns the builtin struct type for a atomic_compare_exchange() builtin call.
Struct* CreateAtomicCompareExchangeResult(Manager& types, SymbolTable& symbols, const Type* ty);

}  // namespace tint::core::type

#endif  // SRC_TINT_LANG_CORE_TYPE_BUILTIN_STRUCTS_H_
