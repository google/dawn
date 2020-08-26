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

#include "src/ast/assignment_statement.h"
#include "src/ast/builtin.h"
#include "src/ast/call_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/constructor_expression.h"
#include "src/ast/else_statement.h"
#include "src/ast/entry_point.h"
#include "src/ast/function.h"
#include "src/ast/import.h"
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
#include "src/ast/type/type.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decoration.h"
#include "src/context.h"
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
 public:
  /// Creates a new parser
  /// @param ctx the non-null context object
  /// @param input the input string to parse
  ParserImpl(Context* ctx, const std::string& input);
  ~ParserImpl();

  /// Run the parser
  /// @returns true if the parse was successful, false otherwise.
  bool Parse();

  /// @returns true if an error was encountered
  bool has_error() const { return error_.size() > 0; }
  /// @returns the parser error string
  const std::string& error() const { return error_; }

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
  /// Sets the error from |t|
  /// @param t the token to set the error from
  void set_error(const Token& t);
  /// Sets the error from |t| or |msg| if |t| is not in error
  /// @param t the token to set the error from
  /// @param msg the error message
  void set_error(const Token& t, const std::string& msg);

  /// Registers a type alias into the parser
  /// @param name the alias name
  /// @param type the alias'd type
  void register_alias(const std::string& name, ast::type::Type* type);
  /// Retrieves an aliased type
  /// @param name The alias name to lookup
  /// @returns the alias type for |name| or nullptr if not found
  ast::type::Type* get_alias(const std::string& name);

  /// Parses the `translation_unit` grammar element
  void translation_unit();
  /// Parses the `global_decl` grammar element
  void global_decl();
  /// Parses the `import_decl grammar element
  /// @returns the import object or nullptr if an error was encountered
  std::unique_ptr<ast::Import> import_decl();
  /// Parses a `global_variable_decl` grammar element
  /// @returns the variable parsed or nullptr
  std::unique_ptr<ast::Variable> global_variable_decl();
  /// Parses a `global_constant_decl` grammar element
  /// @returns the const object or nullptr
  std::unique_ptr<ast::Variable> global_constant_decl();
  /// Parses a `variable_decoration_list` grammar element
  /// @returns the parsed variable decorations
  ast::VariableDecorationList variable_decoration_list();
  /// Parses a `variable_decoration` grammar element
  /// @returns the variable decoration or nullptr if an error is encountered
  std::unique_ptr<ast::VariableDecoration> variable_decoration();
  /// Parses a `variable_decl` grammar element
  /// @returns the parsed variable or nullptr otherwise
  std::unique_ptr<ast::Variable> variable_decl();
  /// Parses a `variable_ident_decl` grammar element
  /// @returns the identifier and type parsed or empty otherwise
  std::pair<std::string, ast::type::Type*> variable_ident_decl();
  /// Parses a `variable_storage_decoration` grammar element
  /// @returns the storage class or StorageClass::kNone if none matched
  ast::StorageClass variable_storage_decoration();
  /// Parses a `type_alias` grammar element
  /// @returns the type alias or nullptr on error
  ast::type::AliasType* type_alias();
  /// Parses a `type_decl` grammar element
  /// @returns the parsed Type or nullptr if none matched. The returned type
  //           is owned by the TypeManager.
  ast::type::Type* type_decl();
  /// Parses a `storage_class` grammar element
  /// @returns the storage class or StorageClass::kNone if none matched
  ast::StorageClass storage_class();
  /// Parses a `struct_decl` grammar element
  /// @returns the struct type or nullptr on error
  std::unique_ptr<ast::type::StructType> struct_decl();
  /// Parses a `struct_decoration_decl` grammar element
  /// @returns the struct decoration or StructDecoraton::kNone
  ast::StructDecoration struct_decoration_decl();
  /// Parses a `struct_decoration` grammar element
  /// @param t the current token
  /// @returns the struct decoration or StructDecoraton::kNone if none matched
  ast::StructDecoration struct_decoration(Token t);
  /// Parses a `struct_body_decl` grammar element
  /// @returns the struct members
  ast::StructMemberList struct_body_decl();
  /// Parses a `struct_member` grammar element
  /// @returns the struct member or nullptr
  std::unique_ptr<ast::StructMember> struct_member();
  /// Parses a `struct_member_decoration_decl` grammar element
  /// @returns the list of decorations
  ast::StructMemberDecorationList struct_member_decoration_decl();
  /// Parses a `struct_member_decoration` grammar element
  /// @returns the decoration or nullptr if none found
  std::unique_ptr<ast::StructMemberDecoration> struct_member_decoration();
  /// Parses a `function_decl` grammar element
  /// @returns the parsed function, nullptr otherwise
  std::unique_ptr<ast::Function> function_decl();
  /// Parses a `function_type_decl` grammar element
  /// @returns the parsed type or nullptr otherwise
  ast::type::Type* function_type_decl();
  /// Parses a `function_header` grammar element
  /// @returns the parsed function nullptr otherwise
  std::unique_ptr<ast::Function> function_header();
  /// Parses a `param_list` grammar element
  /// @returns the parsed variables
  ast::VariableList param_list();
  /// Parses a `entry_point_decl` grammar element
  /// @returns the EntryPoint or nullptr on error
  std::unique_ptr<ast::EntryPoint> entry_point_decl();
  /// Parses a `pipeline_stage` grammar element
  /// @returns the pipeline stage or PipelineStage::kNone if none matched
  ast::PipelineStage pipeline_stage();
  /// Parses a `body_stmt` grammar element
  /// @returns the parsed statements
  std::unique_ptr<ast::BlockStatement> body_stmt();
  /// Parses a `paren_rhs_stmt` grammar element
  /// @returns the parsed element or nullptr
  std::unique_ptr<ast::Expression> paren_rhs_stmt();
  /// Parses a `statements` grammar element
  /// @returns the statements parsed
  std::unique_ptr<ast::BlockStatement> statements();
  /// Parses a `statement` grammar element
  /// @returns the parsed statement or nullptr
  std::unique_ptr<ast::Statement> statement();
  /// Parses a `break_stmt` grammar element
  /// @returns the parsed statement or nullptr
  std::unique_ptr<ast::BreakStatement> break_stmt();
  /// Parses a `return_stmt` grammar element
  /// @returns the parsed statement or nullptr
  std::unique_ptr<ast::ReturnStatement> return_stmt();
  /// Parses a `continue_stmt` grammar element
  /// @returns the parsed statement or nullptr
  std::unique_ptr<ast::ContinueStatement> continue_stmt();
  /// Parses a `variable_stmt` grammar element
  /// @returns the parsed variable or nullptr
  std::unique_ptr<ast::VariableDeclStatement> variable_stmt();
  /// Parses a `if_stmt` grammar element
  /// @returns the parsed statement or nullptr
  std::unique_ptr<ast::IfStatement> if_stmt();
  /// Parses a `elseif_stmt` grammar element
  /// @returns the parsed elements
  ast::ElseStatementList elseif_stmt();
  /// Parses a `else_stmt` grammar element
  /// @returns the parsed statement or nullptr
  std::unique_ptr<ast::ElseStatement> else_stmt();
  /// Parses a `switch_stmt` grammar element
  /// @returns the parsed statement or nullptr
  std::unique_ptr<ast::SwitchStatement> switch_stmt();
  /// Parses a `switch_body` grammar element
  /// @returns the parsed statement or nullptr
  std::unique_ptr<ast::CaseStatement> switch_body();
  /// Parses a `case_selectors` grammar element
  /// @returns the list of literals
  ast::CaseSelectorList case_selectors();
  /// Parses a `case_body` grammar element
  /// @returns the parsed statements
  std::unique_ptr<ast::BlockStatement> case_body();
  /// Parses a `func_call_stmt` grammar element
  /// @returns the parsed function call or nullptr
  std::unique_ptr<ast::CallStatement> func_call_stmt();
  /// Parses a `loop_stmt` grammar element
  /// @returns the parsed loop or nullptr
  std::unique_ptr<ast::LoopStatement> loop_stmt();
  /// Parses a `for_header` grammar element
  /// @returns the parsed for header or nullptr
  std::unique_ptr<ForHeader> for_header();
  /// Parses a `for_stmt` grammar element
  /// @returns the parsed for loop or nullptr
  std::unique_ptr<ast::Statement> for_stmt();
  /// Parses a `continuing_stmt` grammar element
  /// @returns the parsed statements
  std::unique_ptr<ast::BlockStatement> continuing_stmt();
  /// Parses a `const_literal` grammar element
  /// @returns the const literal parsed or nullptr if none found
  std::unique_ptr<ast::Literal> const_literal();
  /// Parses a `const_expr` grammar element
  /// @returns the parsed constructor expression or nullptr on error
  std::unique_ptr<ast::ConstructorExpression> const_expr();
  /// Parses a `primary_expression` grammar element
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> primary_expression();
  /// Parses a `argument_expression_list` grammar element
  /// @returns the list of arguments
  ast::ExpressionList argument_expression_list();
  /// Parses the recursive portion of the postfix_expression
  /// @param prefix the left side of the expression
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> postfix_expr(
      std::unique_ptr<ast::Expression> prefix);
  /// Parses a `postfix_expression` grammar elment
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> postfix_expression();
  /// Parses a `unary_expression` grammar element
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> unary_expression();
  /// Parses the recursive part of the `multiplicative_expression`
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> multiplicative_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses the `multiplicative_expression` grammar element
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> multiplicative_expression();
  /// Parses the recursive part of the `additive_expression`
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> additive_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses the `additive_expression` grammar element
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> additive_expression();
  /// Parses the recursive part of the `shift_expression`
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> shift_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses the `shift_expression` grammar element
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> shift_expression();
  /// Parses the recursive part of the `relational_expression`
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> relational_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses the `relational_expression` grammar element
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> relational_expression();
  /// Parses the recursive part of the `equality_expression`
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> equality_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses the `equality_expression` grammar element
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> equality_expression();
  /// Parses the recursive part of the `and_expression`
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> and_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses the `and_expression` grammar element
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> and_expression();
  /// Parses the recursive part of the `exclusive_or_expression`
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> exclusive_or_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses the `exclusive_or_expression` grammar elememnt
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> exclusive_or_expression();
  /// Parses the recursive part of the `inclusive_or_expression`
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> inclusive_or_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses the `inclusive_or_expression` grammar element
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> inclusive_or_expression();
  /// Parses the recursive part of the `logical_and_expression`
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> logical_and_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses a `logical_and_expression` grammar element
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> logical_and_expression();
  /// Parses the recursive part of the `logical_or_expression`
  /// @param lhs the left side of the expression
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> logical_or_expr(
      std::unique_ptr<ast::Expression> lhs);
  /// Parses a `logical_or_expression` grammar element
  /// @returns the parsed expression or nullptr
  std::unique_ptr<ast::Expression> logical_or_expression();
  /// Parses a `assignment_stmt` grammar element
  /// @returns the parsed assignment or nullptr
  std::unique_ptr<ast::AssignmentStatement> assignment_stmt();

 private:
  ast::type::Type* type_decl_pointer(Token t);
  ast::type::Type* type_decl_vector(Token t);
  ast::type::Type* type_decl_array(Token t, uint32_t stride);
  uint32_t array_decoration_list();
  ast::type::Type* type_decl_matrix(Token t);

  std::unique_ptr<ast::ConstructorExpression> const_expr_internal(
      uint32_t depth);

  Context& ctx_;
  std::string error_;
  std::unique_ptr<Lexer> lexer_;
  std::deque<Token> token_queue_;
  std::unordered_map<std::string, ast::type::Type*> registered_aliases_;
  ast::Module module_;
};

}  // namespace wgsl
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_WGSL_PARSER_IMPL_H_
