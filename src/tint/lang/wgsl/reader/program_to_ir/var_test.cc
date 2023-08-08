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

#include "gmock/gmock.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/wgsl/ast/case_selector.h"
#include "src/tint/lang/wgsl/ast/int_literal_expression.h"
#include "src/tint/lang/wgsl/helpers/ir_program_test.h"

namespace tint::wgsl::reader {
namespace {

using namespace tint::core::fluent_types;  // NOLINT
using namespace tint::number_suffixes;     // NOLINT

using ProgramToIRVarTest = helpers::IRProgramTest;

TEST_F(ProgramToIRVarTest, Emit_GlobalVar_NoInit) {
    GlobalVar("a", ty.u32(), core::AddressSpace::kPrivate);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%b1 = block {  # root
  %a:ptr<private, u32, read_write> = var
}

)");
}

TEST_F(ProgramToIRVarTest, Emit_GlobalVar_Init) {
    auto* expr = Expr(2_u);
    GlobalVar("a", ty.u32(), core::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%b1 = block {  # root
  %a:ptr<private, u32, read_write> = var, 2u
}

)");
}

TEST_F(ProgramToIRVarTest, Emit_GlobalVar_GroupBinding) {
    GlobalVar("a", ty.u32(), core::AddressSpace::kStorage, Vector{Group(2_u), Binding(3_u)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%b1 = block {  # root
  %a:ptr<storage, u32, read> = var @binding_point(2, 3)
}

)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_NoInit) {
    auto* a = Var("a", ty.u32(), core::AddressSpace::kFunction);
    WrapInFunction(a);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, u32, read_write> = var
    ret
  }
}
)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_Init_Constant) {
    auto* expr = Expr(2_u);
    auto* a = Var("a", ty.u32(), core::AddressSpace::kFunction, expr);
    WrapInFunction(a);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, u32, read_write> = var, 2u
    ret
  }
}
)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_Init_NonConstant) {
    auto* a = Var("a", ty.u32(), core::AddressSpace::kFunction);
    auto* b = Var("b", ty.u32(), core::AddressSpace::kFunction, Add("a", 2_u));
    WrapInFunction(a, b);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, u32, read_write> = var
    %3:u32 = load %a
    %4:u32 = add %3, 2u
    %b:ptr<function, u32, read_write> = var, %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_Assign_42i) {
    WrapInFunction(Var("a", ty.i32(), core::AddressSpace::kFunction),  //
                   Assign("a", 42_i));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, i32, read_write> = var
    store %a, 42i
    ret
  }
}
)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_Assign_ArrayOfArray_EvalOrder) {
    Func("f", Vector{Param("p", ty.i32())}, ty.i32(), Vector{Return("p")});

    auto* lhs =                                 //
        IndexAccessor(                          //
            IndexAccessor(                      //
                IndexAccessor("a",              //
                              Call("f", 1_i)),  //
                Call("f", 2_i)),                //
            Call("f", 3_i));

    auto* rhs =                                 //
        IndexAccessor(                          //
            IndexAccessor(                      //
                IndexAccessor("a",              //
                              Call("f", 4_i)),  //
                Call("f", 5_i)),                //
            Call("f", 6_i));

    WrapInFunction(
        Var("a", ty.array<array<array<i32, 5>, 5>, 5>(), core::AddressSpace::kFunction),  //
        Assign(lhs, rhs));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%f = func(%p:i32):i32 -> %b1 {
  %b1 = block {
    ret %p
  }
}
%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %a:ptr<function, array<array<array<i32, 5>, 5>, 5>, read_write> = var
    %5:i32 = call %f, 1i
    %6:i32 = call %f, 2i
    %7:i32 = call %f, 3i
    %8:ptr<function, i32, read_write> = access %a, %5, %6, %7
    %9:i32 = call %f, 4i
    %10:i32 = call %f, 5i
    %11:i32 = call %f, 6i
    %12:ptr<function, i32, read_write> = access %a, %9, %10, %11
    %13:i32 = load %12
    store %8, %13
    ret
  }
}
)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_Assign_ArrayOfVec_EvalOrder) {
    Func("f", Vector{Param("p", ty.i32())}, ty.i32(), Vector{Return("p")});

    auto* lhs =                             //
        IndexAccessor(                      //
            IndexAccessor("a",              //
                          Call("f", 1_i)),  //
            Call("f", 2_i));

    auto* rhs =                             //
        IndexAccessor(                      //
            IndexAccessor("a",              //
                          Call("f", 4_i)),  //
            Call("f", 5_i));

    WrapInFunction(Var("a", ty.array<vec4<f32>, 5>(), core::AddressSpace::kFunction),  //
                   Assign(lhs, rhs));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%f = func(%p:i32):i32 -> %b1 {
  %b1 = block {
    ret %p
  }
}
%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %a:ptr<function, array<vec4<f32>, 5>, read_write> = var
    %5:i32 = call %f, 1i
    %6:ptr<function, vec4<f32>, read_write> = access %a, %5
    %7:i32 = call %f, 2i
    %8:i32 = call %f, 4i
    %9:ptr<function, vec4<f32>, read_write> = access %a, %8
    %10:i32 = call %f, 5i
    %11:f32 = load_vector_element %9, %10
    store_vector_element %6, %7, %11
    ret
  }
}
)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_Assign_ArrayOfMatrix_EvalOrder) {
    Func("f", Vector{Param("p", ty.i32())}, ty.i32(), Vector{Return("p")});

    auto* lhs =                                 //
        IndexAccessor(                          //
            IndexAccessor(                      //
                IndexAccessor("a",              //
                              Call("f", 1_i)),  //
                Call("f", 2_i)),                //
            Call("f", 3_i));

    auto* rhs =                                 //
        IndexAccessor(                          //
            IndexAccessor(                      //
                IndexAccessor("a",              //
                              Call("f", 4_i)),  //
                Call("f", 5_i)),                //
            Call("f", 6_i));

    WrapInFunction(Var("a", ty.array<mat3x4<f32>, 5>(), core::AddressSpace::kFunction),  //
                   Assign(lhs, rhs));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%f = func(%p:i32):i32 -> %b1 {
  %b1 = block {
    ret %p
  }
}
%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %a:ptr<function, array<mat3x4<f32>, 5>, read_write> = var
    %5:i32 = call %f, 1i
    %6:i32 = call %f, 2i
    %7:ptr<function, vec4<f32>, read_write> = access %a, %5, %6
    %8:i32 = call %f, 3i
    %9:i32 = call %f, 4i
    %10:i32 = call %f, 5i
    %11:ptr<function, vec4<f32>, read_write> = access %a, %9, %10
    %12:i32 = call %f, 6i
    %13:f32 = load_vector_element %11, %12
    store_vector_element %7, %8, %13
    ret
  }
}
)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_CompoundAssign_42i) {
    WrapInFunction(Var("a", ty.i32(), core::AddressSpace::kFunction),  //
                   CompoundAssign("a", 42_i, core::BinaryOp::kAdd));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, i32, read_write> = var
    %3:i32 = load %a
    %4:i32 = add %3, 42i
    store %a, %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_CompoundAssign_ArrayOfArray_EvalOrder) {
    Func("f", Vector{Param("p", ty.i32())}, ty.i32(), Vector{Return("p")});

    auto* lhs =                                 //
        IndexAccessor(                          //
            IndexAccessor(                      //
                IndexAccessor("a",              //
                              Call("f", 1_i)),  //
                Call("f", 2_i)),                //
            Call("f", 3_i));

    auto* rhs =                                 //
        IndexAccessor(                          //
            IndexAccessor(                      //
                IndexAccessor("a",              //
                              Call("f", 4_i)),  //
                Call("f", 5_i)),                //
            Call("f", 6_i));

    WrapInFunction(
        Var("a", ty.array<array<array<i32, 5>, 5>, 5>(), core::AddressSpace::kFunction),  //
        CompoundAssign(lhs, rhs, core::BinaryOp::kAdd));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%f = func(%p:i32):i32 -> %b1 {
  %b1 = block {
    ret %p
  }
}
%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %a:ptr<function, array<array<array<i32, 5>, 5>, 5>, read_write> = var
    %5:i32 = call %f, 1i
    %6:i32 = call %f, 2i
    %7:i32 = call %f, 3i
    %8:ptr<function, i32, read_write> = access %a, %5, %6, %7
    %9:i32 = call %f, 4i
    %10:i32 = call %f, 5i
    %11:i32 = call %f, 6i
    %12:ptr<function, i32, read_write> = access %a, %9, %10, %11
    %13:i32 = load %12
    %14:i32 = load %8
    %15:i32 = add %14, %13
    store %8, %15
    ret
  }
}
)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_CompoundAssign_ArrayOfMatrix_EvalOrder) {
    Func("f", Vector{Param("p", ty.i32())}, ty.i32(), Vector{Return("p")});

    auto* lhs =                                 //
        IndexAccessor(                          //
            IndexAccessor(                      //
                IndexAccessor("a",              //
                              Call("f", 1_i)),  //
                Call("f", 2_i)),                //
            Call("f", 3_i));

    auto* rhs =                                 //
        IndexAccessor(                          //
            IndexAccessor(                      //
                IndexAccessor("a",              //
                              Call("f", 4_i)),  //
                Call("f", 5_i)),                //
            Call("f", 6_i));

    WrapInFunction(Var("a", ty.array<mat3x4<f32>, 5>(), core::AddressSpace::kFunction),  //
                   CompoundAssign(lhs, rhs, core::BinaryOp::kAdd));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%f = func(%p:i32):i32 -> %b1 {
  %b1 = block {
    ret %p
  }
}
%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %a:ptr<function, array<mat3x4<f32>, 5>, read_write> = var
    %5:i32 = call %f, 1i
    %6:i32 = call %f, 2i
    %7:ptr<function, vec4<f32>, read_write> = access %a, %5, %6
    %8:i32 = call %f, 3i
    %9:i32 = call %f, 4i
    %10:i32 = call %f, 5i
    %11:ptr<function, vec4<f32>, read_write> = access %a, %9, %10
    %12:i32 = call %f, 6i
    %13:f32 = load_vector_element %11, %12
    %14:f32 = load_vector_element %7, %8
    %15:f32 = add %14, %13
    store_vector_element %7, %8, %15
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::wgsl::reader
