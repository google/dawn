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

#ifndef SRC_TINT_WRITER_MSL_GENERATOR_IMPL_H_
#define SRC_TINT_WRITER_MSL_GENERATOR_IMPL_H_

#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "src/tint/ast/assignment_statement.h"
#include "src/tint/ast/binary_expression.h"
#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/expression.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/index_accessor_expression.h"
#include "src/tint/ast/interpolate_attribute.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/member_accessor_expression.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/ast/unary_op_expression.h"
#include "src/tint/builtin/builtin_value.h"
#include "src/tint/program.h"
#include "src/tint/scope_stack.h"
#include "src/tint/sem/struct.h"
#include "src/tint/utils/string_stream.h"
#include "src/tint/writer/array_length_from_uniform_options.h"
#include "src/tint/writer/msl/generator.h"
#include "src/tint/writer/text_generator.h"

// Forward declarations
namespace tint::sem {
class Builtin;
class Call;
class ValueConstructor;
class ValueConversion;
}  // namespace tint::sem

namespace tint::writer::msl {

/// The result of sanitizing a program for generation.
struct SanitizedResult {
    /// Constructor
    SanitizedResult();
    /// Destructor
    ~SanitizedResult();
    /// Move constructor
    SanitizedResult(SanitizedResult&&);

    /// The sanitized program.
    Program program;
    /// True if the shader needs a UBO of buffer sizes.
    bool needs_storage_buffer_sizes = false;
    /// Indices into the array_length_from_uniform binding that are statically
    /// used.
    std::unordered_set<uint32_t> used_array_length_from_uniform_indices;
};

/// Sanitize a program in preparation for generating MSL.
/// @program The program to sanitize
/// @param options The MSL generator options.
/// @returns the sanitized program and any supplementary information
SanitizedResult Sanitize(const Program* program, const Options& options);

/// Implementation class for MSL generator
class GeneratorImpl : public TextGenerator {
  public:
    /// Constructor
    /// @param program the program to generate
    explicit GeneratorImpl(const Program* program);
    ~GeneratorImpl();

    /// @returns true on successful generation; false otherwise
    bool Generate();

    /// @returns true if an invariant attribute was generated
    bool HasInvariant() { return !invariant_define_name_.empty(); }

    /// @returns a map from entry point to list of required workgroup allocations
    const std::unordered_map<std::string, std::vector<uint32_t>>& DynamicWorkgroupAllocations()
        const {
        return workgroup_allocations_;
    }

    /// Handles generating a declared type
    /// @param ty the declared type to generate
    /// @returns true if the declared type was emitted
    bool EmitTypeDecl(const type::Type* ty);
    /// Handles an index accessor expression
    /// @param out the output of the expression stream
    /// @param expr the expression to emit
    /// @returns true if the index accessor was emitted
    bool EmitIndexAccessor(utils::StringStream& out, const ast::IndexAccessorExpression* expr);
    /// Handles an assignment statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted successfully
    bool EmitAssign(const ast::AssignmentStatement* stmt);
    /// Handles generating a binary expression
    /// @param out the output of the expression stream
    /// @param expr the binary expression
    /// @returns true if the expression was emitted, false otherwise
    bool EmitBinary(utils::StringStream& out, const ast::BinaryExpression* expr);
    /// Handles generating a bitcast expression
    /// @param out the output of the expression stream
    /// @param expr the bitcast expression
    /// @returns true if the bitcast was emitted
    bool EmitBitcast(utils::StringStream& out, const ast::BitcastExpression* expr);
    /// Handles a block statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted successfully
    bool EmitBlock(const ast::BlockStatement* stmt);
    /// Handles a break statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted successfully
    bool EmitBreak(const ast::BreakStatement* stmt);
    /// Handles a break-if statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted successfully
    bool EmitBreakIf(const ast::BreakIfStatement* stmt);
    /// Handles generating a call expression
    /// @param out the output of the expression stream
    /// @param expr the call expression
    /// @returns true if the call expression is emitted
    bool EmitCall(utils::StringStream& out, const ast::CallExpression* expr);
    /// Handles generating a builtin call expression
    /// @param out the output of the expression stream
    /// @param call the call expression
    /// @param builtin the builtin being called
    /// @returns true if the call expression is emitted
    bool EmitBuiltinCall(utils::StringStream& out,
                         const sem::Call* call,
                         const sem::Builtin* builtin);
    /// Handles generating a value conversion expression
    /// @param out the output of the expression stream
    /// @param call the call expression
    /// @param conv the value conversion
    /// @returns true if the expression is emitted
    bool EmitTypeConversion(utils::StringStream& out,
                            const sem::Call* call,
                            const sem::ValueConversion* conv);
    /// Handles generating a value constructor
    /// @param out the output of the expression stream
    /// @param call the call expression
    /// @param ctor the value constructor
    /// @returns true if the initializer is emitted
    bool EmitTypeInitializer(utils::StringStream& out,
                             const sem::Call* call,
                             const sem::ValueConstructor* ctor);
    /// Handles generating a function call
    /// @param out the output of the expression stream
    /// @param call the call expression
    /// @param func the target function
    /// @returns true if the call is emitted
    bool EmitFunctionCall(utils::StringStream& out,
                          const sem::Call* call,
                          const sem::Function* func);
    /// Handles generating a call to an atomic function (`atomicAdd`,
    /// `atomicMax`, etc)
    /// @param out the output of the expression stream
    /// @param expr the call expression
    /// @param builtin the semantic information for the atomic builtin
    /// @returns true if the call expression is emitted
    bool EmitAtomicCall(utils::StringStream& out,
                        const ast::CallExpression* expr,
                        const sem::Builtin* builtin);
    /// Handles generating a call to a texture function (`textureSample`,
    /// `textureSampleGrad`, etc)
    /// @param out the output of the expression stream
    /// @param call the call expression
    /// @param builtin the semantic information for the texture builtin
    /// @returns true if the call expression is emitted
    bool EmitTextureCall(utils::StringStream& out,
                         const sem::Call* call,
                         const sem::Builtin* builtin);
    /// Handles generating a call to the `dot()` builtin
    /// @param out the output of the expression stream
    /// @param expr the call expression
    /// @param builtin the semantic information for the builtin
    /// @returns true if the call expression is emitted
    bool EmitDotCall(utils::StringStream& out,
                     const ast::CallExpression* expr,
                     const sem::Builtin* builtin);
    /// Handles generating a call to the `modf()` builtin
    /// @param out the output of the expression stream
    /// @param expr the call expression
    /// @param builtin the semantic information for the builtin
    /// @returns true if the call expression is emitted
    bool EmitModfCall(utils::StringStream& out,
                      const ast::CallExpression* expr,
                      const sem::Builtin* builtin);
    /// Handles generating a call to the `frexp()` builtin
    /// @param out the output of the expression stream
    /// @param expr the call expression
    /// @param builtin the semantic information for the builtin
    /// @returns true if the call expression is emitted
    bool EmitFrexpCall(utils::StringStream& out,
                       const ast::CallExpression* expr,
                       const sem::Builtin* builtin);
    /// Handles generating a call to the `degrees()` builtin
    /// @param out the output of the expression stream
    /// @param expr the call expression
    /// @param builtin the semantic information for the builtin
    /// @returns true if the call expression is emitted
    bool EmitDegreesCall(utils::StringStream& out,
                         const ast::CallExpression* expr,
                         const sem::Builtin* builtin);
    /// Handles generating a call to the `radians()` builtin
    /// @param out the output of the expression stream
    /// @param expr the call expression
    /// @param builtin the semantic information for the builtin
    /// @returns true if the call expression is emitted
    bool EmitRadiansCall(utils::StringStream& out,
                         const ast::CallExpression* expr,
                         const sem::Builtin* builtin);
    /// Handles a case statement
    /// @param stmt the statement
    /// @returns true if the statement was emitted successfully
    bool EmitCase(const ast::CaseStatement* stmt);
    /// Handles a continue statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted successfully
    bool EmitContinue(const ast::ContinueStatement* stmt);
    /// Handles generating a discard statement
    /// @param stmt the discard statement
    /// @returns true if the statement was successfully emitted
    bool EmitDiscard(const ast::DiscardStatement* stmt);
    /// Handles emitting the entry point function
    /// @param func the entry point function
    /// @returns true if the entry point function was emitted
    bool EmitEntryPointFunction(const ast::Function* func);
    /// Handles generate an Expression
    /// @param out the output of the expression stream
    /// @param expr the expression
    /// @returns true if the expression was emitted
    bool EmitExpression(utils::StringStream& out, const ast::Expression* expr);
    /// Handles generating a function
    /// @param func the function to generate
    /// @returns true if the function was emitted
    bool EmitFunction(const ast::Function* func);
    /// Handles generating an identifier expression
    /// @param out the output of the expression stream
    /// @param expr the identifier expression
    /// @returns true if the identifier was emitted
    bool EmitIdentifier(utils::StringStream& out, const ast::IdentifierExpression* expr);
    /// Handles an if statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was successfully emitted
    bool EmitIf(const ast::IfStatement* stmt);
    /// Handles a constant value
    /// @param out the output stream
    /// @param constant the constant value to emit
    /// @returns true if the constant value was successfully emitted
    bool EmitConstant(utils::StringStream& out, const constant::Value* constant);
    /// Handles a literal
    /// @param out the output of the expression stream
    /// @param lit the literal to emit
    /// @returns true if the literal was successfully emitted
    bool EmitLiteral(utils::StringStream& out, const ast::LiteralExpression* lit);
    /// Handles a loop statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted
    bool EmitLoop(const ast::LoopStatement* stmt);
    /// Handles a for loop statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted
    bool EmitForLoop(const ast::ForLoopStatement* stmt);
    /// Handles a while statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted
    bool EmitWhile(const ast::WhileStatement* stmt);
    /// Handles a member accessor expression
    /// @param out the output of the expression stream
    /// @param expr the member accessor expression
    /// @returns true if the member accessor was emitted
    bool EmitMemberAccessor(utils::StringStream& out, const ast::MemberAccessorExpression* expr);
    /// Handles return statements
    /// @param stmt the statement to emit
    /// @returns true if the statement was successfully emitted
    bool EmitReturn(const ast::ReturnStatement* stmt);
    /// Handles emitting a pipeline stage name
    /// @param out the output of the expression stream
    /// @param stage the stage to emit
    void EmitStage(utils::StringStream& out, ast::PipelineStage stage);
    /// Handles statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted
    bool EmitStatement(const ast::Statement* stmt);
    /// Emits a list of statements
    /// @param stmts the statement list
    /// @returns true if the statements were emitted successfully
    bool EmitStatements(utils::VectorRef<const ast::Statement*> stmts);
    /// Emits a list of statements with an indentation
    /// @param stmts the statement list
    /// @returns true if the statements were emitted successfully
    bool EmitStatementsWithIndent(utils::VectorRef<const ast::Statement*> stmts);
    /// Handles generating a switch statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted
    bool EmitSwitch(const ast::SwitchStatement* stmt);
    /// Handles generating a type
    /// @param out the output of the type stream
    /// @param type the type to generate
    /// @param name the name of the variable, only used for array emission
    /// @param name_printed (optional) if not nullptr and an array was printed
    /// @returns true if the type is emitted
    bool EmitType(utils::StringStream& out,
                  const type::Type* type,
                  const std::string& name,
                  bool* name_printed = nullptr);
    /// Handles generating type and name
    /// @param out the output stream
    /// @param type the type to generate
    /// @param name the name to emit
    /// @returns true if the type is emitted
    bool EmitTypeAndName(utils::StringStream& out, const type::Type* type, const std::string& name);
    /// Handles generating a address space
    /// @param out the output of the type stream
    /// @param sc the address space to generate
    /// @returns true if the address space is emitted
    bool EmitAddressSpace(utils::StringStream& out, builtin::AddressSpace sc);
    /// Handles generating a struct declaration. If the structure has already been emitted, then
    /// this function will simply return `true` without emitting anything.
    /// @param buffer the text buffer that the type declaration will be written to
    /// @param str the struct to generate
    /// @returns true if the struct is emitted
    bool EmitStructType(TextBuffer* buffer, const type::Struct* str);
    /// Handles a unary op expression
    /// @param out the output of the expression stream
    /// @param expr the expression to emit
    /// @returns true if the expression was emitted
    bool EmitUnaryOp(utils::StringStream& out, const ast::UnaryOpExpression* expr);
    /// Handles generating a 'var' declaration
    /// @param var the variable to generate
    /// @returns true if the variable was emitted
    bool EmitVar(const ast::Var* var);
    /// Handles generating a 'let' declaration
    /// @param let the variable to generate
    /// @returns true if the variable was emitted
    bool EmitLet(const ast::Let* let);
    /// Emits the zero value for the given type
    /// @param out the output of the expression stream
    /// @param type the type to emit the value for
    /// @returns true if the zero value was successfully emitted.
    bool EmitZeroValue(utils::StringStream& out, const type::Type* type);

    /// Handles generating a builtin name
    /// @param builtin the semantic info for the builtin
    /// @returns the name or "" if not valid
    std::string generate_builtin_name(const sem::Builtin* builtin);

    /// Converts a builtin to an attribute name
    /// @param builtin the builtin to convert
    /// @returns the string name of the builtin or blank on error
    std::string builtin_to_attribute(builtin::BuiltinValue builtin) const;

    /// Converts interpolation attributes to an MSL attribute
    /// @param type the interpolation type
    /// @param sampling the interpolation sampling
    /// @returns the string name of the attribute or blank on error
    std::string interpolation_to_attribute(builtin::InterpolationType type,
                                           builtin::InterpolationSampling sampling) const;

  private:
    // A pair of byte size and alignment `uint32_t`s.
    struct SizeAndAlign {
        uint32_t size;
        uint32_t align;
    };

    /// CallBuiltinHelper will call the builtin helper function, creating it
    /// if it hasn't been built already. If the builtin needs to be built then
    /// CallBuiltinHelper will generate the function signature and will call
    /// `build` to emit the body of the function.
    /// @param out the output of the expression stream
    /// @param call the call expression
    /// @param builtin the semantic information for the builtin
    /// @param build a function with the signature:
    ///        `bool(TextBuffer* buffer, const std::vector<std::string>& params)`
    ///        Where:
    ///          `buffer` is the body of the generated function
    ///          `params` is the name of all the generated function parameters
    /// @returns true if the call expression is emitted
    template <typename F>
    bool CallBuiltinHelper(utils::StringStream& out,
                           const ast::CallExpression* call,
                           const sem::Builtin* builtin,
                           F&& build);

    /// @returns the name of the templated tint_array helper type, generating it if this is the
    /// first call.
    const std::string& ArrayType();

    TextBuffer helpers_;  // Helper functions emitted at the top of the output

    /// @returns the MSL packed type size and alignment in bytes for the given
    /// type.
    SizeAndAlign MslPackedTypeSizeAndAlign(const type::Type* ty);

    std::function<bool()> emit_continuing_;

    /// Name of atomicCompareExchangeWeak() helper for the given pointer storage
    /// class and struct return type
    using ACEWKeyType =
        utils::UnorderedKeyWrapper<std::tuple<builtin::AddressSpace, const type::Struct*>>;
    std::unordered_map<ACEWKeyType, std::string> atomicCompareExchangeWeak_;

    /// Unique name of the 'TINT_INVARIANT' preprocessor define.
    /// Non-empty only if an invariant attribute has been generated.
    std::string invariant_define_name_;

    /// The generated name for the packed vec3 type.
    std::string packed_vec3_ty_;

    /// Unique name of the tint_array<T, N> template.
    /// Non-empty only if the template has been generated.
    std::string array_template_name_;

    /// A map from entry point name to a list of dynamic workgroup allocations.
    /// Each entry in the vector is the size of the workgroup allocation that
    /// should be created for that index.
    std::unordered_map<std::string, std::vector<uint32_t>> workgroup_allocations_;

    std::unordered_map<const sem::Builtin*, std::string> builtins_;
    std::unordered_map<const type::Type*, std::string> unary_minus_funcs_;
    std::unordered_map<uint32_t, std::string> int_dot_funcs_;
    std::unordered_set<const type::Struct*> emitted_structs_;
};

}  // namespace tint::writer::msl

#endif  // SRC_TINT_WRITER_MSL_GENERATOR_IMPL_H_
