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

#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/ast/access_control.h"
#include "src/program_builder.h"
#include "src/reader/wgsl/parser_impl_detail.h"
#include "src/reader/wgsl/token.h"
#include "src/sem/storage_texture_type.h"

namespace tint {
namespace ast {
class AssignmentStatement;
class BreakStatement;
class CallStatement;
class ContinueStatement;
class IfStatement;
class LoopStatement;
class ReturnStatement;
class SwitchStatement;
class VariableDeclStatement;
}  // namespace ast

namespace reader {
namespace wgsl {

class Lexer;

/// Struct holding information for a for loop
struct ForHeader {
  /// Constructor
  /// @param init the initializer statement
  /// @param cond the condition statement
  /// @param cont the continuing statement
  ForHeader(ast::Statement* init, ast::Expression* cond, ast::Statement* cont);

  ~ForHeader();

  /// The for loop initializer
  ast::Statement* initializer = nullptr;
  /// The for loop condition
  ast::Expression* condition = nullptr;
  /// The for loop continuing statement
  ast::Statement* continuing = nullptr;
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
  /// add_error() and #errored will be set to true.
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
    /// return type will always be a pointer to a non-pointer type. #errored
    /// must be false to call.
    inline typename detail::OperatorArrow<T>::type operator->() {
      TINT_ASSERT(!errored);
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
  /// In the case of a successful grammar match, the Maybe will have #matched
  /// set to true.
  /// In the case of a parse error the called method will have called
  /// add_error() and the Maybe will have #errored set to true.
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
    /// return type will always be a pointer to a non-pointer type. #errored
    /// must be false to call.
    inline typename detail::OperatorArrow<T>::type operator->() {
      TINT_ASSERT(!errored);
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
    /// Constructor
    TypedIdentifier();
    /// Copy constructor
    /// @param other the FunctionHeader to copy
    TypedIdentifier(const TypedIdentifier& other);
    /// Constructor
    /// @param type_in parsed type
    /// @param name_in parsed identifier
    /// @param source_in source to the identifier
    TypedIdentifier(ast::Type* type_in, std::string name_in, Source source_in);
    /// Destructor
    ~TypedIdentifier();

    /// Parsed type.
    ast::Type* type;
    /// Parsed identifier.
    std::string name;
    /// Source to the identifier.
    Source source;
  };

  /// FunctionHeader contains the parsed information for a function header.
  struct FunctionHeader {
    /// Constructor
    FunctionHeader();
    /// Copy constructor
    /// @param other the FunctionHeader to copy
    FunctionHeader(const FunctionHeader& other);
    /// Constructor
    /// @param src parsed header source
    /// @param n function name
    /// @param p function parameters
    /// @param ret_ty function return type
    /// @param ret_decos return type decorations
    FunctionHeader(Source src,
                   std::string n,
                   ast::VariableList p,
                   ast::Type* ret_ty,
                   ast::DecorationList ret_decos);
    /// Destructor
    ~FunctionHeader();
    /// Assignment operator
    /// @param other the FunctionHeader to copy
    /// @returns this FunctionHeader
    FunctionHeader& operator=(const FunctionHeader& other);

    /// Parsed header source
    Source source;
    /// Function name
    std::string name;
    /// Function parameters
    ast::VariableList params;
    /// Function return type
    ast::Type* return_type;
    /// Function return type decorations
    ast::DecorationList return_type_decorations;
  };

  /// VarDeclInfo contains the parsed information for variable declaration.
  struct VarDeclInfo {
    /// Constructor
    VarDeclInfo();
    /// Copy constructor
    /// @param other the VarDeclInfo to copy
    VarDeclInfo(const VarDeclInfo& other);
    /// Constructor
    /// @param source_in variable declaration source
    /// @param name_in variable name
    /// @param storage_class_in variable storage class
    /// @param type_in variable type
    VarDeclInfo(Source source_in,
                std::string name_in,
                ast::StorageClass storage_class_in,
                ast::Type* type_in);
    /// Destructor
    ~VarDeclInfo();

    /// Variable declaration source
    Source source;
    /// Variable name
    std::string name;
    /// Variable storage class
    ast::StorageClass storage_class;
    /// Variable type
    ast::Type* type = nullptr;
  };

  /// Creates a new parser using the given file
  /// @param file the input source file to parse
  explicit ParserImpl(Source::File const* file);
  ~ParserImpl();

  /// Run the parser
  /// @returns true if the parse was successful, false otherwise.
  bool Parse();

  /// set_max_diagnostics sets the maximum number of reported errors before
  /// aborting parsing.
  /// @param limit the new maximum number of errors
  void set_max_errors(size_t limit) { max_errors_ = limit; }

  /// @return the number of maximum number of reported errors before aborting
  /// parsing.
  size_t get_max_errors() const { return max_errors_; }

  /// @returns true if an error was encountered.
  bool has_error() const { return builder_.Diagnostics().contains_errors(); }

  /// @returns the parser error string
  std::string error() const {
    diag::Formatter formatter{{false, false, false, false}};
    return formatter.format(builder_.Diagnostics());
  }

  /// @returns the Program. The program builder in the parser will be reset
  /// after this.
  Program program() { return Program(std::move(builder_)); }

  /// @returns the program builder.
  ProgramBuilder& builder() { return builder_; }

  /// @returns the next token
  Token next();
  /// @returns the next token without advancing
  Token peek();
  /// Peeks ahead and returns the token at `idx` head of the current position
  /// @param idx the index of the token to return
  /// @returns the token `idx` positions ahead without advancing
  Token peek(size_t idx);
  /// @returns the last token that was returned by `next()`
  Token last_token() const;
  /// Appends an error at `t` with the message `msg`
  /// @param t the token to associate the error with
  /// @param msg the error message
  /// @return `Failure::Errored::kError` so that you can combine an add_error()
  /// call and return on the same line.
  Failure::Errored add_error(const Token& t, const std::string& msg);
  /// Appends an error raised when parsing `use` at `t` with the message
  /// `msg`
  /// @param source the source to associate the error with
  /// @param msg the error message
  /// @param use a description of what was being parsed when the error was
  /// raised.
  /// @return `Failure::Errored::kError` so that you can combine an add_error()
  /// call and return on the same line.
  Failure::Errored add_error(const Source& source,
                             const std::string& msg,
                             const std::string& use);
  /// Appends an error at `source` with the message `msg`
  /// @param source the source to associate the error with
  /// @param msg the error message
  /// @return `Failure::Errored::kError` so that you can combine an add_error()
  /// call and return on the same line.
  Failure::Errored add_error(const Source& source, const std::string& msg);
  /// Appends a deprecated-language-feature warning at `source` with the message
  /// `msg`
  /// @param source the source to associate the error with
  /// @param msg the warning message
  void deprecated(const Source& source, const std::string& msg);
  /// Registers a constructed type into the parser
  /// TODO(crbug.com/tint/724): Remove
  /// @param name the constructed name
  /// @param type the constructed type
  void register_constructed(const std::string& name, const ast::Type* type);
  /// Retrieves a constructed type
  /// TODO(crbug.com/tint/724): Remove
  /// @param name The name to lookup
  /// @returns the constructed type for `name` or `nullptr` if not found
  const ast::Type* get_constructed(const std::string& name);

  /// Parses the `translation_unit` grammar element
  void translation_unit();
  /// Parses the `global_decl` grammar element, erroring on parse failure.
  /// @return true on parse success, otherwise an error.
  Expect<bool> expect_global_decl();
  /// Parses a `global_variable_decl` grammar element with the initial
  /// `variable_decoration_list*` provided as `decos`
  /// @returns the variable parsed or nullptr
  /// @param decos the list of decorations for the variable declaration.
  Maybe<ast::Variable*> global_variable_decl(ast::DecorationList& decos);
  /// Parses a `global_constant_decl` grammar element with the initial
  /// `variable_decoration_list*` provided as `decos`
  /// @returns the const object or nullptr
  /// @param decos the list of decorations for the constant declaration.
  Maybe<ast::Variable*> global_constant_decl(ast::DecorationList& decos);
  /// Parses a `variable_decl` grammar element
  /// @returns the parsed variable declaration info
  Maybe<VarDeclInfo> variable_decl();
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
  Maybe<ast::Alias*> type_alias();
  /// Parses a `type_decl` grammar element
  /// @returns the parsed Type or nullptr if none matched.
  Maybe<ast::Type*> type_decl();
  /// Parses a `type_decl` grammar element with the given pre-parsed
  /// decorations.
  /// @param decos the list of decorations for the type.
  /// @returns the parsed Type or nullptr if none matched.
  Maybe<ast::Type*> type_decl(ast::DecorationList& decos);
  /// Parses a `storage_class` grammar element, erroring on parse failure.
  /// @param use a description of what was being parsed if an error was raised.
  /// @returns the storage class or StorageClass::kNone if none matched
  Expect<ast::StorageClass> expect_storage_class(const std::string& use);
  /// Parses a `struct_decl` grammar element with the initial
  /// `struct_decoration_decl*` provided as `decos`.
  /// @returns the struct type or nullptr on error
  /// @param decos the list of decorations for the struct declaration.
  Maybe<ast::Struct*> struct_decl(ast::DecorationList& decos);
  /// Parses a `struct_body_decl` grammar element, erroring on parse failure.
  /// @returns the struct members
  Expect<ast::StructMemberList> expect_struct_body_decl();
  /// Parses a `struct_member` grammar element with the initial
  /// `struct_member_decoration_decl+` provided as `decos`, erroring on parse
  /// failure.
  /// @param decos the list of decorations for the struct member.
  /// @returns the struct member or nullptr
  Expect<ast::StructMember*> expect_struct_member(ast::DecorationList& decos);
  /// Parses a `function_decl` grammar element with the initial
  /// `function_decoration_decl*` provided as `decos`.
  /// @param decos the list of decorations for the function declaration.
  /// @returns the parsed function, nullptr otherwise
  Maybe<ast::Function*> function_decl(ast::DecorationList& decos);
  /// Parses a `texture_sampler_types` grammar element
  /// @returns the parsed Type or nullptr if none matched.
  Maybe<ast::Type*> texture_sampler_types();
  /// Parses a `sampler_type` grammar element
  /// @returns the parsed Type or nullptr if none matched.
  Maybe<ast::Type*> sampler_type();
  /// Parses a `multisampled_texture_type` grammar element
  /// @returns returns the multisample texture dimension or kNone if none
  /// matched.
  Maybe<ast::TextureDimension> multisampled_texture_type();
  /// Parses a `sampled_texture_type` grammar element
  /// @returns returns the sample texture dimension or kNone if none matched.
  Maybe<ast::TextureDimension> sampled_texture_type();
  /// Parses a `storage_texture_type` grammar element
  /// @returns returns the storage texture dimension.
  /// Returns kNone if none matched.
  Maybe<ast::TextureDimension> storage_texture_type();
  /// Parses a `depth_texture_type` grammar element
  /// @returns the parsed Type or nullptr if none matched.
  Maybe<ast::Type*> depth_texture_type();
  /// Parses a 'texture_external_type' grammar element
  /// @returns the parsed Type or nullptr if none matched
  Maybe<ast::Type*> external_texture_type();
  /// Parses a `image_storage_type` grammar element
  /// @param use a description of what was being parsed if an error was raised
  /// @returns returns the image format or kNone if none matched.
  Expect<ast::ImageFormat> expect_image_storage_type(const std::string& use);
  /// Parses a `function_type_decl` grammar element
  /// @returns the parsed type or nullptr otherwise
  Maybe<ast::Type*> function_type_decl();
  /// Parses a `function_header` grammar element
  /// @returns the parsed function header
  Maybe<FunctionHeader> function_header();
  /// Parses a `param_list` grammar element, erroring on parse failure.
  /// @returns the parsed variables
  Expect<ast::VariableList> expect_param_list();
  /// Parses a `param` grammar element, erroring on parse failure.
  /// @returns the parsed variable
  Expect<ast::Variable*> expect_param();
  /// Parses a `pipeline_stage` grammar element, erroring if the next token does
  /// not match a stage name.
  /// @returns the pipeline stage.
  Expect<ast::PipelineStage> expect_pipeline_stage();
  /// Parses an access type identifier, erroring if the next token does not
  /// match a valid access type name.
  /// @returns the parsed access control.
  Expect<ast::AccessControl::Access> expect_access_type();
  /// Parses a builtin identifier, erroring if the next token does not match a
  /// valid builtin name.
  /// @returns the parsed builtin.
  Expect<ast::Builtin> expect_builtin();
  /// Parses a `body_stmt` grammar element, erroring on parse failure.
  /// @returns the parsed statements
  Expect<ast::BlockStatement*> expect_body_stmt();
  /// Parses a `paren_rhs_stmt` grammar element, erroring on parse failure.
  /// @returns the parsed element or nullptr
  Expect<ast::Expression*> expect_paren_rhs_stmt();
  /// Parses a `statements` grammar element
  /// @returns the statements parsed
  Expect<ast::StatementList> expect_statements();
  /// Parses a `statement` grammar element
  /// @returns the parsed statement or nullptr
  Maybe<ast::Statement*> statement();
  /// Parses a `break_stmt` grammar element
  /// @returns the parsed statement or nullptr
  Maybe<ast::BreakStatement*> break_stmt();
  /// Parses a `return_stmt` grammar element
  /// @returns the parsed statement or nullptr
  Maybe<ast::ReturnStatement*> return_stmt();
  /// Parses a `continue_stmt` grammar element
  /// @returns the parsed statement or nullptr
  Maybe<ast::ContinueStatement*> continue_stmt();
  /// Parses a `variable_stmt` grammar element
  /// @returns the parsed variable or nullptr
  Maybe<ast::VariableDeclStatement*> variable_stmt();
  /// Parses a `if_stmt` grammar element
  /// @returns the parsed statement or nullptr
  Maybe<ast::IfStatement*> if_stmt();
  /// Parses a `elseif_stmt` grammar element
  /// @returns the parsed elements
  Maybe<ast::ElseStatementList> elseif_stmt();
  /// Parses a `else_stmt` grammar element
  /// @returns the parsed statement or nullptr
  Maybe<ast::ElseStatement*> else_stmt();
  /// Parses a `switch_stmt` grammar element
  /// @returns the parsed statement or nullptr
  Maybe<ast::SwitchStatement*> switch_stmt();
  /// Parses a `switch_body` grammar element
  /// @returns the parsed statement or nullptr
  Maybe<ast::CaseStatement*> switch_body();
  /// Parses a `case_selectors` grammar element
  /// @returns the list of literals
  Expect<ast::CaseSelectorList> expect_case_selectors();
  /// Parses a `case_body` grammar element
  /// @returns the parsed statements
  Maybe<ast::BlockStatement*> case_body();
  /// Parses a `func_call_stmt` grammar element
  /// @returns the parsed function call or nullptr
  Maybe<ast::CallStatement*> func_call_stmt();
  /// Parses a `loop_stmt` grammar element
  /// @returns the parsed loop or nullptr
  Maybe<ast::LoopStatement*> loop_stmt();
  /// Parses a `for_header` grammar element, erroring on parse failure.
  /// @returns the parsed for header or nullptr
  Expect<std::unique_ptr<ForHeader>> expect_for_header();
  /// Parses a `for_stmt` grammar element
  /// @returns the parsed for loop or nullptr
  Maybe<ast::Statement*> for_stmt();
  /// Parses a `continuing_stmt` grammar element
  /// @returns the parsed statements
  Maybe<ast::BlockStatement*> continuing_stmt();
  /// Parses a `const_literal` grammar element
  /// @returns the const literal parsed or nullptr if none found
  Maybe<ast::Literal*> const_literal();
  /// Parses a `const_expr` grammar element, erroring on parse failure.
  /// @returns the parsed constructor expression or nullptr on error
  Expect<ast::ConstructorExpression*> expect_const_expr();
  /// Parses a `primary_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<ast::Expression*> primary_expression();
  /// Parses a `argument_expression_list` grammar element, erroring on parse
  /// failure.
  /// @param use a description of what was being parsed if an error was raised
  /// @returns the list of arguments
  Expect<ast::ExpressionList> expect_argument_expression_list(
      const std::string& use);
  /// Parses the recursive portion of the postfix_expression
  /// @param prefix the left side of the expression
  /// @returns the parsed expression or nullptr
  Maybe<ast::Expression*> postfix_expression(ast::Expression* prefix);
  /// Parses a `singular_expression` grammar elment
  /// @returns the parsed expression or nullptr
  Maybe<ast::Expression*> singular_expression();
  /// Parses a `unary_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<ast::Expression*> unary_expression();
  /// Parses the recursive part of the `multiplicative_expression`, erroring on
  /// parse failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<ast::Expression*> expect_multiplicative_expr(ast::Expression* lhs);
  /// Parses the `multiplicative_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<ast::Expression*> multiplicative_expression();
  /// Parses the recursive part of the `additive_expression`, erroring on parse
  /// failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<ast::Expression*> expect_additive_expr(ast::Expression* lhs);
  /// Parses the `additive_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<ast::Expression*> additive_expression();
  /// Parses the recursive part of the `shift_expression`, erroring on parse
  /// failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<ast::Expression*> expect_shift_expr(ast::Expression* lhs);
  /// Parses the `shift_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<ast::Expression*> shift_expression();
  /// Parses the recursive part of the `relational_expression`, erroring on
  /// parse failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<ast::Expression*> expect_relational_expr(ast::Expression* lhs);
  /// Parses the `relational_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<ast::Expression*> relational_expression();
  /// Parses the recursive part of the `equality_expression`, erroring on parse
  /// failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<ast::Expression*> expect_equality_expr(ast::Expression* lhs);
  /// Parses the `equality_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<ast::Expression*> equality_expression();
  /// Parses the recursive part of the `and_expression`, erroring on parse
  /// failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<ast::Expression*> expect_and_expr(ast::Expression* lhs);
  /// Parses the `and_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<ast::Expression*> and_expression();
  /// Parses the recursive part of the `exclusive_or_expression`, erroring on
  /// parse failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<ast::Expression*> expect_exclusive_or_expr(ast::Expression* lhs);
  /// Parses the `exclusive_or_expression` grammar elememnt
  /// @returns the parsed expression or nullptr
  Maybe<ast::Expression*> exclusive_or_expression();
  /// Parses the recursive part of the `inclusive_or_expression`, erroring on
  /// parse failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<ast::Expression*> expect_inclusive_or_expr(ast::Expression* lhs);
  /// Parses the `inclusive_or_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<ast::Expression*> inclusive_or_expression();
  /// Parses the recursive part of the `logical_and_expression`, erroring on
  /// parse failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<ast::Expression*> expect_logical_and_expr(ast::Expression* lhs);
  /// Parses a `logical_and_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<ast::Expression*> logical_and_expression();
  /// Parses the recursive part of the `logical_or_expression`, erroring on
  /// parse failure.
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  Expect<ast::Expression*> expect_logical_or_expr(ast::Expression* lhs);
  /// Parses a `logical_or_expression` grammar element
  /// @returns the parsed expression or nullptr
  Maybe<ast::Expression*> logical_or_expression();
  /// Parses a `assignment_stmt` grammar element
  /// @returns the parsed assignment or nullptr
  Maybe<ast::AssignmentStatement*> assignment_stmt();
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
  Maybe<ast::Decoration*> decoration();
  /// Parses a single decoration, reporting an error if the next token does not
  /// represent a decoration.
  /// @see #decoration for the full list of decorations this method parses.
  /// @return the parsed decoration, or nullptr on error.
  Expect<ast::Decoration*> expect_decoration();

 private:
  /// ReturnType resolves to the return type for the function or lambda F.
  template <typename F>
  using ReturnType = typename std::result_of<F()>::type;

  /// ResultType resolves to `T` for a `RESULT` of type Expect<T>.
  template <typename RESULT>
  using ResultType = typename RESULT::type;

  /// @returns true and consumes the next token if it equals `tok`
  /// @param source if not nullptr, the next token's source is written to this
  /// pointer, regardless of success or error
  bool match(Token::Type tok, Source* source = nullptr);
  /// Errors if the next token is not equal to `tok`
  /// Consumes the next token on match.
  /// expect() also updates #synchronized_, setting it to `true` if the next
  /// token is equal to `tok`, otherwise `false`.
  /// @param use a description of what was being parsed if an error was raised.
  /// @param tok the token to test against
  /// @returns true if the next token equals `tok`
  bool expect(const std::string& use, Token::Type tok);
  /// Parses a signed integer from the next token in the stream, erroring if the
  /// next token is not a signed integer.
  /// Consumes the next token on match.
  /// @param use a description of what was being parsed if an error was raised
  /// @returns the parsed integer.
  Expect<int32_t> expect_sint(const std::string& use);
  /// Parses a signed integer from the next token in the stream, erroring if
  /// the next token is not a signed integer or is negative.
  /// Consumes the next token if it is a signed integer (not necessarily
  /// negative).
  /// @param use a description of what was being parsed if an error was raised
  /// @returns the parsed integer.
  Expect<uint32_t> expect_positive_sint(const std::string& use);
  /// Parses a non-zero signed integer from the next token in the stream,
  /// erroring if the next token is not a signed integer or is less than 1.
  /// Consumes the next token if it is a signed integer (not necessarily
  /// >= 1).
  /// @param use a description of what was being parsed if an error was raised
  /// @returns the parsed integer.
  Expect<uint32_t> expect_nonzero_positive_sint(const std::string& use);
  /// Errors if the next token is not an identifier.
  /// Consumes the next token on match.
  /// @param use a description of what was being parsed if an error was raised
  /// @returns the parsed identifier.
  Expect<std::string> expect_ident(const std::string& use);
  /// Parses a lexical block starting with the token `start` and ending with
  /// the token `end`. `body` is called to parse the lexical block body
  /// between the `start` and `end` tokens. If the `start` or `end` tokens
  /// are not matched then an error is generated and a zero-initialized `T` is
  /// returned. If `body` raises an error while parsing then a zero-initialized
  /// `T` is returned.
  /// @param start the token that begins the lexical block
  /// @param end the token that ends the lexical block
  /// @param use a description of what was being parsed if an error was raised
  /// @param body a function or lambda that is called to parse the lexical block
  /// body, with the signature: `Expect<Result>()` or `Maybe<Result>()`.
  /// @return the value returned by `body` if no errors are raised, otherwise
  /// an Expect with error state.
  template <typename F, typename T = ReturnType<F>>
  T expect_block(Token::Type start,
                 Token::Type end,
                 const std::string& use,
                 F&& body);
  /// A convenience function that calls expect_block() passing
  /// `Token::Type::kParenLeft` and `Token::Type::kParenRight` for the `start`
  /// and `end` arguments, respectively.
  /// @param use a description of what was being parsed if an error was raised
  /// @param body a function or lambda that is called to parse the lexical block
  /// body, with the signature: `Expect<Result>()` or `Maybe<Result>()`.
  /// @return the value returned by `body` if no errors are raised, otherwise
  /// an Expect with error state.
  template <typename F, typename T = ReturnType<F>>
  T expect_paren_block(const std::string& use, F&& body);
  /// A convenience function that calls `expect_block` passing
  /// `Token::Type::kBraceLeft` and `Token::Type::kBraceRight` for the `start`
  /// and `end` arguments, respectively.
  /// @param use a description of what was being parsed if an error was raised
  /// @param body a function or lambda that is called to parse the lexical block
  /// body, with the signature: `Expect<Result>()` or `Maybe<Result>()`.
  /// @return the value returned by `body` if no errors are raised, otherwise
  /// an Expect with error state.
  template <typename F, typename T = ReturnType<F>>
  T expect_brace_block(const std::string& use, F&& body);
  /// A convenience function that calls `expect_block` passing
  /// `Token::Type::kLessThan` and `Token::Type::kGreaterThan` for the `start`
  /// and `end` arguments, respectively.
  /// @param use a description of what was being parsed if an error was raised
  /// @param body a function or lambda that is called to parse the lexical block
  /// body, with the signature: `Expect<Result>()` or `Maybe<Result>()`.
  /// @return the value returned by `body` if no errors are raised, otherwise
  /// an Expect with error state.
  template <typename F, typename T = ReturnType<F>>
  T expect_lt_gt_block(const std::string& use, F&& body);

  /// sync() calls the function `func`, and attempts to resynchronize the
  /// parser to the next found resynchronization token if `func` fails. If the
  /// next found resynchronization token is `tok`, then sync will also consume
  /// `tok`.
  ///
  /// sync() will transiently add `tok` to the parser's stack of
  /// synchronization tokens for the duration of the call to `func`. Once @p
  /// func returns,
  /// `tok` is removed from the stack of resynchronization tokens. sync calls
  /// may be nested, and so the number of resynchronization tokens is equal to
  /// the number of sync() calls in the current stack frame.
  ///
  /// sync() updates #synchronized_, setting it to `true` if the next
  /// resynchronization token found was `tok`, otherwise `false`.
  ///
  /// @param tok the token to attempt to synchronize the parser to if `func`
  /// fails.
  /// @param func a function or lambda with the signature: `Expect<Result>()` or
  /// `Maybe<Result>()`.
  /// @return the value returned by `func`
  template <typename F, typename T = ReturnType<F>>
  T sync(Token::Type tok, F&& func);
  /// sync_to() attempts to resynchronize the parser to the next found
  /// resynchronization token or `tok` (whichever comes first).
  ///
  /// Synchronization tokens are transiently defined by calls to sync().
  ///
  /// sync_to() updates #synchronized_, setting it to `true` if a
  /// resynchronization token was found and it was `tok`, otherwise `false`.
  ///
  /// @param tok the token to attempt to synchronize the parser to.
  /// @param consume if true and the next found resynchronization token is
  /// `tok` then sync_to() will also consume `tok`.
  /// @return the state of #synchronized_.
  /// @see sync().
  bool sync_to(Token::Type tok, bool consume);
  /// @return true if `t` is in the stack of resynchronization tokens.
  /// @see sync().
  bool is_sync_token(const Token& t) const;

  /// without_error() calls the function `func` muting any grammatical errors
  /// found while executing the function. This can be used as a best-effort to
  /// produce a meaningful error message when the parser is out of sync.
  /// @param func a function or lambda with the signature: `Expect<Result>()` or
  /// `Maybe<Result>()`.
  /// @return the value returned by `func`
  template <typename F, typename T = ReturnType<F>>
  T without_error(F&& func);

  /// Returns all the decorations taken from `list` that matches the type `T`.
  /// Those that do not match are kept in `list`.
  template <typename T>
  std::vector<T*> take_decorations(ast::DecorationList& list);

  /// Reports an error if the decoration list `list` is not empty.
  /// Used to ensure that all decorations are consumed.
  bool expect_decorations_consumed(const ast::DecorationList& list);

  Expect<ast::Type*> expect_type_decl_pointer(Token t);
  Expect<ast::Type*> expect_type_decl_vector(Token t);
  Expect<ast::Type*> expect_type_decl_array(Token t, ast::DecorationList decos);
  Expect<ast::Type*> expect_type_decl_matrix(Token t);

  Expect<ast::Type*> expect_type(const std::string& use);

  Maybe<ast::Statement*> non_block_statement();
  Maybe<ast::Statement*> for_header_initializer();
  Maybe<ast::Statement*> for_header_continuing();

  class MultiTokenSource;
  MultiTokenSource make_source_range();
  MultiTokenSource make_source_range_from(const Source& start);

  /// Creates a new `ast::Node` owned by the Module. When the Module is
  /// destructed, the `ast::Node` will also be destructed.
  /// @param args the arguments to pass to the type constructor
  /// @returns the node pointer
  template <typename T, typename... ARGS>
  T* create(ARGS&&... args) {
    return builder_.create<T>(std::forward<ARGS>(args)...);
  }

  std::unique_ptr<Lexer> lexer_;
  std::deque<Token> token_queue_;
  Token last_token_;
  bool synchronized_ = true;
  uint32_t sync_depth_ = 0;
  std::vector<Token::Type> sync_tokens_;
  int silence_errors_ = 0;
  std::unordered_map<std::string, const ast::Type*> registered_constructs_;
  ProgramBuilder builder_;
  size_t max_errors_ = 25;
};

}  // namespace wgsl
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_WGSL_PARSER_IMPL_H_
