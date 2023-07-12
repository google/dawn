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

#include <string>
#include <utility>

#include "gmock/gmock.h"
#include "src/tint/ir/builder.h"
#include "src/tint/ir/ir_test_helper.h"
#include "src/tint/ir/validate.h"
#include "src/tint/type/array.h"
#include "src/tint/type/matrix.h"
#include "src/tint/type/pointer.h"
#include "src/tint/type/struct.h"
#include "src/tint/utils/string.h"

namespace tint::ir {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

using IR_ValidateTest = IRTestHelper;

TEST_F(IR_ValidateTest, RootBlock_Var) {
    mod.root_block = b.RootBlock();
    mod.root_block->Append(b.Var(ty.ptr<private_, i32>()));
    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().str();
}

TEST_F(IR_ValidateTest, RootBlock_NonVar) {
    auto* l = b.Loop();
    l->Body()->Append(b.Continue(l));

    mod.root_block = b.RootBlock();
    mod.root_block->Append(l);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:2:3 error: root block: invalid instruction: tint::ir::Loop
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

TEST_F(IR_ValidateTest, Function) {
    auto* f = b.Function("my_func", ty.void_());

    f->SetParams({b.FunctionParam(ty.i32()), b.FunctionParam(ty.f32())});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().str();
}

TEST_F(IR_ValidateTest, Function_Duplicate) {
    auto* f = b.Function("my_func", ty.void_());
    // Function would auto-push by the builder, so this adds a duplicate
    mod.functions.Push(f);

    f->SetParams({b.FunctionParam(ty.i32()), b.FunctionParam(ty.f32())});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(error: function 'my_func' added to module multiple times
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

TEST_F(IR_ValidateTest, Block_NoTerminator) {
    b.Function("my_func", ty.void_());

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:2:3 error: block: does not end in a terminator instruction
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
  }
}
)");
}

TEST_F(IR_ValidateTest, Access_NegativeIndex) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.vec3<f32>());
    f->SetParams({obj});

    b.With(f->Block(), [&] {
        b.Access(ty.f32(), obj, -1_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:25 error: access: constant index must be positive, got -1
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

TEST_F(IR_ValidateTest, Access_OOB_Index_Value) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.mat3x2<f32>());
    f->SetParams({obj});

    b.With(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u, 3_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:29 error: access: index out of bounds for type vec2<f32>
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

TEST_F(IR_ValidateTest, Access_OOB_Index_Ptr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, array<array<f32, 2>, 3>>());
    f->SetParams({obj});

    b.With(f->Block(), [&] {
        b.Access(ty.ptr<private_, f32>(), obj, 1_u, 3_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
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

TEST_F(IR_ValidateTest, Access_StaticallyUnindexableType_Value) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.f32());
    f->SetParams({obj});

    b.With(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:25 error: access: type f32 cannot be indexed
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

TEST_F(IR_ValidateTest, Access_StaticallyUnindexableType_Ptr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, f32>());
    f->SetParams({obj});

    b.With(f->Block(), [&] {
        b.Access(ty.ptr<private_, f32>(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:51 error: access: type ptr<f32> cannot be indexed
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

TEST_F(IR_ValidateTest, Access_DynamicallyUnindexableType_Value) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.i32()},
                                                          });

    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(str_ty);
    auto* idx = b.FunctionParam(ty.i32());
    f->SetParams({obj, idx});

    b.With(f->Block(), [&] {
        b.Access(ty.i32(), obj, idx);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
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

TEST_F(IR_ValidateTest, Access_DynamicallyUnindexableType_Ptr) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.i32()},
                                                          });

    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, read_write>(str_ty));
    auto* idx = b.FunctionParam(ty.i32());
    f->SetParams({obj, idx});

    b.With(f->Block(), [&] {
        b.Access(ty.i32(), obj, idx);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
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

TEST_F(IR_ValidateTest, Access_Incorrect_Type_Value_Value) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.mat3x2<f32>());
    f->SetParams({obj});

    b.With(f->Block(), [&] {
        b.Access(ty.i32(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
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

TEST_F(IR_ValidateTest, Access_Incorrect_Type_Ptr_Ptr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, array<array<f32, 2>, 3>>());
    f->SetParams({obj});

    b.With(f->Block(), [&] {
        b.Access(ty.ptr<private_, i32>(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().str(),
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

TEST_F(IR_ValidateTest, Access_Incorrect_Type_Ptr_Value) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, array<array<f32, 2>, 3>>());
    f->SetParams({obj});

    b.With(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().str(),
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

TEST_F(IR_ValidateTest, Access_IndexVectorPtr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, vec3<f32>>());
    f->SetParams({obj});

    b.With(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
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

TEST_F(IR_ValidateTest, Access_IndexVectorPtr_ViaMatrixPtr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, mat3x2<f32>>());
    f->SetParams({obj});

    b.With(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
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

TEST_F(IR_ValidateTest, Access_IndexVector) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.vec3<f32>());
    f->SetParams({obj});

    b.With(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_TRUE(res) << res.Failure().str();
}

TEST_F(IR_ValidateTest, Access_IndexVector_ViaMatrix) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.mat3x2<f32>());
    f->SetParams({obj});

    b.With(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_TRUE(res) << res.Failure().str();
}

TEST_F(IR_ValidateTest, Block_TerminatorInMiddle) {
    auto* f = b.Function("my_func", ty.void_());

    b.With(f->Block(), [&] {
        b.Return(f);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
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

TEST_F(IR_ValidateTest, If_EmptyFalse) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(true);
    if_->True()->Append(b.Return(f));

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().str();
}

TEST_F(IR_ValidateTest, If_EmptyTrue) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(true);
    if_->False()->Append(b.Return(f));

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:4:7 error: block: does not end in a terminator instruction
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

TEST_F(IR_ValidateTest, If_ConditionIsBool) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(1_i);
    if_->True()->Append(b.Return(f));
    if_->False()->Append(b.Return(f));

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:8 error: if: condition must be a `bool` type
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

TEST_F(IR_ValidateTest, If_ConditionIsNullptr) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(nullptr);
    if_->True()->Append(b.Return(f));
    if_->False()->Append(b.Return(f));

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:8 error: if: operand is undefined
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

TEST_F(IR_ValidateTest, If_NullResult) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(true);
    if_->True()->Append(b.Return(f));
    if_->False()->Append(b.Return(f));

    if_->SetResults(utils::Vector<InstructionResult*, 1>{nullptr});

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:5 error: instruction result is undefined
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

TEST_F(IR_ValidateTest, Loop_OnlyBody) {
    auto* f = b.Function("my_func", ty.void_());

    auto* l = b.Loop();
    l->Body()->Append(b.ExitLoop(l));

    auto sb = b.With(f->Block());
    sb.Append(l);
    sb.Return(f);

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().str();
}

TEST_F(IR_ValidateTest, Loop_EmptyBody) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.With(f->Block());
    sb.Append(b.Loop());
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:4:7 error: block: does not end in a terminator instruction
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

TEST_F(IR_ValidateTest, Var_RootBlock_NullResult) {
    auto* v = mod.instructions.Create<ir::Var>(nullptr);
    b.RootBlock()->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:2:3 error: instruction result is undefined
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

TEST_F(IR_ValidateTest, Var_Function_NullResult) {
    auto* v = mod.instructions.Create<ir::Var>(nullptr);

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.With(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:5 error: instruction result is undefined
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

TEST_F(IR_ValidateTest, Var_Init_WrongType) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.With(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    sb.Return(f);

    auto* result = sb.InstructionResult(ty.i32());
    v->SetInitializer(result);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:41 error: var initializer has incorrect type
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

TEST_F(IR_ValidateTest, Let_NullResult) {
    auto* v = mod.instructions.Create<ir::Let>(nullptr, b.Constant(1_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.With(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:5 error: instruction result is undefined
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

TEST_F(IR_ValidateTest, Let_NullValue) {
    auto* v = mod.instructions.Create<ir::Let>(b.InstructionResult(ty.f32()), nullptr);

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.With(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:18 error: let: operand is undefined
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

TEST_F(IR_ValidateTest, Let_WrongType) {
    auto* v = mod.instructions.Create<ir::Let>(b.InstructionResult(ty.f32()), b.Constant(1_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.With(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:14 error: let result type does not match value type
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

TEST_F(IR_ValidateTest, Instruction_AppendedDead) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.With(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    auto* ret = sb.Return(f);

    v->Destroy();
    v->InsertBefore(ret);

    auto addr = utils::ToString(v);
    auto arrows = std::string(addr.length(), '^');

    std::string expected = R"(:3:5 error: destroyed instruction found in instruction list
    <destroyed tint::ir::Var $ADDRESS>
    ^^^^^^^^^^^^^^^^^^^^^^^^^$ARROWS^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    <destroyed tint::ir::Var $ADDRESS>
    ret
  }
}
)";

    expected = utils::ReplaceAll(expected, "$ADDRESS", addr);
    expected = utils::ReplaceAll(expected, "$ARROWS", arrows);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), expected);
}

TEST_F(IR_ValidateTest, Instruction_NullSource) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.With(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    sb.Return(f);

    v->Result()->SetSource(nullptr);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:5 error: instruction result source is undefined
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

TEST_F(IR_ValidateTest, Instruction_DeadOperand) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.With(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    sb.Return(f);

    auto* result = sb.InstructionResult(ty.f32());
    result->Destroy();
    v->SetInitializer(result);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:46 error: instruction has operand which is not alive
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

TEST_F(IR_ValidateTest, Instruction_OperandUsageRemoved) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.With(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    sb.Return(f);

    auto* result = sb.InstructionResult(ty.f32());
    v->SetInitializer(result);
    result->RemoveUsage({v, 0u});

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:46 error: instruction operand missing usage
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

TEST_F(IR_ValidateTest, Binary_LHS_Nullptr) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.With(f->Block());
    sb.Add(ty.i32(), nullptr, sb.Constant(2_i));
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:18 error: binary: operand is undefined
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

TEST_F(IR_ValidateTest, Binary_RHS_Nullptr) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.With(f->Block());
    sb.Add(ty.i32(), sb.Constant(2_i), nullptr);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:22 error: binary: operand is undefined
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

TEST_F(IR_ValidateTest, Binary_Result_Nullptr) {
    auto* bin = mod.instructions.Create<ir::Binary>(nullptr, ir::Binary::Kind::kAdd,
                                                    b.Constant(3_i), b.Constant(2_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.With(f->Block());
    sb.Append(bin);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:5 error: instruction result is undefined
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

TEST_F(IR_ValidateTest, Unary_Value_Nullptr) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.With(f->Block());
    sb.Negation(ty.i32(), nullptr);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:23 error: unary: operand is undefined
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

TEST_F(IR_ValidateTest, Unary_Result_Nullptr) {
    auto* bin =
        mod.instructions.Create<ir::Unary>(nullptr, ir::Unary::Kind::kNegation, b.Constant(2_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.With(f->Block());
    sb.Append(bin);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:5 error: instruction result is undefined
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

TEST_F(IR_ValidateTest, Unary_ResultTypeNotMatchValueType) {
    auto* bin = b.Complement(ty.f32(), 2_i);

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.With(f->Block());
    sb.Append(bin);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:5 error: unary: result type must match value type
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

TEST_F(IR_ValidateTest, ExitIf) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.With(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().str();
}

TEST_F(IR_ValidateTest, ExitIf_NullIf) {
    auto* if_ = b.If(true);
    if_->True()->Append(mod.instructions.Create<ExitIf>(nullptr));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.With(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:5:9 error: exit: has no parent control instruction
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

TEST_F(IR_ValidateTest, ExitIf_LessOperandsThenIfParams) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_, 1_i));

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    if_->SetResults(utils::Vector{r1, r2});

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.With(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().str(),
        R"(:5:9 error: exit: args count (1) does not match control instruction result count (2)
        exit_if 1i  # if_1
        ^^^^^^^^^^

:4:7 note: In block
      %b2 = block {  # true
      ^^^^^^^^^^^

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

TEST_F(IR_ValidateTest, ExitIf_MoreOperandsThenIfParams) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_, 1_i, 2_f, 3_i));

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    if_->SetResults(utils::Vector{r1, r2});

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.With(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().str(),
        R"(:5:9 error: exit: args count (3) does not match control instruction result count (2)
        exit_if 1i, 2.0f, 3i  # if_1
        ^^^^^^^^^^^^^^^^^^^^

:4:7 note: In block
      %b2 = block {  # true
      ^^^^^^^^^^^

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

TEST_F(IR_ValidateTest, ExitIf_WithResult) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_, 1_i, 2_f));

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    if_->SetResults(utils::Vector{r1, r2});

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.With(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_TRUE(res) << res.Failure().str();
}

TEST_F(IR_ValidateTest, ExitIf_IncorrectResultType) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_, 1_i, 2_i));

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    if_->SetResults(utils::Vector{r1, r2});

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.With(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().str(),
        R"(:5:21 error: exit: argument type (f32) does not match control instruction type (i32)
        exit_if 1i, 2i  # if_1
                    ^^

:4:7 note: In block
      %b2 = block {  # true
      ^^^^^^^^^^^

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

TEST_F(IR_ValidateTest, ExitIf_NotInParentIf) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(true);
    if_->True()->Append(b.Return(f));

    auto sb = b.With(f->Block());
    sb.Append(if_);
    sb.ExitIf(if_);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
              R"(:8:5 error: exit: found outside all control instructions
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

TEST_F(IR_ValidateTest, ExitIf_InvalidJumpsOverIf) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_inner = b.If(true);

    auto* if_outer = b.If(true);
    b.With(if_outer->True(), [&] {
        b.Append(if_inner);
        b.ExitIf(if_outer);
    });

    b.With(if_inner->True(), [&] { b.ExitIf(if_outer); });

    b.With(f->Block(), [&] {
        b.Append(if_outer);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
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

TEST_F(IR_ValidateTest, ExitIf_InvalidJumpOverSwitch) {
    auto* f = b.Function("my_func", ty.void_());

    auto* switch_inner = b.Switch(1_i);

    auto* if_outer = b.If(true);
    b.With(if_outer->True(), [&] {
        b.Append(switch_inner);
        b.ExitIf(if_outer);
    });

    auto* c = b.Case(switch_inner, {Switch::CaseSelector{b.Constant(1_i)}});
    b.With(c, [&] { b.ExitIf(if_outer); });

    b.With(f->Block(), [&] {
        b.Append(if_outer);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
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

TEST_F(IR_ValidateTest, ExitIf_InvalidJumpOverLoop) {
    auto* f = b.Function("my_func", ty.void_());

    auto* loop = b.Loop();

    auto* if_outer = b.If(true);
    b.With(if_outer->True(), [&] {
        b.Append(loop);
        b.ExitIf(if_outer);
    });

    b.With(loop->Body(), [&] { b.ExitIf(if_outer); });

    b.With(f->Block(), [&] {
        b.Append(if_outer);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
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

TEST_F(IR_ValidateTest, ExitSwitch) {
    auto* switch_ = b.Switch(true);

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
    def->Append(b.ExitSwitch(switch_));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.With(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().str();
}

TEST_F(IR_ValidateTest, ExitSwitch_NullSwitch) {
    auto* switch_ = b.Switch(true);

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
    def->Append(mod.instructions.Create<ExitSwitch>(nullptr));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.With(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:5:9 error: exit: has no parent control instruction
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

TEST_F(IR_ValidateTest, ExitSwitch_LessOperandsThenSwitchParams) {
    auto* switch_ = b.Switch(true);

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    switch_->SetResults(utils::Vector{r1, r2});

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
    def->Append(b.ExitSwitch(switch_, 1_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.With(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().str(),
        R"(:5:9 error: exit: args count (1) does not match control instruction result count (2)
        exit_switch 1i  # switch_1
        ^^^^^^^^^^^^^^

:4:7 note: In block
      %b2 = block {  # case
      ^^^^^^^^^^^

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

TEST_F(IR_ValidateTest, ExitSwitch_MoreOperandsThenSwitchParams) {
    auto* switch_ = b.Switch(true);
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    switch_->SetResults(utils::Vector{r1, r2});

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
    def->Append(b.ExitSwitch(switch_, 1_i, 2_f, 3_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.With(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().str(),
        R"(:5:9 error: exit: args count (3) does not match control instruction result count (2)
        exit_switch 1i, 2.0f, 3i  # switch_1
        ^^^^^^^^^^^^^^^^^^^^^^^^

:4:7 note: In block
      %b2 = block {  # case
      ^^^^^^^^^^^

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

TEST_F(IR_ValidateTest, ExitSwitch_WithResult) {
    auto* switch_ = b.Switch(true);
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    switch_->SetResults(utils::Vector{r1, r2});

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
    def->Append(b.ExitSwitch(switch_, 1_i, 2_f));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.With(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_TRUE(res) << res.Failure().str();
}

TEST_F(IR_ValidateTest, ExitSwitch_IncorrectResultType) {
    auto* switch_ = b.Switch(true);
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    switch_->SetResults(utils::Vector{r1, r2});

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
    def->Append(b.ExitSwitch(switch_, 1_i, 2_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.With(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().str(),
        R"(:5:25 error: exit: argument type (f32) does not match control instruction type (i32)
        exit_switch 1i, 2i  # switch_1
                        ^^

:4:7 note: In block
      %b2 = block {  # case
      ^^^^^^^^^^^

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

TEST_F(IR_ValidateTest, ExitSwitch_NotInParentSwitch) {
    auto* switch_ = b.Switch(true);

    auto* f = b.Function("my_func", ty.void_());

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
    def->Append(b.Return(f));

    auto sb = b.With(f->Block());
    sb.Append(switch_);

    auto* if_ = sb.Append(b.If(true));
    b.With(if_->True(), [&] { b.ExitSwitch(switch_); });
    sb.Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
              R"(:10:9 error: exit: switch not found in parent control instructions
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

TEST_F(IR_ValidateTest, ExitSwitch_JumpsOverIfs) {
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
    b.With(def, [&] {
        auto* if_ = b.If(true);
        b.With(if_->True(), [&] {
            auto* inner_if_ = b.If(false);
            b.With(inner_if_->True(), [&] { b.ExitSwitch(switch_); });
            b.Return(f);
        });
        b.ExitSwitch(switch_);
    });

    auto sb = b.With(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().str();
}

TEST_F(IR_ValidateTest, ExitSwitch_InvalidJumpOverSwitch) {
    auto* switch_ = b.Switch(true);

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
    b.With(def, [&] {
        auto* inner = b.Switch(false);
        b.ExitSwitch(switch_);

        auto* inner_def = b.Case(inner, {Switch::CaseSelector{}});
        b.With(inner_def, [&] { b.ExitSwitch(switch_); });
    });

    auto* f = b.Function("my_func", ty.void_());

    b.With(f->Block(), [&] {
        b.Append(switch_);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
              R"(:7:13 error: exit: switch target jumps over other control instructions
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

TEST_F(IR_ValidateTest, ExitSwitch_InvalidJumpOverLoop) {
    auto* switch_ = b.Switch(true);

    auto* def = b.Case(switch_, {Switch::CaseSelector{}});
    b.With(def, [&] {
        auto* loop = b.Loop();
        b.With(loop->Body(), [&] { b.ExitSwitch(switch_); });
        b.ExitSwitch(switch_);
    });

    auto* f = b.Function("my_func", ty.void_());

    b.With(f->Block(), [&] {
        b.Append(switch_);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
              R"(:7:13 error: exit: switch target jumps over other control instructions
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

TEST_F(IR_ValidateTest, ExitLoop) {
    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.With(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().str();
}

TEST_F(IR_ValidateTest, ExitLoop_NullLoop) {
    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(mod.instructions.Create<ExitLoop>(nullptr));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.With(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:5:9 error: exit: has no parent control instruction
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

TEST_F(IR_ValidateTest, ExitLoop_LessOperandsThenLoopParams) {
    auto* loop = b.Loop();
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    loop->SetResults(utils::Vector{r1, r2});

    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop, 1_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.With(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().str(),
        R"(:5:9 error: exit: args count (1) does not match control instruction result count (2)
        exit_loop 1i  # loop_1
        ^^^^^^^^^^^^

:4:7 note: In block
      %b2 = block {  # body
      ^^^^^^^^^^^

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

TEST_F(IR_ValidateTest, ExitLoop_MoreOperandsThenLoopParams) {
    auto* loop = b.Loop();
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    loop->SetResults(utils::Vector{r1, r2});

    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop, 1_i, 2_f, 3_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.With(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().str(),
        R"(:5:9 error: exit: args count (3) does not match control instruction result count (2)
        exit_loop 1i, 2.0f, 3i  # loop_1
        ^^^^^^^^^^^^^^^^^^^^^^

:4:7 note: In block
      %b2 = block {  # body
      ^^^^^^^^^^^

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

TEST_F(IR_ValidateTest, ExitLoop_WithResult) {
    auto* loop = b.Loop();
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    loop->SetResults(utils::Vector{r1, r2});

    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop, 1_i, 2_f));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.With(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_TRUE(res) << res.Failure().str();
}

TEST_F(IR_ValidateTest, ExitLoop_IncorrectResultType) {
    auto* loop = b.Loop();
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    loop->SetResults(utils::Vector{r1, r2});

    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop, 1_i, 2_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.With(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(
        res.Failure().str(),
        R"(:5:23 error: exit: argument type (f32) does not match control instruction type (i32)
        exit_loop 1i, 2i  # loop_1
                      ^^

:4:7 note: In block
      %b2 = block {  # body
      ^^^^^^^^^^^

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

TEST_F(IR_ValidateTest, ExitLoop_NotInParentLoop) {
    auto* f = b.Function("my_func", ty.void_());

    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.Return(f));

    auto sb = b.With(f->Block());
    sb.Append(loop);

    auto* if_ = sb.Append(b.If(true));
    b.With(if_->True(), [&] { b.ExitLoop(loop); });
    sb.Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
              R"(:13:9 error: exit: loop not found in parent control instructions
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

TEST_F(IR_ValidateTest, ExitLoop_JumpsOverIfs) {
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

    b.With(loop->Body(), [&] {
        auto* if_ = b.If(true);
        b.With(if_->True(), [&] {
            auto* inner_if_ = b.If(false);
            b.With(inner_if_->True(), [&] { b.ExitLoop(loop); });
            b.Return(f);
        });
        b.ExitLoop(loop);
    });

    auto sb = b.With(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().str();
}

TEST_F(IR_ValidateTest, ExitLoop_InvalidJumpOverSwitch) {
    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));

    b.With(loop->Body(), [&] {
        auto* inner = b.Switch(false);
        b.ExitLoop(loop);

        auto* inner_def = b.Case(inner, {Switch::CaseSelector{}});
        b.With(inner_def, [&] { b.ExitLoop(loop); });
    });

    auto* f = b.Function("my_func", ty.void_());

    b.With(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
              R"(:7:13 error: exit: loop target jumps over other control instructions
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

TEST_F(IR_ValidateTest, ExitLoop_InvalidJumpOverLoop) {
    auto* outer_loop = b.Loop();

    outer_loop->Continuing()->Append(b.NextIteration(outer_loop));

    b.With(outer_loop->Body(), [&] {
        auto* loop = b.Loop();
        b.With(loop->Body(), [&] { b.ExitLoop(outer_loop); });
        b.ExitLoop(outer_loop);
    });

    auto* f = b.Function("my_func", ty.void_());

    b.With(f->Block(), [&] {
        b.Append(outer_loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
              R"(:7:13 error: exit: loop target jumps over other control instructions
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

TEST_F(IR_ValidateTest, ExitLoop_InvalidInsideContinuing) {
    auto* loop = b.Loop();

    loop->Continuing()->Append(b.ExitLoop(loop));
    loop->Body()->Append(b.Continue(loop));

    auto* f = b.Function("my_func", ty.void_());

    b.With(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
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

TEST_F(IR_ValidateTest, ExitLoop_InvalidInsideContinuingNested) {
    auto* loop = b.Loop();

    b.With(loop->Continuing(), [&]() {
        auto* if_ = b.If(true);
        b.With(if_->True(), [&]() { b.ExitLoop(loop); });
        b.NextIteration(loop);
    });

    b.With(loop->Body(), [&] { b.Continue(loop); });

    auto* f = b.Function("my_func", ty.void_());

    b.With(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
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

TEST_F(IR_ValidateTest, ExitLoop_InvalidInsideInitializer) {
    auto* loop = b.Loop();

    loop->Initializer()->Append(b.ExitLoop(loop));
    loop->Continuing()->Append(b.NextIteration(loop));

    b.With(loop->Body(), [&] { b.Continue(loop); });

    auto* f = b.Function("my_func", ty.void_());

    b.With(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
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

TEST_F(IR_ValidateTest, ExitLoop_InvalidInsideInitializerNested) {
    auto* loop = b.Loop();

    b.With(loop->Initializer(), [&]() {
        auto* if_ = b.If(true);
        b.With(if_->True(), [&]() { b.ExitLoop(loop); });
        b.NextIteration(loop);
    });
    loop->Continuing()->Append(b.NextIteration(loop));

    b.With(loop->Body(), [&] { b.Continue(loop); });

    auto* f = b.Function("my_func", ty.void_());

    b.With(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
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

TEST_F(IR_ValidateTest, LoadVectorElement_NullResult) {
    auto* f = b.Function("my_func", ty.void_());

    b.With(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        b.Append(mod.instructions.Create<ir::LoadVectorElement>(nullptr, var->Result(),
                                                                b.Constant(1_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:4:5 error: instruction result is undefined
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

TEST_F(IR_ValidateTest, LoadVectorElement_NullFrom) {
    auto* f = b.Function("my_func", ty.void_());

    b.With(f->Block(), [&] {
        b.Append(mod.instructions.Create<ir::LoadVectorElement>(b.InstructionResult(ty.f32()),
                                                                nullptr, b.Constant(1_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:34 error: load_vector_element: operand is undefined
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

TEST_F(IR_ValidateTest, LoadVectorElement_NullIndex) {
    auto* f = b.Function("my_func", ty.void_());

    b.With(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        b.Append(mod.instructions.Create<ir::LoadVectorElement>(b.InstructionResult(ty.f32()),
                                                                var->Result(), nullptr));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:4:38 error: load_vector_element: operand is undefined
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

TEST_F(IR_ValidateTest, StoreVectorElement_NullTo) {
    auto* f = b.Function("my_func", ty.void_());

    b.With(f->Block(), [&] {
        b.Append(mod.instructions.Create<ir::StoreVectorElement>(nullptr, b.Constant(1_i),
                                                                 b.Constant(2_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:32 error: store_vector_element: operand is undefined
    store_vector_element undef undef, 1i, 2i
                               ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    store_vector_element undef undef, 1i, 2i
    ret
  }
}
)");
}

TEST_F(IR_ValidateTest, StoreVectorElement_NullIndex) {
    auto* f = b.Function("my_func", ty.void_());

    b.With(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        b.Append(mod.instructions.Create<ir::StoreVectorElement>(var->Result(), nullptr,
                                                                 b.Constant(2_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:4:33 error: store_vector_element: operand is undefined
    store_vector_element %2 %2, undef, 2i
                                ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

:4:40 error: value type does not match vector pointer element type
    store_vector_element %2 %2, undef, 2i
                                       ^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    %2:ptr<function, vec3<f32>, read_write> = var
    store_vector_element %2 %2, undef, 2i
    ret
  }
}
)");
}

TEST_F(IR_ValidateTest, StoreVectorElement_NullValue) {
    auto* f = b.Function("my_func", ty.void_());

    b.With(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        b.Append(mod.instructions.Create<ir::StoreVectorElement>(var->Result(), b.Constant(1_i),
                                                                 nullptr));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:4:37 error: store_vector_element: operand is undefined
    store_vector_element %2 %2, 1i, undef
                                    ^^^^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    %2:ptr<function, vec3<f32>, read_write> = var
    store_vector_element %2 %2, 1i, undef
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::ir
