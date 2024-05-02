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

#include <string>
#include <tuple>
#include <utility>

#include "gmock/gmock.h"

#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/utils/text/string.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_ValidatorTest = IRTestHelper;

TEST_F(IR_ValidatorTest, RootBlock_Var) {
    mod.root_block->Append(b.Var(ty.ptr<private_, i32>()));
    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, RootBlock_NonVar) {
    auto* l = b.Loop();
    l->Body()->Append(b.Continue(l));

    mod.root_block->Append(l);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:3 error: loop: root block: invalid instruction: tint::core::ir::Loop
  loop [b: $B2] {  # loop_1
  ^^^^^^^^^^^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  loop [b: $B2] {  # loop_1
    $B2: {  # body
      continue  # -> $B3
    }
  }
}

)");
}

TEST_F(IR_ValidatorTest, RootBlock_VarBlockMismatch) {
    auto* var = b.Var(ty.ptr<private_, i32>());
    mod.root_block->Append(var);

    auto* f = b.Function("f", ty.void_());
    f->Block()->Append(b.Return(f));
    var->SetBlock(f->Block());

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:38 error: var: instruction in root block does not have root block as parent
  %1:ptr<private, i32, read_write> = var
                                     ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:ptr<private, i32, read_write> = var
}

%f = func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function) {
    auto* f = b.Function("my_func", ty.void_());

    f->SetParams({b.FunctionParam(ty.i32()), b.FunctionParam(ty.f32())});
    f->Block()->Append(b.Return(f));

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, Function_Duplicate) {
    auto* f = b.Function("my_func", ty.void_());
    // Function would auto-push by the builder, so this adds a duplicate
    mod.functions.Push(f);

    f->SetParams({b.FunctionParam(ty.i32()), b.FunctionParam(ty.f32())});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: function %my_func added to module multiple times
%my_func = func(%2:i32, %3:f32):void {
^^^^^^^^

note: # Disassembly
%my_func = func(%2:i32, %3:f32):void {
  $B1: {
    ret
  }
}
%my_func = func(%2:i32, %3:f32):void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_DeadParameter) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.f32());
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    p->Destroy();

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:17 error: destroyed parameter found in function parameter list
%my_func = func(%my_param:f32):void {
                ^^^^^^^^^^^^^

note: # Disassembly
%my_func = func(%my_param:f32):void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_ParameterWithNullFunction) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.f32());
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    p->SetFunction(nullptr);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:17 error: function parameter has nullptr parent function
%my_func = func(%my_param:f32):void {
                ^^^^^^^^^^^^^

note: # Disassembly
%my_func = func(%my_param:f32):void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_ParameterUsedInMultipleFunctions) {
    auto* p = b.FunctionParam("my_param", ty.f32());
    auto* f1 = b.Function("my_func1", ty.void_());
    auto* f2 = b.Function("my_func2", ty.void_());
    f1->SetParams({p});
    f2->SetParams({p});
    f1->Block()->Append(b.Return(f1));
    f2->Block()->Append(b.Return(f2));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:18 error: function parameter has incorrect parent function
%my_func1 = func(%my_param:f32):void {
                 ^^^^^^^^^^^^^

:6:1 note: parent function declared here
%my_func2 = func(%my_param:f32):void {
^^^^^^^^^

note: # Disassembly
%my_func1 = func(%my_param:f32):void {
  $B1: {
    ret
  }
}
%my_func2 = func(%my_param:f32):void {
  $B2: {
    ret
  }
}
)");
}

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
    auto* g = b.Function("g", ty.void_(), Function::PipelineStage::kCompute);

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
%g = @compute func():void {
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
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:28 error: call: function parameter 1 is of type i32, but argument is of type f32
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

TEST_F(IR_ValidatorTest, Block_NoTerminator) {
    b.Function("my_func", ty.void_());

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:3 error: block does not end in a terminator instruction
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
  }
}
)");
}

TEST_F(IR_ValidatorTest, Block_VarBlockMismatch) {
    auto* var = b.Var(ty.ptr<function, i32>());

    auto* f = b.Function("f", ty.void_());
    f->Block()->Append(var);
    f->Block()->Append(b.Return(f));

    auto* g = b.Function("g", ty.void_());
    g->Block()->Append(b.Return(g));

    var->SetBlock(g->Block());

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:41 error: var: block instruction does not have same block as parent
    %2:ptr<function, i32, read_write> = var
                                        ^^^

:2:3 note: in block
  $B1: {
  ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var
    ret
  }
}
%g = func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Block_DeadParameter) {
    auto* f = b.Function("my_func", ty.void_());

    auto* p = b.BlockParam("my_param", ty.f32());
    b.Append(f->Block(), [&] {
        auto* l = b.Loop();
        l->Body()->SetParams({p});
        b.Append(l->Body(), [&] { b.ExitLoop(l); });
        b.Return(f);
    });

    p->Destroy();

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:12 error: destroyed parameter found in block parameter list
      $B2 (%my_param:f32): {  # body
           ^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2 (%my_param:f32): {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Block_ParameterWithNullBlock) {
    auto* f = b.Function("my_func", ty.void_());

    auto* p = b.BlockParam("my_param", ty.f32());
    b.Append(f->Block(), [&] {
        auto* l = b.Loop();
        l->Body()->SetParams({p});
        b.Append(l->Body(), [&] { b.ExitLoop(l); });
        b.Return(f);
    });

    p->SetBlock(nullptr);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:12 error: block parameter has nullptr parent block
      $B2 (%my_param:f32): {  # body
           ^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2 (%my_param:f32): {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Block_ParameterUsedInMultipleBlocks) {
    auto* f = b.Function("my_func", ty.void_());

    auto* p = b.BlockParam("my_param", ty.f32());
    b.Append(f->Block(), [&] {
        auto* l = b.Loop();
        l->Body()->SetParams({p});
        b.Append(l->Body(), [&] { b.Continue(l, p); });
        l->Continuing()->SetParams({p});
        b.Append(l->Continuing(), [&] { b.NextIteration(l, p); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:12 error: block parameter has incorrect parent block
      $B2 (%my_param:f32): {  # body
           ^^^^^^^^^

:7:7 note: parent block declared here
      $B3 (%my_param:f32): {  # continuing
      ^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2 (%my_param:f32): {  # body
        continue %my_param  # -> $B3
      }
      $B3 (%my_param:f32): {  # continuing
        next_iteration %my_param  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_NegativeIndex) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.vec3<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, -1_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:25 error: access: constant index must be positive, got -1
    %3:f32 = access %2, -1i
                        ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:vec3<f32>):void {
  $B1: {
    %3:f32 = access %2, -1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_OOB_Index_Value) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.mat3x2<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u, 3_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:29 error: access: index out of bounds for type vec2<f32>
    %3:f32 = access %2, 1u, 3u
                            ^^

:2:3 note: in block
  $B1: {
  ^^^

:3:29 note: acceptable range: [0..1]
    %3:f32 = access %2, 1u, 3u
                            ^^

note: # Disassembly
%my_func = func(%2:mat3x2<f32>):void {
  $B1: {
    %3:f32 = access %2, 1u, 3u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_OOB_Index_Ptr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, array<array<f32, 2>, 3>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<private_, f32>(), obj, 1_u, 3_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:3:55 error: access: index out of bounds for type ptr<private, array<f32, 2>, read_write>
    %3:ptr<private, f32, read_write> = access %2, 1u, 3u
                                                      ^^

:2:3 note: in block
  $B1: {
  ^^^

:3:55 note: acceptable range: [0..1]
    %3:ptr<private, f32, read_write> = access %2, 1u, 3u
                                                      ^^

note: # Disassembly
%my_func = func(%2:ptr<private, array<array<f32, 2>, 3>, read_write>):void {
  $B1: {
    %3:ptr<private, f32, read_write> = access %2, 1u, 3u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_StaticallyUnindexableType_Value) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.f32());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:25 error: access: type f32 cannot be indexed
    %3:f32 = access %2, 1u
                        ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:f32):void {
  $B1: {
    %3:f32 = access %2, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_StaticallyUnindexableType_Ptr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<private_, f32>(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:51 error: access: type ptr<private, f32, read_write> cannot be indexed
    %3:ptr<private, f32, read_write> = access %2, 1u
                                                  ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:ptr<private, f32, read_write>):void {
  $B1: {
    %3:ptr<private, f32, read_write> = access %2, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_DynamicallyUnindexableType_Value) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.i32()},
                                                          });

    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(str_ty);
    auto* idx = b.FunctionParam(ty.i32());
    f->SetParams({obj, idx});

    b.Append(f->Block(), [&] {
        b.Access(ty.i32(), obj, idx);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:25 error: access: type MyStruct cannot be dynamically indexed
    %4:i32 = access %2, %3
                        ^^

:7:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
MyStruct = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
}

%my_func = func(%2:MyStruct, %3:i32):void {
  $B1: {
    %4:i32 = access %2, %3
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_DynamicallyUnindexableType_Ptr) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.i32()},
                                                          });

    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, read_write>(str_ty));
    auto* idx = b.FunctionParam(ty.i32());
    f->SetParams({obj, idx});

    b.Append(f->Block(), [&] {
        b.Access(ty.i32(), obj, idx);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:8:25 error: access: type ptr<private, MyStruct, read_write> cannot be dynamically indexed
    %4:i32 = access %2, %3
                        ^^

:7:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
MyStruct = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
}

%my_func = func(%2:ptr<private, MyStruct, read_write>, %3:i32):void {
  $B1: {
    %4:i32 = access %2, %3
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_Incorrect_Type_Value_Value) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.mat3x2<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.i32(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:14 error: access: result of access chain is type f32 but instruction type is i32
    %3:i32 = access %2, 1u, 1u
             ^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:mat3x2<f32>):void {
  $B1: {
    %3:i32 = access %2, 1u, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_Incorrect_Type_Ptr_Ptr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, array<array<f32, 2>, 3>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<private_, i32>(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:3:40 error: access: result of access chain is type ptr<private, f32, read_write> but instruction type is ptr<private, i32, read_write>
    %3:ptr<private, i32, read_write> = access %2, 1u, 1u
                                       ^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:ptr<private, array<array<f32, 2>, 3>, read_write>):void {
  $B1: {
    %3:ptr<private, i32, read_write> = access %2, 1u, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_Incorrect_Type_Ptr_Value) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, array<array<f32, 2>, 3>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:3:14 error: access: result of access chain is type ptr<private, f32, read_write> but instruction type is f32
    %3:f32 = access %2, 1u, 1u
             ^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:ptr<private, array<array<f32, 2>, 3>, read_write>):void {
  $B1: {
    %3:f32 = access %2, 1u, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_IndexVectorPtr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, vec3<f32>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:25 error: access: cannot obtain address of vector element
    %3:f32 = access %2, 1u
                        ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:ptr<private, vec3<f32>, read_write>):void {
  $B1: {
    %3:f32 = access %2, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_IndexVectorPtr_WithCapability) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, vec3<f32>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<private_, f32>(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowVectorElementPointer});
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Access_IndexVectorPtr_ViaMatrixPtr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, mat3x2<f32>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:29 error: access: cannot obtain address of vector element
    %3:f32 = access %2, 1u, 1u
                            ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:ptr<private, mat3x2<f32>, read_write>):void {
  $B1: {
    %3:f32 = access %2, 1u, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_IndexVectorPtr_ViaMatrixPtr_WithCapability) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, mat3x2<f32>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<private_, f32>(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowVectorElementPointer});
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Access_Incorrect_Ptr_AddressSpace) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<storage, array<f32, 2>, read>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<uniform, f32, read>(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:3:34 error: access: result of access chain is type ptr<storage, f32, read> but instruction type is ptr<uniform, f32, read>
    %3:ptr<uniform, f32, read> = access %2, 1u
                                 ^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:ptr<storage, array<f32, 2>, read>):void {
  $B1: {
    %3:ptr<uniform, f32, read> = access %2, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_Incorrect_Ptr_Access) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<storage, array<f32, 2>, read>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<storage, f32, read_write>(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:3:40 error: access: result of access chain is type ptr<storage, f32, read> but instruction type is ptr<storage, f32, read_write>
    %3:ptr<storage, f32, read_write> = access %2, 1u
                                       ^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:ptr<storage, array<f32, 2>, read>):void {
  $B1: {
    %3:ptr<storage, f32, read_write> = access %2, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_IndexVector) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.vec3<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Access_IndexVector_ViaMatrix) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.mat3x2<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Block_TerminatorInMiddle) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Return(f);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: return: block terminator which isn't the final instruction
    ret
    ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    ret
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, If_EmptyFalse) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(true);
    if_->True()->Append(b.Return(f));

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, If_EmptyTrue) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(true);
    if_->False()->Append(b.Return(f));

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:7 error: block does not end in a terminator instruction
      $B2: {  # true
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
      }
      $B3: {  # false
        ret
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, If_ConditionIsBool) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(1_i);
    if_->True()->Append(b.Return(f));
    if_->False()->Append(b.Return(f));

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:8 error: if: condition must be a `bool` type
    if 1i [t: $B2, f: $B3] {  # if_1
       ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    if 1i [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        ret
      }
      $B3: {  # false
        ret
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, If_ConditionIsNullptr) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(nullptr);
    if_->True()->Append(b.Return(f));
    if_->False()->Append(b.Return(f));

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:8 error: if: operand is undefined
    if undef [t: $B2, f: $B3] {  # if_1
       ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    if undef [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        ret
      }
      $B3: {  # false
        ret
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, If_NullResult) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(true);
    if_->True()->Append(b.Return(f));
    if_->False()->Append(b.Return(f));

    if_->SetResults(Vector<InstructionResult*, 1>{nullptr});

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: if: result is undefined
    undef = if true [t: $B2, f: $B3] {  # if_1
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    undef = if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        ret
      }
      $B3: {  # false
        ret
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Loop_OnlyBody) {
    auto* f = b.Function("my_func", ty.void_());

    auto* l = b.Loop();
    l->Body()->Append(b.ExitLoop(l));

    auto sb = b.Append(f->Block());
    sb.Append(l);
    sb.Return(f);

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, Loop_EmptyBody) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(b.Loop());
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:7 error: block does not end in a terminator instruction
      $B2: {  # body
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2: {  # body
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_RootBlock_NullResult) {
    auto* v = mod.allocators.instructions.Create<ir::Var>(nullptr);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:2:3 error: var: result is undefined
  undef = var
  ^^^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  undef = var
}

)");
}

TEST_F(IR_ValidatorTest, Var_Function_NullResult) {
    auto* v = mod.allocators.instructions.Create<ir::Var>(nullptr);

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: var: result is undefined
    undef = var
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    undef = var
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Init_WrongType) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, f32>();
        v->SetInitializer(b.Constant(1_i));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:41 error: var: initializer has incorrect type
    %2:ptr<function, f32, read_write> = var, 1i
                                        ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var, 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Let_NullResult) {
    auto* v = mod.allocators.instructions.Create<ir::Let>(nullptr, b.Constant(1_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: let: result is undefined
    undef = let 1i
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    undef = let 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Let_NullValue) {
    auto* v = mod.allocators.instructions.Create<ir::Let>(b.InstructionResult(ty.f32()), nullptr);

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:18 error: let: operand is undefined
    %2:f32 = let undef
                 ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:f32 = let undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Let_WrongType) {
    auto* v =
        mod.allocators.instructions.Create<ir::Let>(b.InstructionResult(ty.f32()), b.Constant(1_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:14 error: let: result type does not match value type
    %2:f32 = let 1i
             ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:f32 = let 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Instruction_AppendedDead) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    auto* ret = sb.Return(f);

    v->Destroy();
    v->InsertBefore(ret);

    auto addr = tint::ToString(v);
    auto arrows = std::string(addr.length(), '^');

    std::string expected = R"(:3:5 error: var: destroyed instruction found in instruction list
    <destroyed tint::core::ir::Var $ADDRESS>
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^$ARROWS^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    <destroyed tint::core::ir::Var $ADDRESS>
    ret
  }
}
)";

    expected = tint::ReplaceAll(expected, "$ADDRESS", addr);
    expected = tint::ReplaceAll(expected, "$ARROWS", arrows);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), expected);
}

TEST_F(IR_ValidatorTest, Instruction_NullInstruction) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    sb.Return(f);

    v->Result(0)->SetInstruction(nullptr);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: var: instruction of result is undefined
    %2:ptr<function, f32, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Instruction_DeadOperand) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    sb.Return(f);

    auto* result = sb.InstructionResult(ty.f32());
    result->Destroy();
    v->SetInitializer(result);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:46 error: var: operand is not alive
    %2:ptr<function, f32, read_write> = var, %3
                                             ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var, %3
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Instruction_OperandUsageRemoved) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    sb.Return(f);

    auto* result = sb.InstructionResult(ty.f32());
    v->SetInitializer(result);
    result->RemoveUsage({v, 0u});

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:46 error: var: operand missing usage
    %2:ptr<function, f32, read_write> = var, %3
                                             ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var, %3
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Instruction_OrphanedInstruction) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    auto* load = sb.Load(v);
    sb.Return(f);

    load->Remove();

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(error: load: orphaned instruction: load
note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Binary_LHS_Nullptr) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Add(ty.i32(), nullptr, sb.Constant(2_i));
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:18 error: binary: operand is undefined
    %2:i32 = add undef, 2i
                 ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32 = add undef, 2i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Binary_RHS_Nullptr) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Add(ty.i32(), sb.Constant(2_i), nullptr);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:22 error: binary: operand is undefined
    %2:i32 = add 2i, undef
                     ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32 = add 2i, undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Binary_Result_Nullptr) {
    auto* bin = mod.allocators.instructions.Create<ir::CoreBinary>(
        nullptr, BinaryOp::kAdd, b.Constant(3_i), b.Constant(2_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(bin);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: binary: result is undefined
    undef = add 3i, 2i
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    undef = add 3i, 2i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Unary_Value_Nullptr) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Negation(ty.i32(), nullptr);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:23 error: unary: operand is undefined
    %2:i32 = negation undef
                      ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32 = negation undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Unary_Result_Nullptr) {
    auto* bin = mod.allocators.instructions.Create<ir::CoreUnary>(nullptr, UnaryOp::kNegation,
                                                                  b.Constant(2_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(bin);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: unary: result is undefined
    undef = negation 2i
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    undef = negation 2i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Unary_ResultTypeNotMatchValueType) {
    auto* bin = b.Complement(ty.f32(), 2_i);

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(bin);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:3:5 error: unary: unary instruction result type (f32) does not match overload result type (i32)
    %2:f32 = complement 2i
    ^^^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:f32 = complement 2i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitIf) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, ExitIf_NullIf) {
    auto* if_ = b.If(true);
    if_->True()->Append(mod.allocators.instructions.Create<ExitIf>(nullptr));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:5:9 error: exit_if: has no parent control instruction
        exit_if  # undef
        ^^^^^^^

:4:7 note: in block
      $B2: {  # true
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    if true [t: $B2] {  # if_1
      $B2: {  # true
        exit_if  # undef
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitIf_LessOperandsThenIfParams) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_, 1_i));

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    if_->SetResults(Vector{r1, r2});

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:5:9 error: exit_if: args count (1) does not match control instruction result count (2)
        exit_if 1i  # if_1
        ^^^^^^^^^^

:4:7 note: in block
      $B2: {  # true
      ^^^

:3:5 note: control instruction
    %2:i32, %3:f32 = if true [t: $B2] {  # if_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32 = if true [t: $B2] {  # if_1
      $B2: {  # true
        exit_if 1i  # if_1
      }
      # implicit false block: exit_if undef, undef
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitIf_MoreOperandsThenIfParams) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_, 1_i, 2_f, 3_i));

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    if_->SetResults(Vector{r1, r2});

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:5:9 error: exit_if: args count (3) does not match control instruction result count (2)
        exit_if 1i, 2.0f, 3i  # if_1
        ^^^^^^^^^^^^^^^^^^^^

:4:7 note: in block
      $B2: {  # true
      ^^^

:3:5 note: control instruction
    %2:i32, %3:f32 = if true [t: $B2] {  # if_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32 = if true [t: $B2] {  # if_1
      $B2: {  # true
        exit_if 1i, 2.0f, 3i  # if_1
      }
      # implicit false block: exit_if undef, undef
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitIf_WithResult) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_, 1_i, 2_f));

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    if_->SetResults(Vector{r1, r2});

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, ExitIf_IncorrectResultType) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_, 1_i, 2_i));

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    if_->SetResults(Vector{r1, r2});

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:5:21 error: exit_if: argument type (f32) does not match control instruction type (i32)
        exit_if 1i, 2i  # if_1
                    ^^

:4:7 note: in block
      $B2: {  # true
      ^^^

:3:5 note: control instruction
    %2:i32, %3:f32 = if true [t: $B2] {  # if_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32 = if true [t: $B2] {  # if_1
      $B2: {  # true
        exit_if 1i, 2i  # if_1
      }
      # implicit false block: exit_if undef, undef
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitIf_NotInParentIf) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(true);
    if_->True()->Append(b.Return(f));

    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.ExitIf(if_);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:5 error: exit_if: found outside all control instructions
    exit_if  # if_1
    ^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    if true [t: $B2] {  # if_1
      $B2: {  # true
        ret
      }
    }
    exit_if  # if_1
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitIf_InvalidJumpsOverIf) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_inner = b.If(true);

    auto* if_outer = b.If(true);
    b.Append(if_outer->True(), [&] {
        b.Append(if_inner);
        b.ExitIf(if_outer);
    });

    b.Append(if_inner->True(), [&] { b.ExitIf(if_outer); });

    b.Append(f->Block(), [&] {
        b.Append(if_outer);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:13 error: exit_if: if target jumps over other control instructions
            exit_if  # if_1
            ^^^^^^^

:6:11 note: in block
          $B3: {  # true
          ^^^

:5:9 note: first control instruction jumped
        if true [t: $B3] {  # if_2
        ^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    if true [t: $B2] {  # if_1
      $B2: {  # true
        if true [t: $B3] {  # if_2
          $B3: {  # true
            exit_if  # if_1
          }
        }
        exit_if  # if_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitIf_InvalidJumpOverSwitch) {
    auto* f = b.Function("my_func", ty.void_());

    auto* switch_inner = b.Switch(1_i);

    auto* if_outer = b.If(true);
    b.Append(if_outer->True(), [&] {
        b.Append(switch_inner);
        b.ExitIf(if_outer);
    });

    auto* c = b.Case(switch_inner, {b.Constant(1_i)});
    b.Append(c, [&] { b.ExitIf(if_outer); });

    b.Append(f->Block(), [&] {
        b.Append(if_outer);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:13 error: exit_if: if target jumps over other control instructions
            exit_if  # if_1
            ^^^^^^^

:6:11 note: in block
          $B3: {  # case
          ^^^

:5:9 note: first control instruction jumped
        switch 1i [c: (1i, $B3)] {  # switch_1
        ^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    if true [t: $B2] {  # if_1
      $B2: {  # true
        switch 1i [c: (1i, $B3)] {  # switch_1
          $B3: {  # case
            exit_if  # if_1
          }
        }
        exit_if  # if_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitIf_InvalidJumpOverLoop) {
    auto* f = b.Function("my_func", ty.void_());

    auto* loop = b.Loop();

    auto* if_outer = b.If(true);
    b.Append(if_outer->True(), [&] {
        b.Append(loop);
        b.ExitIf(if_outer);
    });

    b.Append(loop->Body(), [&] { b.ExitIf(if_outer); });

    b.Append(f->Block(), [&] {
        b.Append(if_outer);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:13 error: exit_if: if target jumps over other control instructions
            exit_if  # if_1
            ^^^^^^^

:6:11 note: in block
          $B3: {  # body
          ^^^

:5:9 note: first control instruction jumped
        loop [b: $B3] {  # loop_1
        ^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    if true [t: $B2] {  # if_1
      $B2: {  # true
        loop [b: $B3] {  # loop_1
          $B3: {  # body
            exit_if  # if_1
          }
        }
        exit_if  # if_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitSwitch) {
    auto* switch_ = b.Switch(true);

    auto* def = b.DefaultCase(switch_);
    def->Append(b.ExitSwitch(switch_));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, ExitSwitch_NullSwitch) {
    auto* switch_ = b.Switch(true);

    auto* def = b.DefaultCase(switch_);
    def->Append(mod.allocators.instructions.Create<ExitSwitch>(nullptr));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:9 error: exit_switch: has no parent control instruction
        exit_switch  # undef
        ^^^^^^^^^^^

:4:7 note: in block
      $B2: {  # case
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    switch true [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        exit_switch  # undef
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitSwitch_LessOperandsThenSwitchParams) {
    auto* switch_ = b.Switch(true);

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    switch_->SetResults(Vector{r1, r2});

    auto* def = b.DefaultCase(switch_);
    def->Append(b.ExitSwitch(switch_, 1_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:5:9 error: exit_switch: args count (1) does not match control instruction result count (2)
        exit_switch 1i  # switch_1
        ^^^^^^^^^^^^^^

:4:7 note: in block
      $B2: {  # case
      ^^^

:3:5 note: control instruction
    %2:i32, %3:f32 = switch true [c: (default, $B2)] {  # switch_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32 = switch true [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        exit_switch 1i  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitSwitch_MoreOperandsThenSwitchParams) {
    auto* switch_ = b.Switch(true);
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    switch_->SetResults(Vector{r1, r2});

    auto* def = b.DefaultCase(switch_);
    def->Append(b.ExitSwitch(switch_, 1_i, 2_f, 3_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:5:9 error: exit_switch: args count (3) does not match control instruction result count (2)
        exit_switch 1i, 2.0f, 3i  # switch_1
        ^^^^^^^^^^^^^^^^^^^^^^^^

:4:7 note: in block
      $B2: {  # case
      ^^^

:3:5 note: control instruction
    %2:i32, %3:f32 = switch true [c: (default, $B2)] {  # switch_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32 = switch true [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        exit_switch 1i, 2.0f, 3i  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitSwitch_WithResult) {
    auto* switch_ = b.Switch(true);
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    switch_->SetResults(Vector{r1, r2});

    auto* def = b.DefaultCase(switch_);
    def->Append(b.ExitSwitch(switch_, 1_i, 2_f));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, ExitSwitch_IncorrectResultType) {
    auto* switch_ = b.Switch(true);
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    switch_->SetResults(Vector{r1, r2});

    auto* def = b.DefaultCase(switch_);
    def->Append(b.ExitSwitch(switch_, 1_i, 2_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:5:25 error: exit_switch: argument type (f32) does not match control instruction type (i32)
        exit_switch 1i, 2i  # switch_1
                        ^^

:4:7 note: in block
      $B2: {  # case
      ^^^

:3:5 note: control instruction
    %2:i32, %3:f32 = switch true [c: (default, $B2)] {  # switch_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32 = switch true [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        exit_switch 1i, 2i  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitSwitch_NotInParentSwitch) {
    auto* switch_ = b.Switch(true);

    auto* f = b.Function("my_func", ty.void_());

    auto* def = b.DefaultCase(switch_);
    def->Append(b.Return(f));

    auto sb = b.Append(f->Block());
    sb.Append(switch_);

    auto* if_ = sb.Append(b.If(true));
    b.Append(if_->True(), [&] { b.ExitSwitch(switch_); });
    sb.Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:10:9 error: exit_switch: switch not found in parent control instructions
        exit_switch  # switch_1
        ^^^^^^^^^^^

:9:7 note: in block
      $B3: {  # true
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    switch true [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        ret
      }
    }
    if true [t: $B3] {  # if_1
      $B3: {  # true
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitSwitch_JumpsOverIfs) {
    // switch(true) {
    //   default: {
    //     if (true) {
    //      if (false) {
    //         break;
    //       }
    //     }
    //     break;
    //  }
    auto* switch_ = b.Switch(true);

    auto* f = b.Function("my_func", ty.void_());

    auto* def = b.DefaultCase(switch_);
    b.Append(def, [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            auto* inner_if_ = b.If(false);
            b.Append(inner_if_->True(), [&] { b.ExitSwitch(switch_); });
            b.Return(f);
        });
        b.ExitSwitch(switch_);
    });

    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, ExitSwitch_InvalidJumpOverSwitch) {
    auto* switch_ = b.Switch(true);

    auto* def = b.DefaultCase(switch_);
    b.Append(def, [&] {
        auto* inner = b.Switch(false);
        b.ExitSwitch(switch_);

        auto* inner_def = b.DefaultCase(inner);
        b.Append(inner_def, [&] { b.ExitSwitch(switch_); });
    });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(switch_);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:13 error: exit_switch: switch target jumps over other control instructions
            exit_switch  # switch_1
            ^^^^^^^^^^^

:6:11 note: in block
          $B3: {  # case
          ^^^

:5:9 note: first control instruction jumped
        switch false [c: (default, $B3)] {  # switch_2
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    switch true [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        switch false [c: (default, $B3)] {  # switch_2
          $B3: {  # case
            exit_switch  # switch_1
          }
        }
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitSwitch_InvalidJumpOverLoop) {
    auto* switch_ = b.Switch(true);

    auto* def = b.DefaultCase(switch_);
    b.Append(def, [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.ExitSwitch(switch_); });
        b.ExitSwitch(switch_);
    });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(switch_);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:13 error: exit_switch: switch target jumps over other control instructions
            exit_switch  # switch_1
            ^^^^^^^^^^^

:6:11 note: in block
          $B3: {  # body
          ^^^

:5:9 note: first control instruction jumped
        loop [b: $B3] {  # loop_1
        ^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    switch true [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        loop [b: $B3] {  # loop_1
          $B3: {  # body
            exit_switch  # switch_1
          }
        }
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop) {
    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, ExitLoop_NullLoop) {
    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(mod.allocators.instructions.Create<ExitLoop>(nullptr));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:9 error: exit_loop: has no parent control instruction
        exit_loop  # undef
        ^^^^^^^^^

:4:7 note: in block
      $B2: {  # body
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        exit_loop  # undef
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_LessOperandsThenLoopParams) {
    auto* loop = b.Loop();
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    loop->SetResults(Vector{r1, r2});

    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop, 1_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:5:9 error: exit_loop: args count (1) does not match control instruction result count (2)
        exit_loop 1i  # loop_1
        ^^^^^^^^^^^^

:4:7 note: in block
      $B2: {  # body
      ^^^

:3:5 note: control instruction
    %2:i32, %3:f32 = loop [b: $B2, c: $B3] {  # loop_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32 = loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        exit_loop 1i  # loop_1
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_MoreOperandsThenLoopParams) {
    auto* loop = b.Loop();
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    loop->SetResults(Vector{r1, r2});

    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop, 1_i, 2_f, 3_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:5:9 error: exit_loop: args count (3) does not match control instruction result count (2)
        exit_loop 1i, 2.0f, 3i  # loop_1
        ^^^^^^^^^^^^^^^^^^^^^^

:4:7 note: in block
      $B2: {  # body
      ^^^

:3:5 note: control instruction
    %2:i32, %3:f32 = loop [b: $B2, c: $B3] {  # loop_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32 = loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        exit_loop 1i, 2.0f, 3i  # loop_1
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_WithResult) {
    auto* loop = b.Loop();
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    loop->SetResults(Vector{r1, r2});

    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop, 1_i, 2_f));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, ExitLoop_IncorrectResultType) {
    auto* loop = b.Loop();
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    loop->SetResults(Vector{r1, r2});

    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop, 1_i, 2_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:5:23 error: exit_loop: argument type (f32) does not match control instruction type (i32)
        exit_loop 1i, 2i  # loop_1
                      ^^

:4:7 note: in block
      $B2: {  # body
      ^^^

:3:5 note: control instruction
    %2:i32, %3:f32 = loop [b: $B2, c: $B3] {  # loop_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32 = loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        exit_loop 1i, 2i  # loop_1
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_NotInParentLoop) {
    auto* f = b.Function("my_func", ty.void_());

    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.Return(f));

    auto sb = b.Append(f->Block());
    sb.Append(loop);

    auto* if_ = sb.Append(b.If(true));
    b.Append(if_->True(), [&] { b.ExitLoop(loop); });
    sb.Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:13:9 error: exit_loop: loop not found in parent control instructions
        exit_loop  # loop_1
        ^^^^^^^^^

:12:7 note: in block
      $B4: {  # true
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        ret
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    if true [t: $B4] {  # if_1
      $B4: {  # true
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_JumpsOverIfs) {
    // loop {
    //   if (true) {
    //    if (false) {
    //       break;
    //     }
    //   }
    //   break;
    // }
    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));

    auto* f = b.Function("my_func", ty.void_());

    b.Append(loop->Body(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            auto* inner_if_ = b.If(false);
            b.Append(inner_if_->True(), [&] { b.ExitLoop(loop); });
            b.Return(f);
        });
        b.ExitLoop(loop);
    });

    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidJumpOverSwitch) {
    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));

    b.Append(loop->Body(), [&] {
        auto* inner = b.Switch(false);
        b.ExitLoop(loop);

        auto* inner_def = b.DefaultCase(inner);
        b.Append(inner_def, [&] { b.ExitLoop(loop); });
    });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:13 error: exit_loop: loop target jumps over other control instructions
            exit_loop  # loop_1
            ^^^^^^^^^

:6:11 note: in block
          $B4: {  # case
          ^^^

:5:9 note: first control instruction jumped
        switch false [c: (default, $B4)] {  # switch_1
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        switch false [c: (default, $B4)] {  # switch_1
          $B4: {  # case
            exit_loop  # loop_1
          }
        }
        exit_loop  # loop_1
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidJumpOverLoop) {
    auto* outer_loop = b.Loop();

    outer_loop->Continuing()->Append(b.NextIteration(outer_loop));

    b.Append(outer_loop->Body(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.ExitLoop(outer_loop); });
        b.ExitLoop(outer_loop);
    });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(outer_loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:13 error: exit_loop: loop target jumps over other control instructions
            exit_loop  # loop_1
            ^^^^^^^^^

:6:11 note: in block
          $B4: {  # body
          ^^^

:5:9 note: first control instruction jumped
        loop [b: $B4] {  # loop_2
        ^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        loop [b: $B4] {  # loop_2
          $B4: {  # body
            exit_loop  # loop_1
          }
        }
        exit_loop  # loop_1
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidInsideContinuing) {
    auto* loop = b.Loop();

    loop->Continuing()->Append(b.ExitLoop(loop));
    loop->Body()->Append(b.Continue(loop));

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:9 error: exit_loop: loop exit jumps out of continuing block
        exit_loop  # loop_1
        ^^^^^^^^^

:7:7 note: in block
      $B3: {  # continuing
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidInsideContinuingNested) {
    auto* loop = b.Loop();

    b.Append(loop->Continuing(), [&]() {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&]() { b.ExitLoop(loop); });
        b.NextIteration(loop);
    });

    b.Append(loop->Body(), [&] { b.Continue(loop); });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:10:13 error: exit_loop: loop exit jumps out of continuing block
            exit_loop  # loop_1
            ^^^^^^^^^

:9:11 note: in block
          $B4: {  # true
          ^^^

:7:7 note: in continuing block
      $B3: {  # continuing
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        if true [t: $B4] {  # if_1
          $B4: {  # true
            exit_loop  # loop_1
          }
        }
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidInsideInitializer) {
    auto* loop = b.Loop();

    loop->Initializer()->Append(b.ExitLoop(loop));
    loop->Continuing()->Append(b.NextIteration(loop));

    b.Append(loop->Body(), [&] { b.Continue(loop); });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:9 error: exit_loop: loop exit not permitted in loop initializer
        exit_loop  # loop_1
        ^^^^^^^^^

:4:7 note: in block
      $B2: {  # initializer
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        exit_loop  # loop_1
      }
      $B3: {  # body
        continue  # -> $B4
      }
      $B4: {  # continuing
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidInsideInitializerNested) {
    auto* loop = b.Loop();

    b.Append(loop->Initializer(), [&]() {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&]() { b.ExitLoop(loop); });
        b.NextIteration(loop);
    });
    loop->Continuing()->Append(b.NextIteration(loop));

    b.Append(loop->Body(), [&] { b.Continue(loop); });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:13 error: exit_loop: loop exit not permitted in loop initializer
            exit_loop  # loop_1
            ^^^^^^^^^

:6:11 note: in block
          $B5: {  # true
          ^^^

:4:7 note: in initializer block
      $B2: {  # initializer
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        if true [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        next_iteration  # -> $B3
      }
      $B3: {  # body
        continue  # -> $B4
      }
      $B4: {  # continuing
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Return) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {  //
        b.Return(f);
    });

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, Return_WithValue) {
    auto* f = b.Function("my_func", ty.i32());
    b.Append(f->Block(), [&] {  //
        b.Return(f, 42_i);
    });

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, Return_NullFunction) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {  //
        b.Return(nullptr);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: return: undefined function
    ret
    ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Return_UnexpectedValue) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {  //
        b.Return(f, 42_i);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: return: unexpected return value
    ret 42i
    ^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    ret 42i
  }
}
)");
}

TEST_F(IR_ValidatorTest, Return_MissingValue) {
    auto* f = b.Function("my_func", ty.i32());
    b.Append(f->Block(), [&] {  //
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: return: expected return value
    ret
    ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():i32 {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Return_WrongValueType) {
    auto* f = b.Function("my_func", ty.i32());
    b.Append(f->Block(), [&] {  //
        b.Return(f, 42_f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: return: return value type does not match function return type
    ret 42.0f
    ^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():i32 {
  $B1: {
    ret 42.0f
  }
}
)");
}

TEST_F(IR_ValidatorTest, Load_NullFrom) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(
            mod.allocators.instructions.Create<ir::Load>(b.InstructionResult(ty.i32()), nullptr));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:19 error: load: operand is undefined
    %2:i32 = load undef
                  ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32 = load undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Load_SourceNotMemoryView) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* let = b.Let("l", 1_i);
        b.Append(mod.allocators.instructions.Create<ir::Load>(b.InstructionResult(ty.f32()),
                                                              let->Result(0)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:19 error: load: load source operand is not a memory view
    %3:f32 = load %l
                  ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %l:i32 = let 1i
    %3:f32 = load %l
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Load_TypeMismatch) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Append(mod.allocators.instructions.Create<ir::Load>(b.InstructionResult(ty.f32()),
                                                              var->Result(0)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:19 error: load: result type does not match source store type
    %3:f32 = load %2
                  ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var
    %3:f32 = load %2
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Store_NullTo) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(mod.allocators.instructions.Create<ir::Store>(nullptr, b.Constant(42_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:11 error: store: operand is undefined
    store undef, 42i
          ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    store undef, 42i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Store_NullFrom) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Append(mod.allocators.instructions.Create<ir::Store>(var->Result(0), nullptr));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:4:15 error: store: operand is undefined
    store %2, undef
              ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var
    store %2, undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Store_TargetNotMemoryView) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* let = b.Let("l", 1_i);
        b.Append(mod.allocators.instructions.Create<ir::Store>(let->Result(0), b.Constant(42_u)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:15 error: store: store target operand is not a memory view
    store %l, 42u
              ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %l:i32 = let 1i
    store %l, 42u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Store_TypeMismatch) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Append(mod.allocators.instructions.Create<ir::Store>(var->Result(0), b.Constant(42_u)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:15 error: store: value type does not match store type
    store %2, 42u
              ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var
    store %2, 42u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, LoadVectorElement_NullResult) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        b.Append(mod.allocators.instructions.Create<ir::LoadVectorElement>(nullptr, var->Result(0),
                                                                           b.Constant(1_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:5 error: load_vector_element: result is undefined
    undef = load_vector_element %2, 1i
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec3<f32>, read_write> = var
    undef = load_vector_element %2, 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, LoadVectorElement_NullFrom) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(mod.allocators.instructions.Create<ir::LoadVectorElement>(
            b.InstructionResult(ty.f32()), nullptr, b.Constant(1_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:34 error: load_vector_element: operand is undefined
    %2:f32 = load_vector_element undef, 1i
                                 ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:f32 = load_vector_element undef, 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, LoadVectorElement_NullIndex) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        b.Append(mod.allocators.instructions.Create<ir::LoadVectorElement>(
            b.InstructionResult(ty.f32()), var->Result(0), nullptr));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:4:38 error: load_vector_element: operand is undefined
    %3:f32 = load_vector_element %2, undef
                                     ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec3<f32>, read_write> = var
    %3:f32 = load_vector_element %2, undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, StoreVectorElement_NullTo) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(mod.allocators.instructions.Create<ir::StoreVectorElement>(
            nullptr, b.Constant(1_i), b.Constant(2_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:26 error: store_vector_element: operand is undefined
    store_vector_element undef, 1i, 2i
                         ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    store_vector_element undef, 1i, 2i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, StoreVectorElement_NullIndex) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        b.Append(mod.allocators.instructions.Create<ir::StoreVectorElement>(var->Result(0), nullptr,
                                                                            b.Constant(2_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:4:30 error: store_vector_element: operand is undefined
    store_vector_element %2, undef, 2i
                             ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

:4:37 error: store_vector_element: value type does not match vector pointer element type
    store_vector_element %2, undef, 2i
                                    ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec3<f32>, read_write> = var
    store_vector_element %2, undef, 2i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, StoreVectorElement_NullValue) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        b.Append(mod.allocators.instructions.Create<ir::StoreVectorElement>(
            var->Result(0), b.Constant(1_i), nullptr));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:4:34 error: store_vector_element: operand is undefined
    store_vector_element %2, 1i, undef
                                 ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec3<f32>, read_write> = var
    store_vector_element %2, 1i, undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Scoping_UseBeforeDecl) {
    auto* f = b.Function("my_func", ty.void_());

    auto* y = b.Add<i32>(2_i, 3_i);
    auto* x = b.Add<i32>(y, 1_i);

    f->Block()->Append(x);
    f->Block()->Append(y);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:18 error: binary: %3 is not in scope
    %2:i32 = add %3, 1i
                 ^^

:2:3 note: in block
  $B1: {
  ^^^

:4:5 note: %3 declared here
    %3:i32 = add 2i, 3i
    ^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32 = add %3, 1i
    %3:i32 = add 2i, 3i
    ret
  }
}
)");
}

template <typename T>
static const type::Type* TypeBuilder(type::Manager& m) {
    return m.Get<T>();
}
template <typename T>
static const type::Type* RefTypeBuilder(type::Manager& m) {
    return m.ref<AddressSpace::kFunction, T>();
}
using TypeBuilderFn = decltype(&TypeBuilder<i32>);

using IR_ValidatorRefTypeTest = IRTestParamHelper<std::tuple</* holds_ref */ bool,
                                                             /* refs_allowed */ bool,
                                                             /* type_builder */ TypeBuilderFn>>;

TEST_P(IR_ValidatorRefTypeTest, Var) {
    bool holds_ref = std::get<0>(GetParam());
    bool refs_allowed = std::get<1>(GetParam());
    auto* type = std::get<2>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        if (auto* view = type->As<type::MemoryView>()) {
            b.Var(view);
        } else {
            b.Var(ty.ptr<function>(type));
        }

        b.Return(fn);
    });

    Capabilities caps;
    if (refs_allowed) {
        caps.Add(Capability::kAllowRefTypes);
    }
    auto res = ir::Validate(mod, caps);
    if (!holds_ref || refs_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("3:5 error: var: reference type is not permitted"));
    }
}

TEST_P(IR_ValidatorRefTypeTest, FnParam) {
    bool holds_ref = std::get<0>(GetParam());
    bool refs_allowed = std::get<1>(GetParam());
    auto* type = std::get<2>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    fn->SetParams(Vector{b.FunctionParam(type)});
    b.Append(fn->Block(), [&] { b.Return(fn); });

    Capabilities caps;
    if (refs_allowed) {
        caps.Add(Capability::kAllowRefTypes);
    }
    auto res = ir::Validate(mod, caps);
    if (!holds_ref) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("references are not permitted as parameter types"));
    }
}

TEST_P(IR_ValidatorRefTypeTest, FnRet) {
    bool holds_ref = std::get<0>(GetParam());
    bool refs_allowed = std::get<1>(GetParam());
    auto* type = std::get<2>(GetParam())(ty);

    auto* fn = b.Function("my_func", type);
    b.Append(fn->Block(), [&] { b.Unreachable(); });

    Capabilities caps;
    if (refs_allowed) {
        caps.Add(Capability::kAllowRefTypes);
    }
    auto res = ir::Validate(mod, caps);
    if (!holds_ref) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("references are not permitted as return types"));
    }
}

INSTANTIATE_TEST_SUITE_P(NonRefTypes,
                         IR_ValidatorRefTypeTest,
                         testing::Combine(/* holds_ref */ testing::Values(false),
                                          /* refs_allowed */ testing::Values(false, true),
                                          /* type_builder */
                                          testing::Values(TypeBuilder<i32>,
                                                          TypeBuilder<bool>,
                                                          TypeBuilder<vec4<f32>>,
                                                          TypeBuilder<array<f32, 3>>)));

INSTANTIATE_TEST_SUITE_P(RefTypes,
                         IR_ValidatorRefTypeTest,
                         testing::Combine(/* holds_ref */ testing::Values(true),
                                          /* refs_allowed */ testing::Values(false, true),
                                          /* type_builder */
                                          testing::Values(RefTypeBuilder<i32>,
                                                          RefTypeBuilder<bool>,
                                                          RefTypeBuilder<vec4<f32>>)));
}  // namespace
}  // namespace tint::core::ir
