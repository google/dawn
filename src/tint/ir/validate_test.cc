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

#include "src/tint/ir/validate.h"
#include "gmock/gmock.h"
#include "src/tint/ir/builder.h"
#include "src/tint/ir/ir_test_helper.h"
#include "src/tint/type/pointer.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_ValidateTest = IRTestHelper;

TEST_F(IR_ValidateTest, RootBlock_Var) {
    mod.root_block = b.CreateRootBlockIfNeeded();
    mod.root_block->Append(b.Declare(mod.Types().pointer(
        mod.Types().i32(), builtin::AddressSpace::kPrivate, builtin::Access::kReadWrite)));
    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().str();
}

TEST_F(IR_ValidateTest, RootBlock_NonVar) {
    auto* l = b.CreateLoop();
    l->Body()->Append(b.Continue(l));

    mod.root_block = b.CreateRootBlockIfNeeded();
    mod.root_block->Append(l);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:3 error: root block: invalid instruction: tint::ir::Loop
  loop [b: %b2]
  ^^^^^^^^^^^^^

:2:1 note: In block
%b1 = block {
^^^^^^^^^^^^^
  loop [b: %b2]
^^^^^^^^^^^^^^^
    # Body block
^^^^^^^^^^^^^^^^
    %b2 = block {
^^^^^^^^^^^^^^^^^
      continue %b3
^^^^^^^^^^^^^^^^^^
    }
^^^^^


}
^

note: # Disassembly
# Root block
%b1 = block {
  loop [b: %b2]
    # Body block
    %b2 = block {
      continue %b3
    }

}

)");
}

TEST_F(IR_ValidateTest, RootBlock_VarBadType) {
    mod.root_block = b.CreateRootBlockIfNeeded();
    mod.root_block->Append(b.Declare(mod.Types().i32()));
    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(),
              R"(:3:12 error: root block: 'var' type is not a pointer: tint::type::I32
  %1:i32 = var
           ^^^

:2:1 note: In block
%b1 = block {
^^^^^^^^^^^^^
  %1:i32 = var
^^^^^^^^^^^^^^
}
^

note: # Disassembly
# Root block
%b1 = block {
  %1:i32 = var
}

)");
}

TEST_F(IR_ValidateTest, Function) {
    auto* f = b.CreateFunction("my_func", mod.Types().void_());
    mod.functions.Push(f);

    f->SetParams(
        utils::Vector{b.FunctionParam(mod.Types().i32()), b.FunctionParam(mod.Types().f32())});
    f->StartTarget()->SetInstructions(utils::Vector{b.Return(f)});
    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure().str();
}

TEST_F(IR_ValidateTest, Block_NoBranchAtEnd) {
    auto* f = b.CreateFunction("my_func", mod.Types().void_());
    mod.functions.Push(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:2:1 error: block: does not end in a branch
  %b1 = block {
^^^^^^^^^^^^^^^
  }
^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
  }
}
)");
}

TEST_F(IR_ValidateTest, Block_BranchInMiddle) {
    auto* f = b.CreateFunction("my_func", mod.Types().void_());
    mod.functions.Push(f);

    f->StartTarget()->SetInstructions(utils::Vector{b.Return(f), b.Return(f)});
    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:5 error: block: branch which isn't the final instruction
    ret
    ^^^

:2:1 note: In block
  %b1 = block {
^^^^^^^^^^^^^^^
    ret
^^^^^^^
    ret
^^^^^^^
  }
^^^

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
    auto* f = b.CreateFunction("my_func", mod.Types().void_());
    mod.functions.Push(f);

    auto* if_ = b.CreateIf(b.Constant(1_i));
    if_->True()->Append(b.Return(f));
    if_->False()->Append(b.Return(f));

    f->StartTarget()->Append(if_);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().str(), R"(:3:8 error: if: condition must be a `bool` type
    if 1i [t: %b2, f: %b3]
       ^^

:2:1 note: In block
  %b1 = block {
^^^^^^^^^^^^^^^
    if 1i [t: %b2, f: %b3]
^^^^^^^^^^^^^^^^^^^^^^^^^^
      # True block
^^^^^^^^^^^^^^^^^^
      %b2 = block {
^^^^^^^^^^^^^^^^^^^
        ret
^^^^^^^^^^^
      }
^^^^^^^


      # False block
^^^^^^^^^^^^^^^^^^^
      %b3 = block {
^^^^^^^^^^^^^^^^^^^
        ret
^^^^^^^^^^^
      }
^^^^^^^


  }
^^^

note: # Disassembly
%my_func = func():void -> %b1 {
  %b1 = block {
    if 1i [t: %b2, f: %b3]
      # True block
      %b2 = block {
        ret
      }

      # False block
      %b3 = block {
        ret
      }

  }
}
)");
}

}  // namespace
}  // namespace tint::ir
