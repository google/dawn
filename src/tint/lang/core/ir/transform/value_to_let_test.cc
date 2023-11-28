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

#include "src/tint/lang/core/ir/transform/value_to_let.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_ValueToLetTest = TransformTest;

TEST_F(IR_ValueToLetTest, Empty) {
    auto* expect = R"(
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ValueToLetTest, NoModify_Blah) {
    auto* func = b.Function("F", ty.void_());
    b.Append(func->Block(), [&] { b.Return(func); });

    auto* src = R"(
%F = func():void -> %b1 {
  %b1 = block {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ValueToLetTest, NoModify_Unsequenced) {
    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 2_i);
        auto* z = b.Let("z", b.Add<i32>(x, y));
        b.Return(fn, z);
    });

    auto* src = R"(
%F = func():i32 -> %b1 {
  %b1 = block {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:i32 = add %x, %y
    %z:i32 = let %4
    ret %z
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}
TEST_F(IR_ValueToLetTest, NoModify_SequencedValueUsedWithNonSequenced) {
    auto* i = b.Var<private_, i32>("i");
    b.ir.root_block->Append(i);

    auto* p = b.FunctionParam<i32>("p");
    auto* rmw = b.Function("rmw", ty.i32());
    rmw->SetParams({p});
    b.Append(rmw->Block(), [&] {
        auto* v = b.Let("v", b.Add<i32>(b.Load(i), p));
        b.Store(i, v);
        b.Return(rmw, v);
    });

    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* x = b.Name("x", b.Call(rmw, 1_i));
        // select is called with one, inlinable sequenced operand and two non-sequenced values.
        auto* y = b.Name("y", b.Call<i32>(core::BuiltinFn::kSelect, 2_i, x, false));
        b.Return(fn, y);
    });

    auto* src = R"(
%b1 = block {  # root
  %i:ptr<private, i32, read_write> = var
}

%rmw = func(%p:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = load %i
    %5:i32 = add %4, %p
    %v:i32 = let %5
    store %i, %v
    ret %v
  }
}
%F = func():i32 -> %b3 {
  %b3 = block {
    %x:i32 = call %rmw, 1i
    %y:i32 = select 2i, %x, false
    ret %y
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ValueToLetTest, NoModify_Inlinable_NestedCalls) {
    auto* i = b.Var<private_, i32>("i");
    b.ir.root_block->Append(i);

    auto* p = b.FunctionParam<i32>("p");
    auto* rmw = b.Function("rmw", ty.i32());
    rmw->SetParams({p});
    b.Append(rmw->Block(), [&] {
        auto* v = b.Let("v", b.Add<i32>(b.Load(i), p));
        b.Store(i, v);
        b.Return(rmw, v);
    });

    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* x = b.Name("x", b.Call(rmw, 1_i));
        auto* y = b.Name("y", b.Call(rmw, x));
        auto* z = b.Name("z", b.Call(rmw, y));
        b.Return(fn, z);
    });

    auto* src = R"(
%b1 = block {  # root
  %i:ptr<private, i32, read_write> = var
}

%rmw = func(%p:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = load %i
    %5:i32 = add %4, %p
    %v:i32 = let %5
    store %i, %v
    ret %v
  }
}
%F = func():i32 -> %b3 {
  %b3 = block {
    %x:i32 = call %rmw, 1i
    %y:i32 = call %rmw, %x
    %z:i32 = call %rmw, %y
    ret %z
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ValueToLetTest, NoModify_LetUsedTwice) {
    auto* i = b.Var<private_, i32>("i");
    b.ir.root_block->Append(i);

    auto* p = b.FunctionParam<i32>("p");
    auto* rmw = b.Function("rmw", ty.i32());
    rmw->SetParams({p});
    b.Append(rmw->Block(), [&] {
        auto* v = b.Let("v", b.Add<i32>(b.Load(i), p));
        b.Store(i, v);
        b.Return(rmw, v);
    });

    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        // No need to create more lets, as these are already in lets
        auto* x = b.Let("x", b.Call(rmw, 1_i));
        auto* y = b.Name("y", b.Add<i32>(x, x));
        b.Return(fn, y);
    });

    auto* src = R"(
%b1 = block {  # root
  %i:ptr<private, i32, read_write> = var
}

%rmw = func(%p:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = load %i
    %5:i32 = add %4, %p
    %v:i32 = let %5
    store %i, %v
    ret %v
  }
}
%F = func():i32 -> %b3 {
  %b3 = block {
    %8:i32 = call %rmw, 1i
    %x:i32 = let %8
    %y:i32 = add %x, %x
    ret %y
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ValueToLetTest, NoModify_VarUsedTwice) {
    auto* p = b.FunctionParam<ptr<function, i32, read_write>>("p");
    auto* fn_g = b.Function("g", ty.i32());
    fn_g->SetParams({p});
    b.Append(fn_g->Block(), [&] { b.Return(fn_g, b.Load(p)); });

    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Var<function, i32>("v");
        auto* x = b.Let("x", b.Call(fn_g, v));
        auto* y = b.Let("y", b.Call(fn_g, v));
        b.Return(fn, b.Add<i32>(x, y));
    });

    auto* src = R"(
%g = func(%p:ptr<function, i32, read_write>):i32 -> %b1 {
  %b1 = block {
    %3:i32 = load %p
    ret %3
  }
}
%F = func():i32 -> %b2 {
  %b2 = block {
    %v:ptr<function, i32, read_write> = var
    %6:i32 = call %g, %v
    %x:i32 = let %6
    %8:i32 = call %g, %v
    %y:i32 = let %8
    %10:i32 = add %x, %y
    ret %10
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ValueToLetTest, VarLoadUsedTwice) {
    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Var<function, i32>("v");
        auto* l = b.Name("l", b.Load(v));
        b.Return(fn, b.Add<i32>(l, l));
    });

    auto* src = R"(
%F = func():i32 -> %b1 {
  %b1 = block {
    %v:ptr<function, i32, read_write> = var
    %l:i32 = load %v
    %4:i32 = add %l, %l
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%F = func():i32 -> %b1 {
  %b1 = block {
    %v:ptr<function, i32, read_write> = var
    %3:i32 = load %v
    %l:i32 = let %3
    %5:i32 = add %l, %l
    ret %5
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ValueToLetTest, VarLoad_ThenStore_ThenUse) {
    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Var<function, i32>("v");
        auto* l = b.Name("l", b.Load(v));
        b.Store(v, 1_i);
        b.Return(fn, l);
    });

    auto* src = R"(
%F = func():i32 -> %b1 {
  %b1 = block {
    %v:ptr<function, i32, read_write> = var
    %l:i32 = load %v
    store %v, 1i
    ret %l
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%F = func():i32 -> %b1 {
  %b1 = block {
    %v:ptr<function, i32, read_write> = var
    %3:i32 = load %v
    %l:i32 = let %3
    store %v, 1i
    ret %l
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ValueToLetTest, TwoCalls_ThenUseReturnValues) {
    auto* i = b.Var<private_, i32>("i");
    b.ir.root_block->Append(i);

    auto* p = b.FunctionParam<i32>("p");
    auto* rmw = b.Function("rmw", ty.i32());
    rmw->SetParams({p});
    b.Append(rmw->Block(), [&] {
        auto* v = b.Let("v", b.Add<i32>(b.Load(i), p));
        b.Store(i, v);
        b.Return(rmw, v);
    });

    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* x = b.Name("x", b.Call(rmw, 1_i));
        auto* y = b.Name("y", b.Call(rmw, 2_i));
        auto* z = b.Name("z", b.Add<i32>(x, y));
        b.Return(fn, z);
    });

    auto* src = R"(
%b1 = block {  # root
  %i:ptr<private, i32, read_write> = var
}

%rmw = func(%p:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = load %i
    %5:i32 = add %4, %p
    %v:i32 = let %5
    store %i, %v
    ret %v
  }
}
%F = func():i32 -> %b3 {
  %b3 = block {
    %x:i32 = call %rmw, 1i
    %y:i32 = call %rmw, 2i
    %z:i32 = add %x, %y
    ret %z
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %i:ptr<private, i32, read_write> = var
}

%rmw = func(%p:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = load %i
    %5:i32 = add %4, %p
    %v:i32 = let %5
    store %i, %v
    ret %v
  }
}
%F = func():i32 -> %b3 {
  %b3 = block {
    %8:i32 = call %rmw, 1i
    %x:i32 = let %8
    %y:i32 = call %rmw, 2i
    %z:i32 = add %x, %y
    ret %z
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());

    Run(ValueToLet);  // running a second time should be no-op

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ValueToLetTest, SequencedUsedInDifferentBlock) {
    auto* i = b.Var<private_, i32>("i");
    b.ir.root_block->Append(i);

    auto* p = b.FunctionParam<i32>("p");
    auto* rmw = b.Function("rmw", ty.i32());
    rmw->SetParams({p});
    b.Append(rmw->Block(), [&] {
        auto* v = b.Let("v", b.Add<i32>(b.Load(i), p));
        b.Store(i, v);
        b.Return(rmw, v);
    });

    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* x = b.Name("x", b.Call(rmw, 1_i));
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {  //
            b.Return(fn, x);
        });
        b.Return(fn, 2_i);
    });

    auto* src = R"(
%b1 = block {  # root
  %i:ptr<private, i32, read_write> = var
}

%rmw = func(%p:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = load %i
    %5:i32 = add %4, %p
    %v:i32 = let %5
    store %i, %v
    ret %v
  }
}
%F = func():i32 -> %b3 {
  %b3 = block {
    %x:i32 = call %rmw, 1i
    if true [t: %b4] {  # if_1
      %b4 = block {  # true
        ret %x
      }
    }
    ret 2i
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %i:ptr<private, i32, read_write> = var
}

%rmw = func(%p:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = load %i
    %5:i32 = add %4, %p
    %v:i32 = let %5
    store %i, %v
    ret %v
  }
}
%F = func():i32 -> %b3 {
  %b3 = block {
    %8:i32 = call %rmw, 1i
    %x:i32 = let %8
    if true [t: %b4] {  # if_1
      %b4 = block {  # true
        ret %x
      }
    }
    ret 2i
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());

    Run(ValueToLet);  // running a second time should be no-op

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ValueToLetTest, NameMe1) {
    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Var<function, i32>("v");
        auto* x = b.Load(v);
        auto* y = b.Add<i32>(x, 1_i);
        b.Store(v, 2_i);
        b.Return(fn, y);
    });

    auto* src = R"(
%F = func():i32 -> %b1 {
  %b1 = block {
    %v:ptr<function, i32, read_write> = var
    %3:i32 = load %v
    %4:i32 = add %3, 1i
    store %v, 2i
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%F = func():i32 -> %b1 {
  %b1 = block {
    %v:ptr<function, i32, read_write> = var
    %3:i32 = load %v
    %4:i32 = add %3, 1i
    %5:i32 = let %4
    store %v, 2i
    ret %5
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());

    Run(ValueToLet);  // running a second time should be no-op

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ValueToLetTest, NameMe2) {
    auto* fn = b.Function("F", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* i = b.Name("i", b.Call<i32>(core::BuiltinFn::kMax, 1_i, 2_i));
        auto* v = b.Var<function>("v", i);
        auto* x = b.Name("x", b.Call<i32>(core::BuiltinFn::kMax, 3_i, 4_i));
        auto* y = b.Name("y", b.Load(v));
        auto* z = b.Name("z", b.Add<i32>(y, x));
        b.Store(v, z);
        b.Return(fn);
    });

    auto* src = R"(
%F = func():void -> %b1 {
  %b1 = block {
    %i:i32 = max 1i, 2i
    %v:ptr<function, i32, read_write> = var, %i
    %x:i32 = max 3i, 4i
    %y:i32 = load %v
    %z:i32 = add %y, %x
    store %v, %z
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%F = func():void -> %b1 {
  %b1 = block {
    %i:i32 = max 1i, 2i
    %v:ptr<function, i32, read_write> = var, %i
    %x:i32 = max 3i, 4i
    %y:i32 = load %v
    %z:i32 = add %y, %x
    store %v, %z
    ret
  }
}
)";

    Run(ValueToLet);

    EXPECT_EQ(expect, str());

    Run(ValueToLet);  // running a second time should be no-op

    EXPECT_EQ(expect, str());
}
}  // namespace
}  // namespace tint::core::ir::transform
