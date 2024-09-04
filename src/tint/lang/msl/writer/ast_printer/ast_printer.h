// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_LANG_MSL_WRITER_AST_PRINTER_AST_PRINTER_H_
#define SRC_TINT_LANG_MSL_WRITER_AST_PRINTER_AST_PRINTER_H_

#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/msl/writer/common/options.h"
#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/binary_expression.h"
#include "src/tint/lang/wgsl/ast/break_statement.h"
#include "src/tint/lang/wgsl/ast/continue_statement.h"
#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/ast/expression.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"
#include "src/tint/lang/wgsl/ast/index_accessor_expression.h"
#include "src/tint/lang/wgsl/ast/interpolate_attribute.h"
#include "src/tint/lang/wgsl/ast/loop_statement.h"
#include "src/tint/lang/wgsl/ast/member_accessor_expression.h"
#include "src/tint/lang/wgsl/ast/return_statement.h"
#include "src/tint/lang/wgsl/ast/switch_statement.h"
#include "src/tint/lang/wgsl/ast/unary_op_expression.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/utils/containers/scope_stack.h"
#include "src/tint/utils/generator/text_generator.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::sem {
class BuiltinFn;
class Call;
class ValueConstructor;
class ValueConversion;
}  // namespace tint::sem

namespace tint::msl::writer {

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
};

/// Sanitize a program in preparation for generating MSL.
/// @param program The program to sanitize
/// @param options The MSL generator options.
/// @returns the sanitized program and any supplementary information
SanitizedResult Sanitize(const Program& program, const Options& options);

/// Implementation class for MSL generator
class ASTPrinter : public tint::TextGenerator {
  public:
    /// Constructor
    /// @param program the program to generate
    explicit ASTPrinter(const Program& program);
    ~ASTPrinter() override;

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
    bool EmitTypeDecl(const core::type::Type* ty);
    /// Handles an index accessor expression
    /// @param out the output of the expression stream
    /// @param expr the expression to emit
    /// @returns true if the index accessor was emitted
    bool EmitIndexAccessor(StringStream& out, const ast::IndexAccessorExpression* expr);
    /// Handles an assignment statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted successfully
    bool EmitAssign(const ast::AssignmentStatement* stmt);
    /// Handles generating a binary expression
    /// @param out the output of the expression stream
    /// @param expr the binary expression
    /// @returns true if the expression was emitted, false otherwise
    bool EmitBinary(StringStream& out, const ast::BinaryExpression* expr);
    /// Handles generating a bitcast expression
    /// @param out the output of the expression stream
    /// @param expr the bitcast expression
    /// @returns true if the bitcast was emitted
    bool EmitBitcastCall(StringStream& out, const ast::CallExpression* expr);
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
    bool EmitCall(StringStream& out, const ast::CallExpression* expr);
    /// Handles generating a builtin call expression
    /// @param out the output of the expression stream
    /// @param call the call expression
    /// @param builtin the builtin being called
    /// @returns true if the call expression is emitted
    bool EmitBuiltinCall(StringStream& out, const sem::Call* call, const sem::BuiltinFn* builtin);
    /// Handles generating a value conversion expression
    /// @param out the output of the expression stream
    /// @param call the call expression
    /// @param conv the value conversion
    /// @returns true if the expression is emitted
    bool EmitTypeConversion(StringStream& out,
                            const sem::Call* call,
                            const sem::ValueConversion* conv);
    /// Handles generating a value constructor
    /// @param out the output of the expression stream
    /// @param call the call expression
    /// @param ctor the value constructor
    /// @returns true if the initializer is emitted
    bool EmitTypeInitializer(StringStream& out,
                             const sem::Call* call,
                             const sem::ValueConstructor* ctor);
    /// Handles generating a function call
    /// @param out the output of the expression stream
    /// @param call the call expression
    /// @param func the target function
    /// @returns true if the call is emitted
    bool EmitFunctionCall(StringStream& out, const sem::Call* call, const sem::Function* func);
    /// Handles generating a call to an atomic function (`atomicAdd`,
    /// `atomicMax`, etc)
    /// @param out the output of the expression stream
    /// @param expr the call expression
    /// @param builtin the semantic information for the atomic builtin
    /// @returns true if the call expression is emitted
    bool EmitAtomicCall(StringStream& out,
                        const ast::CallExpression* expr,
                        const sem::BuiltinFn* builtin);
    /// Handles generating a call to a texture function (`textureSample`,
    /// `textureSampleGrad`, etc)
    /// @param out the output of the expression stream
    /// @param call the call expression
    /// @param builtin the semantic information for the texture builtin
    /// @returns true if the call expression is emitted
    bool EmitTextureCall(StringStream& out, const sem::Call* call, const sem::BuiltinFn* builtin);
    /// Handles generating a call to the `dot()` builtin
    /// @param out the output of the expression stream
    /// @param expr the call expression
    /// @param builtin the semantic information for the builtin
    /// @returns true if the call expression is emitted
    bool EmitDotCall(StringStream& out,
                     const ast::CallExpression* expr,
                     const sem::BuiltinFn* builtin);
    /// Handles generating a call to the `modf()` builtin
    /// @param out the output of the expression stream
    /// @param expr the call expression
    /// @param builtin the semantic information for the builtin
    /// @returns true if the call expression is emitted
    bool EmitModfCall(StringStream& out,
                      const ast::CallExpression* expr,
                      const sem::BuiltinFn* builtin);
    /// Handles generating a call to the `frexp()` builtin
    /// @param out the output of the expression stream
    /// @param expr the call expression
    /// @param builtin the semantic information for the builtin
    /// @returns true if the call expression is emitted
    bool EmitFrexpCall(StringStream& out,
                       const ast::CallExpression* expr,
                       const sem::BuiltinFn* builtin);
    /// Handles generating a call to the `degrees()` builtin
    /// @param out the output of the expression stream
    /// @param expr the call expression
    /// @param builtin the semantic information for the builtin
    /// @returns true if the call expression is emitted
    bool EmitDegreesCall(StringStream& out,
                         const ast::CallExpression* expr,
                         const sem::BuiltinFn* builtin);
    /// Handles generating a call to the `radians()` builtin
    /// @param out the output of the expression stream
    /// @param expr the call expression
    /// @param builtin the semantic information for the builtin
    /// @returns true if the call expression is emitted
    bool EmitRadiansCall(StringStream& out,
                         const ast::CallExpression* expr,
                         const sem::BuiltinFn* builtin);
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
    bool EmitExpression(StringStream& out, const ast::Expression* expr);
    /// Handles generating a function
    /// @param func the function to generate
    /// @returns true if the function was emitted
    bool EmitFunction(const ast::Function* func);
    /// Handles generating an identifier expression
    /// @param out the output of the expression stream
    /// @param expr the identifier expression
    /// @returns true if the identifier was emitted
    bool EmitIdentifier(StringStream& out, const ast::IdentifierExpression* expr);
    /// Handles an if statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was successfully emitted
    bool EmitIf(const ast::IfStatement* stmt);
    /// Handles a constant value
    /// @param out the output stream
    /// @param constant the constant value to emit
    /// @returns true if the constant value was successfully emitted
    bool EmitConstant(StringStream& out, const core::constant::Value* constant);
    /// Handles a literal
    /// @param out the output of the expression stream
    /// @param lit the literal to emit
    /// @returns true if the literal was successfully emitted
    bool EmitLiteral(StringStream& out, const ast::LiteralExpression* lit);
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
    bool EmitMemberAccessor(StringStream& out, const ast::MemberAccessorExpression* expr);
    /// Handles return statements
    /// @param stmt the statement to emit
    /// @returns true if the statement was successfully emitted
    bool EmitReturn(const ast::ReturnStatement* stmt);
    /// Handles emitting a pipeline stage name
    /// @param out the output of the expression stream
    /// @param stage the stage to emit
    void EmitStage(StringStream& out, ast::PipelineStage stage);
    /// Handles statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted
    bool EmitStatement(const ast::Statement* stmt);
    /// Emits a list of statements
    /// @param stmts the statement list
    /// @returns true if the statements were emitted successfully
    bool EmitStatements(VectorRef<const ast::Statement*> stmts);
    /// Emits a list of statements with an indentation
    /// @param stmts the statement list
    /// @returns true if the statements were emitted successfully
    bool EmitStatementsWithIndent(VectorRef<const ast::Statement*> stmts);
    /// Handles generating a switch statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted
    bool EmitSwitch(const ast::SwitchStatement* stmt);
    /// Handles generating a type
    /// @param out the output of the type stream
    /// @param type the type to generate
    /// @returns true if the type is emitted
    bool EmitType(StringStream& out, const core::type::Type* type);
    /// Handles generating type and name
    /// @param out the output stream
    /// @param type the type to generate
    /// @param name the name to emit
    /// @returns true if the type is emitted
    bool EmitTypeAndName(StringStream& out, const core::type::Type* type, const std::string& name);
    /// Handles generating a address space
    /// @param out the output of the type stream
    /// @param sc the address space to generate
    /// @returns true if the address space is emitted
    bool EmitAddressSpace(StringStream& out, core::AddressSpace sc);
    /// Handles generating a struct declaration. If the structure has already been emitted, then
    /// this function will simply return `true` without emitting anything.
    /// @param buffer the text buffer that the type declaration will be written to
    /// @param str the struct to generate
    /// @returns true if the struct is emitted
    bool EmitStructType(TextBuffer* buffer, const core::type::Struct* str);
    /// Handles a unary op expression
    /// @param out the output of the expression stream
    /// @param expr the expression to emit
    /// @returns true if the expression was emitted
    bool EmitUnaryOp(StringStream& out, const ast::UnaryOpExpression* expr);
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
    bool EmitZeroValue(StringStream& out, const core::type::Type* type);
    /// Handles generating a call to the `dot4I8Packed()` builtin
    /// @param out the output of the expression stream
    /// @param expr the call expression
    /// @param builtin the semantic information for the builtin
    /// @returns true if the call expression is emitted
    bool EmitDot4I8PackedCall(StringStream& out,
                              const ast::CallExpression* expr,
                              const sem::BuiltinFn* builtin);
    /// Handles generating a call to the `dot4U8Packed()` builtin
    /// @param out the output of the expression stream
    /// @param expr the call expression
    /// @param builtin the semantic information for the builtin
    /// @returns true if the call expression is emitted
    bool EmitDot4U8PackedCall(StringStream& out,
                              const ast::CallExpression* expr,
                              const sem::BuiltinFn* builtin);

    /// Lazilly generates the TINT_ISOLATE_UB macro, used to prevent UB statements from affecting
    /// later logic.
    /// @return the MSL to call the TINT_ISOLATE_UB macro.
    std::string IsolateUB();

    /// Handles generating a builtin name
    /// @param builtin the semantic info for the builtin
    /// @returns the name or "" if not valid
    std::string generate_builtin_name(const sem::BuiltinFn* builtin);

  private:
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
    bool CallBuiltinHelper(StringStream& out,
                           const ast::CallExpression* call,
                           const sem::BuiltinFn* builtin,
                           F&& build);

    /// @returns the name of the templated tint_array helper type, generating it if this is the
    /// first call.
    const std::string& ArrayType();

    /// @param s the structure
    /// @returns the name of the structure, taking special care of builtin structures that start
    /// with double underscores. If the structure is a builtin, then the returned name will be a
    /// unique name without the leading underscores.
    std::string StructName(const core::type::Struct* s);

    /// @return a new, unique identifier with the given prefix.
    /// @param prefix optional prefix to apply to the generated identifier. If empty "tint_symbol"
    /// will be used.
    std::string UniqueIdentifier(const std::string& prefix = "");

    /// Alias for builder_.TypeOf(ptr)
    template <typename T>
    auto TypeOf(T* ptr) {
        return builder_.TypeOf(ptr);
    }

    ProgramBuilder builder_;

    TextBuffer helpers_;  // Helper functions emitted at the top of the output

    /// Map of builtin structure to unique generated name
    std::unordered_map<const core::type::Struct*, std::string> builtin_struct_names_;

    std::function<bool()> emit_continuing_;

    /// The name of the macro used to prevent UB affecting later control flow.
    /// Do not use this directly, instead call IsolateUB().
    std::string isolate_ub_macro_name_;

    /// Name of atomicCompareExchangeWeak() helper for the given pointer storage
    /// class and struct return type
    using ACEWKeyType =
        tint::UnorderedKeyWrapper<std::tuple<core::AddressSpace, const core::type::Struct*>>;
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

    std::unordered_map<const sem::BuiltinFn*, std::string> builtins_;
    std::unordered_map<const core::type::Type*, std::string> unary_minus_funcs_;
    std::unordered_map<uint32_t, std::string> int_dot_funcs_;
    std::unordered_set<const core::type::Struct*> emitted_structs_;
};

}  // namespace tint::msl::writer

#endif  // SRC_TINT_LANG_MSL_WRITER_AST_PRINTER_AST_PRINTER_H_
