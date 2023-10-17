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

#include "src/tint/lang/msl/writer/ast_printer/helper_test.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::msl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using MslASTPrinterTest = TestHelper;

enum class CallParamType {
    kF32,
    kU32,
    kBool,
    kF16,
};

struct BuiltinData {
    wgsl::BuiltinFn builtin;
    CallParamType type;
    const char* msl_name;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    out << data.msl_name << "<";
    switch (data.type) {
        case CallParamType::kF32:
            out << "f32";
            break;
        case CallParamType::kU32:
            out << "u32";
            break;
        case CallParamType::kBool:
            out << "bool";
            break;
        case CallParamType::kF16:
            out << "f16";
            break;
    }
    out << ">";
    return out;
}

const ast::CallExpression* GenerateCall(wgsl::BuiltinFn builtin,
                                        CallParamType type,
                                        ProgramBuilder* builder) {
    std::string name;
    StringStream str;
    str << name << builtin;
    switch (builtin) {
        case wgsl::BuiltinFn::kAcos:
        case wgsl::BuiltinFn::kAsin:
        case wgsl::BuiltinFn::kAtan:
        case wgsl::BuiltinFn::kCeil:
        case wgsl::BuiltinFn::kCos:
        case wgsl::BuiltinFn::kCosh:
        case wgsl::BuiltinFn::kDpdx:
        case wgsl::BuiltinFn::kDpdxCoarse:
        case wgsl::BuiltinFn::kDpdxFine:
        case wgsl::BuiltinFn::kDpdy:
        case wgsl::BuiltinFn::kDpdyCoarse:
        case wgsl::BuiltinFn::kDpdyFine:
        case wgsl::BuiltinFn::kExp:
        case wgsl::BuiltinFn::kExp2:
        case wgsl::BuiltinFn::kFloor:
        case wgsl::BuiltinFn::kFract:
        case wgsl::BuiltinFn::kFwidth:
        case wgsl::BuiltinFn::kFwidthCoarse:
        case wgsl::BuiltinFn::kFwidthFine:
        case wgsl::BuiltinFn::kInverseSqrt:
        case wgsl::BuiltinFn::kLength:
        case wgsl::BuiltinFn::kLog:
        case wgsl::BuiltinFn::kLog2:
        case wgsl::BuiltinFn::kNormalize:
        case wgsl::BuiltinFn::kRound:
        case wgsl::BuiltinFn::kSin:
        case wgsl::BuiltinFn::kSinh:
        case wgsl::BuiltinFn::kSqrt:
        case wgsl::BuiltinFn::kTan:
        case wgsl::BuiltinFn::kTanh:
        case wgsl::BuiltinFn::kTrunc:
        case wgsl::BuiltinFn::kSign:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2");
            } else {
                return builder->Call(str.str(), "f2");
            }
        case wgsl::BuiltinFn::kLdexp:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2", "i2");
            } else {
                return builder->Call(str.str(), "f2", "i2");
            }
        case wgsl::BuiltinFn::kAtan2:
        case wgsl::BuiltinFn::kDot:
        case wgsl::BuiltinFn::kDistance:
        case wgsl::BuiltinFn::kPow:
        case wgsl::BuiltinFn::kReflect:
        case wgsl::BuiltinFn::kStep:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2", "h2");
            } else {
                return builder->Call(str.str(), "f2", "f2");
            }
        case wgsl::BuiltinFn::kStorageBarrier:
            return builder->Call(str.str());
        case wgsl::BuiltinFn::kCross:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h3", "h3");
            } else {
                return builder->Call(str.str(), "f3", "f3");
            }
        case wgsl::BuiltinFn::kFma:
        case wgsl::BuiltinFn::kMix:
        case wgsl::BuiltinFn::kFaceForward:
        case wgsl::BuiltinFn::kSmoothstep:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2", "h2", "h2");
            } else {
                return builder->Call(str.str(), "f2", "f2", "f2");
            }
        case wgsl::BuiltinFn::kAll:
        case wgsl::BuiltinFn::kAny:
            return builder->Call(str.str(), "b2");
        case wgsl::BuiltinFn::kAbs:
            if (type == CallParamType::kF32) {
                return builder->Call(str.str(), "f2");
            } else if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2");
            } else {
                return builder->Call(str.str(), "u2");
            }
        case wgsl::BuiltinFn::kCountLeadingZeros:
        case wgsl::BuiltinFn::kCountOneBits:
        case wgsl::BuiltinFn::kCountTrailingZeros:
        case wgsl::BuiltinFn::kReverseBits:
            return builder->Call(str.str(), "u2");
        case wgsl::BuiltinFn::kExtractBits:
            return builder->Call(str.str(), "u2", "u1", "u1");
        case wgsl::BuiltinFn::kInsertBits:
            return builder->Call(str.str(), "u2", "u2", "u1", "u1");
        case wgsl::BuiltinFn::kMax:
        case wgsl::BuiltinFn::kMin:
            if (type == CallParamType::kF32) {
                return builder->Call(str.str(), "f2", "f2");
            } else if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2", "h2");
            } else {
                return builder->Call(str.str(), "u2", "u2");
            }
        case wgsl::BuiltinFn::kClamp:
            if (type == CallParamType::kF32) {
                return builder->Call(str.str(), "f2", "f2", "f2");
            } else if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2", "h2", "h2");
            } else {
                return builder->Call(str.str(), "u2", "u2", "u2");
            }
        case wgsl::BuiltinFn::kSelect:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2", "h2", "b2");
            } else {
                return builder->Call(str.str(), "f2", "f2", "b2");
            }
        case wgsl::BuiltinFn::kDeterminant:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "hm2x2");
            } else {
                return builder->Call(str.str(), "m2x2");
            }
        case wgsl::BuiltinFn::kPack2X16Snorm:
        case wgsl::BuiltinFn::kPack2X16Unorm:
            return builder->Call(str.str(), "f2");
        case wgsl::BuiltinFn::kPack4X8Snorm:
        case wgsl::BuiltinFn::kPack4X8Unorm:
            return builder->Call(str.str(), "f4");
        case wgsl::BuiltinFn::kUnpack4X8Snorm:
        case wgsl::BuiltinFn::kUnpack4X8Unorm:
        case wgsl::BuiltinFn::kUnpack2X16Snorm:
        case wgsl::BuiltinFn::kUnpack2X16Unorm:
            return builder->Call(str.str(), "u1");
        case wgsl::BuiltinFn::kWorkgroupBarrier:
            return builder->Call(str.str());
        case wgsl::BuiltinFn::kTranspose:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "hm3x2");
            } else {
                return builder->Call(str.str(), "m3x2");
            }
        default:
            break;
    }
    return nullptr;
}

using MslBuiltinTest = TestParamHelper<BuiltinData>;
TEST_P(MslBuiltinTest, Emit) {
    auto param = GetParam();

    if (param.type == CallParamType::kF16) {
        Enable(wgsl::Extension::kF16);

        GlobalVar("h2", ty.vec2<f16>(), core::AddressSpace::kPrivate);
        GlobalVar("h3", ty.vec3<f16>(), core::AddressSpace::kPrivate);
        GlobalVar("hm2x2", ty.mat2x2<f16>(), core::AddressSpace::kPrivate);
        GlobalVar("hm3x2", ty.mat3x2<f16>(), core::AddressSpace::kPrivate);
    }

    GlobalVar("f2", ty.vec2<f32>(), core::AddressSpace::kPrivate);
    GlobalVar("f3", ty.vec3<f32>(), core::AddressSpace::kPrivate);
    GlobalVar("f4", ty.vec4<f32>(), core::AddressSpace::kPrivate);
    GlobalVar("u1", ty.u32(), core::AddressSpace::kPrivate);
    GlobalVar("u2", ty.vec2<u32>(), core::AddressSpace::kPrivate);
    GlobalVar("i2", ty.vec2<i32>(), core::AddressSpace::kPrivate);
    GlobalVar("b2", ty.vec2<bool>(), core::AddressSpace::kPrivate);
    GlobalVar("m2x2", ty.mat2x2<f32>(), core::AddressSpace::kPrivate);
    GlobalVar("m3x2", ty.mat3x2<f32>(), core::AddressSpace::kPrivate);

    auto* call = GenerateCall(param.builtin, param.type, this);
    ASSERT_NE(nullptr, call) << "Unhandled builtin";
    Func("func", tint::Empty, ty.void_(), Vector{Ignore(call)},
         Vector{create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

    ASTPrinter& gen = Build();

    auto* sem = program->Sem().Get<sem::Call>(call);
    ASSERT_NE(sem, nullptr);
    auto* target = sem->Target();
    ASSERT_NE(target, nullptr);
    auto* builtin = target->As<sem::BuiltinFn>();
    ASSERT_NE(builtin, nullptr);

    EXPECT_EQ(gen.generate_builtin_name(builtin), param.msl_name);
}
INSTANTIATE_TEST_SUITE_P(
    MslASTPrinterTest,
    MslBuiltinTest,
    testing::Values(
        /* Logical built-in */
        BuiltinData{wgsl::BuiltinFn::kAll, CallParamType::kBool, "all"},
        BuiltinData{wgsl::BuiltinFn::kAny, CallParamType::kBool, "any"},
        BuiltinData{wgsl::BuiltinFn::kSelect, CallParamType::kF32, "select"},
        /* Float built-in */
        BuiltinData{wgsl::BuiltinFn::kAbs, CallParamType::kF32, "fabs"},
        BuiltinData{wgsl::BuiltinFn::kAbs, CallParamType::kF16, "fabs"},
        BuiltinData{wgsl::BuiltinFn::kAcos, CallParamType::kF32, "acos"},
        BuiltinData{wgsl::BuiltinFn::kAcos, CallParamType::kF16, "acos"},
        BuiltinData{wgsl::BuiltinFn::kAsin, CallParamType::kF32, "asin"},
        BuiltinData{wgsl::BuiltinFn::kAsin, CallParamType::kF16, "asin"},
        BuiltinData{wgsl::BuiltinFn::kAtan, CallParamType::kF32, "atan"},
        BuiltinData{wgsl::BuiltinFn::kAtan, CallParamType::kF16, "atan"},
        BuiltinData{wgsl::BuiltinFn::kAtan2, CallParamType::kF32, "atan2"},
        BuiltinData{wgsl::BuiltinFn::kAtan2, CallParamType::kF16, "atan2"},
        BuiltinData{wgsl::BuiltinFn::kCeil, CallParamType::kF32, "ceil"},
        BuiltinData{wgsl::BuiltinFn::kCeil, CallParamType::kF16, "ceil"},
        BuiltinData{wgsl::BuiltinFn::kClamp, CallParamType::kF32, "clamp"},
        BuiltinData{wgsl::BuiltinFn::kClamp, CallParamType::kF16, "clamp"},
        BuiltinData{wgsl::BuiltinFn::kCos, CallParamType::kF32, "cos"},
        BuiltinData{wgsl::BuiltinFn::kCos, CallParamType::kF16, "cos"},
        BuiltinData{wgsl::BuiltinFn::kCosh, CallParamType::kF32, "cosh"},
        BuiltinData{wgsl::BuiltinFn::kCosh, CallParamType::kF16, "cosh"},
        BuiltinData{wgsl::BuiltinFn::kCross, CallParamType::kF32, "cross"},
        BuiltinData{wgsl::BuiltinFn::kCross, CallParamType::kF16, "cross"},
        BuiltinData{wgsl::BuiltinFn::kDistance, CallParamType::kF32, "distance"},
        BuiltinData{wgsl::BuiltinFn::kDistance, CallParamType::kF16, "distance"},
        BuiltinData{wgsl::BuiltinFn::kExp, CallParamType::kF32, "exp"},
        BuiltinData{wgsl::BuiltinFn::kExp, CallParamType::kF16, "exp"},
        BuiltinData{wgsl::BuiltinFn::kExp2, CallParamType::kF32, "exp2"},
        BuiltinData{wgsl::BuiltinFn::kExp2, CallParamType::kF16, "exp2"},
        BuiltinData{wgsl::BuiltinFn::kFaceForward, CallParamType::kF32, "faceforward"},
        BuiltinData{wgsl::BuiltinFn::kFaceForward, CallParamType::kF16, "faceforward"},
        BuiltinData{wgsl::BuiltinFn::kFloor, CallParamType::kF32, "floor"},
        BuiltinData{wgsl::BuiltinFn::kFloor, CallParamType::kF16, "floor"},
        BuiltinData{wgsl::BuiltinFn::kFma, CallParamType::kF32, "fma"},
        BuiltinData{wgsl::BuiltinFn::kFma, CallParamType::kF16, "fma"},
        BuiltinData{wgsl::BuiltinFn::kFract, CallParamType::kF32, "fract"},
        BuiltinData{wgsl::BuiltinFn::kFract, CallParamType::kF16, "fract"},
        BuiltinData{wgsl::BuiltinFn::kInverseSqrt, CallParamType::kF32, "rsqrt"},
        BuiltinData{wgsl::BuiltinFn::kInverseSqrt, CallParamType::kF16, "rsqrt"},
        BuiltinData{wgsl::BuiltinFn::kLdexp, CallParamType::kF32, "ldexp"},
        BuiltinData{wgsl::BuiltinFn::kLdexp, CallParamType::kF16, "ldexp"},
        BuiltinData{wgsl::BuiltinFn::kLength, CallParamType::kF32, "length"},
        BuiltinData{wgsl::BuiltinFn::kLength, CallParamType::kF16, "length"},
        BuiltinData{wgsl::BuiltinFn::kLog, CallParamType::kF32, "log"},
        BuiltinData{wgsl::BuiltinFn::kLog, CallParamType::kF16, "log"},
        BuiltinData{wgsl::BuiltinFn::kLog2, CallParamType::kF32, "log2"},
        BuiltinData{wgsl::BuiltinFn::kLog2, CallParamType::kF16, "log2"},
        BuiltinData{wgsl::BuiltinFn::kMax, CallParamType::kF32, "fmax"},
        BuiltinData{wgsl::BuiltinFn::kMax, CallParamType::kF16, "fmax"},
        BuiltinData{wgsl::BuiltinFn::kMin, CallParamType::kF32, "fmin"},
        BuiltinData{wgsl::BuiltinFn::kMin, CallParamType::kF16, "fmin"},
        BuiltinData{wgsl::BuiltinFn::kNormalize, CallParamType::kF32, "normalize"},
        BuiltinData{wgsl::BuiltinFn::kNormalize, CallParamType::kF16, "normalize"},
        BuiltinData{wgsl::BuiltinFn::kPow, CallParamType::kF32, "pow"},
        BuiltinData{wgsl::BuiltinFn::kPow, CallParamType::kF16, "pow"},
        BuiltinData{wgsl::BuiltinFn::kReflect, CallParamType::kF32, "reflect"},
        BuiltinData{wgsl::BuiltinFn::kReflect, CallParamType::kF16, "reflect"},
        BuiltinData{wgsl::BuiltinFn::kSign, CallParamType::kF32, "sign"},
        BuiltinData{wgsl::BuiltinFn::kSign, CallParamType::kF16, "sign"},
        BuiltinData{wgsl::BuiltinFn::kSin, CallParamType::kF32, "sin"},
        BuiltinData{wgsl::BuiltinFn::kSin, CallParamType::kF16, "sin"},
        BuiltinData{wgsl::BuiltinFn::kSinh, CallParamType::kF32, "sinh"},
        BuiltinData{wgsl::BuiltinFn::kSinh, CallParamType::kF16, "sinh"},
        BuiltinData{wgsl::BuiltinFn::kSmoothstep, CallParamType::kF32, "smoothstep"},
        BuiltinData{wgsl::BuiltinFn::kSmoothstep, CallParamType::kF16, "smoothstep"},
        BuiltinData{wgsl::BuiltinFn::kSqrt, CallParamType::kF32, "sqrt"},
        BuiltinData{wgsl::BuiltinFn::kSqrt, CallParamType::kF16, "sqrt"},
        BuiltinData{wgsl::BuiltinFn::kStep, CallParamType::kF32, "step"},
        BuiltinData{wgsl::BuiltinFn::kStep, CallParamType::kF16, "step"},
        BuiltinData{wgsl::BuiltinFn::kTan, CallParamType::kF32, "tan"},
        BuiltinData{wgsl::BuiltinFn::kTan, CallParamType::kF16, "tan"},
        BuiltinData{wgsl::BuiltinFn::kTanh, CallParamType::kF32, "tanh"},
        BuiltinData{wgsl::BuiltinFn::kTanh, CallParamType::kF16, "tanh"},
        BuiltinData{wgsl::BuiltinFn::kTrunc, CallParamType::kF32, "trunc"},
        BuiltinData{wgsl::BuiltinFn::kTrunc, CallParamType::kF16, "trunc"},
        /* Integer built-in */
        BuiltinData{wgsl::BuiltinFn::kAbs, CallParamType::kU32, "abs"},
        BuiltinData{wgsl::BuiltinFn::kClamp, CallParamType::kU32, "clamp"},
        BuiltinData{wgsl::BuiltinFn::kCountLeadingZeros, CallParamType::kU32, "clz"},
        BuiltinData{wgsl::BuiltinFn::kCountOneBits, CallParamType::kU32, "popcount"},
        BuiltinData{wgsl::BuiltinFn::kCountTrailingZeros, CallParamType::kU32, "ctz"},
        BuiltinData{wgsl::BuiltinFn::kExtractBits, CallParamType::kU32, "extract_bits"},
        BuiltinData{wgsl::BuiltinFn::kInsertBits, CallParamType::kU32, "insert_bits"},
        BuiltinData{wgsl::BuiltinFn::kMax, CallParamType::kU32, "max"},
        BuiltinData{wgsl::BuiltinFn::kMin, CallParamType::kU32, "min"},
        BuiltinData{wgsl::BuiltinFn::kReverseBits, CallParamType::kU32, "reverse_bits"},
        BuiltinData{wgsl::BuiltinFn::kRound, CallParamType::kU32, "rint"},
        /* Matrix built-in */
        BuiltinData{wgsl::BuiltinFn::kDeterminant, CallParamType::kF32, "determinant"},
        BuiltinData{wgsl::BuiltinFn::kTranspose, CallParamType::kF32, "transpose"},
        /* Vector built-in */
        BuiltinData{wgsl::BuiltinFn::kDot, CallParamType::kF32, "dot"},
        /* Derivate built-in */
        BuiltinData{wgsl::BuiltinFn::kDpdx, CallParamType::kF32, "dfdx"},
        BuiltinData{wgsl::BuiltinFn::kDpdxCoarse, CallParamType::kF32, "dfdx"},
        BuiltinData{wgsl::BuiltinFn::kDpdxFine, CallParamType::kF32, "dfdx"},
        BuiltinData{wgsl::BuiltinFn::kDpdy, CallParamType::kF32, "dfdy"},
        BuiltinData{wgsl::BuiltinFn::kDpdyCoarse, CallParamType::kF32, "dfdy"},
        BuiltinData{wgsl::BuiltinFn::kDpdyFine, CallParamType::kF32, "dfdy"},
        BuiltinData{wgsl::BuiltinFn::kFwidth, CallParamType::kF32, "fwidth"},
        BuiltinData{wgsl::BuiltinFn::kFwidthCoarse, CallParamType::kF32, "fwidth"},
        BuiltinData{wgsl::BuiltinFn::kFwidthFine, CallParamType::kF32, "fwidth"},
        /* Data packing builtin */
        BuiltinData{wgsl::BuiltinFn::kPack4X8Snorm, CallParamType::kF32, "pack_float_to_snorm4x8"},
        BuiltinData{wgsl::BuiltinFn::kPack4X8Unorm, CallParamType::kF32, "pack_float_to_unorm4x8"},
        BuiltinData{wgsl::BuiltinFn::kPack2X16Snorm, CallParamType::kF32,
                    "pack_float_to_snorm2x16"},
        BuiltinData{wgsl::BuiltinFn::kPack2X16Unorm, CallParamType::kF32,
                    "pack_float_to_unorm2x16"},
        /* Data unpacking builtin */
        BuiltinData{wgsl::BuiltinFn::kUnpack4X8Snorm, CallParamType::kU32,
                    "unpack_snorm4x8_to_float"},
        BuiltinData{wgsl::BuiltinFn::kUnpack4X8Unorm, CallParamType::kU32,
                    "unpack_unorm4x8_to_float"},
        BuiltinData{wgsl::BuiltinFn::kUnpack2X16Snorm, CallParamType::kU32,
                    "unpack_snorm2x16_to_float"},
        BuiltinData{wgsl::BuiltinFn::kUnpack2X16Unorm, CallParamType::kU32,
                    "unpack_unorm2x16_to_float"}));

TEST_F(MslASTPrinterTest, Builtin_Call) {
    GlobalVar("param1", ty.vec2<f32>(), core::AddressSpace::kPrivate);
    GlobalVar("param2", ty.vec2<f32>(), core::AddressSpace::kPrivate);

    auto* call = Call("dot", "param1", "param2");
    WrapInFunction(Decl(Var("r", call)));

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "dot(param1, param2)");
}

TEST_F(MslASTPrinterTest, StorageBarrier) {
    auto* call = Call("storageBarrier");
    WrapInFunction(CallStmt(call));

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "threadgroup_barrier(mem_flags::mem_device)");
}

TEST_F(MslASTPrinterTest, WorkgroupBarrier) {
    auto* call = Call("workgroupBarrier");
    WrapInFunction(CallStmt(call));

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "threadgroup_barrier(mem_flags::mem_threadgroup)");
}

TEST_F(MslASTPrinterTest, Runtime_Modf_Scalar_f32) {
    WrapInFunction(Decl(Let("f", Expr(1.5_f))),  //
                   Decl(Let("v", Call("modf", "f"))));

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

struct modf_result_f32 {
  float fract;
  float whole;
};
modf_result_f32 tint_modf(float param_0) {
  modf_result_f32 result;
  result.fract = modf(param_0, result.whole);
  return result;
}

kernel void test_function() {
  float const f = 1.5f;
  modf_result_f32 const v = tint_modf(f);
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Runtime_Modf_Scalar_f16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Decl(Let("f", Expr(1.5_h))),  //
                   Decl(Let("v", Call("modf", "f"))));

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

struct modf_result_f16 {
  half fract;
  half whole;
};
modf_result_f16 tint_modf(half param_0) {
  modf_result_f16 result;
  result.fract = modf(param_0, result.whole);
  return result;
}

kernel void test_function() {
  half const f = 1.5h;
  modf_result_f16 const v = tint_modf(f);
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Runtime_Modf_Vector_f32) {
    WrapInFunction(Decl(Let("f", Call<vec3<f32>>(1.5_f, 2.5_f, 3.5_f))),  //
                   Decl(Let("v", Call("modf", "f"))));

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

struct modf_result_vec3_f32 {
  float3 fract;
  float3 whole;
};
modf_result_vec3_f32 tint_modf(float3 param_0) {
  modf_result_vec3_f32 result;
  result.fract = modf(param_0, result.whole);
  return result;
}

kernel void test_function() {
  float3 const f = float3(1.5f, 2.5f, 3.5f);
  modf_result_vec3_f32 const v = tint_modf(f);
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Runtime_Modf_Vector_f16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Decl(Let("f", Call<vec3<f16>>(1.5_h, 2.5_h, 3.5_h))),  //
                   Decl(Let("v", Call("modf", "f"))));

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

struct modf_result_vec3_f16 {
  half3 fract;
  half3 whole;
};
modf_result_vec3_f16 tint_modf(half3 param_0) {
  modf_result_vec3_f16 result;
  result.fract = modf(param_0, result.whole);
  return result;
}

kernel void test_function() {
  half3 const f = half3(1.5h, 2.5h, 3.5h);
  modf_result_vec3_f16 const v = tint_modf(f);
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Const_Modf_Scalar_f32) {
    WrapInFunction(Decl(Let("v", Call("modf", 1.5_f))));

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

struct modf_result_f32 {
  float fract;
  float whole;
};
kernel void test_function() {
  modf_result_f32 const v = modf_result_f32{.fract=0.5f, .whole=1.0f};
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Const_Modf_Scalar_f16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Decl(Let("v", Call("modf", 1.5_h))));

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

struct modf_result_f16 {
  half fract;
  half whole;
};
kernel void test_function() {
  modf_result_f16 const v = modf_result_f16{.fract=0.5h, .whole=1.0h};
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Const_Modf_Vector_f32) {
    WrapInFunction(Decl(Let("v", Call("modf", Call<vec3<f32>>(1.5_f, 2.5_f, 3.5_f)))));

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

struct modf_result_vec3_f32 {
  float3 fract;
  float3 whole;
};
kernel void test_function() {
  modf_result_vec3_f32 const v = modf_result_vec3_f32{.fract=float3(0.5f), .whole=float3(1.0f, 2.0f, 3.0f)};
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Const_Modf_Vector_f16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Decl(Let("v", Call("modf", Call<vec3<f16>>(1.5_h, 2.5_h, 3.5_h)))));

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

struct modf_result_vec3_f16 {
  half3 fract;
  half3 whole;
};
kernel void test_function() {
  modf_result_vec3_f16 const v = modf_result_vec3_f16{.fract=half3(0.5h), .whole=half3(1.0h, 2.0h, 3.0h)};
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Runtime_Frexp_Scalar_f32) {
    WrapInFunction(Var("f", Expr(1_f)),  //
                   Var("v", Call("frexp", "f")));

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

struct frexp_result_f32 {
  float fract;
  int exp;
};
frexp_result_f32 tint_frexp(float param_0) {
  frexp_result_f32 result;
  result.fract = frexp(param_0, result.exp);
  return result;
}

kernel void test_function() {
  float f = 1.0f;
  frexp_result_f32 v = tint_frexp(f);
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Runtime_Frexp_Scalar_f16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Var("f", Expr(1_h)),  //
                   Var("v", Call("frexp", "f")));

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

struct frexp_result_f16 {
  half fract;
  int exp;
};
frexp_result_f16 tint_frexp(half param_0) {
  frexp_result_f16 result;
  result.fract = frexp(param_0, result.exp);
  return result;
}

kernel void test_function() {
  half f = 1.0h;
  frexp_result_f16 v = tint_frexp(f);
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Runtime_Frexp_Vector_f32) {
    WrapInFunction(Var("f", Call<vec3<f32>>()),  //
                   Var("v", Call("frexp", "f")));

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

struct frexp_result_vec3_f32 {
  float3 fract;
  int3 exp;
};
frexp_result_vec3_f32 tint_frexp(float3 param_0) {
  frexp_result_vec3_f32 result;
  result.fract = frexp(param_0, result.exp);
  return result;
}

kernel void test_function() {
  float3 f = float3(0.0f);
  frexp_result_vec3_f32 v = tint_frexp(f);
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Runtime_Frexp_Vector_f16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Var("f", Call<vec3<f16>>()),  //
                   Var("v", Call("frexp", "f")));

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

struct frexp_result_vec3_f16 {
  half3 fract;
  int3 exp;
};
frexp_result_vec3_f16 tint_frexp(half3 param_0) {
  frexp_result_vec3_f16 result;
  result.fract = frexp(param_0, result.exp);
  return result;
}

kernel void test_function() {
  half3 f = half3(0.0h);
  frexp_result_vec3_f16 v = tint_frexp(f);
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Const_Frexp_Scalar_f32) {
    WrapInFunction(Decl(Let("v", Call("frexp", 1_f))));

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

struct frexp_result_f32 {
  float fract;
  int exp;
};
kernel void test_function() {
  frexp_result_f32 const v = frexp_result_f32{.fract=0.5f, .exp=1};
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Const_Frexp_Scalar_f16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Decl(Let("v", Call("frexp", 1_h))));

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

struct frexp_result_f16 {
  half fract;
  int exp;
};
kernel void test_function() {
  frexp_result_f16 const v = frexp_result_f16{.fract=0.5h, .exp=1};
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Const_Frexp_Vector_f32) {
    WrapInFunction(Decl(Let("v", Call("frexp", Call<vec3<f32>>()))));

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

struct frexp_result_vec3_f32 {
  float3 fract;
  int3 exp;
};
kernel void test_function() {
  frexp_result_vec3_f32 const v = frexp_result_vec3_f32{};
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Const_Frexp_Vector_f16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Decl(Let("v", Call("frexp", Call<vec3<f16>>()))));

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

struct frexp_result_vec3_f16 {
  half3 fract;
  int3 exp;
};
kernel void test_function() {
  frexp_result_vec3_f16 const v = frexp_result_vec3_f16{};
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Degrees_Scalar_f32) {
    auto* val = Var("val", ty.f32());
    auto* call = Call("degrees", val);
    WrapInFunction(val, call);

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

float tint_degrees(float param_0) {
  return param_0 * 57.29577951308232286465;
}

kernel void test_function() {
  float val = 0.0f;
  float const tint_symbol = tint_degrees(val);
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Degrees_Vector_f32) {
    auto* val = Var("val", ty.vec3<f32>());
    auto* call = Call("degrees", val);
    WrapInFunction(val, call);

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

float3 tint_degrees(float3 param_0) {
  return param_0 * 57.29577951308232286465;
}

kernel void test_function() {
  float3 val = 0.0f;
  float3 const tint_symbol = tint_degrees(val);
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Degrees_Scalar_f16) {
    Enable(wgsl::Extension::kF16);

    auto* val = Var("val", ty.f16());
    auto* call = Call("degrees", val);
    WrapInFunction(val, call);

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

half tint_degrees(half param_0) {
  return param_0 * 57.29577951308232286465;
}

kernel void test_function() {
  half val = 0.0h;
  half const tint_symbol = tint_degrees(val);
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Degrees_Vector_f16) {
    Enable(wgsl::Extension::kF16);

    auto* val = Var("val", ty.vec3<f16>());
    auto* call = Call("degrees", val);
    WrapInFunction(val, call);

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

half3 tint_degrees(half3 param_0) {
  return param_0 * 57.29577951308232286465;
}

kernel void test_function() {
  half3 val = 0.0h;
  half3 const tint_symbol = tint_degrees(val);
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Radians_Scalar_f32) {
    auto* val = Var("val", ty.f32());
    auto* call = Call("radians", val);
    WrapInFunction(val, call);

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

float tint_radians(float param_0) {
  return param_0 * 0.01745329251994329547;
}

kernel void test_function() {
  float val = 0.0f;
  float const tint_symbol = tint_radians(val);
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Radians_Vector_f32) {
    auto* val = Var("val", ty.vec3<f32>());
    auto* call = Call("radians", val);
    WrapInFunction(val, call);

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

float3 tint_radians(float3 param_0) {
  return param_0 * 0.01745329251994329547;
}

kernel void test_function() {
  float3 val = 0.0f;
  float3 const tint_symbol = tint_radians(val);
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Radians_Scalar_f16) {
    Enable(wgsl::Extension::kF16);

    auto* val = Var("val", ty.f16());
    auto* call = Call("radians", val);
    WrapInFunction(val, call);

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

half tint_radians(half param_0) {
  return param_0 * 0.01745329251994329547;
}

kernel void test_function() {
  half val = 0.0h;
  half const tint_symbol = tint_radians(val);
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Radians_Vector_f16) {
    Enable(wgsl::Extension::kF16);

    auto* val = Var("val", ty.vec3<f16>());
    auto* call = Call("radians", val);
    WrapInFunction(val, call);

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

half3 tint_radians(half3 param_0) {
  return param_0 * 0.01745329251994329547;
}

kernel void test_function() {
  half3 val = 0.0h;
  half3 const tint_symbol = tint_radians(val);
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Pack2x16Float) {
    auto* call = Call("pack2x16float", "p1");
    GlobalVar("p1", ty.vec2<f32>(), core::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "as_type<uint>(half2(p1))");
}

TEST_F(MslASTPrinterTest, Unpack2x16Float) {
    auto* call = Call("unpack2x16float", "p1");
    GlobalVar("p1", ty.u32(), core::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "float2(as_type<half2>(p1))");
}

TEST_F(MslASTPrinterTest, DotI32) {
    GlobalVar("v", ty.vec3<i32>(), core::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", Call("dot", "v", "v"))));

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;

template<typename T>
T tint_dot3(vec<T,3> a, vec<T,3> b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}
struct tint_private_vars_struct {
  int3 v;
};

kernel void test_function() {
  thread tint_private_vars_struct tint_private_vars = {};
  int r = tint_dot3(tint_private_vars.v, tint_private_vars.v);
  return;
}

)");
}

TEST_F(MslASTPrinterTest, Ignore) {
    Func("f", Vector{Param("a", ty.i32()), Param("b", ty.i32()), Param("c", ty.i32())}, ty.i32(),
         Vector{Return(Mul(Add("a", "b"), "c"))});

    Func("func", tint::Empty, ty.void_(), Vector{CallStmt(Call("f", 1_i, 2_i, 3_i))},
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;
int f(int a, int b, int c) {
  return as_type<int>((as_type<uint>(as_type<int>((as_type<uint>(a) + as_type<uint>(b)))) * as_type<uint>(c)));
}

kernel void func() {
  f(1, 2, 3);
  return;
}

)");
}

}  // namespace
}  // namespace tint::msl::writer
