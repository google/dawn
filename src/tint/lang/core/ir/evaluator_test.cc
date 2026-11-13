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

#include "gmock/gmock.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/type/u32.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::core::ir::eval::compile_time {
namespace {

class IR_EvaluatorTest : public ir::IRTestHelper {
  protected:
    ir::Value* NonConstI32() {
        auto* dpdx = b.Call(ty.f32(), core::BuiltinFn::kDpdx, 2.0_f);
        return b.Bitcast(ty.i32(), dpdx);
    }

    ir::Value* FailingDivByZeroI32() {
        auto* zero = b.Override("zero", 0_i);
        return b.Divide(1_i, zero);
    }

    ir::InstructionResult* RawAdd(ir::Value* lhs, ir::Value* rhs) {
        return b
            .Append(mod.CreateInstruction<core::ir::CoreBinary>(b.InstructionResult(ty.i32()),
                                                                BinaryOp::kAdd, lhs, rhs))
            ->Result();
    }

    ir::InstructionResult* RawAddChain(uint32_t depth) {
        auto* cur = RawAdd(b.Constant(1_i), b.Constant(1_i));
        for (uint32_t i = 1; i < depth; ++i) {
            cur = RawAdd(cur, b.Constant(1_i));
        }
        return cur;
    }

    ir::InstructionResult* RawBalancedAddTree7() {
        auto* l0 = RawAdd(b.Constant(1_i), b.Constant(1_i));
        auto* l1 = RawAdd(b.Constant(1_i), b.Constant(1_i));
        auto* l2 = RawAdd(b.Constant(1_i), b.Constant(1_i));
        auto* l3 = RawAdd(b.Constant(1_i), b.Constant(1_i));
        auto* m0 = RawAdd(l0, l1);
        auto* m1 = RawAdd(l2, l3);
        return RawAdd(m0, m1);
    }
};

TEST_F(IR_EvaluatorTest, InvalidExpression) {
    auto* inst = b.Negation(4_u);
    b.ir.SetSource(inst, Source{{2, 3}});
    auto res = Eval(b, inst);
    ASSERT_NE(res, Success);

    EXPECT_EQ(res.Failure().reason.Str(), R"(2:3 error: no matching overload for 'operator - (u32)'

2 candidate operators:
 • 'operator - (T  ✗ ) -> T' where:
      ✗  'T' is 'f32', 'i32' or 'f16'
 • 'operator - (vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32' or 'f16'
)");
}

TEST_F(IR_EvaluatorTest, Bitcast) {
    auto* inst = b.Bitcast(mod.Types().i32(), 4_u);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_TRUE(c->Type()->Is<core::type::I32>());
    EXPECT_EQ(4, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, Unary) {
    auto* inst = b.Complement(4_i);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(~4, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, Binary) {
    auto* inst = b.Add(1_i, 2_i);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(3, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, ConstructScalar) {
    auto* inst = b.Construct(ty.i32(), 1_i);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(1, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, ConstructScalarEmpty) {
    auto* inst = b.Construct(ty.i32());
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(0, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, ConstructArray_Access) {
    auto* obj = b.Construct(ty.array<i32, 3>(), 1_i, 2_i, 3_i);
    auto* inst = b.Access(mod.Types().i32(), obj, 1_u);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(2, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, NestedStruct_AccessSuccess) {
    auto* S = ty.Struct(mod.symbols.New("S"), {{mod.symbols.New("data"), ty.array<u32, 4>()}});

    auto* arr = b.Var("struct", ty.ptr(storage, S));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 0_i, 3_i);
    auto res = Eval(b, inst);

    ASSERT_EQ(res, Success);
}

TEST_F(IR_EvaluatorTest, NestedStruct_AccessFailArray) {
    auto* S = ty.Struct(mod.symbols.New("S"), {{mod.symbols.New("data"), ty.array<u32, 4>()}});

    auto* arr = b.Var("struct", ty.ptr(storage, S));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 0_i, 4_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index 4 out of bounds [0..3])");
}

TEST_F(IR_EvaluatorTest, NestedStruct_AccessFailMember) {
    auto* S = ty.Struct(mod.symbols.New("S"), {{mod.symbols.New("data"), ty.array<u32, 4>()}});

    auto* arr = b.Var("struct", ty.ptr(storage, S));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 1_i, 3_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index 1 out of bounds [0..0])");
}

TEST_F(IR_EvaluatorTest, ArrayBounds_OutOfBoundsAccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<u32>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, -1_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);

    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index -1 out of bounds)");
}

TEST_F(IR_EvaluatorTest, ArrayBounds_OverflowBoundsAccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<u32, 3>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 3_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);

    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index 3 out of bounds [0..2])");
}

TEST_F(IR_EvaluatorTest, ArrayBounds_NestedOverflowBoundsAccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<array<u32, 3>, 5>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 3_i, 3_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);

    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index 3 out of bounds [0..2])");
}

TEST_F(IR_EvaluatorTest, ArrayBounds_NestedoundsAccessSuccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<array<u32, 3>, 5>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 4_i, 2_i);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_EvaluatorTest, ArrayBounds_DoubleNestedOverflowBoundsAccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<array<array<u32, 3>, 5>, 7>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 3_i, 3_i, 3_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);

    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index 3 out of bounds [0..2])");
}

TEST_F(IR_EvaluatorTest, ArrayBounds_DoubleNestedBoundsAccessSuccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<array<array<u32, 3>, 5>, 7>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 6_i, 4_i, 2_i);
    auto res = Eval(b, inst);

    ASSERT_EQ(res, Success);
}

TEST_F(IR_EvaluatorTest, ArrayBounds_DoubleNestedEarlyOverflowBoundsAccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<array<array<u32, 3>, 5>, 7>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 7_i, 3_i, 3_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);

    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index 7 out of bounds [0..6])");
}

TEST_F(IR_EvaluatorTest, ArrayBounds_DoubleNestedMidOverflowBoundsAccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<array<array<u32, 3>, 5>, 7>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 3_i, 5_i, 3_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);

    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index 5 out of bounds [0..4])");
}

TEST_F(IR_EvaluatorTest, ArrayBounds_NestedDynamicAndConstantInBounds) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<array<array<u32, 3>, 5>, 7>()));
    auto* x = b.Var("x", 123_i);
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 6_i, x, 2_i);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_EvaluatorTest, ArrayBounds_NestedDynamicAndConstantOutOfBoundsAccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<array<array<u32, 3>, 5>, 7>()));
    auto* x = b.Var("x", 123_i);
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 6_i, x, 3_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);

    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index 3 out of bounds [0..2])");
}

TEST_F(IR_EvaluatorTest, ArrayBounds_NestedVecOverflowBoundsAccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<vec3u, 7>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 3_i, 3_i);
    auto res = Eval(b, inst);

    ASSERT_NE(res, Success);

    EXPECT_EQ(res.Failure().reason.Str(), R"(error: index 3 out of bounds [0..2])");
}

TEST_F(IR_EvaluatorTest, ArrayBounds_NestedVecBoundsAccessSuccess) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<vec3u, 7>()));
    auto* inst = b.Access(ty.ptr<storage, u32>(), arr, 3_i, 2_i);
    auto res = Eval(b, inst);

    ASSERT_EQ(res, Success);
}

TEST_F(IR_EvaluatorTest, ConstructStruct_Access) {
    auto* S = ty.Struct(mod.symbols.New("S"),
                        {{mod.symbols.New("a"), ty.i32()}, {mod.symbols.New("b"), ty.f32()}});

    auto* obj = b.Construct(S, 1_i, 2_f);
    auto* inst = b.Access(mod.Types().i32(), obj, 1_u);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_FLOAT_EQ(2, c->Value()->ValueAs<float>());
}

TEST_F(IR_EvaluatorTest, ConstructVector_Swizzle) {
    auto* obj = b.Construct(ty.vec3i(), 1_i, 2_i, 3_i);
    auto* inst = b.Swizzle(mod.Types().i32(), obj, {1});
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(2, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, ConstructVector_Access) {
    auto* obj = b.Construct(ty.vec3i(), 1_i, 2_i, 3_i);
    auto* inst = b.Access(mod.Types().i32(), obj, 1_u);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success);

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(2, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, Convert) {
    auto* obj = b.Construct(ty.i32(), 1_i);
    auto* inst = b.Convert(ty.u32(), obj);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    ASSERT_EQ(ty.u32(), c->Type());
    EXPECT_EQ(1u, c->Value()->ValueAs<uint32_t>());
}

TEST_F(IR_EvaluatorTest, ConstExprIfSimple) {
    auto* constexpr_if = b.ConstExprIf(true);
    constexpr_if->SetResult(b.InstructionResult(ty.bool_()));
    b.Append(constexpr_if->True(), [&] { b.ExitIf(constexpr_if, true); });
    b.Append(constexpr_if->False(), [&] { b.ExitIf(constexpr_if, false); });
    auto res = Eval(b, constexpr_if);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    ASSERT_EQ(ty.bool_(), c->Type());
    EXPECT_EQ(true, c->Value()->ValueAs<bool>());
}

TEST_F(IR_EvaluatorTest, ConstExprIfalse) {
    auto* constexpr_if = b.ConstExprIf(true);
    constexpr_if->SetResult(b.InstructionResult(ty.bool_()));
    b.Append(constexpr_if->True(), [&] { b.ExitIf(constexpr_if, false); });
    b.Append(constexpr_if->False(), [&] { b.ExitIf(constexpr_if, true); });
    auto res = Eval(b, constexpr_if);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    ASSERT_EQ(ty.bool_(), c->Type());
    EXPECT_EQ(false, c->Value()->ValueAs<bool>());
}

TEST_F(IR_EvaluatorTest, BuiltinCall) {
    auto* inst = b.Call(ty.i32(), core::BuiltinFn::kAbs, -1_i);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(1, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, MultiExpression) {
    auto* abs = b.Call(ty.i32(), core::BuiltinFn::kAbs, -1_i);
    auto* mul = b.Multiply(abs, 5_i);
    auto* cons = b.Construct(ty.vec2i(), mul, mul);
    auto* inst = b.Swizzle(ty.i32(), cons, {1});

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(5, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, NonConstBuiltinCall) {
    auto* inst = b.Call(ty.f32(), core::BuiltinFn::kDpdx, 2.0_f);

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstBuiltinCallArg) {
    auto* dpdx = b.Call(ty.f32(), core::BuiltinFn::kDpdx, 2.0_f);
    auto* inst = b.Call(ty.f32(), core::BuiltinFn::kAbs, dpdx);

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstCallInsideUnary) {
    auto* dpdx = b.Call(ty.f32(), core::BuiltinFn::kDpdx, 2.0_f);
    auto* inst = b.Negation(dpdx);

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstCallInsideBinaryRHS) {
    auto* dpdx = b.Call(ty.f32(), core::BuiltinFn::kDpdx, 2.0_f);
    auto* inst = b.Add(1.0_f, dpdx);

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstCallInsideBinaryLHS) {
    auto* dpdx = b.Call(ty.f32(), core::BuiltinFn::kDpdx, 2.0_f);
    auto* inst = b.Add(dpdx, 1.0_f);

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstCallInsideSwizzle) {
    auto* dpdx = b.Call(ty.vec2f(), core::BuiltinFn::kDpdx, b.Construct(ty.vec2f(), 2.0_f, 2.0_f));
    auto* inst = b.Swizzle(ty.f32(), dpdx, {1});

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstCallInsideConstruct) {
    auto* dpdx = b.Call(ty.f32(), core::BuiltinFn::kDpdx, 2.0_f);
    auto* inst = b.Construct(ty.vec2f(), dpdx, 2_f);

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstCallInsideConvert) {
    auto* dpdx = b.Call(ty.f32(), core::BuiltinFn::kDpdx, 2.0_f);
    auto* inst = b.Convert(ty.u32(), dpdx);

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstCallInsideBitcast) {
    auto* dpdx = b.Call(ty.f32(), core::BuiltinFn::kDpdx, 2.0_f);
    auto* inst = b.Bitcast(ty.u32(), dpdx);

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstCallInsideAccessObject) {
    auto* dpdx = b.Call(ty.vec2f(), core::BuiltinFn::kDpdx, b.Construct(ty.vec2f(), 2.0_f, 2.0_f));
    auto* inst = b.Access(ty.f32(), dpdx, 0_u);

    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_EQ(val, nullptr);
}

TEST_F(IR_EvaluatorTest, DeepOverrideDAG) {
    auto* o = b.Override("o", 0_i);
    ir::Value* v = o->Result();
    for (int i = 0; i < 512; ++i) {
        v = b.Add(v, v);
    }

    auto res = Eval(b, v);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(0, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, DeepMixedExpressionChain) {
    auto* o = b.Override("o", 1_i);
    ir::Value* v = o->Result();
    for (int i = 0; i < 256; ++i) {
        auto* neg = b.Negation(v);
        auto* abs = b.Call(ty.i32(), core::BuiltinFn::kAbs, neg);
        auto* lhs_u = b.Convert(ty.u32(), abs);
        auto* rhs_u = b.Convert(ty.u32(), o);
        auto* sum = b.Add(lhs_u, rhs_u);
        auto* one_u = b.Divide(sum, 2_u);
        auto* vec = b.Construct(ty.vec2u(), 0_u, one_u);
        auto* swz = b.Swizzle(ty.u32(), vec, {1});
        auto* zero_idx = b.Subtract(one_u, 1_u);
        auto* arr = b.Construct(ty.array<u32, 2>(), swz, 9_u);
        auto* acc = b.Access(ty.u32(), arr, zero_idx);
        auto* cond = b.Equal(acc, 1_u);

        auto* cif = b.ConstExprIf(cond);
        cif->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(cif->True(), [&] {
            auto* inner = b.Equal(acc, 1_u);
            b.ExitIf(cif, inner);
        });
        b.Append(cif->False(), [&] { b.ExitIf(cif, false); });

        v = b.Convert(ty.i32(), cif->Result());
    }

    auto res = Eval(b, v);
    ASSERT_EQ(res, Success) << res.Failure();

    auto* val = res.Get();
    ASSERT_NE(val, nullptr);
    auto* c = val->As<core::ir::Constant>();
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(1, c->Value()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, OverrideDivideByZeroFails) {
    auto* inst = FailingDivByZeroI32();
    auto res = Eval(b, inst);
    ASSERT_NE(res, Success);
}

TEST_F(IR_EvaluatorTest, NonConstShortCircuitsBinaryRHS) {
    auto* non_const = NonConstI32();
    auto* div_zero = FailingDivByZeroI32();
    auto* inst = b.Add(non_const, div_zero);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();
    EXPECT_EQ(res.Get(), nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstShortCircuitsConstructArg) {
    auto* non_const = NonConstI32();
    auto* div_zero = FailingDivByZeroI32();
    auto* inst = b.Construct(ty.vec2i(), non_const, div_zero);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();
    EXPECT_EQ(res.Get(), nullptr);
}

TEST_F(IR_EvaluatorTest, NonConstShortCircuitsBuiltinCallArg) {
    auto* non_const = NonConstI32();
    auto* div_zero = FailingDivByZeroI32();
    auto* inst = b.Call(ty.i32(), core::BuiltinFn::kMax, non_const, div_zero);
    auto res = Eval(b, inst);
    ASSERT_EQ(res, Success) << res.Failure();
    EXPECT_EQ(res.Get(), nullptr);
}

TEST_F(IR_EvaluatorTest, BuilderEvalWithinDepthLimit) {
    auto* d4 = RawAddChain(4);  // 4 nested instructions (kMaxBuilderEvalDepth == 4), evaluates to 5
    auto res = Evaluator{b, false}.EvalCoreBinary(BinaryOp::kAdd, ty.i32(), d4, b.Constant(1_i));
    ASSERT_EQ(res, Success);
    ASSERT_NE(res.Get(), nullptr);
    EXPECT_EQ(6, res.Get()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, BuilderEvalExceedDepthLimit) {
    auto* d5 = RawAddChain(5);  // 5 nested instructions (exceeds kMaxBuilderEvalDepth == 4)
    auto res = Evaluator{b, false}.EvalCoreBinary(BinaryOp::kAdd, ty.i32(), d5, b.Constant(1_i));
    ASSERT_EQ(res, Success);
    EXPECT_EQ(res.Get(), nullptr);
}

TEST_F(IR_EvaluatorTest, BuilderEvalWithinCountLimit) {
    auto* root7 = RawBalancedAddTree7();  // 7 instructions (depth 3), evaluates to 8
    auto* extra8 = RawAdd(b.Constant(1_i), b.Constant(1_i));  // 8th instruction, evaluates to 2
    auto res = Evaluator{b, false}.EvalCoreBinary(BinaryOp::kAdd, ty.i32(), root7, extra8);
    ASSERT_EQ(res, Success);
    ASSERT_NE(res.Get(), nullptr);
    EXPECT_EQ(10, res.Get()->ValueAs<int32_t>());
}

TEST_F(IR_EvaluatorTest, BuilderEvalExceedCountLimit) {
    auto* root7 = RawBalancedAddTree7();                      // 7 instructions (depth 3)
    auto* extra8 = RawAdd(b.Constant(1_i), b.Constant(1_i));  // 8th instruction
    auto* extra9 = RawAdd(b.Constant(1_i), b.Constant(1_i));  // 9th instruction (exceeds 8)
    auto res = Evaluator{b, false}.EvalConstruct(
        ty.vec3i(), Vector<core::ir::Value*, 3>{root7, extra8, extra9});
    ASSERT_EQ(res, Success);
    EXPECT_EQ(res.Get(), nullptr);
}

}  // namespace
}  // namespace tint::core::ir::eval::compile_time
