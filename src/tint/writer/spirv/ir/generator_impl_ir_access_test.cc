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

#include "src/tint/writer/spirv/ir/test_helper_ir.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

class SpvGeneratorImplTest_Access : public SpvGeneratorImplTest {
  protected:
    const type::Type* ptr(const type::Type* elem) {
        return ty.pointer(elem, builtin::AddressSpace::kFunction, builtin::Access::kReadWrite);
    }
};

TEST_F(SpvGeneratorImplTest_Access, Array_Value_ConstantIndex) {
    auto* arr_val = b.FunctionParam(ty.array(ty.i32(), 4));
    auto* access = b.Access(ty.i32(), arr_val, utils::Vector{b.Constant(1_u)});
    auto* func = b.CreateFunction("foo", ty.void_());
    func->SetParams(utils::Vector{arr_val});
    func->StartTarget()->SetInstructions(utils::Vector{access, b.Return(func)});

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
OpDecorate %3 ArrayStride 4
%2 = OpTypeVoid
%4 = OpTypeInt 32 1
%6 = OpTypeInt 32 0
%5 = OpConstant %6 4
%3 = OpTypeArray %4 %5
%8 = OpTypeFunction %2 %3
%1 = OpFunction %2 None %8
%7 = OpFunctionParameter %3
%9 = OpLabel
%10 = OpCompositeExtract %4 %7 1
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Array_Pointer_ConstantIndex) {
    auto* arr_var = b.Declare(ptr(ty.array(ty.i32(), 4)));
    auto* access = b.Access(ptr(ty.i32()), arr_var, utils::Vector{b.Constant(1_u)});
    auto* func = b.CreateFunction("foo", ty.void_());
    func->StartTarget()->SetInstructions(utils::Vector{arr_var, access, b.Return(func)});

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
OpDecorate %7 ArrayStride 4
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%8 = OpTypeInt 32 1
%10 = OpTypeInt 32 0
%9 = OpConstant %10 4
%7 = OpTypeArray %8 %9
%6 = OpTypePointer Function %7
%12 = OpTypePointer Function %8
%13 = OpConstant %10 1
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpVariable %6 Function
%11 = OpAccessChain %12 %5 %13
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Array_Pointer_DynamicIndex) {
    auto* arr_var = b.Declare(ptr(ty.array(ty.i32(), 4)));
    auto* idx_var = b.Declare(ptr(ty.i32()));
    auto* idx = b.Load(idx_var);
    auto* access = b.Access(ptr(ty.i32()), arr_var, utils::Vector{idx});
    auto* func = b.CreateFunction("foo", ty.void_());
    func->StartTarget()->SetInstructions(
        utils::Vector{idx_var, idx, arr_var, access, b.Return(func)});

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
OpDecorate %11 ArrayStride 4
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypePointer Function %7
%13 = OpTypeInt 32 0
%12 = OpConstant %13 4
%11 = OpTypeArray %7 %12
%10 = OpTypePointer Function %11
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpVariable %6 Function
%9 = OpVariable %10 Function
%8 = OpLoad %7 %5
%14 = OpAccessChain %6 %9 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Matrix_Value_ConstantIndex) {
    auto* mat_val = b.FunctionParam(ty.mat2x2(ty.f32()));
    auto* access_vec = b.Access(ty.vec2(ty.f32()), mat_val, utils::Vector{b.Constant(1_u)});
    auto* access_el = b.Access(ty.f32(), mat_val, utils::Vector{b.Constant(1_u), b.Constant(0_u)});
    auto* func = b.CreateFunction("foo", ty.void_());
    func->SetParams(utils::Vector{mat_val});
    func->StartTarget()->SetInstructions(utils::Vector{access_vec, access_el, b.Return(func)});

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%5 = OpTypeFloat 32
%4 = OpTypeVector %5 2
%3 = OpTypeMatrix %4 2
%7 = OpTypeFunction %2 %3
%1 = OpFunction %2 None %7
%6 = OpFunctionParameter %3
%8 = OpLabel
%9 = OpCompositeExtract %4 %6 1
%10 = OpCompositeExtract %5 %6 1 0
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Matrix_Pointer_ConstantIndex) {
    auto* mat_var = b.Declare(ptr(ty.mat2x2(ty.f32())));
    auto* access_vec = b.Access(ptr(ty.vec2(ty.f32())), mat_var, utils::Vector{b.Constant(1_u)});
    auto* access_el =
        b.Access(ptr(ty.f32()), mat_var, utils::Vector{b.Constant(1_u), b.Constant(0_u)});
    auto* func = b.CreateFunction("foo", ty.void_());
    func->StartTarget()->SetInstructions(utils::Vector{access_vec, access_el, b.Return(func)});

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 2
%6 = OpTypePointer Function %7
%11 = OpTypeInt 32 0
%10 = OpConstant %11 1
%13 = OpTypePointer Function %8
%14 = OpConstant %11 0
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpAccessChain %6 %9 %10
%12 = OpAccessChain %13 %9 %10 %14
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Matrix_Pointer_DynamicIndex) {
    auto* mat_var = b.Declare(ptr(ty.mat2x2(ty.f32())));
    auto* idx_var = b.Declare(ptr(ty.i32()));
    auto* idx = b.Load(idx_var);
    auto* access_vec = b.Access(ptr(ty.vec2(ty.f32())), mat_var, utils::Vector{idx});
    auto* access_el = b.Access(ptr(ty.f32()), mat_var, utils::Vector{idx, idx});
    auto* func = b.CreateFunction("foo", ty.void_());
    func->StartTarget()->SetInstructions(
        utils::Vector{idx_var, idx, mat_var, access_vec, access_el, b.Return(func)});

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypePointer Function %7
%13 = OpTypeFloat 32
%12 = OpTypeVector %13 2
%11 = OpTypeMatrix %12 2
%10 = OpTypePointer Function %11
%15 = OpTypePointer Function %12
%17 = OpTypePointer Function %13
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpVariable %6 Function
%9 = OpVariable %10 Function
%8 = OpLoad %7 %5
%14 = OpAccessChain %15 %9 %8
%16 = OpAccessChain %17 %9 %8 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Vector_Value_ConstantIndex) {
    auto* vec_val = b.FunctionParam(ty.vec4(ty.i32()));
    auto* access = b.Access(ty.i32(), vec_val, utils::Vector{b.Constant(1_u)});
    auto* func = b.CreateFunction("foo", ty.void_());
    func->SetParams(utils::Vector{vec_val});
    func->StartTarget()->SetInstructions(utils::Vector{access, b.Return(func)});

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%4 = OpTypeInt 32 1
%3 = OpTypeVector %4 4
%6 = OpTypeFunction %2 %3
%1 = OpFunction %2 None %6
%5 = OpFunctionParameter %3
%7 = OpLabel
%8 = OpCompositeExtract %4 %5 1
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Vector_Value_DynamicIndex) {
    auto* vec_val = b.FunctionParam(ty.vec4(ty.i32()));
    auto* idx_var = b.Declare(ptr(ty.i32()));
    auto* idx = b.Load(idx_var);
    auto* access = b.Access(ty.i32(), vec_val, utils::Vector{idx});
    auto* func = b.CreateFunction("foo", ty.void_());
    func->SetParams(utils::Vector{vec_val});
    func->StartTarget()->SetInstructions(utils::Vector{idx_var, idx, access, b.Return(func)});

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%4 = OpTypeInt 32 1
%3 = OpTypeVector %4 4
%6 = OpTypeFunction %2 %3
%9 = OpTypePointer Function %4
%1 = OpFunction %2 None %6
%5 = OpFunctionParameter %3
%7 = OpLabel
%8 = OpVariable %9 Function
%10 = OpLoad %4 %8
%11 = OpVectorExtractDynamic %4 %5 %10
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Vector_Pointer_ConstantIndex) {
    auto* vec_var = b.Declare(ptr(ty.vec4(ty.i32())));
    auto* access = b.Access(ptr(ty.i32()), vec_var, utils::Vector{b.Constant(1_u)});
    auto* func = b.CreateFunction("foo", ty.void_());
    func->StartTarget()->SetInstructions(utils::Vector{vec_var, access, b.Return(func)});

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%8 = OpTypeInt 32 1
%7 = OpTypeVector %8 4
%6 = OpTypePointer Function %7
%10 = OpTypePointer Function %8
%12 = OpTypeInt 32 0
%11 = OpConstant %12 1
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpVariable %6 Function
%9 = OpAccessChain %10 %5 %11
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Vector_Pointer_DynamicIndex) {
    auto* vec_var = b.Declare(ptr(ty.vec4(ty.i32())));
    auto* idx_var = b.Declare(ptr(ty.i32()));
    auto* idx = b.Load(idx_var);
    auto* access = b.Access(ptr(ty.i32()), vec_var, utils::Vector{idx});
    auto* func = b.CreateFunction("foo", ty.void_());
    func->StartTarget()->SetInstructions(
        utils::Vector{idx_var, idx, vec_var, access, b.Return(func)});

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypePointer Function %7
%11 = OpTypeVector %7 4
%10 = OpTypePointer Function %11
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpVariable %6 Function
%9 = OpVariable %10 Function
%8 = OpLoad %7 %5
%12 = OpAccessChain %6 %9 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, NestedVector_Value_DynamicIndex) {
    auto* val = b.FunctionParam(ty.array(ty.array(ty.vec4(ty.i32()), 4), 4));
    auto* idx_var = b.Declare(ptr(ty.i32()));
    auto* idx = b.Load(idx_var);
    auto* access = b.Access(ty.i32(), val, utils::Vector{b.Constant(1_u), b.Constant(2_u), idx});
    auto* func = b.CreateFunction("foo", ty.void_());
    func->SetParams(utils::Vector{val});
    func->StartTarget()->SetInstructions(utils::Vector{idx_var, idx, access, b.Return(func)});

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
OpDecorate %4 ArrayStride 16
OpDecorate %3 ArrayStride 64
%2 = OpTypeVoid
%6 = OpTypeInt 32 1
%5 = OpTypeVector %6 4
%8 = OpTypeInt 32 0
%7 = OpConstant %8 4
%4 = OpTypeArray %5 %7
%3 = OpTypeArray %4 %7
%10 = OpTypeFunction %2 %3
%13 = OpTypePointer Function %6
%1 = OpFunction %2 None %10
%9 = OpFunctionParameter %3
%11 = OpLabel
%12 = OpVariable %13 Function
%14 = OpLoad %6 %12
%16 = OpCompositeExtract %5 %9 1 2
%15 = OpVectorExtractDynamic %6 %16 %14
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Struct_Value_ConstantIndex) {
    auto* str = ty.Get<type::Struct>(
        mod.symbols.Register("MyStruct"),
        utils::Vector{
            ty.Get<type::StructMember>(mod.symbols.Register("a"), ty.f32(), 0u, 0u, 4u, 4u,
                                       type::StructMemberAttributes{}),
            ty.Get<type::StructMember>(mod.symbols.Register("b"), ty.vec4(ty.i32()), 1u, 16u, 16u,
                                       16u, type::StructMemberAttributes{}),
        },
        16u, 32u, 32u);
    auto* str_val = b.FunctionParam(str);
    auto* access_vec = b.Access(ty.i32(), str_val, utils::Vector{b.Constant(1_u)});
    auto* access_el = b.Access(ty.i32(), str_val, utils::Vector{b.Constant(1_u), b.Constant(2_u)});
    auto* func = b.CreateFunction("foo", ty.void_());
    func->SetParams(utils::Vector{str_val});
    func->StartTarget()->SetInstructions(utils::Vector{access_vec, access_el, b.Return(func)});

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
OpMemberName %3 0 "a"
OpMemberName %3 1 "b"
OpName %3 "MyStruct"
OpMemberDecorate %3 0 Offset 0
OpMemberDecorate %3 1 Offset 16
%2 = OpTypeVoid
%4 = OpTypeFloat 32
%6 = OpTypeInt 32 1
%5 = OpTypeVector %6 4
%3 = OpTypeStruct %4 %5
%8 = OpTypeFunction %2 %3
%1 = OpFunction %2 None %8
%7 = OpFunctionParameter %3
%9 = OpLabel
%10 = OpCompositeExtract %6 %7 1
%11 = OpCompositeExtract %6 %7 1 2
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Struct_Pointer_ConstantIndex) {
    auto* str = ty.Get<type::Struct>(
        mod.symbols.Register("MyStruct"),
        utils::Vector{
            ty.Get<type::StructMember>(mod.symbols.Register("a"), ty.f32(), 0u, 0u, 4u, 4u,
                                       type::StructMemberAttributes{}),
            ty.Get<type::StructMember>(mod.symbols.Register("b"), ty.vec4(ty.i32()), 1u, 16u, 16u,
                                       16u, type::StructMemberAttributes{}),
        },
        16u, 32u, 32u);
    auto* str_var = b.Declare(ptr(str));
    auto* access_vec = b.Access(ptr(ty.i32()), str_var, utils::Vector{b.Constant(1_u)});
    auto* access_el =
        b.Access(ptr(ty.i32()), str_var, utils::Vector{b.Constant(1_u), b.Constant(2_u)});
    auto* func = b.CreateFunction("foo", ty.void_());
    func->StartTarget()->SetInstructions(
        utils::Vector{str_var, access_vec, access_el, b.Return(func)});

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
OpMemberName %7 0 "a"
OpMemberName %7 1 "b"
OpName %7 "MyStruct"
OpMemberDecorate %7 0 Offset 0
OpMemberDecorate %7 1 Offset 16
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%8 = OpTypeFloat 32
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 4
%7 = OpTypeStruct %8 %9
%6 = OpTypePointer Function %7
%12 = OpTypePointer Function %10
%14 = OpTypeInt 32 0
%13 = OpConstant %14 1
%16 = OpConstant %14 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpVariable %6 Function
%11 = OpAccessChain %12 %5 %13
%15 = OpAccessChain %12 %5 %13 %16
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
