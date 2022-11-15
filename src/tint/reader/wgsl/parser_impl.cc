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
#include "src/tint/ast/break_if_statement.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/external_texture.h"
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
#include "src/tint/utils/string.h"

namespace tint::reader::wgsl {
namespace {

using Void = ParserImpl::Void;

/// An instance of Void that can be used to signal success for functions that return Expect<Void> or
/// Maybe<NoError>.
static constexpr Void kSuccess;

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

// https://gpuweb.github.io/gpuweb/wgsl.html#reserved-keywords
bool is_reserved(const Token& t) {
    return t == "CompileShader" || t == "ComputeShader" || t == "DomainShader" ||
           t == "GeometryShader" || t == "Hullshader" || t == "NULL" || t == "Self" ||
           t == "abstract" || t == "active" || t == "alignas" || t == "alignof" || t == "as" ||
           t == "asm" || t == "asm_fragment" || t == "async" || t == "attribute" || t == "auto" ||
           t == "await" || t == "become" || t == "binding_array" || t == "cast" || t == "catch" ||
           t == "class" || t == "co_await" || t == "co_return" || t == "co_yield" ||
           t == "coherent" || t == "column_major" || t == "common" || t == "compile" ||
           t == "compile_fragment" || t == "concept" || t == "const_cast" || t == "consteval" ||
           t == "constexpr" || t == "constinit" || t == "crate" || t == "debugger" ||
           t == "decltype" || t == "delete" || t == "demote" || t == "demote_to_helper" ||
           t == "do" || t == "dynamic_cast" || t == "enum" || t == "explicit" || t == "export" ||
           t == "extends" || t == "extern" || t == "external" || t == "filter" || t == "final" ||
           t == "finally" || t == "friend" || t == "from" || t == "fxgroup" || t == "get" ||
           t == "goto" || t == "groupshared" || t == "handle" || t == "highp" || t == "impl" ||
           t == "implements" || t == "import" || t == "inline" || t == "inout" ||
           t == "instanceof" || t == "interface" || t == "invariant" || t == "layout" ||
           t == "line" || t == "lineadj" || t == "lowp" || t == "macro" || t == "macro_rules" ||
           t == "match" || t == "mediump" || t == "meta" || t == "mod" || t == "module" ||
           t == "move" || t == "mut" || t == "mutable" || t == "namespace" || t == "new" ||
           t == "nil" || t == "noexcept" || t == "noinline" || t == "nointerpolation" ||
           t == "noperspective" || t == "null" || t == "nullptr" || t == "of" || t == "operator" ||
           t == "package" || t == "packoffset" || t == "partition" || t == "pass" || t == "patch" ||
           t == "pixelfragment" || t == "point" || t == "precise" || t == "precision" ||
           t == "premerge" || t == "priv" || t == "protected" || t == "pub" || t == "public" ||
           t == "readonly" || t == "ref" || t == "regardless" || t == "register" ||
           t == "reinterpret_cast" || t == "requires" || t == "resource" || t == "restrict" ||
           t == "self" || t == "set" || t == "shared" || t == "signed" || t == "sizeof" ||
           t == "smooth" || t == "snorm" || t == "static" || t == "static_cast" || t == "std" ||
           t == "subroutine" || t == "super" || t == "target" || t == "template" || t == "this" ||
           t == "thread_local" || t == "throw" || t == "trait" || t == "try" || t == "typedef" ||
           t == "typeid" || t == "typename" || t == "typeof" || t == "union" || t == "unless" ||
           t == "unorm" || t == "unsafe" || t == "unsized" || t == "use" || t == "using" ||
           t == "varying" || t == "virtual" || t == "volatile" || t == "wgsl" || t == "where" ||
           t == "with" || t == "writeonly" || t == "yield";
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
        if (t.Is(Token::Type::kBraceLeft)) {
            return brace++;
        }
        if (t.Is(Token::Type::kBraceRight)) {
            return brace--;
        }
        if (t.Is(Token::Type::kBracketLeft)) {
            return bracket++;
        }
        if (t.Is(Token::Type::kBracketRight)) {
            return bracket--;
        }
        if (t.Is(Token::Type::kParenLeft)) {
            return paren++;
        }
        if (t.Is(Token::Type::kParenRight)) {
            return paren--;
        }
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
    MultiTokenSource(ParserImpl* parser, const tint::Source& start)
        : parser_(parser), start_(start) {}

    /// @returns the Source that returns the combined source from start to the current last token's
    /// source.
    tint::Source Source() const {
        auto end = parser_->last_source().End();
        if (end < start_) {
            end = start_;
        }
        return Source::Combine(start_, end);
    }

    /// Implicit conversion to Source that returns the combined source from start to the current
    /// last token's source.
    operator tint::Source() const { return Source(); }

  private:
    ParserImpl* parser_;
    tint::Source start_;
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
                                           utils::VectorRef<const ast::Parameter*> p,
                                           const ast::Type* ret_ty,
                                           utils::VectorRef<const ast::Attribute*> ret_attrs)
    : source(src),
      name(n),
      params(std::move(p)),
      return_type(ret_ty),
      return_type_attributes(std::move(ret_attrs)) {}

ParserImpl::FunctionHeader::~FunctionHeader() = default;

ParserImpl::FunctionHeader& ParserImpl::FunctionHeader::operator=(const FunctionHeader& rhs) =
    default;

ParserImpl::VarDeclInfo::VarDeclInfo() = default;

ParserImpl::VarDeclInfo::VarDeclInfo(const VarDeclInfo&) = default;

ParserImpl::VarDeclInfo::VarDeclInfo(Source source_in,
                                     std::string name_in,
                                     ast::AddressSpace address_space_in,
                                     ast::Access access_in,
                                     const ast::Type* type_in)
    : source(std::move(source_in)),
      name(std::move(name_in)),
      address_space(address_space_in),
      access(access_in),
      type(type_in) {}

ParserImpl::VarDeclInfo::~VarDeclInfo() = default;

ParserImpl::ParserImpl(Source::File const* file) : file_(file) {}

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

const Token& ParserImpl::next() {
    // If the next token is already an error or the end of file, stay there.
    if (tokens_[next_token_idx_].IsEof() || tokens_[next_token_idx_].IsError()) {
        return tokens_[next_token_idx_];
    }

    // Skip over any placeholder elements
    while (true) {
        if (!tokens_[next_token_idx_].IsPlaceholder()) {
            break;
        }
        next_token_idx_++;
    }
    last_source_idx_ = next_token_idx_;

    if (!tokens_[next_token_idx_].IsEof() && !tokens_[next_token_idx_].IsError()) {
        next_token_idx_++;
    }
    return tokens_[last_source_idx_];
}

const Token& ParserImpl::peek(size_t count) {
    for (size_t idx = next_token_idx_; idx < tokens_.size(); idx++) {
        if (tokens_[idx].IsPlaceholder()) {
            continue;
        }
        if (count == 0) {
            return tokens_[idx];
        }
        count--;
    }
    // Walked off the end of the token list, return last token.
    return tokens_[tokens_.size() - 1];
}

bool ParserImpl::peek_is(Token::Type tok, size_t idx) {
    return peek(idx).Is(tok);
}

void ParserImpl::split_token(Token::Type lhs, Token::Type rhs) {
    if (next_token_idx_ == 0) {
        TINT_ICE(Reader, builder_.Diagnostics())
            << "attempt to update placeholder at beginning of tokens";
    }
    if (next_token_idx_ >= tokens_.size()) {
        TINT_ICE(Reader, builder_.Diagnostics())
            << "attempt to update placeholder past end of tokens";
    }
    if (!tokens_[next_token_idx_].IsPlaceholder()) {
        TINT_ICE(Reader, builder_.Diagnostics()) << "attempt to update non-placeholder token";
    }
    tokens_[next_token_idx_ - 1].SetType(lhs);
    tokens_[next_token_idx_].SetType(rhs);
}

Source ParserImpl::last_source() const {
    return tokens_[last_source_idx_].source();
}

void ParserImpl::InitializeLex() {
    Lexer l{file_};
    tokens_ = l.Lex();
}

bool ParserImpl::Parse() {
    InitializeLex();
    translation_unit();
    return !has_error();
}

// translation_unit
//  : global_directive* global_decl* EOF
void ParserImpl::translation_unit() {
    bool after_global_decl = false;
    while (continue_parsing()) {
        auto& p = peek();
        if (p.IsEof()) {
            break;
        }

        auto ed = global_directive(after_global_decl);
        if (!ed.matched && !ed.errored) {
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

// global_directive
//  : enable_directive
Maybe<Void> ParserImpl::global_directive(bool have_parsed_decl) {
    auto& p = peek();
    auto ed = enable_directive();
    if (ed.matched && have_parsed_decl) {
        return add_error(p, "enable directives must come before all global declarations");
    }
    return ed;
}

// enable_directive
//  : enable name SEMICLON
Maybe<Void> ParserImpl::enable_directive() {
    auto decl = sync(Token::Type::kSemicolon, [&]() -> Maybe<Void> {
        if (!match(Token::Type::kEnable)) {
            return Failure::kNoMatch;
        }

        // Match the extension name.
        auto& t = peek();
        if (handle_error(t)) {
            // The token might itself be an error.
            return Failure::kErrored;
        }

        if (t.Is(Token::Type::kParenLeft)) {
            // A common error case is writing `enable(foo);` instead of `enable foo;`.
            synchronized_ = false;
            return add_error(t.source(), "enable directives don't take parenthesis");
        }

        auto extension = ast::Extension::kUndefined;
        if (t.Is(Token::Type::kF16)) {
            // `f16` is a valid extension name and also a keyword
            synchronized_ = true;
            next();
            extension = ast::Extension::kF16;
        } else {
            auto ext = expect_enum("extension", ast::ParseExtension, ast::kExtensionStrings);
            if (ext.errored) {
                return Failure::kErrored;
            }
            extension = ext.value;
        }

        if (!expect("enable directive", Token::Type::kSemicolon)) {
            return Failure::kErrored;
        }
        builder_.AST().AddEnable(create<ast::Enable>(t.source(), extension));
        return kSuccess;
    });

    if (decl.errored) {
        return Failure::kErrored;
    }
    if (decl.matched) {
        return kSuccess;
    }

    return Failure::kNoMatch;
}

// global_decl
//  : SEMICOLON
//  | global_variable_decl SEMICOLON
//  | global_constant_decl SEMICOLON
//  | type_alias_decl SEMICOLON
//  | struct_decl
//  | function_decl
//  | static_assert_statement SEMICOLON
Maybe<Void> ParserImpl::global_decl() {
    if (match(Token::Type::kSemicolon) || match(Token::Type::kEOF)) {
        return kSuccess;
    }

    bool errored = false;
    auto attrs = attribute_list();
    if (attrs.errored) {
        errored = true;
    }
    if (!continue_parsing()) {
        return Failure::kErrored;
    }

    auto decl = sync(Token::Type::kSemicolon, [&]() -> Maybe<Void> {
        auto gv = global_variable_decl(attrs.value);
        if (gv.errored) {
            return Failure::kErrored;
        }
        if (gv.matched) {
            if (!expect("variable declaration", Token::Type::kSemicolon)) {
                return Failure::kErrored;
            }

            builder_.AST().AddGlobalVariable(gv.value);
            return kSuccess;
        }

        auto gc = global_constant_decl(attrs.value);
        if (gc.errored) {
            return Failure::kErrored;
        }
        if (gc.matched) {
            // Avoid the cost of the string allocation for the common no-error case
            if (!peek().Is(Token::Type::kSemicolon)) {
                std::string kind = gc->Kind();
                if (!expect("'" + kind + "' declaration", Token::Type::kSemicolon)) {
                    return Failure::kErrored;
                }
            }

            builder_.AST().AddGlobalVariable(gc.value);
            return kSuccess;
        }

        auto ta = type_alias_decl();
        if (ta.errored) {
            return Failure::kErrored;
        }
        if (ta.matched) {
            if (!expect("type alias", Token::Type::kSemicolon)) {
                return Failure::kErrored;
            }

            builder_.AST().AddTypeDecl(ta.value);
            return kSuccess;
        }

        auto assertion = static_assert_statement();
        if (assertion.errored) {
            return Failure::kErrored;
        }
        if (assertion.matched) {
            builder_.AST().AddStaticAssert(assertion.value);
            if (!expect("static assertion declaration", Token::Type::kSemicolon)) {
                return Failure::kErrored;
            }
            return kSuccess;
        }

        return Failure::kNoMatch;
    });

    if (decl.errored) {
        errored = true;
    }
    if (decl.matched) {
        if (!expect_attributes_consumed(attrs.value)) {
            return Failure::kErrored;
        }
        return kSuccess;
    }

    auto str = struct_decl();
    if (str.errored) {
        errored = true;
    }
    if (str.matched) {
        builder_.AST().AddTypeDecl(str.value);
        if (!expect_attributes_consumed(attrs.value)) {
            return Failure::kErrored;
        }
        return kSuccess;
    }

    auto func = function_decl(attrs.value);
    if (func.errored) {
        errored = true;
    }
    if (func.matched) {
        builder_.AST().AddFunction(func.value);
        return kSuccess;
    }

    if (errored) {
        return Failure::kErrored;
    }

    // Invalid syntax found - try and determine the best error message

    // We have attributes parsed, but nothing to consume them?
    if (attrs.value.Length() > 0) {
        return add_error(next(), "expected declaration after attributes");
    }

    // We have a statement outside of a function?
    auto& t = peek();
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
//  : variable_attribute_list* variable_decl (EQUAL expression)?
Maybe<const ast::Variable*> ParserImpl::global_variable_decl(AttributeList& attrs) {
    auto decl = variable_decl();
    if (decl.errored) {
        return Failure::kErrored;
    }
    if (!decl.matched) {
        return Failure::kNoMatch;
    }

    const ast::Expression* initializer = nullptr;
    if (match(Token::Type::kEqual)) {
        auto expr = expression();
        if (expr.errored) {
            return Failure::kErrored;
        }
        if (!expr.matched) {
            return add_error(peek(), "missing initializer for 'var' declaration");
        }
        initializer = expr.value;
    }

    TINT_DEFER(attrs.Clear());

    return create<ast::Var>(decl->source,                             // source
                            builder_.Symbols().Register(decl->name),  // symbol
                            decl->type,                               // type
                            decl->address_space,                      // address space
                            decl->access,                             // access control
                            initializer,                              // initializer
                            std::move(attrs));                        // attributes
}

// global_constant_decl :
//  | LET optionally_typed_ident global_const_initializer
//  | attribute* override optionally_typed_ident (equal expression)?
// global_const_initializer
//  : EQUAL const_expr
Maybe<const ast::Variable*> ParserImpl::global_constant_decl(AttributeList& attrs) {
    bool is_const = false;
    bool is_overridable = false;
    const char* use = nullptr;
    Source source;
    if (match(Token::Type::kConst)) {
        use = "'const' declaration";
    } else if (match(Token::Type::kOverride)) {
        use = "'override' declaration";
        is_overridable = true;
    } else if (match(Token::Type::kLet, &source)) {
        return add_error(source, "module-scope 'let' is invalid, use 'const'");
    } else {
        return Failure::kNoMatch;
    }

    auto decl = expect_optionally_typed_ident(use);
    if (decl.errored) {
        return Failure::kErrored;
    }

    bool has_initializer = false;
    if (is_overridable) {
        has_initializer = match(Token::Type::kEqual);
    } else {
        if (!expect(use, Token::Type::kEqual)) {
            return Failure::kErrored;
        }
        has_initializer = true;
    }

    const ast::Expression* initializer = nullptr;
    if (has_initializer) {
        auto expr = expression();
        if (expr.errored) {
            return Failure::kErrored;
        }
        if (!expr.matched) {
            return add_error(peek(), "missing initializer for " + std::string(use));
        }
        initializer = std::move(expr.value);
    }

    TINT_DEFER(attrs.Clear());

    if (is_const) {
        return create<ast::Const>(decl->source,                             // source
                                  builder_.Symbols().Register(decl->name),  // symbol
                                  decl->type,                               // type
                                  initializer,                              // initializer
                                  std::move(attrs));                        // attributes
    }
    if (is_overridable) {
        return create<ast::Override>(decl->source,                             // source
                                     builder_.Symbols().Register(decl->name),  // symbol
                                     decl->type,                               // type
                                     initializer,                              // initializer
                                     std::move(attrs));                        // attributes
    }
    return create<ast::Const>(decl->source,                             // source
                              builder_.Symbols().Register(decl->name),  // symbol
                              decl->type,                               // type
                              initializer,                              // initializer
                              std::move(attrs));                        // attributes
}

// variable_decl
//   : VAR variable_qualifier? optionally_typed_ident
//
// Note, the `( LESS_THAN address_space ( COMMA access_mode )? GREATER_THAN ) is pulled out into
// a `variable_qualifier` helper.
Maybe<ParserImpl::VarDeclInfo> ParserImpl::variable_decl() {
    Source source;
    if (!match(Token::Type::kVar, &source)) {
        return Failure::kNoMatch;
    }

    VariableQualifier vq;
    auto explicit_vq = variable_qualifier();
    if (explicit_vq.errored) {
        return Failure::kErrored;
    }
    if (explicit_vq.matched) {
        vq = explicit_vq.value;
    }

    auto decl = expect_optionally_typed_ident("variable declaration");
    if (decl.errored) {
        return Failure::kErrored;
    }

    return VarDeclInfo{decl->source, decl->name, vq.address_space, vq.access, decl->type};
}

// texture_and_sampler_types
//  : sampler_type
//  | depth_texture_type
//  | sampled_texture_type LESS_THAN type_specifier GREATER_THAN
//  | multisampled_texture_type LESS_THAN type_specifier GREATER_THAN
//  | storage_texture_type LESS_THAN texel_format
//                         COMMA access_mode GREATER_THAN
Maybe<const ast::Type*> ParserImpl::texture_and_sampler_types() {
    auto type = sampler_type();
    if (type.matched) {
        return type;
    }

    type = depth_texture_type();
    if (type.matched) {
        return type;
    }

    type = external_texture();
    if (type.matched) {
        return type.value;
    }

    auto source_range = make_source_range();

    auto dim = sampled_texture_type();
    if (dim.matched) {
        const char* use = "sampled texture type";

        auto subtype = expect_lt_gt_block(use, [&] { return expect_type(use); });
        if (subtype.errored) {
            return Failure::kErrored;
        }

        return builder_.ty.sampled_texture(source_range, dim.value, subtype.value);
    }

    auto ms_dim = multisampled_texture_type();
    if (ms_dim.matched) {
        const char* use = "multisampled texture type";

        auto subtype = expect_lt_gt_block(use, [&] { return expect_type(use); });
        if (subtype.errored) {
            return Failure::kErrored;
        }

        return builder_.ty.multisampled_texture(source_range, ms_dim.value, subtype.value);
    }

    auto storage = storage_texture_type();
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

            auto access = expect_access_mode(use);
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

// sampler_type
//  : SAMPLER
//  | SAMPLER_COMPARISON
Maybe<const ast::Type*> ParserImpl::sampler_type() {
    Source source;
    if (match(Token::Type::kSampler, &source)) {
        return builder_.ty.sampler(source, ast::SamplerKind::kSampler);
    }

    if (match(Token::Type::kComparisonSampler, &source)) {
        return builder_.ty.sampler(source, ast::SamplerKind::kComparisonSampler);
    }

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
    if (match(Token::Type::kTextureSampled1d)) {
        return ast::TextureDimension::k1d;
    }

    if (match(Token::Type::kTextureSampled2d)) {
        return ast::TextureDimension::k2d;
    }

    if (match(Token::Type::kTextureSampled2dArray)) {
        return ast::TextureDimension::k2dArray;
    }

    if (match(Token::Type::kTextureSampled3d)) {
        return ast::TextureDimension::k3d;
    }

    if (match(Token::Type::kTextureSampledCube)) {
        return ast::TextureDimension::kCube;
    }

    if (match(Token::Type::kTextureSampledCubeArray)) {
        return ast::TextureDimension::kCubeArray;
    }

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

// multisampled_texture_type
//  : TEXTURE_MULTISAMPLED_2D
Maybe<const ast::TextureDimension> ParserImpl::multisampled_texture_type() {
    if (match(Token::Type::kTextureMultisampled2d)) {
        return ast::TextureDimension::k2d;
    }

    return Failure::kNoMatch;
}

// storage_texture_type
//  : TEXTURE_STORAGE_1D
//  | TEXTURE_STORAGE_2D
//  | TEXTURE_STORAGE_2D_ARRAY
//  | TEXTURE_STORAGE_3D
Maybe<const ast::TextureDimension> ParserImpl::storage_texture_type() {
    if (match(Token::Type::kTextureStorage1d)) {
        return ast::TextureDimension::k1d;
    }
    if (match(Token::Type::kTextureStorage2d)) {
        return ast::TextureDimension::k2d;
    }
    if (match(Token::Type::kTextureStorage2dArray)) {
        return ast::TextureDimension::k2dArray;
    }
    if (match(Token::Type::kTextureStorage3d)) {
        return ast::TextureDimension::k3d;
    }

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
    return expect_enum("texel format", ast::ParseTexelFormat, ast::kTexelFormatStrings, use);
}

Expect<ParserImpl::TypedIdentifier> ParserImpl::expect_ident_with_optional_type_specifier(
    std::string_view use,
    bool allow_inferred) {
    auto ident = expect_ident(use);
    if (ident.errored) {
        return Failure::kErrored;
    }

    if (allow_inferred && !peek_is(Token::Type::kColon)) {
        return TypedIdentifier{nullptr, ident.value, ident.source};
    }

    if (!expect(use, Token::Type::kColon)) {
        return Failure::kErrored;
    }

    auto& t = peek();
    auto type = type_specifier();
    if (type.errored) {
        return Failure::kErrored;
    }
    if (!type.matched) {
        return add_error(t.source(), "invalid type", use);
    }

    return TypedIdentifier{type.value, ident.value, ident.source};
}

// optionally_typed_ident
//   : ident ( COLON typed_decl ) ?
Expect<ParserImpl::TypedIdentifier> ParserImpl::expect_optionally_typed_ident(
    std::string_view use) {
    return expect_ident_with_optional_type_specifier(use, /* allow_inferred */ true);
}

// ident_with_type_specifier
//   : IDENT COLON type_specifier
Expect<ParserImpl::TypedIdentifier> ParserImpl::expect_ident_with_type_specifier(
    std::string_view use) {
    return expect_ident_with_optional_type_specifier(use, /* allow_inferred */ false);
}

// access_mode
//   : 'read'
//   | 'write'
//   | 'read_write'
Expect<ast::Access> ParserImpl::expect_access_mode(std::string_view use) {
    return expect_enum("access control", ast::ParseAccess, ast::kAccessStrings, use);
}

// variable_qualifier
//   : LESS_THAN address_spaces (COMMA access_mode)? GREATER_THAN
Maybe<ParserImpl::VariableQualifier> ParserImpl::variable_qualifier() {
    if (!peek_is(Token::Type::kLessThan)) {
        return Failure::kNoMatch;
    }

    auto* use = "variable declaration";
    auto vq = expect_lt_gt_block(use, [&]() -> Expect<VariableQualifier> {
        auto source = make_source_range();
        auto sc = expect_address_space(use);
        if (sc.errored) {
            return Failure::kErrored;
        }
        if (match(Token::Type::kComma)) {
            auto ac = expect_access_mode(use);
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

// type_alias_decl
//   : TYPE IDENT EQUAL type_specifier
Maybe<const ast::Alias*> ParserImpl::type_alias_decl() {
    if (!peek_is(Token::Type::kType)) {
        return Failure::kNoMatch;
    }

    auto& t = next();
    const char* use = "type alias";

    auto name = expect_ident(use);
    if (name.errored) {
        return Failure::kErrored;
    }

    if (!expect(use, Token::Type::kEqual)) {
        return Failure::kErrored;
    }

    auto type = type_specifier();
    if (type.errored) {
        return Failure::kErrored;
    }
    if (!type.matched) {
        return add_error(peek(), "invalid type alias");
    }

    return builder_.ty.alias(make_source_range_from(t.source()), name.value, type.value);
}

// vec_prefix
//   : 'vec2'
//   | 'vec3'
//   | 'vec4'
Maybe<uint32_t> ParserImpl::vec_prefix() {
    auto& t = peek();
    if (!t.IsVector()) {
        return Failure::kNoMatch;
    }
    next();

    if (t.Is(Token::Type::kVec3)) {
        return 3u;
    }
    if (t.Is(Token::Type::kVec4)) {
        return 4u;
    }
    return 2u;
}

// mat_prefix
//   : 'mat2x2'
//   | 'mat2x3'
//   | 'mat2x4'
//   | 'mat3x2'
//   | 'mat3x3'
//   | 'mat3x4'
//   | 'mat4x2'
//   | 'mat4x3'
//   | 'mat4x4'
Maybe<ParserImpl::MatrixDimensions> ParserImpl::mat_prefix() {
    auto& t = peek();
    if (!t.IsMatrix()) {
        return Failure::kNoMatch;
    }
    next();

    uint32_t columns = 2;
    if (t.IsMat3xN()) {
        columns = 3;
    } else if (t.IsMat4xN()) {
        columns = 4;
    }
    if (t.IsMatNx3()) {
        return MatrixDimensions{columns, 3};
    }
    if (t.IsMatNx4()) {
        return MatrixDimensions{columns, 4};
    }
    return MatrixDimensions{columns, 2};
}

// type_specifier_without_ident:
//   : BOOL
//   | F16
//   | F32
//   | I32
//   | U32
//   | ARRAY LESS_THAN type_specifier ( COMMA element_count_expression )? GREATER_THAN
//   | ATOMIC LESS_THAN type_specifier GREATER_THAN
//   | PTR LESS_THAN address_space COMMA type_specifier ( COMMA access_mode )? GREATER_THAN
//   | mat_prefix LESS_THAN type_specifier GREATER_THAN
//   | vec_prefix LESS_THAN type_specifier GREATER_THAN
//   | texture_and_sampler_types
Maybe<const ast::Type*> ParserImpl::type_specifier_without_ident() {
    auto& t = peek();

    if (match(Token::Type::kBool)) {
        return builder_.ty.bool_(t.source());
    }

    if (match(Token::Type::kF16)) {
        return builder_.ty.f16(t.source());
    }

    if (match(Token::Type::kF32)) {
        return builder_.ty.f32(t.source());
    }

    if (match(Token::Type::kI32)) {
        return builder_.ty.i32(t.source());
    }

    if (match(Token::Type::kU32)) {
        return builder_.ty.u32(t.source());
    }

    if (t.Is(Token::Type::kArray) && peek_is(Token::Type::kLessThan, 1)) {
        if (match(Token::Type::kArray)) {
            return expect_type_specifier_array(t.source());
        }
    }

    if (match(Token::Type::kAtomic)) {
        return expect_type_specifier_atomic(t.source());
    }

    if (match(Token::Type::kPtr)) {
        return expect_type_specifier_pointer(t.source());
    }

    if (t.IsMatrix() && peek_is(Token::Type::kLessThan, 1)) {
        auto mat = mat_prefix();
        if (mat.matched) {
            return expect_type_specifier_matrix(t.source(), mat.value);
        }
    }

    if (t.IsVector() && peek_is(Token::Type::kLessThan, 1)) {
        auto vec = vec_prefix();
        if (vec.matched) {
            return expect_type_specifier_vector(t.source(), vec.value);
        }
    }

    auto texture_or_sampler = texture_and_sampler_types();
    if (texture_or_sampler.errored) {
        return Failure::kErrored;
    }
    if (texture_or_sampler.matched) {
        return texture_or_sampler;
    }

    return Failure::kNoMatch;
}

// type_specifier
//   : IDENTIFIER
//   | type_specifier_without_ident
Maybe<const ast::Type*> ParserImpl::type_specifier() {
    auto& t = peek();
    Source source;
    if (match(Token::Type::kIdentifier, &source)) {
        return builder_.create<ast::TypeName>(source, builder_.Symbols().Register(t.to_str()));
    }

    return type_specifier_without_ident();
}

template <typename ENUM, size_t N>
Expect<ENUM> ParserImpl::expect_enum(std::string_view name,
                                     ENUM (*parse)(std::string_view str),
                                     const char* const (&strings)[N],
                                     std::string_view use) {
    auto& t = peek();
    if (t.IsIdentifier()) {
        auto val = parse(t.to_str());
        if (val != ENUM::kUndefined) {
            synchronized_ = true;
            next();
            return {val, t.source()};
        }
    }

    // Was the token itself an error?
    if (handle_error(t)) {
        return Failure::kErrored;
    }

    /// Create a sensible error message
    std::stringstream err;
    err << "expected " << name;

    if (!use.empty()) {
        err << " for " << use;
    }

    // If the string typed was within kSuggestionDistance of one of the possible enum values,
    // suggest that. Don't bother with suggestions if the string was extremely long.
    constexpr size_t kSuggestionDistance = 5;
    constexpr size_t kSuggestionMaxLength = 64;
    if (auto got = t.to_str(); !got.empty() && got.size() < kSuggestionMaxLength) {
        size_t candidate_dist = kSuggestionDistance;
        const char* candidate = nullptr;
        for (auto* str : strings) {
            auto dist = utils::Distance(str, got);
            if (dist < candidate_dist) {
                candidate = str;
                candidate_dist = dist;
            }
        }
        if (candidate) {
            err << ". Did you mean '" << candidate << "'?";
        }
    }

    // List all the possible enumerator values
    err << "\nPossible values: ";
    for (auto* str : strings) {
        if (str != strings[0]) {
            err << ", ";
        }
        err << "'" << str << "'";
    }

    synchronized_ = false;
    return add_error(t.source(), err.str());
}

Expect<const ast::Type*> ParserImpl::expect_type(std::string_view use) {
    auto type = type_specifier();
    if (type.errored) {
        return Failure::kErrored;
    }
    if (!type.matched) {
        return add_error(peek().source(), "invalid type", use);
    }
    return type.value;
}

// LESS_THAN address_space COMMA type_specifier ( COMMA access_mode )? GREATER_THAN
Expect<const ast::Type*> ParserImpl::expect_type_specifier_pointer(const Source& s) {
    const char* use = "ptr declaration";

    auto address_space = ast::AddressSpace::kNone;
    auto access = ast::Access::kUndefined;

    auto subtype = expect_lt_gt_block(use, [&]() -> Expect<const ast::Type*> {
        auto sc = expect_address_space(use);
        if (sc.errored) {
            return Failure::kErrored;
        }
        address_space = sc.value;

        if (!expect(use, Token::Type::kComma)) {
            return Failure::kErrored;
        }

        auto type = expect_type(use);
        if (type.errored) {
            return Failure::kErrored;
        }

        if (match(Token::Type::kComma)) {
            auto ac = expect_access_mode(use);
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

    return builder_.ty.pointer(make_source_range_from(s), subtype.value, address_space, access);
}

// LESS_THAN type_specifier GREATER_THAN
Expect<const ast::Type*> ParserImpl::expect_type_specifier_atomic(const Source& s) {
    const char* use = "atomic declaration";

    auto subtype = expect_lt_gt_block(use, [&] { return expect_type(use); });
    if (subtype.errored) {
        return Failure::kErrored;
    }

    return builder_.ty.atomic(make_source_range_from(s), subtype.value);
}

// LESS_THAN type_specifier GREATER_THAN
Expect<const ast::Type*> ParserImpl::expect_type_specifier_vector(const Source& s, uint32_t count) {
    const char* use = "vector";
    auto ty = expect_lt_gt_block(use, [&] { return expect_type(use); });
    if (ty.errored) {
        return Failure::kErrored;
    }

    return builder_.ty.vec(make_source_range_from(s), ty.value, count);
}

// LESS_THAN type_specifier ( COMMA element_count_expression )? GREATER_THAN
Expect<const ast::Type*> ParserImpl::expect_type_specifier_array(const Source& s) {
    const char* use = "array declaration";

    struct TypeAndSize {
        const ast::Type* type = nullptr;
        const ast::Expression* size = nullptr;
    };

    if (!peek_is(Token::Type::kLessThan)) {
        return add_error(peek(), "expected < for array");
    }

    auto type_size = expect_lt_gt_block(use, [&]() -> Expect<TypeAndSize> {
        auto type = expect_type(use);
        if (type.errored) {
            return Failure::kErrored;
        }

        if (!match(Token::Type::kComma)) {
            return TypeAndSize{type.value, nullptr};
        }

        auto size = element_count_expression();
        if (size.errored) {
            return Failure::kErrored;
        }
        if (!size.matched) {
            return add_error(peek(), "expected array size expression");
        }

        return TypeAndSize{type.value, size.value};
    });

    if (type_size.errored) {
        return Failure::kErrored;
    }

    return builder_.ty.array(make_source_range_from(s), type_size->type, type_size->size);
}

// LESS_THAN type_specifier GREATER_THAN
Expect<const ast::Type*> ParserImpl::expect_type_specifier_matrix(const Source& s,
                                                                  const MatrixDimensions& dims) {
    const char* use = "matrix";
    auto ty = expect_lt_gt_block(use, [&] { return expect_type(use); });
    if (ty.errored) {
        return Failure::kErrored;
    }

    return builder_.ty.mat(make_source_range_from(s), ty.value, dims.columns, dims.rows);
}

// address_space
//   : 'function'
//   | 'private'
//   | 'workgroup'
//   | 'uniform'
//   | 'storage'
//
// Note, we also parse `push_constant` from the experimental extension
Expect<ast::AddressSpace> ParserImpl::expect_address_space(std::string_view use) {
    return expect_enum("address space", ast::ParseAddressSpace, ast::kAddressSpaceStrings, use);
}

// struct_decl
//   : STRUCT IDENT struct_body_decl
Maybe<const ast::Struct*> ParserImpl::struct_decl() {
    auto& t = peek();

    if (!match(Token::Type::kStruct)) {
        return Failure::kNoMatch;
    }

    auto name = expect_ident("struct declaration");
    if (name.errored) {
        return Failure::kErrored;
    }

    auto body = expect_struct_body_decl();
    if (body.errored) {
        return Failure::kErrored;
    }

    auto sym = builder_.Symbols().Register(name.value);
    return create<ast::Struct>(t.source(), sym, std::move(body.value), utils::Empty);
}

// struct_body_decl
//   : BRACE_LEFT (struct_member COMMA)* struct_member COMMA? BRACE_RIGHT
Expect<ParserImpl::StructMemberList> ParserImpl::expect_struct_body_decl() {
    return expect_brace_block("struct declaration", [&]() -> Expect<StructMemberList> {
        StructMemberList members;
        bool errored = false;
        while (continue_parsing()) {
            // Check for the end of the list.
            auto& t = peek();
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
                members.Push(member.value);
            }

            if (!match(Token::Type::kComma)) {
                break;
            }
        }
        if (errored) {
            return Failure::kErrored;
        }
        return members;
    });
}

// struct_member
//   : attribute* ident_with_type_specifier
Expect<ast::StructMember*> ParserImpl::expect_struct_member() {
    auto attrs = attribute_list();
    if (attrs.errored) {
        return Failure::kErrored;
    }

    auto decl = expect_ident_with_type_specifier("struct member");
    if (decl.errored) {
        return Failure::kErrored;
    }

    return create<ast::StructMember>(decl->source, builder_.Symbols().Register(decl->name),
                                     decl->type, std::move(attrs.value));
}

// static_assert_statement
//   : STATIC_ASSERT expression
Maybe<const ast::StaticAssert*> ParserImpl::static_assert_statement() {
    Source start;
    if (!match(Token::Type::kStaticAssert, &start)) {
        return Failure::kNoMatch;
    }

    auto condition = expression();
    if (condition.errored) {
        return Failure::kErrored;
    }
    if (!condition.matched) {
        return add_error(peek(), "unable to parse condition expression");
    }

    Source source = make_source_range_from(start);
    return create<ast::StaticAssert>(source, condition.value);
}

// function_decl
//   : function_header compound_statement
Maybe<const ast::Function*> ParserImpl::function_decl(AttributeList& attrs) {
    auto header = function_header();
    if (header.errored) {
        if (sync_to(Token::Type::kBraceLeft, /* consume: */ false)) {
            // There were errors in the function header, but the parser has managed to
            // resynchronize with the opening brace. As there's no outer
            // synchronization token for function declarations, attempt to parse the
            // function body. The AST isn't used as we've already errored, but this
            // catches any errors inside the body, and can help keep the parser in
            // sync.
            expect_compound_statement();
        }
        return Failure::kErrored;
    }
    if (!header.matched) {
        return Failure::kNoMatch;
    }

    bool errored = false;

    auto body = expect_compound_statement();
    if (body.errored) {
        errored = true;
    }

    if (errored) {
        return Failure::kErrored;
    }

    TINT_DEFER(attrs.Clear());

    return create<ast::Function>(header->source, builder_.Symbols().Register(header->name),
                                 header->params, header->return_type, body.value, std::move(attrs),
                                 header->return_type_attributes);
}

// function_header
//   : FN IDENT PAREN_LEFT param_list PAREN_RIGHT return_type_specifier_optional
// return_type_specifier_optional
//   :
//   | ARROW attribute_list* type_specifier
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
    AttributeList return_attributes;

    if (match(Token::Type::kArrow)) {
        auto attrs = attribute_list();
        if (attrs.errored) {
            return Failure::kErrored;
        }
        return_attributes = attrs.value;

        auto type = type_specifier();
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

    return FunctionHeader{
        source,      std::move(name.value),        std::move(params.value),
        return_type, std::move(return_attributes),
    };
}

// param_list
//   :
//   | (param COMMA)* param COMMA?
Expect<ParserImpl::ParameterList> ParserImpl::expect_param_list() {
    ParameterList ret;
    while (continue_parsing()) {
        // Check for the end of the list.
        auto& t = peek();
        if (!t.IsIdentifier() && !t.Is(Token::Type::kAttr)) {
            break;
        }

        auto param = expect_param();
        if (param.errored) {
            return Failure::kErrored;
        }
        ret.Push(param.value);

        if (!match(Token::Type::kComma)) {
            break;
        }
    }

    return ret;
}

// param
//   : attribute_list* ident COLON type_specifier
Expect<ast::Parameter*> ParserImpl::expect_param() {
    auto attrs = attribute_list();

    auto decl = expect_ident_with_type_specifier("parameter");
    if (decl.errored) {
        return Failure::kErrored;
    }

    return create<ast::Parameter>(decl->source,                             // source
                                  builder_.Symbols().Register(decl->name),  // symbol
                                  decl->type,                               // type
                                  std::move(attrs.value));                  // attributes
}

// interpolation_sample_name
//   : 'center'
//   | 'centroid'
//   | 'sample'
Expect<ast::InterpolationSampling> ParserImpl::expect_interpolation_sample_name() {
    return expect_enum("interpolation sampling", ast::ParseInterpolationSampling,
                       ast::kInterpolationSamplingStrings);
}

// interpolation_type_name
//   : 'perspective'
//   | 'linear'
//   | 'flat'
Expect<ast::InterpolationType> ParserImpl::expect_interpolation_type_name() {
    return expect_enum("interpolation type", ast::ParseInterpolationType,
                       ast::kInterpolationTypeStrings);
}

// builtin_value_name
//   : frag_depth
//   | front_facing
//   | global_invocation_id
//   | instance_index
//   | local_invocation_id
//   | local_invocation_index
//   | num_workgroups
//   | position
//   | sample_index
//   | sample_mask
//   | vertex_index
//   | workgroup_id
Expect<ast::BuiltinValue> ParserImpl::expect_builtin() {
    return expect_enum("builtin", ast::ParseBuiltinValue, ast::kBuiltinValueStrings);
}

// compound_statement
//   : BRACE_LEFT statement* BRACE_RIGHT
Expect<ast::BlockStatement*> ParserImpl::expect_compound_statement() {
    return expect_brace_block("", [&]() -> Expect<ast::BlockStatement*> {
        auto stmts = expect_statements();
        if (stmts.errored) {
            return Failure::kErrored;
        }
        return create<ast::BlockStatement>(Source{}, stmts.value);
    });
}

// paren_expression
//   : PAREN_LEFT expression PAREN_RIGHT
Expect<const ast::Expression*> ParserImpl::expect_paren_expression() {
    return expect_paren_block("", [&]() -> Expect<const ast::Expression*> {
        auto expr = expression();
        if (expr.errored) {
            return Failure::kErrored;
        }
        if (!expr.matched) {
            return add_error(peek(), "unable to parse expression");
        }

        return expr.value;
    });
}

// statements
//   : statement*
Expect<ParserImpl::StatementList> ParserImpl::expect_statements() {
    bool errored = false;
    StatementList stmts;

    while (continue_parsing()) {
        auto stmt = statement();
        if (stmt.errored) {
            errored = true;
        } else if (stmt.matched) {
            stmts.Push(stmt.value);
        } else {
            break;
        }
    }

    if (errored) {
        return Failure::kErrored;
    }

    return stmts;
}

// statement
//   : SEMICOLON
//   | if_statement
//   | switch_statement
//   | loop_statement
//   | for_statement
//   | while_statement
//   | compound_statement
//   | non_block_statement   // Note, we inject an extra rule in here for simpler parsing
Maybe<const ast::Statement*> ParserImpl::statement() {
    while (match(Token::Type::kSemicolon)) {
        // Skip empty statements
    }

    // Non-block statements that error can resynchronize on semicolon.
    auto stmt = sync(Token::Type::kSemicolon, [&] { return non_block_statement(); });
    if (stmt.errored) {
        return Failure::kErrored;
    }
    if (stmt.matched) {
        return stmt;
    }

    auto stmt_if = if_statement();
    if (stmt_if.errored) {
        return Failure::kErrored;
    }
    if (stmt_if.matched) {
        return stmt_if.value;
    }

    auto sw = switch_statement();
    if (sw.errored) {
        return Failure::kErrored;
    }
    if (sw.matched) {
        return sw.value;
    }

    auto loop = loop_statement();
    if (loop.errored) {
        return Failure::kErrored;
    }
    if (loop.matched) {
        return loop.value;
    }

    auto stmt_for = for_statement();
    if (stmt_for.errored) {
        return Failure::kErrored;
    }
    if (stmt_for.matched) {
        return stmt_for.value;
    }

    auto stmt_while = while_statement();
    if (stmt_while.errored) {
        return Failure::kErrored;
    }
    if (stmt_while.matched) {
        return stmt_while.value;
    }

    if (peek_is(Token::Type::kBraceLeft)) {
        auto body = expect_compound_statement();
        if (body.errored) {
            return Failure::kErrored;
        }
        return body.value;
    }

    return Failure::kNoMatch;
}

// non_block_statement (continued)
//   : return_statement SEMICOLON
//   | func_call_statement SEMICOLON
//   | variable_statement SEMICOLON
//   | break_statement SEMICOLON
//   | continue_statement SEMICOLON
//   | DISCARD SEMICOLON
//   | variable_updating_statement SEMICOLON
//   | static_assert_statement SEMICOLON
Maybe<const ast::Statement*> ParserImpl::non_block_statement() {
    auto stmt = [&]() -> Maybe<const ast::Statement*> {
        auto ret_stmt = return_statement();
        if (ret_stmt.errored) {
            return Failure::kErrored;
        }
        if (ret_stmt.matched) {
            return ret_stmt.value;
        }

        auto func = func_call_statement();
        if (func.errored) {
            return Failure::kErrored;
        }
        if (func.matched) {
            return func.value;
        }

        auto var = variable_statement();
        if (var.errored) {
            return Failure::kErrored;
        }
        if (var.matched) {
            return var.value;
        }

        auto b = break_statement();
        if (b.errored) {
            return Failure::kErrored;
        }
        if (b.matched) {
            return b.value;
        }

        auto cont = continue_statement();
        if (cont.errored) {
            return Failure::kErrored;
        }
        if (cont.matched) {
            return cont.value;
        }

        Source source;
        if (match(Token::Type::kDiscard, &source)) {
            return create<ast::DiscardStatement>(source);
        }

        // Note, this covers assignment, increment and decrement
        auto assign = variable_updating_statement();
        if (assign.errored) {
            return Failure::kErrored;
        }
        if (assign.matched) {
            return assign.value;
        }

        auto stmt_static_assert = static_assert_statement();
        if (stmt_static_assert.errored) {
            return Failure::kErrored;
        }
        if (stmt_static_assert.matched) {
            return stmt_static_assert.value;
        }

        return Failure::kNoMatch;
    }();

    if (stmt.matched && !expect(stmt->Name(), Token::Type::kSemicolon)) {
        return Failure::kErrored;
    }
    return stmt;
}

// return_statement
//   : RETURN expression?
Maybe<const ast::ReturnStatement*> ParserImpl::return_statement() {
    Source source;
    if (!match(Token::Type::kReturn, &source)) {
        return Failure::kNoMatch;
    }

    if (peek_is(Token::Type::kSemicolon)) {
        return create<ast::ReturnStatement>(source, nullptr);
    }

    auto expr = expression();
    if (expr.errored) {
        return Failure::kErrored;
    }

    // TODO(bclayton): Check matched?
    return create<ast::ReturnStatement>(source, expr.value);
}

// variable_statement
//   : variable_decl
//   | variable_decl EQUAL expression
//   | LET optionally_typed_ident EQUAL expression
//   | CONST optionally_typed_ident EQUAL expression
Maybe<const ast::VariableDeclStatement*> ParserImpl::variable_statement() {
    auto decl_source_range = make_source_range();
    if (match(Token::Type::kConst)) {
        auto typed_ident = expect_optionally_typed_ident("'const' declaration");
        if (typed_ident.errored) {
            return Failure::kErrored;
        }

        auto decl_source = decl_source_range.Source();

        if (!expect("'const' declaration", Token::Type::kEqual)) {
            return Failure::kErrored;
        }

        auto initializer = expression();
        if (initializer.errored) {
            return Failure::kErrored;
        }
        if (!initializer.matched) {
            return add_error(peek(), "missing initializer for 'const' declaration");
        }

        auto* const_ = create<ast::Const>(typed_ident->source,                             // source
                                          builder_.Symbols().Register(typed_ident->name),  // symbol
                                          typed_ident->type,                               // type
                                          initializer.value,  // initializer
                                          utils::Empty);      // attributes

        return create<ast::VariableDeclStatement>(decl_source, const_);
    }

    if (match(Token::Type::kLet)) {
        auto typed_ident = expect_optionally_typed_ident("'let' declaration");
        if (typed_ident.errored) {
            return Failure::kErrored;
        }

        auto decl_source = decl_source_range.Source();

        if (!expect("'let' declaration", Token::Type::kEqual)) {
            return Failure::kErrored;
        }

        auto initializer = expression();
        if (initializer.errored) {
            return Failure::kErrored;
        }
        if (!initializer.matched) {
            return add_error(peek(), "missing initializer for 'let' declaration");
        }

        auto* let = create<ast::Let>(typed_ident->source,                             // source
                                     builder_.Symbols().Register(typed_ident->name),  // symbol
                                     typed_ident->type,                               // type
                                     initializer.value,                               // initializer
                                     utils::Empty);                                   // attributes

        return create<ast::VariableDeclStatement>(decl_source, let);
    }

    auto decl = variable_decl();
    if (decl.errored) {
        return Failure::kErrored;
    }
    if (!decl.matched) {
        return Failure::kNoMatch;
    }

    auto decl_source = decl_source_range.Source();

    const ast::Expression* initializer = nullptr;
    if (match(Token::Type::kEqual)) {
        auto initializer_expr = expression();
        if (initializer_expr.errored) {
            return Failure::kErrored;
        }
        if (!initializer_expr.matched) {
            return add_error(peek(), "missing initializer for 'var' declaration");
        }

        initializer = initializer_expr.value;
    }

    auto* var = create<ast::Var>(decl_source,                              // source
                                 builder_.Symbols().Register(decl->name),  // symbol
                                 decl->type,                               // type
                                 decl->address_space,                      // address space
                                 decl->access,                             // access control
                                 initializer,                              // initializer
                                 utils::Empty);                            // attributes

    return create<ast::VariableDeclStatement>(var->source, var);
}

// if_statement
//   : IF expression compound_stmt ( ELSE else_stmt ) ?
// else_stmt
//  : compound_statement
//  | if_statement
Maybe<const ast::IfStatement*> ParserImpl::if_statement() {
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

        auto condition = expression();
        if (condition.errored) {
            return Failure::kErrored;
        }
        if (!condition.matched) {
            return add_error(peek(), "unable to parse condition expression");
        }

        auto body = expect_compound_statement();
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
        auto else_body = expect_compound_statement();
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

// switch_statement
//   : SWITCH expression BRACKET_LEFT switch_body+ BRACKET_RIGHT
Maybe<const ast::SwitchStatement*> ParserImpl::switch_statement() {
    Source source;
    if (!match(Token::Type::kSwitch, &source)) {
        return Failure::kNoMatch;
    }

    auto condition = expression();
    if (condition.errored) {
        return Failure::kErrored;
    }
    if (!condition.matched) {
        return add_error(peek(), "unable to parse selector expression");
    }

    auto body = expect_brace_block("switch statement", [&]() -> Expect<CaseStatementList> {
        bool errored = false;
        CaseStatementList list;
        while (continue_parsing()) {
            auto stmt = switch_body();
            if (stmt.errored) {
                errored = true;
                continue;
            }
            if (!stmt.matched) {
                break;
            }
            list.Push(stmt.value);
        }
        if (errored) {
            return Failure::kErrored;
        }
        return list;
    });

    if (body.errored) {
        return Failure::kErrored;
    }

    return create<ast::SwitchStatement>(source, condition.value, body.value);
}

// switch_body
//   : CASE case_selectors COLON? BRACKET_LEFT case_body BRACKET_RIGHT
//   | DEFAULT COLON? BRACKET_LEFT case_body BRACKET_RIGHT
Maybe<const ast::CaseStatement*> ParserImpl::switch_body() {
    if (!peek_is(Token::Type::kCase) && !peek_is(Token::Type::kDefault)) {
        return Failure::kNoMatch;
    }

    auto& t = next();

    CaseSelectorList selector_list;
    if (t.Is(Token::Type::kCase)) {
        auto selectors = expect_case_selectors();
        if (selectors.errored) {
            return Failure::kErrored;
        }

        selector_list = std::move(selectors.value);
    } else {
        // Push the default case selector
        selector_list.Push(create<ast::CaseSelector>(t.source()));
    }

    // Consume the optional colon if present.
    match(Token::Type::kColon);

    const char* use = "case statement";
    auto body = expect_brace_block(use, [&] { return case_body(); });

    if (body.errored) {
        return Failure::kErrored;
    }
    if (!body.matched) {
        return add_error(body.source, "expected case body");
    }

    return create<ast::CaseStatement>(t.source(), selector_list, body.value);
}

// case_selectors
//   : case_selector (COMMA case_selector)* COMMA?
Expect<ParserImpl::CaseSelectorList> ParserImpl::expect_case_selectors() {
    CaseSelectorList selectors;

    while (continue_parsing()) {
        auto expr = case_selector();
        if (expr.errored) {
            return Failure::kErrored;
        }
        if (!expr.matched) {
            break;
        }
        selectors.Push(expr.value);

        if (!match(Token::Type::kComma)) {
            break;
        }
    }

    if (selectors.IsEmpty()) {
        return add_error(peek(), "expected case selector expression or `default`");
    }

    return selectors;
}

// case_selector
//   : DEFAULT
//   | expression
Maybe<const ast::CaseSelector*> ParserImpl::case_selector() {
    auto& p = peek();

    if (match(Token::Type::kDefault)) {
        return create<ast::CaseSelector>(p.source());
    }

    auto expr = expression();
    if (expr.errored) {
        return Failure::kErrored;
    }
    if (!expr.matched) {
        return Failure::kNoMatch;
    }
    return create<ast::CaseSelector>(p.source(), expr.value);
}

// case_body
//   :
//   | statement case_body
Maybe<const ast::BlockStatement*> ParserImpl::case_body() {
    StatementList stmts;
    while (continue_parsing()) {
        Source source;
        if (match(Token::Type::kFallthrough, &source)) {
            return add_error(
                source,
                "fallthrough is not premitted in WGSL. "
                "Case can accept multiple selectors if the existing case bodies are empty. "
                "(e.g. `case 1, 2, 3:`) "
                "`default` is a valid case selector value. (e.g. `case 1, default:`)");
        }

        auto stmt = statement();
        if (stmt.errored) {
            return Failure::kErrored;
        }
        if (!stmt.matched) {
            break;
        }

        stmts.Push(stmt.value);
    }

    return create<ast::BlockStatement>(Source{}, stmts);
}

// loop_statement
//   : LOOP BRACKET_LEFT statements continuing_statement? BRACKET_RIGHT
Maybe<const ast::LoopStatement*> ParserImpl::loop_statement() {
    Source source;
    if (!match(Token::Type::kLoop, &source)) {
        return Failure::kNoMatch;
    }

    return expect_brace_block("loop", [&]() -> Maybe<const ast::LoopStatement*> {
        auto stmts = expect_statements();
        if (stmts.errored) {
            return Failure::kErrored;
        }

        auto continuing = continuing_statement();
        if (continuing.errored) {
            return Failure::kErrored;
        }

        auto* body = create<ast::BlockStatement>(source, stmts.value);
        return create<ast::LoopStatement>(source, body, continuing.value);
    });
}

ForHeader::ForHeader(const ast::Statement* init,
                     const ast::Expression* cond,
                     const ast::Statement* cont)
    : initializer(init), condition(cond), continuing(cont) {}

ForHeader::~ForHeader() = default;

// (variable_statement | variable_updating_statement | func_call_statement)?
Maybe<const ast::Statement*> ParserImpl::for_header_initializer() {
    auto call = func_call_statement();
    if (call.errored) {
        return Failure::kErrored;
    }
    if (call.matched) {
        return call.value;
    }

    auto var = variable_statement();
    if (var.errored) {
        return Failure::kErrored;
    }
    if (var.matched) {
        return var.value;
    }

    auto assign = variable_updating_statement();
    if (assign.errored) {
        return Failure::kErrored;
    }
    if (assign.matched) {
        return assign.value;
    }

    return Failure::kNoMatch;
}

// (variable_updating_statement | func_call_statement)?
Maybe<const ast::Statement*> ParserImpl::for_header_continuing() {
    auto call_stmt = func_call_statement();
    if (call_stmt.errored) {
        return Failure::kErrored;
    }
    if (call_stmt.matched) {
        return call_stmt.value;
    }

    auto assign = variable_updating_statement();
    if (assign.errored) {
        return Failure::kErrored;
    }
    if (assign.matched) {
        return assign.value;
    }

    return Failure::kNoMatch;
}

// for_header
//   : for_header_initializer? SEMICOLON expression? SEMICOLON for_header_continuing?
Expect<std::unique_ptr<ForHeader>> ParserImpl::expect_for_header() {
    auto initializer = for_header_initializer();
    if (initializer.errored) {
        return Failure::kErrored;
    }

    if (!expect("initializer in for loop", Token::Type::kSemicolon)) {
        return Failure::kErrored;
    }

    auto condition = expression();
    if (condition.errored) {
        return Failure::kErrored;
    }

    if (!expect("condition in for loop", Token::Type::kSemicolon)) {
        return Failure::kErrored;
    }

    auto continuing = for_header_continuing();
    if (continuing.errored) {
        return Failure::kErrored;
    }

    return std::make_unique<ForHeader>(initializer.value, condition.value, continuing.value);
}

// for_statement
//   : FOR PAREN_LEFT for_header PAREN_RIGHT BRACE_LEFT statements BRACE_RIGHT
Maybe<const ast::ForLoopStatement*> ParserImpl::for_statement() {
    Source source;
    if (!match(Token::Type::kFor, &source)) {
        return Failure::kNoMatch;
    }

    auto header = expect_paren_block("for loop", [&] { return expect_for_header(); });
    if (header.errored) {
        return Failure::kErrored;
    }

    auto stmts = expect_brace_block("for loop", [&] { return expect_statements(); });
    if (stmts.errored) {
        return Failure::kErrored;
    }

    return create<ast::ForLoopStatement>(source, header->initializer, header->condition,
                                         header->continuing,
                                         create<ast::BlockStatement>(stmts.value));
}

// while_statement
//   :  WHILE expression compound_statement
Maybe<const ast::WhileStatement*> ParserImpl::while_statement() {
    Source source;
    if (!match(Token::Type::kWhile, &source)) {
        return Failure::kNoMatch;
    }

    auto condition = expression();
    if (condition.errored) {
        return Failure::kErrored;
    }
    if (!condition.matched) {
        return add_error(peek(), "unable to parse while condition expression");
    }

    auto body = expect_compound_statement();
    if (body.errored) {
        return Failure::kErrored;
    }

    return create<ast::WhileStatement>(source, condition.value, body.value);
}

// func_call_statement
//    : IDENT argument_expression_list
Maybe<const ast::CallStatement*> ParserImpl::func_call_statement() {
    auto& t = peek();
    auto& t2 = peek(1);
    if (!t.IsIdentifier() || !t2.Is(Token::Type::kParenLeft)) {
        return Failure::kNoMatch;
    }

    next();  // Consume the first peek

    auto params = expect_argument_expression_list("function call");
    if (params.errored) {
        return Failure::kErrored;
    }

    return create<ast::CallStatement>(
        t.source(),
        create<ast::CallExpression>(
            t.source(),
            create<ast::IdentifierExpression>(t.source(), builder_.Symbols().Register(t.to_str())),
            std::move(params.value)));
}

// break_statement
//   : BREAK
Maybe<const ast::BreakStatement*> ParserImpl::break_statement() {
    Source source;
    if (!match(Token::Type::kBreak, &source)) {
        return Failure::kNoMatch;
    }

    return create<ast::BreakStatement>(source);
}

// continue_statement
//   : CONTINUE
Maybe<const ast::ContinueStatement*> ParserImpl::continue_statement() {
    Source source;
    if (!match(Token::Type::kContinue, &source)) {
        return Failure::kNoMatch;
    }

    return create<ast::ContinueStatement>(source);
}

// break_if_statement:
//    'break' 'if' expression semicolon
Maybe<const ast::Statement*> ParserImpl::break_if_statement() {
    auto& t1 = peek();
    auto& t2 = peek(1);

    // Match both the `break` and `if` at the same time.
    if (!t1.Is(Token::Type::kBreak) || !t2.Is(Token::Type::kIf)) {
        return Failure::kNoMatch;
    }
    next();  // Consume the peek
    next();  // Consume the peek

    auto expr = expression();
    if (expr.errored) {
        return Failure::kErrored;
    }
    if (!expr.matched) {
        return add_error(t1, "expected expression for `break-if`");
    }
    if (!expect("`break-if` statement", Token::Type::kSemicolon)) {
        return Failure::kErrored;
    }

    return create<ast::BreakIfStatement>(t1.source(), expr.value);
}

// continuing_compound_statement:
//   brace_left statement* break_if_statement? brace_right
Maybe<const ast::BlockStatement*> ParserImpl::continuing_compound_statement() {
    return expect_brace_block("", [&]() -> Expect<ast::BlockStatement*> {
        StatementList stmts;

        while (continue_parsing()) {
            // Note, break-if has to parse before statements because statements includes `break`
            auto break_if = break_if_statement();
            if (break_if.errored) {
                return Failure::kErrored;
            }
            if (break_if.matched) {
                stmts.Push(break_if.value);
                continue;
            }

            auto stmt = statement();
            if (stmt.errored) {
                return Failure::kErrored;
            }
            if (!stmt.matched) {
                break;
            }
            stmts.Push(stmt.value);
        }

        return create<ast::BlockStatement>(Source{}, stmts);
    });
}

// continuing_statement
//   : CONTINUING continuing_compound_statement
Maybe<const ast::BlockStatement*> ParserImpl::continuing_statement() {
    if (!match(Token::Type::kContinuing)) {
        return create<ast::BlockStatement>(Source{}, utils::Empty);
    }

    return continuing_compound_statement();
}

// callable
//   : type_specifier_without_ident
//   | ARRAY
//   | mat_prefix
//   | vec_prefix
//
//  Note, `ident` is pulled out to `primary_expression` as it's the only one that
//  doesn't create a `type`. Then we can just return a `type` from here on match and
//  deal with `ident` in `primary_expression.
Maybe<const ast::Type*> ParserImpl::callable() {
    auto& t = peek();

    //  This _must_ match `type_specifier_without_ident` before any of the other types as they're
    //  all prefixes of the types and we want to match the longer `vec3<f32>` then the shorter
    //  prefix match of `vec3`.
    auto ty = type_specifier_without_ident();
    if (ty.errored) {
        return Failure::kErrored;
    }
    if (ty.matched) {
        return ty.value;
    }

    if (match(Token::Type::kArray)) {
        return builder_.ty.array(make_source_range_from(t.source()), nullptr, nullptr);
    }

    auto vec = vec_prefix();
    if (vec.matched) {
        return builder_.ty.vec(make_source_range_from(t.source()), nullptr, vec.value);
    }

    auto mat = mat_prefix();
    if (mat.matched) {
        return builder_.ty.mat(make_source_range_from(t.source()), nullptr, mat.value.columns,
                               mat.value.rows);
    }

    return Failure::kNoMatch;
}

// primary_expression
//   : BITCAST LESS_THAN type_specifier GREATER_THAN paren_expression
//   | callable argument_expression_list
//   | const_literal
//   | IDENT argument_expression_list?
//   | paren_expression
//
// Note, PAREN_LEFT ( expression ( COMMA expression ) * COMMA? )? PAREN_RIGHT is replaced
// with `argument_expression_list`.
//
// Note, this is matching the `callable` ident here instead of having to come from
// callable so we can return a `type` from callable.
Maybe<const ast::Expression*> ParserImpl::primary_expression() {
    auto& t = peek();

    if (match(Token::Type::kBitcast)) {
        const char* use = "bitcast expression";

        auto type = expect_lt_gt_block(use, [&] { return expect_type(use); });
        if (type.errored) {
            return Failure::kErrored;
        }

        auto params = expect_paren_expression();
        if (params.errored) {
            return Failure::kErrored;
        }

        return create<ast::BitcastExpression>(t.source(), type.value, params.value);
    }

    auto call = callable();
    if (call.errored) {
        return Failure::kErrored;
    }
    if (call.matched) {
        auto params = expect_argument_expression_list("type initializer");
        if (params.errored) {
            return Failure::kErrored;
        }

        return builder_.Construct(t.source(), call.value, std::move(params.value));
    }

    auto lit = const_literal();
    if (lit.errored) {
        return Failure::kErrored;
    }
    if (lit.matched) {
        return lit.value;
    }

    if (t.IsIdentifier()) {
        next();

        auto* ident =
            create<ast::IdentifierExpression>(t.source(), builder_.Symbols().Register(t.to_str()));

        if (peek_is(Token::Type::kParenLeft)) {
            auto params = expect_argument_expression_list("function call");
            if (params.errored) {
                return Failure::kErrored;
            }

            return create<ast::CallExpression>(t.source(), ident, std::move(params.value));
        }

        return ident;
    }

    if (t.Is(Token::Type::kParenLeft)) {
        auto paren = expect_paren_expression();
        if (paren.errored) {
            return Failure::kErrored;
        }

        return paren.value;
    }

    return Failure::kNoMatch;
}

// component_or_swizzle_specifier
//   :
//   | BRACE_LEFT expression BRACE_RIGHT component_or_swizzle_specifier?
//   | PERIOD member_ident component_or_swizzle_specifier?
//   | PERIOD swizzle_name component_or_swizzle_specifier?
Maybe<const ast::Expression*> ParserImpl::component_or_swizzle_specifier(
    const ast::Expression* prefix) {
    Source source;

    while (continue_parsing()) {
        if (match(Token::Type::kBracketLeft, &source)) {
            auto res = sync(Token::Type::kBracketRight, [&]() -> Maybe<const ast::Expression*> {
                auto param = expression();
                if (param.errored) {
                    return Failure::kErrored;
                }
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

// argument_expression_list
//   : PAREN_LEFT ((expression COMMA)* expression COMMA?)? PAREN_RIGHT
Expect<ParserImpl::ExpressionList> ParserImpl::expect_argument_expression_list(
    std::string_view use) {
    return expect_paren_block(use, [&]() -> Expect<ExpressionList> {
        ExpressionList ret;
        while (continue_parsing()) {
            auto arg = expression();
            if (arg.errored) {
                return Failure::kErrored;
            } else if (!arg.matched) {
                break;
            }
            ret.Push(arg.value);

            if (!match(Token::Type::kComma)) {
                break;
            }
        }
        return ret;
    });
}

// bitwise_expression.post.unary_expression
//   : AND unary_expression (AND unary_expression)*
//   | OR unary_expression (OR unary_expression)*
//   | XOR unary_expression (XOR unary_expression)*
Maybe<const ast::Expression*> ParserImpl::bitwise_expression_post_unary_expression(
    const ast::Expression* lhs) {
    auto& t = peek();

    ast::BinaryOp op = ast::BinaryOp::kXor;
    switch (t.type()) {
        case Token::Type::kAnd:
            op = ast::BinaryOp::kAnd;
            break;
        case Token::Type::kOr:
            op = ast::BinaryOp::kOr;
            break;
        case Token::Type::kXor:
            op = ast::BinaryOp::kXor;
            break;
        default:
            return Failure::kNoMatch;
    }
    next();  // Consume t

    while (continue_parsing()) {
        auto rhs = unary_expression();
        if (rhs.errored) {
            return Failure::kErrored;
        }
        if (!rhs.matched) {
            return add_error(peek(), std::string("unable to parse right side of ") +
                                         std::string(t.to_name()) + " expression");
        }

        lhs = create<ast::BinaryExpression>(t.source(), op, lhs, rhs.value);

        if (!match(t.type())) {
            return lhs;
        }
    }
    return Failure::kErrored;
}

// multiplicative_operator
//   : FORWARD_SLASH
//   | MODULO
//   | STAR
Maybe<ast::BinaryOp> ParserImpl::multiplicative_operator() {
    if (match(Token::Type::kForwardSlash)) {
        return ast::BinaryOp::kDivide;
    }
    if (match(Token::Type::kMod)) {
        return ast::BinaryOp::kModulo;
    }
    if (match(Token::Type::kStar)) {
        return ast::BinaryOp::kMultiply;
    }

    return Failure::kNoMatch;
}

// multiplicative_expression.post.unary_expression
//   : (multiplicative_operator unary_expression)*
Expect<const ast::Expression*> ParserImpl::expect_multiplicative_expression_post_unary_expression(
    const ast::Expression* lhs) {
    while (continue_parsing()) {
        auto& t = peek();

        auto op = multiplicative_operator();
        if (op.errored) {
            return Failure::kErrored;
        }
        if (!op.matched) {
            return lhs;
        }

        auto rhs = unary_expression();
        if (rhs.errored) {
            return Failure::kErrored;
        }
        if (!rhs.matched) {
            return add_error(peek(), std::string("unable to parse right side of ") +
                                         std::string(t.to_name()) + " expression");
        }

        lhs = create<ast::BinaryExpression>(t.source(), op.value, lhs, rhs.value);
    }
    return Failure::kErrored;
}

// additive_operator
//   : MINUS
//   | PLUS
//
// Note, this also splits a `--` token. This is currently safe as the only way to get into
// here is through additive expression and rules for where `--` are allowed are very restrictive.
Maybe<ast::BinaryOp> ParserImpl::additive_operator() {
    if (match(Token::Type::kPlus)) {
        return ast::BinaryOp::kAdd;
    }

    auto& t = peek();
    if (t.Is(Token::Type::kMinusMinus)) {
        next();
        split_token(Token::Type::kMinus, Token::Type::kMinus);
    } else if (t.Is(Token::Type::kMinus)) {
        next();
    } else {
        return Failure::kNoMatch;
    }

    return ast::BinaryOp::kSubtract;
}

// additive_expression.pos.unary_expression
//   : (additive_operator unary_expression expect_multiplicative_expression.post.unary_expression)*
//
// This is `( additive_operator unary_expression ( multiplicative_operator unary_expression )* )*`
// split apart.
Expect<const ast::Expression*> ParserImpl::expect_additive_expression_post_unary_expression(
    const ast::Expression* lhs) {
    while (continue_parsing()) {
        auto& t = peek();

        auto op = additive_operator();
        if (op.errored) {
            return Failure::kErrored;
        }
        if (!op.matched) {
            return lhs;
        }

        auto unary = unary_expression();
        if (unary.errored) {
            return Failure::kErrored;
        }
        if (!unary.matched) {
            return add_error(peek(), std::string("unable to parse right side of ") +
                                         std::string(t.to_name()) + " expression");
        }

        // The multiplicative binds tigher, so pass the unary into that and build that expression
        // before creating the additve expression.
        auto rhs = expect_multiplicative_expression_post_unary_expression(unary.value);
        if (rhs.errored) {
            return Failure::kErrored;
        }

        lhs = create<ast::BinaryExpression>(t.source(), op.value, lhs, rhs.value);
    }
    return Failure::kErrored;
}

// math_expression.post.unary_expression
//   : multiplicative_expression.post.unary_expression additive_expression.post.unary_expression
//
// This is `( multiplicative_operator unary_expression )* ( additive_operator unary_expression (
// multiplicative_operator unary_expression )* )*` split apart.
Expect<const ast::Expression*> ParserImpl::expect_math_expression_post_unary_expression(
    const ast::Expression* lhs) {
    auto rhs = expect_multiplicative_expression_post_unary_expression(lhs);
    if (rhs.errored) {
        return Failure::kErrored;
    }

    return expect_additive_expression_post_unary_expression(rhs.value);
}

// element_count_expression
//   : unary_expression math_expression.post.unary_expression
//   | unary_expression bitwise_expression.post.unary_expression
//
// Note, this moves the `( multiplicative_operator unary_expression )* ( additive_operator
// unary_expression ( multiplicative_operator unary_expression )* )*` expression for the first
// branch out into helper methods.
Maybe<const ast::Expression*> ParserImpl::element_count_expression() {
    auto lhs = unary_expression();
    if (lhs.errored) {
        return Failure::kErrored;
    }
    if (!lhs.matched) {
        return Failure::kNoMatch;
    }

    auto bitwise = bitwise_expression_post_unary_expression(lhs.value);
    if (bitwise.errored) {
        return Failure::kErrored;
    }
    if (bitwise.matched) {
        return bitwise.value;
    }

    auto math = expect_math_expression_post_unary_expression(lhs.value);
    if (math.errored) {
        return Failure::kErrored;
    }
    return math.value;
}

// shift_expression
//   : unary_expression shift_expression.post.unary_expression
Maybe<const ast::Expression*> ParserImpl::shift_expression() {
    auto lhs = unary_expression();
    if (lhs.errored) {
        return Failure::kErrored;
    }
    if (!lhs.matched) {
        return Failure::kNoMatch;
    }
    return expect_shift_expression_post_unary_expression(lhs.value);
}

// shift_expression.post.unary_expression
//   : math_expression.post.unary_expression?
//   | SHIFT_LEFT unary_expression
//   | SHIFT_RIGHT unary_expression
//
// Note, add the `math_expression.post.unary_expression` is added here to make
// implementation simpler.
Expect<const ast::Expression*> ParserImpl::expect_shift_expression_post_unary_expression(
    const ast::Expression* lhs) {
    auto& t = peek();
    if (match(Token::Type::kShiftLeft) || match(Token::Type::kShiftRight)) {
        std::string name;
        ast::BinaryOp op = ast::BinaryOp::kNone;
        if (t.Is(Token::Type::kShiftLeft)) {
            op = ast::BinaryOp::kShiftLeft;
            name = "<<";
        } else if (t.Is(Token::Type::kShiftRight)) {
            op = ast::BinaryOp::kShiftRight;
            name = ">>";
        }

        auto& rhs_start = peek();
        auto rhs = unary_expression();
        if (rhs.errored) {
            return Failure::kErrored;
        }
        if (!rhs.matched) {
            return add_error(rhs_start,
                             std::string("unable to parse right side of ") + name + " expression");
        }
        return create<ast::BinaryExpression>(t.source(), op, lhs, rhs.value);
    }

    return expect_math_expression_post_unary_expression(lhs);
}

// relational_expression
//   : unary_expression relational_expression.post.unary_expression
Maybe<const ast::Expression*> ParserImpl::relational_expression() {
    auto lhs = unary_expression();
    if (lhs.errored) {
        return Failure::kErrored;
    }
    if (!lhs.matched) {
        return Failure::kNoMatch;
    }
    return expect_relational_expression_post_unary_expression(lhs.value);
}

// relational_expression.post.unary_expression
//   : shift_expression.post.unary_expression
//   | shift_expression.post.unary_expression EQUAL_EQUAL shift_expression
//   | shift_expression.post.unary_expression GREATER_THAN shift_expression
//   | shift_expression.post.unary_expression GREATER_THAN_EQUAL shift_expression
//   | shift_expression.post.unary_expression LESS_THAN shift_expression
//   | shift_expression.post.unary_expression LESS_THAN_EQUAL shift_expression
//   | shift_expression.post.unary_expression NOT_EQUAL shift_expression
//
// Note, a `shift_expression` element was added to simplify many of the right sides
Expect<const ast::Expression*> ParserImpl::expect_relational_expression_post_unary_expression(
    const ast::Expression* lhs) {
    auto lhs_result = expect_shift_expression_post_unary_expression(lhs);
    if (lhs_result.errored) {
        return Failure::kErrored;
    }
    lhs = lhs_result.value;

    auto& tok_op = peek();

    ast::BinaryOp op = ast::BinaryOp::kNone;
    switch (tok_op.type()) {
        case Token::Type::kLessThan:
            op = ast::BinaryOp::kLessThan;
            break;
        case Token::Type::kGreaterThan:
            op = ast::BinaryOp::kGreaterThan;
            break;
        case Token::Type::kLessThanEqual:
            op = ast::BinaryOp::kLessThanEqual;
            break;
        case Token::Type::kGreaterThanEqual:
            op = ast::BinaryOp::kGreaterThanEqual;
            break;
        case Token::Type::kEqualEqual:
            op = ast::BinaryOp::kEqual;
            break;
        case Token::Type::kNotEqual:
            op = ast::BinaryOp::kNotEqual;
            break;
        default:
            return lhs;
    }

    next();  // consume tok_op

    auto& tok_rhs = peek();
    auto rhs = shift_expression();
    if (rhs.errored) {
        return Failure::kErrored;
    }
    if (!rhs.matched) {
        return add_error(tok_rhs, std::string("unable to parse right side of ") +
                                      std::string(tok_op.to_name()) + " expression");
    }

    return create<ast::BinaryExpression>(tok_op.source(), op, lhs, rhs.value);
}

// expression
//   : unary_expression bitwise_expression.post.unary_expression
//   | unary_expression relational_expression.post.unary_expression
//   | unary_expression relational_expression.post.unary_expression and_and
//        relational_expression ( and_and relational_expression )*
//   | unary_expression relational_expression.post.unary_expression or_or
//        relational_expression ( or_or relational_expression )*
//
// Note, a `relational_expression` element was added to simplify many of the right sides
Maybe<const ast::Expression*> ParserImpl::expression() {
    auto expr = [&]() -> Maybe<const ast::Expression*> {
        auto lhs = unary_expression();
        if (lhs.errored) {
            return Failure::kErrored;
        }
        if (!lhs.matched) {
            return Failure::kNoMatch;
        }

        auto bitwise = bitwise_expression_post_unary_expression(lhs.value);
        if (bitwise.errored) {
            return Failure::kErrored;
        }
        if (bitwise.matched) {
            return bitwise.value;
        }

        auto relational = expect_relational_expression_post_unary_expression(lhs.value);
        if (relational.errored) {
            return Failure::kErrored;
        }
        auto* ret = relational.value;

        auto& t = peek();
        if (t.Is(Token::Type::kAndAnd) || t.Is(Token::Type::kOrOr)) {
            ast::BinaryOp op = ast::BinaryOp::kNone;
            if (t.Is(Token::Type::kAndAnd)) {
                op = ast::BinaryOp::kLogicalAnd;
            } else if (t.Is(Token::Type::kOrOr)) {
                op = ast::BinaryOp::kLogicalOr;
            }

            while (continue_parsing()) {
                auto& n = peek();
                if (!n.Is(t.type())) {
                    break;
                }
                next();

                auto rhs = relational_expression();
                if (rhs.errored) {
                    return Failure::kErrored;
                }
                if (!rhs.matched) {
                    return add_error(peek(), std::string("unable to parse right side of ") +
                                                 std::string(t.to_name()) + " expression");
                }

                ret = create<ast::BinaryExpression>(t.source(), op, ret, rhs.value);
            }
        }
        return ret;
    }();

    if (expr.matched) {
        // Note, expression is greedy an will consume all the operators of the same type
        // so, `a & a & a` would all be consumed above. If you see any binary operator
        // after this then it _must_ be a different one, and hence an error.
        if (auto* lhs = expr->As<ast::BinaryExpression>()) {
            if (auto& n = peek(); n.IsBinaryOperator()) {
                auto source = Source::Combine(expr->source, n.source());
                add_error(source, std::string("mixing '") + ast::Operator(lhs->op) + "' and '" +
                                      std::string(n.to_name()) + "' requires parenthesis");
                return Failure::kErrored;
            }
        }
    }

    return expr;
}

// singular_expression
//   : primary_expression postfix_expr
Maybe<const ast::Expression*> ParserImpl::singular_expression() {
    auto prefix = primary_expression();
    if (prefix.errored) {
        return Failure::kErrored;
    }
    if (!prefix.matched) {
        return Failure::kNoMatch;
    }

    return component_or_swizzle_specifier(prefix.value);
}

// unary_expression
//   : singular_expression
//   | MINUS unary_expression
//   | BANG unary_expression
//   | TILDE unary_expression
//   | STAR unary_expression
//   | AND unary_expression
//
// The `primary_expression component_or_swizzle_specifier ?` is moved out into a
// `singular_expression`
Maybe<const ast::Expression*> ParserImpl::unary_expression() {
    auto& t = peek();

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

// compound_assignment_operator
//   : plus_equal
//   | minus_equal
//   | times_equal
//   | division_equal
//   | modulo_equal
//   | and_equal
//   | or_equal
//   | xor_equal
//   | shift_right_equal
//   | shift_left_equal
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
    } else if (peek_is(Token::Type::kShiftLeftEqual)) {
        compound_op = ast::BinaryOp::kShiftLeft;
    } else if (peek_is(Token::Type::kShiftRightEqual)) {
        compound_op = ast::BinaryOp::kShiftRight;
    }
    if (compound_op != ast::BinaryOp::kNone) {
        next();
        return compound_op;
    }
    return Failure::kNoMatch;
}

// core_lhs_expression
//   : ident
//   | PAREN_LEFT lhs_expression PAREN_RIGHT
Maybe<const ast::Expression*> ParserImpl::core_lhs_expression() {
    auto& t = peek();
    if (t.IsIdentifier()) {
        next();

        return create<ast::IdentifierExpression>(t.source(),
                                                 builder_.Symbols().Register(t.to_str()));
    }

    if (peek_is(Token::Type::kParenLeft)) {
        return expect_paren_block("", [&]() -> Expect<const ast::Expression*> {
            auto expr = lhs_expression();
            if (expr.errored) {
                return Failure::kErrored;
            }
            if (!expr.matched) {
                return add_error(t, "invalid expression");
            }
            return expr.value;
        });
    }

    return Failure::kNoMatch;
}

// lhs_expression
//   : ( STAR | AND )* core_lhs_expression component_or_swizzle_specifier?
Maybe<const ast::Expression*> ParserImpl::lhs_expression() {
    std::vector<const Token*> prefixes;
    while (peek_is(Token::Type::kStar) || peek_is(Token::Type::kAnd) ||
           peek_is(Token::Type::kAndAnd)) {
        auto& t = next();

        // If an '&&' is provided split into '&' and '&'
        if (t.Is(Token::Type::kAndAnd)) {
            split_token(Token::Type::kAnd, Token::Type::kAnd);
        }

        prefixes.push_back(&t);
    }

    auto core_expr = core_lhs_expression();
    if (core_expr.errored) {
        return Failure::kErrored;
    } else if (!core_expr.matched) {
        if (prefixes.empty()) {
            return Failure::kNoMatch;
        }

        return add_error(peek(), "missing expression");
    }

    const auto* expr = core_expr.value;
    for (auto it = prefixes.rbegin(); it != prefixes.rend(); ++it) {
        auto& t = **it;
        ast::UnaryOp op = ast::UnaryOp::kAddressOf;
        if (t.Is(Token::Type::kStar)) {
            op = ast::UnaryOp::kIndirection;
        }
        expr = create<ast::UnaryOpExpression>(t.source(), op, expr);
    }

    auto e = component_or_swizzle_specifier(expr);
    if (e.errored) {
        return Failure::kErrored;
    }
    return e.value;
}

// variable_updating_statement
//   : lhs_expression ( EQUAL | compound_assignment_operator ) expression
//   | lhs_expression MINUS_MINUS
//   | lhs_expression PLUS_PLUS
//   | UNDERSCORE EQUAL expression
//
// Note, this is a simplification of the recursive grammar statement with the `lhs_expression`
// substituted back into the expression.
Maybe<const ast::Statement*> ParserImpl::variable_updating_statement() {
    auto& t = peek();

    // tint:295 - Test for `ident COLON` - this is invalid grammar, and without
    // special casing will error as "missing = for assignment", which is less
    // helpful than this error message:
    if (peek_is(Token::Type::kIdentifier) && peek_is(Token::Type::kColon, 1)) {
        return add_error(peek(0).source(), "expected 'var' for variable declaration");
    }

    const ast::Expression* lhs = nullptr;
    ast::BinaryOp compound_op = ast::BinaryOp::kNone;
    if (peek_is(Token::Type::kUnderscore)) {
        next();  // Consume the peek.

        if (!expect("assignment", Token::Type::kEqual)) {
            return Failure::kErrored;
        }

        lhs = create<ast::PhonyExpression>(t.source());

    } else {
        auto lhs_result = lhs_expression();
        if (lhs_result.errored) {
            return Failure::kErrored;
        }
        if (!lhs_result.matched) {
            return Failure::kNoMatch;
        }

        lhs = lhs_result.value;

        // Handle increment and decrement statements.
        if (match(Token::Type::kPlusPlus)) {
            return create<ast::IncrementDecrementStatement>(t.source(), lhs, true);
        }
        if (match(Token::Type::kMinusMinus)) {
            return create<ast::IncrementDecrementStatement>(t.source(), lhs, false);
        }

        auto compound_op_result = compound_assignment_operator();
        if (compound_op_result.errored) {
            return Failure::kErrored;
        }
        if (compound_op_result.matched) {
            compound_op = compound_op_result.value;
        } else {
            if (!expect("assignment", Token::Type::kEqual)) {
                return Failure::kErrored;
            }
        }
    }

    auto rhs = expression();
    if (rhs.errored) {
        return Failure::kErrored;
    }
    if (!rhs.matched) {
        return add_error(peek(), "unable to parse right side of assignment");
    }

    if (compound_op != ast::BinaryOp::kNone) {
        return create<ast::CompoundAssignmentStatement>(t.source(), lhs, rhs.value, compound_op);
    }
    return create<ast::AssignmentStatement>(t.source(), lhs, rhs.value);
}

// const_literal
//   : INT_LITERAL
//   | FLOAT_LITERAL
//   | bool_literal
//
// bool_literal
//   : TRUE
//   | FALSE
Maybe<const ast::LiteralExpression*> ParserImpl::const_literal() {
    auto& t = peek();
    if (match(Token::Type::kIntLiteral)) {
        return create<ast::IntLiteralExpression>(t.source(), t.to_i64(),
                                                 ast::IntLiteralExpression::Suffix::kNone);
    }
    if (match(Token::Type::kIntLiteral_I)) {
        return create<ast::IntLiteralExpression>(t.source(), t.to_i64(),
                                                 ast::IntLiteralExpression::Suffix::kI);
    }
    if (match(Token::Type::kIntLiteral_U)) {
        return create<ast::IntLiteralExpression>(t.source(), t.to_i64(),
                                                 ast::IntLiteralExpression::Suffix::kU);
    }
    if (match(Token::Type::kFloatLiteral)) {
        return create<ast::FloatLiteralExpression>(t.source(), t.to_f64(),
                                                   ast::FloatLiteralExpression::Suffix::kNone);
    }
    if (match(Token::Type::kFloatLiteral_F)) {
        return create<ast::FloatLiteralExpression>(t.source(), t.to_f64(),
                                                   ast::FloatLiteralExpression::Suffix::kF);
    }
    if (match(Token::Type::kFloatLiteral_H)) {
        return create<ast::FloatLiteralExpression>(t.source(), t.to_f64(),
                                                   ast::FloatLiteralExpression::Suffix::kH);
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

Maybe<ParserImpl::AttributeList> ParserImpl::attribute_list() {
    bool errored = false;
    AttributeList attrs;

    while (continue_parsing()) {
        if (match(Token::Type::kAttr)) {
            if (auto attr = expect_attribute(); attr.errored) {
                errored = true;
            } else {
                attrs.Push(attr.value);
            }
        } else {
            break;
        }
    }

    if (errored) {
        return Failure::kErrored;
    }

    if (attrs.IsEmpty()) {
        return Failure::kNoMatch;
    }

    return attrs;
}

Expect<const ast::Attribute*> ParserImpl::expect_attribute() {
    auto& t = peek();
    auto attr = attribute();
    if (attr.errored) {
        return Failure::kErrored;
    }
    if (attr.matched) {
        return attr.value;
    }
    return add_error(t, "expected attribute");
}

// attribute
//   : ATTR 'align' PAREN_LEFT expression COMMA? PAREN_RIGHT
//   | ATTR 'binding' PAREN_LEFT expression COMMA? PAREN_RIGHT
//   | ATTR 'builtin' PAREN_LEFT builtin_value_name COMMA? PAREN_RIGHT
//   | ATTR 'const'
//   | ATTR 'group' PAREN_LEFT expression COMMA? PAREN_RIGHT
//   | ATTR 'id' PAREN_LEFT expression COMMA? PAREN_RIGHT
//   | ATTR 'interpolate' PAREN_LEFT interpolation_type_name COMMA? PAREN_RIGHT
//   | ATTR 'interpolate' PAREN_LEFT interpolation_type_name COMMA
//                                   interpolation_sample_name COMMA? PAREN_RIGHT
//   | ATTR 'invariant'
//   | ATTR 'location' PAREN_LEFT expression COMMA? PAREN_RIGHT
//   | ATTR 'size' PAREN_LEFT expression COMMA? PAREN_RIGHT
//   | ATTR 'workgroup_size' PAREN_LEFT expression COMMA? PAREN_RIGHT
//   | ATTR 'workgroup_size' PAREN_LEFT expression COMMA expression COMMA? PAREN_RIGHT
//   | ATTR 'workgroup_size' PAREN_LEFT expression COMMA expression COMMA
//                                      expression COMMA? PAREN_RIGHT
//   | ATTR 'vertex'
//   | ATTR 'fragment'
//   | ATTR 'compute'
Maybe<const ast::Attribute*> ParserImpl::attribute() {
    using Result = Maybe<const ast::Attribute*>;
    auto& t = next();

    if (!t.IsIdentifier()) {
        return Failure::kNoMatch;
    }

    if (t == "align") {
        const char* use = "align attribute";
        return expect_paren_block(use, [&]() -> Result {
            auto expr = expression();
            if (expr.errored) {
                return Failure::kErrored;
            }
            if (!expr.matched) {
                return add_error(peek(), "expected align expression");
            }
            match(Token::Type::kComma);

            return create<ast::StructMemberAlignAttribute>(t.source(), expr.value);
        });
    }

    if (t == "binding") {
        const char* use = "binding attribute";
        return expect_paren_block(use, [&]() -> Result {
            auto expr = expression();
            if (expr.errored) {
                return Failure::kErrored;
            }
            if (!expr.matched) {
                return add_error(peek(), "expected binding expression");
            }
            match(Token::Type::kComma);

            return create<ast::BindingAttribute>(t.source(), expr.value);
        });
    }

    if (t == "builtin") {
        return expect_paren_block("builtin attribute", [&]() -> Result {
            auto builtin = expect_builtin();
            if (builtin.errored) {
                return Failure::kErrored;
            }
            match(Token::Type::kComma);

            return create<ast::BuiltinAttribute>(t.source(), builtin.value);
        });
    }

    if (t == "compute") {
        return create<ast::StageAttribute>(t.source(), ast::PipelineStage::kCompute);
    }

    // Note, `const` is not valid in a WGSL source file, it's internal only

    if (t == "fragment") {
        return create<ast::StageAttribute>(t.source(), ast::PipelineStage::kFragment);
    }

    if (t == "group") {
        const char* use = "group attribute";
        return expect_paren_block(use, [&]() -> Result {
            auto expr = expression();
            if (expr.errored) {
                return Failure::kErrored;
            }
            if (!expr.matched) {
                return add_error(peek(), "expected group expression");
            }
            match(Token::Type::kComma);

            return create<ast::GroupAttribute>(t.source(), expr.value);
        });
    }

    if (t == "id") {
        const char* use = "id attribute";
        return expect_paren_block(use, [&]() -> Result {
            auto expr = expression();
            if (expr.errored) {
                return Failure::kErrored;
            }
            if (!expr.matched) {
                return add_error(peek(), "expected id expression");
            }
            match(Token::Type::kComma);

            return create<ast::IdAttribute>(t.source(), expr.value);
        });
    }

    if (t == "interpolate") {
        return expect_paren_block("interpolate attribute", [&]() -> Result {
            auto type = expect_interpolation_type_name();
            if (type.errored) {
                return Failure::kErrored;
            }

            ast::InterpolationSampling sampling = ast::InterpolationSampling::kUndefined;
            if (match(Token::Type::kComma)) {
                if (!peek_is(Token::Type::kParenRight)) {
                    auto sample = expect_interpolation_sample_name();
                    if (sample.errored) {
                        return Failure::kErrored;
                    }

                    sampling = sample.value;
                    match(Token::Type::kComma);
                }
            }

            return create<ast::InterpolateAttribute>(t.source(), type.value, sampling);
        });
    }

    if (t == "invariant") {
        return create<ast::InvariantAttribute>(t.source());
    }

    if (t == "location") {
        const char* use = "location attribute";
        return expect_paren_block(use, [&]() -> Result {
            auto expr = expression();
            if (expr.errored) {
                return Failure::kErrored;
            }
            if (!expr.matched) {
                return add_error(peek(), "expected location expression");
            }
            match(Token::Type::kComma);

            return builder_.Location(t.source(), expr.value);
        });
    }

    if (t == "size") {
        const char* use = "size attribute";
        return expect_paren_block(use, [&]() -> Result {
            auto expr = expression();
            if (expr.errored) {
                return Failure::kErrored;
            }
            if (!expr.matched) {
                return add_error(peek(), "expected size expression");
            }
            match(Token::Type::kComma);

            return builder_.MemberSize(t.source(), expr.value);
        });
    }

    if (t == "vertex") {
        return create<ast::StageAttribute>(t.source(), ast::PipelineStage::kVertex);
    }

    if (t == "workgroup_size") {
        return expect_paren_block("workgroup_size attribute", [&]() -> Result {
            const ast::Expression* x = nullptr;
            const ast::Expression* y = nullptr;
            const ast::Expression* z = nullptr;

            auto expr = expression();
            if (expr.errored) {
                return Failure::kErrored;
            } else if (!expr.matched) {
                return add_error(peek(), "expected workgroup_size x parameter");
            }
            x = std::move(expr.value);

            if (match(Token::Type::kComma)) {
                if (!peek_is(Token::Type::kParenRight)) {
                    expr = expression();
                    if (expr.errored) {
                        return Failure::kErrored;
                    } else if (!expr.matched) {
                        return add_error(peek(), "expected workgroup_size y parameter");
                    }
                    y = std::move(expr.value);

                    if (match(Token::Type::kComma)) {
                        if (!peek_is(Token::Type::kParenRight)) {
                            expr = expression();
                            if (expr.errored) {
                                return Failure::kErrored;
                            } else if (!expr.matched) {
                                return add_error(peek(), "expected workgroup_size z parameter");
                            }
                            z = std::move(expr.value);

                            match(Token::Type::kComma);
                        }
                    }
                }
            }

            return create<ast::WorkgroupAttribute>(t.source(), x, y, z);
        });
    }
    return Failure::kNoMatch;
}

bool ParserImpl::expect_attributes_consumed(utils::VectorRef<const ast::Attribute*> in) {
    if (in.IsEmpty()) {
        return true;
    }
    add_error(in[0]->source, "unexpected attributes");
    return false;
}

bool ParserImpl::match(Token::Type tok, Source* source /*= nullptr*/) {
    auto& t = peek();

    if (source != nullptr) {
        *source = t.source();
    }

    if (t.Is(tok)) {
        next();
        return true;
    }
    return false;
}

bool ParserImpl::expect(std::string_view use, Token::Type tok) {
    auto& t = peek();
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
        if (t.Is(Token::Type::kShiftRight)) {
            split_token(Token::Type::kGreaterThan, Token::Type::kGreaterThan);
        } else if (t.Is(Token::Type::kGreaterThanEqual)) {
            split_token(Token::Type::kGreaterThan, Token::Type::kEqual);
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
    auto& t = peek();
    if (!t.Is(Token::Type::kIntLiteral) && !t.Is(Token::Type::kIntLiteral_I)) {
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
    if (sint.errored) {
        return Failure::kErrored;
    }

    if (sint.value < 0) {
        return add_error(sint.source, std::string(use) + " must be positive");
    }

    return {static_cast<uint32_t>(sint.value), sint.source};
}

Expect<uint32_t> ParserImpl::expect_nonzero_positive_sint(std::string_view use) {
    auto sint = expect_sint(use);
    if (sint.errored) {
        return Failure::kErrored;
    }

    if (sint.value <= 0) {
        return add_error(sint.source, std::string(use) + " must be greater than 0");
    }

    return {static_cast<uint32_t>(sint.value), sint.source};
}

Expect<std::string> ParserImpl::expect_ident(std::string_view use) {
    auto& t = peek();
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

        if (res.errored) {
            return Failure::kErrored;
        }

        if (!expect(use, end)) {
            return Failure::kErrored;
        }

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
        auto& t = peek(i);
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
