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

#ifndef SRC_TINT_SEM_TYPE_MAPPINGS_H_
#define SRC_TINT_SEM_TYPE_MAPPINGS_H_

#include <type_traits>

// Forward declarations
namespace tint::ast {
class Array;
class Expression;
class ForLoopStatement;
class Function;
class IfStatement;
class Node;
class Override;
class Statement;
class Struct;
class StructMember;
class SwitchStatement;
class Type;
class TypeDecl;
class Variable;
class WhileStatement;
}  // namespace tint::ast
namespace tint::sem {
class Array;
class Expression;
class ForLoopStatement;
class Function;
class IfStatement;
class Node;
class GlobalVariable;
class Statement;
class Struct;
class StructMember;
class SwitchStatement;
class Type;
class Variable;
class WhileStatement;
}  // namespace tint::sem

namespace tint::sem {

/// TypeMappings is a struct that holds undefined `operator()` methods that's
/// used by SemanticNodeTypeFor to map AST / type node types to their
/// corresponding semantic node types. The standard operator overload resolving
/// rules will be used to infer the return type based on the argument type.
struct TypeMappings {
    //! @cond Doxygen_Suppress
    Array* operator()(ast::Array*);
    Expression* operator()(ast::Expression*);
    ForLoopStatement* operator()(ast::ForLoopStatement*);
    Function* operator()(ast::Function*);
    IfStatement* operator()(ast::IfStatement*);
    Node* operator()(ast::Node*);
    GlobalVariable* operator()(ast::Override*);
    Statement* operator()(ast::Statement*);
    Struct* operator()(ast::Struct*);
    StructMember* operator()(ast::StructMember*);
    SwitchStatement* operator()(ast::SwitchStatement*);
    Type* operator()(ast::Type*);
    Type* operator()(ast::TypeDecl*);
    Variable* operator()(ast::Variable*);
    WhileStatement* operator()(ast::WhileStatement*);
    //! @endcond
};

/// SemanticNodeTypeFor resolves to the appropriate sem::Node type for the
/// AST or type node `AST_OR_TYPE`.
template <typename AST_OR_TYPE>
using SemanticNodeTypeFor =
    typename std::remove_pointer<decltype(TypeMappings()(std::declval<AST_OR_TYPE*>()))>::type;

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_TYPE_MAPPINGS_H_
