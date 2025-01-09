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

#include "src/tint/lang/core/ir/validator_test.h"

#include <string>

#include "gtest/gtest.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/storage_texture.h"

namespace tint::core::ir {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(IR_ValidatorTest, CallToFunctionOutsideModule) {
    auto* f = b.Function("f", ty.void_());
    auto* g = b.Function("g", ty.void_());
    mod.functions.Pop();  // Remove g

    b.Append(f->Block(), [&] {
        b.Call(g);
        b.Return(f);
    });
    b.Append(g->Block(), [&] { b.Return(g); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:20 error: call: %g is not part of the module
    %2:void = call %g
                   ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    %2:void = call %g
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToEntryPointFunction) {
    auto* f = b.Function("f", ty.void_());
    auto* g = ComputeEntryPoint("g");

    b.Append(f->Block(), [&] {
        b.Call(g);
        b.Return(f);
    });
    b.Append(g->Block(), [&] { b.Return(g); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:20 error: call: call target must not have a pipeline stage
    %2:void = call %g
                   ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    %2:void = call %g
    ret
  }
}
%g = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToFunctionTooFewArguments) {
    auto* g = b.Function("g", ty.void_());
    g->SetParams({b.FunctionParam<i32>(), b.FunctionParam<i32>()});
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Call(g, 42_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:20 error: call: function has 2 parameters, but call provides 1 arguments
    %5:void = call %g, 42i
                   ^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
%g = func(%2:i32, %3:i32):void {
  $B1: {
    ret
  }
}
%f = func():void {
  $B2: {
    %5:void = call %g, 42i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToFunctionTooManyArguments) {
    auto* g = b.Function("g", ty.void_());
    g->SetParams({b.FunctionParam<i32>(), b.FunctionParam<i32>()});
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Call(g, 1_i, 2_i, 3_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:20 error: call: function has 2 parameters, but call provides 3 arguments
    %5:void = call %g, 1i, 2i, 3i
                   ^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
%g = func(%2:i32, %3:i32):void {
  $B1: {
    ret
  }
}
%f = func():void {
  $B2: {
    %5:void = call %g, 1i, 2i, 3i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToFunctionWrongArgType) {
    auto* g = b.Function("g", ty.void_());
    g->SetParams({b.FunctionParam<i32>(), b.FunctionParam<i32>(), b.FunctionParam<i32>()});
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Call(g, 1_i, 2_f, 3_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:8:28 error: call: function parameter 1 is of type 'i32', but argument is of type 'f32'
    %6:void = call %g, 1i, 2.0f, 3i
                           ^^^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
%g = func(%2:i32, %3:i32, %4:i32):void {
  $B1: {
    ret
  }
}
%f = func():void {
  $B2: {
    %6:void = call %g, 1i, 2.0f, 3i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToFunctionNullArg) {
    auto* g = b.Function("g", ty.void_());
    g->SetParams({b.FunctionParam<i32>()});
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Call(g, nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:24 error: call: operand is undefined
    %4:void = call %g, undef
                       ^^^^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
%g = func(%2:i32):void {
  $B1: {
    ret
  }
}
%f = func():void {
  $B2: {
    %4:void = call %g, undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToNullFunction) {
    auto* g = b.Function("g", ty.void_());
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(g);
        c->SetOperands(Vector{static_cast<ir::Value*>(nullptr)});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:20 error: call: operand is undefined
    %3:void = call undef
                   ^^^^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
%g = func():void {
  $B1: {
    ret
  }
}
%f = func():void {
  $B2: {
    %3:void = call undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToFunctionNoResult) {
    auto* g = b.Function("g", ty.void_());
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(g);
        c->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:13 error: call: expected exactly 1 results, got 0
    undef = call %g
            ^^^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
%g = func():void {
  $B1: {
    ret
  }
}
%f = func():void {
  $B2: {
    undef = call %g
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToFunctionNoOperands) {
    auto* g = b.Function("g", ty.void_());
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(g);
        c->ClearOperands();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:15 error: call: expected at least 1 operands, got 0
    %3:void = call undef
              ^^^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
%g = func():void {
  $B1: {
    ret
  }
}
%f = func():void {
  $B2: {
    %3:void = call undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToNonFunctionTarget) {
    auto* g = b.Function("g", ty.void_());
    mod.functions.Pop();  // Remove g, since it isn't actually going to be used, it is just
                          // needed to create the UserCall before mangling it

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(g);
        c->SetOperands(Vector{b.Value(0_i)});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:20 error: call: target not defined or not a function
    %2:void = call 0i
                   ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    %2:void = call 0i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToBuiltin_MissingResult) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(ty.f32(), BuiltinFn::kAbs, 1_f);
        c->SetResults(Vector{static_cast<ir::InstructionResult*>(nullptr)});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:13 error: abs: call to builtin does not have a return type
    undef = abs 1.0f
            ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    undef = abs 1.0f
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToBuiltin_MismatchResultType) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(ty.f32(), BuiltinFn::kAbs, 1_f);
        c->Result(0)->SetType(ty.i32());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:14 error: abs: call result type does not match builtin return type
    %2:i32 = abs 1.0f
             ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    %2:i32 = abs 1.0f
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToBuiltin_ArgNullType) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* i = b.Var<function, f32>("i");
        i->SetInitializer(b.Constant(0_f));
        auto* load = b.Load(i);
        auto* load_ret = load->Result(0);
        b.Call(ty.f32(), BuiltinFn::kAbs, load_ret);
        load_ret->SetType(nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:5 error: load: result type is undefined
    %3:undef = load %i
    ^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

:5:14 error: abs: argument to builtin has undefined type
    %4:f32 = abs %3
             ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    %i:ptr<function, f32, read_write> = var, 0.0f
    %3:undef = load %i
    %4:f32 = abs %3
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToBuiltin_NonSingularResult) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* i = b.Var<function, f32>("i");
        i->SetInitializer(b.Constant(0_f));
        auto* load = b.Load(i);
        auto* load_ret = load->Result(0);
        auto* too_many_call = b.Call(ty.f32(), BuiltinFn::kAbs, load_ret);
        too_many_call->SetResults(Vector{load_ret, load_ret});
        auto* too_few_call = b.Call(ty.f32(), BuiltinFn::kAbs, load_ret);
        too_few_call->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:14 error: abs: call to builtin has 2 results, when 1 is expected
    %3:f32 = abs %3
             ^^^

:2:3 note: in block
  $B1: {
  ^^^

:6:13 error: abs: call to builtin has 0 results, when 1 is expected
    undef = abs %3
            ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    %i:ptr<function, f32, read_write> = var, 0.0f
    %3:f32 = load %i
    %3:f32 = abs %3
    undef = abs %3
    ret
  }
}
)");
}

}  // namespace tint::core::ir
