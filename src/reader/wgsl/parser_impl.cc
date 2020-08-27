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

#include "src/reader/wgsl/parser_impl.h"

#include <memory>
#include <vector>

#include "src/ast/array_accessor_expression.h"
#include "src/ast/as_expression.h"
#include "src/ast/binary_expression.h"
#include "src/ast/binding_decoration.h"
#include "src/ast/bool_literal.h"
#include "src/ast/break_statement.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/call_expression.h"
#include "src/ast/case_statement.h"
#include "src/ast/cast_expression.h"
#include "src/ast/continue_statement.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/discard_statement.h"
#include "src/ast/else_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/location_decoration.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/set_decoration.h"
#include "src/ast/sint_literal.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/ast/unary_op.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/reader/wgsl/lexer.h"
#include "src/type_manager.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

/// Controls the maximum number of times we'll call into the const_expr function
/// from itself. This is to guard against stack overflow when there is an
/// excessive number of type constructors inside the const_expr.
uint32_t kMaxConstExprDepth = 128;

ast::Builtin ident_to_builtin(const std::string& str) {
  if (str == "position") {
    return ast::Builtin::kPosition;
  }
  if (str == "vertex_idx") {
    return ast::Builtin::kVertexIdx;
  }
  if (str == "instance_idx") {
    return ast::Builtin::kInstanceIdx;
  }
  if (str == "front_facing") {
    return ast::Builtin::kFrontFacing;
  }
  if (str == "frag_coord") {
    return ast::Builtin::kFragCoord;
  }
  if (str == "frag_depth") {
    return ast::Builtin::kFragDepth;
  }
  if (str == "workgroup_size") {
    return ast::Builtin::kWorkgroupSize;
  }
  if (str == "local_invocation_id") {
    return ast::Builtin::kLocalInvocationId;
  }
  if (str == "local_invocation_idx") {
    return ast::Builtin::kLocalInvocationIdx;
  }
  if (str == "global_invocation_id") {
    return ast::Builtin::kGlobalInvocationId;
  }
  return ast::Builtin::kNone;
}

}  // namespace

ParserImpl::ParserImpl(Context* ctx, const std::string& input)
    : ctx_(*ctx), lexer_(std::make_unique<Lexer>(input)) {}

ParserImpl::~ParserImpl() = default;

void ParserImpl::set_error(const Token& t, const std::string& err) {
  auto prefix =
      std::to_string(t.line()) + ":" + std::to_string(t.column()) + ": ";

  if (t.IsReservedKeyword()) {
    error_ = prefix + "reserved token (" + t.to_str() + ") found";
    return;
  }
  if (t.IsError()) {
    error_ = prefix + t.to_str();
    return;
  }

  if (err.size() != 0) {
    error_ = prefix + err;
  } else {
    error_ = prefix + "invalid token (" + t.to_name() + ") encountered";
  }
}

void ParserImpl::set_error(const Token& t) {
  set_error(t, "");
}

Token ParserImpl::next() {
  if (!token_queue_.empty()) {
    auto t = token_queue_.front();
    token_queue_.pop_front();
    return t;
  }
  return lexer_->next();
}

Token ParserImpl::peek(size_t idx) {
  while (token_queue_.size() < (idx + 1))
    token_queue_.push_back(lexer_->next());

  return token_queue_[idx];
}

Token ParserImpl::peek() {
  return peek(0);
}

void ParserImpl::register_alias(const std::string& name,
                                ast::type::Type* type) {
  assert(type);
  registered_aliases_[name] = type;
}

ast::type::Type* ParserImpl::get_alias(const std::string& name) {
  if (registered_aliases_.find(name) == registered_aliases_.end()) {
    return nullptr;
  }
  return registered_aliases_[name];
}

bool ParserImpl::Parse() {
  translation_unit();
  return !has_error();
}

// translation_unit
//  : global_decl* EOF
void ParserImpl::translation_unit() {
  for (;;) {
    global_decl();
    if (has_error())
      return;

    if (peek().IsEof())
      break;
  }

  assert(module_.IsValid());
}

// global_decl
//  : SEMICOLON
//  | import_decl SEMICOLON
//  | global_variable_decl SEMICLON
//  | global_constant_decl SEMICOLON
//  | entry_point_decl SEMICOLON
//  | type_alias SEMICOLON
//  | function_decl
void ParserImpl::global_decl() {
  auto t = peek();
  if (t.IsEof())
    return;

  if (t.IsSemicolon()) {
    next();  // consume the peek
    return;
  }

  auto import = import_decl();
  if (has_error())
    return;
  if (import != nullptr) {
    t = next();
    if (!t.IsSemicolon()) {
      set_error(t, "missing ';' for import");
      return;
    }
    module_.AddImport(std::move(import));
    return;
  }

  auto gv = global_variable_decl();
  if (has_error())
    return;
  if (gv != nullptr) {
    t = next();
    if (!t.IsSemicolon()) {
      set_error(t, "missing ';' for variable declaration");
      return;
    }
    module_.AddGlobalVariable(std::move(gv));
    return;
  }

  auto gc = global_constant_decl();
  if (has_error())
    return;
  if (gc != nullptr) {
    t = next();
    if (!t.IsSemicolon()) {
      set_error(t, "missing ';' for constant declaration");
      return;
    }
    module_.AddGlobalVariable(std::move(gc));
    return;
  }

  auto ep = entry_point_decl();
  if (has_error())
    return;
  if (ep != nullptr) {
    t = next();
    if (!t.IsSemicolon()) {
      set_error(t, "missing ';' for entry point");
      return;
    }
    module_.AddEntryPoint(std::move(ep));
    return;
  }

  auto* ta = type_alias();
  if (has_error())
    return;
  if (ta != nullptr) {
    t = next();
    if (!t.IsSemicolon()) {
      set_error(t, "missing ';' for type alias");
      return;
    }
    module_.AddAliasType(ta);
    return;
  }

  auto func = function_decl();
  if (has_error())
    return;
  if (func != nullptr) {
    module_.AddFunction(std::move(func));
    return;
  }

  set_error(t);
}

// import_decl
//  : IMPORT STRING_LITERAL AS (IDENT NAMESPACE)* IDENT
std::unique_ptr<ast::Import> ParserImpl::import_decl() {
  auto t = peek();
  if (!t.IsImport())
    return {};

  auto source = t.source();
  next();  // consume the import token

  t = next();
  if (!t.IsStringLiteral()) {
    set_error(t, "missing path for import");
    return {};
  }
  auto path = t.to_str();
  if (path.length() == 0) {
    set_error(t, "import path must not be empty");
    return {};
  }

  t = next();
  if (!t.IsAs()) {
    set_error(t, "missing 'as' for import");
    return {};
  }

  std::string name = "";
  for (;;) {
    t = peek();
    if (!t.IsIdentifier()) {
      break;
    }
    next();  // consume the peek

    name += t.to_str();

    t = peek();
    if (!t.IsNamespace()) {
      break;
    }
    next();  // consume the peek

    name += "::";
  }
  if (name.length() == 0) {
    if (t.IsEof() || t.IsSemicolon()) {
      set_error(t, "missing name for import");
    } else {
      set_error(t, "invalid name for import");
    }
    return {};
  }
  if (name.length() > 2) {
    auto end = name.length() - 1;
    if (name[end] == ':' && name[end - 1] == ':') {
      set_error(t, "invalid name for import");
      return {};
    }
  }
  return std::make_unique<ast::Import>(source, path, name);
}

// global_variable_decl
//  : variable_decoration_list variable_decl
//  | variable_decoration_list variable_decl EQUAL const_expr
std::unique_ptr<ast::Variable> ParserImpl::global_variable_decl() {
  auto decos = variable_decoration_list();
  if (has_error())
    return nullptr;

  auto var = variable_decl();
  if (has_error())
    return nullptr;
  if (var == nullptr) {
    if (decos.size() > 0)
      set_error(peek(), "error parsing variable declaration");

    return nullptr;
  }

  if (decos.size() > 0) {
    auto dv = std::make_unique<ast::DecoratedVariable>(std::move(var));
    dv->set_decorations(std::move(decos));

    var = std::move(dv);
  }

  auto t = peek();
  if (t.IsEqual()) {
    next();  // Consume the peek

    auto expr = const_expr();
    if (has_error())
      return nullptr;
    if (expr == nullptr) {
      set_error(peek(), "invalid expression");
      return nullptr;
    }

    var->set_constructor(std::move(expr));
  }
  return var;
}

// global_constant_decl
//  : CONST variable_ident_decl EQUAL const_expr
std::unique_ptr<ast::Variable> ParserImpl::global_constant_decl() {
  auto t = peek();
  if (!t.IsConst())
    return nullptr;

  auto source = t.source();
  next();  // Consume the peek

  std::string name;
  ast::type::Type* type;
  std::tie(name, type) = variable_ident_decl();
  if (has_error())
    return nullptr;
  if (name.empty() || type == nullptr) {
    set_error(peek(), "error parsing constant variable identifier");
    return nullptr;
  }

  auto var = std::make_unique<ast::Variable>(source, name,
                                             ast::StorageClass::kNone, type);
  var->set_is_const(true);

  t = next();
  if (!t.IsEqual()) {
    set_error(t, "missing = for const declaration");
    return nullptr;
  }

  auto init = const_expr();
  if (has_error())
    return nullptr;
  if (init == nullptr) {
    set_error(peek(), "error parsing scalar constructor");
    return nullptr;
  }
  var->set_constructor(std::move(init));

  return var;
}

// variable_decoration_list
//  : ATTR_LEFT (variable_decoration COMMA)* variable_decoration ATTR_RIGHT
ast::VariableDecorationList ParserImpl::variable_decoration_list() {
  ast::VariableDecorationList decos;

  auto t = peek();
  if (!t.IsAttrLeft())
    return decos;

  next();  // consume the peek

  auto deco = variable_decoration();
  if (has_error())
    return {};
  if (deco == nullptr) {
    t = peek();
    if (t.IsAttrRight()) {
      set_error(t, "empty variable decoration list");
      return {};
    }
    set_error(t, "missing variable decoration for decoration list");
    return {};
  }
  for (;;) {
    decos.push_back(std::move(deco));

    t = peek();
    if (!t.IsComma()) {
      break;
    }
    next();  // consume the peek

    deco = variable_decoration();
    if (has_error())
      return {};
    if (deco == nullptr) {
      set_error(peek(), "missing variable decoration after comma");
      return {};
    }
  }

  t = peek();
  if (!t.IsAttrRight()) {
    deco = variable_decoration();
    if (deco != nullptr) {
      set_error(t, "missing comma in variable decoration list");
      return {};
    }
    set_error(t, "missing ]] for variable decoration");
    return {};
  }
  next();  // consume the peek

  return decos;
}

// variable_decoration
//  : LOCATION INT_LITERAL
//  | BUILTIN IDENT
//  | BINDING INT_LITERAL
//  | SET INT_LITERAL
std::unique_ptr<ast::VariableDecoration> ParserImpl::variable_decoration() {
  auto t = peek();
  if (t.IsLocation()) {
    next();  // consume the peek

    t = next();
    if (!t.IsSintLiteral()) {
      set_error(t, "invalid value for location decoration");
      return {};
    }

    return std::make_unique<ast::LocationDecoration>(t.to_i32());
  }
  if (t.IsBuiltin()) {
    next();  // consume the peek

    t = next();
    if (!t.IsIdentifier() || t.to_str().empty()) {
      set_error(t, "expected identifier for builtin");
      return {};
    }

    ast::Builtin builtin = ident_to_builtin(t.to_str());
    if (builtin == ast::Builtin::kNone) {
      set_error(t, "invalid value for builtin decoration");
      return {};
    }

    return std::make_unique<ast::BuiltinDecoration>(builtin);
  }
  if (t.IsBinding()) {
    next();  // consume the peek

    t = next();
    if (!t.IsSintLiteral()) {
      set_error(t, "invalid value for binding decoration");
      return {};
    }

    return std::make_unique<ast::BindingDecoration>(t.to_i32());
  }
  if (t.IsSet()) {
    next();  // consume the peek

    t = next();
    if (!t.IsSintLiteral()) {
      set_error(t, "invalid value for set decoration");
      return {};
    }

    return std::make_unique<ast::SetDecoration>(t.to_i32());
  }

  return nullptr;
}

// variable_decl
//   : VAR variable_storage_decoration? variable_ident_decl
std::unique_ptr<ast::Variable> ParserImpl::variable_decl() {
  auto t = peek();
  auto source = t.source();
  if (!t.IsVar())
    return nullptr;

  next();  // Consume the peek

  auto sc = variable_storage_decoration();
  if (has_error())
    return {};

  std::string name;
  ast::type::Type* type;
  std::tie(name, type) = variable_ident_decl();
  if (has_error())
    return nullptr;
  if (name.empty() || type == nullptr) {
    set_error(peek(), "invalid identifier declaration");
    return nullptr;
  }

  return std::make_unique<ast::Variable>(source, name, sc, type);
}

// variable_ident_decl
//   : IDENT COLON type_decl
std::pair<std::string, ast::type::Type*> ParserImpl::variable_ident_decl() {
  auto t = peek();
  if (!t.IsIdentifier())
    return {};

  auto name = t.to_str();
  next();  // Consume the peek

  t = next();
  if (!t.IsColon()) {
    set_error(t, "missing : for identifier declaration");
    return {};
  }

  auto* type = type_decl();
  if (has_error())
    return {};
  if (type == nullptr) {
    set_error(peek(), "invalid type for identifier declaration");
    return {};
  }

  return {name, type};
}

// variable_storage_decoration
//   : LESS_THAN storage_class GREATER_THAN
ast::StorageClass ParserImpl::variable_storage_decoration() {
  auto t = peek();
  if (!t.IsLessThan())
    return ast::StorageClass::kNone;

  next();  // Consume the peek

  auto sc = storage_class();
  if (has_error())
    return sc;
  if (sc == ast::StorageClass::kNone) {
    set_error(peek(), "invalid storage class for variable decoration");
    return sc;
  }

  t = next();
  if (!t.IsGreaterThan()) {
    set_error(t, "missing > for variable decoration");
    return ast::StorageClass::kNone;
  }

  return sc;
}

// type_alias
//   : TYPE IDENT EQUAL type_decl
//   | TYPE IDENT EQUAL struct_decl
ast::type::AliasType* ParserImpl::type_alias() {
  auto t = peek();
  if (!t.IsType())
    return nullptr;

  next();  // Consume the peek

  t = next();
  if (!t.IsIdentifier()) {
    set_error(t, "missing identifier for type alias");
    return nullptr;
  }
  auto name = t.to_str();

  t = next();
  if (!t.IsEqual()) {
    set_error(t, "missing = for type alias");
    return nullptr;
  }

  auto* type = type_decl();
  if (has_error())
    return nullptr;
  if (type == nullptr) {
    auto str = struct_decl();
    if (has_error())
      return nullptr;
    if (str == nullptr) {
      set_error(peek(), "invalid type alias");
      return nullptr;
    }

    str->set_name(name);
    type = ctx_.type_mgr().Get(std::move(str));
  }
  if (type == nullptr) {
    set_error(peek(), "invalid type for alias");
    return nullptr;
  }

  auto* alias =
      ctx_.type_mgr().Get(std::make_unique<ast::type::AliasType>(name, type));
  register_alias(name, alias);

  return alias->AsAlias();
}

// type_decl
//   : IDENTIFIER
//   | BOOL
//   | FLOAT32
//   | INT32
//   | UINT32
//   | VEC2 LESS_THAN type_decl GREATER_THAN
//   | VEC3 LESS_THAN type_decl GREATER_THAN
//   | VEC4 LESS_THAN type_decl GREATER_THAN
//   | PTR LESS_THAN storage_class, type_decl GREATER_THAN
//   | array_decoration_list? ARRAY LESS_THAN type_decl COMMA
//          INT_LITERAL GREATER_THAN
//   | array_decoration_list? ARRAY LESS_THAN type_decl
//          GREATER_THAN
//   | MAT2x2 LESS_THAN type_decl GREATER_THAN
//   | MAT2x3 LESS_THAN type_decl GREATER_THAN
//   | MAT2x4 LESS_THAN type_decl GREATER_THAN
//   | MAT3x2 LESS_THAN type_decl GREATER_THAN
//   | MAT3x3 LESS_THAN type_decl GREATER_THAN
//   | MAT3x4 LESS_THAN type_decl GREATER_THAN
//   | MAT4x2 LESS_THAN type_decl GREATER_THAN
//   | MAT4x3 LESS_THAN type_decl GREATER_THAN
//   | MAT4x4 LESS_THAN type_decl GREATER_THAN
ast::type::Type* ParserImpl::type_decl() {
  auto t = peek();
  if (t.IsIdentifier()) {
    next();  // Consume the peek
    auto* alias = get_alias(t.to_str());
    if (alias == nullptr) {
      set_error(t, "unknown type alias '" + t.to_str() + "'");
      return nullptr;
    }
    return alias;
  }
  if (t.IsBool()) {
    next();  // Consume the peek
    return ctx_.type_mgr().Get(std::make_unique<ast::type::BoolType>());
  }
  if (t.IsF32()) {
    next();  // Consume the peek
    return ctx_.type_mgr().Get(std::make_unique<ast::type::F32Type>());
  }
  if (t.IsI32()) {
    next();  // Consume the peek
    return ctx_.type_mgr().Get(std::make_unique<ast::type::I32Type>());
  }
  if (t.IsU32()) {
    next();  // Consume the peek
    return ctx_.type_mgr().Get(std::make_unique<ast::type::U32Type>());
  }
  if (t.IsVec2() || t.IsVec3() || t.IsVec4()) {
    return type_decl_vector(t);
  }
  if (t.IsPtr()) {
    return type_decl_pointer(t);
  }

  auto deco = array_decoration_list();
  if (has_error()) {
    return nullptr;
  }
  if (deco != 0) {
    t = peek();
  }
  if (deco != 0 && !t.IsArray()) {
    set_error(t, "found array decoration but no array");
    return nullptr;
  }
  if (t.IsArray()) {
    return type_decl_array(t, deco);
  }
  if (t.IsMat2x2() || t.IsMat2x3() || t.IsMat2x4() || t.IsMat3x2() ||
      t.IsMat3x3() || t.IsMat3x4() || t.IsMat4x2() || t.IsMat4x3() ||
      t.IsMat4x4()) {
    return type_decl_matrix(t);
  }
  return nullptr;
}

ast::type::Type* ParserImpl::type_decl_pointer(Token t) {
  next();  // Consume the peek

  t = next();
  if (!t.IsLessThan()) {
    set_error(t, "missing < for ptr declaration");
    return nullptr;
  }

  auto sc = storage_class();
  if (has_error())
    return nullptr;
  if (sc == ast::StorageClass::kNone) {
    set_error(peek(), "missing storage class for ptr declaration");
    return nullptr;
  }

  t = next();
  if (!t.IsComma()) {
    set_error(t, "missing , for ptr declaration");
    return nullptr;
  }

  auto* subtype = type_decl();
  if (has_error())
    return nullptr;
  if (subtype == nullptr) {
    set_error(peek(), "missing type for ptr declaration");
    return nullptr;
  }

  t = next();
  if (!t.IsGreaterThan()) {
    set_error(t, "missing > for ptr declaration");
    return nullptr;
  }

  return ctx_.type_mgr().Get(
      std::make_unique<ast::type::PointerType>(subtype, sc));
}

ast::type::Type* ParserImpl::type_decl_vector(Token t) {
  next();  // Consume the peek

  uint32_t count = 2;
  if (t.IsVec3())
    count = 3;
  else if (t.IsVec4())
    count = 4;

  t = next();
  if (!t.IsLessThan()) {
    set_error(t, "missing < for vector");
    return nullptr;
  }

  auto* subtype = type_decl();
  if (has_error())
    return nullptr;
  if (subtype == nullptr) {
    set_error(peek(), "unable to determine subtype for vector");
    return nullptr;
  }

  t = next();
  if (!t.IsGreaterThan()) {
    set_error(t, "missing > for vector");
    return nullptr;
  }

  return ctx_.type_mgr().Get(
      std::make_unique<ast::type::VectorType>(subtype, count));
}

ast::type::Type* ParserImpl::type_decl_array(Token t, uint32_t stride) {
  next();  // Consume the peek

  t = next();
  if (!t.IsLessThan()) {
    set_error(t, "missing < for array declaration");
    return nullptr;
  }

  auto* subtype = type_decl();
  if (has_error())
    return nullptr;
  if (subtype == nullptr) {
    set_error(peek(), "invalid type for array declaration");
    return nullptr;
  }

  t = next();
  uint32_t size = 0;
  if (t.IsComma()) {
    t = next();
    if (!t.IsSintLiteral()) {
      set_error(t, "missing size of array declaration");
      return nullptr;
    }
    if (t.to_i32() <= 0) {
      set_error(t, "invalid size for array declaration");
      return nullptr;
    }
    size = static_cast<uint32_t>(t.to_i32());
    t = next();
  }
  if (!t.IsGreaterThan()) {
    set_error(t, "missing > for array declaration");
    return nullptr;
  }

  auto ty = std::make_unique<ast::type::ArrayType>(subtype, size);
  if (stride != 0) {
    ty->set_array_stride(stride);
  }
  return ctx_.type_mgr().Get(std::move(ty));
}

// array_decoration_list
//   : ATTR_LEFT (array_decoration COMMA)* array_decoration ATTR_RIGHT
// array_decoration
//   : STRIDE INT_LITERAL
//
// As there is currently only one decoration I'm combining these for now.
// we can split apart later if needed.
uint32_t ParserImpl::array_decoration_list() {
  auto t = peek();
  if (!t.IsAttrLeft()) {
    return 0;
  }
  t = peek(1);
  if (!t.IsStride()) {
    return 0;
  }

  next();  // consume the peek of [[
  next();  // consume the peek of stride

  t = next();
  if (!t.IsSintLiteral()) {
    set_error(t, "missing value for stride decoration");
    return 0;
  }
  if (t.to_i32() < 0) {
    set_error(t, "invalid stride value: " + t.to_str());
    return 0;
  }

  uint32_t stride = static_cast<uint32_t>(t.to_i32());
  t = next();
  if (!t.IsAttrRight()) {
    set_error(t, "missing ]] for array decoration");
    return 0;
  }
  return stride;
}

ast::type::Type* ParserImpl::type_decl_matrix(Token t) {
  next();  // Consume the peek

  uint32_t rows = 2;
  uint32_t columns = 2;
  if (t.IsMat3x2() || t.IsMat3x3() || t.IsMat3x4()) {
    rows = 3;
  } else if (t.IsMat4x2() || t.IsMat4x3() || t.IsMat4x4()) {
    rows = 4;
  }
  if (t.IsMat2x3() || t.IsMat3x3() || t.IsMat4x3()) {
    columns = 3;
  } else if (t.IsMat2x4() || t.IsMat3x4() || t.IsMat4x4()) {
    columns = 4;
  }

  t = next();
  if (!t.IsLessThan()) {
    set_error(t, "missing < for matrix");
    return nullptr;
  }

  auto* subtype = type_decl();
  if (has_error())
    return nullptr;
  if (subtype == nullptr) {
    set_error(peek(), "unable to determine subtype for matrix");
    return nullptr;
  }

  t = next();
  if (!t.IsGreaterThan()) {
    set_error(t, "missing > for matrix");
    return nullptr;
  }

  return ctx_.type_mgr().Get(
      std::make_unique<ast::type::MatrixType>(subtype, rows, columns));
}

// storage_class
//  : INPUT
//  | OUTPUT
//  | UNIFORM
//  | WORKGROUP
//  | UNIFORM_CONSTANT
//  | STORAGE_BUFFER
//  | IMAGE
//  | PRIVATE
//  | FUNCTION
ast::StorageClass ParserImpl::storage_class() {
  auto t = peek();
  if (t.IsIn()) {
    next();  // consume the peek
    return ast::StorageClass::kInput;
  }
  if (t.IsOut()) {
    next();  // consume the peek
    return ast::StorageClass::kOutput;
  }
  if (t.IsUniform()) {
    next();  // consume the peek
    return ast::StorageClass::kUniform;
  }
  if (t.IsWorkgroup()) {
    next();  // consume the peek
    return ast::StorageClass::kWorkgroup;
  }
  if (t.IsUniformConstant()) {
    next();  // consume the peek
    return ast::StorageClass::kUniformConstant;
  }
  if (t.IsStorageBuffer()) {
    next();  // consume the peek
    return ast::StorageClass::kStorageBuffer;
  }
  if (t.IsImage()) {
    next();  // consume the peek
    return ast::StorageClass::kImage;
  }
  if (t.IsPrivate()) {
    next();  // consume the peek
    return ast::StorageClass::kPrivate;
  }
  if (t.IsFunction()) {
    next();  // consume the peek
    return ast::StorageClass::kFunction;
  }
  return ast::StorageClass::kNone;
}

// struct_decl
//   : struct_decoration_decl? STRUCT struct_body_decl
std::unique_ptr<ast::type::StructType> ParserImpl::struct_decl() {
  auto t = peek();
  auto source = t.source();

  auto deco = struct_decoration_decl();
  if (has_error())
    return nullptr;

  t = next();
  if (!t.IsStruct()) {
    set_error(t, "missing struct declaration");
    return nullptr;
  }

  auto body = struct_body_decl();
  if (has_error()) {
    return nullptr;
  }

  return std::make_unique<ast::type::StructType>(
      std::make_unique<ast::Struct>(source, deco, std::move(body)));
}

// struct_decoration_decl
//  : ATTR_LEFT struct_decoration ATTR_RIGHT
ast::StructDecoration ParserImpl::struct_decoration_decl() {
  auto t = peek();
  if (!t.IsAttrLeft())
    return ast::StructDecoration::kNone;

  auto deco = struct_decoration(peek(1));
  if (has_error())
    return ast::StructDecoration::kNone;
  if (deco == ast::StructDecoration::kNone) {
    return deco;
  }

  next();  // Consume the peek of [[
  next();  // Consume the peek from the struct_decoration

  t = next();
  if (!t.IsAttrRight()) {
    set_error(t, "missing ]] for struct decoration");
    return ast::StructDecoration::kNone;
  }

  return deco;
}

// struct_decoration
//  : BLOCK
ast::StructDecoration ParserImpl::struct_decoration(Token t) {
  if (t.IsBlock()) {
    return ast::StructDecoration::kBlock;
  }
  return ast::StructDecoration::kNone;
}

// struct_body_decl
//   : BRACKET_LEFT struct_member* BRACKET_RIGHT
ast::StructMemberList ParserImpl::struct_body_decl() {
  auto t = peek();
  if (!t.IsBraceLeft()) {
    set_error(t, "missing { for struct declaration");
    return {};
  }
  next();  // Consume the peek

  t = peek();
  if (t.IsBraceRight()) {
    next();  // Consume the peek
    return {};
  }

  ast::StructMemberList members;
  for (;;) {
    auto mem = struct_member();
    if (has_error())
      return {};
    if (mem == nullptr) {
      set_error(peek(), "invalid struct member");
      return {};
    }

    members.push_back(std::move(mem));

    t = peek();
    if (t.IsBraceRight() || t.IsEof())
      break;
  }

  t = next();
  if (!t.IsBraceRight()) {
    set_error(t, "missing } for struct declaration");
    return {};
  }

  return members;
}

// struct_member
//   : struct_member_decoration_decl variable_ident_decl SEMICOLON
std::unique_ptr<ast::StructMember> ParserImpl::struct_member() {
  auto t = peek();
  auto source = t.source();

  auto decos = struct_member_decoration_decl();
  if (has_error())
    return nullptr;

  std::string name;
  ast::type::Type* type;
  std::tie(name, type) = variable_ident_decl();
  if (has_error())
    return nullptr;
  if (name.empty() || type == nullptr) {
    set_error(peek(), "invalid identifier declaration");
    return nullptr;
  }

  t = next();
  if (!t.IsSemicolon()) {
    set_error(t, "missing ; for struct member");
    return nullptr;
  }

  return std::make_unique<ast::StructMember>(source, name, type,
                                             std::move(decos));
}

// struct_member_decoration_decl
//   :
//   | ATTR_LEFT (struct_member_decoration COMMA)*
//                struct_member_decoration ATTR_RIGHT
ast::StructMemberDecorationList ParserImpl::struct_member_decoration_decl() {
  auto t = peek();
  if (!t.IsAttrLeft())
    return {};

  next();  // Consume the peek

  t = peek();
  if (t.IsAttrRight()) {
    set_error(t, "empty struct member decoration found");
    return {};
  }

  ast::StructMemberDecorationList decos;
  bool found_offset = false;
  for (;;) {
    auto deco = struct_member_decoration();
    if (has_error())
      return {};
    if (deco == nullptr)
      break;

    if (deco->IsOffset()) {
      if (found_offset) {
        set_error(peek(), "duplicate offset decoration found");
        return {};
      }
      found_offset = true;
    }
    decos.push_back(std::move(deco));

    t = next();
    if (!t.IsComma())
      break;
  }

  if (!t.IsAttrRight()) {
    set_error(t, "missing ]] for struct member decoration");
    return {};
  }
  return decos;
}

// struct_member_decoration
//   : OFFSET INT_LITERAL
std::unique_ptr<ast::StructMemberDecoration>
ParserImpl::struct_member_decoration() {
  auto t = peek();
  if (!t.IsOffset())
    return nullptr;

  next();  // Consume the peek

  t = next();
  if (!t.IsSintLiteral()) {
    set_error(t, "invalid value for offset decoration");
    return nullptr;
  }
  int32_t val = t.to_i32();
  if (val < 0) {
    set_error(t, "offset value must be >= 0");
    return nullptr;
  }

  return std::make_unique<ast::StructMemberOffsetDecoration>(val);
}

// function_decl
//   : function_header body_stmt
std::unique_ptr<ast::Function> ParserImpl::function_decl() {
  auto f = function_header();
  if (has_error())
    return nullptr;
  if (f == nullptr)
    return nullptr;

  auto body = body_stmt();
  if (has_error())
    return nullptr;

  f->set_body(std::move(body));
  return f;
}

// function_type_decl
//   : type_decl
//   | VOID
ast::type::Type* ParserImpl::function_type_decl() {
  auto t = peek();
  if (t.IsVoid()) {
    next();  // Consume the peek
    return ctx_.type_mgr().Get(std::make_unique<ast::type::VoidType>());
  }
  return type_decl();
}

// function_header
//   : FN IDENT PAREN_LEFT param_list PAREN_RIGHT ARROW function_type_decl
std::unique_ptr<ast::Function> ParserImpl::function_header() {
  auto t = peek();
  if (!t.IsFn())
    return nullptr;

  auto source = t.source();
  next();  // Consume the peek

  t = next();
  if (!t.IsIdentifier()) {
    set_error(t, "missing identifier for function");
    return nullptr;
  }
  auto name = t.to_str();

  t = next();
  if (!t.IsParenLeft()) {
    set_error(t, "missing ( for function declaration");
    return nullptr;
  }

  auto params = param_list();
  if (has_error())
    return nullptr;

  t = next();
  if (!t.IsParenRight()) {
    set_error(t, "missing ) for function declaration");
    return nullptr;
  }

  t = next();
  if (!t.IsArrow()) {
    set_error(t, "missing -> for function declaration");
    return nullptr;
  }

  auto* type = function_type_decl();
  if (has_error())
    return nullptr;
  if (type == nullptr) {
    set_error(peek(), "unable to determine function return type");
    return nullptr;
  }

  return std::make_unique<ast::Function>(source, name, std::move(params), type);
}

// param_list
//   :
//   | (variable_ident_decl COMMA)* variable_ident_decl
ast::VariableList ParserImpl::param_list() {
  auto t = peek();
  auto source = t.source();

  ast::VariableList ret;

  std::string name;
  ast::type::Type* type;
  std::tie(name, type) = variable_ident_decl();
  if (has_error())
    return {};
  if (name.empty() || type == nullptr)
    return {};

  for (;;) {
    ret.push_back(std::make_unique<ast::Variable>(
        source, name, ast::StorageClass::kNone, type));

    t = peek();
    if (!t.IsComma())
      break;

    source = t.source();
    next();  // Consume the peek

    std::tie(name, type) = variable_ident_decl();
    if (has_error())
      return {};
    if (name.empty() || type == nullptr) {
      set_error(t, "found , but no variable declaration");
      return {};
    }
  }

  return ret;
}

// entry_point_decl
//   : ENTRY_POINT pipeline_stage EQUAL IDENT
//   | ENTRY_POINT pipeline_stage AS STRING_LITERAL EQUAL IDENT
//   | ENTRY_POINT pipeline_stage AS IDENT EQUAL IDENT
std::unique_ptr<ast::EntryPoint> ParserImpl::entry_point_decl() {
  auto t = peek();
  auto source = t.source();
  if (!t.IsEntryPoint())
    return nullptr;

  next();  // Consume the peek

  auto stage = pipeline_stage();
  if (has_error())
    return nullptr;
  if (stage == ast::PipelineStage::kNone) {
    set_error(peek(), "missing pipeline stage for entry point");
    return nullptr;
  }

  t = next();
  std::string name;
  if (t.IsAs()) {
    t = next();
    if (t.IsStringLiteral()) {
      name = t.to_str();
    } else if (t.IsIdentifier()) {
      name = t.to_str();
    } else {
      set_error(t, "invalid name for entry point");
      return nullptr;
    }
    t = next();
  }

  if (!t.IsEqual()) {
    set_error(t, "missing = for entry point");
    return nullptr;
  }

  t = next();
  if (!t.IsIdentifier()) {
    set_error(t, "invalid function name for entry point");
    return nullptr;
  }
  auto fn_name = t.to_str();

  // Set the name to the function name if it isn't provided
  if (name.length() == 0)
    name = fn_name;

  return std::make_unique<ast::EntryPoint>(source, stage, name, fn_name);
}

// pipeline_stage
//   : VERTEX
//   | FRAGMENT
//   | COMPUTE
ast::PipelineStage ParserImpl::pipeline_stage() {
  auto t = peek();
  if (t.IsVertex()) {
    next();  // consume the peek
    return ast::PipelineStage::kVertex;
  }
  if (t.IsFragment()) {
    next();  // consume the peek
    return ast::PipelineStage::kFragment;
  }
  if (t.IsCompute()) {
    next();  // consume the peek
    return ast::PipelineStage::kCompute;
  }
  return ast::PipelineStage::kNone;
}

// body_stmt
//   : BRACKET_LEFT statements BRACKET_RIGHT
std::unique_ptr<ast::BlockStatement> ParserImpl::body_stmt() {
  auto t = peek();
  if (!t.IsBraceLeft())
    return {};

  next();  // Consume the peek

  auto stmts = statements();
  if (has_error())
    return {};

  t = next();
  if (!t.IsBraceRight()) {
    set_error(t, "missing }");
    return {};
  }

  return stmts;
}

// paren_rhs_stmt
//   : PAREN_LEFT logical_or_expression PAREN_RIGHT
std::unique_ptr<ast::Expression> ParserImpl::paren_rhs_stmt() {
  auto t = peek();
  if (!t.IsParenLeft()) {
    set_error(t, "expected (");
    return nullptr;
  }
  next();  // Consume the peek

  auto expr = logical_or_expression();
  if (has_error())
    return nullptr;
  if (expr == nullptr) {
    set_error(peek(), "unable to parse expression");
    return nullptr;
  }

  t = next();
  if (!t.IsParenRight()) {
    set_error(t, "expected )");
    return nullptr;
  }

  return expr;
}

// statements
//   : statement*
std::unique_ptr<ast::BlockStatement> ParserImpl::statements() {
  auto ret = std::make_unique<ast::BlockStatement>();

  for (;;) {
    auto stmt = statement();
    if (has_error())
      return {};
    if (stmt == nullptr)
      break;

    ret->append(std::move(stmt));
  }

  return ret;
}

// statement
//   : SEMICOLON
//   | return_stmt SEMICOLON
//   | if_stmt
//   | switch_stmt
//   | loop_stmt
//   | for_stmt
//   | func_call_stmt SEMICOLON
//   | variable_stmt SEMICOLON
//   | break_stmt SEMICOLON
//   | continue_stmt SEMICOLON
//   | DISCARD SEMICOLON
//   | assignment_stmt SEMICOLON
//   | body_stmt
std::unique_ptr<ast::Statement> ParserImpl::statement() {
  auto t = peek();
  if (t.IsSemicolon()) {
    next();  // Consume the peek
    return statement();
  }

  auto ret_stmt = return_stmt();
  if (has_error())
    return nullptr;
  if (ret_stmt != nullptr) {
    t = next();
    if (!t.IsSemicolon()) {
      set_error(t, "missing ;");
      return nullptr;
    }
    return ret_stmt;
  }

  auto stmt_if = if_stmt();
  if (has_error())
    return nullptr;
  if (stmt_if != nullptr)
    return stmt_if;

  auto sw = switch_stmt();
  if (has_error())
    return nullptr;
  if (sw != nullptr)
    return sw;

  auto loop = loop_stmt();
  if (has_error())
    return nullptr;
  if (loop != nullptr)
    return loop;

  auto stmt_for = for_stmt();
  if (has_error())
    return nullptr;
  if (stmt_for != nullptr)
    return stmt_for;

  auto func = func_call_stmt();
  if (has_error())
    return nullptr;
  if (func != nullptr) {
    t = next();
    if (!t.IsSemicolon()) {
      set_error(t, "missing ;");
      return nullptr;
    }
    return func;
  }

  auto var = variable_stmt();
  if (has_error())
    return nullptr;
  if (var != nullptr) {
    t = next();
    if (!t.IsSemicolon()) {
      set_error(t, "missing ;");
      return nullptr;
    }
    return var;
  }

  auto b = break_stmt();
  if (has_error())
    return nullptr;
  if (b != nullptr) {
    t = next();
    if (!t.IsSemicolon()) {
      set_error(t, "missing ;");
      return nullptr;
    }
    return b;
  }

  auto cont = continue_stmt();
  if (has_error())
    return nullptr;
  if (cont != nullptr) {
    t = next();
    if (!t.IsSemicolon()) {
      set_error(t, "missing ;");
      return nullptr;
    }
    return cont;
  }

  if (t.IsDiscard()) {
    auto source = t.source();
    next();  // Consume the peek

    t = next();
    if (!t.IsSemicolon()) {
      set_error(t, "missing ;");
      return nullptr;
    }
    return std::make_unique<ast::DiscardStatement>(source);
  }

  auto assign = assignment_stmt();
  if (has_error())
    return nullptr;
  if (assign != nullptr) {
    t = next();
    if (!t.IsSemicolon()) {
      set_error(t, "missing ;");
      return nullptr;
    }
    return assign;
  }

  auto body = body_stmt();
  if (has_error())
    return nullptr;
  if (body != nullptr)
    return body;

  return nullptr;
}

// return_stmt
//   : RETURN logical_or_expression?
std::unique_ptr<ast::ReturnStatement> ParserImpl::return_stmt() {
  auto t = peek();
  if (!t.IsReturn())
    return nullptr;

  auto source = t.source();
  next();  // Consume the peek

  std::unique_ptr<ast::Expression> expr = nullptr;
  t = peek();
  if (!t.IsSemicolon()) {
    expr = logical_or_expression();
    if (has_error())
      return nullptr;
  }
  return std::make_unique<ast::ReturnStatement>(source, std::move(expr));
}

// variable_stmt
//   : variable_decl
//   | variable_decl EQUAL logical_or_expression
//   | CONST variable_ident_decl EQUAL logical_or_expression
std::unique_ptr<ast::VariableDeclStatement> ParserImpl::variable_stmt() {
  auto t = peek();
  auto source = t.source();
  if (t.IsConst()) {
    next();  // Consume the peek

    std::string name;
    ast::type::Type* type;
    std::tie(name, type) = variable_ident_decl();
    if (has_error())
      return nullptr;
    if (name.empty() || type == nullptr) {
      set_error(peek(), "unable to parse variable declaration");
      return nullptr;
    }

    t = next();
    if (!t.IsEqual()) {
      set_error(t, "missing = for constant declaration");
      return nullptr;
    }

    auto constructor = logical_or_expression();
    if (has_error())
      return nullptr;
    if (constructor == nullptr) {
      set_error(peek(), "missing constructor for const declaration");
      return nullptr;
    }

    auto var = std::make_unique<ast::Variable>(source, name,
                                               ast::StorageClass::kNone, type);
    var->set_is_const(true);
    var->set_constructor(std::move(constructor));

    return std::make_unique<ast::VariableDeclStatement>(source, std::move(var));
  }

  auto var = variable_decl();
  if (has_error())
    return nullptr;
  if (var == nullptr)
    return nullptr;

  t = peek();
  if (t.IsEqual()) {
    next();  // Consume the peek
    auto constructor = logical_or_expression();
    if (has_error())
      return nullptr;
    if (constructor == nullptr) {
      set_error(peek(), "missing constructor for variable declaration");
      return nullptr;
    }
    var->set_constructor(std::move(constructor));
  }

  return std::make_unique<ast::VariableDeclStatement>(source, std::move(var));
}

// if_stmt
//   : IF paren_rhs_stmt body_stmt elseif_stmt? else_stmt?
std::unique_ptr<ast::IfStatement> ParserImpl::if_stmt() {
  auto t = peek();
  if (!t.IsIf())
    return nullptr;

  auto source = t.source();
  next();  // Consume the peek

  auto condition = paren_rhs_stmt();
  if (has_error())
    return nullptr;
  if (condition == nullptr) {
    set_error(peek(), "unable to parse if condition");
    return nullptr;
  }

  t = peek();
  if (!t.IsBraceLeft()) {
    set_error(t, "missing {");
    return nullptr;
  }

  auto body = body_stmt();
  if (has_error())
    return nullptr;

  auto elseif = elseif_stmt();
  if (has_error())
    return nullptr;

  auto el = else_stmt();
  if (has_error())
    return nullptr;

  auto stmt = std::make_unique<ast::IfStatement>(source, std::move(condition),
                                                 std::move(body));
  if (el != nullptr) {
    elseif.push_back(std::move(el));
  }
  stmt->set_else_statements(std::move(elseif));

  return stmt;
}

// elseif_stmt
//   : ELSE_IF paren_rhs_stmt body_stmt elseif_stmt?
ast::ElseStatementList ParserImpl::elseif_stmt() {
  auto t = peek();
  if (!t.IsElseIf())
    return {};

  ast::ElseStatementList ret;
  for (;;) {
    auto source = t.source();
    next();  // Consume the peek

    auto condition = paren_rhs_stmt();
    if (has_error())
      return {};
    if (condition == nullptr) {
      set_error(peek(), "unable to parse condition expression");
      return {};
    }

    t = peek();
    if (!t.IsBraceLeft()) {
      set_error(t, "missing {");
      return {};
    }

    auto body = body_stmt();
    if (has_error())
      return {};

    ret.push_back(std::make_unique<ast::ElseStatement>(
        source, std::move(condition), std::move(body)));

    t = peek();
    if (!t.IsElseIf())
      break;
  }

  return ret;
}

// else_stmt
//   : ELSE body_stmt
std::unique_ptr<ast::ElseStatement> ParserImpl::else_stmt() {
  auto t = peek();
  if (!t.IsElse())
    return nullptr;

  auto source = t.source();
  next();  // Consume the peek

  t = peek();
  if (!t.IsBraceLeft()) {
    set_error(t, "missing {");
    return nullptr;
  }

  auto body = body_stmt();
  if (has_error())
    return nullptr;

  return std::make_unique<ast::ElseStatement>(source, std::move(body));
}

// switch_stmt
//   : SWITCH paren_rhs_stmt BRACKET_LEFT switch_body+ BRACKET_RIGHT
std::unique_ptr<ast::SwitchStatement> ParserImpl::switch_stmt() {
  auto t = peek();
  if (!t.IsSwitch())
    return nullptr;

  auto source = t.source();
  next();  // Consume the peek

  auto condition = paren_rhs_stmt();
  if (has_error())
    return nullptr;
  if (condition == nullptr) {
    set_error(peek(), "unable to parse switch expression");
    return nullptr;
  }

  t = next();
  if (!t.IsBraceLeft()) {
    set_error(t, "missing { for switch statement");
    return nullptr;
  }

  ast::CaseStatementList body;
  for (;;) {
    auto stmt = switch_body();
    if (has_error())
      return nullptr;
    if (stmt == nullptr)
      break;

    body.push_back(std::move(stmt));
  }

  t = next();
  if (!t.IsBraceRight()) {
    set_error(t, "missing } for switch statement");
    return nullptr;
  }
  return std::make_unique<ast::SwitchStatement>(source, std::move(condition),
                                                std::move(body));
}

// switch_body
//   : CASE case_selectors COLON BRACKET_LEFT case_body BRACKET_RIGHT
//   | DEFAULT COLON BRACKET_LEFT case_body BRACKET_RIGHT
std::unique_ptr<ast::CaseStatement> ParserImpl::switch_body() {
  auto t = peek();
  if (!t.IsCase() && !t.IsDefault())
    return nullptr;

  auto source = t.source();
  next();  // Consume the peek

  auto stmt = std::make_unique<ast::CaseStatement>();
  stmt->set_source(source);
  if (t.IsCase()) {
    auto selectors = case_selectors();
    if (has_error())
      return nullptr;
    if (selectors.empty()) {
      set_error(peek(), "unable to parse case selectors");
      return nullptr;
    }
    stmt->set_selectors(std::move(selectors));
  }

  t = next();
  if (!t.IsColon()) {
    set_error(t, "missing : for case statement");
    return nullptr;
  }

  t = next();
  if (!t.IsBraceLeft()) {
    set_error(t, "missing { for case statement");
    return nullptr;
  }

  auto body = case_body();
  if (has_error())
    return nullptr;

  stmt->set_body(std::move(body));

  t = next();
  if (!t.IsBraceRight()) {
    set_error(t, "missing } for case statement");
    return nullptr;
  }

  return stmt;
}

// case_selectors
//   : const_literal (COMMA const_literal)*
ast::CaseSelectorList ParserImpl::case_selectors() {
  ast::CaseSelectorList selectors;

  for (;;) {
    auto t = peek();
    auto cond = const_literal();
    if (has_error())
      return {};
    if (cond == nullptr)
      break;
    if (!cond->IsInt()) {
      set_error(t, "invalid case selector must be an integer value");
      return {};
    }

    std::unique_ptr<ast::IntLiteral> selector(cond.release()->AsInt());
    selectors.push_back(std::move(selector));
  }

  return selectors;
}

// case_body
//   :
//   | statement case_body
//   | FALLTHROUGH SEMICOLON
std::unique_ptr<ast::BlockStatement> ParserImpl::case_body() {
  auto ret = std::make_unique<ast::BlockStatement>();
  for (;;) {
    auto t = peek();
    if (t.IsFallthrough()) {
      auto source = t.source();
      next();  // Consume the peek

      t = next();
      if (!t.IsSemicolon()) {
        set_error(t, "missing ;");
        return {};
      }

      ret->append(std::make_unique<ast::FallthroughStatement>(source));
      break;
    }

    auto stmt = statement();
    if (has_error())
      return {};
    if (stmt == nullptr)
      break;

    ret->append(std::move(stmt));
  }

  return ret;
}

// loop_stmt
//   : LOOP BRACKET_LEFT statements continuing_stmt? BRACKET_RIGHT
std::unique_ptr<ast::LoopStatement> ParserImpl::loop_stmt() {
  auto t = peek();
  if (!t.IsLoop())
    return nullptr;

  auto source = t.source();
  next();  // Consume the peek

  t = next();
  if (!t.IsBraceLeft()) {
    set_error(t, "missing { for loop");
    return nullptr;
  }

  auto body = statements();
  if (has_error())
    return nullptr;

  auto continuing = continuing_stmt();
  if (has_error())
    return nullptr;

  t = next();
  if (!t.IsBraceRight()) {
    set_error(t, "missing } for loop");
    return nullptr;
  }

  return std::make_unique<ast::LoopStatement>(source, std::move(body),
                                              std::move(continuing));
}

ForHeader::ForHeader(std::unique_ptr<ast::Statement> init,
                     std::unique_ptr<ast::Expression> cond,
                     std::unique_ptr<ast::Statement> cont)
    : initializer(std::move(init)),
      condition(std::move(cond)),
      continuing(std::move(cont)) {}

ForHeader::~ForHeader() = default;

// for_header
//   : (variable_stmt | assignment_stmt | func_call_stmt)?
//   SEMICOLON
//      logical_or_expression? SEMICOLON
//      (assignment_stmt | func_call_stmt)?
std::unique_ptr<ForHeader> ParserImpl::for_header() {
  std::unique_ptr<ast::Statement> initializer = nullptr;
  if (initializer == nullptr) {
    initializer = func_call_stmt();
    if (has_error()) {
      return nullptr;
    }
  }
  if (initializer == nullptr) {
    initializer = variable_stmt();
    if (has_error()) {
      return nullptr;
    }
  }
  if (initializer == nullptr) {
    initializer = assignment_stmt();
    if (has_error()) {
      return nullptr;
    }
  }

  auto t = next();
  if (!t.IsSemicolon()) {
    set_error(t, "missing ';' after initializer in for loop");
    return nullptr;
  }

  auto condition = logical_or_expression();
  if (has_error()) {
    return nullptr;
  }

  t = next();
  if (!t.IsSemicolon()) {
    set_error(t, "missing ';' after condition in for loop");
    return nullptr;
  }

  std::unique_ptr<ast::Statement> continuing = nullptr;
  if (continuing == nullptr) {
    continuing = func_call_stmt();
    if (has_error()) {
      return nullptr;
    }
  }
  if (continuing == nullptr) {
    continuing = assignment_stmt();
    if (has_error()) {
      return nullptr;
    }
  }

  return std::make_unique<ForHeader>(
      std::move(initializer), std::move(condition), std::move(continuing));
}

// for_statement
//   : FOR PAREN_LEFT for_header PAREN_RIGHT BRACE_LEFT statements BRACE_RIGHT
std::unique_ptr<ast::Statement> ParserImpl::for_stmt() {
  auto t = peek();
  if (!t.IsFor())
    return nullptr;

  auto source = t.source();
  next();  // Consume the peek

  t = next();
  if (!t.IsParenLeft()) {
    set_error(t, "missing for loop (");
    return nullptr;
  }

  auto header = for_header();
  if (has_error())
    return nullptr;

  t = next();
  if (!t.IsParenRight()) {
    set_error(t, "missing for loop )");
    return nullptr;
  }

  t = next();
  if (!t.IsBraceLeft()) {
    set_error(t, "missing for loop {");
    return nullptr;
  }

  auto body = statements();
  if (has_error())
    return nullptr;

  t = next();
  if (!t.IsBraceRight()) {
    set_error(t, "missing for loop }");
    return nullptr;
  }

  // The for statement is a syntactic sugar on top of the loop statement.
  // We create corresponding nodes in ast with the exact same behaviour
  // as we would expect from the loop statement.

  if (header->condition != nullptr) {
    // !condition
    auto not_condition = std::make_unique<ast::UnaryOpExpression>(
        header->condition->source(), ast::UnaryOp::kNot,
        std::move(header->condition));
    // { break; }
    auto break_stmt =
        std::make_unique<ast::BreakStatement>(not_condition->source());
    auto break_body =
        std::make_unique<ast::BlockStatement>(not_condition->source());
    break_body->append(std::move(break_stmt));
    // if (!condition) { break; }
    auto break_if_not_condition = std::make_unique<ast::IfStatement>(
        not_condition->source(), std::move(not_condition),
        std::move(break_body));
    body->insert(0, std::move(break_if_not_condition));
  }

  std::unique_ptr<ast::BlockStatement> continuing_body = nullptr;
  if (header->continuing != nullptr) {
    continuing_body =
        std::make_unique<ast::BlockStatement>(header->continuing->source());
    continuing_body->append(std::move(header->continuing));
  }

  auto loop = std::make_unique<ast::LoopStatement>(source, std::move(body),
                                                   std::move(continuing_body));

  if (header->initializer != nullptr) {
    auto result = std::make_unique<ast::BlockStatement>(source);
    result->append(std::move(header->initializer));
    result->append(std::move(loop));
    return result;
  }

  return loop;
}

// func_call_stmt
//    : IDENT PAREN_LEFT argument_expression_list* PAREN_RIGHT
std::unique_ptr<ast::CallStatement> ParserImpl::func_call_stmt() {
  auto t = peek();
  auto t2 = peek(1);
  if (!t.IsIdentifier() || !t2.IsParenLeft())
    return nullptr;

  auto source = t.source();

  next();  // Consume the peek
  next();  // Consume the 2nd peek

  auto name = t.to_str();

  t = peek();
  ast::ExpressionList params;
  if (!t.IsParenRight() && !t.IsEof()) {
    params = argument_expression_list();
    if (has_error())
      return nullptr;
  }

  t = next();
  if (!t.IsParenRight()) {
    set_error(t, "missing ) for call statement");
    return nullptr;
  }

  return std::make_unique<ast::CallStatement>(
      std::make_unique<ast::CallExpression>(
          source, std::make_unique<ast::IdentifierExpression>(name),
          std::move(params)));
}

// break_stmt
//   : BREAK
std::unique_ptr<ast::BreakStatement> ParserImpl::break_stmt() {
  auto t = peek();
  if (!t.IsBreak())
    return nullptr;

  next();  // Consume the peek
  return std::make_unique<ast::BreakStatement>(t.source());
}

// continue_stmt
//   : CONTINUE
std::unique_ptr<ast::ContinueStatement> ParserImpl::continue_stmt() {
  auto t = peek();
  if (!t.IsContinue())
    return nullptr;

  next();  // Consume the peek
  return std::make_unique<ast::ContinueStatement>(t.source());
}

// continuing_stmt
//   : CONTINUING body_stmt
std::unique_ptr<ast::BlockStatement> ParserImpl::continuing_stmt() {
  auto t = peek();
  if (!t.IsContinuing()) {
    return std::make_unique<ast::BlockStatement>();
  }

  next();  // Consume the peek
  return body_stmt();
}

// primary_expression
//   : (IDENT NAMESPACE)* IDENT
//   | type_decl PAREN_LEFT argument_expression_list* PAREN_RIGHT
//   | const_literal
//   | paren_rhs_stmt
//   | CAST LESS_THAN type_decl GREATER_THAN paren_rhs_stmt
//   | AS LESS_THAN type_decl GREATER_THAN paren_rhs_stmt
std::unique_ptr<ast::Expression> ParserImpl::primary_expression() {
  auto t = peek();
  auto source = t.source();

  auto lit = const_literal();
  if (has_error())
    return nullptr;
  if (lit != nullptr) {
    return std::make_unique<ast::ScalarConstructorExpression>(source,
                                                              std::move(lit));
  }

  t = peek();
  if (t.IsParenLeft()) {
    auto paren = paren_rhs_stmt();
    if (has_error())
      return nullptr;

    return paren;
  }

  if (t.IsCast() || t.IsAs()) {
    auto src = t;

    next();  // Consume the peek

    t = next();
    if (!t.IsLessThan()) {
      set_error(t, "missing < for " + src.to_name() + " expression");
      return nullptr;
    }

    auto* type = type_decl();
    if (has_error())
      return nullptr;
    if (type == nullptr) {
      set_error(peek(), "missing type for " + src.to_name() + " expression");
      return nullptr;
    }

    t = next();
    if (!t.IsGreaterThan()) {
      set_error(t, "missing > for " + src.to_name() + " expression");
      return nullptr;
    }

    auto params = paren_rhs_stmt();
    if (has_error())
      return nullptr;
    if (params == nullptr) {
      set_error(peek(), "unable to parse parameters");
      return nullptr;
    }

    if (src.IsCast()) {
      return std::make_unique<ast::CastExpression>(source, type,
                                                   std::move(params));
    } else {
      return std::make_unique<ast::AsExpression>(source, type,
                                                 std::move(params));
    }

  } else if (t.IsIdentifier()) {
    next();  // Consume the peek

    std::vector<std::string> ident;
    ident.push_back(t.to_str());
    for (;;) {
      t = peek();
      if (!t.IsNamespace())
        break;

      next();  // Consume the peek
      t = next();
      if (!t.IsIdentifier()) {
        set_error(t, "identifier expected");
        return nullptr;
      }

      ident.push_back(t.to_str());
    }
    return std::make_unique<ast::IdentifierExpression>(source,
                                                       std::move(ident));
  }

  auto* type = type_decl();
  if (has_error())
    return nullptr;
  if (type != nullptr) {
    t = next();
    if (!t.IsParenLeft()) {
      set_error(t, "missing ( for type constructor");
      return nullptr;
    }

    t = peek();
    ast::ExpressionList params;
    if (!t.IsParenRight() && !t.IsEof()) {
      params = argument_expression_list();
      if (has_error())
        return nullptr;
    }

    t = next();
    if (!t.IsParenRight()) {
      set_error(t, "missing ) for type constructor");
      return nullptr;
    }
    return std::make_unique<ast::TypeConstructorExpression>(source, type,
                                                            std::move(params));
  }
  return nullptr;
}

// postfix_expr
//   :
//   | BRACE_LEFT logical_or_expression BRACE_RIGHT postfix_expr
//   | PAREN_LEFT argument_expression_list* PAREN_RIGHT postfix_expr
//   | PERIOD IDENTIFIER postfix_expr
std::unique_ptr<ast::Expression> ParserImpl::postfix_expr(
    std::unique_ptr<ast::Expression> prefix) {
  std::unique_ptr<ast::Expression> expr = nullptr;

  auto t = peek();
  auto source = t.source();
  if (t.IsBracketLeft()) {
    next();  // Consume the peek

    auto param = logical_or_expression();
    if (has_error())
      return nullptr;
    if (param == nullptr) {
      set_error(peek(), "unable to parse expression inside []");
      return nullptr;
    }

    t = next();
    if (!t.IsBracketRight()) {
      set_error(t, "missing ] for array accessor");
      return nullptr;
    }
    expr = std::make_unique<ast::ArrayAccessorExpression>(
        source, std::move(prefix), std::move(param));

  } else if (t.IsParenLeft()) {
    next();  // Consume the peek

    t = peek();
    ast::ExpressionList params;
    if (!t.IsParenRight() && !t.IsEof()) {
      params = argument_expression_list();
      if (has_error())
        return nullptr;
    }

    t = next();
    if (!t.IsParenRight()) {
      set_error(t, "missing ) for call expression");
      return nullptr;
    }
    expr = std::make_unique<ast::CallExpression>(source, std::move(prefix),
                                                 std::move(params));
  } else if (t.IsPeriod()) {
    next();  // Consume the peek

    t = next();
    if (!t.IsIdentifier()) {
      set_error(t, "missing identifier for member accessor");
      return nullptr;
    }

    expr = std::make_unique<ast::MemberAccessorExpression>(
        source, std::move(prefix),
        std::make_unique<ast::IdentifierExpression>(t.source(), t.to_str()));
  } else {
    return prefix;
  }
  return postfix_expr(std::move(expr));
}

// postfix_expression
//   : primary_expression postfix_expr
std::unique_ptr<ast::Expression> ParserImpl::postfix_expression() {
  auto prefix = primary_expression();
  if (has_error())
    return nullptr;
  if (prefix == nullptr)
    return nullptr;

  return postfix_expr(std::move(prefix));
}

// argument_expression_list
//   : (logical_or_expression COMMA)* logical_or_expression
ast::ExpressionList ParserImpl::argument_expression_list() {
  auto arg = logical_or_expression();
  if (has_error())
    return {};
  if (arg == nullptr) {
    set_error(peek(), "unable to parse argument expression");
    return {};
  }

  ast::ExpressionList ret;
  ret.push_back(std::move(arg));

  for (;;) {
    auto t = peek();
    if (!t.IsComma())
      break;

    next();  // Consume the peek

    arg = logical_or_expression();
    if (has_error())
      return {};
    if (arg == nullptr) {
      set_error(peek(), "unable to parse argument expression after comma");
      return {};
    }
    ret.push_back(std::move(arg));
  }
  return ret;
}

// unary_expression
//   : postfix_expression
//   | MINUS unary_expression
//   | BANG unary_expression
std::unique_ptr<ast::Expression> ParserImpl::unary_expression() {
  auto t = peek();
  auto source = t.source();
  if (t.IsMinus() || t.IsBang()) {
    auto name = t.to_name();

    next();  // Consume the peek

    auto op = ast::UnaryOp::kNegation;
    if (t.IsBang())
      op = ast::UnaryOp::kNot;

    auto expr = unary_expression();
    if (has_error())
      return nullptr;
    if (expr == nullptr) {
      set_error(peek(),
                "unable to parse right side of " + name + " expression");
      return nullptr;
    }
    return std::make_unique<ast::UnaryOpExpression>(source, op,
                                                    std::move(expr));
  }
  return postfix_expression();
}

// multiplicative_expr
//   :
//   | STAR unary_expression multiplicative_expr
//   | FORWARD_SLASH unary_expression multiplicative_expr
//   | MODULO unary_expression multiplicative_expr
std::unique_ptr<ast::Expression> ParserImpl::multiplicative_expr(
    std::unique_ptr<ast::Expression> lhs) {
  auto t = peek();

  ast::BinaryOp op = ast::BinaryOp::kNone;
  if (t.IsStar())
    op = ast::BinaryOp::kMultiply;
  else if (t.IsForwardSlash())
    op = ast::BinaryOp::kDivide;
  else if (t.IsMod())
    op = ast::BinaryOp::kModulo;
  else
    return lhs;

  auto source = t.source();
  auto name = t.to_name();
  next();  // Consume the peek

  auto rhs = unary_expression();
  if (has_error())
    return nullptr;
  if (rhs == nullptr) {
    set_error(peek(), "unable to parse right side of " + name + " expression");
    return nullptr;
  }
  return multiplicative_expr(std::make_unique<ast::BinaryExpression>(
      source, op, std::move(lhs), std::move(rhs)));
}

// multiplicative_expression
//   : unary_expression multiplicative_expr
std::unique_ptr<ast::Expression> ParserImpl::multiplicative_expression() {
  auto lhs = unary_expression();
  if (has_error())
    return nullptr;
  if (lhs == nullptr)
    return nullptr;

  return multiplicative_expr(std::move(lhs));
}

// additive_expr
//   :
//   | PLUS multiplicative_expression additive_expr
//   | MINUS multiplicative_expression additive_expr
std::unique_ptr<ast::Expression> ParserImpl::additive_expr(
    std::unique_ptr<ast::Expression> lhs) {
  auto t = peek();

  ast::BinaryOp op = ast::BinaryOp::kNone;
  if (t.IsPlus())
    op = ast::BinaryOp::kAdd;
  else if (t.IsMinus())
    op = ast::BinaryOp::kSubtract;
  else
    return lhs;

  auto source = t.source();
  next();  // Consume the peek

  auto rhs = multiplicative_expression();
  if (has_error())
    return nullptr;
  if (rhs == nullptr) {
    set_error(peek(), "unable to parse right side of + expression");
    return nullptr;
  }
  return additive_expr(std::make_unique<ast::BinaryExpression>(
      source, op, std::move(lhs), std::move(rhs)));
}

// additive_expression
//   : multiplicative_expression additive_expr
std::unique_ptr<ast::Expression> ParserImpl::additive_expression() {
  auto lhs = multiplicative_expression();
  if (has_error())
    return nullptr;
  if (lhs == nullptr)
    return nullptr;

  return additive_expr(std::move(lhs));
}

// shift_expr
//   :
//   | LESS_THAN LESS_THAN additive_expression shift_expr
//   | GREATER_THAN GREATER_THAN additive_expression shift_expr
std::unique_ptr<ast::Expression> ParserImpl::shift_expr(
    std::unique_ptr<ast::Expression> lhs) {
  auto t = peek();
  auto source = t.source();
  auto t2 = peek(1);

  auto* name = "";
  ast::BinaryOp op = ast::BinaryOp::kNone;
  if (t.IsLessThan() && t2.IsLessThan()) {
    next();  // Consume the t peek
    next();  // Consume the t2 peek
    op = ast::BinaryOp::kShiftLeft;
    name = "<<";
  } else if (t.IsGreaterThan() && t2.IsGreaterThan()) {
    next();  // Consume the t peek
    next();  // Consume the t2 peek
    op = ast::BinaryOp::kShiftRight;
    name = ">>";
  } else {
    return lhs;
  }

  auto rhs = additive_expression();
  if (has_error())
    return nullptr;
  if (rhs == nullptr) {
    set_error(peek(), std::string("unable to parse right side of ") + name +
                          " expression");
    return nullptr;
  }
  return shift_expr(std::make_unique<ast::BinaryExpression>(
      source, op, std::move(lhs), std::move(rhs)));
}

// shift_expression
//   : additive_expression shift_expr
std::unique_ptr<ast::Expression> ParserImpl::shift_expression() {
  auto lhs = additive_expression();
  if (has_error())
    return nullptr;
  if (lhs == nullptr)
    return nullptr;

  return shift_expr(std::move(lhs));
}

// relational_expr
//   :
//   | LESS_THAN shift_expression relational_expr
//   | GREATER_THAN shift_expression relational_expr
//   | LESS_THAN_EQUAL shift_expression relational_expr
//   | GREATER_THAN_EQUAL shift_expression relational_expr
std::unique_ptr<ast::Expression> ParserImpl::relational_expr(
    std::unique_ptr<ast::Expression> lhs) {
  auto t = peek();
  ast::BinaryOp op = ast::BinaryOp::kNone;
  if (t.IsLessThan())
    op = ast::BinaryOp::kLessThan;
  else if (t.IsGreaterThan())
    op = ast::BinaryOp::kGreaterThan;
  else if (t.IsLessThanEqual())
    op = ast::BinaryOp::kLessThanEqual;
  else if (t.IsGreaterThanEqual())
    op = ast::BinaryOp::kGreaterThanEqual;
  else
    return lhs;

  auto source = t.source();
  auto name = t.to_name();
  next();  // Consume the peek

  auto rhs = shift_expression();
  if (has_error())
    return nullptr;
  if (rhs == nullptr) {
    set_error(peek(), "unable to parse right side of " + name + " expression");
    return nullptr;
  }
  return relational_expr(std::make_unique<ast::BinaryExpression>(
      source, op, std::move(lhs), std::move(rhs)));
}

// relational_expression
//   : shift_expression relational_expr
std::unique_ptr<ast::Expression> ParserImpl::relational_expression() {
  auto lhs = shift_expression();
  if (has_error())
    return nullptr;
  if (lhs == nullptr)
    return nullptr;

  return relational_expr(std::move(lhs));
}

// equality_expr
//   :
//   | EQUAL_EQUAL relational_expression equality_expr
//   | NOT_EQUAL relational_expression equality_expr
std::unique_ptr<ast::Expression> ParserImpl::equality_expr(
    std::unique_ptr<ast::Expression> lhs) {
  auto t = peek();
  ast::BinaryOp op = ast::BinaryOp::kNone;
  if (t.IsEqualEqual())
    op = ast::BinaryOp::kEqual;
  else if (t.IsNotEqual())
    op = ast::BinaryOp::kNotEqual;
  else
    return lhs;

  auto source = t.source();
  auto name = t.to_name();
  next();  // Consume the peek

  auto rhs = relational_expression();
  if (has_error())
    return nullptr;
  if (rhs == nullptr) {
    set_error(peek(), "unable to parse right side of " + name + " expression");
    return nullptr;
  }
  return equality_expr(std::make_unique<ast::BinaryExpression>(
      source, op, std::move(lhs), std::move(rhs)));
}

// equality_expression
//   : relational_expression equality_expr
std::unique_ptr<ast::Expression> ParserImpl::equality_expression() {
  auto lhs = relational_expression();
  if (has_error())
    return nullptr;
  if (lhs == nullptr)
    return nullptr;

  return equality_expr(std::move(lhs));
}

// and_expr
//   :
//   | AND equality_expression and_expr
std::unique_ptr<ast::Expression> ParserImpl::and_expr(
    std::unique_ptr<ast::Expression> lhs) {
  auto t = peek();
  if (!t.IsAnd())
    return lhs;

  auto source = t.source();
  next();  // Consume the peek

  auto rhs = equality_expression();
  if (has_error())
    return nullptr;
  if (rhs == nullptr) {
    set_error(peek(), "unable to parse right side of & expression");
    return nullptr;
  }
  return and_expr(std::make_unique<ast::BinaryExpression>(
      source, ast::BinaryOp::kAnd, std::move(lhs), std::move(rhs)));
}

// and_expression
//   : equality_expression and_expr
std::unique_ptr<ast::Expression> ParserImpl::and_expression() {
  auto lhs = equality_expression();
  if (has_error())
    return nullptr;
  if (lhs == nullptr)
    return nullptr;

  return and_expr(std::move(lhs));
}

// exclusive_or_expr
//   :
//   | XOR and_expression exclusive_or_expr
std::unique_ptr<ast::Expression> ParserImpl::exclusive_or_expr(
    std::unique_ptr<ast::Expression> lhs) {
  auto t = peek();
  if (!t.IsXor())
    return lhs;

  auto source = t.source();
  next();  // Consume the peek

  auto rhs = and_expression();
  if (has_error())
    return nullptr;
  if (rhs == nullptr) {
    set_error(peek(), "unable to parse right side of ^ expression");
    return nullptr;
  }
  return exclusive_or_expr(std::make_unique<ast::BinaryExpression>(
      source, ast::BinaryOp::kXor, std::move(lhs), std::move(rhs)));
}

// exclusive_or_expression
//   : and_expression exclusive_or_expr
std::unique_ptr<ast::Expression> ParserImpl::exclusive_or_expression() {
  auto lhs = and_expression();
  if (has_error())
    return nullptr;
  if (lhs == nullptr)
    return nullptr;

  return exclusive_or_expr(std::move(lhs));
}

// inclusive_or_expr
//   :
//   | OR exclusive_or_expression inclusive_or_expr
std::unique_ptr<ast::Expression> ParserImpl::inclusive_or_expr(
    std::unique_ptr<ast::Expression> lhs) {
  auto t = peek();
  if (!t.IsOr())
    return lhs;

  auto source = t.source();
  next();  // Consume the peek

  auto rhs = exclusive_or_expression();
  if (has_error())
    return nullptr;
  if (rhs == nullptr) {
    set_error(peek(), "unable to parse right side of | expression");
    return nullptr;
  }
  return inclusive_or_expr(std::make_unique<ast::BinaryExpression>(
      source, ast::BinaryOp::kOr, std::move(lhs), std::move(rhs)));
}

// inclusive_or_expression
//   : exclusive_or_expression inclusive_or_expr
std::unique_ptr<ast::Expression> ParserImpl::inclusive_or_expression() {
  auto lhs = exclusive_or_expression();
  if (has_error())
    return nullptr;
  if (lhs == nullptr)
    return nullptr;

  return inclusive_or_expr(std::move(lhs));
}

// logical_and_expr
//   :
//   | AND_AND inclusive_or_expression logical_and_expr
std::unique_ptr<ast::Expression> ParserImpl::logical_and_expr(
    std::unique_ptr<ast::Expression> lhs) {
  auto t = peek();
  if (!t.IsAndAnd())
    return lhs;

  auto source = t.source();
  next();  // Consume the peek

  auto rhs = inclusive_or_expression();
  if (has_error())
    return nullptr;
  if (rhs == nullptr) {
    set_error(peek(), "unable to parse right side of && expression");
    return nullptr;
  }
  return logical_and_expr(std::make_unique<ast::BinaryExpression>(
      source, ast::BinaryOp::kLogicalAnd, std::move(lhs), std::move(rhs)));
}

// logical_and_expression
//   : inclusive_or_expression logical_and_expr
std::unique_ptr<ast::Expression> ParserImpl::logical_and_expression() {
  auto lhs = inclusive_or_expression();
  if (has_error())
    return nullptr;
  if (lhs == nullptr)
    return nullptr;

  return logical_and_expr(std::move(lhs));
}

// logical_or_expr
//   :
//   | OR_OR logical_and_expression logical_or_expr
std::unique_ptr<ast::Expression> ParserImpl::logical_or_expr(
    std::unique_ptr<ast::Expression> lhs) {
  auto t = peek();
  if (!t.IsOrOr())
    return lhs;

  auto source = t.source();
  next();  // Consume the peek

  auto rhs = logical_and_expression();
  if (has_error())
    return nullptr;
  if (rhs == nullptr) {
    set_error(peek(), "unable to parse right side of || expression");
    return nullptr;
  }
  return logical_or_expr(std::make_unique<ast::BinaryExpression>(
      source, ast::BinaryOp::kLogicalOr, std::move(lhs), std::move(rhs)));
}

// logical_or_expression
//   : logical_and_expression logical_or_expr
std::unique_ptr<ast::Expression> ParserImpl::logical_or_expression() {
  auto lhs = logical_and_expression();
  if (has_error())
    return nullptr;
  if (lhs == nullptr)
    return nullptr;

  return logical_or_expr(std::move(lhs));
}

// assignment_stmt
//   : unary_expression EQUAL logical_or_expression
std::unique_ptr<ast::AssignmentStatement> ParserImpl::assignment_stmt() {
  auto t = peek();
  auto source = t.source();

  auto lhs = unary_expression();
  if (has_error())
    return nullptr;
  if (lhs == nullptr)
    return nullptr;

  t = next();
  if (!t.IsEqual()) {
    set_error(t, "missing = for assignment");
    return nullptr;
  }

  auto rhs = logical_or_expression();
  if (has_error())
    return nullptr;
  if (rhs == nullptr) {
    set_error(peek(), "unable to parse right side of assignment");
    return nullptr;
  }

  return std::make_unique<ast::AssignmentStatement>(source, std::move(lhs),
                                                    std::move(rhs));
}

// const_literal
//   : INT_LITERAL
//   | UINT_LITERAL
//   | FLOAT_LITERAL
//   | TRUE
//   | FALSE
std::unique_ptr<ast::Literal> ParserImpl::const_literal() {
  auto t = peek();
  if (t.IsTrue()) {
    next();  // Consume the peek

    auto* type = ctx_.type_mgr().Get(std::make_unique<ast::type::BoolType>());
    if (!type) {
      return nullptr;
    }
    return std::make_unique<ast::BoolLiteral>(type, true);
  }
  if (t.IsFalse()) {
    next();  // Consume the peek
    auto* type = ctx_.type_mgr().Get(std::make_unique<ast::type::BoolType>());
    if (!type) {
      return nullptr;
    }
    return std::make_unique<ast::BoolLiteral>(type, false);
  }
  if (t.IsSintLiteral()) {
    next();  // Consume the peek
    auto* type = ctx_.type_mgr().Get(std::make_unique<ast::type::I32Type>());
    if (!type) {
      return nullptr;
    }
    return std::make_unique<ast::SintLiteral>(type, t.to_i32());
  }
  if (t.IsUintLiteral()) {
    next();  // Consume the peek
    auto* type = ctx_.type_mgr().Get(std::make_unique<ast::type::U32Type>());
    if (!type) {
      return nullptr;
    }
    return std::make_unique<ast::UintLiteral>(type, t.to_u32());
  }
  if (t.IsFloatLiteral()) {
    next();  // Consume the peek
    auto* type = ctx_.type_mgr().Get(std::make_unique<ast::type::F32Type>());
    if (!type) {
      return nullptr;
    }
    return std::make_unique<ast::FloatLiteral>(type, t.to_f32());
  }
  return nullptr;
}

// const_expr
//   : type_decl PAREN_LEFT (const_expr COMMA)? const_expr PAREN_RIGHT
//   | const_literal
std::unique_ptr<ast::ConstructorExpression> ParserImpl::const_expr() {
  return const_expr_internal(0);
}

std::unique_ptr<ast::ConstructorExpression> ParserImpl::const_expr_internal(
    uint32_t depth) {
  auto t = peek();

  if (depth > kMaxConstExprDepth) {
    set_error(t, "max const_expr depth reached");
    return nullptr;
  }

  auto source = t.source();

  auto* type = type_decl();
  if (type != nullptr) {
    t = next();
    if (!t.IsParenLeft()) {
      set_error(t, "missing ( for type constructor");
      return nullptr;
    }

    ast::ExpressionList params;
    auto param = const_expr_internal(depth + 1);
    if (has_error())
      return nullptr;
    if (param == nullptr) {
      set_error(peek(), "unable to parse constant expression");
      return nullptr;
    }
    params.push_back(std::move(param));
    for (;;) {
      t = peek();
      if (!t.IsComma())
        break;

      next();  // Consume the peek

      param = const_expr_internal(depth + 1);
      if (has_error())
        return nullptr;
      if (param == nullptr) {
        set_error(peek(), "unable to parse constant expression");
        return nullptr;
      }
      params.push_back(std::move(param));
    }

    t = next();
    if (!t.IsParenRight()) {
      set_error(t, "missing ) for type constructor");
      return nullptr;
    }
    return std::make_unique<ast::TypeConstructorExpression>(source, type,
                                                            std::move(params));
  }

  auto lit = const_literal();
  if (has_error())
    return nullptr;
  if (lit == nullptr) {
    set_error(peek(), "unable to parse const literal");
    return nullptr;
  }
  return std::make_unique<ast::ScalarConstructorExpression>(source,
                                                            std::move(lit));
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
