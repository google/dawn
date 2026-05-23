// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/transform/resource_table.h"

#include <span>
#include <unordered_map>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/resource_type.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The configuration.
    const std::optional<ResourceTableConfig>& config;

    /// The IR module.
    core::ir::Module& ir;

    /// The helper
    ResourceTableHelper* helper;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The storage buffer used to hold API metadata
    core::ir::Var* storage_buffer = nullptr;

    /// Map from a tint Type to the `var` which holds the resource table of that
    /// type
    Hashmap<const core::type::Type*, core::ir::Var*, 4> var_for_type{};

    /// Maps resource_index value to the default offset for that type
    std::unordered_map<ResourceType, uint32_t> resource_type_to_default_idx{};

    struct Info {
        const core::type::Type* binding_type;
        core::ir::Value* slot_idx;
        size_t operand_idx;
    };
    /// A list of instructions which need to be replaced with the, possibly,
    /// two indices which are using a resource_table element.
    struct CallArgs {
        std::optional<Info> texture;
        std::optional<Info> sampler;
    };
    Hashmap<ir::CoreBuiltinCall*, CallArgs, 8> sampled_call_replacements{};

    /// Process the module.
    Result<SuccessType> Process() {
        // Find calls to hasResource() and getResource().
        Vector<core::ir::CoreBuiltinCall*, 8> has_resource_calls;
        Vector<core::ir::CoreBuiltinCall*, 8> get_resource_calls;
        for (auto* inst : ir.Instructions()) {
            auto* call = inst->As<core::ir::CoreBuiltinCall>();
            if (call == nullptr) {
                continue;
            }
            if (call->Func() == core::BuiltinFn::kHasResource) {
                has_resource_calls.Push(call);
            }
            if (call->Func() == core::BuiltinFn::kGetResource) {
                get_resource_calls.Push(call);
            }
        }
        if (has_resource_calls.IsEmpty() && get_resource_calls.IsEmpty()) {
            return Success;
        }
        if (!config.has_value()) {
            return Failure{"hasResource and getResource require a resource table"};
        }

        b.Append(ir.root_block, [&] {
            var_for_type = helper->GenerateVars(b, config->resource_table_binding,
                                                config->default_binding_type_order);
            InjectStorageBuffer(config->storage_buffer_binding);
        });
        TINT_IR_ASSERT(ir, storage_buffer != nullptr);

        size_t def_size = config->default_binding_type_order.size();
        for (size_t i = 0; i < def_size; ++i) {
            auto res_ty = static_cast<ResourceType>(config->default_binding_type_order[i]);
            resource_type_to_default_idx.insert({res_ty, static_cast<uint32_t>(i)});
        }

        // Replace the calls to hasResource() and getResource() that we found earlier.
        for (auto* call : has_resource_calls) {
            b.InsertBefore(call, [&] {
                auto* binding_ty = call->ExplicitTemplateParams()[0];
                auto* idx = b.InsertConvertIfNeeded(ty.u32(), call->Args()[0]);

                GenHasResource(call->DetachResult(), binding_ty, idx);
            });
            call->Destroy();
        }
        for (auto* call : get_resource_calls) {
            b.InsertBefore(call, [&] {
                auto* binding_ty = call->ExplicitTemplateParams()[0];
                auto* idx = b.InsertConvertIfNeeded(ty.u32(), call->Args()[0]);
                ReplaceUsages(call->Result(), binding_ty, idx);
            });
        }
        for (auto& entry : sampled_call_replacements) {
            GenSampledGetResource(entry.key, entry.value);
        }

        for (auto* call : get_resource_calls) {
            TINT_IR_ASSERT(ir, call->Result()->UsagesUnsorted().IsEmpty());
            call->Destroy();
        }

        return Success;
    }

    // Get the type id from the metadata buffer
    core::ir::InstructionResult* GetTypeId(core::ir::Value* idx) {
        // Get the type id from the metadata buffer
        auto* metadata_access = b.Access(ty.ptr<storage, u32, read>(), storage_buffer, 1_u, idx);
        return b.Load(metadata_access)->Result();
    }

    bool IsSampler(const core::type::Type* binding_type) const {
        auto res_type = core::type::TypeToResourceType(binding_type);
        return tint::IsSampler(res_type);
    }

    // Note, assumes it's called inside a builder append block.
    void GenHasResource(core::ir::InstructionResult* result,
                        const core::type::Type* binding_type,
                        core::ir::Value* idx) {
        // Get the table's API size, which is stored at index 0 in the metadata buffer
        auto* length = b.Access(ty.ptr<storage, u32, read>(), storage_buffer, 0_u);
        ir.SetName(length, "tint_storage_metadata_length");

        auto* len_check = b.LessThan(idx, b.Load(length));

        auto* has_check = b.If(len_check);
        has_check->SetResult(result);

        b.Append(has_check->True(), [&] {
            auto* metadata_val = GetTypeId(idx);

            ir::Value* type_id = nullptr;
            if (config->get_sampler_index_from_metadata && IsSampler(binding_type)) {
                // Type id is in lower 16 bits
                type_id = b.And(metadata_val, u32(0xFFFF))->Result();
            } else {
                type_id = metadata_val;
            }

            core::ir::Value* eq = nullptr;
            std::vector<ResourceType> conv = core::type::ConvertsFrom(binding_type);
            if (!conv.empty()) {
                auto* conv_ty = ty.vec(ty.u32(), static_cast<uint32_t>(conv.size()));
                auto* lhs = b.Construct(conv_ty, type_id);

                Vector<Value*, 4> vals;
                for (auto& r : conv) {
                    vals.Push(b.Value(u32(r)));
                }

                auto* rhs = b.Construct(conv_ty, vals);
                auto* cmp = b.Equal(lhs, rhs);
                eq = b.Call(ty.bool_(), core::BuiltinFn::kAny, cmp)->Result();
            } else {
                ResourceType resource_ty = core::type::TypeToResourceType(binding_type);
                eq = b.Equal(type_id, u32(resource_ty))->Result();
            }
            b.ExitIf(has_check, eq);
        });

        b.Append(has_check->False(), [&] { b.ExitIf(has_check, b.Constant(false)); });
    }

    // Replaces all the usages of a `call` instruction (which is required to be
    // a `GetResource` call with the needed conditional checks and code to
    // retrieve from the storage buffer.
    void ReplaceUsages(core::ir::InstructionResult* result,
                       const core::type::Type* binding_type,
                       core::ir::Value* idx) {
        for (auto& usage : result->UsagesSorted()) {
            tint::Switch(
                usage.instruction,  //
                [&](core::ir::CoreBuiltinCall* call) {
                    switch (call->Func()) {
                        case core::BuiltinFn::kTextureStore:
                        case core::BuiltinFn::kTextureLoad:
                        case core::BuiltinFn::kTextureDimensions:
                        case core::BuiltinFn::kTextureNumLayers:
                        case core::BuiltinFn::kTextureNumLevels:
                        case core::BuiltinFn::kTextureNumSamples: {
                            auto* has_result = b.InstructionResult(ty.bool_());
                            GenHasResource(has_result, binding_type, idx);

                            ir::InstructionResult* res =
                                GenNonSampledGetResource(has_result, binding_type, idx);
                            call->SetOperand(usage.operand_index, res);
                            break;
                        }
                        case core::BuiltinFn::kTextureGather:
                        case core::BuiltinFn::kTextureGatherCompare:
                        case core::BuiltinFn::kTextureSample:
                        case core::BuiltinFn::kTextureSampleBias:
                        case core::BuiltinFn::kTextureSampleCompare:
                        case core::BuiltinFn::kTextureSampleCompareLevel:
                        case core::BuiltinFn::kTextureSampleGrad:
                        case core::BuiltinFn::kTextureSampleLevel:
                        case core::BuiltinFn::kTextureSampleBaseClampToEdge: {
                            auto& info = sampled_call_replacements.GetOrAddZeroEntry(call);
                            if (binding_type->Is<core::type::Texture>()) {
                                info.value.texture = {
                                    .binding_type = binding_type,
                                    .slot_idx = idx,
                                    .operand_idx = usage.operand_index,
                                };
                            } else {
                                info.value.sampler = {
                                    .binding_type = binding_type,
                                    .slot_idx = idx,
                                    .operand_idx = usage.operand_index,
                                };
                            }
                            break;
                        }
                        default:
                            TINT_IR_UNREACHABLE(ir);
                    }
                },
                [&](Default) { TINT_IR_UNREACHABLE(ir); }

            );
        }
    }

    // Note, assumes it's called inside a builder append block.
    ir::InstructionResult* GenNonSampledGetResource(core::ir::Value* has_result,
                                                    const core::type::Type* binding_type,
                                                    core::ir::Value* idx) {
        auto* get_check = b.If(has_result);
        auto* res = b.InstructionResult(ty.u32());
        get_check->SetResult(res);

        auto var = var_for_type.Get(binding_type);
        TINT_IR_ASSERT(ir, var);

        b.Append(get_check->True(), [&] {
            // Table lookup succeeded, use the input index
            b.ExitIf(get_check, idx);
        });

        b.Append(get_check->False(), [&] {
            // Table lookup failed, so get default resource located at the end of the table,
            // at API size + resource type index.

            // This texture is not being combined with a sampler, so we just
            // need _a_ default, this could be filterable or unfilterable it
            // doesn't matter.
            //
            // This does mean for every possible filterable type, the default
            // value must be bound (currently this is always the `filterable`
            // variant. This is fine because Dawn binds all of the possible
            // defaults.
            auto res_type = core::type::DefaultResourceTypeFor(binding_type);
            auto idx_iter = resource_type_to_default_idx.find(res_type);
            TINT_IR_ASSERT(ir, idx_iter != resource_type_to_default_idx.end());

            // Get the table's API size, which is stored at index 0 in the metadata buffer
            auto* len_access = b.Access(ty.ptr<storage, u32, read>(), storage_buffer, 0_u);
            auto* num_elements = b.Load(len_access);

            auto* r = b.Add(u32(idx_iter->second), num_elements);
            b.ExitIf(get_check, r);
        });

        ir::Value* final_index = res;
        if (config->get_sampler_index_from_metadata && IsSampler(binding_type)) {
            // Get the sampler index from the metadata entry (high 16 bits)
            // TODO(crbug.com/503755700): Optimize to avoid loading twice from storage_buffer[idx].
            auto* metadata_val = GetTypeId(res);
            auto* sampler_index = b.ShiftRight(metadata_val, u32(16));
            final_index = sampler_index->Result();
        }

        // TODO(439627523): Fix pointer access
        auto* ptr_ty = ty.ptr(handle, binding_type, read);
        auto* access = b.Access(ptr_ty, (*var)->Result(), final_index);
        return b.Load(access)->Result();
    }

    // We have a bind-ful texture/sampler, get the ResourceKind the API reported
    ir::InstructionResult* HandleBindfulKind(ir::CoreBuiltinCall* call,
                                             size_t idx,
                                             std::string_view name) {
        core::ir::InstructionResult* opt_res =
            call->Operands()[idx]->As<core::ir::InstructionResult>();
        TINT_IR_ASSERT(ir, opt_res);

        core::ir::Var* var = RootVarFor(opt_res);
        TINT_IR_ASSERT(ir, var);

        auto iter = config->binding_to_resource_type.find(var->BindingPoint().value());
        TINT_IR_ASSERT(ir, iter != config->binding_to_resource_type.end());

        ir::InstructionResult* res = nullptr;
        b.InsertBefore(call, [&] { res = b.Let(u32(iter->second))->Result(); });
        ir.SetName(res, name);
        return res;
    }

    ir::InstructionResult* HandleBindlessKind(ir::CoreBuiltinCall* call,
                                              const Info& info,
                                              bool is_sampler_comparison_call,
                                              std::string_view name) {
        ir::InstructionResult* resource_kind = nullptr;
        b.InsertBefore(call, [&] {
            // Determine if the slot kind matches the type in WGSL
            auto* has_result = b.InstructionResult(ty.bool_());
            GenHasResource(has_result, info.binding_type, info.slot_idx);

            if (!is_sampler_comparison_call) {
                // Get the ResourceKind for the resource
                core::ir::If* if_ = b.If(has_result);
                resource_kind = b.InstructionResult(ty.u32());
                if_->SetResult(resource_kind);

                b.Append(if_->True(), [&] {  //
                    b.ExitIf(if_, GetTypeId(info.slot_idx));
                });
                b.Append(if_->False(), [&] {
                    ResourceType res_type = core::type::DefaultResourceTypeFor(info.binding_type);
                    b.ExitIf(if_, u32(res_type));
                });
                ir.SetName(resource_kind, name);
            }

            // Get the resource from the table and update the argument
            ir::InstructionResult* res =
                GenNonSampledGetResource(has_result, info.binding_type, info.slot_idx);

            call->SetOperand(info.operand_idx, res);
        });
        return resource_kind;
    }

    ir::InstructionResult* GetKind(ir::CoreBuiltinCall* call,
                                   const std::optional<Info>& opt,
                                   bool is_sampler_comparison_call,
                                   size_t idx,
                                   std::string_view name) {
        core::ir::InstructionResult* kind = nullptr;
        if (opt.has_value()) {
            kind = HandleBindlessKind(call, opt.value(), is_sampler_comparison_call, name);
        } else if (!is_sampler_comparison_call) {
            kind = HandleBindfulKind(call, idx, name);
        }
        return kind;
    }

    void GenSampledGetResource(ir::CoreBuiltinCall* call, const CallArgs& args) {
        // we can get the texture, or the sampler, or both.
        TINT_IR_ASSERT(ir, args.texture.has_value() || args.sampler.has_value());

        bool is_sampler_comparison_call =
            call->Func() == core::BuiltinFn::kTextureSampleCompare ||
            call->Func() == core::BuiltinFn::kTextureSampleCompareLevel ||
            call->Func() == core::BuiltinFn::kTextureGatherCompare;

        // Pre-calculate the operand indices. We know we have at least one of these, so if the
        // needed optional isn't set we can always use the other optional to calculate the index.
        size_t texture_operand_idx =
            args.texture.has_value() ? args.texture->operand_idx : args.sampler->operand_idx - 1;
        size_t sampler_operand_idx =
            args.sampler.has_value() ? args.sampler->operand_idx : args.texture->operand_idx + 1;

        core::ir::InstructionResult* texture_kind = GetKind(
            call, args.texture, is_sampler_comparison_call, texture_operand_idx, "texture_kind");
        core::ir::InstructionResult* sampler_kind = GetKind(
            call, args.sampler, is_sampler_comparison_call, sampler_operand_idx, "sampler_kind");

        // Only need to validate the texture/sampler if we aren't don't a
        // comparison sampler.
        if (is_sampler_comparison_call) {
            return;
        }

        // Validate that the sampler/texture pair work together
        b.InsertBefore(call, [&] {
            // We only care if we're using a filtering sampler with a non_filterable texture, so
            // check if the sampler is filtering
            core::ir::Instruction* sampler_compare =
                b.Equal(sampler_kind, u32(ResourceType::kSampler_filtering));

            // Returns `true` if we need to replace the sampler with a default non_filtering
            // sampler
            core::ir::If* samp_if = b.If(sampler_compare);
            core::ir::InstructionResult* samp_res = b.InstructionResult(ty.bool_());
            ir.SetName(samp_res, "use_sampler");
            samp_if->SetResult(samp_res);

            // If the sampler is filtering
            b.Append(samp_if->True(), [&] {
                size_t idx = 0;
                const std::span<const ResourceType> filterable_types =
                    core::type::FilterableResources();

                core::ir::Binary* chk = b.Equal(texture_kind, u32(filterable_types[idx++]));
                core::ir::If* t = b.If(chk);
                core::ir::InstructionResult* tex_res = b.InstructionResult(ty.bool_());
                t->SetResult(tex_res);

                // The texture is filterable, so we can use the sampler
                b.Append(t->True(), [&] { b.ExitIf(t, true); });

                for (; idx < filterable_types.size(); ++idx) {
                    b.Append(t->False(), [&] {
                        core::ir::Binary* sub_chk =
                            b.Equal(texture_kind, u32(filterable_types[idx]));
                        core::ir::If* sub_if = b.If(sub_chk);
                        core::ir::InstructionResult* r = b.InstructionResult(ty.bool_());
                        sub_if->SetResult(r);

                        // The texture is filterable, so we can use the sampler
                        b.Append(sub_if->True(), [&] { b.ExitIf(sub_if, true); });
                        b.ExitIf(t, r);

                        t = sub_if;
                    });
                }
                // Texture is not filterable, return false
                b.Append(t->False(), [&] { b.ExitIf(t, false); });

                b.ExitIf(samp_if, tex_res);
            });

            // Sampler != filtering, so use_sampler is true
            b.Append(samp_if->False(), [&] { b.ExitIf(samp_if, true); });

            const core::type::Type* result_ty = call->Result()->Type();

            // Branch over if we can use the sampler or not, the `if` returns the result of the
            // `call` we're attempting to make
            core::ir::If* check = b.If(samp_res);
            check->SetResult(call->DetachResult());

            // Sampler and texture matched, just call
            b.Append(check->True(), [&] {
                core::ir::Call* c = b.Call(result_ty, call->Func());
                for (ir::Value* arg : call->Args()) {
                    c->AppendArg(arg);
                }
                b.ExitIf(check, c);
            });

            // Sampler and texture mismatch, pull a default sampler and use that
            b.Append(check->False(), [&] {
                auto idx_iter =
                    resource_type_to_default_idx.find(ResourceType::kSampler_non_filtering);
                TINT_IR_ASSERT(ir, idx_iter != resource_type_to_default_idx.end());

                // Get the table's API size, which is stored at index 0 in the metadata buffer
                auto* len_access = b.Access(ty.ptr<storage, u32, read>(), storage_buffer, 0_u);
                auto* num_elements = b.Load(len_access);

                ir::Value* final_index = b.Add(u32(idx_iter->second), num_elements)->Result();

                if (config->get_sampler_index_from_metadata) {
                    // Get the sampler index from the metadata entry (high 16 bits)
                    // TODO(crbug.com/503755700): Optimize to avoid loading twice from
                    // storage_buffer[idx].
                    auto* metadata_val = GetTypeId(final_index);
                    auto* sampler_index = b.ShiftRight(metadata_val, u32(16));
                    final_index = sampler_index->Result();
                }

                const core::type::Sampler* binding_type = ty.sampler();
                auto var = var_for_type.Get(binding_type);
                TINT_IR_ASSERT(ir, var);

                const core::type::Pointer* ptr_ty = ty.ptr(handle, binding_type, read);
                auto* access = b.Access(ptr_ty, (*var)->Result(), final_index);
                ir::Instruction* sampler = b.Load(access);

                // Create the call and swap in the new sampler
                core::ir::Call* c = b.Call(result_ty, call->Func());
                for (ir::Value* arg : call->Args()) {
                    c->AppendArg(arg);
                }
                c->SetOperand(sampler_operand_idx, sampler->Result());

                b.ExitIf(check, c);
            });
        });
        call->Destroy();
    }

    // Returns the root Var for `value` by walking up the chain of instructions,
    // or nullptr if none is found.
    Var* RootVarFor(Value* value) {
        Var* result = nullptr;
        while (value) {
            TINT_IR_ASSERT(ir, value->Alive());

            auto* res = value->As<core::ir::InstructionResult>();
            TINT_IR_ASSERT(ir, res);

            // value was emitted by an instruction
            auto* inst = res->Instruction();
            value = tint::Switch(
                inst,
                [&](Load* l) {
                    ir::InstructionResult* from = l->From()->As<core::ir::InstructionResult>();
                    TINT_IR_ASSERT(ir, from);

                    result = from->Instruction()->As<core::ir::Var>();  // Done
                    TINT_IR_ASSERT(ir, result);

                    return nullptr;
                },
                [&](Var* var) {
                    result = var;
                    return nullptr;  // Done
                },
                TINT_ICE_ON_NO_MATCH);
        }
        return result;
    }

    void InjectStorageBuffer(const BindingPoint& bp) {
        auto* str = ty.Struct(ir.symbols.New("tint_resource_table_metadata_struct"),
                              Vector<core::type::Manager::StructMemberDesc, 2>{
                                  {ir.symbols.New("array_length"), ty.u32()},
                                  {ir.symbols.New("bindings"), ty.array<u32>()},
                              });
        auto* sb_ty = ty.ptr(storage, str, read);

        storage_buffer = b.Var("tint_resource_table_metadata", sb_ty);
        storage_buffer->SetBindingPoint(bp.group, bp.binding);
    }
};

}  // namespace

ResourceTableHelper::~ResourceTableHelper() = default;

Result<SuccessType> ResourceTable(core::ir::Module& ir,
                                  const std::optional<ResourceTableConfig>& config,
                                  ResourceTableHelper* helper) {
    AssertValid(ir,
                core::ir::Capabilities{
                    core::ir::Capability::kAllowDuplicateBindings,
                    core::ir::Capability::kAllow8BitIntegers,
                    core::ir::Capability::kAllow16BitIntegers,
                },
                "before core.ResourceTable");

    return State{config, ir, helper}.Process();
}

}  // namespace tint::core::ir::transform
