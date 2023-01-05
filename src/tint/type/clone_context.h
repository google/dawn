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

#ifndef SRC_TINT_TYPE_CLONE_CONTEXT_H_
#define SRC_TINT_TYPE_CLONE_CONTEXT_H_

// Forward Declarations
namespace tint {
class SymbolTable;
}  // namespace tint
namespace tint::type {
class Manager;
}  // namespace tint::type

namespace tint::type {

/// Context information for cloning of types
struct CloneContext {
    /// Source information
    struct {
        /// The source symbol table
        const SymbolTable* st;
    } src;

    /// Destination information
    struct {
        /// The destination symbol table
        SymbolTable* st;
        /// The destination type manger
        Manager* mgr;
    } dst;
};

}  // namespace tint::type

#endif  // SRC_TINT_TYPE_CLONE_CONTEXT_H_
