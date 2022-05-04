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

#include "src/tint/reader/wgsl/parser_impl.h"

#include <limits>

#include "src/tint/ast/array.h"
#include "src/tint/ast/assignment_statement.h"
#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/external_texture.h"
#include "src/tint/ast/fallthrough_statement.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/increment_decrement_statement.h"
#include "src/tint/ast/invariant_attribute.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/ast/type_name.h"
#include "src/tint/ast/unary_op_expression.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/ast/vector.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/reader/wgsl/lexer.h"
#include "src/tint/sem/depth_texture.h"
#include "src/tint/sem/external_texture.h"
#include "src/tint/sem/multisampled_texture.h"
#include "src/tint/sem/sampled_texture.h"

namespace tint::reader::wgsl {
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

ast::Builtin ident_to_builtin(std::string_view str) {
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

const char kBindingAttribute[] = "binding";
const char kBuiltinAttribute[] = "builtin";
const char kGroupAttribute[] = "group";
const char kIdAttribute[] = "id";
const char kInterpolateAttribute[] = "interpolate";
const char kInvariantAttribute[] = "invariant";
const char kLocationAttribute[] = "location";
const char kSizeAttribute[] = "size";
const char kAlignAttribute[] = "align";
const char kStageAttribute[] = "stage";
const char kWorkgroupSizeAttribute[] = "workgroup_size";

// https://gpuweb.github.io/gpuweb/wgsl.html#reserved-keywords
bool is_reserved(Token t) {
    return t == "asm" || t == "bf16" || t == "const" || t == "do" || t == "enum" || t == "f16" ||
           t == "f64" || t == "handle" || t == "i8" || t == "i16" || t == "i64" || t == "mat" ||
           t == "premerge" || t == "regardless" || t == "typedef" || t == "u8" || t == "u16" ||
           t == "u64" || t == "unless" || t == "using" || t == "vec" || t == "void" || t == "while";
}

/// Enter-exit counters for block token types.
/// Used by sync_to() to skip over closing block tokens that were opened during
/// the forward scan.
struct BlockCounters {
    int brace = 0;    // {   }
    int bracket = 0;  // [   ]
    int paren = 0;    // (   )

    /// @return the current enter-exit depth for the given block token type. If
    /// `t` is not a block token type, then 0 is always returned.
    int consume(const Token& t) {
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
    MultiTokenSource(ParserImpl* parser, const Source& start) : parser_(parser), start_(start) {}

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
                                           ast::AttributeList ret_attrs)
    : source(src), name(n), params(p), return_type(ret_ty), return_type_attributes(ret_attrs) {}

ParserImpl::FunctionHeader::~FunctionHeader() = default;

ParserImpl::FunctionHeader& ParserImpl::FunctionHeader::operator=(const FunctionHeader& rhs) =
    default;

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

ParserImpl::ParserImpl(Source::File const* file) : lexer_(std::make_unique<Lexer>(file)) {}

ParserImpl::~ParserImpl() = default;

ParserImpl::Failure::Errored ParserImpl::add_error(const Source& source,
                                                   std::string_view err,
                                                   std::string_view use) {
    std::stringstream msg;
    msg << err;
    if (!use.empty()) {
        msg << " for " << use;
    }
    add_error(source, msg.str());
    return Failure::kErrored;
}

ParserImpl::Failure::Errored ParserImpl::add_error(const Token& t, const std::string& err) {
    add_error(t.source(), err);
    return Failure::kErrored;
}

ParserImpl::Failure::Errored ParserImpl::add_error(const Source& source, const std::string& err) {
    if (silence_errors_ == 0) {
        builder_.Diagnostics().add_error(diag::System::Reader, err, source);
    }
    return Failure::kErrored;
}

void ParserImpl::deprecated(const Source& source, const std::string& msg) {
    builder_.Diagnostics().add_warning(diag::System::Reader,
                                       "use of deprecated language feature: " + msg, source);
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
    while (token_queue_.size() < (idx + 1)) {
        token_queue_.push_back(lexer_->next());
    }
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
//  : enable_directive* global_decl* EOF
void ParserImpl::translation_unit() {
    bool after_global_decl = false;
    while (continue_parsing()) {
        auto p = peek();
        if (p.IsEof()) {
            break;
        }

        auto ed = enable_directive();
        if (ed.matched) {
            if (after_global_decl) {
                add_error(p, "enable directives must come before all global declarations");
            }
        } else {
            auto gd = global_decl();

            if (gd.matched) {
                after_global_decl = true;
            }

            if (!gd.matched && !gd.errored) {
                add_error(p, "unexpected token");
            }
        }

        if (builder_.Diagnostics().error_count() >= max_errors_) {
            add_error(Source{{}, p.source().file},
                      "stopping after " + std::to_string(max_errors_) + " errors");
            break;
        }
    }
}

// enable_directive
//  : enable name SEMICLON
Maybe<bool> ParserImpl::enable_directive() {
    auto decl = sync(Token::Type::kSemicolon, [&]() -> Maybe<bool> {
        if (!match(Token::Type::kEnable)) {
            return Failure::kNoMatch;
        }

        // Match the extension name.
        Expect<std::string> name = {""};
        auto t = peek();
        if (t.IsIdentifier()) {
            synchronized_ = true;
            next();
            name = {t.to_str(), t.source()};
        } else if (handle_error(t)) {
            // The token might itself be an error.
            return Failure::kErrored;
        } else {
            // Failed to match an extension name.
            synchronized_ = false;
            return add_error(t.source(), "invalid extension name");
        }

        if (!expect("enable directive", Token::Type::kSemicolon)) {
            return Failure::kErrored;
        }

        if (ast::Enable::NameToKind(name.value) != ast::Enable::ExtensionKind::kNotAnExtension) {
            const ast::Enable* extension = create<ast::Enable>(name.source, name.value);
            builder_.AST().AddEnable(extension);
        } else {
            // Error if an unknown extension is used
            return add_error(name.source, "unsupported extension: '" + name.value + "'");
        }

        return true;
    });

    if (decl.errored) {
        return Failure::kErrored;
    }
    if (decl.matched) {
        return true;
    }

    return Failure::kNoMatch;
}

// global_decl
//  : SEMICOLON
//  | global_variable_decl SEMICLON
//  | global_constant_decl SEMICOLON
//  | type_alias SEMICOLON
//  | struct_decl
//  | function_decl
Maybe<bool> ParserImpl::global_decl() {
    if (match(Token::Type::kSemicolon) || match(Token::Type::kEOF))
        return true;

    bool errored = false;

    auto attrs = attribute_list();
    if (attrs.errored)
        errored = true;
    if (!continue_parsing())
        return Failure::kErrored;

    auto decl = sync(Token::Type::kSemicolon, [&]() -> Maybe<bool> {
        auto gv = global_variable_decl(attrs.value);
        if (gv.errored)
            return Failure::kErrored;
        if (gv.matched) {
            if (!expect("variable declaration", Token::Type::kSemicolon))
                return Failure::kErrored;

            builder_.AST().AddGlobalVariable(gv.value);
            return true;
        }

        auto gc = global_constant_decl(attrs.value);
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

        auto str = struct_decl();
        if (str.errored)
            return Failure::kErrored;

        if (str.matched) {
            builder_.AST().AddTypeDecl(str.value);
            return true;
        }

        return Failure::kNoMatch;
    });

    if (decl.errored) {
        errored = true;
    }
    if (decl.matched) {
        return expect_attributes_consumed(attrs.value);
    }

    auto func = function_decl(attrs.value);
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

    // We have attributes parsed, but nothing to consume them?
    if (attrs.value.size() > 0) {
        return add_error(next(), "expected declaration after attributes");
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
    if (handle_error(t)) {
        return Failure::kErrored;
    }

    // Exhausted all attempts to make sense of where we're at.
    // Return a no-match

    return Failure::kNoMatch;
}

// global_variable_decl
//  : variable_attribute_list* variable_decl
//  | variable_attribute_list* variable_decl EQUAL const_expr
Maybe<const ast::Variable*> ParserImpl::global_variable_decl(ast::AttributeList& attrs) {
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

    return create<ast::Variable>(decl->source,                             // source
                                 builder_.Symbols().Register(decl->name),  // symbol
                                 decl->storage_class,                      // storage class
                                 decl->access,                             // access control
                                 decl->type,                               // type
                                 false,                                    // is_const
                                 false,                                    // is_overridable
                                 constructor,                              // constructor
                                 std::move(attrs));                        // attributes
}

// global_constant_decl :
//  | LET (ident | variable_ident_decl) global_const_initializer
//  | attribute* override (ident | variable_ident_decl) (equal expression)?
// global_const_initializer
//  : EQUAL const_expr
Maybe<const ast::Variable*> ParserImpl::global_constant_decl(ast::AttributeList& attrs) {
    bool is_overridable = false;
    const char* use = nullptr;
    if (match(Token::Type::kLet)) {
        use = "let declaration";
    } else if (match(Token::Type::kOverride)) {
        use = "override declaration";
        is_overridable = true;
    } else {
        return Failure::kNoMatch;
    }

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

    return create<ast::Variable>(decl->source,                             // source
                                 builder_.Symbols().Register(decl->name),  // symbol
                                 ast::StorageClass::kNone,                 // storage class
                                 ast::Access::kUndefined,                  // access control
                                 decl->type,                               // type
                                 true,                                     // is_const
                                 is_overridable,                           // is_overridable
                                 initializer,                              // constructor
                                 std::move(attrs));                        // attributes
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

    auto decl = expect_variable_ident_decl("variable declaration", allow_inferred);
    if (decl.errored)
        return Failure::kErrored;

    return VarDeclInfo{decl->source, decl->name, vq.storage_class, vq.access, decl->type};
}

// texture_samplers
//  : sampler
//  | depth_texture
//  | sampled_texture LESS_THAN type_decl GREATER_THAN
//  | multisampled_texture LESS_THAN type_decl GREATER_THAN
//  | storage_texture LESS_THAN texel_format
//                         COMMA access GREATER_THAN
Maybe<const ast::Type*> ParserImpl::texture_samplers() {
    auto type = sampler();
    if (type.matched)
        return type;

    type = depth_texture();
    if (type.matched)
        return type;

    type = external_texture();
    if (type.matched)
        return type.value;

    auto source_range = make_source_range();

    auto dim = sampled_texture();
    if (dim.matched) {
        const char* use = "sampled texture type";

        auto subtype = expect_lt_gt_block(use, [&] { return expect_type(use); });
        if (subtype.errored)
            return Failure::kErrored;

        return builder_.ty.sampled_texture(source_range, dim.value, subtype.value);
    }

    auto ms_dim = multisampled_texture();
    if (ms_dim.matched) {
        const char* use = "multisampled texture type";

        auto subtype = expect_lt_gt_block(use, [&] { return expect_type(use); });
        if (subtype.errored)
            return Failure::kErrored;

        return builder_.ty.multisampled_texture(source_range, ms_dim.value, subtype.value);
    }

    auto storage = storage_texture();
    if (storage.matched) {
        const char* use = "storage texture type";
        using StorageTextureInfo = std::pair<tint::ast::TexelFormat, tint::ast::Access>;
        auto params = expect_lt_gt_block(use, [&]() -> Expect<StorageTextureInfo> {
            auto format = expect_texel_format(use);
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

        return builder_.ty.storage_texture(source_range, storage.value, params->first,
                                           params->second);
    }

    return Failure::kNoMatch;
}

// sampler
//  : SAMPLER
//  | SAMPLER_COMPARISON
Maybe<const ast::Type*> ParserImpl::sampler() {
    Source source;
    if (match(Token::Type::kSampler, &source))
        return builder_.ty.sampler(source, ast::SamplerKind::kSampler);

    if (match(Token::Type::kComparisonSampler, &source))
        return builder_.ty.sampler(source, ast::SamplerKind::kComparisonSampler);

    return Failure::kNoMatch;
}

// sampled_texture
//  : TEXTURE_SAMPLED_1D
//  | TEXTURE_SAMPLED_2D
//  | TEXTURE_SAMPLED_2D_ARRAY
//  | TEXTURE_SAMPLED_3D
//  | TEXTURE_SAMPLED_CUBE
//  | TEXTURE_SAMPLED_CUBE_ARRAY
Maybe<const ast::TextureDimension> ParserImpl::sampled_texture() {
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

// external_texture
//  : TEXTURE_EXTERNAL
Maybe<const ast::Type*> ParserImpl::external_texture() {
    Source source;
    if (match(Token::Type::kTextureExternal, &source)) {
        return builder_.ty.external_texture(source);
    }

    return Failure::kNoMatch;
}

// multisampled_texture
//  : TEXTURE_MULTISAMPLED_2D
Maybe<const ast::TextureDimension> ParserImpl::multisampled_texture() {
    if (match(Token::Type::kTextureMultisampled2d))
        return ast::TextureDimension::k2d;

    return Failure::kNoMatch;
}

// storage_texture
//  : TEXTURE_STORAGE_1D
//  | TEXTURE_STORAGE_2D
//  | TEXTURE_STORAGE_2D_ARRAY
//  | TEXTURE_STORAGE_3D
Maybe<const ast::TextureDimension> ParserImpl::storage_texture() {
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

// depth_texture
//  : TEXTURE_DEPTH_2D
//  | TEXTURE_DEPTH_2D_ARRAY
//  | TEXTURE_DEPTH_CUBE
//  | TEXTURE_DEPTH_CUBE_ARRAY
//  | TEXTURE_DEPTH_MULTISAMPLED_2D
Maybe<const ast::Type*> ParserImpl::depth_texture() {
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
        return builder_.ty.depth_multisampled_texture(source, ast::TextureDimension::k2d);
    }
    return Failure::kNoMatch;
}

// texel_format
//  : 'rgba8unorm'
//  | 'rgba8snorm'
//  | 'rgba8uint'
//  | 'rgba8sint'
//  | 'rgba16uint'
//  | 'rgba16sint'
//  | 'rgba16float'
//  | 'r32uint'
//  | 'r32sint'
//  | 'r32float'
//  | 'rg32uint'
//  | 'rg32sint'
//  | 'rg32float'
//  | 'rgba32uint'
//  | 'rgba32sint'
//  | 'rgba32float'
Expect<ast::TexelFormat> ParserImpl::expect_texel_format(std::string_view use) {
    auto t = next();
    if (t == "rgba8unorm") {
        return ast::TexelFormat::kRgba8Unorm;
    }
    if (t == "rgba8snorm") {
        return ast::TexelFormat::kRgba8Snorm;
    }
    if (t == "rgba8uint") {
        return ast::TexelFormat::kRgba8Uint;
    }
    if (t == "rgba8sint") {
        return ast::TexelFormat::kRgba8Sint;
    }
    if (t == "rgba16uint") {
        return ast::TexelFormat::kRgba16Uint;
    }
    if (t == "rgba16sint") {
        return ast::TexelFormat::kRgba16Sint;
    }
    if (t == "rgba16float") {
        return ast::TexelFormat::kRgba16Float;
    }
    if (t == "r32uint") {
        return ast::TexelFormat::kR32Uint;
    }
    if (t == "r32sint") {
        return ast::TexelFormat::kR32Sint;
    }
    if (t == "r32float") {
        return ast::TexelFormat::kR32Float;
    }
    if (t == "rg32uint") {
        return ast::TexelFormat::kRg32Uint;
    }
    if (t == "rg32sint") {
        return ast::TexelFormat::kRg32Sint;
    }
    if (t == "rg32float") {
        return ast::TexelFormat::kRg32Float;
    }
    if (t == "rgba32uint") {
        return ast::TexelFormat::kRgba32Uint;
    }
    if (t == "rgba32sint") {
        return ast::TexelFormat::kRgba32Sint;
    }
    if (t == "rgba32float") {
        return ast::TexelFormat::kRgba32Float;
    }
    return add_error(t.source(), "invalid format", use);
}

// variable_ident_decl
//   : IDENT COLON type_decl
Expect<ParserImpl::TypedIdentifier> ParserImpl::expect_variable_ident_decl(std::string_view use,
                                                                           bool allow_inferred) {
    auto ident = expect_ident(use);
    if (ident.errored)
        return Failure::kErrored;

    if (allow_inferred && !peek_is(Token::Type::kColon)) {
        return TypedIdentifier{nullptr, ident.value, ident.source};
    }

    if (!expect(use, Token::Type::kColon))
        return Failure::kErrored;

    auto t = peek();
    auto type = type_decl();
    if (type.errored)
        return Failure::kErrored;
    if (!type.matched)
        return add_error(t.source(), "invalid type", use);

    return TypedIdentifier{type.value, ident.value, ident.source};
}

Expect<ast::Access> ParserImpl::expect_access(std::string_view use) {
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
        return Expect<VariableQualifier>{VariableQualifier{sc.value, ast::Access::kUndefined},
                                         source};
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

    return builder_.ty.alias(make_source_range_from(t.source()), name.value, type.value);
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
//   | array_attribute_list* ARRAY LESS_THAN type_decl COMMA
//          INT_LITERAL GREATER_THAN
//   | array_attribute_list* ARRAY LESS_THAN type_decl
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
//   | texture_samplers
Maybe<const ast::Type*> ParserImpl::type_decl() {
    auto t = peek();
    Source source;
    if (match(Token::Type::kIdentifier, &source)) {
        return builder_.create<ast::TypeName>(source, builder_.Symbols().Register(t.to_str()));
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
        return expect_type_decl_array(t);
    }

    if (t.IsMatrix()) {
        next();  // Consume the peek
        return expect_type_decl_matrix(t);
    }

    auto texture_or_sampler = texture_samplers();
    if (texture_or_sampler.errored)
        return Failure::kErrored;
    if (texture_or_sampler.matched)
        return texture_or_sampler;

    return Failure::kNoMatch;
}

Expect<const ast::Type*> ParserImpl::expect_type(std::string_view use) {
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

    return builder_.ty.pointer(make_source_range_from(t.source()), subtype.value, storage_class,
                               access);
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
    if (t.Is(Token::Type::kVec3)) {
        count = 3;
    } else if (t.Is(Token::Type::kVec4)) {
        count = 4;
    }

    const ast::Type* subtype = nullptr;
    if (peek_is(Token::Type::kLessThan)) {
        const char* use = "vector";
        auto ty = expect_lt_gt_block(use, [&] { return expect_type(use); });
        if (ty.errored) {
            return Failure::kErrored;
        }
        subtype = ty.value;
    }

    return builder_.ty.vec(make_source_range_from(t.source()), subtype, count);
}

Expect<const ast::Type*> ParserImpl::expect_type_decl_array(Token t) {
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

    return builder_.ty.array(make_source_range_from(t.source()), subtype.value, size);
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

    const ast::Type* subtype = nullptr;
    if (peek_is(Token::Type::kLessThan)) {
        const char* use = "matrix";
        auto ty = expect_lt_gt_block(use, [&] { return expect_type(use); });
        if (ty.errored) {
            return Failure::kErrored;
        }
        subtype = ty.value;
    }

    return builder_.ty.mat(make_source_range_from(t.source()), subtype, columns, rows);
}

// storage_class
//  : INPUT
//  | OUTPUT
//  | UNIFORM
//  | WORKGROUP
//  | STORAGE
//  | PRIVATE
//  | FUNCTION
Expect<ast::StorageClass> ParserImpl::expect_storage_class(std::string_view use) {
    auto source = peek().source();

    if (match(Token::Type::kUniform))
        return {ast::StorageClass::kUniform, source};

    if (match(Token::Type::kWorkgroup))
        return {ast::StorageClass::kWorkgroup, source};

    if (match(Token::Type::kStorage))
        return {ast::StorageClass::kStorage, source};

    if (match(Token::Type::kPrivate))
        return {ast::StorageClass::kPrivate, source};

    if (match(Token::Type::kFunction))
        return {ast::StorageClass::kFunction, source};

    return add_error(source, "invalid storage class", use);
}

// struct_decl
//   : STRUCT IDENT struct_body_decl
Maybe<const ast::Struct*> ParserImpl::struct_decl() {
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
    return create<ast::Struct>(source, sym, std::move(body.value), ast::AttributeList{});
}

// struct_body_decl
//   : BRACE_LEFT (struct_member COMMA)* struct_member COMMA? BRACE_RIGHT
Expect<ast::StructMemberList> ParserImpl::expect_struct_body_decl() {
    return expect_brace_block("struct declaration", [&]() -> Expect<ast::StructMemberList> {
        ast::StructMemberList members;
        bool errored = false;
        while (continue_parsing()) {
            // Check for the end of the list.
            auto t = peek();
            if (!t.IsIdentifier() && !t.Is(Token::Type::kAttr)) {
                break;
            }

            auto member = expect_struct_member();
            if (member.errored) {
                errored = true;
                if (!sync_to(Token::Type::kComma, /* consume: */ false)) {
                    return Failure::kErrored;
                }
            } else {
                members.push_back(member.value);
            }

            // TODO(crbug.com/tint/1475): Remove support for semicolons.
            if (auto sc = peek(); sc.Is(Token::Type::kSemicolon)) {
                deprecated(sc.source(), "struct members should be separated with commas");
                next();
                continue;
            }
            if (!match(Token::Type::kComma))
                break;
        }
        if (errored) {
            return Failure::kErrored;
        }
        return members;
    });
}

// struct_member
//   : attribute* variable_ident_decl
Expect<ast::StructMember*> ParserImpl::expect_struct_member() {
    auto attrs = attribute_list();
    if (attrs.errored) {
        return Failure::kErrored;
    }

    auto decl = expect_variable_ident_decl("struct member");
    if (decl.errored)
        return Failure::kErrored;

    return create<ast::StructMember>(decl->source, builder_.Symbols().Register(decl->name),
                                     decl->type, std::move(attrs.value));
}

// function_decl
//   : function_header body_stmt
Maybe<const ast::Function*> ParserImpl::function_decl(ast::AttributeList& attrs) {
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

    return create<ast::Function>(header->source, builder_.Symbols().Register(header->name),
                                 header->params, header->return_type, body.value, attrs,
                                 header->return_type_attributes);
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
    ast::AttributeList return_attributes;

    if (match(Token::Type::kArrow)) {
        auto attrs = attribute_list();
        if (attrs.errored) {
            return Failure::kErrored;
        }
        return_attributes = attrs.value;

        auto type = type_decl();
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

    return FunctionHeader{source, name.value, std::move(params.value), return_type,
                          std::move(return_attributes)};
}

// param_list
//   :
//   | (param COMMA)* param COMMA?
Expect<ast::VariableList> ParserImpl::expect_param_list() {
    ast::VariableList ret;
    while (continue_parsing()) {
        // Check for the end of the list.
        auto t = peek();
        if (!t.IsIdentifier() && !t.Is(Token::Type::kAttr)) {
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
//   : attribute_list* variable_ident_decl
Expect<ast::Variable*> ParserImpl::expect_param() {
    auto attrs = attribute_list();

    auto decl = expect_variable_ident_decl("parameter");
    if (decl.errored)
        return Failure::kErrored;

    auto* var = create<ast::Variable>(decl->source,                             // source
                                      builder_.Symbols().Register(decl->name),  // symbol
                                      ast::StorageClass::kNone,                 // storage class
                                      ast::Access::kUndefined,                  // access control
                                      decl->type,                               // type
                                      true,                                     // is_const
                                      false,                                    // is_overridable
                                      nullptr,                                  // constructor
                                      std::move(attrs.value));                  // attributes
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
    if (t == kVertexStage) {
        next();  // Consume the peek
        return {ast::PipelineStage::kVertex, t.source()};
    }
    if (t == kFragmentStage) {
        next();  // Consume the peek
        return {ast::PipelineStage::kFragment, t.source()};
    }
    if (t == kComputeStage) {
        next();  // Consume the peek
        return {ast::PipelineStage::kCompute, t.source()};
    }
    return add_error(peek(), "invalid value for stage attribute");
}

Expect<ast::Builtin> ParserImpl::expect_builtin() {
    auto ident = expect_ident("builtin");
    if (ident.errored)
        return Failure::kErrored;

    ast::Builtin builtin = ident_to_builtin(ident.value);
    if (builtin == ast::Builtin::kNone)
        return add_error(ident.source, "invalid value for builtin attribute");

    return {builtin, ident.source};
}

// body_stmt
//   : BRACE_LEFT statements BRACE_RIGHT
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
//      | increment_stmt SEMICOLON
//      | decrement_stmt SEMICOLON
Maybe<const ast::Statement*> ParserImpl::statement() {
    while (match(Token::Type::kSemicolon)) {
        // Skip empty statements
    }

    // Non-block statments that error can resynchronize on semicolon.
    auto stmt = sync(Token::Type::kSemicolon, [&] { return non_block_statement(); });

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
//   | increment_stmt SEMICOLON
//   | decrement_stmt SEMICOLON
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

        auto* var = create<ast::Variable>(decl->source,                             // source
                                          builder_.Symbols().Register(decl->name),  // symbol
                                          ast::StorageClass::kNone,                 // storage class
                                          ast::Access::kUndefined,  // access control
                                          decl->type,               // type
                                          true,                     // is_const
                                          false,                    // is_overridable
                                          constructor.value,        // constructor
                                          ast::AttributeList{});    // attributes

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

    auto* var = create<ast::Variable>(decl->source,                             // source
                                      builder_.Symbols().Register(decl->name),  // symbol
                                      decl->storage_class,                      // storage class
                                      decl->access,                             // access control
                                      decl->type,                               // type
                                      false,                                    // is_const
                                      false,                                    // is_overridable
                                      constructor,                              // constructor
                                      ast::AttributeList{});                    // attributes

    return create<ast::VariableDeclStatement>(var->source, var);
}

// if_stmt
//   : IF expression compound_stmt ( ELSE else_stmt ) ?
// else_stmt
//  : body_stmt
//  | if_stmt
Maybe<const ast::IfStatement*> ParserImpl::if_stmt() {
    // Parse if-else chains iteratively instead of recursively, to avoid
    // stack-overflow for long chains of if-else statements.

    struct IfInfo {
        Source source;
        const ast::Expression* condition;
        const ast::BlockStatement* body;
    };

    // Parse an if statement, capturing the source, condition, and body statement.
    auto parse_if = [&]() -> Maybe<IfInfo> {
        Source source;
        if (!match(Token::Type::kIf, &source)) {
            return Failure::kNoMatch;
        }

        auto condition = logical_or_expression();
        if (condition.errored) {
            return Failure::kErrored;
        }
        if (!condition.matched) {
            return add_error(peek(), "unable to parse condition expression");
        }

        auto body = expect_body_stmt();
        if (body.errored) {
            return Failure::kErrored;
        }

        return IfInfo{source, condition.value, body.value};
    };

    std::vector<IfInfo> statements;

    // Parse the first if statement.
    auto first_if = parse_if();
    if (first_if.errored) {
        return Failure::kErrored;
    } else if (!first_if.matched) {
        return Failure::kNoMatch;
    }
    statements.push_back(first_if.value);

    // Parse the components of every "else {if}" in the chain.
    const ast::Statement* last_stmt = nullptr;
    while (continue_parsing()) {
        if (!match(Token::Type::kElse)) {
            break;
        }

        // Try to parse an "else if".
        auto else_if = parse_if();
        if (else_if.errored) {
            return Failure::kErrored;
        } else if (else_if.matched) {
            statements.push_back(else_if.value);
            continue;
        }

        // If it wasn't an "else if", it must just be an "else".
        auto else_body = expect_body_stmt();
        if (else_body.errored) {
            return Failure::kErrored;
        }
        last_stmt = else_body.value;
        break;
    }

    // Now walk back through the statements to create their AST nodes.
    for (auto itr = statements.rbegin(); itr != statements.rend(); itr++) {
        last_stmt = create<ast::IfStatement>(itr->source, itr->condition, itr->body, last_stmt);
    }

    return last_stmt->As<ast::IfStatement>();
}

// switch_stmt
//   : SWITCH paren_rhs_stmt BRACKET_LEFT switch_body+ BRACKET_RIGHT
Maybe<const ast::SwitchStatement*> ParserImpl::switch_stmt() {
    Source source;
    if (!match(Token::Type::kSwitch, &source))
        return Failure::kNoMatch;

    auto condition = logical_or_expression();
    if (condition.errored)
        return Failure::kErrored;
    if (!condition.matched) {
        return add_error(peek(), "unable to parse selector expression");
    }

    auto body = expect_brace_block("switch statement", [&]() -> Expect<ast::CaseStatementList> {
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
//   : CASE case_selectors COLON? BRACKET_LEFT case_body BRACKET_RIGHT
//   | DEFAULT COLON? BRACKET_LEFT case_body BRACKET_RIGHT
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

    // Consume the optional colon if present.
    match(Token::Type::kColon);

    const char* use = "case statement";
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
            return add_error(cond.value->source, "invalid case selector must be an integer value");
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

// (variable_stmt | increment_stmt | decrement_stmt | assignment_stmt |
// func_call_stmt)?
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

// (increment_stmt | decrement_stmt | assignment_stmt | func_call_stmt)?
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

    return std::make_unique<ForHeader>(initializer.value, condition.value, continuing.value);
}

// for_statement
//   : FOR PAREN_LEFT for_header PAREN_RIGHT BRACE_LEFT statements BRACE_RIGHT
Maybe<const ast::ForLoopStatement*> ParserImpl::for_stmt() {
    Source source;
    if (!match(Token::Type::kFor, &source))
        return Failure::kNoMatch;

    auto header = expect_paren_block("for loop", [&] { return expect_for_header(); });
    if (header.errored)
        return Failure::kErrored;

    auto stmts = expect_brace_block("for loop", [&] { return expect_statements(); });
    if (stmts.errored)
        return Failure::kErrored;

    return create<ast::ForLoopStatement>(source, header->initializer, header->condition,
                                         header->continuing,
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
        source,
        create<ast::CallExpression>(
            source, create<ast::IdentifierExpression>(source, builder_.Symbols().Register(name)),
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

        auto* ident =
            create<ast::IdentifierExpression>(t.source(), builder_.Symbols().Register(t.to_str()));

        if (peek_is(Token::Type::kParenLeft)) {
            auto params = expect_argument_expression_list("function call");
            if (params.errored)
                return Failure::kErrored;

            return create<ast::CallExpression>(source, ident, std::move(params.value));
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
Maybe<const ast::Expression*> ParserImpl::postfix_expression(const ast::Expression* prefix) {
    Source source;

    while (continue_parsing()) {
        if (match(Token::Type::kBracketLeft, &source)) {
            auto res = sync(Token::Type::kBracketRight, [&]() -> Maybe<const ast::Expression*> {
                auto param = logical_or_expression();
                if (param.errored)
                    return Failure::kErrored;
                if (!param.matched) {
                    return add_error(peek(), "unable to parse expression inside []");
                }

                if (!expect("index accessor", Token::Type::kBracketRight)) {
                    return Failure::kErrored;
                }

                return create<ast::IndexAccessorExpression>(source, prefix, param.value);
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
                create<ast::IdentifierExpression>(ident.source,
                                                  builder_.Symbols().Register(ident.value)));
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
Expect<ast::ExpressionList> ParserImpl::expect_argument_expression_list(std::string_view use) {
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
            peek(), "unable to parse right side of " + std::string(t.to_name()) + " expression");
    }

    return create<ast::UnaryOpExpression>(t.source(), op, expr.value);
}

// multiplicative_expr
//   :
//   | STAR unary_expression multiplicative_expr
//   | FORWARD_SLASH unary_expression multiplicative_expr
//   | MODULO unary_expression multiplicative_expr
Expect<const ast::Expression*> ParserImpl::expect_multiplicative_expr(const ast::Expression* lhs) {
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
                             "unable to parse right side of " + std::string(name) + " expression");
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
Expect<const ast::Expression*> ParserImpl::expect_additive_expr(const ast::Expression* lhs) {
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
Expect<const ast::Expression*> ParserImpl::expect_shift_expr(const ast::Expression* lhs) {
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
            return add_error(peek(),
                             std::string("unable to parse right side of ") + name + " expression");
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
Expect<const ast::Expression*> ParserImpl::expect_relational_expr(const ast::Expression* lhs) {
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
                             "unable to parse right side of " + std::string(name) + " expression");
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
Expect<const ast::Expression*> ParserImpl::expect_equality_expr(const ast::Expression* lhs) {
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
                             "unable to parse right side of " + std::string(name) + " expression");
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
Expect<const ast::Expression*> ParserImpl::expect_and_expr(const ast::Expression* lhs) {
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

        lhs = create<ast::BinaryExpression>(source, ast::BinaryOp::kAnd, lhs, rhs.value);
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
Expect<const ast::Expression*> ParserImpl::expect_exclusive_or_expr(const ast::Expression* lhs) {
    while (continue_parsing()) {
        Source source;
        if (!match(Token::Type::kXor, &source))
            return lhs;

        auto rhs = and_expression();
        if (rhs.errored)
            return Failure::kErrored;
        if (!rhs.matched)
            return add_error(peek(), "unable to parse right side of ^ expression");

        lhs = create<ast::BinaryExpression>(source, ast::BinaryOp::kXor, lhs, rhs.value);
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
Expect<const ast::Expression*> ParserImpl::expect_inclusive_or_expr(const ast::Expression* lhs) {
    while (continue_parsing()) {
        Source source;
        if (!match(Token::Type::kOr))
            return lhs;

        auto rhs = exclusive_or_expression();
        if (rhs.errored)
            return Failure::kErrored;
        if (!rhs.matched)
            return add_error(peek(), "unable to parse right side of | expression");

        lhs = create<ast::BinaryExpression>(source, ast::BinaryOp::kOr, lhs, rhs.value);
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
Expect<const ast::Expression*> ParserImpl::expect_logical_and_expr(const ast::Expression* lhs) {
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

        lhs = create<ast::BinaryExpression>(source, ast::BinaryOp::kLogicalAnd, lhs, rhs.value);
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
Expect<const ast::Expression*> ParserImpl::expect_logical_or_expr(const ast::Expression* lhs) {
    while (continue_parsing()) {
        Source source;
        if (!match(Token::Type::kOrOr))
            return lhs;

        auto rhs = logical_and_expression();
        if (rhs.errored)
            return Failure::kErrored;
        if (!rhs.matched)
            return add_error(peek(), "unable to parse right side of || expression");

        lhs = create<ast::BinaryExpression>(source, ast::BinaryOp::kLogicalOr, lhs, rhs.value);
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

// compound_assignment_operator:
// | plus_equal
// | minus_equal
// | times_equal
// | division_equal
// | modulo_equal
// | and_equal
// | or_equal
// | xor_equal
Maybe<ast::BinaryOp> ParserImpl::compound_assignment_operator() {
    ast::BinaryOp compound_op = ast::BinaryOp::kNone;
    if (peek_is(Token::Type::kPlusEqual)) {
        compound_op = ast::BinaryOp::kAdd;
    } else if (peek_is(Token::Type::kMinusEqual)) {
        compound_op = ast::BinaryOp::kSubtract;
    } else if (peek_is(Token::Type::kTimesEqual)) {
        compound_op = ast::BinaryOp::kMultiply;
    } else if (peek_is(Token::Type::kDivisionEqual)) {
        compound_op = ast::BinaryOp::kDivide;
    } else if (peek_is(Token::Type::kModuloEqual)) {
        compound_op = ast::BinaryOp::kModulo;
    } else if (peek_is(Token::Type::kAndEqual)) {
        compound_op = ast::BinaryOp::kAnd;
    } else if (peek_is(Token::Type::kOrEqual)) {
        compound_op = ast::BinaryOp::kOr;
    } else if (peek_is(Token::Type::kXorEqual)) {
        compound_op = ast::BinaryOp::kXor;
    }
    if (compound_op != ast::BinaryOp::kNone) {
        next();
        return compound_op;
    }
    return Failure::kNoMatch;
}

// assignment_stmt
// | lhs_expression ( equal | compound_assignment_operator ) expression
// | underscore equal expression
// increment_stmt
// | lhs_expression PLUS_PLUS
// decrement_stmt
// | lhs_expression MINUS_MINUS
Maybe<const ast::Statement*> ParserImpl::assignment_stmt() {
    auto t = peek();
    auto source = t.source();

    // tint:295 - Test for `ident COLON` - this is invalid grammar, and without
    // special casing will error as "missing = for assignment", which is less
    // helpful than this error message:
    if (peek_is(Token::Type::kIdentifier) && peek_is(Token::Type::kColon, 1)) {
        return add_error(peek(0).source(), "expected 'var' for variable declaration");
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

    // Handle increment and decrement statements.
    // We do this here because the parsing of the LHS expression overlaps with
    // the assignment statement, and we cannot tell which we are parsing until we
    // hit the ++/--/= token.
    if (match(Token::Type::kPlusPlus)) {
        return create<ast::IncrementDecrementStatement>(source, lhs.value, true);
    } else if (match(Token::Type::kMinusMinus)) {
        return create<ast::IncrementDecrementStatement>(source, lhs.value, false);
    }

    auto compound_op = compound_assignment_operator();
    if (compound_op.errored) {
        return Failure::kErrored;
    }
    if (!compound_op.matched) {
        if (!expect("assignment", Token::Type::kEqual)) {
            return Failure::kErrored;
        }
    }

    auto rhs = logical_or_expression();
    if (rhs.errored) {
        return Failure::kErrored;
    }
    if (!rhs.matched) {
        return add_error(peek(), "unable to parse right side of assignment");
    }

    if (compound_op.value != ast::BinaryOp::kNone) {
        return create<ast::CompoundAssignmentStatement>(source, lhs.value, rhs.value,
                                                        compound_op.value);
    } else {
        return create<ast::AssignmentStatement>(source, lhs.value, rhs.value);
    }
}

// const_literal
//   : INT_LITERAL
//   | FLOAT_LITERAL
//   | TRUE
//   | FALSE
Maybe<const ast::LiteralExpression*> ParserImpl::const_literal() {
    auto t = peek();
    if (match(Token::Type::kIntLiteral)) {
        return create<ast::IntLiteralExpression>(t.source(), t.to_i64(),
                                                 ast::IntLiteralExpression::Suffix::kNone);
    }
    if (match(Token::Type::kIntILiteral)) {
        return create<ast::IntLiteralExpression>(t.source(), t.to_i64(),
                                                 ast::IntLiteralExpression::Suffix::kI);
    }
    if (match(Token::Type::kIntULiteral)) {
        return create<ast::IntLiteralExpression>(t.source(), t.to_i64(),
                                                 ast::IntLiteralExpression::Suffix::kU);
    }
    if (match(Token::Type::kFloatLiteral)) {
        return create<ast::FloatLiteralExpression>(t.source(), t.to_f32());
    }
    if (match(Token::Type::kTrue)) {
        return create<ast::BoolLiteralExpression>(t.source(), true);
    }
    if (match(Token::Type::kFalse)) {
        return create<ast::BoolLiteralExpression>(t.source(), false);
    }
    if (handle_error(t)) {
        return Failure::kErrored;
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

    if (peek_is(Token::Type::kParenLeft, 1) || peek_is(Token::Type::kLessThan, 1)) {
        auto type = expect_type("const_expr");
        if (type.errored) {
            return Failure::kErrored;
        }

        auto params = expect_paren_block("type constructor", [&]() -> Expect<ast::ExpressionList> {
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

Maybe<ast::AttributeList> ParserImpl::attribute_list() {
    bool errored = false;
    ast::AttributeList attrs;

    while (continue_parsing()) {
        if (match(Token::Type::kAttr)) {
            if (auto attr = expect_attribute(); attr.errored) {
                errored = true;
            } else {
                attrs.emplace_back(attr.value);
            }
        } else {
            break;
        }
    }

    if (errored)
        return Failure::kErrored;

    if (attrs.empty())
        return Failure::kNoMatch;

    return attrs;
}

Expect<const ast::Attribute*> ParserImpl::expect_attribute() {
    auto t = peek();
    auto attr = attribute();
    if (attr.errored)
        return Failure::kErrored;
    if (attr.matched)
        return attr.value;
    return add_error(t, "expected attribute");
}

Maybe<const ast::Attribute*> ParserImpl::attribute() {
    using Result = Maybe<const ast::Attribute*>;
    auto t = next();

    if (!t.IsIdentifier()) {
        return Failure::kNoMatch;
    }

    if (t == kLocationAttribute) {
        const char* use = "location attribute";
        return expect_paren_block(use, [&]() -> Result {
            auto val = expect_positive_sint(use);
            if (val.errored)
                return Failure::kErrored;

            return create<ast::LocationAttribute>(t.source(), val.value);
        });
    }

    if (t == kBindingAttribute) {
        const char* use = "binding attribute";
        return expect_paren_block(use, [&]() -> Result {
            auto val = expect_positive_sint(use);
            if (val.errored)
                return Failure::kErrored;

            return create<ast::BindingAttribute>(t.source(), val.value);
        });
    }

    if (t == kGroupAttribute) {
        const char* use = "group attribute";
        return expect_paren_block(use, [&]() -> Result {
            auto val = expect_positive_sint(use);
            if (val.errored)
                return Failure::kErrored;

            return create<ast::GroupAttribute>(t.source(), val.value);
        });
    }

    if (t == kInterpolateAttribute) {
        return expect_paren_block("interpolate attribute", [&]() -> Result {
            ast::InterpolationType type;
            ast::InterpolationSampling sampling = ast::InterpolationSampling::kNone;

            auto type_tok = next();
            if (type_tok == "perspective") {
                type = ast::InterpolationType::kPerspective;
            } else if (type_tok == "linear") {
                type = ast::InterpolationType::kLinear;
            } else if (type_tok == "flat") {
                type = ast::InterpolationType::kFlat;
            } else {
                return add_error(type_tok, "invalid interpolation type");
            }

            if (match(Token::Type::kComma)) {
                auto sampling_tok = next();
                if (sampling_tok == "center") {
                    sampling = ast::InterpolationSampling::kCenter;
                } else if (sampling_tok == "centroid") {
                    sampling = ast::InterpolationSampling::kCentroid;
                } else if (sampling_tok == "sample") {
                    sampling = ast::InterpolationSampling::kSample;
                } else {
                    return add_error(sampling_tok, "invalid interpolation sampling");
                }
            }

            return create<ast::InterpolateAttribute>(t.source(), type, sampling);
        });
    }

    if (t == kInvariantAttribute) {
        return create<ast::InvariantAttribute>(t.source());
    }

    if (t == kBuiltinAttribute) {
        return expect_paren_block("builtin attribute", [&]() -> Result {
            auto builtin = expect_builtin();
            if (builtin.errored)
                return Failure::kErrored;

            return create<ast::BuiltinAttribute>(t.source(), builtin.value);
        });
    }

    if (t == kWorkgroupSizeAttribute) {
        return expect_paren_block("workgroup_size attribute", [&]() -> Result {
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

            return create<ast::WorkgroupAttribute>(t.source(), x, y, z);
        });
    }

    if (t == kStageAttribute) {
        return expect_paren_block("stage attribute", [&]() -> Result {
            auto stage = expect_pipeline_stage();
            if (stage.errored)
                return Failure::kErrored;

            return create<ast::StageAttribute>(t.source(), stage.value);
        });
    }

    if (t == kSizeAttribute) {
        const char* use = "size attribute";
        return expect_paren_block(use, [&]() -> Result {
            auto val = expect_positive_sint(use);
            if (val.errored)
                return Failure::kErrored;

            return create<ast::StructMemberSizeAttribute>(t.source(), val.value);
        });
    }

    if (t == kAlignAttribute) {
        const char* use = "align attribute";
        return expect_paren_block(use, [&]() -> Result {
            auto val = expect_positive_sint(use);
            if (val.errored)
                return Failure::kErrored;

            return create<ast::StructMemberAlignAttribute>(t.source(), val.value);
        });
    }

    if (t == kIdAttribute) {
        const char* use = "id attribute";
        return expect_paren_block(use, [&]() -> Result {
            auto val = expect_positive_sint(use);
            if (val.errored)
                return Failure::kErrored;

            return create<ast::IdAttribute>(t.source(), val.value);
        });
    }

    return Failure::kNoMatch;
}

bool ParserImpl::expect_attributes_consumed(ast::AttributeList& in) {
    if (in.empty()) {
        return true;
    }
    add_error(in[0]->source, "unexpected attributes");
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

bool ParserImpl::expect(std::string_view use, Token::Type tok) {
    auto t = peek();
    if (t.Is(tok)) {
        next();
        synchronized_ = true;
        return true;
    }

    // Special case to split `>>` and `>=` tokens if we are looking for a `>`.
    if (tok == Token::Type::kGreaterThan &&
        (t.Is(Token::Type::kShiftRight) || t.Is(Token::Type::kGreaterThanEqual))) {
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

    // Error cases
    synchronized_ = false;
    if (handle_error(t)) {
        return false;
    }

    std::stringstream err;
    err << "expected '" << Token::TypeToName(tok) << "'";
    if (!use.empty()) {
        err << " for " << use;
    }
    add_error(t, err.str());
    return false;
}

Expect<int32_t> ParserImpl::expect_sint(std::string_view use) {
    auto t = peek();
    if (!t.Is(Token::Type::kIntLiteral) && !t.Is(Token::Type::kIntILiteral)) {
        return add_error(t.source(), "expected signed integer literal", use);
    }

    int64_t val = t.to_i64();
    if ((val > std::numeric_limits<int32_t>::max()) ||
        (val < std::numeric_limits<int32_t>::min())) {
        // TODO(crbug.com/tint/1504): Test this when abstract int is implemented
        return add_error(t.source(), "value overflows i32", use);
    }

    next();
    return {static_cast<int32_t>(t.to_i64()), t.source()};
}

Expect<uint32_t> ParserImpl::expect_positive_sint(std::string_view use) {
    auto sint = expect_sint(use);
    if (sint.errored)
        return Failure::kErrored;

    if (sint.value < 0)
        return add_error(sint.source, std::string(use) + " must be positive");

    return {static_cast<uint32_t>(sint.value), sint.source};
}

Expect<uint32_t> ParserImpl::expect_nonzero_positive_sint(std::string_view use) {
    auto sint = expect_sint(use);
    if (sint.errored)
        return Failure::kErrored;

    if (sint.value <= 0)
        return add_error(sint.source, std::string(use) + " must be greater than 0");

    return {static_cast<uint32_t>(sint.value), sint.source};
}

Expect<std::string> ParserImpl::expect_ident(std::string_view use) {
    auto t = peek();
    if (t.IsIdentifier()) {
        synchronized_ = true;
        next();

        if (is_reserved(t)) {
            return add_error(t.source(), "'" + t.to_str() + "' is a reserved keyword");
        }

        return {t.to_str(), t.source()};
    }
    if (handle_error(t)) {
        return Failure::kErrored;
    }
    synchronized_ = false;
    return add_error(t.source(), "expected identifier", use);
}

template <typename F, typename T>
T ParserImpl::expect_block(Token::Type start, Token::Type end, std::string_view use, F&& body) {
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
T ParserImpl::expect_paren_block(std::string_view use, F&& body) {
    return expect_block(Token::Type::kParenLeft, Token::Type::kParenRight, use,
                        std::forward<F>(body));
}

template <typename F, typename T>
T ParserImpl::expect_brace_block(std::string_view use, F&& body) {
    return expect_block(Token::Type::kBraceLeft, Token::Type::kBraceRight, use,
                        std::forward<F>(body));
}

template <typename F, typename T>
T ParserImpl::expect_lt_gt_block(std::string_view use, F&& body) {
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

bool ParserImpl::handle_error(const Token& t) {
    // The token might itself be an error.
    if (t.IsError()) {
        synchronized_ = false;
        add_error(t.source(), t.to_str());
        return true;
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

ParserImpl::MultiTokenSource ParserImpl::make_source_range_from(const Source& start) {
    return MultiTokenSource(this, start);
}

}  // namespace tint::reader::wgsl
