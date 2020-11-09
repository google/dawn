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

#ifndef SRC_READER_WGSL_PARSER_IMPL_H_
#define SRC_READER_WGSL_PARSER_IMPL_H_

#include <cassert>
#include <deque>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/ast/array_decoration.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/builtin.h"
#include "src/ast/call_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/constructor_expression.h"
#include "src/ast/else_statement.h"
#include "src/ast/function.h"
#include "src/ast/literal.h"
#include "src/ast/loop_statement.h"
#include "src/ast/module.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/statement.h"
#include "src/ast/storage_class.h"
#include "src/ast/struct.h"
#include "src/ast/struct_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/type/storage_texture_type.h"
#include "src/ast/type/texture_type.h"
#include "src/ast/type/type.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decoration.h"
#include "src/context.h"
#include "src/diagnostic/diagnostic.h"
#include "src/diagnostic/formatter.h"
#include "src/reader/wgsl/parser_impl_detail.h"
#include "src/reader/wgsl/token.h"

namespace tint {
namespace reader {
namespace wgsl {

class Lexer;

/// Struct holding information for a for loop
struct ForHeader {
  /// Constructor
  /// @param init the initializer statement
  /// @param cond the condition statement
  /// @param cont the continuing statement
  ForHeader(std::unique_ptr<ast::Statement> init,
            std::unique_ptr<ast::Expression> cond,
            std::unique_ptr<ast::Statement> cont);

  ~ForHeader();

  /// The for loop initializer
  std::unique_ptr<ast::Statement> initializer;
  /// The for loop condition
  std::unique_ptr<ast::Expression> condition;
  /// The for loop continuing statement
  std::unique_ptr<ast::Statement> continuing;
};

/// ParserImpl for WGSL source data
class ParserImpl {
  /// Failure holds enumerator values used for the constructing an Expect and
  /// Match in an errored state.
  struct Failure {
    enum Errored { kErrored };
    enum NoMatch { kNoMatch };
  };

 public:
  /// Expect is the return type of the parser methods that are expected to
  /// return a parsed value of type T, unless there was an parse error.
  /// In the case of a parse error the called method will have called
  /// |add_error()| and the Expect will have |errored| set to true.
  template <typename T>
  struct Expect {
    /// An alias to the templated type T.
    using type = T;

    /// Don't allow an Expect to take a nullptr.
    inline Expect(std::nullptr_t) = delete;  // NOLINT

    /// Constructor for a successful parse.
    /// @param val the result value of the parse
    /// @param s the optional source of the value
    template <typename U>
    inline Expect(U&& val, const Source& s = {})  // NOLINT
        : value(std::forward<U>(val)), source(s) {}

    /// Constructor for parse error.
    inline Expect(Failure::Errored) : errored(true) {}  // NOLINT

    /// Copy constructor
    inline Expect(const Expect&) = default;
    /// Move constructor
    inline Expect(Expect&&) = default;
    /// Assignment operator
    /// @return this Expect
    inline Expect& operator=(const Expect&) = default;
    /// Assignment move operator
    /// @return this Expect
    inline Expect& operator=(Expect&&) = default;

    /// @return a pointer to the returned value. If T is a pointer or
    /// std::unique_ptr, operator->() automatically dereferences so that the
    /// return type will always be a pointer to a non-pointer type. |errored|
    /// must be false to call.
    inline typename detail::OperatorArrow<T>::type operator->() {
      assert(!errored);
      return detail::OperatorArrow<T>::ptr(value);
    }

    /// The expected value of a successful parse.
    /// Zero-initialized when there was a parse error.
    T value{};
    /// Optional source of the value.
    Source source;
    /// True if there was a error parsing.
    bool errored = false;
  };

  /// Maybe is the return type of the parser methods that attempts to match a
  /// grammar and return a parsed value of type T, or may parse part of the
  /// grammar and then hit a parse error.
  /// In the case of a successful grammar match, the Maybe will have |matched|
  /// set to true.
  /// In the case of a parse error the called method will have called
  /// |add_error()| and the Maybe will have |errored| set to true.
  template <typename T>
  struct Maybe {
    inline Maybe(std::nullptr_t) = delete;  // NOLINT

    /// Constructor for a successful parse.
    /// @param val the result value of the parse
    /// @param s the optional source of the value
    template <typename U>
    inline Maybe(U&& val, const Source& s = {})  // NOLINT
        : value(std::forward<U>(val)), source(s), matched(true) {}

    /// Constructor for parse error state.
    inline Maybe(Failure::Errored) : errored(true) {}  // NOLINT

    /// Constructor for the no-match state.
    inline Maybe(Failure::NoMatch) {}  // NOLINT

    /// Constructor from an Expect.
    /// @param e the Expect to copy this Maybe from
    template <typename U>
    inline Maybe(const Expect<U>& e)  // NOLINT
        : value(e.value),
          source(e.value),
          errored(e.errored),
          matched(!e.errored) {}

    /// Move from an Expect.
    /// @param e the Expect to move this Maybe from
    template <typename U>
    inline Maybe(Expect<U>&& e)  // NOLINT
        : value(std::move(e.value)),
          source(std::move(e.source)),
          errored(e.errored),
          matched(!e.errored) {}

    /// Copy constructor
    inline Maybe(const Maybe&) = default;
    /// Move constructor
    inline Maybe(Maybe&&) = default;
    /// Assignment operator
    /// @return this Maybe
    inline Maybe& operator=(const Maybe&) = default;
    /// Assignment move operator
    /// @return this Maybe
    inline Maybe& operator=(Maybe&&) = default;

    /// @return a pointer to the returned value. If T is a pointer or
    /// std::unique_ptr, operator->() automatically dereferences so that the
    /// return type will always be a pointer to a non-pointer type. |errored|
    /// must be false to call.
    inline typename detail::OperatorArrow<T>::type operator->() {
      assert(!errored);
      return detail::OperatorArrow<T>::ptr(value);
    }

    /// The value of a successful parse.
    /// Zero-initialized when there was a parse error.
    T value{};
    /// Optional source of the value.
    Source source;
    /// True if there was a error parsing.
    bool errored = false;
    /// True if there was a error parsing.
    bool matched = false;
  };

  /// TypedIdentifier holds a parsed identifier and type. Returned by
  /// variable_ident_decl().
  struct TypedIdentifier {
    /// Parsed type.
    ast::type::Type* type = nullptr;
    /// Parsed identifier.
    std::string name;
    /// Source to the identifier.
    Source source;
  };

  /// Creates a new parser using the given file
  /// @param ctx the non-null context object
  /// @param file the input source file to parse
  ParserImpl(Context* ctx, Source::File const* file);
  ~ParserImpl();

  /// Run the parser
  /// @returns true if the parse was successful, false otherwise.
  bool Parse();

  /// @returns true if an error was encountered.
  bool has_error() const { return diags_.contains_errors(); }

  /// @returns the parser error string
  std::string error() const {
    diag::Formatter formatter{{false, false, false}};
    return formatter.format(diags_);
  }

  /// @returns the diagnostic messages
  const diag::List& diagnostics() const { return diags_; }

  /// @returns the diagnostic messages
  diag::List& diagnostics() { return diags_; }

  /// @returns the module. The module in the parser will be reset after this.
  ast::Module module() { return std::move(module_); }

  /// @returns the next token
  Token next();
  /// @returns the next token without advancing
  Token peek();
  /// Peeks ahead and returns the token at |idx| head of the current position
  /// @param idx the index of the token to return
  /// @returns the token |idx| positions ahead without advancing
  Token peek(size_t idx);
  /// Appends an error at |t| with the message |msg|
  /// @param t the token to associate the error with
  /// @param msg the error message
  /// @return |errored| so that you can combine an add_error call and return on
  /// the same line.
  Failure::Errored add_error(const Token& t, const std::string& msg);
  /// Appends an error raised when parsing |use| at |t| with the message |msg|
  /// @param source the source to associate the error with
  /// @param msg the error message
  /// @param use a description of what was being parsed when the error was
  /// raised.
  /// @return |errored| so that you can combine an add_error call and return on
  /// the same line.
  Failure::Errored add_error(const Source& source,
                             const std::string& msg,
                             const std::string& use);
  /// Appends an error at |source| with the message |msg|
  /// @param source the source to associate the error with
  /// @param msg the error message
  /// @return |errored| so that you can combine an add_error call and return on
  /// the same line.
  Failure::Errored add_error(const Source& source, const std::string& msg);

  /// Registers a constructed type into the parser
  /// @param name the constructed name
  /// @param type the constructed type
  void register_constructed(const std::string& name, ast::type::Type* type);
  /// Retrieves a constructed type
  /// @param name The name to lookup
  /// @returns the constructed type for |name| or nullptr if not found
  ast::type::Type* get_constructed(const std::string& name);

  /// Parses the `translation_unit` grammar element
  void translation_unit();
  /// Parses the `global_decl` grammar element, erroring on parse failure.
  /// @return true on parse success, otherwise an error.
  Expect<bool> expect_global_decl();
  /// Parses a `global_variable_decl` grammar element with the initial
  /// `variable_decoration_list*` provided as |decos|.
  /// @returns the variable parsed or nullptr
  /// @param decos the list of decorations for the variable declaration.
  Maybe<std::unique_ptr<ast::Variable>> global_variable_decl(
      ast::DecorationList& decos);
  /// Parses a `global_constant_decl` grammar element
  /// @returns the const object or nullptr
  Maybe<std::unique_ptr<ast::Variable>> global_constant_decl();
  /// Parses a `variable_decl` grammar element
  /// @returns the parsed variable or nullptr otherwise
  Maybe<std::unique_ptr<ast::Variable>> variable_decl();
  /// Parses a `variable_ident_decl` grammar element, erroring on parse
  /// failure.
  /// @param use a description of what was being parsed if an error was raised.
  /// @returns the identifier and type parsed or empty otherwise
  Expect<TypedIdentifier> expect_variable_ident_decl(const std::string& use);
  /// Parses a `variable_storage_decoration` grammar element
  /// @returns the storage class or StorageClass::kNone if none matched
  Maybe<ast::StorageClass> variable_storage_decoration();
  /// Parses a `type_alias` grammar element
  /// @returns the type alias or nullptr on error
  Maybe<ast::type::Type*> type_alias();
  /// Parses a `type_decl` grammar element
  /// @returns the parsed Type or nullptr if none matched.
  Maybe<ast::type::Type*> type_decl();
  /// Parses a `storage_class` grammar element, erroring on parse failure.
  /// @param use a description of what was being parsed if an error was raised.
  /// @returns the storage class or StorageClass::kNone if none matched
  Expect<ast::StorageClass> expect_storage_class(const std::string& use);
  /// Parses a `struct_decl` grammar element with the initial
  /// `struct_decoration_decl*` provided as |decos|.
  /// @returns the struct type or nullptr on error
  /// @param decos the list of decorations for the struct declaration.
  Maybe<std::unique_ptr<ast::type::StructType>> struct_decl(
      ast::DecorationList& decos);
  /// Parses a `struct_body_decl` grammar element, erroring on parse failure.
  /// @returns the struct members
  Expect<ast::StructMemberList> expect_struct_body_decl();
  /// Parses a `struct_member` grammar element with the initial
  /// `struct_member_decoration_decl+` provided as |decos|, erroring on parse
  /// failure.
  /// @param decos the list of decorations for the struct member.
  /// @returns the struct member or nullptr
  Expect<std::unique_ptr<ast::StructMember>> expect_struct_member(
      ast::DecorationList& decos);
  /// Parses a `function_decl` grammar element with the initial
  /// `function_decoration_decl*` provided as |decos|.
  /// @param decos the list of decorations for the function declaration.
  /// @returns the parsed function, nullptr otherwise
  Maybe<std::unique_ptr<ast::Function>> function_decl(
      ast::DecorationList& decos);
  /// Parses a `texture_sampler_types` grammar element
  /// @returns the parsed Type or nullptr if none matched.
  Maybe<ast::type::Type*> texture_sampler_types();
  /// Parses a `sampler_type` grammar element
  /// @returns the parsed Type or nullptr if none matched.
  Maybe<ast::type::Type*> sampler_type();
  /// Parses a `multisampled_texture_type` grammar element
  /// @returns returns the multisample texture dimension or kNone if none
  /// matched.
  Maybe<ast::type::TextureDimension> multisampled_texture_type();
  /// Parses a `sampled_texture_type` grammar element
  /// @returns returns the sample texture dimension or kNone if none matched.
  Maybe<ast::type::TextureDimension> sampled_texture_type();
  /// Parses a `storage_texture_type` grammar element
  /// @returns returns the storage texture dimension and the storage access.
  ///          Returns kNone and kRead if none matched.
  Maybe<std::pair<ast::type::TextureDimension, ast::AccessControl>>
  storage_texture_type();
  /// Parses a `depth_texture_type` grammar element
  /// @returns the parsed Type or nullptr if none matched.
  Maybe<ast::type::Type*> depth_texture_type();
  /// Parses a `image_storage_type` grammar element
  /// @param use a description of what was being parsed if an error was raised
  /// @returns returns the image format or kNone if none matched.
  Expect<ast::type::ImageFormat> expect_image_storage_type(
      const std::string& use);
  /// Parses a `function_type_decl` grammar element
  /// @returns the parsed type or nullptr otherwise
  Maybe<ast::type::Type*> function_type_decl();
  /// Parses a `function_header` grammar element
  /// @returns the parsed function nullptr otherwise
  Maybe<std::unique_ptr<ast::Function>> function_header();
  /// Parses a `param_list` grammar element, erroring on parse failure.
  /// @returns the parsed variables
  Expect<ast::VariableList> expect_param_list();
  /// Parses a `pipeline_stage` grammar element, erroring if the next token does
  /// not match a stage name.
  /// @returns the pipeline stage.
  Expect<ast::PipelineStage> expect_pipeline_stage();
  /// Parses a builtin identifier, erroring if the next token does not match a
  /// valid builtin name.
  /// @returns the parsed builtin.
  Expect<ast::Builtin> expect_builtin();
  /// Parses a `body_stmt` grammar element, erroring on parse failure.
  /// @returns the parsed statements
  Expect<std::unique_ptr<ast::BlockStatement>> expect_body_stmt();
  /// Parses a `paren_rhs_stmt` grammar element, erroring on parse failure.
  /// @returns the parsed element or nullptr
  Expect<std::unique_ptr<ast::Expression>> expect_paren_rhs_stmt();
  /// Parses a `statements` grammar element
  /// @returns the statements parsed
  Expect<std::unique_ptr<ast::BlockStatement>> expect_statements();
  /// Parses a `statement` grammar element
  /// @returns the parsed statement or nullptr
  Maybe<std::unique_ptr<ast::Statement>> statement();
  /// Parses a `break_stmt` grammar element
  /// @returns the parsed statement or nullptr
  Maybe<std::unique_ptr<ast::BreakStatement>> break_stmt();
  /// Parses a `return_stmt` grammar element
  /// @returns the parsed statement or nullptr
  Maybe<std::unique_ptr<ast::ReturnStatement>> return_stmt();
  /// Parses a `continue_stmt` grammar element
  /// @returns the parsed statement or nullptr
  Maybe<std::unique_ptr<ast::ContinueStatement>> continue_stmt();
  /// Parses a `variable_stmt` grammar element
  /// @returns the parsed variable or nullptr
  Maybe<std::unique_ptr<ast::VariableDeclStatement>> variable_stmt();
  /// Parses a `if_stmt` grammar element
  /// @returns the parsed statement or nullptr
  Maybe<std::unique_ptr<ast::IfStatement>> if_stmt();
  /// Parses a `elseif_stmt` grammar element
  /// @returns the parsed elements
  Maybe<ast::ElseStatementList> elseif_stmt();
  /// Parses a `else_stmt` grammar element
  /// @returns the parsed statement or nullptr
  Maybe<std::unique_ptr<ast::ElseStatement>> else_stmt();
  /// Parses a `switch_stmt` grammar element
  /// @returns the parsed statement or nullptr
  Maybe<std::unique_ptr<ast::SwitchStatement>> switch_stmt();
  /// Parses a `switch_body` grammar element
  /// @returns the parsed statement or nullptr
  Maybe<std::unique_ptr<ast::CaseStatement>> switch_body();
  /// Parses a `case_selectors` grammar element
  /// @returns the list of literals
  Expect<ast::CaseSelectorList> expect_case_selectors();
  /// Parses a `case_body` grammar element
  /// @returns the parsed statements
  Maybe<std::unique_ptr<ast::BlockStatement>> case_body();
  /// Parses a `func_call_stmt` grammar element
  /// @returns the parsed function call or nullptr
  Maybe<std::unique_ptr<ast::CallStatement>> func_call_stmt();
  /// Parses a `loop_stmt` grammar element
  /// @returns the parsed loop or nullptr
  Maybe<std::unique_ptr<ast::LoopStatement>> loop_stmt();
  /// Parses a `for_header` grammar element, erroring on parse failure.
  /// @returns the parsed for header or nullptr
  Expect<std::unique_ptr<ForHeader>> expect_for_header();
  /// Parses a `for_stmt` grammar element
  /// @returns the parsed for loop or nullptr
  Maybe<std::unique_ptr<ast::Statement>> for_stmt();
  /// Parses a `continuing_stmt` grammar element
  /// @returns the parsed statements
  Maybe<std::unique_ptr<ast::BlockStatement>> continuing_stmt();
  /// Parses a `const_literal` grammar element
  /// @returns the const literal parsed or nullptr if none found
  Maybe<std::unique_ptr<ast::Literal>> const_literal();
  /// Parses a `const_expr` grammar element, erroring on parse failure.
  /// @returns the parsed constructor expression or nullptr on error
  Expect<std::unique_ptr<ast::ConstructorExpression>> expect_const_expr();
  /// Parses a `primary_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<std::unique_ptr<ast::Expression>> primary_expression();
  /// Parses a `argument_expression_list` grammar element, erroring on parse
  /// failure.
  /// @returns the list of arguments
  Expect<ast::ExpressionList> expect_argument_expression_list();
  /// Parses the recursive portion of the postfix_expression
  /// @param prefix the left side of the expression
  /// @returns the parsed expression or nullptr
  Maybe<std::unique_ptr<ast::Expression>> postfix_expr(
      std::unique_ptr<ast::Expression> prefix);
  /// Parses a `postfix_expression` grammar elment
  /// @returns the parsed expression or nullptr
  Maybe<std::unique_ptr<ast::Expression>> postfix_expression();
  /// Parses a `unary_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<std::unique_ptr<ast::Expression>> unary_expression();
  /// Parses the recursive part of the `multiplicative_expression`, erroring on
  /// parse failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<std::unique_ptr<ast::Expression>> expect_multiplicative_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses the `multiplicative_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<std::unique_ptr<ast::Expression>> multiplicative_expression();
  /// Parses the recursive part of the `additive_expression`, erroring on parse
  /// failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<std::unique_ptr<ast::Expression>> expect_additive_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses the `additive_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<std::unique_ptr<ast::Expression>> additive_expression();
  /// Parses the recursive part of the `shift_expression`, erroring on parse
  /// failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<std::unique_ptr<ast::Expression>> expect_shift_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses the `shift_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<std::unique_ptr<ast::Expression>> shift_expression();
  /// Parses the recursive part of the `relational_expression`, erroring on
  /// parse failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<std::unique_ptr<ast::Expression>> expect_relational_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses the `relational_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<std::unique_ptr<ast::Expression>> relational_expression();
  /// Parses the recursive part of the `equality_expression`, erroring on parse
  /// failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<std::unique_ptr<ast::Expression>> expect_equality_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses the `equality_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<std::unique_ptr<ast::Expression>> equality_expression();
  /// Parses the recursive part of the `and_expression`, erroring on parse
  /// failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<std::unique_ptr<ast::Expression>> expect_and_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses the `and_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<std::unique_ptr<ast::Expression>> and_expression();
  /// Parses the recursive part of the `exclusive_or_expression`, erroring on
  /// parse failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<std::unique_ptr<ast::Expression>> expect_exclusive_or_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses the `exclusive_or_expression` grammar elememnt
  /// @returns the parsed expression or nullptr
  Maybe<std::unique_ptr<ast::Expression>> exclusive_or_expression();
  /// Parses the recursive part of the `inclusive_or_expression`, erroring on
  /// parse failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<std::unique_ptr<ast::Expression>> expect_inclusive_or_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses the `inclusive_or_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<std::unique_ptr<ast::Expression>> inclusive_or_expression();
  /// Parses the recursive part of the `logical_and_expression`, erroring on
  /// parse failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<std::unique_ptr<ast::Expression>> expect_logical_and_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses a `logical_and_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<std::unique_ptr<ast::Expression>> logical_and_expression();
  /// Parses the recursive part of the `logical_or_expression`, erroring on
  /// parse failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<std::unique_ptr<ast::Expression>> expect_logical_or_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses a `logical_or_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<std::unique_ptr<ast::Expression>> logical_or_expression();
  /// Parses a `assignment_stmt` grammar element
  /// @returns the parsed assignment or nullptr
  Maybe<std::unique_ptr<ast::AssignmentStatement>> assignment_stmt();
  /// Parses one or more bracketed decoration lists.
  /// @return the parsed decoration list, or an empty list on error.
  Maybe<ast::DecorationList> decoration_list();
  /// Parses a list of decorations between `ATTR_LEFT` and `ATTR_RIGHT`
  /// brackets.
  /// @param decos the list to append newly parsed decorations to.
  /// @return true if any decorations were be parsed, otherwise false.
  Maybe<bool> decoration_bracketed_list(ast::DecorationList& decos);
  /// Parses a single decoration of the following types:
  /// * `struct_decoration`
  /// * `struct_member_decoration`
  /// * `array_decoration`
  /// * `variable_decoration`
  /// * `global_const_decoration`
  /// * `function_decoration`
  /// @return the parsed decoration, or nullptr.
  Maybe<std::unique_ptr<ast::Decoration>> decoration();
  /// Parses a single decoration, reporting an error if the next token does not
  /// represent a decoration.
  /// @see #decoration for the full list of decorations this method parses.
  /// @return the parsed decoration, or nullptr on error.
  Expect<std::unique_ptr<ast::Decoration>> expect_decoration();

 private:
  /// ReturnType resolves to the return type for the function or lambda F.
  template <typename F>
  using ReturnType = typename std::result_of<F()>::type;

  /// ResultType resolves to |T| for a |RESULT| of type Expect<T>.
  template <typename RESULT>
  using ResultType = typename RESULT::type;

  /// @returns true and consumes the next token if it equals |tok|.
  /// @param source if not nullptr, the next token's source is written to this
  /// pointer, regardless of success or error
  bool match(Token::Type tok, Source* source = nullptr);
  /// Errors if the next token is not equal to |tok|.
  /// Always consumes the next token.
  /// @param use a description of what was being parsed if an error was raised.
  /// @param tok the token to test against
  /// @returns true if the next token equals |tok|.
  bool expect(const std::string& use, Token::Type tok);
  /// Parses a signed integer from the next token in the stream, erroring if the
  /// next token is not a signed integer.
  /// Always consumes the next token.
  /// @param use a description of what was being parsed if an error was raised
  /// @returns the parsed integer.
  Expect<int32_t> expect_sint(const std::string& use);
  /// Parses a signed integer from the next token in the stream, erroring if
  /// the next token is not a signed integer or is negative.
  /// Always consumes the next token.
  /// @param use a description of what was being parsed if an error was raised
  /// @returns the parsed integer.
  Expect<uint32_t> expect_positive_sint(const std::string& use);
  /// Parses a non-zero signed integer from the next token in the stream,
  /// erroring if the next token is not a signed integer or is less than 1.
  /// Always consumes the next token.
  /// @param use a description of what was being parsed if an error was raised
  /// @returns the parsed integer.
  Expect<uint32_t> expect_nonzero_positive_sint(const std::string& use);
  /// Errors if the next token is not an identifier.
  /// Always consumes the next token.
  /// @param use a description of what was being parsed if an error was raised
  /// @returns the parsed identifier.
  Expect<std::string> expect_ident(const std::string& use);
  /// Parses a lexical block starting with the token |start| and ending with
  /// the token |end|. |body| is called to parse the lexical block body between
  /// the |start| and |end| tokens.
  /// If the |start| or |end| tokens are not matched then an error is generated
  /// and a zero-initialized |T| is returned.
  /// If |body| raises an error while parsing then a zero-initialized |T| is
  /// returned.
  /// @param start the token that begins the lexical block
  /// @param end the token that ends the lexical block
  /// @param use a description of what was being parsed if an error was raised
  /// @param body a function or lambda that is called to parse the lexical block
  /// body, with the signature: `Expect<Result>()`.
  /// @return the value returned by |body| if no errors are raised, otherwise
  /// an Expect with error state.
  template <typename F, typename T = ReturnType<F>>
  T expect_block(Token::Type start,
                 Token::Type end,
                 const std::string& use,
                 F&& body);
  /// A convenience function that calls |expect_block| passing
  /// |Token::Type::kParenLeft| and |Token::Type::kParenRight| for the |start|
  /// and |end| arguments, respectively.
  /// @param use a description of what was being parsed if an error was raised
  /// @param body a function or lambda that is called to parse the lexical block
  /// body, with the signature: `Expect<Result>()`.
  /// @return the value returned by |body| if no errors are raised, otherwise
  /// an Expect with error state.
  template <typename F, typename T = ReturnType<F>>
  T expect_paren_block(const std::string& use, F&& body);
  /// A convenience function that calls |expect_block| passing
  /// |Token::Type::kBraceLeft| and |Token::Type::kBraceRight| for the |start|
  /// and |end| arguments, respectively.
  /// @param use a description of what was being parsed if an error was raised
  /// @param body a function or lambda that is called to parse the lexical block
  /// body, with the signature: `Expect<Result>()`.
  /// @return the value returned by |body| if no errors are raised, otherwise
  /// an Expect with error state.
  template <typename F, typename T = ReturnType<F>>
  T expect_brace_block(const std::string& use, F&& body);

  /// Downcasts all the decorations in |list| to the type |T|, raising a parser
  /// error if any of the decorations aren't of the type |T|.
  template <typename T>
  Expect<std::vector<std::unique_ptr<T>>> cast_decorations(
      ast::DecorationList& in);
  /// Reports an error if the decoration list |list| is not empty.
  /// Used to ensure that all decorations are consumed.
  bool expect_decorations_consumed(const ast::DecorationList& list);

  Expect<ast::type::Type*> expect_type_decl_pointer();
  Expect<ast::type::Type*> expect_type_decl_vector(Token t);
  Expect<ast::type::Type*> expect_type_decl_array(
      ast::ArrayDecorationList decos);
  Expect<ast::type::Type*> expect_type_decl_matrix(Token t);

  Expect<std::unique_ptr<ast::ConstructorExpression>>
  expect_const_expr_internal(uint32_t depth);

  Maybe<std::unique_ptr<ast::Statement>> for_header_initializer();
  Maybe<std::unique_ptr<ast::Statement>> for_header_continuing();

  Context& ctx_;
  diag::List diags_;
  std::unique_ptr<Lexer> lexer_;
  std::deque<Token> token_queue_;
  std::unordered_map<std::string, ast::type::Type*> registered_constructs_;
  ast::Module module_;
};

}  // namespace wgsl
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_WGSL_PARSER_IMPL_H_
