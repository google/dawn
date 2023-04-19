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

#include "src/tint/sem/builtin_enum_expression.h"

// Forward declarations
namespace tint::utils {
class CastableBase;
}  // namespace tint::utils
namespace tint::ast {
class AccessorExpression;
class BinaryExpression;
class BitcastExpression;
class BuiltinAttribute;
class CallExpression;
class Expression;
class ForLoopStatement;
class Function;
class IfStatement;
class LiteralExpression;
class Node;
class Override;
class PhonyExpression;
class Statement;
class Struct;
class StructMember;
class SwitchStatement;
class TypeDecl;
class Variable;
class WhileStatement;
class UnaryOpExpression;
}  // namespace tint::ast
namespace tint::builtin {
enum class BuiltinValue;
}
namespace tint::sem {
class Expression;
class ForLoopStatement;
class Function;
class GlobalVariable;
class IfStatement;
class Node;
class Statement;
class Struct;
class StructMember;
class SwitchStatement;
class ValueExpression;
class Variable;
class WhileStatement;
}  // namespace tint::sem
namespace tint::type {
class Array;
class Type;
}  // namespace tint::type

namespace tint::sem {

/// TypeMappings is a struct that holds undefined `operator()` methods that's
/// used by SemanticNodeTypeFor to map AST / type node types to their
/// corresponding semantic node types. The standard operator overload resolving
/// rules will be used to infer the return type based on the argument type.
struct TypeMappings {
    //! @cond Doxygen_Suppress
    BuiltinEnumExpression<builtin::BuiltinValue>* operator()(ast::BuiltinAttribute*);
    utils::CastableBase* operator()(ast::Node*);
    Expression* operator()(ast::Expression*);
    ForLoopStatement* operator()(ast::ForLoopStatement*);
    Function* operator()(ast::Function*);
    GlobalVariable* operator()(ast::Override*);
    IfStatement* operator()(ast::IfStatement*);
    Statement* operator()(ast::Statement*);
    Struct* operator()(ast::Struct*);
    StructMember* operator()(ast::StructMember*);
    SwitchStatement* operator()(ast::SwitchStatement*);
    type::Type* operator()(ast::TypeDecl*);
    ValueExpression* operator()(ast::AccessorExpression*);
    ValueExpression* operator()(ast::BinaryExpression*);
    ValueExpression* operator()(ast::BitcastExpression*);
    ValueExpression* operator()(ast::CallExpression*);
    ValueExpression* operator()(ast::LiteralExpression*);
    ValueExpression* operator()(ast::PhonyExpression*);
    ValueExpression* operator()(ast::UnaryOpExpression*);
    Variable* operator()(ast::Variable*);
    WhileStatement* operator()(ast::WhileStatement*);
    //! @endcond
};

/// SemanticNodeTypeFor resolves to the appropriate sem::Node type for the
/// AST node `AST`.
template <typename AST>
using SemanticNodeTypeFor =
    typename std::remove_pointer<decltype(TypeMappings()(std::declval<AST*>()))>::type;

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_TYPE_MAPPINGS_H_
