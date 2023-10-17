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

#include "src/tint/lang/wgsl/helpers/ir_program_test.h"

#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/wgsl/ast/case_selector.h"
#include "src/tint/lang/wgsl/ast/int_literal_expression.h"

namespace tint::wgsl::reader {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

using ProgramToIRUnaryTest = helpers::IRProgramTest;

TEST_F(ProgramToIRUnaryTest, EmitExpression_Unary_Not) {
    Func("my_func", tint::Empty, ty.bool_(), Vector{Return(false)});
    auto* expr = Not(Call("my_func"));
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()), R"(%my_func = func():bool -> %b1 {
  %b1 = block {
    ret false
  }
}
%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:bool = call %my_func
    %tint_symbol:bool = eq %3, false
    ret
  }
}
)");
}

TEST_F(ProgramToIRUnaryTest, EmitExpression_Unary_Not_Vector) {
    Func("my_func", tint::Empty, ty.vec4<bool>(), Vector{Return(vec(ty.bool_(), 4, false))});
    auto* expr = Not(Call("my_func"));
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()), R"(%my_func = func():vec4<bool> -> %b1 {
  %b1 = block {
    ret vec4<bool>(false)
  }
}
%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:vec4<bool> = call %my_func
    %tint_symbol:vec4<bool> = eq %3, vec4<bool>(false)
    ret
  }
}
)");
}

TEST_F(ProgramToIRUnaryTest, EmitExpression_Unary_Complement) {
    Func("my_func", tint::Empty, ty.u32(), Vector{Return(1_u)});
    auto* expr = Complement(Call("my_func"));
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()), R"(%my_func = func():u32 -> %b1 {
  %b1 = block {
    ret 1u
  }
}
%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:u32 = call %my_func
    %tint_symbol:u32 = complement %3
    ret
  }
}
)");
}

TEST_F(ProgramToIRUnaryTest, EmitExpression_Unary_Negation) {
    Func("my_func", tint::Empty, ty.i32(), Vector{Return(1_i)});
    auto* expr = Negation(Call("my_func"));
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()), R"(%my_func = func():i32 -> %b1 {
  %b1 = block {
    ret 1i
  }
}
%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:i32 = call %my_func
    %tint_symbol:i32 = negation %3
    ret
  }
}
)");
}

TEST_F(ProgramToIRUnaryTest, EmitExpression_Unary_AddressOf) {
    GlobalVar("v1", core::AddressSpace::kPrivate, ty.i32());

    auto* expr = Decl(Let("v2", AddressOf("v1")));
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()), R"(%b1 = block {  # root
  %v1:ptr<private, i32, read_write> = var
}

%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %v2:ptr<private, i32, read_write> = let %v1
    ret
  }
}
)");
}

TEST_F(ProgramToIRUnaryTest, EmitExpression_Unary_Indirection) {
    GlobalVar("v1", core::AddressSpace::kPrivate, ty.i32());
    Vector stmts = {
        Decl(Let("v3", AddressOf("v1"))),
        Assign(Deref("v3"), 42_i),
    };
    WrapInFunction(stmts);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()), R"(%b1 = block {  # root
  %v1:ptr<private, i32, read_write> = var
}

%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %v3:ptr<private, i32, read_write> = let %v1
    store %v3, 42i
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::wgsl::reader
