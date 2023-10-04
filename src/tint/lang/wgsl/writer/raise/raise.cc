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

#include "src/tint/lang/wgsl/writer/raise/raise.h"

#include <utility>

#include "src/tint/lang/core/builtin_fn.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/wgsl/builtin_fn.h"
#include "src/tint/lang/wgsl/ir/builtin_call.h"

namespace tint::wgsl::writer {
namespace {

wgsl::BuiltinFn Convert(core::BuiltinFn fn) {
#define CASE(NAME)              \
    case core::BuiltinFn::NAME: \
        return wgsl::BuiltinFn::NAME;

    switch (fn) {
        CASE(kAbs)
        CASE(kAcos)
        CASE(kAcosh)
        CASE(kAll)
        CASE(kAny)
        CASE(kArrayLength)
        CASE(kAsin)
        CASE(kAsinh)
        CASE(kAtan)
        CASE(kAtan2)
        CASE(kAtanh)
        CASE(kCeil)
        CASE(kClamp)
        CASE(kCos)
        CASE(kCosh)
        CASE(kCountLeadingZeros)
        CASE(kCountOneBits)
        CASE(kCountTrailingZeros)
        CASE(kCross)
        CASE(kDegrees)
        CASE(kDeterminant)
        CASE(kDistance)
        CASE(kDot)
        CASE(kDot4I8Packed)
        CASE(kDot4U8Packed)
        CASE(kDpdx)
        CASE(kDpdxCoarse)
        CASE(kDpdxFine)
        CASE(kDpdy)
        CASE(kDpdyCoarse)
        CASE(kDpdyFine)
        CASE(kExp)
        CASE(kExp2)
        CASE(kExtractBits)
        CASE(kFaceForward)
        CASE(kFirstLeadingBit)
        CASE(kFirstTrailingBit)
        CASE(kFloor)
        CASE(kFma)
        CASE(kFract)
        CASE(kFrexp)
        CASE(kFwidth)
        CASE(kFwidthCoarse)
        CASE(kFwidthFine)
        CASE(kInsertBits)
        CASE(kInverseSqrt)
        CASE(kLdexp)
        CASE(kLength)
        CASE(kLog)
        CASE(kLog2)
        CASE(kMax)
        CASE(kMin)
        CASE(kMix)
        CASE(kModf)
        CASE(kNormalize)
        CASE(kPack2X16Float)
        CASE(kPack2X16Snorm)
        CASE(kPack2X16Unorm)
        CASE(kPack4X8Snorm)
        CASE(kPack4X8Unorm)
        CASE(kPow)
        CASE(kQuantizeToF16)
        CASE(kRadians)
        CASE(kReflect)
        CASE(kRefract)
        CASE(kReverseBits)
        CASE(kRound)
        CASE(kSaturate)
        CASE(kSelect)
        CASE(kSign)
        CASE(kSin)
        CASE(kSinh)
        CASE(kSmoothstep)
        CASE(kSqrt)
        CASE(kStep)
        CASE(kStorageBarrier)
        CASE(kTan)
        CASE(kTanh)
        CASE(kTranspose)
        CASE(kTrunc)
        CASE(kUnpack2X16Float)
        CASE(kUnpack2X16Snorm)
        CASE(kUnpack2X16Unorm)
        CASE(kUnpack4X8Snorm)
        CASE(kUnpack4X8Unorm)
        CASE(kWorkgroupBarrier)
        CASE(kTextureBarrier)
        CASE(kTextureDimensions)
        CASE(kTextureGather)
        CASE(kTextureGatherCompare)
        CASE(kTextureNumLayers)
        CASE(kTextureNumLevels)
        CASE(kTextureNumSamples)
        CASE(kTextureSample)
        CASE(kTextureSampleBias)
        CASE(kTextureSampleCompare)
        CASE(kTextureSampleCompareLevel)
        CASE(kTextureSampleGrad)
        CASE(kTextureSampleLevel)
        CASE(kTextureSampleBaseClampToEdge)
        CASE(kTextureStore)
        CASE(kTextureLoad)
        CASE(kAtomicLoad)
        CASE(kAtomicStore)
        CASE(kAtomicAdd)
        CASE(kAtomicSub)
        CASE(kAtomicMax)
        CASE(kAtomicMin)
        CASE(kAtomicAnd)
        CASE(kAtomicOr)
        CASE(kAtomicXor)
        CASE(kAtomicExchange)
        CASE(kAtomicCompareExchangeWeak)
        CASE(kSubgroupBallot)
        CASE(kSubgroupBroadcast)
        default:
            TINT_ICE() << "unhandled builtin function: " << fn;
            return wgsl::BuiltinFn::kNone;
    }
}

void ReplaceBuiltinFnCall(core::ir::Module& mod, core::ir::CoreBuiltinCall* call) {
    Vector<core::ir::Value*, 8> args(call->Args());
    auto* replacement = mod.instructions.Create<wgsl::ir::BuiltinCall>(
        call->Result(), Convert(call->Func()), std::move(args));
    call->ReplaceWith(replacement);
    call->ClearResults();
    call->Destroy();
}

void ReplaceWorkgroupBarrier(core::ir::Module& mod, core::ir::CoreBuiltinCall* call) {
    // Pattern match:
    //    call workgroupBarrier
    //    %value = load &ptr
    //    call workgroupBarrier
    // And replace with:
    //    %value = call workgroupUniformLoad %ptr

    auto* load = As<core::ir::Load>(call->next);
    if (!load || load->From()->Type()->As<core::type::Pointer>()->AddressSpace() !=
                     core::AddressSpace::kWorkgroup) {
        // No match
        ReplaceBuiltinFnCall(mod, call);
        return;
    }

    auto* post_load = As<core::ir::CoreBuiltinCall>(load->next);
    if (!post_load || post_load->Func() != core::BuiltinFn::kWorkgroupBarrier) {
        // No match
        ReplaceBuiltinFnCall(mod, call);
        return;
    }

    // Remove both calls to workgroupBarrier
    post_load->Destroy();
    call->Destroy();

    // Replace load with workgroupUniformLoad
    auto* replacement = mod.instructions.Create<wgsl::ir::BuiltinCall>(
        load->Result(), wgsl::BuiltinFn::kWorkgroupUniformLoad, Vector{load->From()});
    load->ReplaceWith(replacement);
    load->ClearResults();
    load->Destroy();
}

}  // namespace

Result<SuccessType> Raise(core::ir::Module& mod) {
    for (auto* inst : mod.instructions.Objects()) {
        if (!inst->Alive()) {
            continue;
        }
        if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
            switch (call->Func()) {
                case core::BuiltinFn::kWorkgroupBarrier:
                    ReplaceWorkgroupBarrier(mod, call);
                    break;
                default:
                    ReplaceBuiltinFnCall(mod, call);
                    break;
            }
        }
    }
    return Success;
}

}  // namespace tint::wgsl::writer
