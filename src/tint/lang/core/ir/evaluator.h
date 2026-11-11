// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_IR_EVALUATOR_H_
#define SRC_TINT_LANG_CORE_IR_EVALUATOR_H_

#include <cstdint>

#include "src/tint/lang/core/constant/eval.h"
#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/constexpr_if.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/core_binary.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/core_unary.h"
#include "src/tint/lang/core/ir/override.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/vector.h"

namespace tint::core::ir {
class Builder;

/// An evaluator to take a given `ir::Value` and return the result of evaluating the expression.
class Evaluator {
  public:
    /// Constructor
    /// @param builder the ir builder
    explicit Evaluator(ir::Builder& builder, bool eval_override = true);
    /// Destructor
    ~Evaluator();

    /// Evaluate the given `src` expression.
    /// @param src the source expression
    /// @returns the generated constant or a failure result.
    diag::Result<core::ir::Constant*> Evaluate(core::ir::Value* src);

    enum class EvalFailure : uint8_t {
        /// Evaluation encountered a diagnostic error (recorded in `diagnostics_`).
        kError,
        /// A dependency has not been evaluated yet and was pushed onto `EvalTasks`.
        kNotEvaluated,
    };
    using EvalResult = Result<const core::constant::Value*, EvalFailure>;
    using EvalTasks = Vector<core::ir::InstructionResult*, 16>;

    /// Evaluate the binary operator `op` on `lhs` and `rhs`
    /// @param op the operation
    /// @param result_ty the result type
    /// @param lhs the lhs value
    /// @param rhs the rhs value
    /// @param source the source location
    /// @param tasks optional work stack for iterative dependency evaluation
    EvalResult EvalCoreBinary(BinaryOp op,
                              const core::type::Type* result_ty,
                              core::ir::Value* lhs,
                              core::ir::Value* rhs,
                              const Source& source = {},
                              EvalTasks* tasks = nullptr);

    /// Evaluate the unary operator `op` on `input`
    /// @param op the operation
    /// @param result_ty the result type
    /// @param input the input value
    /// @param source the source location
    /// @param tasks optional work stack for iterative dependency evaluation
    EvalResult EvalCoreUnary(UnaryOp op,
                             const core::type::Type* result_ty,
                             core::ir::Value* input,
                             const Source& source = {},
                             EvalTasks* tasks = nullptr);

    /// Evaluate the core builtin call on `args`
    /// @param fn the core builtin function
    /// @param result_ty the result type
    /// @param args the argument values
    /// @param explicit_params the explicit template parameters
    /// @param source the source location
    /// @param tasks optional work stack for iterative dependency evaluation
    EvalResult EvalCoreBuiltinCall(
        core::BuiltinFn fn,
        const core::type::Type* result_ty,
        VectorRef<core::ir::Value*> args,
        VectorRef<core::ir::TemplateParameter> explicit_params = tint::Empty,
        const Source& source = {},
        EvalTasks* tasks = nullptr);

    /// Evaluate construct of `result_ty` with `args`
    /// @param result_ty the result type
    /// @param args the argument values
    /// @param source the source location
    /// @param tasks optional work stack for iterative dependency evaluation
    EvalResult EvalConstruct(const core::type::Type* result_ty,
                             VectorRef<core::ir::Value*> args,
                             const Source& source = {},
                             EvalTasks* tasks = nullptr);

    /// Evaluate conversion of `arg` to `result_ty`
    /// @param result_ty the result type
    /// @param arg the argument value
    /// @param source the source location
    /// @param tasks optional work stack for iterative dependency evaluation
    EvalResult EvalConvert(const core::type::Type* result_ty,
                           core::ir::Value* arg,
                           const Source& source = {},
                           EvalTasks* tasks = nullptr);

    /// Evaluate swizzle of `object` with `indices`
    /// @param result_ty the result type
    /// @param object the object
    /// @param indices the indices
    /// @param tasks optional work stack for iterative dependency evaluation
    EvalResult EvalSwizzle(const core::type::Type* result_ty,
                           core::ir::Value* object,
                           VectorRef<uint32_t> indices,
                           EvalTasks* tasks = nullptr);

    /// Evaluate access of `object` with `indices`
    /// @param object the object
    /// @param indices the indices
    /// @param source the source location
    /// @param tasks optional work stack for iterative dependency evaluation
    EvalResult EvalAccess(core::ir::Value* object,
                          VectorRef<core::ir::Value*> indices,
                          const Source& source = {},
                          EvalTasks* tasks = nullptr);

  private:
    struct CacheEntry {
        bool in_progress = true;
        EvalResult result{nullptr};
    };

    diag::Diagnostic& AddError(Source src);
    Source SourceOf(core::ir::Instruction* val);

    EvalResult EvalValue(core::ir::Value* val, EvalTasks* tasks = nullptr);
    EvalResult EvalAccess(core::ir::Access* a, EvalTasks* tasks);
    EvalResult EvalConvert(core::ir::Convert* c, EvalTasks* tasks);
    EvalResult EvalConstruct(core::ir::Construct* c, EvalTasks* tasks);
    EvalResult EvalSwizzle(core::ir::Swizzle* s, EvalTasks* tasks);
    EvalResult EvalOverride(core::ir::Override* o, EvalTasks* tasks);
    EvalResult EvalUnary(core::ir::CoreUnary* u, EvalTasks* tasks);
    EvalResult EvalBinary(core::ir::CoreBinary* cb, EvalTasks* tasks);
    EvalResult EvalCoreBuiltinCall(core::ir::CoreBuiltinCall* c, EvalTasks* tasks);
    EvalResult EvalConstExprIf(core::ir::ConstExprIf* c, EvalTasks* tasks);

    ir::Builder& b_;
    diag::List diagnostics_;
    core::constant::Eval const_eval_;
    bool eval_override_ = true;
    uint32_t inst_evals_ = 0;
    Hashmap<core::ir::InstructionResult*, CacheEntry, 0> eval_cache_;
};

namespace eval {

/// Evaluate the given `inst` with the provided `b`.
/// @param b the builder
/// @param inst the instruction
/// @returns the evaluated constant for `inst` or a `Failure` otherwise.
diag::Result<core::ir::Constant*> Eval(core::ir::Builder& b, core::ir::Instruction* inst);

/// Evaluate the given `val` with the provided `b`.
/// @param b the builder
/// @param val the value
/// @returns the evaluated constant for `val` or a `Failure` otherwise.
diag::Result<core::ir::Constant*> Eval(core::ir::Builder& b, core::ir::Value* val);

}  // namespace eval

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_EVALUATOR_H_
