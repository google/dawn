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
#include "src/ast/binary_expression.h"
#include "src/ast/binding_decoration.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/bool_literal.h"
#include "src/ast/break_statement.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/call_expression.h"
#include "src/ast/case_statement.h"
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
#include "src/ast/stage_decoration.h"
#include "src/ast/stride_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/depth_texture_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/multisampled_texture_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/sampled_texture_type.h"
#include "src/ast/type/sampler_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/ast/unary_op.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/workgroup_decoration.h"
#include "src/reader/wgsl/lexer.h"
#include "src/type_manager.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

template <typename T>
using Expect = ParserImpl::Expect<T>;

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

bool is_decoration(Token t) {
  return t.IsLocation() || t.IsBinding() || t.IsSet() || t.IsBuiltin() ||
         t.IsWorkgroupSize() || t.IsStage() || t.IsBlock() || t.IsStride() ||
         t.IsOffset();
}

}  // namespace

ParserImpl::ParserImpl(Context* ctx, Source::File const* file)
    : ctx_(*ctx), lexer_(std::make_unique<Lexer>(file)) {}

ParserImpl::~ParserImpl() = default;

ParserImpl::Failure::Errored ParserImpl::add_error(const Source& source,
                                                   const std::string& err,
                                                   const std::string& use) {
  std::stringstream msg;
  msg << err;
  if (!use.empty()) {
    msg << " for " << use;
  }
  add_error(source, msg.str());
  return Failure::kErrored;
}

ParserImpl::Failure::Errored ParserImpl::add_error(const Token& t,
                                                   const std::string& err) {
  add_error(t.source(), err);
  return Failure::kErrored;
}

ParserImpl::Failure::Errored ParserImpl::add_error(const Source& source,
                                                   const std::string& err) {
  diag::Diagnostic diagnostic;
  diagnostic.severity = diag::Severity::Error;
  diagnostic.message = err;
  diagnostic.source = source;
  diags_.add(std::move(diagnostic));
  return Failure::kErrored;
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

void ParserImpl::register_constructed(const std::string& name,
                                      ast::type::Type* type) {
  assert(type);
  registered_constructs_[name] = type;
}

ast::type::Type* ParserImpl::get_constructed(const std::string& name) {
  if (registered_constructs_.find(name) == registered_constructs_.end()) {
    return nullptr;
  }
  return registered_constructs_[name];
}

bool ParserImpl::Parse() {
  translation_unit();
  return !has_error();
}

// translation_unit
//  : global_decl* EOF
void ParserImpl::translation_unit() {
  while (!peek().IsEof()) {
    auto decl = expect_global_decl();
    if (decl.errored)
      break;
  }

  assert(module_.IsValid());
}

// global_decl
//  : SEMICOLON
//  | global_variable_decl SEMICLON
//  | global_constant_decl SEMICOLON
//  | type_alias SEMICOLON
//  | struct_decl SEMICOLON
//  | function_decl
Expect<bool> ParserImpl::expect_global_decl() {
  if (match(Token::Type::kSemicolon) || match(Token::Type::kEOF))
    return true;

  auto decos = decoration_list();

  auto gv = global_variable_decl(decos);
  if (has_error())
    return Failure::kErrored;

  if (gv != nullptr) {
    if (!expect("variable declaration", Token::Type::kSemicolon))
      return Failure::kErrored;

    module_.AddGlobalVariable(std::move(gv));
    return true;
  }

  auto gc = global_constant_decl();
  if (has_error())
    return Failure::kErrored;

  if (gc != nullptr) {
    if (!expect("constant declaration", Token::Type::kSemicolon))
      return Failure::kErrored;

    module_.AddGlobalVariable(std::move(gc));
    return true;
  }

  auto* ta = type_alias();
  if (has_error())
    return Failure::kErrored;

  if (ta != nullptr) {
    if (!expect("type alias", Token::Type::kSemicolon))
      return Failure::kErrored;

    module_.AddConstructedType(ta);
    return true;
  }

  auto str = struct_decl(decos);
  if (has_error())
    return Failure::kErrored;

  if (str != nullptr) {
    if (!expect("struct declaration", Token::Type::kSemicolon))
      return Failure::kErrored;

    auto* type = ctx_.type_mgr().Get(std::move(str));
    register_constructed(type->AsStruct()->name(), type);
    module_.AddConstructedType(type);
    return true;
  }

  auto func = function_decl(decos);
  if (has_error())
    return Failure::kErrored;

  if (func != nullptr) {
    module_.AddFunction(std::move(func));
    return true;
  }

  if (decos.size() > 0) {
    add_error(peek(), "expected declaration after decorations");
  } else {
    add_error(peek(), "invalid token");
  }
  return Failure::kErrored;
}

// global_variable_decl
//  : variable_decoration_list* variable_decl
//  | variable_decoration_list* variable_decl EQUAL const_expr
std::unique_ptr<ast::Variable> ParserImpl::global_variable_decl(
    ast::DecorationList& decos) {
  auto var = variable_decl();
  if (has_error() || var == nullptr)
    return nullptr;

  auto var_decos = cast_decorations<ast::VariableDecoration>(decos);
  if (var_decos.errored)
    return nullptr;

  if (var_decos.value.size() > 0) {
    auto dv = std::make_unique<ast::DecoratedVariable>(std::move(var));
    dv->set_decorations(std::move(var_decos.value));
    var = std::move(dv);
  }

  if (match(Token::Type::kEqual)) {
    auto expr = expect_const_expr();
    if (expr.errored)
      return nullptr;
    var->set_constructor(std::move(expr.value));
  }
  return var;
}

// global_constant_decl
//  : CONST variable_ident_decl EQUAL const_expr
std::unique_ptr<ast::Variable> ParserImpl::global_constant_decl() {
  if (!match(Token::Type::kConst))
    return nullptr;

  const char* use = "constant declaration";

  auto decl = expect_variable_ident_decl(use);
  if (decl.errored)
    return nullptr;

  auto var = std::make_unique<ast::Variable>(
      decl->source, decl->name, ast::StorageClass::kNone, decl->type);
  var->set_is_const(true);

  if (!expect(use, Token::Type::kEqual))
    return nullptr;

  auto init = expect_const_expr();
  if (init.errored)
    return nullptr;

  var->set_constructor(std::move(init.value));

  return var;
}

// variable_decl
//   : VAR variable_storage_decoration? variable_ident_decl
std::unique_ptr<ast::Variable> ParserImpl::variable_decl() {
  if (!match(Token::Type::kVar))
    return nullptr;

  auto sc = variable_storage_decoration();
  if (has_error())
    return nullptr;

  auto decl = expect_variable_ident_decl("variable declaration");
  if (decl.errored)
    return nullptr;

  return std::make_unique<ast::Variable>(decl->source, decl->name, sc,
                                         decl->type);
}

// texture_sampler_types
//  : sampler_type
//  | depth_texture_type
//  | sampled_texture_type LESS_THAN type_decl GREATER_THAN
//  | multisampled_texture_type LESS_THAN type_decl GREATER_THAN
//  | storage_texture_type LESS_THAN image_storage_type GREATER_THAN
ast::type::Type* ParserImpl::texture_sampler_types() {
  auto* type = sampler_type();
  if (type != nullptr) {
    return type;
  }

  type = depth_texture_type();
  if (type != nullptr) {
    return type;
  }

  auto dim = sampled_texture_type();
  if (dim != ast::type::TextureDimension::kNone) {
    const char* use = "sampled texture type";

    if (!expect(use, Token::Type::kLessThan))
      return nullptr;

    auto* subtype = type_decl();
    if (has_error())
      return nullptr;
    if (subtype == nullptr) {
      add_error(peek().source(), "invalid subtype", use);
      return nullptr;
    }

    if (!expect(use, Token::Type::kGreaterThan))
      return nullptr;

    return ctx_.type_mgr().Get(
        std::make_unique<ast::type::SampledTextureType>(dim, subtype));
  }

  dim = multisampled_texture_type();
  if (dim != ast::type::TextureDimension::kNone) {
    const char* use = "multisampled texture type";

    if (!expect(use, Token::Type::kLessThan))
      return nullptr;

    auto* subtype = type_decl();
    if (has_error())
      return nullptr;
    if (subtype == nullptr) {
      add_error(peek().source(), "invalid subtype", use);
      return nullptr;
    }

    if (!expect(use, Token::Type::kGreaterThan))
      return nullptr;

    return ctx_.type_mgr().Get(
        std::make_unique<ast::type::MultisampledTextureType>(dim, subtype));
  }

  ast::type::TextureDimension storage_dim;
  ast::AccessControl access;
  std::tie(storage_dim, access) = storage_texture_type();
  if (storage_dim != ast::type::TextureDimension::kNone) {
    const char* use = "storage texture type";

    if (!expect(use, Token::Type::kLessThan))
      return nullptr;

    auto format = expect_image_storage_type(use);
    if (format.errored)
      return nullptr;

    if (!expect(use, Token::Type::kGreaterThan))
      return nullptr;

    return ctx_.type_mgr().Get(std::make_unique<ast::type::StorageTextureType>(
        storage_dim, access, format.value));
  }

  return nullptr;
}

// sampler_type
//  : SAMPLER
//  | SAMPLER_COMPARISON
ast::type::Type* ParserImpl::sampler_type() {
  if (match(Token::Type::kSampler))
    return ctx_.type_mgr().Get(std::make_unique<ast::type::SamplerType>(
        ast::type::SamplerKind::kSampler));

  if (match(Token::Type::kComparisonSampler))
    return ctx_.type_mgr().Get(std::make_unique<ast::type::SamplerType>(
        ast::type::SamplerKind::kComparisonSampler));

  return nullptr;
}

// sampled_texture_type
//  : TEXTURE_SAMPLED_1D
//  | TEXTURE_SAMPLED_1D_ARRAY
//  | TEXTURE_SAMPLED_2D
//  | TEXTURE_SAMPLED_2D_ARRAY
//  | TEXTURE_SAMPLED_3D
//  | TEXTURE_SAMPLED_CUBE
//  | TEXTURE_SAMPLED_CUBE_ARRAY
ast::type::TextureDimension ParserImpl::sampled_texture_type() {
  if (match(Token::Type::kTextureSampled1d))
    return ast::type::TextureDimension::k1d;

  if (match(Token::Type::kTextureSampled1dArray))
    return ast::type::TextureDimension::k1dArray;

  if (match(Token::Type::kTextureSampled2d))
    return ast::type::TextureDimension::k2d;

  if (match(Token::Type::kTextureSampled2dArray))
    return ast::type::TextureDimension::k2dArray;

  if (match(Token::Type::kTextureSampled3d))
    return ast::type::TextureDimension::k3d;

  if (match(Token::Type::kTextureSampledCube))
    return ast::type::TextureDimension::kCube;

  if (match(Token::Type::kTextureSampledCubeArray))
    return ast::type::TextureDimension::kCubeArray;

  return ast::type::TextureDimension::kNone;
}

// multisampled_texture_type
//  : TEXTURE_MULTISAMPLED_2D
ast::type::TextureDimension ParserImpl::multisampled_texture_type() {
  if (match(Token::Type::kTextureMultisampled2d))
    return ast::type::TextureDimension::k2d;

  return ast::type::TextureDimension::kNone;
}

// storage_texture_type
//  : TEXTURE_RO_1D
//  | TEXTURE_RO_1D_ARRAY
//  | TEXTURE_RO_2D
//  | TEXTURE_RO_2D_ARRAY
//  | TEXTURE_RO_3D
//  | TEXTURE_WO_1D
//  | TEXTURE_WO_1D_ARRAY
//  | TEXTURE_WO_2D
//  | TEXTURE_WO_2D_ARRAY
//  | TEXTURE_WO_3D
//  | TEXTURE_STORAGE_RO_1D
//  | TEXTURE_STORAGE_RO_1D_ARRAY
//  | TEXTURE_STORAGE_RO_2D
//  | TEXTURE_STORAGE_RO_2D_ARRAY
//  | TEXTURE_STORAGE_RO_3D
//  | TEXTURE_STORAGE_WO_1D
//  | TEXTURE_STORAGE_WO_1D_ARRAY
//  | TEXTURE_STORAGE_WO_2D
//  | TEXTURE_STORAGE_WO_2D_ARRAY
//  | TEXTURE_STORAGE_WO_3D
std::pair<ast::type::TextureDimension, ast::AccessControl>
ParserImpl::storage_texture_type() {
  if (match(Token::Type::kTextureStorageReadonly1d))
    return {ast::type::TextureDimension::k1d, ast::AccessControl::kReadOnly};

  if (match(Token::Type::kTextureStorageReadonly1dArray))
    return {ast::type::TextureDimension::k1dArray,
            ast::AccessControl::kReadOnly};

  if (match(Token::Type::kTextureStorageReadonly2d))
    return {ast::type::TextureDimension::k2d, ast::AccessControl::kReadOnly};

  if (match(Token::Type::kTextureStorageReadonly2dArray))
    return {ast::type::TextureDimension::k2dArray,
            ast::AccessControl::kReadOnly};

  if (match(Token::Type::kTextureStorageReadonly3d))
    return {ast::type::TextureDimension::k3d, ast::AccessControl::kReadOnly};

  if (match(Token::Type::kTextureStorageWriteonly1d))
    return {ast::type::TextureDimension::k1d, ast::AccessControl::kWriteOnly};

  if (match(Token::Type::kTextureStorageWriteonly1dArray))
    return {ast::type::TextureDimension::k1dArray,
            ast::AccessControl::kWriteOnly};

  if (match(Token::Type::kTextureStorageWriteonly2d))
    return {ast::type::TextureDimension::k2d, ast::AccessControl::kWriteOnly};

  if (match(Token::Type::kTextureStorageWriteonly2dArray))
    return {ast::type::TextureDimension::k2dArray,
            ast::AccessControl::kWriteOnly};

  if (match(Token::Type::kTextureStorageWriteonly3d))
    return {ast::type::TextureDimension::k3d, ast::AccessControl::kWriteOnly};

  return {ast::type::TextureDimension::kNone, ast::AccessControl::kReadOnly};
}

// depth_texture_type
//  : TEXTURE_DEPTH_2D
//  | TEXTURE_DEPTH_2D_ARRAY
//  | TEXTURE_DEPTH_CUBE
//  | TEXTURE_DEPTH_CUBE_ARRAY
ast::type::Type* ParserImpl::depth_texture_type() {
  if (match(Token::Type::kTextureDepth2d))
    return ctx_.type_mgr().Get(std::make_unique<ast::type::DepthTextureType>(
        ast::type::TextureDimension::k2d));

  if (match(Token::Type::kTextureDepth2dArray))
    return ctx_.type_mgr().Get(std::make_unique<ast::type::DepthTextureType>(
        ast::type::TextureDimension::k2dArray));

  if (match(Token::Type::kTextureDepthCube))
    return ctx_.type_mgr().Get(std::make_unique<ast::type::DepthTextureType>(
        ast::type::TextureDimension::kCube));

  if (match(Token::Type::kTextureDepthCubeArray))
    return ctx_.type_mgr().Get(std::make_unique<ast::type::DepthTextureType>(
        ast::type::TextureDimension::kCubeArray));

  return nullptr;
}

// image_storage_type
//  : R8UNORM
//  | R8SNORM
//  | R8UINT
//  | R8SINT
//  | R16UINT
//  | R16SINT
//  | R16FLOAT
//  | RG8UNORM
//  | RG8SNORM
//  | RG8UINT
//  | RG8SINT
//  | R32UINT
//  | R32SINT
//  | R32FLOAT
//  | RG16UINT
//  | RG16SINT
//  | RG16FLOAT
//  | RGBA8UNORM
/// | RGBA8UNORM-SRGB
//  | RGBA8SNORM
//  | RGBA8UINT
//  | RGBA8SINT
//  | BGRA8UNORM
//  | BGRA8UNORM-SRGB
//  | RGB10A2UNORM
//  | RG11B10FLOAT
//  | RG32UINT
//  | RG32SINT
//  | RG32FLOAT
//  | RGBA16UINT
//  | RGBA16SINT
//  | RGBA16FLOAT
//  | RGBA32UINT
//  | RGBA32SINT
//  | RGBA32FLOAT
Expect<ast::type::ImageFormat> ParserImpl::expect_image_storage_type(
    const std::string& use) {
  if (match(Token::Type::kFormatR8Unorm))
    return ast::type::ImageFormat::kR8Unorm;

  if (match(Token::Type::kFormatR8Snorm))
    return ast::type::ImageFormat::kR8Snorm;

  if (match(Token::Type::kFormatR8Uint))
    return ast::type::ImageFormat::kR8Uint;

  if (match(Token::Type::kFormatR8Sint))
    return ast::type::ImageFormat::kR8Sint;

  if (match(Token::Type::kFormatR16Uint))
    return ast::type::ImageFormat::kR16Uint;

  if (match(Token::Type::kFormatR16Sint))
    return ast::type::ImageFormat::kR16Sint;

  if (match(Token::Type::kFormatR16Float))
    return ast::type::ImageFormat::kR16Float;

  if (match(Token::Type::kFormatRg8Unorm))
    return ast::type::ImageFormat::kRg8Unorm;

  if (match(Token::Type::kFormatRg8Snorm))
    return ast::type::ImageFormat::kRg8Snorm;

  if (match(Token::Type::kFormatRg8Uint))
    return ast::type::ImageFormat::kRg8Uint;

  if (match(Token::Type::kFormatRg8Sint))
    return ast::type::ImageFormat::kRg8Sint;

  if (match(Token::Type::kFormatR32Uint))
    return ast::type::ImageFormat::kR32Uint;

  if (match(Token::Type::kFormatR32Sint))
    return ast::type::ImageFormat::kR32Sint;

  if (match(Token::Type::kFormatR32Float))
    return ast::type::ImageFormat::kR32Float;

  if (match(Token::Type::kFormatRg16Uint))
    return ast::type::ImageFormat::kRg16Uint;

  if (match(Token::Type::kFormatRg16Sint))
    return ast::type::ImageFormat::kRg16Sint;

  if (match(Token::Type::kFormatRg16Float))
    return ast::type::ImageFormat::kRg16Float;

  if (match(Token::Type::kFormatRgba8Unorm))
    return ast::type::ImageFormat::kRgba8Unorm;

  if (match(Token::Type::kFormatRgba8UnormSrgb))
    return ast::type::ImageFormat::kRgba8UnormSrgb;

  if (match(Token::Type::kFormatRgba8Snorm))
    return ast::type::ImageFormat::kRgba8Snorm;

  if (match(Token::Type::kFormatRgba8Uint))
    return ast::type::ImageFormat::kRgba8Uint;

  if (match(Token::Type::kFormatRgba8Sint))
    return ast::type::ImageFormat::kRgba8Sint;

  if (match(Token::Type::kFormatBgra8Unorm))
    return ast::type::ImageFormat::kBgra8Unorm;

  if (match(Token::Type::kFormatBgra8UnormSrgb))
    return ast::type::ImageFormat::kBgra8UnormSrgb;

  if (match(Token::Type::kFormatRgb10A2Unorm))
    return ast::type::ImageFormat::kRgb10A2Unorm;

  if (match(Token::Type::kFormatRg11B10Float))
    return ast::type::ImageFormat::kRg11B10Float;

  if (match(Token::Type::kFormatRg32Uint))
    return ast::type::ImageFormat::kRg32Uint;

  if (match(Token::Type::kFormatRg32Sint))
    return ast::type::ImageFormat::kRg32Sint;

  if (match(Token::Type::kFormatRg32Float))
    return ast::type::ImageFormat::kRg32Float;

  if (match(Token::Type::kFormatRgba16Uint))
    return ast::type::ImageFormat::kRgba16Uint;

  if (match(Token::Type::kFormatRgba16Sint))
    return ast::type::ImageFormat::kRgba16Sint;

  if (match(Token::Type::kFormatRgba16Float))
    return ast::type::ImageFormat::kRgba16Float;

  if (match(Token::Type::kFormatRgba32Uint))
    return ast::type::ImageFormat::kRgba32Uint;

  if (match(Token::Type::kFormatRgba32Sint))
    return ast::type::ImageFormat::kRgba32Sint;

  if (match(Token::Type::kFormatRgba32Float))
    return ast::type::ImageFormat::kRgba32Float;

  return add_error(peek().source(), "invalid format", use);
}

// variable_ident_decl
//   : IDENT COLON type_decl
Expect<ParserImpl::TypedIdentifier> ParserImpl::expect_variable_ident_decl(
    const std::string& use) {
  auto ident = expect_ident(use);
  if (ident.errored)
    return Failure::kErrored;

  if (!expect(use, Token::Type::kColon))
    return Failure::kErrored;

  auto t = peek();
  auto* type = type_decl();
  if (has_error())
    return Failure::kErrored;

  if (type == nullptr) {
    add_error(t.source(), "invalid type", use);
    return Failure::kErrored;
  }

  return TypedIdentifier{type, ident.value, ident.source};
}

// variable_storage_decoration
//   : LESS_THAN storage_class GREATER_THAN
ast::StorageClass ParserImpl::variable_storage_decoration() {
  if (!match(Token::Type::kLessThan))
    return ast::StorageClass::kNone;

  const char* use = "variable decoration";

  auto sc = expect_storage_class(use);
  if (sc.errored)
    return ast::StorageClass::kNone;

  if (!expect(use, Token::Type::kGreaterThan))
    return ast::StorageClass::kNone;

  return sc.value;
}

// type_alias
//   : TYPE IDENT EQUAL type_decl
ast::type::Type* ParserImpl::type_alias() {
  auto t = peek();
  if (!t.IsType())
    return nullptr;

  next();  // Consume the peek

  const char* use = "type alias";

  auto name = expect_ident(use);
  if (name.errored)
    return nullptr;

  if (!expect(use, Token::Type::kEqual))
    return nullptr;

  auto* type = type_decl();
  if (has_error())
    return nullptr;
  if (type == nullptr) {
    add_error(peek(), "invalid type alias");
    return nullptr;
  }

  auto* alias = ctx_.type_mgr().Get(
      std::make_unique<ast::type::AliasType>(name.value, type));
  register_constructed(name.value, alias);

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
//   | array_decoration_list* ARRAY LESS_THAN type_decl COMMA
//          INT_LITERAL GREATER_THAN
//   | array_decoration_list* ARRAY LESS_THAN type_decl
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
//   | texture_sampler_types
ast::type::Type* ParserImpl::type_decl() {
  auto t = peek();
  if (match(Token::Type::kIdentifier)) {
    auto* ty = get_constructed(t.to_str());
    if (ty == nullptr) {
      add_error(t, "unknown constructed type '" + t.to_str() + "'");
      return nullptr;
    }
    return ty;
  }

  if (match(Token::Type::kBool))
    return ctx_.type_mgr().Get(std::make_unique<ast::type::BoolType>());

  if (match(Token::Type::kF32))
    return ctx_.type_mgr().Get(std::make_unique<ast::type::F32Type>());

  if (match(Token::Type::kI32))
    return ctx_.type_mgr().Get(std::make_unique<ast::type::I32Type>());

  if (match(Token::Type::kU32))
    return ctx_.type_mgr().Get(std::make_unique<ast::type::U32Type>());

  if (t.IsVec2() || t.IsVec3() || t.IsVec4()) {
    return expect_type_decl_vector(t).value;
  }

  if (match(Token::Type::kPtr))
    return expect_type_decl_pointer().value;

  auto decos = decoration_list();
  if (has_error())
    return nullptr;

  if (match(Token::Type::kArray)) {
    auto array_decos = cast_decorations<ast::ArrayDecoration>(decos);
    if (array_decos.errored)
      return nullptr;
    return expect_type_decl_array(std::move(array_decos.value)).value;
  }

  expect_decorations_consumed(decos);

  if (t.IsMat2x2() || t.IsMat2x3() || t.IsMat2x4() || t.IsMat3x2() ||
      t.IsMat3x3() || t.IsMat3x4() || t.IsMat4x2() || t.IsMat4x3() ||
      t.IsMat4x4()) {
    return expect_type_decl_matrix(t).value;
  }

  auto* texture_or_sampler = texture_sampler_types();
  if (has_error()) {
    return nullptr;
  }
  if (texture_or_sampler != nullptr) {
    return texture_or_sampler;
  }

  return nullptr;
}

Expect<ast::type::Type*> ParserImpl::expect_type_decl_pointer() {
  const char* use = "ptr declaration";

  if (!expect(use, Token::Type::kLessThan))
    return Failure::kErrored;

  auto sc = expect_storage_class(use);
  if (sc.errored)
    return Failure::kErrored;

  if (!expect(use, Token::Type::kComma))
    return Failure::kErrored;

  auto* subtype = type_decl();
  if (has_error())
    return Failure::kErrored;
  if (subtype == nullptr)
    return add_error(peek().source(), "missing type", use);

  if (!expect(use, Token::Type::kGreaterThan))
    return Failure::kErrored;

  return ctx_.type_mgr().Get(
      std::make_unique<ast::type::PointerType>(subtype, sc.value));
}

Expect<ast::type::Type*> ParserImpl::expect_type_decl_vector(Token t) {
  next();  // Consume the peek

  uint32_t count = 2;
  if (t.IsVec3())
    count = 3;
  else if (t.IsVec4())
    count = 4;

  const char* use = "vector";

  if (!expect(use, Token::Type::kLessThan))
    return Failure::kErrored;

  auto* subtype = type_decl();
  if (has_error())
    return Failure::kErrored;
  if (subtype == nullptr) {
    return add_error(peek().source(), "unable to determine subtype", use);
  }

  if (!expect(use, Token::Type::kGreaterThan))
    return Failure::kErrored;

  return ctx_.type_mgr().Get(
      std::make_unique<ast::type::VectorType>(subtype, count));
}

Expect<ast::type::Type*> ParserImpl::expect_type_decl_array(
    ast::ArrayDecorationList decos) {
  const char* use = "array declaration";

  if (!expect(use, Token::Type::kLessThan))
    return Failure::kErrored;

  auto* subtype = type_decl();
  if (has_error())
    return Failure::kErrored;
  if (subtype == nullptr)
    return add_error(peek(), "invalid type for array declaration");

  uint32_t size = 0;
  if (match(Token::Type::kComma)) {
    auto val = expect_nonzero_positive_sint("array size");
    if (val.errored)
      return Failure::kErrored;
    size = val.value;
  }

  if (!expect(use, Token::Type::kGreaterThan))
    return Failure::kErrored;

  auto ty = std::make_unique<ast::type::ArrayType>(subtype, size);
  ty->set_decorations(std::move(decos));
  return ctx_.type_mgr().Get(std::move(ty));
}

Expect<ast::type::Type*> ParserImpl::expect_type_decl_matrix(Token t) {
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
  if (!t.IsLessThan())
    return add_error(t, "missing < for matrix");

  auto* subtype = type_decl();
  if (has_error())
    return Failure::kErrored;
  if (subtype == nullptr)
    return add_error(peek(), "unable to determine subtype for matrix");

  t = next();
  if (!t.IsGreaterThan())
    return add_error(t, "missing > for matrix");

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
Expect<ast::StorageClass> ParserImpl::expect_storage_class(
    const std::string& use) {
  if (match(Token::Type::kIn))
    return ast::StorageClass::kInput;

  if (match(Token::Type::kOut))
    return ast::StorageClass::kOutput;

  if (match(Token::Type::kUniform))
    return ast::StorageClass::kUniform;

  if (match(Token::Type::kWorkgroup))
    return ast::StorageClass::kWorkgroup;

  if (match(Token::Type::kUniformConstant))
    return ast::StorageClass::kUniformConstant;

  if (match(Token::Type::kStorageBuffer))
    return ast::StorageClass::kStorageBuffer;

  if (match(Token::Type::kImage))
    return ast::StorageClass::kImage;

  if (match(Token::Type::kPrivate))
    return ast::StorageClass::kPrivate;

  if (match(Token::Type::kFunction))
    return ast::StorageClass::kFunction;

  return add_error(peek().source(), "invalid storage class", use);
}

// struct_decl
//   : struct_decoration_decl* STRUCT IDENT struct_body_decl
std::unique_ptr<ast::type::StructType> ParserImpl::struct_decl(
    ast::DecorationList& decos) {
  auto t = peek();
  auto source = t.source();

  if (!match(Token::Type::kStruct))
    return nullptr;

  auto struct_decos = cast_decorations<ast::StructDecoration>(decos);
  if (struct_decos.errored)
    return nullptr;

  auto name = expect_ident("struct declaration");
  if (name.errored)
    return nullptr;

  auto body = expect_struct_body_decl();
  if (body.errored)
    return nullptr;

  return std::make_unique<ast::type::StructType>(
      name.value,
      std::make_unique<ast::Struct>(source, std::move(struct_decos.value),
                                    std::move(body.value)));
}

// struct_body_decl
//   : BRACKET_LEFT struct_member* BRACKET_RIGHT
Expect<ast::StructMemberList> ParserImpl::expect_struct_body_decl() {
  return expect_brace_block(
      "struct declaration", [&]() -> Expect<ast::StructMemberList> {
        ast::StructMemberList members;

        while (!peek().IsBraceRight() && !peek().IsEof()) {
          auto decos = decoration_list();
          if (has_error())
            return Failure::kErrored;

          auto mem = expect_struct_member(decos);
          if (mem.errored)
            return Failure::kErrored;

          members.push_back(std::move(mem.value));
        }

        return members;
      });
}

// struct_member
//   : struct_member_decoration_decl+ variable_ident_decl SEMICOLON
Expect<std::unique_ptr<ast::StructMember>> ParserImpl::expect_struct_member(
    ast::DecorationList& decos) {
  // FUDGE - Abort early if we enter with an error state to avoid accumulating
  // multiple error messages. This is a work around for the unit tests that
  // call:
  //   auto decos = p->decoration_list();
  //   auto m = p->expect_struct_member(decos);
  // ... and expect a single error message due to bad decorations.
  // While expect_struct_body_decl() aborts after checking for decoration parse
  // errors (and so these tests do not currently reflect full-parse behaviour),
  // they do test the long-term desired behavior where the parser can
  // resynchronize at the ']]'.
  // TODO(ben-clayton) - remove this once resynchronization is implemented.
  if (has_error())
    return Failure::kErrored;

  auto decl = expect_variable_ident_decl("struct member");
  if (decl.errored)
    return Failure::kErrored;

  auto member_decos = cast_decorations<ast::StructMemberDecoration>(decos);

  if (!expect("struct member", Token::Type::kSemicolon))
    return Failure::kErrored;

  return std::make_unique<ast::StructMember>(
      decl->source, decl->name, decl->type, std::move(member_decos.value));
}

// function_decl
//   : function_header body_stmt
std::unique_ptr<ast::Function> ParserImpl::function_decl(
    ast::DecorationList& decos) {
  auto f = function_header();
  if (f == nullptr || has_error())
    return nullptr;

  auto func_decos = cast_decorations<ast::FunctionDecoration>(decos);
  if (func_decos.errored)
    return nullptr;
  f->set_decorations(std::move(func_decos.value));

  auto body = expect_body_stmt();
  if (body.errored)
    return nullptr;

  f->set_body(std::move(body.value));
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
  Source source;
  if (!match(Token::Type::kFn, &source))
    return nullptr;

  const char* use = "function declaration";

  auto name = expect_ident(use);
  if (name.errored)
    return nullptr;

  auto params = expect_paren_block(use, [&] { return expect_param_list(); });

  if (params.errored)
    return nullptr;

  auto t = next();
  if (!t.IsArrow()) {
    add_error(t, "missing -> for function declaration");
    return nullptr;
  }

  auto* type = function_type_decl();
  if (has_error())
    return nullptr;
  if (type == nullptr) {
    add_error(peek(), "unable to determine function return type");
    return nullptr;
  }

  return std::make_unique<ast::Function>(source, name.value,
                                         std::move(params.value), type);
}

// param_list
//   :
//   | (variable_ident_decl COMMA)* variable_ident_decl
Expect<ast::VariableList> ParserImpl::expect_param_list() {
  if (!peek().IsIdentifier())  // Empty list
    return ast::VariableList{};

  auto decl = expect_variable_ident_decl("parameter");
  if (decl.errored)
    return Failure::kErrored;

  ast::VariableList ret;
  for (;;) {
    auto var = std::make_unique<ast::Variable>(
        decl->source, decl->name, ast::StorageClass::kNone, decl->type);
    // Formal parameters are treated like a const declaration where the
    // initializer value is provided by the call's argument.  The key point is
    // that it's not updatable after intially set.  This is unlike C or GLSL
    // which treat formal parameters like local variables that can be updated.
    var->set_is_const(true);
    ret.push_back(std::move(var));

    if (!match(Token::Type::kComma))
      break;

    decl = expect_variable_ident_decl("parameter");
    if (decl.errored)
      return Failure::kErrored;
  }

  return ret;
}

// pipeline_stage
//   : VERTEX
//   | FRAGMENT
//   | COMPUTE
Expect<ast::PipelineStage> ParserImpl::expect_pipeline_stage() {
  Source source;
  if (match(Token::Type::kVertex, &source))
    return {ast::PipelineStage::kVertex, source};

  if (match(Token::Type::kFragment, &source))
    return {ast::PipelineStage::kFragment, source};

  if (match(Token::Type::kCompute, &source))
    return {ast::PipelineStage::kCompute, source};

  return add_error(peek(), "invalid value for stage decoration");
}

Expect<ast::Builtin> ParserImpl::expect_builtin() {
  auto ident = expect_ident("builtin");
  if (ident.errored)
    return Failure::kErrored;

  ast::Builtin builtin = ident_to_builtin(ident.value);
  if (builtin == ast::Builtin::kNone)
    return add_error(ident.source, "invalid value for builtin decoration");

  return {builtin, ident.source};
}

// body_stmt
//   : BRACKET_LEFT statements BRACKET_RIGHT
Expect<std::unique_ptr<ast::BlockStatement>> ParserImpl::expect_body_stmt() {
  return expect_brace_block("", [&] { return expect_statements(); });
}

// paren_rhs_stmt
//   : PAREN_LEFT logical_or_expression PAREN_RIGHT
Expect<std::unique_ptr<ast::Expression>> ParserImpl::expect_paren_rhs_stmt() {
  return expect_paren_block(
      "", [&]() -> Expect<std::unique_ptr<ast::Expression>> {
        auto expr = logical_or_expression();
        if (has_error())
          return Failure::kErrored;

        if (expr == nullptr)
          return add_error(peek(), "unable to parse expression");

        return expr;
      });
}

// statements
//   : statement*
Expect<std::unique_ptr<ast::BlockStatement>> ParserImpl::expect_statements() {
  auto ret = std::make_unique<ast::BlockStatement>();

  for (;;) {
    auto stmt = statement();
    if (has_error())
      return Failure::kErrored;
    if (!stmt)
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
//   | body_stmt?
std::unique_ptr<ast::Statement> ParserImpl::statement() {
  while (match(Token::Type::kSemicolon)) {
    // Skip empty statements
  }

  auto t = peek();
  auto ret_stmt = return_stmt();
  if (has_error())
    return nullptr;
  if (ret_stmt != nullptr) {
    if (!expect("return statement", Token::Type::kSemicolon))
      return nullptr;

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
    if (!expect("function call", Token::Type::kSemicolon))
      return nullptr;

    return func;
  }

  auto var = variable_stmt();
  if (has_error())
    return nullptr;
  if (var != nullptr) {
    if (!expect("variable declaration", Token::Type::kSemicolon))
      return nullptr;

    return var;
  }

  auto b = break_stmt();
  if (has_error())
    return nullptr;
  if (b != nullptr) {
    if (!expect("break statement", Token::Type::kSemicolon))
      return nullptr;

    return b;
  }

  auto cont = continue_stmt();
  if (has_error())
    return nullptr;
  if (cont != nullptr) {
    if (!expect("continue statement", Token::Type::kSemicolon))
      return nullptr;

    return cont;
  }

  if (t.IsDiscard()) {
    auto source = t.source();
    next();  // Consume the peek

    if (!expect("discard statement", Token::Type::kSemicolon))
      return nullptr;

    return std::make_unique<ast::DiscardStatement>(source);
  }

  auto assign = assignment_stmt();
  if (has_error())
    return nullptr;
  if (assign != nullptr) {
    if (!expect("assignment statement", Token::Type::kSemicolon))
      return nullptr;

    return assign;
  }

  t = peek();
  if (t.IsBraceLeft()) {
    auto body = expect_body_stmt();
    if (body.errored)
      return nullptr;
    return std::move(body.value);
  }

  return nullptr;
}

// return_stmt
//   : RETURN logical_or_expression?
std::unique_ptr<ast::ReturnStatement> ParserImpl::return_stmt() {
  Source source;
  if (!match(Token::Type::kReturn, &source))
    return nullptr;

  std::unique_ptr<ast::Expression> expr = nullptr;
  if (!peek().IsSemicolon()) {
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
  if (t.IsConst()) {
    next();  // Consume the peek

    auto decl = expect_variable_ident_decl("constant declaration");
    if (has_error())
      return nullptr;

    if (!expect("constant declaration", Token::Type::kEqual))
      return nullptr;

    auto constructor = logical_or_expression();
    if (has_error())
      return nullptr;
    if (constructor == nullptr) {
      add_error(peek(), "missing constructor for const declaration");
      return nullptr;
    }

    auto var = std::make_unique<ast::Variable>(
        decl->source, decl->name, ast::StorageClass::kNone, decl->type);
    var->set_is_const(true);
    var->set_constructor(std::move(constructor));

    return std::make_unique<ast::VariableDeclStatement>(decl->source,
                                                        std::move(var));
  }

  auto var = variable_decl();
  if (has_error())
    return nullptr;
  if (var == nullptr)
    return nullptr;

  if (match(Token::Type::kEqual)) {
    auto constructor = logical_or_expression();
    if (has_error())
      return nullptr;
    if (constructor == nullptr) {
      add_error(peek(), "missing constructor for variable declaration");
      return nullptr;
    }
    var->set_constructor(std::move(constructor));
  }

  return std::make_unique<ast::VariableDeclStatement>(var->source(),
                                                      std::move(var));
}

// if_stmt
//   : IF paren_rhs_stmt body_stmt elseif_stmt? else_stmt?
std::unique_ptr<ast::IfStatement> ParserImpl::if_stmt() {
  Source source;
  if (!match(Token::Type::kIf, &source))
    return nullptr;

  auto condition = expect_paren_rhs_stmt();
  if (condition.errored)
    return nullptr;

  auto body = expect_body_stmt();
  if (body.errored)
    return nullptr;

  auto elseif = elseif_stmt();
  if (has_error())
    return nullptr;

  auto el = else_stmt();
  if (has_error())
    return nullptr;

  auto stmt = std::make_unique<ast::IfStatement>(
      source, std::move(condition.value), std::move(body.value));
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

    auto condition = expect_paren_rhs_stmt();
    if (condition.errored)
      return {};

    auto body = expect_body_stmt();
    if (body.errored)
      return {};

    ret.push_back(std::make_unique<ast::ElseStatement>(
        source, std::move(condition.value), std::move(body.value)));

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

  auto body = expect_body_stmt();
  if (body.errored)
    return nullptr;

  return std::make_unique<ast::ElseStatement>(source, std::move(body.value));
}

// switch_stmt
//   : SWITCH paren_rhs_stmt BRACKET_LEFT switch_body+ BRACKET_RIGHT
std::unique_ptr<ast::SwitchStatement> ParserImpl::switch_stmt() {
  Source source;
  if (!match(Token::Type::kSwitch, &source))
    return nullptr;

  auto condition = expect_paren_rhs_stmt();
  if (condition.errored)
    return nullptr;

  ast::CaseStatementList body;
  bool ok = expect_brace_block_old("switch statement", [&] {
    for (;;) {
      auto stmt = switch_body();
      if (has_error())
        return false;
      if (stmt == nullptr)
        break;

      body.push_back(std::move(stmt));
    }
    return true;
  });

  if (!ok)
    return nullptr;

  return std::make_unique<ast::SwitchStatement>(
      source, std::move(condition.value), std::move(body));
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
    auto selectors = expect_case_selectors();
    if (has_error())
      return nullptr;
    if (selectors.value.empty()) {
      add_error(peek(), "unable to parse case selectors");
      return nullptr;
    }
    stmt->set_selectors(std::move(selectors.value));
  }

  const char* use = "case statement";

  if (!expect(use, Token::Type::kColon))
    return nullptr;

  auto body = expect_brace_block_old(use, [&] { return case_body(); });

  if (body == nullptr)
    return nullptr;

  stmt->set_body(std::move(body));

  return stmt;
}

// case_selectors
//   : const_literal (COMMA const_literal)*
Expect<ast::CaseSelectorList> ParserImpl::expect_case_selectors() {
  ast::CaseSelectorList selectors;

  for (;;) {
    auto t = peek();
    auto cond = const_literal();
    if (has_error())
      return Failure::kErrored;
    if (cond == nullptr)
      break;
    if (!cond->IsInt())
      return add_error(t, "invalid case selector must be an integer value");

    std::unique_ptr<ast::IntLiteral> selector(cond.release()->AsInt());
    selectors.push_back(std::move(selector));
  }

  if (selectors.empty())
    return add_error(peek(), "unable to parse case selectors");

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

      if (!expect("fallthrough statement", Token::Type::kSemicolon))
        return nullptr;

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
  Source source;
  if (!match(Token::Type::kLoop, &source))
    return nullptr;

  return expect_brace_block_old(
      "loop", [&]() -> std::unique_ptr<ast::LoopStatement> {
        auto body = expect_statements();
        if (body.errored)
          return nullptr;

        auto continuing = continuing_stmt();
        if (has_error())
          return nullptr;

        return std::make_unique<ast::LoopStatement>(
            source, std::move(body.value), std::move(continuing));
      });
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
Expect<std::unique_ptr<ForHeader>> ParserImpl::expect_for_header() {
  std::unique_ptr<ast::Statement> initializer = nullptr;
  if (initializer == nullptr) {
    initializer = func_call_stmt();
    if (has_error()) {
      return Failure::kErrored;
    }
  }
  if (initializer == nullptr) {
    initializer = variable_stmt();
    if (has_error()) {
      return Failure::kErrored;
    }
  }
  if (initializer == nullptr) {
    initializer = assignment_stmt();
    if (has_error()) {
      return Failure::kErrored;
    }
  }

  if (!expect("initializer in for loop", Token::Type::kSemicolon))
    return Failure::kErrored;

  auto condition = logical_or_expression();
  if (has_error()) {
    return Failure::kErrored;
  }

  if (!expect("condition in for loop", Token::Type::kSemicolon))
    return Failure::kErrored;

  std::unique_ptr<ast::Statement> continuing = nullptr;
  if (continuing == nullptr) {
    continuing = func_call_stmt();
    if (has_error()) {
      return Failure::kErrored;
    }
  }
  if (continuing == nullptr) {
    continuing = assignment_stmt();
    if (has_error()) {
      return Failure::kErrored;
    }
  }

  return std::make_unique<ForHeader>(
      std::move(initializer), std::move(condition), std::move(continuing));
}

// for_statement
//   : FOR PAREN_LEFT for_header PAREN_RIGHT BRACE_LEFT statements BRACE_RIGHT
std::unique_ptr<ast::Statement> ParserImpl::for_stmt() {
  Source source;
  if (!match(Token::Type::kFor, &source))
    return nullptr;

  auto header =
      expect_paren_block("for loop", [&] { return expect_for_header(); });
  if (header.errored)
    return nullptr;

  auto body =
      expect_brace_block("for loop", [&] { return expect_statements(); });

  if (body.errored)
    return nullptr;

  // The for statement is a syntactic sugar on top of the loop statement.
  // We create corresponding nodes in ast with the exact same behaviour
  // as we would expect from the loop statement.
  if (header.value->condition != nullptr) {
    // !condition
    auto not_condition = std::make_unique<ast::UnaryOpExpression>(
        header.value->condition->source(), ast::UnaryOp::kNot,
        std::move(header.value->condition));
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
    body.value->insert(0, std::move(break_if_not_condition));
  }

  std::unique_ptr<ast::BlockStatement> continuing_body = nullptr;
  if (header.value->continuing != nullptr) {
    continuing_body = std::make_unique<ast::BlockStatement>(
        header.value->continuing->source());
    continuing_body->append(std::move(header.value->continuing));
  }

  auto loop = std::make_unique<ast::LoopStatement>(
      source, std::move(body.value), std::move(continuing_body));

  if (header.value->initializer != nullptr) {
    auto result = std::make_unique<ast::BlockStatement>(source);
    result->append(std::move(header.value->initializer));
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

  ast::ExpressionList params;

  t = peek();
  if (!t.IsParenRight() && !t.IsEof()) {
    auto list = expect_argument_expression_list();
    if (list.errored)
      return nullptr;
    params = std::move(list.value);
  }

  if (!expect("call statement", Token::Type::kParenRight))
    return nullptr;

  return std::make_unique<ast::CallStatement>(
      std::make_unique<ast::CallExpression>(
          source, std::make_unique<ast::IdentifierExpression>(name),
          std::move(params)));
}

// break_stmt
//   : BREAK
std::unique_ptr<ast::BreakStatement> ParserImpl::break_stmt() {
  Source source;
  if (!match(Token::Type::kBreak, &source))
    return nullptr;

  return std::make_unique<ast::BreakStatement>(source);
}

// continue_stmt
//   : CONTINUE
std::unique_ptr<ast::ContinueStatement> ParserImpl::continue_stmt() {
  Source source;
  if (!match(Token::Type::kContinue, &source))
    return nullptr;

  return std::make_unique<ast::ContinueStatement>(source);
}

// continuing_stmt
//   : CONTINUING body_stmt
std::unique_ptr<ast::BlockStatement> ParserImpl::continuing_stmt() {
  if (!match(Token::Type::kContinuing))
    return std::make_unique<ast::BlockStatement>();

  return expect_body_stmt().value;
}

// primary_expression
//   : IDENT
//   | type_decl PAREN_LEFT argument_expression_list* PAREN_RIGHT
//   | const_literal
//   | paren_rhs_stmt
//   | BITCAST LESS_THAN type_decl GREATER_THAN paren_rhs_stmt
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
    auto paren = expect_paren_rhs_stmt();
    if (paren.errored)
      return nullptr;

    return std::move(paren.value);
  }

  if (t.IsBitcast()) {
    auto src = t;

    next();  // Consume the peek

    t = next();
    if (!t.IsLessThan()) {
      add_error(t, "missing < for bitcast expression");
      return nullptr;
    }

    auto* type = type_decl();
    if (has_error())
      return nullptr;
    if (type == nullptr) {
      add_error(peek(), "missing type for bitcast expression");
      return nullptr;
    }

    t = next();
    if (!t.IsGreaterThan()) {
      add_error(t, "missing > for bitcast expression");
      return nullptr;
    }

    auto params = expect_paren_rhs_stmt();
    if (params.errored)
      return nullptr;

    return std::make_unique<ast::BitcastExpression>(source, type,
                                                    std::move(params.value));
  } else if (t.IsIdentifier()) {
    next();  // Consume the peek

    return std::make_unique<ast::IdentifierExpression>(source, t.to_str());
  }

  auto* type = type_decl();
  if (has_error())
    return nullptr;
  if (type != nullptr) {
    auto expr = expect_paren_block(
        "type constructor",
        [&]() -> Expect<std::unique_ptr<ast::TypeConstructorExpression>> {
          t = peek();
          if (t.IsParenRight() || t.IsEof())
            return std::make_unique<ast::TypeConstructorExpression>(
                source, type, ast::ExpressionList{});

          auto params = expect_argument_expression_list();
          if (params.errored)
            return Failure::kErrored;

          return std::make_unique<ast::TypeConstructorExpression>(
              source, type, std::move(params.value));
        });

    if (expr.errored)
      return nullptr;

    return std::move(expr.value);
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
      add_error(peek(), "unable to parse expression inside []");
      return nullptr;
    }

    t = next();
    if (!t.IsBracketRight()) {
      add_error(t, "missing ] for array accessor");
      return nullptr;
    }
    expr = std::make_unique<ast::ArrayAccessorExpression>(
        source, std::move(prefix), std::move(param));

  } else if (t.IsParenLeft()) {
    next();  // Consume the peek

    ast::ExpressionList params;

    t = peek();
    if (!t.IsParenRight() && !t.IsEof()) {
      auto list = expect_argument_expression_list();
      if (list.errored)
        return nullptr;
      params = std::move(list.value);
    }

    if (!expect("call expression", Token::Type::kParenRight))
      return nullptr;

    expr = std::make_unique<ast::CallExpression>(source, std::move(prefix),
                                                 std::move(params));
  } else if (t.IsPeriod()) {
    next();  // Consume the peek

    auto ident = expect_ident("member accessor");
    if (ident.errored)
      return nullptr;

    expr = std::make_unique<ast::MemberAccessorExpression>(
        ident.source, std::move(prefix),
        std::make_unique<ast::IdentifierExpression>(ident.source, ident.value));
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
Expect<ast::ExpressionList> ParserImpl::expect_argument_expression_list() {
  auto arg = logical_or_expression();
  if (has_error())
    return Failure::kErrored;
  if (arg == nullptr)
    return add_error(peek(), "unable to parse argument expression");

  ast::ExpressionList ret;
  ret.push_back(std::move(arg));

  while (match(Token::Type::kComma)) {
    arg = logical_or_expression();
    if (has_error())
      return Failure::kErrored;
    if (arg == nullptr) {
      return add_error(peek(),
                       "unable to parse argument expression after comma");
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
      add_error(peek(),
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
Expect<std::unique_ptr<ast::Expression>> ParserImpl::expect_multiplicative_expr(
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
    return Failure::kErrored;
  if (rhs == nullptr) {
    return add_error(peek(),
                     "unable to parse right side of " + name + " expression");
  }
  return expect_multiplicative_expr(std::make_unique<ast::BinaryExpression>(
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

  return expect_multiplicative_expr(std::move(lhs)).value;
}

// additive_expr
//   :
//   | PLUS multiplicative_expression additive_expr
//   | MINUS multiplicative_expression additive_expr
Expect<std::unique_ptr<ast::Expression>> ParserImpl::expect_additive_expr(
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
    return Failure::kErrored;
  if (rhs == nullptr)
    return add_error(peek(), "unable to parse right side of + expression");
  return expect_additive_expr(std::make_unique<ast::BinaryExpression>(
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

  return expect_additive_expr(std::move(lhs)).value;
}

// shift_expr
//   :
//   | LESS_THAN LESS_THAN additive_expression shift_expr
//   | GREATER_THAN GREATER_THAN additive_expression shift_expr
Expect<std::unique_ptr<ast::Expression>> ParserImpl::expect_shift_expr(
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
    return Failure::kErrored;
  if (rhs == nullptr) {
    return add_error(peek(), std::string("unable to parse right side of ") +
                                 name + " expression");
  }
  return expect_shift_expr(std::make_unique<ast::BinaryExpression>(
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

  return expect_shift_expr(std::move(lhs)).value;
}

// relational_expr
//   :
//   | LESS_THAN shift_expression relational_expr
//   | GREATER_THAN shift_expression relational_expr
//   | LESS_THAN_EQUAL shift_expression relational_expr
//   | GREATER_THAN_EQUAL shift_expression relational_expr
Expect<std::unique_ptr<ast::Expression>> ParserImpl::expect_relational_expr(
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
    return Failure::kErrored;
  if (rhs == nullptr) {
    return add_error(peek(),
                     "unable to parse right side of " + name + " expression");
  }

  return expect_relational_expr(std::make_unique<ast::BinaryExpression>(
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

  return expect_relational_expr(std::move(lhs)).value;
}

// equality_expr
//   :
//   | EQUAL_EQUAL relational_expression equality_expr
//   | NOT_EQUAL relational_expression equality_expr
Expect<std::unique_ptr<ast::Expression>> ParserImpl::expect_equality_expr(
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
    return Failure::kErrored;
  if (rhs == nullptr) {
    return add_error(peek(),
                     "unable to parse right side of " + name + " expression");
  }

  return expect_equality_expr(std::make_unique<ast::BinaryExpression>(
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

  return expect_equality_expr(std::move(lhs)).value;
}

// and_expr
//   :
//   | AND equality_expression and_expr
Expect<std::unique_ptr<ast::Expression>> ParserImpl::expect_and_expr(
    std::unique_ptr<ast::Expression> lhs) {
  auto t = peek();
  if (!t.IsAnd())
    return lhs;

  auto source = t.source();
  next();  // Consume the peek

  auto rhs = equality_expression();
  if (has_error())
    return Failure::kErrored;
  if (rhs == nullptr)
    return add_error(peek(), "unable to parse right side of & expression");

  return expect_and_expr(std::make_unique<ast::BinaryExpression>(
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

  return expect_and_expr(std::move(lhs)).value;
}

// exclusive_or_expr
//   :
//   | XOR and_expression exclusive_or_expr
Expect<std::unique_ptr<ast::Expression>> ParserImpl::expect_exclusive_or_expr(
    std::unique_ptr<ast::Expression> lhs) {
  auto t = peek();
  if (!t.IsXor())
    return lhs;

  auto source = t.source();
  next();  // Consume the peek

  auto rhs = and_expression();
  if (has_error())
    return Failure::kErrored;
  if (rhs == nullptr)
    return add_error(peek(), "unable to parse right side of ^ expression");

  return expect_exclusive_or_expr(std::make_unique<ast::BinaryExpression>(
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

  return expect_exclusive_or_expr(std::move(lhs)).value;
}

// inclusive_or_expr
//   :
//   | OR exclusive_or_expression inclusive_or_expr
Expect<std::unique_ptr<ast::Expression>> ParserImpl::expect_inclusive_or_expr(
    std::unique_ptr<ast::Expression> lhs) {
  auto t = peek();
  if (!t.IsOr())
    return lhs;

  auto source = t.source();
  next();  // Consume the peek

  auto rhs = exclusive_or_expression();
  if (has_error())
    return Failure::kErrored;
  if (rhs == nullptr)
    return add_error(peek(), "unable to parse right side of | expression");

  return expect_inclusive_or_expr(std::make_unique<ast::BinaryExpression>(
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

  return expect_inclusive_or_expr(std::move(lhs)).value;
}

// logical_and_expr
//   :
//   | AND_AND inclusive_or_expression logical_and_expr
Expect<std::unique_ptr<ast::Expression>> ParserImpl::expect_logical_and_expr(
    std::unique_ptr<ast::Expression> lhs) {
  auto t = peek();
  if (!t.IsAndAnd())
    return lhs;

  auto source = t.source();
  next();  // Consume the peek

  auto rhs = inclusive_or_expression();
  if (has_error())
    return Failure::kErrored;
  if (rhs == nullptr)
    return add_error(peek(), "unable to parse right side of && expression");

  return expect_logical_and_expr(std::make_unique<ast::BinaryExpression>(
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

  return expect_logical_and_expr(std::move(lhs)).value;
}

// logical_or_expr
//   :
//   | OR_OR logical_and_expression logical_or_expr
Expect<std::unique_ptr<ast::Expression>> ParserImpl::expect_logical_or_expr(
    std::unique_ptr<ast::Expression> lhs) {
  auto t = peek();
  if (!t.IsOrOr())
    return lhs;

  auto source = t.source();
  next();  // Consume the peek

  auto rhs = logical_and_expression();
  if (has_error())
    return Failure::kErrored;
  if (rhs == nullptr)
    return add_error(peek(), "unable to parse right side of || expression");

  return expect_logical_or_expr(std::make_unique<ast::BinaryExpression>(
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

  return expect_logical_or_expr(std::move(lhs)).value;
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
    add_error(t, "missing = for assignment");
    return nullptr;
  }

  auto rhs = logical_or_expression();
  if (has_error())
    return nullptr;
  if (rhs == nullptr) {
    add_error(peek(), "unable to parse right side of assignment");
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
  if (match(Token::Type::kTrue)) {
    auto* type = ctx_.type_mgr().Get(std::make_unique<ast::type::BoolType>());
    return std::make_unique<ast::BoolLiteral>(type, true);
  }
  if (match(Token::Type::kFalse)) {
    auto* type = ctx_.type_mgr().Get(std::make_unique<ast::type::BoolType>());
    return std::make_unique<ast::BoolLiteral>(type, false);
  }
  if (match(Token::Type::kSintLiteral)) {
    auto* type = ctx_.type_mgr().Get(std::make_unique<ast::type::I32Type>());
    return std::make_unique<ast::SintLiteral>(type, t.to_i32());
  }
  if (match(Token::Type::kUintLiteral)) {
    auto* type = ctx_.type_mgr().Get(std::make_unique<ast::type::U32Type>());
    return std::make_unique<ast::UintLiteral>(type, t.to_u32());
  }
  if (match(Token::Type::kFloatLiteral)) {
    auto* type = ctx_.type_mgr().Get(std::make_unique<ast::type::F32Type>());
    return std::make_unique<ast::FloatLiteral>(type, t.to_f32());
  }
  return nullptr;
}

// const_expr
//   : type_decl PAREN_LEFT (const_expr COMMA)? const_expr PAREN_RIGHT
//   | const_literal
Expect<std::unique_ptr<ast::ConstructorExpression>>
ParserImpl::expect_const_expr() {
  return expect_const_expr_internal(0);
}

Expect<std::unique_ptr<ast::ConstructorExpression>>
ParserImpl::expect_const_expr_internal(uint32_t depth) {
  auto t = peek();

  if (depth > kMaxConstExprDepth) {
    return add_error(t, "max const_expr depth reached");
  }

  auto source = t.source();

  auto* type = type_decl();
  if (type != nullptr) {
    auto params = expect_paren_block(
        "type constructor", [&]() -> Expect<ast::ExpressionList> {
          ast::ExpressionList list;
          auto param = expect_const_expr_internal(depth + 1);
          if (param.errored)
            return Failure::kErrored;
          list.emplace_back(std::move(param.value));
          while (match(Token::Type::kComma)) {
            param = expect_const_expr_internal(depth + 1);
            if (param.errored)
              return Failure::kErrored;
            list.emplace_back(std::move(param.value));
          }
          return list;
        });

    if (params.errored)
      return Failure::kErrored;

    return std::make_unique<ast::TypeConstructorExpression>(
        source, type, std::move(params.value));
  }

  auto lit = const_literal();
  if (has_error())
    return Failure::kErrored;
  if (lit == nullptr) {
    add_error(peek(), "unable to parse const literal");
    return Failure::kErrored;
  }
  return std::make_unique<ast::ScalarConstructorExpression>(source,
                                                            std::move(lit));
}

ast::DecorationList ParserImpl::decoration_list() {
  ast::DecorationList decos;
  while (decoration_bracketed_list(decos)) {
  }
  return decos;
}

bool ParserImpl::decoration_bracketed_list(ast::DecorationList& decos) {
  if (!match(Token::Type::kAttrLeft)) {
    return false;
  }

  Source source;
  if (match(Token::Type::kAttrRight, &source)) {
    add_error(source, "empty decoration list");
    return false;
  }

  while (true) {
    auto deco = expect_decoration();
    if (deco.errored)
      return false;

    decos.emplace_back(std::move(deco.value));

    if (match(Token::Type::kComma)) {
      continue;
    }

    if (is_decoration(peek())) {
      // We have two decorations in a bracket without a separating comma.
      // e.g. [[location(1) set(2)]]
      //                    ^^^ expected comma
      expect("decoration list", Token::Type::kComma);
      return false;
    }

    return expect("decoration list", Token::Type::kAttrRight);
  }
}

Expect<std::unique_ptr<ast::Decoration>> ParserImpl::expect_decoration() {
  auto t = peek();
  auto deco = decoration();
  if (has_error())
    return Failure::kErrored;
  if (deco == nullptr)
    return add_error(t, "expected decoration");
  return std::move(deco);
}

std::unique_ptr<ast::Decoration> ParserImpl::decoration() {
  using Result = std::unique_ptr<ast::Decoration>;
  auto t = next();
  if (t.IsLocation()) {
    const char* use = "location decoration";
    return expect_paren_block_old(use, [&]() -> Result {
      auto val = expect_positive_sint(use);
      if (val.errored)
        return nullptr;

      return std::make_unique<ast::LocationDecoration>(val.value, val.source);
    });
  }
  if (t.IsBinding()) {
    const char* use = "binding decoration";
    return expect_paren_block_old(use, [&]() -> Result {
      auto val = expect_positive_sint(use);
      if (val.errored)
        return nullptr;

      return std::make_unique<ast::BindingDecoration>(val.value, val.source);
    });
  }
  if (t.IsSet()) {
    const char* use = "set decoration";
    return expect_paren_block_old(use, [&]() -> Result {
      auto val = expect_positive_sint(use);
      if (val.errored)
        return nullptr;

      return std::make_unique<ast::SetDecoration>(val.value, val.source);
    });
  }
  if (t.IsBuiltin()) {
    return expect_paren_block_old("builtin decoration", [&]() -> Result {
      auto builtin = expect_builtin();
      if (builtin.errored)
        return nullptr;

      return std::make_unique<ast::BuiltinDecoration>(builtin.value,
                                                      builtin.source);
    });
  }
  if (t.IsWorkgroupSize()) {
    return expect_paren_block_old("workgroup_size decoration", [&]() -> Result {
      uint32_t x;
      uint32_t y = 1;
      uint32_t z = 1;

      auto val = expect_nonzero_positive_sint("workgroup_size x parameter");
      if (val.errored)
        return nullptr;
      x = val.value;

      if (match(Token::Type::kComma)) {
        val = expect_nonzero_positive_sint("workgroup_size y parameter");
        if (val.errored)
          return nullptr;
        y = val.value;

        if (match(Token::Type::kComma)) {
          val = expect_nonzero_positive_sint("workgroup_size z parameter");
          if (val.errored)
            return nullptr;
          z = val.value;
        }
      }

      return std::make_unique<ast::WorkgroupDecoration>(x, y, z, t.source());
    });
  }
  if (t.IsStage()) {
    return expect_paren_block_old("stage decoration", [&]() -> Result {
      auto stage = expect_pipeline_stage();
      if (stage.errored)
        return nullptr;

      return std::make_unique<ast::StageDecoration>(stage.value, stage.source);
    });
  }
  if (t.IsBlock()) {
    return std::make_unique<ast::StructBlockDecoration>(t.source());
  }
  if (t.IsStride()) {
    const char* use = "stride decoration";
    return expect_paren_block_old(use, [&]() -> Result {
      auto val = expect_nonzero_positive_sint(use);
      if (val.errored)
        return nullptr;

      return std::make_unique<ast::StrideDecoration>(val.value, t.source());
    });
  }
  if (t.IsOffset()) {
    const char* use = "offset decoration";
    return expect_paren_block_old(use, [&]() -> Result {
      auto val = expect_positive_sint(use);
      if (val.errored)
        return nullptr;

      return std::make_unique<ast::StructMemberOffsetDecoration>(val.value,
                                                                 t.source());
    });
  }
  return nullptr;
}

template <typename T>
Expect<std::vector<std::unique_ptr<T>>> ParserImpl::cast_decorations(
    ast::DecorationList& in) {
  bool ok = true;
  std::vector<std::unique_ptr<T>> out;
  out.reserve(in.size());
  for (auto& deco : in) {
    if (!deco->Is<T>()) {
      std::stringstream msg;
      msg << deco->GetKind() << " decoration type cannot be used for "
          << T::Kind;
      add_error(deco->GetSource(), msg.str());
      ok = false;
      continue;
    }
    out.emplace_back(ast::As<T>(std::move(deco)));
  }
  // clear in so that we can verify decorations were consumed with
  // expect_decorations_consumed()
  in.clear();

  if (!ok)
    return Failure::kErrored;

  return out;
}

bool ParserImpl::expect_decorations_consumed(const ast::DecorationList& in) {
  if (in.empty()) {
    return true;
  }
  add_error(in[0]->GetSource(), "unexpected decorations");
  return false;
}

bool ParserImpl::match(Token::Type tok, Source* source /*= nullptr*/) {
  auto t = peek();

  if (source != nullptr)
    *source = t.source();

  if (t.Is(tok)) {
    next();
    return true;
  }
  return false;
}

bool ParserImpl::expect(const std::string& use, Token::Type tok) {
  auto t = next();
  if (!t.Is(tok)) {
    std::stringstream err;
    err << "expected '" << Token::TypeToName(tok) << "'";
    if (!use.empty()) {
      err << " for " << use;
    }
    add_error(t, err.str());
    return false;
  }
  return true;
}

Expect<int32_t> ParserImpl::expect_sint(const std::string& use) {
  auto t = next();

  if (!t.IsSintLiteral())
    return add_error(t.source(), "expected signed integer literal", use);

  return {t.to_i32(), t.source()};
}

Expect<uint32_t> ParserImpl::expect_positive_sint(const std::string& use) {
  auto sint = expect_sint(use);
  if (sint.errored)
    return Failure::kErrored;

  if (sint.value < 0)
    return add_error(sint.source, use + " must be positive");

  return {static_cast<uint32_t>(sint.value), sint.source};
}

Expect<uint32_t> ParserImpl::expect_nonzero_positive_sint(
    const std::string& use) {
  auto sint = expect_sint(use);
  if (sint.errored)
    return Failure::kErrored;

  if (sint.value <= 0)
    return add_error(sint.source, use + " must be greater than 0");

  return {static_cast<uint32_t>(sint.value), sint.source};
}

Expect<std::string> ParserImpl::expect_ident(const std::string& use) {
  auto t = next();
  if (!t.IsIdentifier())
    return add_error(t.source(), "expected identifier", use);

  return {t.to_str(), t.source()};
}

template <typename F, typename T>
T ParserImpl::expect_block_old(Token::Type start,
                               Token::Type end,
                               const std::string& use,
                               F&& body) {
  if (!expect(use, start)) {
    return {};
  }
  auto res = body();
  if (has_error()) {
    return {};
  }
  if (!expect(use, end)) {
    return {};
  }
  return res;
}

template <typename F, typename T>
T ParserImpl::expect_block(Token::Type start,
                           Token::Type end,
                           const std::string& use,
                           F&& body) {
  if (!expect(use, start)) {
    return Failure::kErrored;
  }
  auto res = body();
  if (res.errored) {
    return Failure::kErrored;
  }
  if (!expect(use, end)) {
    return Failure::kErrored;
  }
  return res;
}

template <typename F, typename T>
T ParserImpl::expect_paren_block_old(const std::string& use, F&& body) {
  return expect_block_old(Token::Type::kParenLeft, Token::Type::kParenRight,
                          use, std::forward<F>(body));
}

template <typename F, typename T>
T ParserImpl::expect_brace_block_old(const std::string& use, F&& body) {
  return expect_block_old(Token::Type::kBraceLeft, Token::Type::kBraceRight,
                          use, std::forward<F>(body));
}

template <typename F, typename T>
T ParserImpl::expect_paren_block(const std::string& use, F&& body) {
  return expect_block(Token::Type::kParenLeft, Token::Type::kParenRight, use,
                      std::forward<F>(body));
}

template <typename F, typename T>
T ParserImpl::expect_brace_block(const std::string& use, F&& body) {
  return expect_block(Token::Type::kBraceLeft, Token::Type::kBraceRight, use,
                      std::forward<F>(body));
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
