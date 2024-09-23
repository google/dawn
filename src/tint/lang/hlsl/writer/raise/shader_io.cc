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

#include "src/tint/lang/hlsl/writer/raise/shader_io.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/transform/shader_io.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/hlsl/builtin_fn.h"
#include "src/tint/lang/hlsl/ir/builtin_call.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {

namespace {

/// PIMPL state for the parts of the shader IO transform specific to HLSL.
/// For HLSL, move all inputs to a struct passed as an entry point parameter, and wrap outputs in
/// a structure returned by the entry point.
struct StateImpl : core::ir::transform::ShaderIOBackendState {
    /// The config
    const ShaderIOConfig& config;

    /// The input parameter
    core::ir::FunctionParam* input_param = nullptr;

    Vector<uint32_t, 4> input_indices;
    Vector<uint32_t, 4> output_indices;

    /// The output struct type.
    core::type::Struct* output_struct = nullptr;

    /// The output values to return from the entry point.
    Vector<core::ir::Value*, 4> output_values;

    // Indices of inputs that require special handling
    std::optional<uint32_t> subgroup_invocation_id_index;
    std::optional<uint32_t> subgroup_size_index;
    std::optional<uint32_t> num_workgroups_index;

    /// Constructor
    StateImpl(core::ir::Module& mod, core::ir::Function* f, const ShaderIOConfig& c)
        : ShaderIOBackendState(mod, f), config(c) {}

    /// Destructor
    ~StateImpl() override {}

    /// FXC is sensitive to field order in structures, this is used by StructMemberComparator to
    /// ensure that FXC is happy with the order of emitted fields.
    uint32_t BuiltinOrder(core::BuiltinValue builtin) {
        switch (builtin) {
            case core::BuiltinValue::kPosition:
                return 1;
            case core::BuiltinValue::kVertexIndex:
                return 2;
            case core::BuiltinValue::kInstanceIndex:
                return 3;
            case core::BuiltinValue::kFrontFacing:
                return 4;
            case core::BuiltinValue::kFragDepth:
                return 5;
            case core::BuiltinValue::kLocalInvocationId:
                return 6;
            case core::BuiltinValue::kLocalInvocationIndex:
                return 7;
            case core::BuiltinValue::kGlobalInvocationId:
                return 8;
            case core::BuiltinValue::kWorkgroupId:
                return 9;
            case core::BuiltinValue::kNumWorkgroups:
                return 10;
            case core::BuiltinValue::kSampleIndex:
                return 11;
            case core::BuiltinValue::kSampleMask:
                return 12;
            case core::BuiltinValue::kPointSize:
                return 13;
            default:
                break;
        }
        TINT_UNREACHABLE();
    }

    struct MemberInfo {
        core::type::Manager::StructMemberDesc member;
        uint32_t idx;
    };

    /// Comparison function used to reorder struct members such that all members with
    /// color attributes appear first (ordered by color slot), then location attributes (ordered by
    /// location slot), then blend_src attributes (ordered by blend_src slot), followed by those
    /// with builtin attributes (ordered by BuiltinOrder).
    /// @param x a struct member
    /// @param y another struct member
    /// @returns true if a comes before b
    bool StructMemberComparator(const MemberInfo& x, const MemberInfo& y) {
        if (x.member.attributes.color.has_value() && y.member.attributes.color.has_value() &&
            x.member.attributes.color != y.member.attributes.color) {
            // Both have color attributes: smallest goes first.
            return x.member.attributes.color < y.member.attributes.color;
        } else if (x.member.attributes.color.has_value() != y.member.attributes.color.has_value()) {
            // The member with the color goes first
            return x.member.attributes.color.has_value();
        }

        if (x.member.attributes.location.has_value() && y.member.attributes.location.has_value() &&
            x.member.attributes.location != y.member.attributes.location) {
            // Both have location attributes: smallest goes first.
            return x.member.attributes.location < y.member.attributes.location;
        } else if (x.member.attributes.location.has_value() !=
                   y.member.attributes.location.has_value()) {
            // The member with the location goes first
            return x.member.attributes.location.has_value();
        }

        if (x.member.attributes.blend_src.has_value() &&
            y.member.attributes.blend_src.has_value() &&
            x.member.attributes.blend_src != y.member.attributes.blend_src) {
            // Both have blend_src attributes: smallest goes first.
            return x.member.attributes.blend_src < y.member.attributes.blend_src;
        } else if (x.member.attributes.blend_src.has_value() !=
                   y.member.attributes.blend_src.has_value()) {
            // The member with the blend_src goes first
            return x.member.attributes.blend_src.has_value();
        }

        auto x_blt = x.member.attributes.builtin;
        auto y_blt = y.member.attributes.builtin;
        if (x_blt.has_value() && y_blt.has_value()) {
            // Both are builtins: order matters for FXC.
            auto order_a = BuiltinOrder(*x_blt);
            auto order_b = BuiltinOrder(*y_blt);
            if (order_a != order_b) {
                return order_a < order_b;
            }
        } else if (x_blt.has_value() != y_blt.has_value()) {
            // The member with the builtin goes first
            return x_blt.has_value();
        }

        // Control flow reaches here if x is the same as y.
        // Sort algorithms sometimes do that.
        return false;
    }

    /// @copydoc ShaderIO::BackendState::FinalizeInputs
    Vector<core::ir::FunctionParam*, 4> FinalizeInputs() override {
        Vector<core::type::Manager::StructMemberDesc, 4> input_struct_members;

        Vector<MemberInfo, 4> input_data;
        for (uint32_t i = 0; i < inputs.Length(); ++i) {
            // If subgroup invocation id or size, save the index for GetInput
            if (auto builtin = inputs[i].attributes.builtin) {
                if (*builtin == core::BuiltinValue::kSubgroupInvocationId) {
                    subgroup_invocation_id_index = i;
                    continue;
                } else if (*builtin == core::BuiltinValue::kSubgroupSize) {
                    subgroup_size_index = i;
                    continue;
                } else if (*builtin == core::BuiltinValue::kNumWorkgroups) {
                    num_workgroups_index = i;
                    continue;
                }
            }

            input_data.Push(MemberInfo{inputs[i], i});
        }

        input_indices.Resize(input_data.Length());

        // Sort the struct members to satisfy HLSL interfacing matching rules.
        std::sort(input_data.begin(), input_data.end(),
                  [&](auto& x, auto& y) { return StructMemberComparator(x, y); });

        for (auto& input : input_data) {
            input_indices[input.idx] = static_cast<uint32_t>(input_struct_members.Length());
            input_struct_members.Push(input.member);
        }

        if (!input_struct_members.IsEmpty()) {
            auto* input_struct = ty.Struct(ir.symbols.New(ir.NameOf(func).Name() + "_inputs"),
                                           std::move(input_struct_members));
            switch (func->Stage()) {
                case core::ir::Function::PipelineStage::kFragment:
                    input_struct->AddUsage(core::type::PipelineStageUsage::kFragmentInput);
                    break;
                case core::ir::Function::PipelineStage::kVertex:
                    input_struct->AddUsage(core::type::PipelineStageUsage::kVertexInput);
                    break;
                case core::ir::Function::PipelineStage::kCompute:
                    input_struct->AddUsage(core::type::PipelineStageUsage::kComputeInput);
                    break;
                case core::ir::Function::PipelineStage::kUndefined:
                    TINT_UNREACHABLE();
            }
            input_param = b.FunctionParam("inputs", input_struct);
            return {input_param};
        }

        return tint::Empty;
    }

    /// @copydoc ShaderIO::BackendState::FinalizeOutputs
    const core::type::Type* FinalizeOutputs() override {
        if (outputs.IsEmpty()) {
            return ty.void_();
        }

        Vector<MemberInfo, 4> output_data;
        for (uint32_t i = 0; i < outputs.Length(); ++i) {
            output_data.Push(MemberInfo{outputs[i], i});
        }

        // Sort the struct members to satisfy HLSL interfacing matching rules.
        std::sort(output_data.begin(), output_data.end(),
                  [&](auto& x, auto& y) { return StructMemberComparator(x, y); });

        output_indices.Resize(outputs.Length());
        output_values.Resize(outputs.Length());

        Vector<core::type::Manager::StructMemberDesc, 4> output_struct_members;
        for (size_t i = 0; i < output_data.Length(); ++i) {
            output_indices[output_data[i].idx] = static_cast<uint32_t>(i);
            output_struct_members.Push(output_data[i].member);
        }

        output_struct =
            ty.Struct(ir.symbols.New(ir.NameOf(func).Name() + "_outputs"), output_struct_members);
        switch (func->Stage()) {
            case core::ir::Function::PipelineStage::kFragment:
                output_struct->AddUsage(core::type::PipelineStageUsage::kFragmentOutput);
                break;
            case core::ir::Function::PipelineStage::kVertex:
                output_struct->AddUsage(core::type::PipelineStageUsage::kVertexOutput);
                break;
            case core::ir::Function::PipelineStage::kCompute:
                output_struct->AddUsage(core::type::PipelineStageUsage::kComputeOutput);
                break;
            case core::ir::Function::PipelineStage::kUndefined:
                TINT_UNREACHABLE();
        }
        return output_struct;
    }

    /// Handles kNumWorkgroups builtin by emitting a UBO to hold the num_workgroups value,
    /// along with the load of the value. Returns the loaded value.
    core::ir::Value* GetInputForNumWorkgroups(core::ir::Builder& builder) {
        // Create uniform var that will receive the number of workgroups
        core::ir::Var* num_wg_var = nullptr;
        builder.Append(ir.root_block, [&] {
            num_wg_var = builder.Var("tint_num_workgroups", ty.ptr(uniform, ty.vec3<u32>()));
        });
        if (config.num_workgroups_binding.has_value()) {
            // If config.num_workgroups_binding holds a value, use it.
            auto bp = *config.num_workgroups_binding;
            num_wg_var->SetBindingPoint(bp.group, bp.binding);
        } else {
            // Otherwise, use the binding 0 of the largest used group plus 1, or group 0 if no
            // resources are bound.
            uint32_t group = 0;
            for (auto* inst : *ir.root_block.Get()) {
                if (auto* var = inst->As<core::ir::Var>()) {
                    if (const auto& bp = var->BindingPoint()) {
                        if (bp->group >= group) {
                            group = bp->group + 1;
                        }
                    }
                }
            }
            num_wg_var->SetBindingPoint(group, 0);
        }
        auto* load = builder.Load(num_wg_var);
        return load->Result(0);
    }

    /// @copydoc ShaderIO::BackendState::GetInput
    core::ir::Value* GetInput(core::ir::Builder& builder, uint32_t idx) override {
        if (subgroup_invocation_id_index == idx) {
            return builder
                .Call<hlsl::ir::BuiltinCall>(ty.u32(), hlsl::BuiltinFn::kWaveGetLaneIndex)
                ->Result(0);
        }
        if (subgroup_size_index == idx) {
            return builder
                .Call<hlsl::ir::BuiltinCall>(ty.u32(), hlsl::BuiltinFn::kWaveGetLaneCount)
                ->Result(0);
        }
        if (num_workgroups_index == idx) {
            return GetInputForNumWorkgroups(builder);
        }

        auto index = input_indices[idx];

        core::ir::Value* v = builder.Access(inputs[idx].type, input_param, u32(index))->Result(0);

        // If this is an input position builtin we need to invert the 'w' component of the vector.
        if (inputs[idx].attributes.builtin == core::BuiltinValue::kPosition) {
            auto* w = builder.Access(ty.f32(), v, 3_u);
            auto* div = builder.Divide(ty.f32(), 1.0_f, w);
            auto* swizzle = builder.Swizzle(ty.vec3<f32>(), v, {0, 1, 2});
            v = builder.Construct(ty.vec4<f32>(), swizzle, div)->Results()[0];
        }

        return v;
    }

    /// @copydoc ShaderIO::BackendState::SetOutput
    void SetOutput(core::ir::Builder&, uint32_t idx, core::ir::Value* value) override {
        auto index = output_indices[idx];
        output_values[index] = value;
    }

    /// @copydoc ShaderIO::BackendState::MakeReturnValue
    core::ir::Value* MakeReturnValue(core::ir::Builder& builder) override {
        if (!output_struct) {
            return nullptr;
        }
        return builder.Construct(output_struct, std::move(output_values))->Result(0);
    }
};
}  // namespace

Result<SuccessType> ShaderIO(core::ir::Module& ir, const ShaderIOConfig& config) {
    auto result = ValidateAndDumpIfNeeded(ir, "ShaderIO transform");
    if (result != Success) {
        return result;
    }

    core::ir::transform::RunShaderIOBase(ir, [&](core::ir::Module& mod, core::ir::Function* func) {
        return std::make_unique<StateImpl>(mod, func, config);
    });

    return Success;
}

}  // namespace tint::hlsl::writer::raise
