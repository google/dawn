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

#include "src/tint/lang/core/ir/analysis/integer_range_analysis.h"

#include <limits>

#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/traverse.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::core::ir::analysis {

namespace {
/// Returns true if v is the integer constant 1.
bool IsOne(const Value* v) {
    if (auto* cv = v->As<Constant>()) {
        return Switch(
            cv->Type(),
            [&](const core::type::I32*) { return cv->Value()->ValueAs<int32_t>() == 1; },
            [&](const core::type::U32*) { return cv->Value()->ValueAs<uint32_t>() == 1; },
            [&](const Default) -> bool { return false; });
    }
    return false;
}

bool IsConstantInteger(const Value* v) {
    if (auto* cv = v->As<Constant>()) {
        return cv->Type()->IsIntegerScalar();
    }
    return false;
}

int64_t GetValueFromConstant(const Constant* value) {
    // Return an int64_t is enough as the type of `value` can only be either i32 or u32.
    [[maybe_unused]] bool is_i32_or_u32 = value->Type()->IsAnyOf<type::I32, type::U32>();
    TINT_ASSERT(is_i32_or_u32);
    return value->Value()->ValueAs<int64_t>();
}

struct CompareOpAndConstRHS {
    BinaryOp op;
    int64_t const_rhs = 0;
    const Binary* binary = nullptr;
};

IntegerRangeInfo ToIntegerRangeInfo(const Constant* constant,
                                    int64_t min_value,
                                    int64_t max_value) {
    if (constant->Type()->IsSignedIntegerScalar()) {
        return IntegerRangeInfo(min_value, max_value);
    } else {
        return IntegerRangeInfo(static_cast<uint64_t>(min_value), static_cast<uint64_t>(max_value));
    }
}

}  // namespace

IntegerRangeInfo::IntegerRangeInfo(int64_t min_bound, int64_t max_bound) {
    TINT_ASSERT(min_bound <= max_bound);
    range = SignedIntegerRange{min_bound, max_bound};
}

IntegerRangeInfo::IntegerRangeInfo(uint64_t min_bound, uint64_t max_bound) {
    TINT_ASSERT(min_bound <= max_bound);
    range = UnsignedIntegerRange{min_bound, max_bound};
}

struct IntegerRangeAnalysisImpl {
    explicit IntegerRangeAnalysisImpl(Module* ir_module) {
        for (Function* func : ir_module->functions) {
            // Analyze all the function parameters.
            AnalyzeFunctionParameters(func);

            // Analyze all of the loops in the function.
            Traverse(func->Block(), [&](Loop* l) { AnalyzeLoop(l); });
        }
    }

    const IntegerRangeInfo* GetInfo(const FunctionParam* param, uint32_t index) {
        const auto& range_info = integer_function_param_range_info_map_.Get(param);
        if (!range_info) {
            return nullptr;
        }
        TINT_ASSERT(range_info.value->Length() > index);
        return &(*range_info.value)[index];
    }

    const IntegerRangeInfo* GetInfo(const Var* var) {
        return integer_var_range_info_map_.Get(var).value;
    }

    const IntegerRangeInfo* GetInfo(const Load* load) {
        const InstructionResult* instruction = load->From()->As<InstructionResult>();
        if (!instruction) {
            return nullptr;
        }
        const Var* load_from_var = instruction->Instruction()->As<Var>();
        if (!load_from_var) {
            return nullptr;
        }
        return GetInfo(load_from_var);
    }

    const IntegerRangeInfo* GetInfo(const Access* access) {
        const Value* obj = access->Object();

        // Currently we only support the access to `local_invocation_id` or `local_invocation_index`
        // as a function parameter.
        const FunctionParam* function_param = obj->As<FunctionParam>();
        if (!function_param) {
            return nullptr;
        }
        if (access->Indices().Length() > 1) {
            return nullptr;
        }
        if (!access->Indices()[0]->As<Constant>()) {
            return nullptr;
        }
        uint32_t index =
            static_cast<uint32_t>(GetValueFromConstant(access->Indices()[0]->As<Constant>()));
        return GetInfo(function_param, index);
    }

    const IntegerRangeInfo* GetInfo(const Let* let) { return GetInfo(let->Value()); }

    const IntegerRangeInfo* GetInfo(const Constant* constant) {
        if (!IsConstantInteger(constant)) {
            return nullptr;
        }
        const IntegerRangeInfo& range_info =
            integer_constant_range_info_map_.GetOrAdd(constant, [&]() -> IntegerRangeInfo {
                int64_t const_value = GetValueFromConstant(constant);
                return ToIntegerRangeInfo(constant, const_value, const_value);
            });
        return &range_info;
    }

    const IntegerRangeInfo* GetInfo(const Convert* convert) {
        const IntegerRangeInfo* existing_range = integer_convert_range_info_map_.Get(convert).value;
        if (existing_range) {
            return existing_range;
        }

        auto* result_type = convert->Result()->Type();
        if (!result_type->IsIntegerScalar()) {
            return nullptr;
        }

        const auto* operand = convert->Operand(Convert::kValueOperandOffset);
        const IntegerRangeInfo* operand_range_info = GetInfo(operand);
        if (!operand_range_info) {
            return nullptr;
        }
        auto* operand_type = operand->Type();
        TINT_ASSERT(operand_type->IsIntegerScalar());

        if (operand_type == result_type) {
            return operand_range_info;
        }

        if (std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(
                operand_range_info->range)) {
            // result = convert<u32>(operand), operand cannot be negative.
            TINT_ASSERT(result_type->As<type::U32>());
            const auto& range =
                std::get<IntegerRangeInfo::SignedIntegerRange>(operand_range_info->range);
            if (range.min_bound < 0) {
                return nullptr;
            }
            auto result = integer_convert_range_info_map_.Add(
                convert, IntegerRangeInfo(static_cast<uint64_t>(range.min_bound),
                                          static_cast<uint64_t>(range.max_bound)));
            return &result.value;
        } else {
            // result = convert<i32>(operand), operand cannot be greater than `i32::kHighestValue`.
            TINT_ASSERT(result_type->As<type::I32>());
            const auto& range =
                std::get<IntegerRangeInfo::UnsignedIntegerRange>(operand_range_info->range);
            if (range.max_bound > i32::kHighestValue) {
                return nullptr;
            }
            auto result = integer_convert_range_info_map_.Add(
                convert, IntegerRangeInfo(static_cast<int64_t>(range.min_bound),
                                          static_cast<int64_t>(range.max_bound)));
            return &result.value;
        }
    }

    const IntegerRangeInfo* GetInfo(const Value* value) {
        return Switch(
            value, [&](const Constant* constant) { return GetInfo(constant); },
            [&](const FunctionParam* param) -> const IntegerRangeInfo* {
                if (!param->Type()->IsIntegerScalar()) {
                    return nullptr;
                }
                return GetInfo(param, 0);
            },
            [&](const InstructionResult* r) {
                // TODO(348701956): Support more instruction types
                return Switch(
                    r->Instruction(), [&](const Var* var) { return GetInfo(var); },
                    [&](const Load* load) { return GetInfo(load); },
                    [&](const Access* access) { return GetInfo(access); },
                    [&](const Let* let) { return GetInfo(let); },
                    [&](const Binary* binary) { return GetInfo(binary); },
                    [&](const Convert* convert) { return GetInfo(convert); },
                    [](Default) { return nullptr; });
            },
            [](Default) { return nullptr; });
    }

    const IntegerRangeInfo* GetInfo(const Binary* binary) {
        const IntegerRangeInfo* existing_range = integer_binary_range_info_map_.Get(binary).value;
        if (existing_range) {
            return existing_range;
        }

        const IntegerRangeInfo* range_lhs = GetInfo(binary->LHS());
        if (!range_lhs) {
            return nullptr;
        }
        const IntegerRangeInfo* range_rhs = GetInfo(binary->RHS());
        if (!range_rhs) {
            return nullptr;
        }

        // TODO(348701956): Support more binary operators
        switch (binary->Op()) {
            case BinaryOp::kAdd:
                return ComputeAndCacheIntegerRangeForBinaryAdd(binary, range_lhs, range_rhs);

            case BinaryOp::kSubtract:
                return ComputeAndCacheIntegerRangeForBinarySubtract(binary, range_lhs, range_rhs);

            case BinaryOp::kMultiply:
                return ComputeAndCacheIntegerRangeForBinaryMultiply(binary, range_lhs, range_rhs);

            case BinaryOp::kDivide:
                return ComputeAndCacheIntegerRangeForBinaryDivide(binary, range_lhs, range_rhs);

            default:
                return nullptr;
        }
    }

    /// Analyze a loop to compute the range of the loop control variable if possible.
    void AnalyzeLoop(const Loop* loop) {
        const Var* index = GetLoopControlVariableFromConstantInitializer(loop);
        if (!index) {
            return;
        }
        const Binary* update = GetBinaryToUpdateLoopControlVariableInContinuingBlock(loop, index);
        if (!update) {
            return;
        }
        CompareOpAndConstRHS compare_info =
            GetCompareInfoOfLoopControlVariableInLoopBody(loop, index);
        if (!compare_info.binary) {
            return;
        }

        TINT_ASSERT(index->Initializer());
        TINT_ASSERT(index->Initializer()->As<Constant>());

        // for (var i = const_init; ...)
        const Constant* constant_initializer = index->Initializer()->As<Constant>();
        int64_t const_init = GetValueFromConstant(constant_initializer);

        // for (...; i++) or for(...; i--)
        bool index_is_increasing = update->Op() == BinaryOp::kAdd;

        switch (compare_info.op) {
            case BinaryOp::kLessThanEqual: {
                // for (var index = const_init; index <= const_rhs; index++)
                // Only `const_init <= const_rhs` is valid. The range of `index` is:
                // [const_init, const_rhs]
                // Note that in `GetBinaryToCompareLoopControlVariableInLoopBody()` we disallow
                // `const_rhs` to be the maximum value of `i32` or `u32`, so:
                // - `index + 1` won't be overflow as `const_init` cannot be greater than
                //   `const_rhs`.
                // - `index <= const_rhs` can correctly exit when `const_init + 1` is the maximum
                //   value of `i32` or `u32`.
                if (index_is_increasing && const_init <= compare_info.const_rhs) {
                    IntegerRangeInfo range_info = ToIntegerRangeInfo(
                        constant_initializer, const_init, compare_info.const_rhs);
                    integer_var_range_info_map_.Add(index, range_info);
                }
                break;
            }
            case BinaryOp::kGreaterThanEqual: {
                // for (var index = const_init; index >= const_rhs; index--)
                // Only `const_init >= const_rhs` is valid. The range of `index` is:
                // [const_rhs, const_init]
                // Note that in `GetBinaryToCompareLoopControlVariableInLoopBody()` we disallow
                // `const_rhs` to be the minimum value of `i32` or `u32`, so:
                // - `index - 1` won't be underflow as `const_init` cannot be less than `const_rhs`.
                // - `index >= const_rhs` can correctly exit when `const_init - 1` is the minimum
                //   value of `i32` or `u32`.
                if (!index_is_increasing && const_init >= compare_info.const_rhs) {
                    IntegerRangeInfo range_info = ToIntegerRangeInfo(
                        constant_initializer, compare_info.const_rhs, const_init);
                    integer_var_range_info_map_.Add(index, range_info);
                }
                break;
            }
            default:
                break;
        }
    }

    const Var* GetLoopControlVariableFromConstantInitializer(const Loop* loop) {
        TINT_ASSERT(loop);

        auto* init_block = loop->Initializer();
        if (!init_block) {
            return nullptr;
        }

        // Currently we only support the loop initializer of a simple for-loop, which only has two
        // instructions
        // - The first instruction is to initialize the loop control variable
        //   with a constant integer (signed or unsigned) value.
        // - The second instruction is `next_iteration`
        // e.g. for (var i = 0; ...)
        if (init_block->Length() != 2u) {
            return nullptr;
        }

        auto* var = init_block->Front()->As<Var>();
        if (!var) {
            return nullptr;
        }

        if (!init_block->Back()->As<NextIteration>()) {
            return nullptr;
        }

        const auto* pointer = var->Result()->Type()->As<core::type::Pointer>();
        if (!pointer->StoreType()->IsIntegerScalar()) {
            return nullptr;
        }

        const auto* initializer = var->Initializer();
        if (!initializer) {
            return nullptr;
        }

        if (!initializer->As<Constant>()) {
            return nullptr;
        }

        return var;
    }

    // Currently we only support the loop continuing of a simple for-loop, which only has 4
    // instructions
    /// - The first instruction is to load the loop control variable into a temporary variable.
    /// - The second instruction is to add one or minus one to the temporary variable.
    /// - The third instruction is to store the value of the temporary variable into the loop
    ///   control variable.
    /// - The fourth instruction is `next_iteration`.
    const Binary* GetBinaryToUpdateLoopControlVariableInContinuingBlock(
        const Loop* loop,
        const Var* loop_control_variable) {
        TINT_ASSERT(loop);
        TINT_ASSERT(loop_control_variable);

        auto* continuing_block = loop->Continuing();
        if (!continuing_block) {
            return nullptr;
        }

        if (continuing_block->Length() != 4u) {
            return nullptr;
        }

        // 1st instruction:
        // %src = load %loop_control_variable
        const auto* load_from_loop_control_variable = continuing_block->Instructions()->As<Load>();
        if (!load_from_loop_control_variable) {
            return nullptr;
        }
        if (load_from_loop_control_variable->From() != loop_control_variable->Result()) {
            return nullptr;
        }

        // 2nd instruction:
        // %dst = add %src, 1
        // or %dst = add 1, %src
        // or %dst = sub %src, 1
        const auto* add_or_sub_from_loop_control_variable =
            load_from_loop_control_variable->next->As<Binary>();
        if (!add_or_sub_from_loop_control_variable) {
            return nullptr;
        }
        const auto* src = load_from_loop_control_variable->Result();
        const auto* lhs = add_or_sub_from_loop_control_variable->LHS();
        const auto* rhs = add_or_sub_from_loop_control_variable->RHS();
        switch (add_or_sub_from_loop_control_variable->Op()) {
            case BinaryOp::kAdd: {
                // %dst = add %src, 1
                if (lhs == src && IsOne(rhs)) {
                    break;
                }
                // %dst = add 1, %src
                if (rhs == src && IsOne(lhs)) {
                    break;
                }
                return nullptr;
            }
            case BinaryOp::kSubtract: {
                // %dst = sub %src, 1
                if (lhs == src && IsOne(rhs)) {
                    break;
                }
                return nullptr;
            }
            default:
                return nullptr;
        }

        // 3rd instruction:
        // store %loop_control_variable, %dst
        const auto* store_into_loop_control_variable =
            add_or_sub_from_loop_control_variable->next->As<Store>();
        if (!store_into_loop_control_variable) {
            return nullptr;
        }
        const auto* dst = add_or_sub_from_loop_control_variable->Result();
        if (store_into_loop_control_variable->From() != dst) {
            return nullptr;
        }
        if (store_into_loop_control_variable->To() != loop_control_variable->Result()) {
            return nullptr;
        }

        // 4th instruction:
        // next_iteration
        if (!store_into_loop_control_variable->next->As<NextIteration>()) {
            return nullptr;
        }

        return add_or_sub_from_loop_control_variable;
    }

    // Currently we only support the loop continuing of a simple for-loop which meets all the below
    // requirements:
    // - The loop control variable is only used as the parameter of the load instruction.
    // - The first instruction is to load the loop control variable into a temporary variable.
    // - The second instruction is a `compare` binary to compare the temporary variable with a
    //   constant value and save the result to a boolean variable.
    // - The second instruction cannot be a comparison that will never return true.
    // - The third instruction is an `ifelse` expression that uses the boolean variable got in the
    //   second instruction as the condition.
    // - The true block of the above `ifelse` expression doesn't contain `exit_loop`.
    // - The false block of the above `ifelse` expression only contains `exit_loop`.
    // A valid `compare` binary instruction (the second instruction) can be in one of the below
    // formats:
    // 1. variable >= constant variable <= constant
    // 2. variable > constant variable < constant
    // 3. constant >= variable constant <= variable
    // 4. constant > variable constant < variable
    // This function analyzes the `compare` binary and returns both `variable` and `constant` in the
    // equivalent format of format 1 and a pointer to the `compare` binary in a
    // `CompareOpAndConstRHS`struct when the input `loop` meets all the above requirements.
    // `CompareOpAndConstRHS.binary` will be set to nullptr otherwise.
    CompareOpAndConstRHS GetCompareInfoOfLoopControlVariableInLoopBody(
        const Loop* loop,
        const Var* loop_control_variable) {
        TINT_ASSERT(loop);
        TINT_ASSERT(loop_control_variable);

        auto* body_block = loop->Body();

        CompareOpAndConstRHS compare_info = {};

        // Reject any non-load instructions unless it is a store in the continuing block
        const auto& uses = loop_control_variable->Result(0)->UsagesUnsorted();
        for (auto& use : uses) {
            if (use->instruction->Is<Load>()) {
                continue;
            }
            if (use->instruction->Is<Store>() && use->instruction->Block() == loop->Continuing()) {
                continue;
            }
            return compare_info;
        }

        // 1st instruction:
        // %src = load %loop_control_variable
        const auto* load_from_loop_control_variable = body_block->Instructions()->As<Load>();
        if (!load_from_loop_control_variable) {
            return compare_info;
        }
        if (load_from_loop_control_variable->From() != loop_control_variable->Result(0)) {
            return compare_info;
        }

        // 2nd instruction:
        // %condition:bool = lt(gt, lte, gte) %src, constant_value
        // or %condition:bool = lt(gt, lte, gte) constant_value, %src
        const auto* exit_condition_on_loop_control_variable =
            load_from_loop_control_variable->next->As<Binary>();
        if (!exit_condition_on_loop_control_variable) {
            return compare_info;
        }
        BinaryOp op = exit_condition_on_loop_control_variable->Op();
        auto* lhs = exit_condition_on_loop_control_variable->LHS();
        auto* rhs = exit_condition_on_loop_control_variable->RHS();
        switch (op) {
            case BinaryOp::kLessThan:
            case BinaryOp::kGreaterThan:
            case BinaryOp::kLessThanEqual:
            case BinaryOp::kGreaterThanEqual: {
                if (IsConstantInteger(rhs) && lhs == load_from_loop_control_variable->Result(0)) {
                    break;
                }
                if (IsConstantInteger(lhs) && rhs == load_from_loop_control_variable->Result(0)) {
                    break;
                }
                return compare_info;
            }
            default:
                return compare_info;
        }

        const bool is_signed = lhs->Type()->IsSignedIntegerScalar();
        auto const_i32 = [](const Value* val) {
            return val->As<Constant>()->Value()->ValueAs<int32_t>();
        };
        auto const_u32 = [](const Value* val) {
            return val->As<Constant>()->Value()->ValueAs<uint32_t>();
        };

        // Check and extract necessary information from `binary`.
        // Note that we should early return when the comparison will never return true.
        // e.g. for (...; i < 0u; ...)  // The loop body will never be executed.
        // We should also early return when the comparison will always return true, which means the
        // loop exit condition takes no effect and the loop control variable can actually take all
        // the numbers.
        // e.g. for (i = 100u; i >= 0u; i--) // `i` can be any u32 value instead of [0u, 100u]
        if (IsConstantInteger(rhs)) {
            // Handle `binary` in the format of `variable op const_rhs`
            int64_t const_rhs = GetValueFromConstant(rhs->As<Constant>());

            switch (op) {
                case BinaryOp::kLessThan: {
                    // variable < std::numeric_limits<uint32_t>::min()
                    if (!is_signed && const_u32(rhs) == u32::kLowestValue) {
                        return compare_info;
                    }
                    // variable < std::numeric_limits<int32_t>::min()
                    if (is_signed && const_i32(rhs) == i32::kLowestValue) {
                        return compare_info;
                    }

                    // variable < constant => variable <= constant - 1
                    // `const_rhs - 1` will always be safe after the above checks.
                    compare_info.op = BinaryOp::kLessThanEqual;
                    compare_info.const_rhs = const_rhs - 1;
                    break;
                }

                case BinaryOp::kGreaterThan: {
                    // variable > std::numeric_limits<uint32_t>::max()
                    if (!is_signed && const_u32(rhs) == u32::kHighestValue) {
                        return compare_info;
                    }
                    // variable > std::numeric_limits<int32_t>::max()
                    if (is_signed && const_i32(rhs) == i32::kHighestValue) {
                        return compare_info;
                    }

                    // variable > constant => variable >= constant + 1
                    // `const_rhs` will always be safe after the above checks.
                    compare_info.op = BinaryOp::kGreaterThanEqual;
                    compare_info.const_rhs = const_rhs + 1;
                    break;
                }

                case BinaryOp::kLessThanEqual: {
                    // variable <= std::numeric_limits<uint32_t>::max()
                    if (!is_signed && const_u32(rhs) == u32::kHighestValue) {
                        return compare_info;
                    }
                    // variable <= std::numeric_limits<int32_t>::max()
                    if (is_signed && const_i32(rhs) == i32::kHighestValue) {
                        return compare_info;
                    }

                    compare_info.op = op;
                    compare_info.const_rhs = const_rhs;
                    break;
                }
                case BinaryOp::kGreaterThanEqual: {
                    // variable >= std::numeric_limits<uint32_t>::min()
                    if (!is_signed && const_u32(rhs) == u32::kLowestValue) {
                        return compare_info;
                    }
                    // variable >= std::numeric_limits<int32_t>::min()
                    if (is_signed && const_i32(rhs) == i32::kLowestValue) {
                        return compare_info;
                    }

                    compare_info.op = op;
                    compare_info.const_rhs = const_rhs;
                    break;
                }
                default:
                    TINT_UNREACHABLE();
            }
        } else {
            // Handle `binary` in the format of `const_lhs op variable`
            TINT_ASSERT(IsConstantInteger(lhs));
            int64_t const_lhs = GetValueFromConstant(lhs->As<Constant>());

            switch (op) {
                case BinaryOp::kLessThan: {
                    // std::numeric_limits<uint32_t>::max() < variable
                    if (!is_signed && const_u32(lhs) == u32::kHighestValue) {
                        return compare_info;
                    }
                    // std::numeric_limits<int32_t>::max() < variable
                    if (is_signed && const_i32(lhs) == i32::kHighestValue) {
                        return compare_info;
                    }

                    // constant < variable => variable > constant => variable >= constant + 1
                    // `const_lhs + 1` will always be safe after the above checks.
                    compare_info.op = BinaryOp::kGreaterThanEqual;
                    compare_info.const_rhs = const_lhs + 1;
                    break;
                }

                case BinaryOp::kGreaterThan: {
                    // std::numeric_limits<uint32_t>::min() > variable
                    if (!is_signed && const_u32(lhs) == u32::kLowestValue) {
                        return compare_info;
                    }
                    // std::numeric_limits<int32_t>::min() > variable
                    if (is_signed && const_i32(lhs) == i32::kLowestValue) {
                        return compare_info;
                    }

                    // constant > variable => variable < constant => variable <= constant - 1
                    // `const_lhs - 1` will always be safe after the above checks.
                    compare_info.op = BinaryOp::kLessThanEqual;
                    compare_info.const_rhs = const_lhs - 1;
                    break;
                }

                case BinaryOp::kLessThanEqual: {
                    // std::numeric_limits<uint32_t>::min() <= variable
                    if (!is_signed && const_u32(lhs) == u32::kLowestValue) {
                        return compare_info;
                    }
                    // std::numeric_limits<int32_t>::min() <= variable
                    if (is_signed && const_i32(lhs) == i32::kLowestValue) {
                        return compare_info;
                    }

                    // constant <= variable => variable >= constant
                    compare_info.op = BinaryOp::kGreaterThanEqual;
                    compare_info.const_rhs = const_lhs;
                    break;
                }

                case BinaryOp::kGreaterThanEqual: {
                    // std::numeric_limits<uint32_t>::max() >= variable
                    if (!is_signed && const_u32(lhs) == u32::kHighestValue) {
                        return compare_info;
                    }
                    // std::numeric_limits<int32_t>::max() >= variable
                    if (is_signed && const_i32(lhs) == i32::kHighestValue) {
                        return compare_info;
                    }

                    // constant >= variable => variable <= constant
                    compare_info.op = BinaryOp::kLessThanEqual;
                    compare_info.const_rhs = const_lhs;
                    break;
                }

                default:
                    TINT_UNREACHABLE();
            }
        }

        // 3rd instruction:
        // if %condition [t: $true, f: $false] {
        //   $true: {
        //     // Maybe some other instructions
        //     exit_if
        //   }
        //   $false: { exit_loop }
        // }
        const auto* if_on_exit_condition = exit_condition_on_loop_control_variable->next->As<If>();
        if (!if_on_exit_condition) {
            return compare_info;
        }
        if (if_on_exit_condition->Condition() !=
            exit_condition_on_loop_control_variable->Result(0)) {
            return compare_info;
        }
        if (!if_on_exit_condition->True()->Terminator()->As<ExitIf>()) {
            return compare_info;
        }
        const auto* front_inst = if_on_exit_condition->False()->Front();
        if (!front_inst || !front_inst->As<ExitLoop>()) {
            return compare_info;
        }

        // Set `compare_info.binary` after the loop passes all the above checks.
        compare_info.binary = exit_condition_on_loop_control_variable;
        return compare_info;
    }

  private:
    void AnalyzeFunctionParameters(const Function* func) {
        for (const auto* param : func->Params()) {
            if (!param->Type()->IsIntegerScalarOrVector()) {
                return;
            }

            // Currently we only support the query of ranges of `local_invocation_id` or
            // `local_invocation_index`.
            if (!param->Builtin()) {
                return;
            }

            switch (*param->Builtin()) {
                case BuiltinValue::kLocalInvocationIndex: {
                    // We shouldn't be trying to use range analysis on a module that has
                    // non-constant workgroup sizes, since we will always have replaced pipeline
                    // overrides with constant values early in the pipeline.
                    TINT_ASSERT(func->WorkgroupSizeAsConst().has_value());
                    std::array<uint32_t, 3> workgroup_size = func->WorkgroupSizeAsConst().value();
                    uint64_t max_bound =
                        workgroup_size[0] * workgroup_size[1] * workgroup_size[2] - 1u;
                    constexpr uint64_t kMinBound = 0;
                    integer_function_param_range_info_map_.Add(
                        param,
                        Vector<IntegerRangeInfo, 3>({IntegerRangeInfo(kMinBound, max_bound)}));
                    break;
                }
                case BuiltinValue::kLocalInvocationId: {
                    TINT_ASSERT(func->WorkgroupSizeAsConst().has_value());
                    std::array<uint32_t, 3> workgroup_size = func->WorkgroupSizeAsConst().value();

                    constexpr uint64_t kMinBound = 0;
                    Vector<IntegerRangeInfo, 3> integerRanges;
                    for (uint32_t size_x_y_z : workgroup_size) {
                        integerRanges.Push({kMinBound, size_x_y_z - 1u});
                    }
                    integer_function_param_range_info_map_.Add(param, integerRanges);
                }

                break;
                default:
                    break;
            }
        }
    }

    const IntegerRangeInfo* ComputeAndCacheIntegerRangeForBinaryAdd(const Binary* binary,
                                                                    const IntegerRangeInfo* lhs,
                                                                    const IntegerRangeInfo* rhs) {
        // Add two 32-bit signed integer values saved in int64_t. Return {} when either overflow or
        // underflow happens.
        auto SafeAddI32 = [](int64_t a, int64_t b) -> std::optional<int64_t> {
            TINT_ASSERT(a >= i32::kLowestValue && a <= i32::kHighestValue);
            TINT_ASSERT(b >= i32::kLowestValue && b <= i32::kHighestValue);

            int64_t sum = a + b;
            if (sum > i32::kHighestValue || sum < i32::kLowestValue) {
                return {};
            }
            return sum;
        };

        // No-overflow add between two 32-bit unsigned integer values saved in uint64_t. Return {}
        // when overflow happens.
        auto SafeAddU32 = [](uint64_t a, uint64_t b) -> std::optional<uint64_t> {
            TINT_ASSERT(a <= u32::kHighestValue);
            TINT_ASSERT(b <= u32::kHighestValue);

            uint64_t sum = a + b;
            if (sum > u32::kHighestValue) {
                return {};
            }
            return sum;
        };

        if (std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(lhs->range)) {
            auto lhs_i32 = std::get<IntegerRangeInfo::SignedIntegerRange>(lhs->range);
            auto rhs_i32 = std::get<IntegerRangeInfo::SignedIntegerRange>(rhs->range);

            // [min1, max1] + [min2, max2] => [min1 + min2, max1 + max2]
            std::optional<int64_t> min_bound = SafeAddI32(lhs_i32.min_bound, rhs_i32.min_bound);
            std::optional<int64_t> max_bound = SafeAddI32(lhs_i32.max_bound, rhs_i32.max_bound);
            if (!min_bound.has_value() || !max_bound.has_value()) {
                return nullptr;
            }
            auto result = integer_binary_range_info_map_.Add(
                binary, IntegerRangeInfo(*min_bound, *max_bound));
            return &result.value;
        } else {
            auto lhs_u32 = std::get<IntegerRangeInfo::UnsignedIntegerRange>(lhs->range);
            auto rhs_u32 = std::get<IntegerRangeInfo::UnsignedIntegerRange>(rhs->range);

            // [min1, max1] + [min2, max2] => [min1 + min2, max1 + max2]
            std::optional<uint64_t> min_bound = SafeAddU32(lhs_u32.min_bound, rhs_u32.min_bound);
            std::optional<uint64_t> max_bound = SafeAddU32(lhs_u32.max_bound, rhs_u32.max_bound);
            if (!min_bound || !max_bound) {
                return nullptr;
            }
            auto result = integer_binary_range_info_map_.Add(
                binary, IntegerRangeInfo(*min_bound, *max_bound));
            return &result.value;
        }
    }

    const IntegerRangeInfo* ComputeAndCacheIntegerRangeForBinarySubtract(
        const Binary* binary,
        const IntegerRangeInfo* lhs,
        const IntegerRangeInfo* rhs) {
        // Subtract two 32-bit signed integer values saved in int64_t. Return {} when either
        // overflow or underflow happens.
        auto SafeSubtractI32 = [](int64_t a, int64_t b) -> std::optional<int64_t> {
            TINT_ASSERT(a >= i32::kLowestValue && a <= i32::kHighestValue);
            TINT_ASSERT(b >= i32::kLowestValue && b <= i32::kHighestValue);

            int64_t diff = a - b;
            if (diff > i32::kHighestValue || diff < i32::kLowestValue) {
                return {};
            }
            return diff;
        };

        // No-underflow Subtract between two 32-bit unsigned integer values saved in uint64_t.
        // Return {} when underflow happens.
        auto SafeSubtractU32 = [](uint64_t a, uint64_t b) -> std::optional<uint64_t> {
            TINT_ASSERT(a <= u32::kHighestValue);
            TINT_ASSERT(b <= u32::kHighestValue);

            if (a < b) {
                return {};
            }
            return a - b;
        };

        if (std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(lhs->range)) {
            auto lhs_i32 = std::get<IntegerRangeInfo::SignedIntegerRange>(lhs->range);
            auto rhs_i32 = std::get<IntegerRangeInfo::SignedIntegerRange>(rhs->range);

            // [min1, max1] - [min2, max2] => [min1 - max2, max1 - min2]
            std::optional<int64_t> min_bound =
                SafeSubtractI32(lhs_i32.min_bound, rhs_i32.max_bound);
            std::optional<int64_t> max_bound =
                SafeSubtractI32(lhs_i32.max_bound, rhs_i32.min_bound);
            if (!min_bound.has_value() || !max_bound.has_value()) {
                return nullptr;
            }
            auto result = integer_binary_range_info_map_.Add(
                binary, IntegerRangeInfo(*min_bound, *max_bound));
            return &result.value;
        } else {
            auto lhs_u32 = std::get<IntegerRangeInfo::UnsignedIntegerRange>(lhs->range);
            auto rhs_u32 = std::get<IntegerRangeInfo::UnsignedIntegerRange>(rhs->range);

            // [min1, max1] - [min2, max2] => [min1 - max2, max1 - min2]
            std::optional<uint64_t> min_bound =
                SafeSubtractU32(lhs_u32.min_bound, rhs_u32.max_bound);
            std::optional<uint64_t> max_bound =
                SafeSubtractU32(lhs_u32.max_bound, rhs_u32.min_bound);
            if (!min_bound || !max_bound) {
                return nullptr;
            }
            auto result = integer_binary_range_info_map_.Add(
                binary, IntegerRangeInfo(*min_bound, *max_bound));
            return &result.value;
        }
    }

    const IntegerRangeInfo* ComputeAndCacheIntegerRangeForBinaryMultiply(
        const Binary* binary,
        const IntegerRangeInfo* lhs,
        const IntegerRangeInfo* rhs) {
        // Multiply two 32-bit non-negative signed integer values saved in int64_t. Return {} when
        // overflow happens.
        auto SafeMultiplyNonNegativeI32 = [](int64_t a, int64_t b) -> std::optional<int64_t> {
            TINT_ASSERT(a >= 0 && a <= i32::kHighestValue);
            TINT_ASSERT(b >= 0 && b <= i32::kHighestValue);

            int64_t multiply = a * b;
            if (multiply > i32::kHighestValue) {
                return {};
            }
            return multiply;
        };

        // No-underflow multiply two 32-bit unsigned integer values saved in uint64_t.
        // Return {} when underflow happens.
        auto SafeMultiplyU32 = [](uint64_t a, uint64_t b) -> std::optional<uint64_t> {
            TINT_ASSERT(a <= u32::kHighestValue);
            TINT_ASSERT(b <= u32::kHighestValue);

            uint64_t multiply = a * b;
            if (multiply > u32::kHighestValue) {
                return {};
            }
            return multiply;
        };

        if (std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(lhs->range)) {
            auto lhs_i32 = std::get<IntegerRangeInfo::SignedIntegerRange>(lhs->range);
            auto rhs_i32 = std::get<IntegerRangeInfo::SignedIntegerRange>(rhs->range);

            // Currently we only handle multiplication with non-negative values, which means
            // 0 <= min_bound <= max_bound
            if (lhs_i32.min_bound < 0 || rhs_i32.min_bound < 0) {
                return nullptr;
            }

            // min1 >= 0, min2 >= 0
            // [min1, max1] * [min2, max2] => [min1 * min2, max1 * max2]
            std::optional<int64_t> min_bound =
                SafeMultiplyNonNegativeI32(lhs_i32.min_bound, rhs_i32.min_bound);
            std::optional<int64_t> max_bound =
                SafeMultiplyNonNegativeI32(lhs_i32.max_bound, rhs_i32.max_bound);
            if (!min_bound.has_value() || !max_bound.has_value()) {
                return nullptr;
            }
            auto result = integer_binary_range_info_map_.Add(
                binary, IntegerRangeInfo(*min_bound, *max_bound));
            return &result.value;
        } else {
            auto lhs_u32 = std::get<IntegerRangeInfo::UnsignedIntegerRange>(lhs->range);
            auto rhs_u32 = std::get<IntegerRangeInfo::UnsignedIntegerRange>(rhs->range);

            // [min1, max1] * [min2, max2] => [min1 * min2, max1 * max2]
            std::optional<uint64_t> min_bound =
                SafeMultiplyU32(lhs_u32.min_bound, rhs_u32.min_bound);
            std::optional<uint64_t> max_bound =
                SafeMultiplyU32(lhs_u32.max_bound, rhs_u32.max_bound);
            if (!min_bound || !max_bound) {
                return nullptr;
            }
            auto result = integer_binary_range_info_map_.Add(
                binary, IntegerRangeInfo(*min_bound, *max_bound));
            return &result.value;
        }
    }

    const IntegerRangeInfo* ComputeAndCacheIntegerRangeForBinaryDivide(
        const Binary* binary,
        const IntegerRangeInfo* lhs,
        const IntegerRangeInfo* rhs) {
        if (std::holds_alternative<IntegerRangeInfo::SignedIntegerRange>(lhs->range)) {
            auto lhs_i32 = std::get<IntegerRangeInfo::SignedIntegerRange>(lhs->range);
            auto rhs_i32 = std::get<IntegerRangeInfo::SignedIntegerRange>(rhs->range);

            // Currently we require `lhs` must be non-negative, and `rhs` must be positive.
            // 0 <= lhs.min_bound <= lhs.max_bound
            if (lhs_i32.min_bound < 0) {
                return nullptr;
            }
            // 0 < rhs.min_bound <= rhs.max_bound
            if (rhs_i32.min_bound <= 0) {
                return nullptr;
            }

            // min1 >= 0, min2 > 0
            // [min1, max1] / [min2, max2] => [min1 / max2, max1 / min2]
            int64_t min_bound = lhs_i32.min_bound / rhs_i32.max_bound;
            int64_t max_bound = lhs_i32.max_bound / rhs_i32.min_bound;
            auto result =
                integer_binary_range_info_map_.Add(binary, IntegerRangeInfo(min_bound, max_bound));
            return &result.value;
        } else {
            auto lhs_u32 = std::get<IntegerRangeInfo::UnsignedIntegerRange>(lhs->range);
            auto rhs_u32 = std::get<IntegerRangeInfo::UnsignedIntegerRange>(rhs->range);

            // `rhs` must be positive.
            if (rhs_u32.min_bound == 0) {
                return nullptr;
            }

            // [min1, max1] / [min2, max2] => [min1 / max2, max1 / min2]
            uint64_t min_bound = lhs_u32.min_bound / rhs_u32.max_bound;
            uint64_t max_bound = lhs_u32.max_bound / rhs_u32.min_bound;
            auto result =
                integer_binary_range_info_map_.Add(binary, IntegerRangeInfo(min_bound, max_bound));
            return &result.value;
        }
    }

    Hashmap<const FunctionParam*, Vector<IntegerRangeInfo, 3>, 4>
        integer_function_param_range_info_map_;
    Hashmap<const Var*, IntegerRangeInfo, 8> integer_var_range_info_map_;
    Hashmap<const Constant*, IntegerRangeInfo, 8> integer_constant_range_info_map_;
    Hashmap<const Binary*, IntegerRangeInfo, 8> integer_binary_range_info_map_;
    Hashmap<const Convert*, IntegerRangeInfo, 8> integer_convert_range_info_map_;
};

IntegerRangeAnalysis::IntegerRangeAnalysis(Module* ir_module)
    : impl_(new IntegerRangeAnalysisImpl(ir_module)) {}

IntegerRangeAnalysis::~IntegerRangeAnalysis() = default;

const IntegerRangeInfo* IntegerRangeAnalysis::GetInfo(const FunctionParam* param, uint32_t index) {
    return impl_->GetInfo(param, index);
}

const Var* IntegerRangeAnalysis::GetLoopControlVariableFromConstantInitializerForTest(
    const Loop* loop) {
    return impl_->GetLoopControlVariableFromConstantInitializer(loop);
}

const Binary* IntegerRangeAnalysis::GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(
    const Loop* loop,
    const Var* loop_control_variable) {
    return impl_->GetBinaryToUpdateLoopControlVariableInContinuingBlock(loop,
                                                                        loop_control_variable);
}

const Binary* IntegerRangeAnalysis::GetBinaryToCompareLoopControlVariableInLoopBodyForTest(
    const Loop* loop,
    const Var* loop_control_variable) {
    return impl_->GetCompareInfoOfLoopControlVariableInLoopBody(loop, loop_control_variable).binary;
}

const IntegerRangeInfo* IntegerRangeAnalysis::GetInfo(const Var* var) {
    return impl_->GetInfo(var);
}

const IntegerRangeInfo* IntegerRangeAnalysis::GetInfo(const Load* load) {
    return impl_->GetInfo(load);
}

const IntegerRangeInfo* IntegerRangeAnalysis::GetInfo(const Access* access) {
    return impl_->GetInfo(access);
}

const IntegerRangeInfo* IntegerRangeAnalysis::GetInfo(const Constant* constant) {
    return impl_->GetInfo(constant);
}

const IntegerRangeInfo* IntegerRangeAnalysis::GetInfo(const Value* value) {
    return impl_->GetInfo(value);
}

const IntegerRangeInfo* IntegerRangeAnalysis::GetInfo(const Binary* binary) {
    return impl_->GetInfo(binary);
}

const IntegerRangeInfo* IntegerRangeAnalysis::GetInfo(const Let* let) {
    return impl_->GetInfo(let);
}

const IntegerRangeInfo* IntegerRangeAnalysis::GetInfo(const Convert* convert) {
    return impl_->GetInfo(convert);
}

}  // namespace tint::core::ir::analysis
