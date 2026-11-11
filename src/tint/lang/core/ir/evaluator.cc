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

#include "src/tint/lang/core/ir/evaluator.h"

#include "src/tint/lang/core/binary_op.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/constexpr_if.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/override.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/utils/internal_limits.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::core::ir {
namespace eval {

diag::Result<core::ir::Constant*> Eval(core::ir::Builder& b, core::ir::Instruction* inst) {
    return Eval(b, inst->Result());
}

diag::Result<core::ir::Constant*> Eval(core::ir::Builder& b, core::ir::Value* val) {
    ir::Evaluator e(b);
    return e.Evaluate(val);
}

}  // namespace eval

Evaluator::Evaluator(ir::Builder& builder, bool eval_override)
    : b_(builder),
      const_eval_(b_.ir.constant_values, diagnostics_),
      eval_override_(eval_override) {}

Evaluator::~Evaluator() = default;

diag::Result<core::ir::Constant*> Evaluator::Evaluate(core::ir::Value* src) {
    auto res = EvalValue(src);
    if (res != Success) {
        TINT_ASSERT(res != EvalFailure::kNotEvaluated);
        return diag::Failure(diagnostics_);
    }
    if (!res.Get()) {
        return nullptr;
    }

    return b_.Constant(res.Get());
}

diag::Diagnostic& Evaluator::AddError(Source src) {
    diag::Diagnostic diag;
    diag.source = src;
    return diagnostics_.Add(diag);
}

Source Evaluator::SourceOf(core::ir::Instruction* val) {
    return b_.ir.SourceOf(val);
}

Evaluator::EvalResult Evaluator::EvalValue(core::ir::Value* val, EvalTasks* tasks) {
    auto try_eval = [&](core::ir::Value* v, EvalTasks& task_stack) -> EvalResult {
        return tint::Switch(
            v,  //
            [&](core::ir::Constant* c) -> EvalResult { return c->Value(); },
            [&](core::ir::FunctionParam*) -> EvalResult { return nullptr; },
            [&](core::ir::BlockParam*) -> EvalResult { return nullptr; },  //
            [&](core::ir::Unused*) -> EvalResult { return nullptr; },
            [&](core::ir::InstructionResult* r) -> EvalResult {
                if (auto cached = eval_cache_.Get(r)) {
                    TINT_ASSERT(!cached->in_progress);
                    return cached->result;
                }
                task_stack.Push(r);
                return EvalFailure::kNotEvaluated;
            },
            TINT_ICE_ON_NO_MATCH);
    };

    if (tasks != nullptr) {
        return try_eval(val, *tasks);
    }

    EvalTasks local_tasks;
    auto initial = try_eval(val, local_tasks);
    if (initial != EvalFailure::kNotEvaluated) {
        return initial;
    }

    while (!local_tasks.IsEmpty()) {
        auto* r = local_tasks.Back();
        auto cached = eval_cache_.Get(r);
        if (!cached) {
            if (!eval_override_) {
                constexpr uint32_t kMaxBuilderEvalDepth = 4;
                constexpr uint32_t kMaxBuilderInstEvals = 8;
                if (local_tasks.Length() > kMaxBuilderEvalDepth ||
                    inst_evals_ >= kMaxBuilderInstEvals) {
                    eval_cache_.Add(r, CacheEntry{/* in_progress */ false, nullptr});
                    local_tasks.Pop();
                    continue;
                }
                ++inst_evals_;
            }
            eval_cache_.Add(r, CacheEntry{/* in_progress */ true, nullptr});
        } else if (!cached->in_progress) {
            local_tasks.Pop();
            continue;
        }

        const size_t prev_len = local_tasks.Length();
        auto res = tint::Switch(
            r->Instruction(),  //
            [&](core::ir::Access* a) { return EvalAccess(a, &local_tasks); },
            [&](core::ir::ConstExprIf* c) { return EvalConstExprIf(c, &local_tasks); },
            [&](core::ir::Construct* c) { return EvalConstruct(c, &local_tasks); },
            [&](core::ir::Convert* c) { return EvalConvert(c, &local_tasks); },
            [&](core::ir::CoreBinary* cb) { return EvalBinary(cb, &local_tasks); },
            [&](core::ir::CoreBuiltinCall* c) { return EvalCoreBuiltinCall(c, &local_tasks); },
            [&](core::ir::CoreUnary* u) { return EvalUnary(u, &local_tasks); },
            [&](core::ir::Override* o) { return EvalOverride(o, &local_tasks); },
            [&](core::ir::Swizzle* s) { return EvalSwizzle(s, &local_tasks); },
            [&](Default) -> EvalResult {
                // Treat any unknown instruction as a termination point for trying to eval.
                return nullptr;
            });
        if (res != Success) {
            if (res == EvalFailure::kNotEvaluated) {
                TINT_ASSERT(local_tasks.Length() > prev_len);
                continue;
            }
            eval_cache_.Replace(r, CacheEntry{/* in_progress */ false, res});
            return res;
        }

        eval_cache_.Replace(r, CacheEntry{/* in_progress */ false, res});
        local_tasks.Pop();
    }

    auto cached = eval_cache_.Get(val->As<core::ir::InstructionResult>());
    TINT_ASSERT(cached && !cached->in_progress);
    return cached->result;
}

Evaluator::EvalResult Evaluator::EvalAccess(core::ir::Access* a, EvalTasks* tasks) {
    return EvalAccess(a->Object(), a->Indices(), SourceOf(a), tasks);
}

Evaluator::EvalResult Evaluator::EvalAccess(core::ir::Value* object,
                                            VectorRef<core::ir::Value*> indices,
                                            const Source& source,
                                            EvalTasks* tasks) {
    // Some transforms create invalid instructions.
    if (!object || !object->Type()) {
        return nullptr;
    }

    TINT_CHECK_RESULT_UNWRAP(obj, EvalValue(object, tasks));
    if (!eval_override_ && !obj) {
        return nullptr;
    }

    auto* access_obj_type = object->Type()->UnwrapPtrOrRef();
    for (auto* idx : indices) {
        TINT_CHECK_RESULT_UNWRAP(val, EvalValue(idx, tasks));

        // Check if the value could be evaluated
        constexpr uint32_t kDefaultConstIndex = 0;
        uint32_t index_const = kDefaultConstIndex;
        if (val) {
            TINT_ASSERT(val->Is<core::constant::Value>());

            auto res = const_eval_.Index(obj, access_obj_type, val, source);
            if (res != Success) {
                return EvalFailure::kError;
            }
            index_const = val->ValueAs<u32>();
            obj = res.Get();
        } else {
            // No constant array evaluation possible for non-const (dynamic) indices. Only
            // validation of bounds is possible at this stage.
            obj = nullptr;
        }

        // Index element to type to support type nested access.
        access_obj_type = access_obj_type->Element(index_const);
        TINT_ASSERT(access_obj_type);
    }
    return obj;
}

Evaluator::EvalResult Evaluator::EvalConstruct(core::ir::Construct* c, EvalTasks* tasks) {
    return EvalConstruct(c->Result()->Type(), c->Args(), SourceOf(c), tasks);
}

Evaluator::EvalResult Evaluator::EvalConstruct(const core::type::Type* result_ty,
                                               VectorRef<core::ir::Value*> args,
                                               const Source& source,
                                               EvalTasks* tasks) {
    // Some transforms create invalid instructions.
    if (!result_ty) {
        return nullptr;
    }

    Vector<const core::type::Type*, 4> arg_types;
    arg_types.Reserve(args.Length());
    Vector<const core::constant::Value*, 4> arg_values;
    arg_values.Reserve(args.Length());

    for (auto* arg : args) {
        arg_types.Push(arg->Type());

        TINT_CHECK_RESULT_UNWRAP(val, EvalValue(arg, tasks));
        // Check if the value could be evaluated
        if (!val) {
            return nullptr;
        }
        arg_values.Push(val);
    }

    auto table = core::intrinsic::Table<core::intrinsic::Dialect>(b_.ir.Types(), b_.ir.symbols);
    auto eval_stage =
        eval_override_ ? core::EvaluationStage::kOverride : core::EvaluationStage::kConstant;
    auto mat_vec = [&](const core::type::Type* type,
                       core::intrinsic::CtorConv intrinsic) -> constant::Eval::Result {
        auto op =
            table.Lookup(intrinsic, Vector<TemplateParameter, 1>{type}, arg_types, eval_stage);
        if (op != Success) {
            AddError(source) << "unable to find intrinsic for construct: " << op.Failure();
            return Failure();
        }
        if (!op->info->flags.Contains(intrinsic::OverloadFlag::kIsConstructor)) {
            return Failure();
        }
        if (!op->const_eval_fn) {
            AddError(source) << "unhandled type constructor";
            return Failure();
        }
        TINT_CHECK_RESULT_UNWRAP(r,
                                 (const_eval_.*op->const_eval_fn)(result_ty, arg_values, source));
        return r;
    };

    // Dispatch to the appropriate const eval function.
    auto r = tint::Switch(
        result_ty,  //
        [&](const core::type::Array*) {
            return const_eval_.ArrayOrStructCtor(result_ty, arg_values);
        },
        [&](const core::type::Struct*) {
            return const_eval_.ArrayOrStructCtor(result_ty, arg_values);
        },
        [&](const core::type::Vector* vec) {
            return mat_vec(vec->Type(), core::intrinsic::VectorCtorConv(vec->Width()));
        },
        [&](const core::type::Matrix* mat) {
            return mat_vec(mat->Type(),
                           core::intrinsic::MatrixCtorConv(mat->Columns(), mat->Rows()));
        },
        [&](Default) {
            if (!result_ty->Is<core::type::Scalar>()) {
                AddError(source) << "unhandled type constructor";
                return core::constant::Eval::Result(nullptr);
            }
            if (arg_values.IsEmpty()) {
                return const_eval_.Zero(result_ty, arg_values, source);
            }
            // For scalars, this must be an identity constructor.
            if (arg_values[0]->Type() != result_ty) {
                AddError(source) << "invalid type constructor";
                return core::constant::Eval::Result(nullptr);
            }
            if (arg_values.Length() != 1) {
                return core::constant::Eval::Result(nullptr);
            }
            return const_eval_.Identity(result_ty, arg_values, source);
        });

    if (r != Success) {
        return EvalFailure::kError;
    }
    return r.Get();
}

Evaluator::EvalResult Evaluator::EvalConvert(core::ir::Convert* c, EvalTasks* tasks) {
    return EvalConvert(c->Result()->Type(), c->Args()[0], SourceOf(c), tasks);
}

Evaluator::EvalResult Evaluator::EvalConvert(const core::type::Type* result_ty,
                                             core::ir::Value* arg,
                                             const Source& source,
                                             EvalTasks* tasks) {
    TINT_CHECK_RESULT_UNWRAP(val, EvalValue(arg, tasks));
    // Check if the value could be evaluated
    if (!val) {
        return nullptr;
    }
    auto r = const_eval_.Convert(result_ty, val, source);
    if (r != Success) {
        return EvalFailure::kError;
    }
    return r.Get();
}

Evaluator::EvalResult Evaluator::EvalOverride(core::ir::Override* o, EvalTasks* tasks) {
    if (!eval_override_) {
        return nullptr;
    }

    TINT_CHECK_RESULT_UNWRAP(val, EvalValue(o->Initializer(), tasks));
    // Check if the value could be evaluated
    if (!val) {
        return nullptr;
    }
    return val;
}

Evaluator::EvalResult Evaluator::EvalConstExprIf(core::ir::ConstExprIf* c, EvalTasks* tasks) {
    TINT_CHECK_RESULT_UNWRAP(val, EvalValue(c->Condition(), tasks));
    // Check if the value could be evaluated
    if (!val) {
        return nullptr;
    }
    bool branch_val = val->ValueAs<bool>();
    auto* inline_block = branch_val ? c->True() : c->False();

    // Note: ConstExprIf must be limited to side effect boolean values for this evaluation to be
    // correct. This is currently the case as ConstExprIf was created for the singular purpose of
    // correctly semantically representing short-circuiting operations (and/or).
    TINT_CHECK_RESULT_UNWRAP(ret, EvalValue(inline_block->Terminator()->Args()[0], tasks));
    // Check if the value could be evaluated
    if (!ret) {
        return nullptr;
    }

    return ret;
}

Evaluator::EvalResult Evaluator::EvalSwizzle(core::ir::Swizzle* s, EvalTasks* tasks) {
    return EvalSwizzle(s->Result()->Type(), s->Object(), s->Indices(), tasks);
}

Evaluator::EvalResult Evaluator::EvalSwizzle(const core::type::Type* result_ty,
                                             core::ir::Value* object,
                                             VectorRef<uint32_t> indices,
                                             EvalTasks* tasks) {
    TINT_CHECK_RESULT_UNWRAP(val, EvalValue(object, tasks));
    // Check if the value could be evaluated
    if (!val) {
        return nullptr;
    }

    auto r = const_eval_.Swizzle(result_ty, val, indices);
    if (r != Success) {
        return EvalFailure::kError;
    }
    return r.Get();
}

Evaluator::EvalResult Evaluator::EvalUnary(core::ir::CoreUnary* u, EvalTasks* tasks) {
    return EvalCoreUnary(u->Op(), u->Result()->Type(), u->Val(), SourceOf(u), tasks);
}

Evaluator::EvalResult Evaluator::EvalCoreUnary(UnaryOp op,
                                               const core::type::Type* result_ty,
                                               core::ir::Value* input,
                                               const Source& source,
                                               EvalTasks* tasks) {
    TINT_CHECK_RESULT_UNWRAP(val, EvalValue(input, tasks));
    // Check if the value could be evaluated
    if (!val) {
        return nullptr;
    }

    intrinsic::Context context{core::intrinsic::Dialect::kData, b_.ir.Types(), b_.ir.symbols};

    auto overload =
        core::intrinsic::LookupUnary(context, op, result_ty, core::EvaluationStage::kOverride);
    if (overload != Success) {
        AddError(source) << overload.Failure().Plain();
        return EvalFailure::kError;
    }

    auto const_eval_fn = overload->const_eval_fn;
    if (!const_eval_fn) {
        AddError(source) << "invalid unary expression";
        return EvalFailure::kError;
    }

    auto r = (const_eval_.*const_eval_fn)(result_ty, Vector{val}, source);
    if (r != Success) {
        return EvalFailure::kError;
    }
    return r.Get();
}

Evaluator::EvalResult Evaluator::EvalBinary(core::ir::CoreBinary* cb, EvalTasks* tasks) {
    return EvalCoreBinary(cb->Op(), cb->Result()->Type(), cb->LHS(), cb->RHS(), SourceOf(cb),
                          tasks);
}

Evaluator::EvalResult Evaluator::EvalCoreBinary(core::BinaryOp op,
                                                const core::type::Type* result_ty,
                                                core::ir::Value* lhs,
                                                core::ir::Value* rhs,
                                                const Source& source,
                                                EvalTasks* tasks) {
    // These short-circuiting operators should not be present in the IR at any time. These need
    // special handling and are transformed into ConstExprIfs on program construction.
    TINT_ASSERT(op != tint::core::BinaryOp::kLogicalAnd && op != tint::core::BinaryOp::kLogicalOr);

    TINT_CHECK_RESULT_UNWRAP(eval_lhs, EvalValue(lhs, tasks));
    // Check LHS could be evaluated
    if (!eval_lhs) {
        return nullptr;
    }

    TINT_CHECK_RESULT_UNWRAP(eval_rhs, EvalValue(rhs, tasks));
    // Check RHS could be evaluated
    if (!eval_rhs) {
        return nullptr;
    }

    intrinsic::Context context{core::intrinsic::Dialect::kData, b_.ir.Types(), b_.ir.symbols};

    auto overload =
        core::intrinsic::LookupBinary(context, op, lhs->Type(), rhs->Type(),
                                      core::EvaluationStage::kOverride, /* is_compound */ false);
    if (overload != Success) {
        AddError(source) << overload.Failure().Plain();
        return EvalFailure::kError;
    }

    auto const_eval_fn = overload->const_eval_fn;
    if (!const_eval_fn) {
        AddError(source) << "invalid binary expression";
        return EvalFailure::kError;
    }

    auto r = (const_eval_.*const_eval_fn)(result_ty, Vector{eval_lhs, eval_rhs}, source);
    if (r != Success) {
        return EvalFailure::kError;
    }
    return r.Get();
}

Evaluator::EvalResult Evaluator::EvalCoreBuiltinCall(core::ir::CoreBuiltinCall* c,
                                                     EvalTasks* tasks) {
    return EvalCoreBuiltinCall(c->Func(), c->Result()->Type(), c->Args(),
                               c->ExplicitTemplateParams(), SourceOf(c), tasks);
}

Evaluator::EvalResult Evaluator::EvalCoreBuiltinCall(
    core::BuiltinFn fn,
    const core::type::Type* result_ty,
    VectorRef<core::ir::Value*> args,
    VectorRef<core::ir::TemplateParameter> explicit_params,
    const Source& source,
    EvalTasks* tasks) {
    intrinsic::Context context{core::intrinsic::Dialect::kData, b_.ir.Types(), b_.ir.symbols};

    Vector<const core::type::Type*, 4> arg_types;
    arg_types.Reserve(args.Length());
    Vector<const core::constant::Value*, 4> arg_values;
    arg_values.Reserve(args.Length());
    for (auto* arg : args) {
        arg_types.Push(arg->Type());

        TINT_CHECK_RESULT_UNWRAP(val, EvalValue(arg, tasks));
        // Check if the value could be evaluated
        if (!val) {
            return nullptr;
        }
        arg_values.Push(val);
    }

    auto eval_stage =
        eval_override_ ? core::EvaluationStage::kOverride : core::EvaluationStage::kConstant;
    auto overload = core::intrinsic::LookupFn(context, core::str(fn), static_cast<size_t>(fn),
                                              explicit_params, arg_types, eval_stage);
    if (overload != Success) {
        AddError(source) << overload.Failure();
        return EvalFailure::kError;
    }

    // If there is no `@const` override, don't fail the eval, return a nullptr. This is
    // because eval can be called for things like `dpdx` which is not overridable but that's not an
    // eval failure, just don't eval.
    auto const_eval_fn = overload->const_eval_fn;
    if (!const_eval_fn) {
        return nullptr;
    }

    auto r = (const_eval_.*const_eval_fn)(result_ty, arg_values, source);
    if (r != Success) {
        return EvalFailure::kError;
    }
    return r.Get();
}

}  // namespace tint::core::ir
