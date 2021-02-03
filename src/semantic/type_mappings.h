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

#ifndef SRC_SEMANTIC_TYPE_MAPPINGS_H_
#define SRC_SEMANTIC_TYPE_MAPPINGS_H_

#include <type_traits>

namespace tint {

// Forward declarations
namespace ast {

class Expression;
class Function;

}  // namespace ast

namespace semantic {

class Expression;
class Function;

/// TypeMappings is a struct that holds dummy `operator()` methods that's used
/// by SemanticNodeTypeFor to map AST node types to their corresponding semantic
/// node types.
/// The standard operator overload resolving rules will be used to infer the
/// return type based on the argument type.
struct TypeMappings {
  //! @cond Doxygen_Suppress
  semantic::Expression* operator()(ast::Expression*);
  semantic::Function* operator()(ast::Function*);
  //! @endcond
};

/// SemanticNodeTypeFor resolves to the appropriate semantic::Node type for the
/// AST node type `AST`.
template <typename AST>
using SemanticNodeTypeFor = typename std::remove_pointer<decltype(
    TypeMappings()(std::declval<AST*>()))>::type;

}  // namespace semantic
}  // namespace tint

#endif  // SRC_SEMANTIC_TYPE_MAPPINGS_H_
