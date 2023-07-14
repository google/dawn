// Copyright 2023 The Tint Authors.
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

#include "src/tint/ir/transform/builtin_polyfill_spirv.h"

#include <utility>

#include "spirv/unified1/spirv.h"
#include "src/tint/ir/builder.h"
#include "src/tint/ir/module.h"
#include "src/tint/type/depth_multisampled_texture.h"
#include "src/tint/type/depth_texture.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/texture.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::transform::BuiltinPolyfillSpirv);
TINT_INSTANTIATE_TYPEINFO(tint::ir::transform::BuiltinPolyfillSpirv::LiteralOperand);
TINT_INSTANTIATE_TYPEINFO(tint::ir::transform::BuiltinPolyfillSpirv::SampledImage);

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ir::transform {

BuiltinPolyfillSpirv::BuiltinPolyfillSpirv() = default;

BuiltinPolyfillSpirv::~BuiltinPolyfillSpirv() = default;

/// PIMPL state for the transform.
struct BuiltinPolyfillSpirv::State {
    /// The IR module.
    Module* ir = nullptr;

    /// The IR builder.
    Builder b{*ir};

    /// The type manager.
    type::Manager& ty{ir->Types()};

    /// Process the module.
    void Process() {
        // Find the builtins that need replacing.
        utils::Vector<CoreBuiltinCall*, 4> worklist;
        for (auto* inst : ir->instructions.Objects()) {
            if (!inst->Alive()) {
                continue;
            }
            if (auto* builtin = inst->As<CoreBuiltinCall>()) {
                switch (builtin->Func()) {
                    case builtin::Function::kDot:
                    case builtin::Function::kSelect:
                    case builtin::Function::kTextureSample:
                    case builtin::Function::kTextureSampleBias:
                    case builtin::Function::kTextureSampleCompare:
                    case builtin::Function::kTextureSampleCompareLevel:
                    case builtin::Function::kTextureSampleGrad:
                    case builtin::Function::kTextureSampleLevel:
                        worklist.Push(builtin);
                        break;
                    default:
                        break;
                }
            }
        }

        // Replace the builtins that we found.
        for (auto* builtin : worklist) {
            Value* replacement = nullptr;
            switch (builtin->Func()) {
                case builtin::Function::kDot:
                    replacement = Dot(builtin);
                    break;
                case builtin::Function::kSelect:
                    replacement = Select(builtin);
                    break;
                case builtin::Function::kTextureSample:
                case builtin::Function::kTextureSampleBias:
                case builtin::Function::kTextureSampleCompare:
                case builtin::Function::kTextureSampleCompareLevel:
                case builtin::Function::kTextureSampleGrad:
                case builtin::Function::kTextureSampleLevel:
                    replacement = TextureSample(builtin);
                    break;
                default:
                    break;
            }
            TINT_ASSERT_OR_RETURN(Transform, replacement);

            // Replace the old builtin result with the new value.
            if (auto name = ir->NameOf(builtin->Result())) {
                ir->SetName(replacement, name);
            }
            builtin->Result()->ReplaceAllUsesWith(replacement);
            builtin->Destroy();
        }
    }

    /// Handle a `dot()` builtin.
    /// @param builtin the builtin call instruction
    /// @returns the replacement value
    Value* Dot(CoreBuiltinCall* builtin) {
        // OpDot only supports floating point operands, so we need to polyfill the integer case.
        // TODO(crbug.com/tint/1267): If SPV_KHR_integer_dot_product is supported, use that instead.
        if (builtin->Result()->Type()->is_integer_scalar()) {
            Instruction* sum = nullptr;

            auto* v1 = builtin->Args()[0];
            auto* v2 = builtin->Args()[1];
            auto* vec = v1->Type()->As<type::Vector>();
            auto* elty = vec->type();
            for (uint32_t i = 0; i < vec->Width(); i++) {
                auto* e1 = b.Access(elty, v1, u32(i));
                e1->InsertBefore(builtin);
                auto* e2 = b.Access(elty, v2, u32(i));
                e2->InsertBefore(builtin);
                auto* mul = b.Multiply(elty, e1, e2);
                mul->InsertBefore(builtin);
                if (sum) {
                    sum = b.Add(elty, sum, mul);
                    sum->InsertBefore(builtin);
                } else {
                    sum = mul;
                }
            }
            return sum->Result();
        }

        // Replace the builtin call with a call to the spirv.dot intrinsic.
        auto args = utils::Vector<Value*, 4>(builtin->Args());
        auto* call =
            b.Call(builtin->Result()->Type(), IntrinsicCall::Kind::kSpirvDot, std::move(args));
        call->InsertBefore(builtin);
        return call->Result();
    }

    /// Handle a `select()` builtin.
    /// @param builtin the builtin call instruction
    /// @returns the replacement value
    Value* Select(CoreBuiltinCall* builtin) {
        // Argument order is different in SPIR-V: (condition, true_operand, false_operand).
        utils::Vector<Value*, 4> args = {
            builtin->Args()[2],
            builtin->Args()[1],
            builtin->Args()[0],
        };

        // If the condition is scalar and the objects are vectors, we need to splat the condition
        // into a vector of the same size.
        // TODO(jrprice): We don't need to do this if we're targeting SPIR-V 1.4 or newer.
        auto* vec = builtin->Result()->Type()->As<type::Vector>();
        if (vec && args[0]->Type()->Is<type::Scalar>()) {
            utils::Vector<Value*, 4> elements;
            elements.Resize(vec->Width(), args[0]);

            auto* construct = b.Construct(ty.vec(ty.bool_(), vec->Width()), std::move(elements));
            construct->InsertBefore(builtin);
            args[0] = construct->Result();
        }

        // Replace the builtin call with a call to the spirv.select intrinsic.
        auto* call =
            b.Call(builtin->Result()->Type(), IntrinsicCall::Kind::kSpirvSelect, std::move(args));
        call->InsertBefore(builtin);
        return call->Result();
    }

    /// Handle a textureSample*() builtin.
    /// @param builtin the builtin call instruction
    /// @returns the replacement value
    Value* TextureSample(CoreBuiltinCall* builtin) {
        // Helper to get the next argument from the call, or nullptr if there are no more arguments.
        uint32_t arg_idx = 0;
        auto next_arg = [&]() {
            return arg_idx < builtin->Args().Length() ? builtin->Args()[arg_idx++] : nullptr;
        };

        auto* texture = next_arg();
        auto* sampler = next_arg();
        auto* coords = next_arg();
        auto* texture_ty = texture->Type()->As<type::Texture>();
        auto* array_idx = IsTextureArray(texture_ty->dim()) ? next_arg() : nullptr;
        Value* depth = nullptr;

        // Use OpSampledImage to create an OpTypeSampledImage object.
        auto* sampled_image =
            b.Call(ty.Get<SampledImage>(texture_ty), IntrinsicCall::Kind::kSpirvSampledImage,
                   utils::Vector{texture, sampler});
        sampled_image->InsertBefore(builtin);

        // Append the array index to the coordinates if provided.
        if (array_idx) {
            // Convert the index to an f32.
            auto* array_idx_f32 = b.Convert(ty.f32(), array_idx);
            array_idx_f32->InsertBefore(builtin);

            // Construct a new coordinate vector.
            auto num_coords = coords->Type()->As<type::Vector>()->Width();
            auto* coord_ty = ty.vec(ty.f32(), num_coords + 1);
            auto* construct = b.Construct(coord_ty, utils::Vector{coords, array_idx_f32->Result()});
            construct->InsertBefore(builtin);
            coords = construct->Result();
        }

        // Determine which SPIR-V intrinsic to use and which optional image operands are needed.
        enum IntrinsicCall::Kind intrinsic;
        struct ImageOperands {
            Value* bias = nullptr;
            Value* lod = nullptr;
            Value* ddx = nullptr;
            Value* ddy = nullptr;
            Value* offset = nullptr;
            Value* sample = nullptr;
        } operands;
        switch (builtin->Func()) {
            case builtin::Function::kTextureSample:
                intrinsic = IntrinsicCall::Kind::kSpirvImageSampleImplicitLod;
                operands.offset = next_arg();
                break;
            case builtin::Function::kTextureSampleBias:
                intrinsic = IntrinsicCall::Kind::kSpirvImageSampleImplicitLod;
                operands.bias = next_arg();
                operands.offset = next_arg();
                break;
            case builtin::Function::kTextureSampleCompare:
                intrinsic = IntrinsicCall::Kind::kSpirvImageSampleDrefImplicitLod;
                depth = next_arg();
                operands.offset = next_arg();
                break;
            case builtin::Function::kTextureSampleCompareLevel:
                intrinsic = IntrinsicCall::Kind::kSpirvImageSampleDrefExplicitLod;
                depth = next_arg();
                operands.lod = b.Constant(0_f);
                operands.offset = next_arg();
                break;
            case builtin::Function::kTextureSampleGrad:
                intrinsic = IntrinsicCall::Kind::kSpirvImageSampleExplicitLod;
                operands.ddx = next_arg();
                operands.ddy = next_arg();
                operands.offset = next_arg();
                break;
            case builtin::Function::kTextureSampleLevel:
                intrinsic = IntrinsicCall::Kind::kSpirvImageSampleExplicitLod;
                operands.lod = next_arg();
                operands.offset = next_arg();
                break;
            default:
                return nullptr;
        }

        // Start building the argument list for the intrinsic.
        // The first two operands are always the sampled image and then the coordinates, followed by
        // the depth reference if used.
        utils::Vector<Value*, 8> intrinsic_args;
        intrinsic_args.Push(sampled_image->Result());
        intrinsic_args.Push(coords);
        if (depth) {
            intrinsic_args.Push(depth);
        }

        // Add a placeholder argument for the image operand mask, which we'll fill in when we've
        // processed the image operands.
        uint32_t image_operand_mask = 0u;
        size_t mask_idx = intrinsic_args.Length();
        intrinsic_args.Push(nullptr);

        // Add each of the optional image operands if used, updating the image operand mask.
        if (operands.bias) {
            image_operand_mask |= SpvImageOperandsBiasMask;
            intrinsic_args.Push(operands.bias);
        }
        if (operands.lod) {
            image_operand_mask |= SpvImageOperandsLodMask;
            if (operands.lod->Type()->is_integer_scalar()) {
                // Some builtins take the lod as an integer, but SPIR-V always requires an f32.
                auto* convert = b.Convert(ty.f32(), operands.lod);
                convert->InsertBefore(builtin);
                operands.lod = convert->Result();
            }
            intrinsic_args.Push(operands.lod);
        }
        if (operands.ddx) {
            image_operand_mask |= SpvImageOperandsGradMask;
            intrinsic_args.Push(operands.ddx);
            intrinsic_args.Push(operands.ddy);
        }
        if (operands.offset) {
            image_operand_mask |= SpvImageOperandsConstOffsetMask;
            intrinsic_args.Push(operands.offset);
        }
        if (operands.sample) {
            image_operand_mask |= SpvImageOperandsSampleMask;
            intrinsic_args.Push(operands.sample);
        }

        // Replace the image operand mask with the final mask value, as a literal operand.
        auto* literal = ir->constant_values.Get(u32(image_operand_mask));
        intrinsic_args[mask_idx] = ir->values.Create<LiteralOperand>(literal);

        // Call the intrinsic.
        // If this is a depth comparison, the result is always f32, otherwise vec4f.
        auto* result_ty = depth ? static_cast<const type::Type*>(ty.f32()) : ty.vec4<f32>();
        auto* texture_call = b.Call(result_ty, intrinsic, std::move(intrinsic_args));
        texture_call->InsertBefore(builtin);

        auto* result = texture_call->Result();

        // If this is not a depth comparison but we are sampling a depth texture, extract the first
        // component to get the scalar f32 that SPIR-V expects.
        if (!depth && texture_ty->IsAnyOf<type::DepthTexture, type::DepthMultisampledTexture>()) {
            auto* extract = b.Access(ty.f32(), result, 0_u);
            extract->InsertBefore(builtin);
            result = extract->Result();
        }

        return result;
    }
};

void BuiltinPolyfillSpirv::Run(ir::Module* ir, const DataMap&, DataMap&) const {
    State{ir}.Process();
}

BuiltinPolyfillSpirv::LiteralOperand::LiteralOperand(const constant::Value* value) : Base(value) {}

BuiltinPolyfillSpirv::LiteralOperand::~LiteralOperand() = default;

BuiltinPolyfillSpirv::SampledImage::SampledImage(const type::Type* image)
    : Base(static_cast<size_t>(
               utils::Hash(utils::TypeInfo::Of<BuiltinPolyfillSpirv::SampledImage>().full_hashcode,
                           image)),
           type::Flags{}),
      image_(image) {}

BuiltinPolyfillSpirv::SampledImage* BuiltinPolyfillSpirv::SampledImage::Clone(
    type::CloneContext& ctx) const {
    auto* image = image_->Clone(ctx);
    return ctx.dst.mgr->Get<BuiltinPolyfillSpirv::SampledImage>(image);
}

}  // namespace tint::ir::transform
