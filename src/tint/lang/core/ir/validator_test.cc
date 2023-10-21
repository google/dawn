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
#include <utility>

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/utils/text/string.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_ValidatorTest = IRTestHelper;

TEST_F(IR_ValidatorTest, RootBlock_Var) {
    mod.root_block->Append(b.Var(ty.ptr<private_, i32>()));
    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().reason.str();
}

TEST_F(IR_ValidatorTest, RootBlock_NonVar) {
    auto* l = b.Loop();
    l->Body()->Append(b.Continue(l));

    mod.root_block->Append(l);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:2:3 error: root block: invalid instruction: tint::core::ir::Loop
  loop [b: %b2] {  # loop_1
  ^^^^^^^^^^^^^

:1:1 note: In block
%b1 = block {  # root
^^^^^^^^^^^

note: # Disassembly
%b1 = block {  # root
  loop [b: %b2] {  # loop_1
    %b2 = block {  # body
      continue %b3
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:2:38 error: var: instruction in root block does not have root block as parent
  %1:ptr<private, i32, read_write> = var
                                     ^^^

:1:1 note: In block
%b1 = block {  # root
^^^^^^^^^^^

note: # Disassembly
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%f = func():void -> %b2 {
  %b2 = block {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function) {
    auto* f = b.Function("my_func", ty.void_());

    f->SetParams({b.FunctionParam(ty.i32()), b.FunctionParam(ty.f32())});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().reason.str();
}

TEST_F(IR_ValidatorTest, Function_Duplicate) {
    auto* f = b.Function("my_func", ty.void_());
    // Function would auto-push by the builder, so this adds a duplicate
    mod.functions.Push(f);

    f->SetParams({b.FunctionParam(ty.i32()), b.FunctionParam(ty.f32())});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(error: function 'my_func' added to module multiple times
note: # Disassembly
%my_func = func(%2:i32, %3:f32):void -> %b1 {
  %b1 = block {
    ret
  }
}
%my_func = func(%2:i32, %3:f32):void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:3:20 error: call: call target is not part of the module
    %2:void = call %g
                   ^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%f = func():void -> %b1 {
  %b1 = block {
    %2:void = call %g
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Block_NoTerminator) {
    b.Function("my_func", ty.void_());

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:2:3 error: block: does not end in a terminator instruction
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:3:41 error: var: block instruction does not have same block as parent
    %2:ptr<function, i32, read_write> = var
                                        ^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%f = func():void -> %b1 {
  %b1 = block {
    %2:ptr<function, i32, read_write> = var
    ret
  }
}
%g = func():void -> %b2 {
  %b2 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:3:25 error: access: constant index must be positive, got -1
    %3:f32 = access %2, -1i
                        ^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func(%2:vec3<f32>):void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:3:29 error: access: index out of bounds for type vec2<f32>
    %3:f32 = access %2, 1u, 3u
                            ^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

:3:29 note: acceptable range: [0..1]
    %3:f32 = access %2, 1u, 3u
                            ^^

note: # Disassembly
%my_func = func(%2:mat3x2<f32>):void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:3:55 error: access: index out of bounds for type ptr<array<f32, 2>>
    %3:ptr<private, f32, read_write> = access %2, 1u, 3u
                                                      ^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

:3:55 note: acceptable range: [0..1]
    %3:ptr<private, f32, read_write> = access %2, 1u, 3u
                                                      ^^

note: # Disassembly
%my_func = func(%2:ptr<private, array<array<f32, 2>, 3>, read_write>):void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:25 error: access: type f32 cannot be indexed
    %3:f32 = access %2, 1u
                        ^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func(%2:f32):void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:51 error: access: type ptr<f32> cannot be indexed
    %3:ptr<private, f32, read_write> = access %2, 1u
                                                  ^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func(%2:ptr<private, f32, read_write>):void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:8:25 error: access: type MyStruct cannot be dynamically indexed
    %4:i32 = access %2, %3
                        ^^

:7:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
MyStruct = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
}

%my_func = func(%2:MyStruct, %3:i32):void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:8:25 error: access: type ptr<MyStruct> cannot be dynamically indexed
    %4:i32 = access %2, %3
                        ^^

:7:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
MyStruct = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
}

%my_func = func(%2:ptr<private, MyStruct, read_write>, %3:i32):void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:3:14 error: access: result of access chain is type f32 but instruction type is i32
    %3:i32 = access %2, 1u, 1u
             ^^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func(%2:mat3x2<f32>):void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().reason.str(),
        R"(:3:40 error: access: result of access chain is type ptr<f32> but instruction type is ptr<i32>
    %3:ptr<private, i32, read_write> = access %2, 1u, 1u
                                       ^^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func(%2:ptr<private, array<array<f32, 2>, 3>, read_write>):void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().reason.str(),
        R"(:3:14 error: access: result of access chain is type ptr<f32> but instruction type is f32
    %3:f32 = access %2, 1u, 1u
             ^^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func(%2:ptr<private, array<array<f32, 2>, 3>, read_write>):void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:3:25 error: access: cannot obtain address of vector element
    %3:f32 = access %2, 1u
                        ^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func(%2:ptr<private, vec3<f32>, read_write>):void -> %b1 {
  %b1 = block {
    %3:f32 = access %2, 1u
    ret
  }
}
)");
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:3:29 error: access: cannot obtain address of vector element
    %3:f32 = access %2, 1u, 1u
                            ^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func(%2:ptr<private, mat3x2<f32>, read_write>):void -> %b1 {
  %b1 = block {
    %3:f32 = access %2, 1u, 1u
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
    ASSERT_TRUE(res) << res.Failure().reason.str();
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
    ASSERT_TRUE(res) << res.Failure().reason.str();
}

TEST_F(IR_ValidatorTest, Block_TerminatorInMiddle) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Return(f);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:3:5 error: block: terminator which isn't the final instruction
    ret
    ^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().reason.str();
}

TEST_F(IR_ValidatorTest, If_EmptyTrue) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(true);
    if_->False()->Append(b.Return(f));

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:4:7 error: block: does not end in a terminator instruction
      %b2 = block {  # true
      ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    if true [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
      }
      %b3 = block {  # false
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:8 error: if: condition must be a `bool` type
    if 1i [t: %b2, f: %b3] {  # if_1
       ^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    if 1i [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        ret
      }
      %b3 = block {  # false
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:8 error: if: operand is undefined
    if undef [t: %b2, f: %b3] {  # if_1
       ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    if undef [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        ret
      }
      %b3 = block {  # false
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:5 error: if: instruction result is undefined
    undef = if true [t: %b2, f: %b3] {  # if_1
    ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    undef = if true [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        ret
      }
      %b3 = block {  # false
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

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().reason.str();
}

TEST_F(IR_ValidatorTest, Loop_EmptyBody) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(b.Loop());
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:4:7 error: block: does not end in a terminator instruction
      %b2 = block {  # body
      ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    loop [b: %b2] {  # loop_1
      %b2 = block {  # body
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_RootBlock_NullResult) {
    auto* v = mod.instructions.Create<ir::Var>(nullptr);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:2:3 error: var: instruction result is undefined
  undef = var
  ^^^^^

:1:1 note: In block
%b1 = block {  # root
^^^^^^^^^^^

note: # Disassembly
%b1 = block {  # root
  undef = var
}

)");
}

TEST_F(IR_ValidatorTest, Var_Function_NullResult) {
    auto* v = mod.instructions.Create<ir::Var>(nullptr);

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:5 error: var: instruction result is undefined
    undef = var
    ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    undef = var
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Init_WrongType) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    sb.Return(f);

    auto* result = sb.InstructionResult(ty.i32());
    v->SetInitializer(result);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:41 error: var: initializer has incorrect type
    %2:ptr<function, f32, read_write> = var, %3
                                        ^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    %2:ptr<function, f32, read_write> = var, %3
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Let_NullResult) {
    auto* v = mod.instructions.Create<ir::Let>(nullptr, b.Constant(1_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:5 error: let: instruction result is undefined
    undef = let 1i
    ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    undef = let 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Let_NullValue) {
    auto* v = mod.instructions.Create<ir::Let>(b.InstructionResult(ty.f32()), nullptr);

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:18 error: let: operand is undefined
    %2:f32 = let undef
                 ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    %2:f32 = let undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Let_WrongType) {
    auto* v = mod.instructions.Create<ir::Let>(b.InstructionResult(ty.f32()), b.Constant(1_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:14 error: let: result type does not match value type
    %2:f32 = let 1i
             ^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    <destroyed tint::core::ir::Var $ADDRESS>
    ret
  }
}
)";

    expected = tint::ReplaceAll(expected, "$ADDRESS", addr);
    expected = tint::ReplaceAll(expected, "$ARROWS", arrows);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), expected);
}

TEST_F(IR_ValidatorTest, Instruction_NullSource) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    sb.Return(f);

    v->Result()->SetSource(nullptr);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:5 error: var: instruction result source is undefined
    %2:ptr<function, f32, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:46 error: var: instruction operand 0 is not alive
    %2:ptr<function, f32, read_write> = var, %3
                                             ^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:46 error: var: instruction operand 0 missing usage
    %2:ptr<function, f32, read_write> = var, %3
                                             ^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(error: orphaned instruction: load
note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:18 error: binary: operand is undefined
    %2:i32 = add undef, 2i
                 ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:22 error: binary: operand is undefined
    %2:i32 = add 2i, undef
                     ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    %2:i32 = add 2i, undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Binary_Result_Nullptr) {
    auto* bin = mod.instructions.Create<ir::Binary>(nullptr, BinaryOp::kAdd, b.Constant(3_i),
                                                    b.Constant(2_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(bin);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:5 error: binary: instruction result is undefined
    undef = add 3i, 2i
    ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:23 error: unary: operand is undefined
    %2:i32 = negation undef
                      ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    %2:i32 = negation undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Unary_Result_Nullptr) {
    auto* bin =
        mod.instructions.Create<ir::Unary>(nullptr, ir::UnaryOp::kNegation, b.Constant(2_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(bin);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:5 error: unary: instruction result is undefined
    undef = negation 2i
    ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:5 error: unary: result type must match value type
    %2:f32 = complement 2i
    ^^^^^^^^^^^^^^^^^^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().reason.str();
}

TEST_F(IR_ValidatorTest, ExitIf_NullIf) {
    auto* if_ = b.If(true);
    if_->True()->Append(mod.instructions.Create<ExitIf>(nullptr));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:5:9 error: exit_if: has no parent control instruction
        exit_if  # undef
        ^^^^^^^

:4:7 note: In block
      %b2 = block {  # true
      ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    if true [t: %b2] {  # if_1
      %b2 = block {  # true
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
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().reason.str(),
        R"(:5:9 error: exit_if: args count (1) does not match control instruction result count (2)
        exit_if 1i  # if_1
        ^^^^^^^^^^

:4:7 note: In block
      %b2 = block {  # true
      ^^^^^^^^^^^

:3:5 note: control instruction
    %2:i32, %3:f32 = if true [t: %b2] {  # if_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    %2:i32, %3:f32 = if true [t: %b2] {  # if_1
      %b2 = block {  # true
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
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().reason.str(),
        R"(:5:9 error: exit_if: args count (3) does not match control instruction result count (2)
        exit_if 1i, 2.0f, 3i  # if_1
        ^^^^^^^^^^^^^^^^^^^^

:4:7 note: In block
      %b2 = block {  # true
      ^^^^^^^^^^^

:3:5 note: control instruction
    %2:i32, %3:f32 = if true [t: %b2] {  # if_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    %2:i32, %3:f32 = if true [t: %b2] {  # if_1
      %b2 = block {  # true
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
    ASSERT_TRUE(res) << res.Failure().reason.str();
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
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().reason.str(),
        R"(:5:21 error: exit_if: argument type (f32) does not match control instruction type (i32)
        exit_if 1i, 2i  # if_1
                    ^^

:4:7 note: In block
      %b2 = block {  # true
      ^^^^^^^^^^^

:3:5 note: control instruction
    %2:i32, %3:f32 = if true [t: %b2] {  # if_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    %2:i32, %3:f32 = if true [t: %b2] {  # if_1
      %b2 = block {  # true
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:8:5 error: exit_if: found outside all control instructions
    exit_if  # if_1
    ^^^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    if true [t: %b2] {  # if_1
      %b2 = block {  # true
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:7:13 error: exit_if: if target jumps over other control instructions
            exit_if  # if_1
            ^^^^^^^

:6:11 note: In block
          %b3 = block {  # true
          ^^^^^^^^^^^

:5:9 note: first control instruction jumped
        if true [t: %b3] {  # if_2
        ^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    if true [t: %b2] {  # if_1
      %b2 = block {  # true
        if true [t: %b3] {  # if_2
          %b3 = block {  # true
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

    auto* c = b.Case(switch_inner, {Switch::CaseSelector{b.Constant(1_i)}});
    b.Append(c, [&] { b.ExitIf(if_outer); });

    b.Append(f->Block(), [&] {
        b.Append(if_outer);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:7:13 error: exit_if: if target jumps over other control instructions
            exit_if  # if_1
            ^^^^^^^

:6:11 note: In block
          %b3 = block {  # case
          ^^^^^^^^^^^

:5:9 note: first control instruction jumped
        switch 1i [c: (1i, %b3)] {  # switch_1
        ^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    if true [t: %b2] {  # if_1
      %b2 = block {  # true
        switch 1i [c: (1i, %b3)] {  # switch_1
          %b3 = block {  # case
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:7:13 error: exit_if: if target jumps over other control instructions
            exit_if  # if_1
            ^^^^^^^

:6:11 note: In block
          %b3 = block {  # body
          ^^^^^^^^^^^

:5:9 note: first control instruction jumped
        loop [b: %b3] {  # loop_1
        ^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    if true [t: %b2] {  # if_1
      %b2 = block {  # true
        loop [b: %b3] {  # loop_1
          %b3 = block {  # body
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

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
    def->Append(b.ExitSwitch(switch_));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().reason.str();
}

TEST_F(IR_ValidatorTest, ExitSwitch_NullSwitch) {
    auto* switch_ = b.Switch(true);

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
    def->Append(mod.instructions.Create<ExitSwitch>(nullptr));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:5:9 error: exit_switch: has no parent control instruction
        exit_switch  # undef
        ^^^^^^^^^^^

:4:7 note: In block
      %b2 = block {  # case
      ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    switch true [c: (default, %b2)] {  # switch_1
      %b2 = block {  # case
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

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
    def->Append(b.ExitSwitch(switch_, 1_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().reason.str(),
        R"(:5:9 error: exit_switch: args count (1) does not match control instruction result count (2)
        exit_switch 1i  # switch_1
        ^^^^^^^^^^^^^^

:4:7 note: In block
      %b2 = block {  # case
      ^^^^^^^^^^^

:3:5 note: control instruction
    %2:i32, %3:f32 = switch true [c: (default, %b2)] {  # switch_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    %2:i32, %3:f32 = switch true [c: (default, %b2)] {  # switch_1
      %b2 = block {  # case
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

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
    def->Append(b.ExitSwitch(switch_, 1_i, 2_f, 3_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().reason.str(),
        R"(:5:9 error: exit_switch: args count (3) does not match control instruction result count (2)
        exit_switch 1i, 2.0f, 3i  # switch_1
        ^^^^^^^^^^^^^^^^^^^^^^^^

:4:7 note: In block
      %b2 = block {  # case
      ^^^^^^^^^^^

:3:5 note: control instruction
    %2:i32, %3:f32 = switch true [c: (default, %b2)] {  # switch_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    %2:i32, %3:f32 = switch true [c: (default, %b2)] {  # switch_1
      %b2 = block {  # case
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

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
    def->Append(b.ExitSwitch(switch_, 1_i, 2_f));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_TRUE(res) << res.Failure().reason.str();
}

TEST_F(IR_ValidatorTest, ExitSwitch_IncorrectResultType) {
    auto* switch_ = b.Switch(true);
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    switch_->SetResults(Vector{r1, r2});

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
    def->Append(b.ExitSwitch(switch_, 1_i, 2_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().reason.str(),
        R"(:5:25 error: exit_switch: argument type (f32) does not match control instruction type (i32)
        exit_switch 1i, 2i  # switch_1
                        ^^

:4:7 note: In block
      %b2 = block {  # case
      ^^^^^^^^^^^

:3:5 note: control instruction
    %2:i32, %3:f32 = switch true [c: (default, %b2)] {  # switch_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    %2:i32, %3:f32 = switch true [c: (default, %b2)] {  # switch_1
      %b2 = block {  # case
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

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
    def->Append(b.Return(f));

    auto sb = b.Append(f->Block());
    sb.Append(switch_);

    auto* if_ = sb.Append(b.If(true));
    b.Append(if_->True(), [&] { b.ExitSwitch(switch_); });
    sb.Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:10:9 error: exit_switch: switch not found in parent control instructions
        exit_switch  # switch_1
        ^^^^^^^^^^^

:9:7 note: In block
      %b3 = block {  # true
      ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    switch true [c: (default, %b2)] {  # switch_1
      %b2 = block {  # case
        ret
      }
    }
    if true [t: %b3] {  # if_1
      %b3 = block {  # true
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

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
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

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().reason.str();
}

TEST_F(IR_ValidatorTest, ExitSwitch_InvalidJumpOverSwitch) {
    auto* switch_ = b.Switch(true);

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
    b.Append(def, [&] {
        auto* inner = b.Switch(false);
        b.ExitSwitch(switch_);

        auto* inner_def = b.Case(inner, {Switch::CaseSelector{}});
        b.Append(inner_def, [&] { b.ExitSwitch(switch_); });
    });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(switch_);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:7:13 error: exit_switch: switch target jumps over other control instructions
            exit_switch  # switch_1
            ^^^^^^^^^^^

:6:11 note: In block
          %b3 = block {  # case
          ^^^^^^^^^^^

:5:9 note: first control instruction jumped
        switch false [c: (default, %b3)] {  # switch_2
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    switch true [c: (default, %b2)] {  # switch_1
      %b2 = block {  # case
        switch false [c: (default, %b3)] {  # switch_2
          %b3 = block {  # case
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

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:7:13 error: exit_switch: switch target jumps over other control instructions
            exit_switch  # switch_1
            ^^^^^^^^^^^

:6:11 note: In block
          %b3 = block {  # body
          ^^^^^^^^^^^

:5:9 note: first control instruction jumped
        loop [b: %b3] {  # loop_1
        ^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    switch true [c: (default, %b2)] {  # switch_1
      %b2 = block {  # case
        loop [b: %b3] {  # loop_1
          %b3 = block {  # body
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

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().reason.str();
}

TEST_F(IR_ValidatorTest, ExitLoop_NullLoop) {
    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(mod.instructions.Create<ExitLoop>(nullptr));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:5:9 error: exit_loop: has no parent control instruction
        exit_loop  # undef
        ^^^^^^^^^

:4:7 note: In block
      %b2 = block {  # body
      ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        exit_loop  # undef
      }
      %b3 = block {  # continuing
        next_iteration %b2
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
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().reason.str(),
        R"(:5:9 error: exit_loop: args count (1) does not match control instruction result count (2)
        exit_loop 1i  # loop_1
        ^^^^^^^^^^^^

:4:7 note: In block
      %b2 = block {  # body
      ^^^^^^^^^^^

:3:5 note: control instruction
    %2:i32, %3:f32 = loop [b: %b2, c: %b3] {  # loop_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    %2:i32, %3:f32 = loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        exit_loop 1i  # loop_1
      }
      %b3 = block {  # continuing
        next_iteration %b2
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
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().reason.str(),
        R"(:5:9 error: exit_loop: args count (3) does not match control instruction result count (2)
        exit_loop 1i, 2.0f, 3i  # loop_1
        ^^^^^^^^^^^^^^^^^^^^^^

:4:7 note: In block
      %b2 = block {  # body
      ^^^^^^^^^^^

:3:5 note: control instruction
    %2:i32, %3:f32 = loop [b: %b2, c: %b3] {  # loop_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    %2:i32, %3:f32 = loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        exit_loop 1i, 2.0f, 3i  # loop_1
      }
      %b3 = block {  # continuing
        next_iteration %b2
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
    ASSERT_TRUE(res) << res.Failure().reason.str();
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
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().reason.str(),
        R"(:5:23 error: exit_loop: argument type (f32) does not match control instruction type (i32)
        exit_loop 1i, 2i  # loop_1
                      ^^

:4:7 note: In block
      %b2 = block {  # body
      ^^^^^^^^^^^

:3:5 note: control instruction
    %2:i32, %3:f32 = loop [b: %b2, c: %b3] {  # loop_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    %2:i32, %3:f32 = loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        exit_loop 1i, 2i  # loop_1
      }
      %b3 = block {  # continuing
        next_iteration %b2
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:13:9 error: exit_loop: loop not found in parent control instructions
        exit_loop  # loop_1
        ^^^^^^^^^

:12:7 note: In block
      %b4 = block {  # true
      ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        ret
      }
      %b3 = block {  # continuing
        next_iteration %b2
      }
    }
    if true [t: %b4] {  # if_1
      %b4 = block {  # true
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

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().reason.str();
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidJumpOverSwitch) {
    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));

    b.Append(loop->Body(), [&] {
        auto* inner = b.Switch(false);
        b.ExitLoop(loop);

        auto* inner_def = b.Case(inner, {Switch::CaseSelector{}});
        b.Append(inner_def, [&] { b.ExitLoop(loop); });
    });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:7:13 error: exit_loop: loop target jumps over other control instructions
            exit_loop  # loop_1
            ^^^^^^^^^

:6:11 note: In block
          %b4 = block {  # case
          ^^^^^^^^^^^

:5:9 note: first control instruction jumped
        switch false [c: (default, %b4)] {  # switch_1
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        switch false [c: (default, %b4)] {  # switch_1
          %b4 = block {  # case
            exit_loop  # loop_1
          }
        }
        exit_loop  # loop_1
      }
      %b3 = block {  # continuing
        next_iteration %b2
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:7:13 error: exit_loop: loop target jumps over other control instructions
            exit_loop  # loop_1
            ^^^^^^^^^

:6:11 note: In block
          %b4 = block {  # body
          ^^^^^^^^^^^

:5:9 note: first control instruction jumped
        loop [b: %b4] {  # loop_2
        ^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        loop [b: %b4] {  # loop_2
          %b4 = block {  # body
            exit_loop  # loop_1
          }
        }
        exit_loop  # loop_1
      }
      %b3 = block {  # continuing
        next_iteration %b2
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:8:9 error: exit_loop: loop exit jumps out of continuing block
        exit_loop  # loop_1
        ^^^^^^^^^

:7:7 note: In block
      %b3 = block {  # continuing
      ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        continue %b3
      }
      %b3 = block {  # continuing
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:10:13 error: exit_loop: loop exit jumps out of continuing block
            exit_loop  # loop_1
            ^^^^^^^^^

:9:11 note: In block
          %b4 = block {  # true
          ^^^^^^^^^^^

:7:7 note: in continuing block
      %b3 = block {  # continuing
      ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        continue %b3
      }
      %b3 = block {  # continuing
        if true [t: %b4] {  # if_1
          %b4 = block {  # true
            exit_loop  # loop_1
          }
        }
        next_iteration %b2
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:5:9 error: exit_loop: loop exit not permitted in loop initializer
        exit_loop  # loop_1
        ^^^^^^^^^

:4:7 note: In block
      %b2 = block {  # initializer
      ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    loop [i: %b2, b: %b3, c: %b4] {  # loop_1
      %b2 = block {  # initializer
        exit_loop  # loop_1
      }
      %b3 = block {  # body
        continue %b4
      }
      %b4 = block {  # continuing
        next_iteration %b3
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:7:13 error: exit_loop: loop exit not permitted in loop initializer
            exit_loop  # loop_1
            ^^^^^^^^^

:6:11 note: In block
          %b5 = block {  # true
          ^^^^^^^^^^^

:4:7 note: in initializer block
      %b2 = block {  # initializer
      ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    loop [i: %b2, b: %b3, c: %b4] {  # loop_1
      %b2 = block {  # initializer
        if true [t: %b5] {  # if_1
          %b5 = block {  # true
            exit_loop  # loop_1
          }
        }
        next_iteration %b3
      }
      %b3 = block {  # body
        continue %b4
      }
      %b4 = block {  # continuing
        next_iteration %b3
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

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().reason.str();
}

TEST_F(IR_ValidatorTest, Return_WithValue) {
    auto* f = b.Function("my_func", ty.i32());
    b.Append(f->Block(), [&] {  //
        b.Return(f, 42_i);
    });

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().reason.str();
}

TEST_F(IR_ValidatorTest, Return_NullFunction) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {  //
        b.Return(nullptr);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:5 error: return: undefined function
    ret
    ^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:5 error: return: unexpected return value
    ret 42i
    ^^^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:5 error: return: expected return value
    ret
    ^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():i32 -> %b1 {
  %b1 = block {
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
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:3:5 error: return: return value type does not match function return type
    ret 42.0f
    ^^^^^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():i32 -> %b1 {
  %b1 = block {
    ret 42.0f
  }
}
)");
}

TEST_F(IR_ValidatorTest, Store_NullTo) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(mod.instructions.Create<ir::Store>(nullptr, b.Constant(42_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:11 error: store: operand is undefined
    store undef, 42i
          ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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
        b.Append(mod.instructions.Create<ir::Store>(var->Result(), nullptr));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:4:15 error: store: operand is undefined
    store %2, undef
              ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    %2:ptr<function, i32, read_write> = var
    store %2, undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Store_TypeMismatch) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Append(mod.instructions.Create<ir::Store>(var->Result(), b.Constant(42_u)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:4:15 error: value type does not match pointer element type
    store %2, 42u
              ^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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
        b.Append(mod.instructions.Create<ir::LoadVectorElement>(nullptr, var->Result(),
                                                                b.Constant(1_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(),
              R"(:4:5 error: load_vector_element: instruction result is undefined
    undef = load_vector_element %2, 1i
    ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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
        b.Append(mod.instructions.Create<ir::LoadVectorElement>(b.InstructionResult(ty.f32()),
                                                                nullptr, b.Constant(1_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:34 error: load_vector_element: operand is undefined
    %2:f32 = load_vector_element undef, 1i
                                 ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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
        b.Append(mod.instructions.Create<ir::LoadVectorElement>(b.InstructionResult(ty.f32()),
                                                                var->Result(), nullptr));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:4:38 error: load_vector_element: operand is undefined
    %3:f32 = load_vector_element %2, undef
                                     ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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
        b.Append(mod.instructions.Create<ir::StoreVectorElement>(nullptr, b.Constant(1_i),
                                                                 b.Constant(2_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:3:26 error: store_vector_element: operand is undefined
    store_vector_element undef, 1i, 2i
                         ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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
        b.Append(mod.instructions.Create<ir::StoreVectorElement>(var->Result(), nullptr,
                                                                 b.Constant(2_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:4:30 error: store_vector_element: operand is undefined
    store_vector_element %2, undef, 2i
                             ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

:4:37 error: value type does not match vector pointer element type
    store_vector_element %2, undef, 2i
                                    ^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
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
        b.Append(mod.instructions.Create<ir::StoreVectorElement>(var->Result(), b.Constant(1_i),
                                                                 nullptr));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(:4:34 error: store_vector_element: operand is undefined
    store_vector_element %2, 1i, undef
                                 ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    %2:ptr<function, vec3<f32>, read_write> = var
    store_vector_element %2, 1i, undef
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::core::ir
