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

TEST_F(IR_ValidateTest, Valid_Access_Value) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.mat3x2<f32>());
    f->SetParams({obj});

    b.With(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u, 0_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().str();
}

TEST_F(IR_ValidateTest, Valid_Access_Ptr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, mat3x2<f32>>());
    f->SetParams({obj});

    b.With(f->Block(), [&] {
        b.Access(ty.ptr<private_, f32>(), obj, 1_u, 0_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().str();
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
    auto* obj = b.FunctionParam(ty.ptr<private_, mat3x2<f32>>());
    f->SetParams({obj});

    b.With(f->Block(), [&] {
        b.Access(ty.ptr<private_, f32>(), obj, 1_u, 3_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
              R"(:3:55 error: access: index out of bounds for type ptr<vec2<f32>>
    %3:ptr<private, f32, read_write> = access %2, 1u, 3u
                                                      ^^

:2:3 note: In block
  %b1 = block {
  ^^^^^^^^^^^

:3:55 note: acceptable range: [0..1]
    %3:ptr<private, f32, read_write> = access %2, 1u, 3u
                                                      ^^

note: # Disassembly
%my_func = func(%2:ptr<private, mat3x2<f32>, read_write>):void -> %b1 {
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
    auto* obj = b.FunctionParam(ty.ptr<private_, mat3x2<f32>>());
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
%my_func = func(%2:ptr<private, mat3x2<f32>, read_write>):void -> %b1 {
  %b1 = block {
    %3:ptr<private, i32, read_write> = access %2, 1u, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidateTest, Access_Incorrect_Type_Ptr_Value) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, mat3x2<f32>>());
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
%my_func = func(%2:ptr<private, mat3x2<f32>, read_write>):void -> %b1 {
  %b1 = block {
    %3:f32 = access %2, 1u, 1u
    ret
  }
}
)");
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
    EXPECT_EQ(res.Failure().str(), R"(:3:8 error: if: condition operand is undefined
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
    EXPECT_EQ(res.Failure().str(), R"(:3:18 error: binary: left operand is undefined
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
    EXPECT_EQ(res.Failure().str(), R"(:3:22 error: binary: right operand is undefined
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
    EXPECT_EQ(res.Failure().str(), R"(:3:23 error: unary: value operand is undefined
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

}  // namespace
}  // namespace tint::ir
