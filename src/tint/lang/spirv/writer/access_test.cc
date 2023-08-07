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

#include "src/tint/lang/spirv/writer/common/helper_test.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::core::fluent_types;  // NOLINT
using namespace tint::number_suffixes;     // NOLINT

TEST_F(SpirvWriterTest, Access_Array_Value_ConstantIndex) {
    auto* arr_val = b.FunctionParam("arr", ty.array(ty.i32(), 4));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({arr_val});
    b.Append(func->Block(), [&] {
        auto* result = b.Access(ty.i32(), arr_val, 1_u);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpCompositeExtract %int %arr 1");
}

TEST_F(SpirvWriterTest, Access_Array_Pointer_ConstantIndex) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* arr_var = b.Var("arr", ty.ptr<function, array<i32, 4>>());
        auto* result = b.Access(ty.ptr<function, i32>(), arr_var, 1_u);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAccessChain %_ptr_Function_int %arr %uint_1");
}

TEST_F(SpirvWriterTest, Access_Array_Pointer_DynamicIndex) {
    auto* idx = b.FunctionParam("idx", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* arr_var = b.Var("arr", ty.ptr<function, array<i32, 4>>());
        auto* result = b.Access(ty.ptr<function, i32>(), arr_var, idx);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAccessChain %_ptr_Function_int %arr %idx");
}

TEST_F(SpirvWriterTest, Access_Matrix_Value_ConstantIndex) {
    auto* mat_val = b.FunctionParam("mat", ty.mat2x2(ty.f32()));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({mat_val});
    b.Append(func->Block(), [&] {
        auto* result_vector = b.Access(ty.vec2(ty.f32()), mat_val, 1_u);
        auto* result_scalar = b.Access(ty.f32(), mat_val, 1_u, 0_u);
        b.Return(func);
        mod.SetName(result_vector, "result_vector");
        mod.SetName(result_scalar, "result_scalar");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result_vector = OpCompositeExtract %v2float %mat 1");
    EXPECT_INST("%result_scalar = OpCompositeExtract %float %mat 1 0");
}

TEST_F(SpirvWriterTest, Access_Matrix_Pointer_ConstantIndex) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* mat_var = b.Var("mat", ty.ptr<function, mat2x2<f32>>());
        auto* result_vector = b.Access(ty.ptr<function, vec2<f32>>(), mat_var, 1_u);
        auto* result_scalar = b.LoadVectorElement(result_vector, 0_u);
        b.Return(func);
        mod.SetName(result_vector, "result_vector");
        mod.SetName(result_scalar, "result_scalar");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result_vector = OpAccessChain %_ptr_Function_v2float %mat %uint_1");
    EXPECT_INST("%14 = OpAccessChain %_ptr_Function_float %result_vector %uint_0");
    EXPECT_INST("%result_scalar = OpLoad %float %14");
}

TEST_F(SpirvWriterTest, Access_Matrix_Pointer_DynamicIndex) {
    auto* idx = b.FunctionParam("idx", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* mat_var = b.Var("mat", ty.ptr<function, mat2x2<f32>>());
        auto* result_vector = b.Access(ty.ptr<function, vec2<f32>>(), mat_var, idx);
        auto* result_scalar = b.LoadVectorElement(result_vector, idx);
        b.Return(func);
        mod.SetName(result_vector, "result_vector");
        mod.SetName(result_scalar, "result_scalar");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result_vector = OpAccessChain %_ptr_Function_v2float %mat %idx");
    EXPECT_INST("%14 = OpAccessChain %_ptr_Function_float %result_vector %idx");
    EXPECT_INST("%result_scalar = OpLoad %float %14");
}

TEST_F(SpirvWriterTest, Access_Vector_Value_ConstantIndex) {
    auto* vec_val = b.FunctionParam("vec", ty.vec4(ty.i32()));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({vec_val});
    b.Append(func->Block(), [&] {
        auto* result = b.Access(ty.i32(), vec_val, 1_u);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpCompositeExtract %int %vec 1");
}

TEST_F(SpirvWriterTest, Access_Vector_Value_DynamicIndex) {
    auto* vec_val = b.FunctionParam("vec", ty.vec4(ty.i32()));
    auto* idx = b.FunctionParam("idx", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({vec_val, idx});
    b.Append(func->Block(), [&] {
        auto* result = b.Access(ty.i32(), vec_val, idx);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpVectorExtractDynamic %int %vec %idx");
}

TEST_F(SpirvWriterTest, Access_NestedVector_Value_DynamicIndex) {
    auto* val = b.FunctionParam("arr", ty.array(ty.array(ty.vec4(ty.i32()), 4), 4));
    auto* idx = b.FunctionParam("idx", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({val, idx});
    b.Append(func->Block(), [&] {
        auto* result = b.Access(ty.i32(), val, 1_u, 2_u, idx);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%14 = OpCompositeExtract %v4int %arr 1 2");
    EXPECT_INST("%result = OpVectorExtractDynamic %int %14 %idx");
}

TEST_F(SpirvWriterTest, Access_Struct_Value_ConstantIndex) {
    auto* str =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.Register("a"), ty.f32()},
                                                   {mod.symbols.Register("b"), ty.vec4<i32>()},
                                               });
    auto* str_val = b.FunctionParam("str", str);
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({str_val});
    b.Append(func->Block(), [&] {
        auto* result_a = b.Access(ty.f32(), str_val, 0_u);
        auto* result_b = b.Access(ty.i32(), str_val, 1_u, 2_u);
        b.Return(func);
        mod.SetName(result_a, "result_a");
        mod.SetName(result_b, "result_b");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result_a = OpCompositeExtract %float %str 0");
    EXPECT_INST("%result_b = OpCompositeExtract %int %str 1 2");
}

TEST_F(SpirvWriterTest, Access_Struct_Pointer_ConstantIndex) {
    auto* str =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.Register("a"), ty.f32()},
                                                   {mod.symbols.Register("b"), ty.vec4<i32>()},
                                               });
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* str_var = b.Var("str", ty.ptr(function, str, read_write));
        auto* result_a = b.Access(ty.ptr<function, f32>(), str_var, 0_u);
        auto* result_b = b.Access(ty.ptr<function, vec4<i32>>(), str_var, 1_u);
        b.Return(func);
        mod.SetName(result_a, "result_a");
        mod.SetName(result_b, "result_b");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result_a = OpAccessChain %_ptr_Function_float %str %uint_0");
    EXPECT_INST("%result_b = OpAccessChain %_ptr_Function_v4int %str %uint_1");
}

TEST_F(SpirvWriterTest, LoadVectorElement_ConstantIndex) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* vec_var = b.Var("vec", ty.ptr<function, vec4<i32>>());
        auto* result = b.LoadVectorElement(vec_var, 1_u);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%9 = OpAccessChain %_ptr_Function_int %vec %uint_1");
    EXPECT_INST("%result = OpLoad %int %9");
}

TEST_F(SpirvWriterTest, LoadVectorElement_DynamicIndex) {
    auto* idx = b.FunctionParam("idx", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* vec_var = b.Var("vec", ty.ptr<function, vec4<i32>>());
        auto* result = b.LoadVectorElement(vec_var, idx);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%10 = OpAccessChain %_ptr_Function_int %vec %idx");
    EXPECT_INST("%result = OpLoad %int %10");
}

TEST_F(SpirvWriterTest, StoreVectorElement_ConstantIndex) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* vec_var = b.Var("vec", ty.ptr<function, vec4<i32>>());
        b.StoreVectorElement(vec_var, 1_u, b.Constant(42_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%9 = OpAccessChain %_ptr_Function_int %vec %uint_1");
    EXPECT_INST("OpStore %9 %int_42");
}

TEST_F(SpirvWriterTest, StoreVectorElement_DynamicIndex) {
    auto* idx = b.FunctionParam("idx", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* vec_var = b.Var("vec", ty.ptr<function, vec4<i32>>());
        b.StoreVectorElement(vec_var, idx, b.Constant(42_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%10 = OpAccessChain %_ptr_Function_int %vec %idx");
    EXPECT_INST("OpStore %10 %int_42");
}

}  // namespace
}  // namespace tint::spirv::writer
