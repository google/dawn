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

#include "src/ast/array.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/discard_statement.h"
#include "src/ast/external_texture.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/invariant_decoration.h"
#include "src/ast/loop_statement.h"
#include "src/ast/override_decoration.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type_name.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/vector.h"
#include "src/ast/workgroup_decoration.h"
#include "src/reader/wgsl/lexer.h"
#include "src/sem/depth_texture_type.h"
#include "src/sem/external_texture_type.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/sampled_texture_type.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

template <typename T>
using Expect = ParserImpl::Expect<T>;

template <typename T>
using Maybe = ParserImpl::Maybe<T>;

/// Controls the maximum number of times we'll call into the sync() and
/// unary_expression() functions from themselves. This is to guard against stack
/// overflow when there is an excessive number of blocks.
constexpr uint32_t kMaxParseDepth = 128;

/// The maximum number of tokens to look ahead to try and sync the
/// parser on error.
constexpr size_t const kMaxResynchronizeLookahead = 32;

const char kVertexStage[] = "vertex";
const char kFragmentStage[] = "fragment";
const char kComputeStage[] = "compute";

const char kReadAccess[] = "read";
const char kWriteAccess[] = "write";
const char kReadWriteAccess[] = "read_write";

ast::Builtin ident_to_builtin(const std::string& str) {
  if (str == "position") {
    return ast::Builtin::kPosition;
  }
  if (str == "vertex_index") {
    return ast::Builtin::kVertexIndex;
  }
  if (str == "instance_index") {
    return ast::Builtin::kInstanceIndex;
  }
  if (str == "front_facing") {
    return ast::Builtin::kFrontFacing;
  }
  if (str == "frag_depth") {
    return ast::Builtin::kFragDepth;
  }
  if (str == "local_invocation_id") {
    return ast::Builtin::kLocalInvocationId;
  }
  if (str == "local_invocation_idx" || str == "local_invocation_index") {
    return ast::Builtin::kLocalInvocationIndex;
  }
  if (str == "global_invocation_id") {
    return ast::Builtin::kGlobalInvocationId;
  }
  if (str == "workgroup_id") {
    return ast::Builtin::kWorkgroupId;
  }
  if (str == "num_workgroups") {
    return ast::Builtin::kNumWorkgroups;
  }
  if (str == "sample_index") {
    return ast::Builtin::kSampleIndex;
  }
  if (str == "sample_mask") {
    return ast::Builtin::kSampleMask;
  }
  return ast::Builtin::kNone;
}

const char kBindingDecoration[] = "binding";
const char kBlockDecoration[] = "block";
const char kBuiltinDecoration[] = "builtin";
const char kGroupDecoration[] = "group";
const char kInterpolateDecoration[] = "interpolate";
const char kInvariantDecoration[] = "invariant";
const char kLocationDecoration[] = "location";
const char kOverrideDecoration[] = "override";
const char kSizeDecoration[] = "size";
const char kAlignDecoration[] = "align";
const char kStageDecoration[] = "stage";
const char kStrideDecoration[] = "stride";
const char kWorkgroupSizeDecoration[] = "workgroup_size";

bool is_decoration(Token t) {
  if (!t.IsIdentifier()) {
    return false;
  }

  auto s = t.to_str();
  return s == kAlignDecoration || s == kBindingDecoration ||
         s == kBlockDecoration || s == kBuiltinDecoration ||
         s == kGroupDecoration || s == kInterpolateDecoration ||
         s == kLocationDecoration || s == kOverrideDecoration ||
         s == kSizeDecoration || s == kStageDecoration ||
         s == kStrideDecoration || s == kWorkgroupSizeDecoration;
}

// https://gpuweb.github.io/gpuweb/wgsl.html#reserved-keywords
bool is_reserved(Token t) {
  auto s = t.to_str();
  return s == "asm" || s == "bf16" || s == "const" || s == "do" ||
         s == "enum" || s == "f16" || s == "f64" || s == "handle" ||
         s == "i8" || s == "i16" || s == "i64" || s == "mat" ||
         s == "premerge" || s == "regardless" || s == "typedef" || s == "u8" ||
         s == "u16" || s == "u64" || s == "unless" || s == "using" ||
         s == "vec" || s == "void" || s == "while";
}

/// Enter-exit counters for block token types.
/// Used by sync_to() to skip over closing block tokens that were opened during
/// the forward scan.
struct BlockCounters {
  int attrs = 0;    // [[ ]]
  int brace = 0;    // {   }
  int bracket = 0;  // [   ]
  int paren = 0;    // (   )

  /// @return the current enter-exit depth for the given block token type. If
  /// `t` is not a block token type, then 0 is always returned.
  int consume(const Token& t) {
    if (t.Is(Token::Type::kAttrLeft))
      return attrs++;
    if (t.Is(Token::Type::kAttrRight))
      return attrs--;
    if (t.Is(Token::Type::kBraceLeft))
      return brace++;
    if (t.Is(Token::Type::kBraceRight))
      return brace--;
    if (t.Is(Token::Type::kBracketLeft))
      return bracket++;
    if (t.Is(Token::Type::kBracketRight))
      return bracket--;
    if (t.Is(Token::Type::kParenLeft))
      return paren++;
    if (t.Is(Token::Type::kParenRight))
      return paren--;
    return 0;
  }
};
}  // namespace

/// RAII helper that combines a Source on construction with the last token's
/// source when implicitly converted to `Source`.
class ParserImpl::MultiTokenSource {
 public:
  /// Constructor that starts with Source at the current peek position
  /// @param parser the parser
  explicit MultiTokenSource(ParserImpl* parser)
      : MultiTokenSource(parser, parser->peek().source().Begin()) {}

  /// Constructor that starts with the input `start` Source
  /// @param parser the parser
  /// @param start the start source of the range
  MultiTokenSource(ParserImpl* parser, const Source& start)
      : parser_(parser), start_(start) {}

  /// Implicit conversion to Source that returns the combined source from start
  /// to the current last token's source.
  operator Source() const {
    Source end = parser_->last_token().source().End();
    if (end < start_) {
      end = start_;
    }
    return Source::Combine(start_, end);
  }

 private:
  ParserImpl* parser_;
  Source start_;
};

ParserImpl::TypedIdentifier::TypedIdentifier() = default;

ParserImpl::TypedIdentifier::TypedIdentifier(const TypedIdentifier&) = default;

ParserImpl::TypedIdentifier::TypedIdentifier(const ast::Type* type_in,
                                             std::string name_in,
                                             Source source_in)
    : type(type_in), name(std::move(name_in)), source(std::move(source_in)) {}

ParserImpl::TypedIdentifier::~TypedIdentifier() = default;

ParserImpl::FunctionHeader::FunctionHeader() = default;

ParserImpl::FunctionHeader::FunctionHeader(const FunctionHeader&) = default;

ParserImpl::FunctionHeader::FunctionHeader(Source src,
                                           std::string n,
                                           ast::VariableList p,
                                           const ast::Type* ret_ty,
                                           ast::DecorationList ret_decos)
    : source(src),
      name(n),
      params(p),
      return_type(ret_ty),
      return_type_decorations(ret_decos) {}

ParserImpl::FunctionHeader::~FunctionHeader() = default;

ParserImpl::FunctionHeader& ParserImpl::FunctionHeader::operator=(
    const FunctionHeader& rhs) = default;

ParserImpl::VarDeclInfo::VarDeclInfo() = default;

ParserImpl::VarDeclInfo::VarDeclInfo(const VarDeclInfo&) = default;

ParserImpl::VarDeclInfo::VarDeclInfo(Source source_in,
                                     std::string name_in,
                                     ast::StorageClass storage_class_in,
                                     ast::Access access_in,
                                     const ast::Type* type_in)
    : source(std::move(source_in)),
      name(std::move(name_in)),
      storage_class(storage_class_in),
      access(access_in),
      type(type_in) {}

ParserImpl::VarDeclInfo::~VarDeclInfo() = default;

ParserImpl::ParserImpl(Source::File const* file)
    : lexer_(std::make_unique<Lexer>(file->path, &file->content)) {}

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
  if (silence_errors_ == 0) {
    builder_.Diagnostics().add_error(diag::System::Reader, err, source);
  }
  return Failure::kErrored;
}

void ParserImpl::deprecated(const Source& source, const std::string& msg) {
  builder_.Diagnostics().add_warning(
      diag::System::Reader, "use of deprecated language feature: " + msg,
      source);
}

Token ParserImpl::next() {
  if (!token_queue_.empty()) {
    auto t = token_queue_.front();
    token_queue_.pop_front();
    last_token_ = t;
    return last_token_;
  }
  last_token_ = lexer_->next();
  return last_token_;
}

Token ParserImpl::peek(size_t idx) {
  while (token_queue_.size() < (idx + 1))
    token_queue_.push_back(lexer_->next());
  return token_queue_[idx];
}

bool ParserImpl::peek_is(Token::Type tok, size_t idx) {
  return peek(idx).Is(tok);
}

Token ParserImpl::last_token() const {
  return last_token_;
}

bool ParserImpl::Parse() {
  translation_unit();
  return !has_error();
}

// translation_unit
//  : global_decl* EOF
void ParserImpl::translation_unit() {
  while (continue_parsing()) {
    auto p = peek();
    if (p.IsEof()) {
      break;
    }
    expect_global_decl();
    if (builder_.Diagnostics().error_count() >= max_errors_) {
      add_error(Source{{}, p.source().file_path},
                "stopping after " + std::to_string(max_errors_) + " errors");
      break;
    }
  }
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

  bool errored = false;

  auto decos = decoration_list();
  if (decos.errored)
    errored = true;
  if (!continue_parsing())
    return Failure::kErrored;

  auto decl = sync(Token::Type::kSemicolon, [&]() -> Maybe<bool> {
    auto gv = global_variable_decl(decos.value);
    if (gv.errored)
      return Failure::kErrored;
    if (gv.matched) {
      if (!expect("variable declaration", Token::Type::kSemicolon))
        return Failure::kErrored;

      builder_.AST().AddGlobalVariable(gv.value);
      return true;
    }

    auto gc = global_constant_decl(decos.value);
    if (gc.errored)
      return Failure::kErrored;

    if (gc.matched) {
      if (!expect("let declaration", Token::Type::kSemicolon))
        return Failure::kErrored;

      builder_.AST().AddGlobalVariable(gc.value);
      return true;
    }

    auto ta = type_alias();
    if (ta.errored)
      return Failure::kErrored;

    if (ta.matched) {
      if (!expect("type alias", Token::Type::kSemicolon))
        return Failure::kErrored;

      builder_.AST().AddTypeDecl(ta.value);
      return true;
    }

    auto str = struct_decl(decos.value);
    if (str.errored)
      return Failure::kErrored;

    if (str.matched) {
      if (!expect("struct declaration", Token::Type::kSemicolon))
        return Failure::kErrored;

      builder_.AST().AddTypeDecl(str.value);
      return true;
    }

    return Failure::kNoMatch;
  });

  if (decl.errored) {
    errored = true;
  }
  if (decl.matched) {
    return expect_decorations_consumed(decos.value);
  }

  auto func = function_decl(decos.value);
  if (func.errored) {
    errored = true;
  }
  if (func.matched) {
    builder_.AST().AddFunction(func.value);
    return true;
  }

  if (errored) {
    return Failure::kErrored;
  }

  // Invalid syntax found - try and determine the best error message

  // We have decorations parsed, but nothing to consume them?
  if (decos.value.size() > 0) {
    return add_error(next(), "expected declaration after decorations");
  }

  // We have a statement outside of a function?
  auto t = peek();
  auto stat = without_error([&] { return statement(); });
  if (stat.matched) {
    // Attempt to jump to the next '}' - the function might have just been
    // missing an opening line.
    sync_to(Token::Type::kBraceRight, true);
    return add_error(t, "statement found outside of function body");
  }
  if (!stat.errored) {
    // No match, no error - the parser might not have progressed.
    // Ensure we always make _some_ forward progress.
    next();
  }

  // The token might itself be an error.
  if (t.IsError()) {
    next();  // Consume it.
    return add_error(t.source(), t.to_str());
  }

  // Exhausted all attempts to make sense of where we're at.
  // Spew a generic error.

  return add_error(t, "unexpected token");
}

// global_variable_decl
//  : variable_decoration_list* variable_decl
//  | variable_decoration_list* variable_decl EQUAL const_expr
Maybe<const ast::Variable*> ParserImpl::global_variable_decl(
    ast::DecorationList& decos) {
  auto decl = variable_decl();
  if (decl.errored)
    return Failure::kErrored;
  if (!decl.matched)
    return Failure::kNoMatch;

  const ast::Expression* constructor = nullptr;
  if (match(Token::Type::kEqual)) {
    auto expr = expect_const_expr();
    if (expr.errored)
      return Failure::kErrored;
    constructor = expr.value;
  }

  return create<ast::Variable>(
      decl->source,                             // source
      builder_.Symbols().Register(decl->name),  // symbol
      decl->storage_class,                      // storage class
      decl->access,                             // access control
      decl->type,                               // type
      false,                                    // is_const
      constructor,                              // constructor
      std::move(decos));                        // decorations
}

// global_constant_decl
//  : attribute_list* LET variable_ident_decl global_const_initializer?
// global_const_initializer
//  : EQUAL const_expr
Maybe<const ast::Variable*> ParserImpl::global_constant_decl(
    ast::DecorationList& decos) {
  if (!match(Token::Type::kLet)) {
    return Failure::kNoMatch;
  }

  const char* use = "let declaration";

  auto decl = expect_variable_ident_decl(use, /* allow_inferred = */ true);
  if (decl.errored)
    return Failure::kErrored;

  const ast::Expression* initializer = nullptr;
  if (match(Token::Type::kEqual)) {
    auto init = expect_const_expr();
    if (init.errored) {
      return Failure::kErrored;
    }
    initializer = std::move(init.value);
  }

  return create<ast::Variable>(
      decl->source,                             // source
      builder_.Symbols().Register(decl->name),  // symbol
      ast::StorageClass::kNone,                 // storage class
      ast::Access::kUndefined,                  // access control
      decl->type,                               // type
      true,                                     // is_const
      initializer,                              // constructor
      std::move(decos));                        // decorations
}

// variable_decl
//   : VAR variable_qualifier? variable_ident_decl
Maybe<ParserImpl::VarDeclInfo> ParserImpl::variable_decl(bool allow_inferred) {
  Source source;
  if (!match(Token::Type::kVar, &source))
    return Failure::kNoMatch;

  VariableQualifier vq;
  auto explicit_vq = variable_qualifier();
  if (explicit_vq.errored)
    return Failure::kErrored;
  if (explicit_vq.matched) {
    vq = explicit_vq.value;
  }

  auto decl =
      expect_variable_ident_decl("variable declaration", allow_inferred);
  if (decl.errored)
    return Failure::kErrored;

  return VarDeclInfo{decl->source, decl->name, vq.storage_class, vq.access,
                     decl->type};
}

// texture_sampler_types
//  : sampler_type
//  | depth_texture_type
//  | sampled_texture_type LESS_THAN type_decl GREATER_THAN
//  | multisampled_texture_type LESS_THAN type_decl GREATER_THAN
//  | storage_texture_type LESS_THAN image_storage_type
//                         COMMA access GREATER_THAN
Maybe<const ast::Type*> ParserImpl::texture_sampler_types() {
  auto type = sampler_type();
  if (type.matched)
    return type;

  type = depth_texture_type();
  if (type.matched)
    return type;

  type = external_texture_type();
  if (type.matched)
    return type.value;

  auto source_range = make_source_range();

  auto dim = sampled_texture_type();
  if (dim.matched) {
    const char* use = "sampled texture type";

    auto subtype = expect_lt_gt_block(use, [&] { return expect_type(use); });
    if (subtype.errored)
      return Failure::kErrored;

    return builder_.ty.sampled_texture(source_range, dim.value, subtype.value);
  }

  auto ms_dim = multisampled_texture_type();
  if (ms_dim.matched) {
    const char* use = "multisampled texture type";

    auto subtype = expect_lt_gt_block(use, [&] { return expect_type(use); });
    if (subtype.errored)
      return Failure::kErrored;

    return builder_.ty.multisampled_texture(source_range, ms_dim.value,
                                            subtype.value);
  }

  auto storage = storage_texture_type();
  if (storage.matched) {
    const char* use = "storage texture type";
    using StorageTextureInfo =
        std::pair<tint::ast::ImageFormat, tint::ast::Access>;
    auto params = expect_lt_gt_block(use, [&]() -> Expect<StorageTextureInfo> {
      auto format = expect_image_storage_type(use);
      if (format.errored) {
        return Failure::kErrored;
      }

      if (!expect("access control", Token::Type::kComma)) {
        return Failure::kErrored;
      }

      auto access = expect_access("access control");
      if (access.errored) {
        return Failure::kErrored;
      }

      return std::make_pair(format.value, access.value);
    });

    if (params.errored) {
      return Failure::kErrored;
    }

    return builder_.ty.storage_texture(source_range, storage.value,
                                       params->first, params->second);
  }

  return Failure::kNoMatch;
}

// sampler_type
//  : SAMPLER
//  | SAMPLER_COMPARISON
Maybe<const ast::Type*> ParserImpl::sampler_type() {
  Source source;
  if (match(Token::Type::kSampler, &source))
    return builder_.ty.sampler(source, ast::SamplerKind::kSampler);

  if (match(Token::Type::kComparisonSampler, &source))
    return builder_.ty.sampler(source, ast::SamplerKind::kComparisonSampler);

  return Failure::kNoMatch;
}

// sampled_texture_type
//  : TEXTURE_SAMPLED_1D
//  | TEXTURE_SAMPLED_2D
//  | TEXTURE_SAMPLED_2D_ARRAY
//  | TEXTURE_SAMPLED_3D
//  | TEXTURE_SAMPLED_CUBE
//  | TEXTURE_SAMPLED_CUBE_ARRAY
Maybe<const ast::TextureDimension> ParserImpl::sampled_texture_type() {
  if (match(Token::Type::kTextureSampled1d))
    return ast::TextureDimension::k1d;

  if (match(Token::Type::kTextureSampled2d))
    return ast::TextureDimension::k2d;

  if (match(Token::Type::kTextureSampled2dArray))
    return ast::TextureDimension::k2dArray;

  if (match(Token::Type::kTextureSampled3d))
    return ast::TextureDimension::k3d;

  if (match(Token::Type::kTextureSampledCube))
    return ast::TextureDimension::kCube;

  if (match(Token::Type::kTextureSampledCubeArray))
    return ast::TextureDimension::kCubeArray;

  return Failure::kNoMatch;
}

// external_texture_type
//  : TEXTURE_EXTERNAL
Maybe<const ast::Type*> ParserImpl::external_texture_type() {
  Source source;
  if (match(Token::Type::kTextureExternal, &source)) {
    return builder_.ty.external_texture(source);
  }

  return Failure::kNoMatch;
}

// multisampled_texture_type
//  : TEXTURE_MULTISAMPLED_2D
Maybe<const ast::TextureDimension> ParserImpl::multisampled_texture_type() {
  if (match(Token::Type::kTextureMultisampled2d))
    return ast::TextureDimension::k2d;

  return Failure::kNoMatch;
}

// storage_texture_type
//  : TEXTURE_STORAGE_1D
//  | TEXTURE_STORAGE_2D
//  | TEXTURE_STORAGE_2D_ARRAY
//  | TEXTURE_STORAGE_3D
Maybe<const ast::TextureDimension> ParserImpl::storage_texture_type() {
  if (match(Token::Type::kTextureStorage1d))
    return ast::TextureDimension::k1d;
  if (match(Token::Type::kTextureStorage2d))
    return ast::TextureDimension::k2d;
  if (match(Token::Type::kTextureStorage2dArray))
    return ast::TextureDimension::k2dArray;
  if (match(Token::Type::kTextureStorage3d))
    return ast::TextureDimension::k3d;

  return Failure::kNoMatch;
}

// depth_texture_type
//  : TEXTURE_DEPTH_2D
//  | TEXTURE_DEPTH_2D_ARRAY
//  | TEXTURE_DEPTH_CUBE
//  | TEXTURE_DEPTH_CUBE_ARRAY
//  | TEXTURE_DEPTH_MULTISAMPLED_2D
Maybe<const ast::Type*> ParserImpl::depth_texture_type() {
  Source source;
  if (match(Token::Type::kTextureDepth2d, &source)) {
    return builder_.ty.depth_texture(source, ast::TextureDimension::k2d);
  }
  if (match(Token::Type::kTextureDepth2dArray, &source)) {
    return builder_.ty.depth_texture(source, ast::TextureDimension::k2dArray);
  }
  if (match(Token::Type::kTextureDepthCube, &source)) {
    return builder_.ty.depth_texture(source, ast::TextureDimension::kCube);
  }
  if (match(Token::Type::kTextureDepthCubeArray, &source)) {
    return builder_.ty.depth_texture(source, ast::TextureDimension::kCubeArray);
  }
  if (match(Token::Type::kTextureDepthMultisampled2d, &source)) {
    return builder_.ty.depth_multisampled_texture(source,
                                                  ast::TextureDimension::k2d);
  }
  return Failure::kNoMatch;
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
Expect<ast::ImageFormat> ParserImpl::expect_image_storage_type(
    const std::string& use) {
  if (match(Token::Type::kFormatR8Unorm))
    return ast::ImageFormat::kR8Unorm;

  if (match(Token::Type::kFormatR8Snorm))
    return ast::ImageFormat::kR8Snorm;

  if (match(Token::Type::kFormatR8Uint))
    return ast::ImageFormat::kR8Uint;

  if (match(Token::Type::kFormatR8Sint))
    return ast::ImageFormat::kR8Sint;

  if (match(Token::Type::kFormatR16Uint))
    return ast::ImageFormat::kR16Uint;

  if (match(Token::Type::kFormatR16Sint))
    return ast::ImageFormat::kR16Sint;

  if (match(Token::Type::kFormatR16Float))
    return ast::ImageFormat::kR16Float;

  if (match(Token::Type::kFormatRg8Unorm))
    return ast::ImageFormat::kRg8Unorm;

  if (match(Token::Type::kFormatRg8Snorm))
    return ast::ImageFormat::kRg8Snorm;

  if (match(Token::Type::kFormatRg8Uint))
    return ast::ImageFormat::kRg8Uint;

  if (match(Token::Type::kFormatRg8Sint))
    return ast::ImageFormat::kRg8Sint;

  if (match(Token::Type::kFormatR32Uint))
    return ast::ImageFormat::kR32Uint;

  if (match(Token::Type::kFormatR32Sint))
    return ast::ImageFormat::kR32Sint;

  if (match(Token::Type::kFormatR32Float))
    return ast::ImageFormat::kR32Float;

  if (match(Token::Type::kFormatRg16Uint))
    return ast::ImageFormat::kRg16Uint;

  if (match(Token::Type::kFormatRg16Sint))
    return ast::ImageFormat::kRg16Sint;

  if (match(Token::Type::kFormatRg16Float))
    return ast::ImageFormat::kRg16Float;

  if (match(Token::Type::kFormatRgba8Unorm))
    return ast::ImageFormat::kRgba8Unorm;

  if (match(Token::Type::kFormatRgba8UnormSrgb))
    return ast::ImageFormat::kRgba8UnormSrgb;

  if (match(Token::Type::kFormatRgba8Snorm))
    return ast::ImageFormat::kRgba8Snorm;

  if (match(Token::Type::kFormatRgba8Uint))
    return ast::ImageFormat::kRgba8Uint;

  if (match(Token::Type::kFormatRgba8Sint))
    return ast::ImageFormat::kRgba8Sint;

  if (match(Token::Type::kFormatBgra8Unorm))
    return ast::ImageFormat::kBgra8Unorm;

  if (match(Token::Type::kFormatBgra8UnormSrgb))
    return ast::ImageFormat::kBgra8UnormSrgb;

  if (match(Token::Type::kFormatRgb10A2Unorm))
    return ast::ImageFormat::kRgb10A2Unorm;

  if (match(Token::Type::kFormatRg11B10Float))
    return ast::ImageFormat::kRg11B10Float;

  if (match(Token::Type::kFormatRg32Uint))
    return ast::ImageFormat::kRg32Uint;

  if (match(Token::Type::kFormatRg32Sint))
    return ast::ImageFormat::kRg32Sint;

  if (match(Token::Type::kFormatRg32Float))
    return ast::ImageFormat::kRg32Float;

  if (match(Token::Type::kFormatRgba16Uint))
    return ast::ImageFormat::kRgba16Uint;

  if (match(Token::Type::kFormatRgba16Sint))
    return ast::ImageFormat::kRgba16Sint;

  if (match(Token::Type::kFormatRgba16Float))
    return ast::ImageFormat::kRgba16Float;

  if (match(Token::Type::kFormatRgba32Uint))
    return ast::ImageFormat::kRgba32Uint;

  if (match(Token::Type::kFormatRgba32Sint))
    return ast::ImageFormat::kRgba32Sint;

  if (match(Token::Type::kFormatRgba32Float))
    return ast::ImageFormat::kRgba32Float;

  return add_error(peek().source(), "invalid format", use);
}

// variable_ident_decl
//   : IDENT COLON variable_decoration_list* type_decl
Expect<ParserImpl::TypedIdentifier> ParserImpl::expect_variable_ident_decl(
    const std::string& use,
    bool allow_inferred) {
  auto ident = expect_ident(use);
  if (ident.errored)
    return Failure::kErrored;

  if (allow_inferred && !peek_is(Token::Type::kColon)) {
    return TypedIdentifier{nullptr, ident.value, ident.source};
  }

  if (!expect(use, Token::Type::kColon))
    return Failure::kErrored;

  auto decos = decoration_list();
  if (decos.errored)
    return Failure::kErrored;

  auto t = peek();
  auto type = type_decl(decos.value);
  if (type.errored)
    return Failure::kErrored;
  if (!type.matched)
    return add_error(t.source(), "invalid type", use);

  if (!expect_decorations_consumed(decos.value))
    return Failure::kErrored;

  return TypedIdentifier{type.value, ident.value, ident.source};
}

Expect<ast::Access> ParserImpl::expect_access(const std::string& use) {
  auto ident = expect_ident(use);
  if (ident.errored)
    return Failure::kErrored;

  if (ident.value == kReadAccess)
    return {ast::Access::kRead, ident.source};
  if (ident.value == kWriteAccess)
    return {ast::Access::kWrite, ident.source};
  if (ident.value == kReadWriteAccess)
    return {ast::Access::kReadWrite, ident.source};

  return add_error(ident.source, "invalid value for access control");
}

// variable_qualifier
//   : LESS_THAN storage_class (COMMA access_mode)? GREATER_THAN
Maybe<ParserImpl::VariableQualifier> ParserImpl::variable_qualifier() {
  if (!peek_is(Token::Type::kLessThan)) {
    return Failure::kNoMatch;
  }

  auto* use = "variable declaration";
  auto vq = expect_lt_gt_block(use, [&]() -> Expect<VariableQualifier> {
    auto source = make_source_range();
    auto sc = expect_storage_class(use);
    if (sc.errored) {
      return Failure::kErrored;
    }
    if (match(Token::Type::kComma)) {
      auto ac = expect_access(use);
      if (ac.errored) {
        return Failure::kErrored;
      }
      return VariableQualifier{sc.value, ac.value};
    }
    return Expect<VariableQualifier>{
        VariableQualifier{sc.value, ast::Access::kUndefined}, source};
  });

  if (vq.errored) {
    return Failure::kErrored;
  }

  return vq;
}

// type_alias
//   : TYPE IDENT EQUAL type_decl
Maybe<const ast::Alias*> ParserImpl::type_alias() {
  if (!peek_is(Token::Type::kType))
    return Failure::kNoMatch;

  auto t = next();
  const char* use = "type alias";

  auto name = expect_ident(use);
  if (name.errored)
    return Failure::kErrored;

  if (!expect(use, Token::Type::kEqual))
    return Failure::kErrored;

  auto type = type_decl();
  if (type.errored)
    return Failure::kErrored;
  if (!type.matched)
    return add_error(peek(), "invalid type alias");

  return builder_.ty.alias(make_source_range_from(t.source()), name.value,
                           type.value);
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
//   | PTR LESS_THAN storage_class, type_decl (COMMA access_mode)? GREATER_THAN
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
Maybe<const ast::Type*> ParserImpl::type_decl() {
  auto decos = decoration_list();
  if (decos.errored)
    return Failure::kErrored;

  auto type = type_decl(decos.value);
  if (type.errored) {
    return Failure::kErrored;
  }
  if (!expect_decorations_consumed(decos.value)) {
    return Failure::kErrored;
  }
  if (!type.matched) {
    return Failure::kNoMatch;
  }

  return type;
}

Maybe<const ast::Type*> ParserImpl::type_decl(ast::DecorationList& decos) {
  auto t = peek();
  Source source;
  if (match(Token::Type::kIdentifier, &source)) {
    return builder_.create<ast::TypeName>(
        source, builder_.Symbols().Register(t.to_str()));
  }

  if (match(Token::Type::kBool, &source))
    return builder_.ty.bool_(source);

  if (match(Token::Type::kF32, &source))
    return builder_.ty.f32(source);

  if (match(Token::Type::kI32, &source))
    return builder_.ty.i32(source);

  if (match(Token::Type::kU32, &source))
    return builder_.ty.u32(source);

  if (t.IsVector()) {
    next();  // Consume the peek
    return expect_type_decl_vector(t);
  }

  if (match(Token::Type::kPtr)) {
    return expect_type_decl_pointer(t);
  }

  if (match(Token::Type::kAtomic)) {
    return expect_type_decl_atomic(t);
  }

  if (match(Token::Type::kArray, &source)) {
    return expect_type_decl_array(t, std::move(decos));
  }

  if (t.IsMatrix()) {
    next();  // Consume the peek
    return expect_type_decl_matrix(t);
  }

  auto texture_or_sampler = texture_sampler_types();
  if (texture_or_sampler.errored)
    return Failure::kErrored;
  if (texture_or_sampler.matched)
    return texture_or_sampler;

  return Failure::kNoMatch;
}

Expect<const ast::Type*> ParserImpl::expect_type(const std::string& use) {
  auto type = type_decl();
  if (type.errored)
    return Failure::kErrored;
  if (!type.matched)
    return add_error(peek().source(), "invalid type", use);
  return type.value;
}

Expect<const ast::Type*> ParserImpl::expect_type_decl_pointer(Token t) {
  const char* use = "ptr declaration";

  auto storage_class = ast::StorageClass::kNone;
  auto access = ast::Access::kUndefined;

  auto subtype = expect_lt_gt_block(use, [&]() -> Expect<const ast::Type*> {
    auto sc = expect_storage_class(use);
    if (sc.errored) {
      return Failure::kErrored;
    }
    storage_class = sc.value;

    if (!expect(use, Token::Type::kComma)) {
      return Failure::kErrored;
    }

    auto type = expect_type(use);
    if (type.errored) {
      return Failure::kErrored;
    }

    if (match(Token::Type::kComma)) {
      auto ac = expect_access("access control");
      if (ac.errored) {
        return Failure::kErrored;
      }
      access = ac.value;
    }

    return type.value;
  });

  if (subtype.errored) {
    return Failure::kErrored;
  }

  return builder_.ty.pointer(make_source_range_from(t.source()), subtype.value,
                             storage_class, access);
}

Expect<const ast::Type*> ParserImpl::expect_type_decl_atomic(Token t) {
  const char* use = "atomic declaration";

  auto subtype = expect_lt_gt_block(use, [&] { return expect_type(use); });
  if (subtype.errored) {
    return Failure::kErrored;
  }

  return builder_.ty.atomic(make_source_range_from(t.source()), subtype.value);
}

Expect<const ast::Type*> ParserImpl::expect_type_decl_vector(Token t) {
  uint32_t count = 2;
  if (t.Is(Token::Type::kVec3))
    count = 3;
  else if (t.Is(Token::Type::kVec4))
    count = 4;

  const char* use = "vector";

  auto subtype = expect_lt_gt_block(use, [&] { return expect_type(use); });
  if (subtype.errored)
    return Failure::kErrored;

  return builder_.ty.vec(make_source_range_from(t.source()), subtype.value,
                         count);
}

Expect<const ast::Type*> ParserImpl::expect_type_decl_array(
    Token t,
    ast::DecorationList decos) {
  const char* use = "array declaration";

  const ast::Expression* size = nullptr;

  auto subtype = expect_lt_gt_block(use, [&]() -> Expect<const ast::Type*> {
    auto type = expect_type(use);
    if (type.errored)
      return Failure::kErrored;

    if (match(Token::Type::kComma)) {
      auto expr = primary_expression();
      if (expr.errored) {
        return Failure::kErrored;
      } else if (!expr.matched) {
        return add_error(peek(), "expected array size expression");
      }

      size = std::move(expr.value);
    }

    return type.value;
  });

  if (subtype.errored) {
    return Failure::kErrored;
  }

  return builder_.ty.array(make_source_range_from(t.source()), subtype.value,
                           size, std::move(decos));
}

Expect<const ast::Type*> ParserImpl::expect_type_decl_matrix(Token t) {
  uint32_t rows = 2;
  uint32_t columns = 2;
  if (t.IsMat3xN()) {
    columns = 3;
  } else if (t.IsMat4xN()) {
    columns = 4;
  }
  if (t.IsMatNx3()) {
    rows = 3;
  } else if (t.IsMatNx4()) {
    rows = 4;
  }

  const char* use = "matrix";

  auto subtype = expect_lt_gt_block(use, [&] { return expect_type(use); });
  if (subtype.errored)
    return Failure::kErrored;

  return builder_.ty.mat(make_source_range_from(t.source()), subtype.value,
                         columns, rows);
}

// storage_class
//  : INPUT
//  | OUTPUT
//  | UNIFORM
//  | WORKGROUP
//  | STORAGE
//  | IMAGE
//  | PRIVATE
//  | FUNCTION
Expect<ast::StorageClass> ParserImpl::expect_storage_class(
    const std::string& use) {
  auto source = peek().source();

  if (match(Token::Type::kUniform))
    return {ast::StorageClass::kUniform, source};

  if (match(Token::Type::kWorkgroup))
    return {ast::StorageClass::kWorkgroup, source};

  if (match(Token::Type::kStorage))
    return {ast::StorageClass::kStorage, source};

  if (match(Token::Type::kImage))
    return {ast::StorageClass::kImage, source};

  if (match(Token::Type::kPrivate))
    return {ast::StorageClass::kPrivate, source};

  if (match(Token::Type::kFunction))
    return {ast::StorageClass::kFunction, source};

  return add_error(source, "invalid storage class", use);
}

// struct_decl
//   : struct_decoration_decl* STRUCT IDENT struct_body_decl
Maybe<const ast::Struct*> ParserImpl::struct_decl(ast::DecorationList& decos) {
  auto t = peek();
  auto source = t.source();

  if (!match(Token::Type::kStruct))
    return Failure::kNoMatch;

  auto name = expect_ident("struct declaration");
  if (name.errored)
    return Failure::kErrored;

  auto body = expect_struct_body_decl();
  if (body.errored)
    return Failure::kErrored;

  auto sym = builder_.Symbols().Register(name.value);
  return create<ast::Struct>(source, sym, std::move(body.value),
                             std::move(decos));
}

// struct_body_decl
//   : BRACKET_LEFT struct_member* BRACKET_RIGHT
Expect<ast::StructMemberList> ParserImpl::expect_struct_body_decl() {
  return expect_brace_block(
      "struct declaration", [&]() -> Expect<ast::StructMemberList> {
        bool errored = false;

        ast::StructMemberList members;

        while (continue_parsing() && !peek_is(Token::Type::kBraceRight) &&
               !peek_is(Token::Type::kEOF)) {
          auto member = sync(Token::Type::kSemicolon,
                             [&]() -> Expect<ast::StructMember*> {
                               auto decos = decoration_list();
                               if (decos.errored) {
                                 errored = true;
                               }
                               if (!synchronized_) {
                                 return Failure::kErrored;
                               }
                               return expect_struct_member(decos.value);
                             });

          if (member.errored) {
            errored = true;
          } else {
            members.push_back(member.value);
          }
        }

        if (errored)
          return Failure::kErrored;

        return members;
      });
}

// struct_member
//   : struct_member_decoration_decl+ variable_ident_decl SEMICOLON
Expect<ast::StructMember*> ParserImpl::expect_struct_member(
    ast::DecorationList& decos) {
  auto decl = expect_variable_ident_decl("struct member");
  if (decl.errored)
    return Failure::kErrored;

  if (!expect("struct member", Token::Type::kSemicolon))
    return Failure::kErrored;

  return create<ast::StructMember>(decl->source,
                                   builder_.Symbols().Register(decl->name),
                                   decl->type, std::move(decos));
}

// function_decl
//   : function_header body_stmt
Maybe<const ast::Function*> ParserImpl::function_decl(
    ast::DecorationList& decos) {
  auto header = function_header();
  if (header.errored) {
    if (sync_to(Token::Type::kBraceLeft, /* consume: */ false)) {
      // There were errors in the function header, but the parser has managed to
      // resynchronize with the opening brace. As there's no outer
      // synchronization token for function declarations, attempt to parse the
      // function body. The AST isn't used as we've already errored, but this
      // catches any errors inside the body, and can help keep the parser in
      // sync.
      expect_body_stmt();
    }
    return Failure::kErrored;
  }
  if (!header.matched)
    return Failure::kNoMatch;

  bool errored = false;

  auto body = expect_body_stmt();
  if (body.errored)
    errored = true;

  if (errored)
    return Failure::kErrored;

  return create<ast::Function>(
      header->source, builder_.Symbols().Register(header->name), header->params,
      header->return_type, body.value, decos, header->return_type_decorations);
}

// function_header
//   : FN IDENT PAREN_LEFT param_list PAREN_RIGHT return_type_decl_optional
// return_type_decl_optional
//   :
//   | ARROW attribute_list* type_decl
Maybe<ParserImpl::FunctionHeader> ParserImpl::function_header() {
  Source source;
  if (!match(Token::Type::kFn, &source)) {
    return Failure::kNoMatch;
  }

  const char* use = "function declaration";
  bool errored = false;

  auto name = expect_ident(use);
  if (name.errored) {
    errored = true;
    if (!sync_to(Token::Type::kParenLeft, /* consume: */ false)) {
      return Failure::kErrored;
    }
  }

  auto params = expect_paren_block(use, [&] { return expect_param_list(); });
  if (params.errored) {
    errored = true;
    if (!synchronized_) {
      return Failure::kErrored;
    }
  }

  const ast::Type* return_type = nullptr;
  ast::DecorationList return_decorations;

  if (match(Token::Type::kArrow)) {
    auto decos = decoration_list();
    if (decos.errored) {
      return Failure::kErrored;
    }
    return_decorations = decos.value;

    // Apply stride decorations to the type node instead of the function.
    ast::DecorationList type_decorations;
    auto itr = std::find_if(
        return_decorations.begin(), return_decorations.end(),
        [](auto* deco) { return Is<ast::StrideDecoration>(deco); });
    if (itr != return_decorations.end()) {
      type_decorations.emplace_back(*itr);
      return_decorations.erase(itr);
    }

    auto tok = peek();

    auto type = type_decl(type_decorations);
    if (type.errored) {
      errored = true;
    } else if (!type.matched) {
      return add_error(peek(), "unable to determine function return type");
    } else {
      return_type = type.value;
    }
  } else {
    return_type = builder_.ty.void_();
  }

  if (errored) {
    return Failure::kErrored;
  }

  return FunctionHeader{source, name.value, std::move(params.value),
                        return_type, std::move(return_decorations)};
}

// param_list
//   :
//   | (param COMMA)* param COMMA?
Expect<ast::VariableList> ParserImpl::expect_param_list() {
  ast::VariableList ret;
  while (continue_parsing()) {
    // Check for the end of the list.
    auto t = peek();
    if (!t.IsIdentifier() && !t.Is(Token::Type::kAttrLeft)) {
      break;
    }

    auto param = expect_param();
    if (param.errored)
      return Failure::kErrored;
    ret.push_back(param.value);

    if (!match(Token::Type::kComma))
      break;
  }

  return ret;
}

// param
//   : decoration_list* variable_ident_decl
Expect<ast::Variable*> ParserImpl::expect_param() {
  auto decos = decoration_list();

  auto decl = expect_variable_ident_decl("parameter");
  if (decl.errored)
    return Failure::kErrored;

  auto* var =
      create<ast::Variable>(decl->source,                             // source
                            builder_.Symbols().Register(decl->name),  // symbol
                            ast::StorageClass::kNone,  // storage class
                            ast::Access::kUndefined,   // access control
                            decl->type,                // type
                            true,                      // is_const
                            nullptr,                   // constructor
                            std::move(decos.value));   // decorations
  // Formal parameters are treated like a const declaration where the
  // initializer value is provided by the call's argument.  The key point is
  // that it's not updatable after initially set.  This is unlike C or GLSL
  // which treat formal parameters like local variables that can be updated.

  return var;
}

// pipeline_stage
//   : VERTEX
//   | FRAGMENT
//   | COMPUTE
Expect<ast::PipelineStage> ParserImpl::expect_pipeline_stage() {
  auto t = peek();
  if (!t.IsIdentifier()) {
    return add_error(t, "invalid value for stage decoration");
  }

  auto s = t.to_str();
  if (s == kVertexStage) {
    next();  // Consume the peek
    return {ast::PipelineStage::kVertex, t.source()};
  }
  if (s == kFragmentStage) {
    next();  // Consume the peek
    return {ast::PipelineStage::kFragment, t.source()};
  }
  if (s == kComputeStage) {
    next();  // Consume the peek
    return {ast::PipelineStage::kCompute, t.source()};
  }

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
Expect<ast::BlockStatement*> ParserImpl::expect_body_stmt() {
  return expect_brace_block("", [&]() -> Expect<ast::BlockStatement*> {
    auto stmts = expect_statements();
    if (stmts.errored)
      return Failure::kErrored;
    return create<ast::BlockStatement>(Source{}, stmts.value);
  });
}

// paren_rhs_stmt
//   : PAREN_LEFT logical_or_expression PAREN_RIGHT
Expect<const ast::Expression*> ParserImpl::expect_paren_rhs_stmt() {
  return expect_paren_block("", [&]() -> Expect<const ast::Expression*> {
    auto expr = logical_or_expression();
    if (expr.errored)
      return Failure::kErrored;
    if (!expr.matched)
      return add_error(peek(), "unable to parse expression");

    return expr.value;
  });
}

// statements
//   : statement*
Expect<ast::StatementList> ParserImpl::expect_statements() {
  bool errored = false;
  ast::StatementList stmts;

  while (continue_parsing()) {
    auto stmt = statement();
    if (stmt.errored) {
      errored = true;
    } else if (stmt.matched) {
      stmts.emplace_back(stmt.value);
    } else {
      break;
    }
  }

  if (errored)
    return Failure::kErrored;

  return stmts;
}

// statement
//   : SEMICOLON
//   | body_stmt?
//   | if_stmt
//   | switch_stmt
//   | loop_stmt
//   | for_stmt
//   | non_block_statement
//      : return_stmt SEMICOLON
//      | func_call_stmt SEMICOLON
//      | variable_stmt SEMICOLON
//      | break_stmt SEMICOLON
//      | continue_stmt SEMICOLON
//      | DISCARD SEMICOLON
//      | assignment_stmt SEMICOLON
Maybe<const ast::Statement*> ParserImpl::statement() {
  while (match(Token::Type::kSemicolon)) {
    // Skip empty statements
  }

  // Non-block statments that error can resynchronize on semicolon.
  auto stmt =
      sync(Token::Type::kSemicolon, [&] { return non_block_statement(); });

  if (stmt.errored)
    return Failure::kErrored;
  if (stmt.matched)
    return stmt;

  auto stmt_if = if_stmt();
  if (stmt_if.errored)
    return Failure::kErrored;
  if (stmt_if.matched)
    return stmt_if.value;

  auto sw = switch_stmt();
  if (sw.errored)
    return Failure::kErrored;
  if (sw.matched)
    return sw.value;

  auto loop = loop_stmt();
  if (loop.errored)
    return Failure::kErrored;
  if (loop.matched)
    return loop.value;

  auto stmt_for = for_stmt();
  if (stmt_for.errored)
    return Failure::kErrored;
  if (stmt_for.matched)
    return stmt_for.value;

  if (peek_is(Token::Type::kBraceLeft)) {
    auto body = expect_body_stmt();
    if (body.errored)
      return Failure::kErrored;
    return body.value;
  }

  return Failure::kNoMatch;
}

// statement (continued)
//   : return_stmt SEMICOLON
//   | func_call_stmt SEMICOLON
//   | variable_stmt SEMICOLON
//   | break_stmt SEMICOLON
//   | continue_stmt SEMICOLON
//   | DISCARD SEMICOLON
//   | assignment_stmt SEMICOLON
Maybe<const ast::Statement*> ParserImpl::non_block_statement() {
  auto stmt = [&]() -> Maybe<const ast::Statement*> {
    auto ret_stmt = return_stmt();
    if (ret_stmt.errored)
      return Failure::kErrored;
    if (ret_stmt.matched)
      return ret_stmt.value;

    auto func = func_call_stmt();
    if (func.errored)
      return Failure::kErrored;
    if (func.matched)
      return func.value;

    auto var = variable_stmt();
    if (var.errored)
      return Failure::kErrored;
    if (var.matched)
      return var.value;

    auto b = break_stmt();
    if (b.errored)
      return Failure::kErrored;
    if (b.matched)
      return b.value;

    auto cont = continue_stmt();
    if (cont.errored)
      return Failure::kErrored;
    if (cont.matched)
      return cont.value;

    auto assign = assignment_stmt();
    if (assign.errored)
      return Failure::kErrored;
    if (assign.matched)
      return assign.value;

    Source source;
    if (match(Token::Type::kDiscard, &source))
      return create<ast::DiscardStatement>(source);

    return Failure::kNoMatch;
  }();

  if (stmt.matched && !expect(stmt->Name(), Token::Type::kSemicolon))
    return Failure::kErrored;

  return stmt;
}

// return_stmt
//   : RETURN logical_or_expression?
Maybe<const ast::ReturnStatement*> ParserImpl::return_stmt() {
  Source source;
  if (!match(Token::Type::kReturn, &source))
    return Failure::kNoMatch;

  if (peek_is(Token::Type::kSemicolon))
    return create<ast::ReturnStatement>(source, nullptr);

  auto expr = logical_or_expression();
  if (expr.errored)
    return Failure::kErrored;

  // TODO(bclayton): Check matched?
  return create<ast::ReturnStatement>(source, expr.value);
}

// variable_stmt
//   : variable_decl
//   | variable_decl EQUAL logical_or_expression
//   | CONST variable_ident_decl EQUAL logical_or_expression
Maybe<const ast::VariableDeclStatement*> ParserImpl::variable_stmt() {
  if (match(Token::Type::kLet)) {
    auto decl = expect_variable_ident_decl("let declaration",
                                           /*allow_inferred = */ true);
    if (decl.errored)
      return Failure::kErrored;

    if (!expect("let declaration", Token::Type::kEqual))
      return Failure::kErrored;

    auto constructor = logical_or_expression();
    if (constructor.errored)
      return Failure::kErrored;
    if (!constructor.matched)
      return add_error(peek(), "missing constructor for let declaration");

    auto* var = create<ast::Variable>(
        decl->source,                             // source
        builder_.Symbols().Register(decl->name),  // symbol
        ast::StorageClass::kNone,                 // storage class
        ast::Access::kUndefined,                  // access control
        decl->type,                               // type
        true,                                     // is_const
        constructor.value,                        // constructor
        ast::DecorationList{});                   // decorations

    return create<ast::VariableDeclStatement>(decl->source, var);
  }

  auto decl = variable_decl(/*allow_inferred = */ true);
  if (decl.errored)
    return Failure::kErrored;
  if (!decl.matched)
    return Failure::kNoMatch;

  const ast::Expression* constructor = nullptr;
  if (match(Token::Type::kEqual)) {
    auto constructor_expr = logical_or_expression();
    if (constructor_expr.errored)
      return Failure::kErrored;
    if (!constructor_expr.matched)
      return add_error(peek(), "missing constructor for variable declaration");

    constructor = constructor_expr.value;
  }

  auto* var =
      create<ast::Variable>(decl->source,                             // source
                            builder_.Symbols().Register(decl->name),  // symbol
                            decl->storage_class,     // storage class
                            decl->access,            // access control
                            decl->type,              // type
                            false,                   // is_const
                            constructor,             // constructor
                            ast::DecorationList{});  // decorations

  return create<ast::VariableDeclStatement>(var->source, var);
}

// if_stmt
//   : IF paren_rhs_stmt body_stmt elseif_stmt? else_stmt?
Maybe<const ast::IfStatement*> ParserImpl::if_stmt() {
  Source source;
  if (!match(Token::Type::kIf, &source))
    return Failure::kNoMatch;

  auto condition = expect_paren_rhs_stmt();
  if (condition.errored)
    return Failure::kErrored;

  auto body = expect_body_stmt();
  if (body.errored)
    return Failure::kErrored;

  auto elseif = elseif_stmt();
  if (elseif.errored)
    return Failure::kErrored;

  auto el = else_stmt();
  if (el.errored)
    return Failure::kErrored;
  if (el.matched)
    elseif.value.push_back(el.value);

  return create<ast::IfStatement>(source, condition.value, body.value,
                                  elseif.value);
}

// elseif_stmt
//   : ELSE_IF paren_rhs_stmt body_stmt elseif_stmt?
Maybe<ast::ElseStatementList> ParserImpl::elseif_stmt() {
  Source source;
  if (!match(Token::Type::kElseIf, &source))
    return Failure::kNoMatch;

  ast::ElseStatementList ret;
  while (continue_parsing()) {
    auto condition = expect_paren_rhs_stmt();
    if (condition.errored)
      return Failure::kErrored;

    auto body = expect_body_stmt();
    if (body.errored)
      return Failure::kErrored;

    ret.push_back(
        create<ast::ElseStatement>(source, condition.value, body.value));

    if (!match(Token::Type::kElseIf, &source))
      break;
  }

  return ret;
}

// else_stmt
//   : ELSE body_stmt
Maybe<const ast::ElseStatement*> ParserImpl::else_stmt() {
  Source source;
  if (!match(Token::Type::kElse, &source))
    return Failure::kNoMatch;

  auto body = expect_body_stmt();
  if (body.errored)
    return Failure::kErrored;

  return create<ast::ElseStatement>(source, nullptr, body.value);
}

// switch_stmt
//   : SWITCH paren_rhs_stmt BRACKET_LEFT switch_body+ BRACKET_RIGHT
Maybe<const ast::SwitchStatement*> ParserImpl::switch_stmt() {
  Source source;
  if (!match(Token::Type::kSwitch, &source))
    return Failure::kNoMatch;

  auto condition = expect_paren_rhs_stmt();
  if (condition.errored)
    return Failure::kErrored;

  auto body = expect_brace_block("switch statement",
                                 [&]() -> Expect<ast::CaseStatementList> {
                                   bool errored = false;
                                   ast::CaseStatementList list;
                                   while (continue_parsing()) {
                                     auto stmt = switch_body();
                                     if (stmt.errored) {
                                       errored = true;
                                       continue;
                                     }
                                     if (!stmt.matched)
                                       break;
                                     list.push_back(stmt.value);
                                   }
                                   if (errored)
                                     return Failure::kErrored;
                                   return list;
                                 });

  if (body.errored)
    return Failure::kErrored;

  return create<ast::SwitchStatement>(source, condition.value, body.value);
}

// switch_body
//   : CASE case_selectors COLON BRACKET_LEFT case_body BRACKET_RIGHT
//   | DEFAULT COLON BRACKET_LEFT case_body BRACKET_RIGHT
Maybe<const ast::CaseStatement*> ParserImpl::switch_body() {
  if (!peek_is(Token::Type::kCase) && !peek_is(Token::Type::kDefault))
    return Failure::kNoMatch;

  auto t = next();
  auto source = t.source();

  ast::CaseSelectorList selector_list;
  if (t.Is(Token::Type::kCase)) {
    auto selectors = expect_case_selectors();
    if (selectors.errored)
      return Failure::kErrored;

    selector_list = std::move(selectors.value);
  }

  const char* use = "case statement";

  if (!expect(use, Token::Type::kColon))
    return Failure::kErrored;

  auto body = expect_brace_block(use, [&] { return case_body(); });

  if (body.errored)
    return Failure::kErrored;
  if (!body.matched)
    return add_error(body.source, "expected case body");

  return create<ast::CaseStatement>(source, selector_list, body.value);
}

// case_selectors
//   : const_literal (COMMA const_literal)* COMMA?
Expect<ast::CaseSelectorList> ParserImpl::expect_case_selectors() {
  ast::CaseSelectorList selectors;

  while (continue_parsing()) {
    auto cond = const_literal();
    if (cond.errored) {
      return Failure::kErrored;
    } else if (!cond.matched) {
      break;
    } else if (!cond->Is<ast::IntLiteralExpression>()) {
      return add_error(cond.value->source,
                       "invalid case selector must be an integer value");
    }

    selectors.push_back(cond.value->As<ast::IntLiteralExpression>());

    if (!match(Token::Type::kComma)) {
      break;
    }
  }

  if (selectors.empty())
    return add_error(peek(), "unable to parse case selectors");

  return selectors;
}

// case_body
//   :
//   | statement case_body
//   | FALLTHROUGH SEMICOLON
Maybe<const ast::BlockStatement*> ParserImpl::case_body() {
  ast::StatementList stmts;
  while (continue_parsing()) {
    Source source;
    if (match(Token::Type::kFallthrough, &source)) {
      if (!expect("fallthrough statement", Token::Type::kSemicolon))
        return Failure::kErrored;

      stmts.emplace_back(create<ast::FallthroughStatement>(source));
      break;
    }

    auto stmt = statement();
    if (stmt.errored)
      return Failure::kErrored;
    if (!stmt.matched)
      break;

    stmts.emplace_back(stmt.value);
  }

  return create<ast::BlockStatement>(Source{}, stmts);
}

// loop_stmt
//   : LOOP BRACKET_LEFT statements continuing_stmt? BRACKET_RIGHT
Maybe<const ast::LoopStatement*> ParserImpl::loop_stmt() {
  Source source;
  if (!match(Token::Type::kLoop, &source))
    return Failure::kNoMatch;

  return expect_brace_block("loop", [&]() -> Maybe<const ast::LoopStatement*> {
    auto stmts = expect_statements();
    if (stmts.errored)
      return Failure::kErrored;

    auto continuing = continuing_stmt();
    if (continuing.errored)
      return Failure::kErrored;

    auto* body = create<ast::BlockStatement>(source, stmts.value);
    return create<ast::LoopStatement>(source, body, continuing.value);
  });
}

ForHeader::ForHeader(const ast::Statement* init,
                     const ast::Expression* cond,
                     const ast::Statement* cont)
    : initializer(init), condition(cond), continuing(cont) {}

ForHeader::~ForHeader() = default;

// (variable_stmt | assignment_stmt | func_call_stmt)?
Maybe<const ast::Statement*> ParserImpl::for_header_initializer() {
  auto call = func_call_stmt();
  if (call.errored)
    return Failure::kErrored;
  if (call.matched)
    return call.value;

  auto var = variable_stmt();
  if (var.errored)
    return Failure::kErrored;
  if (var.matched)
    return var.value;

  auto assign = assignment_stmt();
  if (assign.errored)
    return Failure::kErrored;
  if (assign.matched)
    return assign.value;

  return Failure::kNoMatch;
}

// (assignment_stmt | func_call_stmt)?
Maybe<const ast::Statement*> ParserImpl::for_header_continuing() {
  auto call_stmt = func_call_stmt();
  if (call_stmt.errored)
    return Failure::kErrored;
  if (call_stmt.matched)
    return call_stmt.value;

  auto assign = assignment_stmt();
  if (assign.errored)
    return Failure::kErrored;
  if (assign.matched)
    return assign.value;

  return Failure::kNoMatch;
}

// for_header
//   : (variable_stmt | assignment_stmt | func_call_stmt)?
//   SEMICOLON
//      logical_or_expression? SEMICOLON
//      (assignment_stmt | func_call_stmt)?
Expect<std::unique_ptr<ForHeader>> ParserImpl::expect_for_header() {
  auto initializer = for_header_initializer();
  if (initializer.errored)
    return Failure::kErrored;

  if (!expect("initializer in for loop", Token::Type::kSemicolon))
    return Failure::kErrored;

  auto condition = logical_or_expression();
  if (condition.errored)
    return Failure::kErrored;

  if (!expect("condition in for loop", Token::Type::kSemicolon))
    return Failure::kErrored;

  auto continuing = for_header_continuing();
  if (continuing.errored)
    return Failure::kErrored;

  return std::make_unique<ForHeader>(initializer.value, condition.value,
                                     continuing.value);
}

// for_statement
//   : FOR PAREN_LEFT for_header PAREN_RIGHT BRACE_LEFT statements BRACE_RIGHT
Maybe<const ast::ForLoopStatement*> ParserImpl::for_stmt() {
  Source source;
  if (!match(Token::Type::kFor, &source))
    return Failure::kNoMatch;

  auto header =
      expect_paren_block("for loop", [&] { return expect_for_header(); });
  if (header.errored)
    return Failure::kErrored;

  auto stmts =
      expect_brace_block("for loop", [&] { return expect_statements(); });
  if (stmts.errored)
    return Failure::kErrored;

  return create<ast::ForLoopStatement>(
      source, header->initializer, header->condition, header->continuing,
      create<ast::BlockStatement>(stmts.value));
}

// func_call_stmt
//    : IDENT argument_expression_list
Maybe<const ast::CallStatement*> ParserImpl::func_call_stmt() {
  auto t = peek();
  auto t2 = peek(1);
  if (!t.IsIdentifier() || !t2.Is(Token::Type::kParenLeft))
    return Failure::kNoMatch;

  next();  // Consume the first peek

  auto source = t.source();
  auto name = t.to_str();

  auto params = expect_argument_expression_list("function call");
  if (params.errored)
    return Failure::kErrored;

  return create<ast::CallStatement>(
      source, create<ast::CallExpression>(
                  source,
                  create<ast::IdentifierExpression>(
                      source, builder_.Symbols().Register(name)),
                  std::move(params.value)));
}

// break_stmt
//   : BREAK
Maybe<const ast::BreakStatement*> ParserImpl::break_stmt() {
  Source source;
  if (!match(Token::Type::kBreak, &source))
    return Failure::kNoMatch;

  return create<ast::BreakStatement>(source);
}

// continue_stmt
//   : CONTINUE
Maybe<const ast::ContinueStatement*> ParserImpl::continue_stmt() {
  Source source;
  if (!match(Token::Type::kContinue, &source))
    return Failure::kNoMatch;

  return create<ast::ContinueStatement>(source);
}

// continuing_stmt
//   : CONTINUING body_stmt
Maybe<const ast::BlockStatement*> ParserImpl::continuing_stmt() {
  if (!match(Token::Type::kContinuing))
    return create<ast::BlockStatement>(Source{}, ast::StatementList{});

  return expect_body_stmt();
}

// primary_expression
//   : IDENT argument_expression_list?
//   | type_decl argument_expression_list
//   | const_literal
//   | paren_rhs_stmt
//   | BITCAST LESS_THAN type_decl GREATER_THAN paren_rhs_stmt
Maybe<const ast::Expression*> ParserImpl::primary_expression() {
  auto t = peek();
  auto source = t.source();

  auto lit = const_literal();
  if (lit.errored) {
    return Failure::kErrored;
  }
  if (lit.matched) {
    return lit.value;
  }

  if (t.Is(Token::Type::kParenLeft)) {
    auto paren = expect_paren_rhs_stmt();
    if (paren.errored) {
      return Failure::kErrored;
    }

    return paren.value;
  }

  if (match(Token::Type::kBitcast)) {
    const char* use = "bitcast expression";

    auto type = expect_lt_gt_block(use, [&] { return expect_type(use); });
    if (type.errored)
      return Failure::kErrored;

    auto params = expect_paren_rhs_stmt();
    if (params.errored)
      return Failure::kErrored;

    return create<ast::BitcastExpression>(source, type.value, params.value);
  }

  if (t.IsIdentifier()) {
    next();

    auto* ident = create<ast::IdentifierExpression>(
        t.source(), builder_.Symbols().Register(t.to_str()));

    if (peek_is(Token::Type::kParenLeft)) {
      auto params = expect_argument_expression_list("function call");
      if (params.errored)
        return Failure::kErrored;

      return create<ast::CallExpression>(source, ident,
                                         std::move(params.value));
    }

    return ident;
  }

  auto type = type_decl();
  if (type.errored)
    return Failure::kErrored;
  if (type.matched) {
    auto params = expect_argument_expression_list("type constructor");
    if (params.errored)
      return Failure::kErrored;

    return builder_.Construct(source, type.value, std::move(params.value));
  }

  return Failure::kNoMatch;
}

// postfix_expression
//   :
//   | BRACE_LEFT logical_or_expression BRACE_RIGHT postfix_expr
//   | PERIOD IDENTIFIER postfix_expr
Maybe<const ast::Expression*> ParserImpl::postfix_expression(
    const ast::Expression* prefix) {
  Source source;

  while (continue_parsing()) {
    if (match(Token::Type::kPlusPlus, &source) ||
        match(Token::Type::kMinusMinus, &source)) {
      add_error(source,
                "postfix increment and decrement operators are reserved for a "
                "future WGSL version");
      return Failure::kErrored;
    }

    if (match(Token::Type::kBracketLeft, &source)) {
      auto res = sync(
          Token::Type::kBracketRight, [&]() -> Maybe<const ast::Expression*> {
            auto param = logical_or_expression();
            if (param.errored)
              return Failure::kErrored;
            if (!param.matched) {
              return add_error(peek(), "unable to parse expression inside []");
            }

            if (!expect("index accessor", Token::Type::kBracketRight)) {
              return Failure::kErrored;
            }

            return create<ast::IndexAccessorExpression>(source, prefix,
                                                        param.value);
          });

      if (res.errored) {
        return res;
      }
      prefix = res.value;
      continue;
    }

    if (match(Token::Type::kPeriod)) {
      auto ident = expect_ident("member accessor");
      if (ident.errored) {
        return Failure::kErrored;
      }

      prefix = create<ast::MemberAccessorExpression>(
          ident.source, prefix,
          create<ast::IdentifierExpression>(
              ident.source, builder_.Symbols().Register(ident.value)));
      continue;
    }

    return prefix;
  }

  return Failure::kErrored;
}

// singular_expression
//   : primary_expression postfix_expr
Maybe<const ast::Expression*> ParserImpl::singular_expression() {
  auto prefix = primary_expression();
  if (prefix.errored)
    return Failure::kErrored;
  if (!prefix.matched)
    return Failure::kNoMatch;

  return postfix_expression(prefix.value);
}

// argument_expression_list
//   : PAREN_LEFT ((logical_or_expression COMMA)* logical_or_expression COMMA?)?
//   PAREN_RIGHT
Expect<ast::ExpressionList> ParserImpl::expect_argument_expression_list(
    const std::string& use) {
  return expect_paren_block(use, [&]() -> Expect<ast::ExpressionList> {
    ast::ExpressionList ret;
    while (continue_parsing()) {
      auto arg = logical_or_expression();
      if (arg.errored) {
        return Failure::kErrored;
      } else if (!arg.matched) {
        break;
      }
      ret.push_back(arg.value);

      if (!match(Token::Type::kComma)) {
        break;
      }
    }
    return ret;
  });
}

// unary_expression
//   : singular_expression
//   | MINUS unary_expression
//   | BANG unary_expression
//   | TILDE unary_expression
//   | STAR unary_expression
//   | AND unary_expression
Maybe<const ast::Expression*> ParserImpl::unary_expression() {
  auto t = peek();

  if (match(Token::Type::kPlusPlus) || match(Token::Type::kMinusMinus)) {
    add_error(t.source(),
              "prefix increment and decrement operators are reserved for a "
              "future WGSL version");
    return Failure::kErrored;
  }

  ast::UnaryOp op;
  if (match(Token::Type::kMinus)) {
    op = ast::UnaryOp::kNegation;
  } else if (match(Token::Type::kBang)) {
    op = ast::UnaryOp::kNot;
  } else if (match(Token::Type::kTilde)) {
    op = ast::UnaryOp::kComplement;
  } else if (match(Token::Type::kStar)) {
    op = ast::UnaryOp::kIndirection;
  } else if (match(Token::Type::kAnd)) {
    op = ast::UnaryOp::kAddressOf;
  } else {
    return singular_expression();
  }

  if (parse_depth_ >= kMaxParseDepth) {
    // We've hit a maximum parser recursive depth.
    // We can't call into unary_expression() as we might stack overflow.
    // Instead, report an error
    add_error(peek(), "maximum parser recursive depth reached");
    return Failure::kErrored;
  }

  ++parse_depth_;
  auto expr = unary_expression();
  --parse_depth_;

  if (expr.errored) {
    return Failure::kErrored;
  }
  if (!expr.matched) {
    return add_error(
        peek(), "unable to parse right side of " + t.to_name() + " expression");
  }

  return create<ast::UnaryOpExpression>(t.source(), op, expr.value);
}

// multiplicative_expr
//   :
//   | STAR unary_expression multiplicative_expr
//   | FORWARD_SLASH unary_expression multiplicative_expr
//   | MODULO unary_expression multiplicative_expr
Expect<const ast::Expression*> ParserImpl::expect_multiplicative_expr(
    const ast::Expression* lhs) {
  while (continue_parsing()) {
    ast::BinaryOp op = ast::BinaryOp::kNone;
    if (peek_is(Token::Type::kStar))
      op = ast::BinaryOp::kMultiply;
    else if (peek_is(Token::Type::kForwardSlash))
      op = ast::BinaryOp::kDivide;
    else if (peek_is(Token::Type::kMod))
      op = ast::BinaryOp::kModulo;
    else
      return lhs;

    auto t = next();
    auto source = t.source();
    auto name = t.to_name();

    auto rhs = unary_expression();
    if (rhs.errored)
      return Failure::kErrored;
    if (!rhs.matched) {
      return add_error(peek(),
                       "unable to parse right side of " + name + " expression");
    }

    lhs = create<ast::BinaryExpression>(source, op, lhs, rhs.value);
  }
  return Failure::kErrored;
}

// multiplicative_expression
//   : unary_expression multiplicative_expr
Maybe<const ast::Expression*> ParserImpl::multiplicative_expression() {
  auto lhs = unary_expression();
  if (lhs.errored)
    return Failure::kErrored;
  if (!lhs.matched)
    return Failure::kNoMatch;

  return expect_multiplicative_expr(lhs.value);
}

// additive_expr
//   :
//   | PLUS multiplicative_expression additive_expr
//   | MINUS multiplicative_expression additive_expr
Expect<const ast::Expression*> ParserImpl::expect_additive_expr(
    const ast::Expression* lhs) {
  while (continue_parsing()) {
    ast::BinaryOp op = ast::BinaryOp::kNone;
    if (peek_is(Token::Type::kPlus))
      op = ast::BinaryOp::kAdd;
    else if (peek_is(Token::Type::kMinus))
      op = ast::BinaryOp::kSubtract;
    else
      return lhs;

    auto t = next();
    auto source = t.source();

    auto rhs = multiplicative_expression();
    if (rhs.errored)
      return Failure::kErrored;
    if (!rhs.matched)
      return add_error(peek(), "unable to parse right side of + expression");

    lhs = create<ast::BinaryExpression>(source, op, lhs, rhs.value);
  }
  return Failure::kErrored;
}

// additive_expression
//   : multiplicative_expression additive_expr
Maybe<const ast::Expression*> ParserImpl::additive_expression() {
  auto lhs = multiplicative_expression();
  if (lhs.errored)
    return Failure::kErrored;
  if (!lhs.matched)
    return Failure::kNoMatch;

  return expect_additive_expr(lhs.value);
}

// shift_expr
//   :
//   | SHIFT_LEFT additive_expression shift_expr
//   | SHIFT_RIGHT additive_expression shift_expr
Expect<const ast::Expression*> ParserImpl::expect_shift_expr(
    const ast::Expression* lhs) {
  while (continue_parsing()) {
    auto* name = "";
    ast::BinaryOp op = ast::BinaryOp::kNone;
    if (peek_is(Token::Type::kShiftLeft)) {
      op = ast::BinaryOp::kShiftLeft;
      name = "<<";
    } else if (peek_is(Token::Type::kShiftRight)) {
      op = ast::BinaryOp::kShiftRight;
      name = ">>";
    } else {
      return lhs;
    }

    auto t = next();
    auto source = t.source();
    auto rhs = additive_expression();
    if (rhs.errored)
      return Failure::kErrored;
    if (!rhs.matched) {
      return add_error(peek(), std::string("unable to parse right side of ") +
                                   name + " expression");
    }

    return lhs = create<ast::BinaryExpression>(source, op, lhs, rhs.value);
  }
  return Failure::kErrored;
}

// shift_expression
//   : additive_expression shift_expr
Maybe<const ast::Expression*> ParserImpl::shift_expression() {
  auto lhs = additive_expression();
  if (lhs.errored)
    return Failure::kErrored;
  if (!lhs.matched)
    return Failure::kNoMatch;

  return expect_shift_expr(lhs.value);
}

// relational_expr
//   :
//   | LESS_THAN shift_expression relational_expr
//   | GREATER_THAN shift_expression relational_expr
//   | LESS_THAN_EQUAL shift_expression relational_expr
//   | GREATER_THAN_EQUAL shift_expression relational_expr
Expect<const ast::Expression*> ParserImpl::expect_relational_expr(
    const ast::Expression* lhs) {
  while (continue_parsing()) {
    ast::BinaryOp op = ast::BinaryOp::kNone;
    if (peek_is(Token::Type::kLessThan))
      op = ast::BinaryOp::kLessThan;
    else if (peek_is(Token::Type::kGreaterThan))
      op = ast::BinaryOp::kGreaterThan;
    else if (peek_is(Token::Type::kLessThanEqual))
      op = ast::BinaryOp::kLessThanEqual;
    else if (peek_is(Token::Type::kGreaterThanEqual))
      op = ast::BinaryOp::kGreaterThanEqual;
    else
      return lhs;

    auto t = next();
    auto source = t.source();
    auto name = t.to_name();

    auto rhs = shift_expression();
    if (rhs.errored)
      return Failure::kErrored;
    if (!rhs.matched) {
      return add_error(peek(),
                       "unable to parse right side of " + name + " expression");
    }

    lhs = create<ast::BinaryExpression>(source, op, lhs, rhs.value);
  }
  return Failure::kErrored;
}

// relational_expression
//   : shift_expression relational_expr
Maybe<const ast::Expression*> ParserImpl::relational_expression() {
  auto lhs = shift_expression();
  if (lhs.errored)
    return Failure::kErrored;
  if (!lhs.matched)
    return Failure::kNoMatch;

  return expect_relational_expr(lhs.value);
}

// equality_expr
//   :
//   | EQUAL_EQUAL relational_expression equality_expr
//   | NOT_EQUAL relational_expression equality_expr
Expect<const ast::Expression*> ParserImpl::expect_equality_expr(
    const ast::Expression* lhs) {
  while (continue_parsing()) {
    ast::BinaryOp op = ast::BinaryOp::kNone;
    if (peek_is(Token::Type::kEqualEqual))
      op = ast::BinaryOp::kEqual;
    else if (peek_is(Token::Type::kNotEqual))
      op = ast::BinaryOp::kNotEqual;
    else
      return lhs;

    auto t = next();
    auto source = t.source();
    auto name = t.to_name();

    auto rhs = relational_expression();
    if (rhs.errored)
      return Failure::kErrored;
    if (!rhs.matched) {
      return add_error(peek(),
                       "unable to parse right side of " + name + " expression");
    }

    lhs = create<ast::BinaryExpression>(source, op, lhs, rhs.value);
  }
  return Failure::kErrored;
}

// equality_expression
//   : relational_expression equality_expr
Maybe<const ast::Expression*> ParserImpl::equality_expression() {
  auto lhs = relational_expression();
  if (lhs.errored)
    return Failure::kErrored;
  if (!lhs.matched)
    return Failure::kNoMatch;

  return expect_equality_expr(lhs.value);
}

// and_expr
//   :
//   | AND equality_expression and_expr
Expect<const ast::Expression*> ParserImpl::expect_and_expr(
    const ast::Expression* lhs) {
  while (continue_parsing()) {
    if (!peek_is(Token::Type::kAnd)) {
      return lhs;
    }

    auto t = next();
    auto source = t.source();

    auto rhs = equality_expression();
    if (rhs.errored)
      return Failure::kErrored;
    if (!rhs.matched)
      return add_error(peek(), "unable to parse right side of & expression");

    lhs = create<ast::BinaryExpression>(source, ast::BinaryOp::kAnd, lhs,
                                        rhs.value);
  }
  return Failure::kErrored;
}

// and_expression
//   : equality_expression and_expr
Maybe<const ast::Expression*> ParserImpl::and_expression() {
  auto lhs = equality_expression();
  if (lhs.errored)
    return Failure::kErrored;
  if (!lhs.matched)
    return Failure::kNoMatch;

  return expect_and_expr(lhs.value);
}

// exclusive_or_expr
//   :
//   | XOR and_expression exclusive_or_expr
Expect<const ast::Expression*> ParserImpl::expect_exclusive_or_expr(
    const ast::Expression* lhs) {
  while (continue_parsing()) {
    Source source;
    if (!match(Token::Type::kXor, &source))
      return lhs;

    auto rhs = and_expression();
    if (rhs.errored)
      return Failure::kErrored;
    if (!rhs.matched)
      return add_error(peek(), "unable to parse right side of ^ expression");

    lhs = create<ast::BinaryExpression>(source, ast::BinaryOp::kXor, lhs,
                                        rhs.value);
  }
  return Failure::kErrored;
}

// exclusive_or_expression
//   : and_expression exclusive_or_expr
Maybe<const ast::Expression*> ParserImpl::exclusive_or_expression() {
  auto lhs = and_expression();
  if (lhs.errored)
    return Failure::kErrored;
  if (!lhs.matched)
    return Failure::kNoMatch;

  return expect_exclusive_or_expr(lhs.value);
}

// inclusive_or_expr
//   :
//   | OR exclusive_or_expression inclusive_or_expr
Expect<const ast::Expression*> ParserImpl::expect_inclusive_or_expr(
    const ast::Expression* lhs) {
  while (continue_parsing()) {
    Source source;
    if (!match(Token::Type::kOr))
      return lhs;

    auto rhs = exclusive_or_expression();
    if (rhs.errored)
      return Failure::kErrored;
    if (!rhs.matched)
      return add_error(peek(), "unable to parse right side of | expression");

    lhs = create<ast::BinaryExpression>(source, ast::BinaryOp::kOr, lhs,
                                        rhs.value);
  }
  return Failure::kErrored;
}

// inclusive_or_expression
//   : exclusive_or_expression inclusive_or_expr
Maybe<const ast::Expression*> ParserImpl::inclusive_or_expression() {
  auto lhs = exclusive_or_expression();
  if (lhs.errored)
    return Failure::kErrored;
  if (!lhs.matched)
    return Failure::kNoMatch;

  return expect_inclusive_or_expr(lhs.value);
}

// logical_and_expr
//   :
//   | AND_AND inclusive_or_expression logical_and_expr
Expect<const ast::Expression*> ParserImpl::expect_logical_and_expr(
    const ast::Expression* lhs) {
  while (continue_parsing()) {
    if (!peek_is(Token::Type::kAndAnd)) {
      return lhs;
    }

    auto t = next();
    auto source = t.source();

    auto rhs = inclusive_or_expression();
    if (rhs.errored)
      return Failure::kErrored;
    if (!rhs.matched)
      return add_error(peek(), "unable to parse right side of && expression");

    lhs = create<ast::BinaryExpression>(source, ast::BinaryOp::kLogicalAnd, lhs,
                                        rhs.value);
  }
  return Failure::kErrored;
}

// logical_and_expression
//   : inclusive_or_expression logical_and_expr
Maybe<const ast::Expression*> ParserImpl::logical_and_expression() {
  auto lhs = inclusive_or_expression();
  if (lhs.errored)
    return Failure::kErrored;
  if (!lhs.matched)
    return Failure::kNoMatch;

  return expect_logical_and_expr(lhs.value);
}

// logical_or_expr
//   :
//   | OR_OR logical_and_expression logical_or_expr
Expect<const ast::Expression*> ParserImpl::expect_logical_or_expr(
    const ast::Expression* lhs) {
  while (continue_parsing()) {
    Source source;
    if (!match(Token::Type::kOrOr))
      return lhs;

    auto rhs = logical_and_expression();
    if (rhs.errored)
      return Failure::kErrored;
    if (!rhs.matched)
      return add_error(peek(), "unable to parse right side of || expression");

    lhs = create<ast::BinaryExpression>(source, ast::BinaryOp::kLogicalOr, lhs,
                                        rhs.value);
  }
  return Failure::kErrored;
}

// logical_or_expression
//   : logical_and_expression logical_or_expr
Maybe<const ast::Expression*> ParserImpl::logical_or_expression() {
  auto lhs = logical_and_expression();
  if (lhs.errored)
    return Failure::kErrored;
  if (!lhs.matched)
    return Failure::kNoMatch;

  return expect_logical_or_expr(lhs.value);
}

// assignment_stmt
//   : (unary_expression | underscore) EQUAL logical_or_expression
Maybe<const ast::AssignmentStatement*> ParserImpl::assignment_stmt() {
  auto t = peek();
  auto source = t.source();

  // tint:295 - Test for `ident COLON` - this is invalid grammar, and without
  // special casing will error as "missing = for assignment", which is less
  // helpful than this error message:
  if (peek_is(Token::Type::kIdentifier) && peek_is(Token::Type::kColon, 1)) {
    return add_error(peek(0).source(),
                     "expected 'var' for variable declaration");
  }

  auto lhs = unary_expression();
  if (lhs.errored) {
    return Failure::kErrored;
  }
  if (!lhs.matched) {
    if (!match(Token::Type::kUnderscore, &source)) {
      return Failure::kNoMatch;
    }
    lhs = create<ast::PhonyExpression>(source);
  }

  if (!expect("assignment", Token::Type::kEqual)) {
    return Failure::kErrored;
  }

  auto rhs = logical_or_expression();
  if (rhs.errored) {
    return Failure::kErrored;
  }
  if (!rhs.matched) {
    return add_error(peek(), "unable to parse right side of assignment");
  }

  return create<ast::AssignmentStatement>(source, lhs.value, rhs.value);
}

// const_literal
//   : INT_LITERAL
//   | UINT_LITERAL
//   | FLOAT_LITERAL
//   | TRUE
//   | FALSE
Maybe<const ast::LiteralExpression*> ParserImpl::const_literal() {
  auto t = peek();
  if (t.IsError()) {
    return add_error(t.source(), t.to_str());
  }
  if (match(Token::Type::kTrue)) {
    return create<ast::BoolLiteralExpression>(t.source(), true);
  }
  if (match(Token::Type::kFalse)) {
    return create<ast::BoolLiteralExpression>(t.source(), false);
  }
  if (match(Token::Type::kSintLiteral)) {
    return create<ast::SintLiteralExpression>(t.source(), t.to_i32());
  }
  if (match(Token::Type::kUintLiteral)) {
    return create<ast::UintLiteralExpression>(t.source(), t.to_u32());
  }
  if (match(Token::Type::kFloatLiteral)) {
    return create<ast::FloatLiteralExpression>(t.source(), t.to_f32());
  }
  return Failure::kNoMatch;
}

// const_expr
//   : type_decl PAREN_LEFT ((const_expr COMMA)? const_expr COMMA?)? PAREN_RIGHT
//   | const_literal
Expect<const ast::Expression*> ParserImpl::expect_const_expr() {
  auto t = peek();
  auto source = t.source();
  if (t.IsLiteral()) {
    auto lit = const_literal();
    if (lit.errored) {
      return Failure::kErrored;
    }
    if (!lit.matched) {
      return add_error(peek(), "unable to parse constant literal");
    }
    return lit.value;
  }

  if (peek_is(Token::Type::kParenLeft, 1) ||
      peek_is(Token::Type::kLessThan, 1)) {
    auto type = expect_type("const_expr");
    if (type.errored) {
      return Failure::kErrored;
    }

    auto params = expect_paren_block(
        "type constructor", [&]() -> Expect<ast::ExpressionList> {
          ast::ExpressionList list;
          while (continue_parsing()) {
            if (peek_is(Token::Type::kParenRight)) {
              break;
            }

            auto arg = expect_const_expr();
            if (arg.errored) {
              return Failure::kErrored;
            }
            list.emplace_back(arg.value);

            if (!match(Token::Type::kComma)) {
              break;
            }
          }
          return list;
        });

    if (params.errored)
      return Failure::kErrored;

    return builder_.Construct(source, type.value, params.value);
  }
  return add_error(peek(), "unable to parse const_expr");
}

Maybe<ast::DecorationList> ParserImpl::decoration_list() {
  bool errored = false;
  bool matched = false;
  ast::DecorationList decos;

  while (continue_parsing()) {
    auto list = decoration_bracketed_list(decos);
    if (list.errored)
      errored = true;
    if (!list.matched)
      break;

    matched = true;
  }

  if (errored)
    return Failure::kErrored;

  if (!matched)
    return Failure::kNoMatch;

  return decos;
}

Maybe<bool> ParserImpl::decoration_bracketed_list(ast::DecorationList& decos) {
  const char* use = "decoration list";

  if (!match(Token::Type::kAttrLeft)) {
    return Failure::kNoMatch;
  }

  Source source;
  if (match(Token::Type::kAttrRight, &source))
    return add_error(source, "empty decoration list");

  return sync(Token::Type::kAttrRight, [&]() -> Expect<bool> {
    bool errored = false;

    while (continue_parsing()) {
      auto deco = expect_decoration();
      if (deco.errored) {
        errored = true;
      }
      decos.emplace_back(deco.value);

      if (match(Token::Type::kComma)) {
        continue;
      }

      if (is_decoration(peek())) {
        // We have two decorations in a bracket without a separating comma.
        // e.g. [[location(1) group(2)]]
        //                    ^^^ expected comma
        expect(use, Token::Type::kComma);
        return Failure::kErrored;
      }

      break;
    }

    if (errored) {
      return Failure::kErrored;
    }

    if (!expect(use, Token::Type::kAttrRight)) {
      return Failure::kErrored;
    }

    return true;
  });
}

Expect<const ast::Decoration*> ParserImpl::expect_decoration() {
  auto t = peek();
  auto deco = decoration();
  if (deco.errored)
    return Failure::kErrored;
  if (deco.matched)
    return deco.value;
  return add_error(t, "expected decoration");
}

Maybe<const ast::Decoration*> ParserImpl::decoration() {
  using Result = Maybe<const ast::Decoration*>;
  auto t = next();

  if (!t.IsIdentifier()) {
    return Failure::kNoMatch;
  }

  auto s = t.to_str();

  if (s == kLocationDecoration) {
    const char* use = "location decoration";
    return expect_paren_block(use, [&]() -> Result {
      auto val = expect_positive_sint(use);
      if (val.errored)
        return Failure::kErrored;

      return create<ast::LocationDecoration>(t.source(), val.value);
    });
  }

  if (s == kBindingDecoration) {
    const char* use = "binding decoration";
    return expect_paren_block(use, [&]() -> Result {
      auto val = expect_positive_sint(use);
      if (val.errored)
        return Failure::kErrored;

      return create<ast::BindingDecoration>(t.source(), val.value);
    });
  }

  if (s == kGroupDecoration) {
    const char* use = "group decoration";
    return expect_paren_block(use, [&]() -> Result {
      auto val = expect_positive_sint(use);
      if (val.errored)
        return Failure::kErrored;

      return create<ast::GroupDecoration>(t.source(), val.value);
    });
  }

  if (s == kInterpolateDecoration) {
    return expect_paren_block("interpolate decoration", [&]() -> Result {
      ast::InterpolationType type;
      ast::InterpolationSampling sampling = ast::InterpolationSampling::kNone;

      auto type_tok = next();
      auto type_str = type_tok.to_str();
      if (type_str == "perspective") {
        type = ast::InterpolationType::kPerspective;
      } else if (type_str == "linear") {
        type = ast::InterpolationType::kLinear;
      } else if (type_str == "flat") {
        type = ast::InterpolationType::kFlat;
      } else {
        return add_error(type_tok, "invalid interpolation type");
      }

      if (match(Token::Type::kComma)) {
        auto sampling_tok = next();
        auto sampling_str = sampling_tok.to_str();
        if (sampling_str == "center") {
          sampling = ast::InterpolationSampling::kCenter;
        } else if (sampling_str == "centroid") {
          sampling = ast::InterpolationSampling::kCentroid;
        } else if (sampling_str == "sample") {
          sampling = ast::InterpolationSampling::kSample;
        } else {
          return add_error(sampling_tok, "invalid interpolation sampling");
        }
      }

      return create<ast::InterpolateDecoration>(t.source(), type, sampling);
    });
  }

  if (s == kInvariantDecoration) {
    return create<ast::InvariantDecoration>(t.source());
  }

  if (s == kBuiltinDecoration) {
    return expect_paren_block("builtin decoration", [&]() -> Result {
      auto builtin = expect_builtin();
      if (builtin.errored)
        return Failure::kErrored;

      return create<ast::BuiltinDecoration>(t.source(), builtin.value);
    });
  }

  if (s == kWorkgroupSizeDecoration) {
    return expect_paren_block("workgroup_size decoration", [&]() -> Result {
      const ast::Expression* x = nullptr;
      const ast::Expression* y = nullptr;
      const ast::Expression* z = nullptr;

      auto expr = primary_expression();
      if (expr.errored) {
        return Failure::kErrored;
      } else if (!expr.matched) {
        return add_error(peek(), "expected workgroup_size x parameter");
      }
      x = std::move(expr.value);

      if (match(Token::Type::kComma)) {
        expr = primary_expression();
        if (expr.errored) {
          return Failure::kErrored;
        } else if (!expr.matched) {
          return add_error(peek(), "expected workgroup_size y parameter");
        }
        y = std::move(expr.value);

        if (match(Token::Type::kComma)) {
          expr = primary_expression();
          if (expr.errored) {
            return Failure::kErrored;
          } else if (!expr.matched) {
            return add_error(peek(), "expected workgroup_size z parameter");
          }
          z = std::move(expr.value);
        }
      }

      return create<ast::WorkgroupDecoration>(t.source(), x, y, z);
    });
  }

  if (s == kStageDecoration) {
    return expect_paren_block("stage decoration", [&]() -> Result {
      auto stage = expect_pipeline_stage();
      if (stage.errored)
        return Failure::kErrored;

      return create<ast::StageDecoration>(t.source(), stage.value);
    });
  }

  if (s == kBlockDecoration) {
    return create<ast::StructBlockDecoration>(t.source());
  }

  if (s == kStrideDecoration) {
    const char* use = "stride decoration";
    return expect_paren_block(use, [&]() -> Result {
      auto val = expect_nonzero_positive_sint(use);
      if (val.errored)
        return Failure::kErrored;

      return create<ast::StrideDecoration>(t.source(), val.value);
    });
  }

  if (s == kSizeDecoration) {
    const char* use = "size decoration";
    return expect_paren_block(use, [&]() -> Result {
      auto val = expect_positive_sint(use);
      if (val.errored)
        return Failure::kErrored;

      return create<ast::StructMemberSizeDecoration>(t.source(), val.value);
    });
  }

  if (s == kAlignDecoration) {
    const char* use = "align decoration";
    return expect_paren_block(use, [&]() -> Result {
      auto val = expect_positive_sint(use);
      if (val.errored)
        return Failure::kErrored;

      return create<ast::StructMemberAlignDecoration>(t.source(), val.value);
    });
  }

  if (s == kOverrideDecoration) {
    const char* use = "override decoration";

    if (peek_is(Token::Type::kParenLeft)) {
      // [[override(x)]]
      return expect_paren_block(use, [&]() -> Result {
        auto val = expect_positive_sint(use);
        if (val.errored)
          return Failure::kErrored;

        return create<ast::OverrideDecoration>(t.source(), val.value);
      });
    } else {
      // [[override]]
      return create<ast::OverrideDecoration>(t.source());
    }
  }

  return Failure::kNoMatch;
}

bool ParserImpl::expect_decorations_consumed(ast::DecorationList& in) {
  if (in.empty()) {
    return true;
  }
  add_error(in[0]->source, "unexpected decorations");
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
  auto t = peek();
  if (t.Is(tok)) {
    next();
    synchronized_ = true;
    return true;
  }

  // Special case to split `>>` and `>=` tokens if we are looking for a `>`.
  if (tok == Token::Type::kGreaterThan &&
      (t.Is(Token::Type::kShiftRight) ||
       t.Is(Token::Type::kGreaterThanEqual))) {
    next();

    // Push the second character to the token queue.
    auto source = t.source();
    source.range.begin.column++;
    if (t.Is(Token::Type::kShiftRight)) {
      token_queue_.push_front(Token(Token::Type::kGreaterThan, source));
    } else if (t.Is(Token::Type::kGreaterThanEqual)) {
      token_queue_.push_front(Token(Token::Type::kEqual, source));
    }

    synchronized_ = true;
    return true;
  }

  // Handle the case when `]` is expected but the actual token is `]]`.
  // For example, in `arr1[arr2[0]]`.
  if (tok == Token::Type::kBracketRight && t.Is(Token::Type::kAttrRight)) {
    next();
    auto source = t.source();
    source.range.begin.column++;
    token_queue_.push_front({Token::Type::kBracketRight, source});
    synchronized_ = true;
    return true;
  }

  std::stringstream err;
  err << "expected '" << Token::TypeToName(tok) << "'";
  if (!use.empty()) {
    err << " for " << use;
  }
  add_error(t, err.str());
  synchronized_ = false;
  return false;
}

Expect<int32_t> ParserImpl::expect_sint(const std::string& use) {
  auto t = peek();
  if (!t.Is(Token::Type::kSintLiteral))
    return add_error(t.source(), "expected signed integer literal", use);

  next();
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
  auto t = peek();
  if (t.IsIdentifier()) {
    synchronized_ = true;
    next();

    if (is_reserved(t)) {
      return add_error(t.source(),
                       "'" + t.to_str() + "' is a reserved keyword");
    }

    return {t.to_str(), t.source()};
  }
  synchronized_ = false;
  return add_error(t.source(), "expected identifier", use);
}

template <typename F, typename T>
T ParserImpl::expect_block(Token::Type start,
                           Token::Type end,
                           const std::string& use,
                           F&& body) {
  if (!expect(use, start)) {
    return Failure::kErrored;
  }

  return sync(end, [&]() -> T {
    auto res = body();

    if (res.errored)
      return Failure::kErrored;

    if (!expect(use, end))
      return Failure::kErrored;

    return res;
  });
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

template <typename F, typename T>
T ParserImpl::expect_lt_gt_block(const std::string& use, F&& body) {
  return expect_block(Token::Type::kLessThan, Token::Type::kGreaterThan, use,
                      std::forward<F>(body));
}

template <typename F, typename T>
T ParserImpl::sync(Token::Type tok, F&& body) {
  if (parse_depth_ >= kMaxParseDepth) {
    // We've hit a maximum parser recursive depth.
    // We can't call into body() as we might stack overflow.
    // Instead, report an error...
    add_error(peek(), "maximum parser recursive depth reached");
    // ...and try to resynchronize. If we cannot resynchronize to `tok` then
    // synchronized_ is set to false, and the parser knows that forward progress
    // is not being made.
    sync_to(tok, /* consume: */ true);
    return Failure::kErrored;
  }

  sync_tokens_.push_back(tok);

  ++parse_depth_;
  auto result = body();
  --parse_depth_;

  if (sync_tokens_.back() != tok) {
    TINT_ICE(Reader, builder_.Diagnostics()) << "sync_tokens is out of sync";
  }
  sync_tokens_.pop_back();

  if (result.errored) {
    sync_to(tok, /* consume: */ true);
  }

  return result;
}

bool ParserImpl::sync_to(Token::Type tok, bool consume) {
  // Clear the synchronized state - gets set to true again on success.
  synchronized_ = false;

  BlockCounters counters;

  for (size_t i = 0; i < kMaxResynchronizeLookahead; i++) {
    auto t = peek(i);
    if (counters.consume(t) > 0) {
      continue;  // Nested block
    }
    if (!t.Is(tok) && !is_sync_token(t)) {
      continue;  // Not a synchronization point
    }

    // Synchronization point found.

    // Skip any tokens we don't understand, bringing us to just before the
    // resync point.
    while (i-- > 0) {
      next();
    }

    // Is this synchronization token |tok|?
    if (t.Is(tok)) {
      if (consume) {
        next();
      }
      synchronized_ = true;
      return true;
    }
    break;
  }

  return false;
}

bool ParserImpl::is_sync_token(const Token& t) const {
  for (auto r : sync_tokens_) {
    if (t.Is(r)) {
      return true;
    }
  }
  return false;
}

template <typename F, typename T>
T ParserImpl::without_error(F&& body) {
  silence_errors_++;
  auto result = body();
  silence_errors_--;
  return result;
}

ParserImpl::MultiTokenSource ParserImpl::make_source_range() {
  return MultiTokenSource(this);
}

ParserImpl::MultiTokenSource ParserImpl::make_source_range_from(
    const Source& start) {
  return MultiTokenSource(this, start);
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
