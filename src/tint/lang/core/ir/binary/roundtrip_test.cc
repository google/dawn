// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/ir_helper_test.h"

#include "src/tint/lang/core/ir/binary/decode.h"
#include "src/tint/lang/core/ir/binary/encode.h"
#include "src/tint/lang/core/ir/disassembler.h"

namespace tint::core::ir::binary {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

template <typename T = testing::Test>
class IRBinaryRoundtripTestBase : public IRTestParamHelper<T> {
  public:
    std::pair<std::string, std::string> Roundtrip() {
        auto pre = Disassemble(this->mod);
        auto encoded = Encode(this->mod);
        if (!encoded) {
            return {pre, encoded.Failure().reason.str()};
        }
        auto decoded = Decode(encoded->Slice());
        if (!decoded) {
            return {pre, decoded.Failure().reason.str()};
        }
        auto post = Disassemble(decoded.Get());
        return {pre, post};
    }
};

#define RUN_TEST()                      \
    {                                   \
        auto [pre, post] = Roundtrip(); \
        EXPECT_EQ(pre, post);           \
    }                                   \
    TINT_REQUIRE_SEMICOLON

using IRBinaryRoundtripTest = IRBinaryRoundtripTestBase<>;
TEST_F(IRBinaryRoundtripTest, EmptyModule) {
    RUN_TEST();
}

////////////////////////////////////////////////////////////////////////////////
// Root block
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRBinaryRoundtripTest, RootBlock_Var_private_i32_Unnamed) {
    b.Append(b.ir.root_block, [&] { b.Var<private_, i32>(); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, RootBlock_Var_workgroup_f32_Named) {
    b.Append(b.ir.root_block, [&] { b.Var<workgroup, f32>("WG"); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, RootBlock_Var_storage_binding) {
    b.Append(b.ir.root_block, [&] {
        auto* v = b.Var<storage, f32>();
        v->SetBindingPoint(10, 20);
    });
    RUN_TEST();
}
////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRBinaryRoundtripTest, Fn_i32_ret) {
    b.Function("Function", ty.i32());
    RUN_TEST();
}

using IRBinaryRoundtripTest_FnPipelineStage = IRBinaryRoundtripTestBase<Function::PipelineStage>;
TEST_P(IRBinaryRoundtripTest_FnPipelineStage, Test) {
    b.Function("Function", ty.i32(), GetParam());
    RUN_TEST();
}
INSTANTIATE_TEST_SUITE_P(,
                         IRBinaryRoundtripTest_FnPipelineStage,
                         testing::Values(Function::PipelineStage::kCompute,
                                         Function::PipelineStage::kFragment,
                                         Function::PipelineStage::kVertex));

TEST_F(IRBinaryRoundtripTest, Fn_WorkgroupSize) {
    b.Function("Function", ty.i32(), Function::PipelineStage::kCompute,
               std::array<uint32_t, 3>{1, 2, 3});
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Fn_Parameters) {
    auto* fn = b.Function("Function", ty.void_());
    auto* p0 = b.FunctionParam(ty.i32());
    auto* p1 = b.FunctionParam(ty.u32());
    auto* p2 = b.FunctionParam(ty.f32());
    b.ir.SetName(p1, "p1");
    fn->SetParams({p0, p1, p2});
    RUN_TEST();
}

////////////////////////////////////////////////////////////////////////////////
// Instructions
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRBinaryRoundtripTest, Return) {
    auto* fn = b.Function("Function", ty.void_());
    b.Append(fn->Block(), [&] { b.Return(fn); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_bool) {
    auto* fn = b.Function("Function", ty.bool_());
    b.Append(fn->Block(), [&] { b.Return(fn, true); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_i32) {
    auto* fn = b.Function("Function", ty.i32());
    b.Append(fn->Block(), [&] { b.Return(fn, 42_i); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_u32) {
    auto* fn = b.Function("Function", ty.u32());
    b.Append(fn->Block(), [&] { b.Return(fn, 42_u); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_f32) {
    auto* fn = b.Function("Function", ty.f32());
    b.Append(fn->Block(), [&] { b.Return(fn, 42_f); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_f16) {
    auto* fn = b.Function("Function", ty.f16());
    b.Append(fn->Block(), [&] { b.Return(fn, 42_h); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_vec3f_Composite) {
    auto* fn = b.Function("Function", ty.vec3<f32>());
    b.Append(fn->Block(), [&] { b.Return(fn, b.Composite<vec3<f32>>(1_f, 2_f, 3_f)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_vec3f_Splat) {
    auto* fn = b.Function("Function", ty.vec3<f32>());
    b.Append(fn->Block(), [&] { b.Return(fn, b.Splat<vec3<f32>>(1_f, 3)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_mat2x3f_Composite) {
    auto* fn = b.Function("Function", ty.mat2x3<f32>());
    b.Append(fn->Block(),
             [&] { b.Return(fn, b.Composite<mat2x3<f32>>(1_f, 2_f, 3_f, 4_f, 5_f, 6_f)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_mat2x3f_Splat) {
    auto* fn = b.Function("Function", ty.mat2x3<f32>());
    b.Append(fn->Block(), [&] { b.Return(fn, b.Splat<mat2x3<f32>>(1_f, 6)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_array_f32_Composite) {
    auto* fn = b.Function("Function", ty.array<f32, 3>());
    b.Append(fn->Block(), [&] { b.Return(fn, b.Composite<array<f32, 3>>(1_i, 2_i, 3_i)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Return_array_f32_Splat) {
    auto* fn = b.Function("Function", ty.array<f32, 3>());
    b.Append(fn->Block(), [&] { b.Return(fn, b.Splat<array<f32, 3>>(1_i, 3)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Construct) {
    auto* fn = b.Function("Function", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Construct<vec3<f32>>(1_f, 2_f, 3_f);
        b.Return(fn);
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Discard) {
    auto* fn = b.Function("Function", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Discard();
        b.Return(fn);
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Let) {
    auto* fn = b.Function("Function", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Let("Let", b.Constant(42_i));
        b.Return(fn);
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Var) {
    auto* fn = b.Function("Function", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Var<function>("Var", b.Constant(42_i));
        b.Return(fn);
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Access) {
    auto* fn = b.Function("Function", ty.f32());
    b.Append(fn->Block(),
             [&] { b.Return(fn, b.Access<f32>(b.Construct<mat4x4<f32>>(), 1_u, 2_u)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, UserCall) {
    auto* fn_a = b.Function("A", ty.f32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 42_f); });
    auto* fn_b = b.Function("B", ty.f32());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, b.Call(fn_a)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Load) {
    auto p = b.FunctionParam<ptr<function, f32, read_write>>("p");
    auto* fn = b.Function("Function", ty.f32());
    fn->SetParams({p});
    b.Append(fn->Block(), [&] { b.Return(fn, b.Load(p)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Store) {
    auto p = b.FunctionParam<ptr<function, f32, read_write>>("p");
    auto* fn = b.Function("Function", ty.void_());
    fn->SetParams({p});
    b.Append(fn->Block(), [&] {
        b.Store(p, 42_f);
        b.Return(fn);
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, LoadVectorElement) {
    auto p = b.FunctionParam<ptr<function, vec3<f32>, read_write>>("p");
    auto* fn = b.Function("Function", ty.f32());
    fn->SetParams({p});
    b.Append(fn->Block(), [&] { b.Return(fn, b.LoadVectorElement(p, 1_i)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, StoreVectorElement) {
    auto p = b.FunctionParam<ptr<function, vec3<f32>, read_write>>("p");
    auto* fn = b.Function("Function", ty.void_());
    fn->SetParams({p});
    b.Append(fn->Block(), [&] {
        b.StoreVectorElement(p, 1_u, 42_f);
        b.Return(fn);
    });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, UnaryOp) {
    auto x = b.FunctionParam<bool>("x");
    auto* fn = b.Function("Function", ty.bool_());
    fn->SetParams({x});
    b.Append(fn->Block(), [&] { b.Return(fn, b.Not<bool>(x)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, BinaryOp) {
    auto x = b.FunctionParam<f32>("x");
    auto y = b.FunctionParam<f32>("y");
    auto* fn = b.Function("Function", ty.f32());
    fn->SetParams({x, y});
    b.Append(fn->Block(), [&] { b.Return(fn, b.Add<f32>(x, y)); });
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Swizzle) {
    auto x = b.FunctionParam<vec4<f32>>("x");
    auto* fn = b.Function("Function", ty.vec3<f32>());
    fn->SetParams({x});
    b.Append(fn->Block(), [&] {
        b.Return(fn, b.Swizzle<vec3<f32>>(x, Vector<uint32_t, 3>{1, 0, 2}));
    });
    RUN_TEST();
}

}  // namespace
}  // namespace tint::core::ir::binary
