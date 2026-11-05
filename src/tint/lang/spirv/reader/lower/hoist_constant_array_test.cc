// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/reader/lower/hoist_constant_array.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::spirv::reader::lower {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvReader_HoistConstantArrayTest = core::ir::transform::TransformTest;

TEST_F(SpirvReader_HoistConstantArrayTest, ReplaceExpectedPattern) {
    auto* i = b.FunctionParam("i", ty.i32());
    auto* foo = b.Function("foo", ty.u32());
    foo->SetParams({i});
    b.Append(foo->Block(), [&] {
        auto* v = b.Var<function, array<u32, 3>>("v");
        b.Store(v, b.Composite<array<u32, 3>>(10_u, 20_u, 30_u));
        auto* p = b.Access<ptr<function, u32>>(v, i);
        auto* elem = b.Load(p);
        b.Return(foo, elem);
    });

    auto* src = R"(
%foo = func(%i:i32):u32 {
  $B1: {
    %v:ptr<function, array<u32, 3>, read_write> = var
    store %v, array<u32, 3>(10u, 20u, 30u)
    %4:ptr<function, u32, read_write> = access %v, %i
    %5:u32 = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%i:i32):u32 {
  $B1: {
    %3:u32 = access array<u32, 3>(10u, 20u, 30u), %i
    ret %3
  }
}
)";

    Run(HoistConstantArray);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_HoistConstantArrayTest, ReplaceExpectedPatternMultiDimArray) {
    auto* i = b.FunctionParam("i", ty.i32());
    auto* j = b.FunctionParam("j", ty.i32());
    auto* foo = b.Function("foo", ty.u32());
    foo->SetParams({i, j});
    b.Append(foo->Block(), [&] {
        auto* v = b.Var<function, array<array<u32, 2>, 2>>("v");
        auto* a0 = b.Composite<array<u32, 2>>(10_u, 20_u);
        auto* a1 = b.Composite<array<u32, 2>>(30_u, 40_u);
        b.Store(v, b.Composite<array<array<u32, 2>, 2>>(a0, a1));
        auto* p = b.Access<ptr<function, u32>>(v, i, j);
        auto* elem = b.Load(p);
        b.Return(foo, elem);
    });

    auto* src = R"(
%foo = func(%i:i32, %j:i32):u32 {
  $B1: {
    %v:ptr<function, array<array<u32, 2>, 2>, read_write> = var
    store %v, array<array<u32, 2>, 2>(array<u32, 2>(10u, 20u), array<u32, 2>(30u, 40u))
    %5:ptr<function, u32, read_write> = access %v, %i, %j
    %6:u32 = load %5
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%i:i32, %j:i32):u32 {
  $B1: {
    %4:u32 = access array<array<u32, 2>, 2>(array<u32, 2>(10u, 20u), array<u32, 2>(30u, 40u)), %i, %j
    ret %4
  }
}
)";

    Run(HoistConstantArray);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_HoistConstantArrayTest, LoadLaterInSameBlock) {
    auto* i = b.FunctionParam("i", ty.i32());
    auto* foo = b.Function("foo", ty.u32());
    foo->SetParams({i});
    b.Append(foo->Block(), [&] {
        auto* v = b.Var<function, array<u32, 3>>("v");
        b.Store(v, b.Composite<array<u32, 3>>(10_u, 20_u, 30_u));
        auto* p = b.Access<ptr<function, u32>>(v, i);
        auto* other = b.Let("other", 3_u);
        auto* elem = b.Load(p);
        auto* sum = b.Add(elem, other);
        b.Return(foo, sum);
    });

    auto* src = R"(
%foo = func(%i:i32):u32 {
  $B1: {
    %v:ptr<function, array<u32, 3>, read_write> = var
    store %v, array<u32, 3>(10u, 20u, 30u)
    %4:ptr<function, u32, read_write> = access %v, %i
    %other:u32 = let 3u
    %6:u32 = load %4
    %7:u32 = add %6, %other
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%i:i32):u32 {
  $B1: {
    %3:u32 = access array<u32, 3>(10u, 20u, 30u), %i
    %other:u32 = let 3u
    %5:u32 = add %3, %other
    ret %5
  }
}
)";

    Run(HoistConstantArray);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_HoistConstantArrayTest, LoadInLaterBlock) {
    auto* i = b.FunctionParam("i", ty.i32());
    auto* foo = b.Function("foo", ty.void_());
    foo->SetParams({i});
    b.Append(foo->Block(), [&] {
        auto* v = b.Var<function, array<u32, 3>>("v");
        b.Store(v, b.Composite<array<u32, 3>>(10_u, 20_u, 30_u));
        auto* p = b.Access<ptr<function, u32>>(v, i);
        auto* ifelse = b.If(true);
        b.Append(ifelse->True(), [&] {
            auto* elem = b.Load(p);
            b.Let("x", elem);
            b.ExitIf(ifelse);
        });
        b.Return(foo);
    });

    auto* src = R"(
%foo = func(%i:i32):void {
  $B1: {
    %v:ptr<function, array<u32, 3>, read_write> = var
    store %v, array<u32, 3>(10u, 20u, 30u)
    %4:ptr<function, u32, read_write> = access %v, %i
    if true [t: $B2] {  # if_1
      $B2: {  # true
        %5:u32 = load %4
        %x:u32 = let %5
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%i:i32):void {
  $B1: {
    %3:u32 = access array<u32, 3>(10u, 20u, 30u), %i
    if true [t: $B2] {  # if_1
      $B2: {  # true
        %x:u32 = let %3
        exit_if  # if_1
      }
    }
    ret
  }
}
)";

    Run(HoistConstantArray);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_HoistConstantArrayTest, PatternSplitAcrossBlocks) {
    // We could try harder to match this, but it is not what Glslang
    // generatets.
    auto* i = b.FunctionParam("i", ty.i32());
    auto* foo = b.Function("foo", ty.void_());
    foo->SetParams({i});
    b.Append(foo->Block(), [&] {
        auto* v = b.Var<function, array<u32, 3>>("v");
        b.Store(v, b.Composite<array<u32, 3>>(10_u, 20_u, 30_u));
        auto* ifelse = b.If(true);
        b.Append(ifelse->True(), [&] {
            auto* p = b.Access<ptr<function, u32>>(v, i);
            b.Load(p);
            b.ExitIf(ifelse);
        });
        b.Return(foo);
    });

    auto* src = R"(
%foo = func(%i:i32):void {
  $B1: {
    %v:ptr<function, array<u32, 3>, read_write> = var
    store %v, array<u32, 3>(10u, 20u, 30u)
    if true [t: $B2] {  # if_1
      $B2: {  # true
        %4:ptr<function, u32, read_write> = access %v, %i
        %5:u32 = load %4
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(HoistConstantArray);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_HoistConstantArrayTest, HasInitializer) {
    auto* i = b.FunctionParam("i", ty.i32());
    auto* foo = b.Function("foo", ty.u32());
    foo->SetParams({i});
    b.Append(foo->Block(), [&] {
        auto* v = b.Var("v", b.Zero<array<u32, 3>>());
        b.Store(v, b.Composite<array<u32, 3>>(10_u, 20_u, 30_u));
        auto* p = b.Access<ptr<function, u32>>(v, i);
        auto* elem = b.Load(p);
        b.Return(foo, elem);
    });

    auto* src = R"(
%foo = func(%i:i32):u32 {
  $B1: {
    %v:ptr<function, array<u32, 3>, read_write> = var array<u32, 3>(0u)
    store %v, array<u32, 3>(10u, 20u, 30u)
    %4:ptr<function, u32, read_write> = access %v, %i
    %5:u32 = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(HoistConstantArray);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_HoistConstantArrayTest, VarOtherUse_Ptr) {
    auto* i = b.FunctionParam("i", ty.i32());
    auto* foo = b.Function("foo", ty.u32());
    foo->SetParams({i});
    b.Append(foo->Block(), [&] {
        auto* v = b.Var<function, array<u32, 3>>("v");
        b.Store(v, b.Composite<array<u32, 3>>(10_u, 20_u, 30_u));
        auto* p = b.Access<ptr<function, u32>>(v, i);
        auto* elem = b.Load(p);
        b.Let("other_ptr", v);
        b.Return(foo, elem);
    });

    auto* src = R"(
%foo = func(%i:i32):u32 {
  $B1: {
    %v:ptr<function, array<u32, 3>, read_write> = var
    store %v, array<u32, 3>(10u, 20u, 30u)
    %4:ptr<function, u32, read_write> = access %v, %i
    %5:u32 = load %4
    %other_ptr:ptr<function, array<u32, 3>, read_write> = let %v
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(HoistConstantArray);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_HoistConstantArrayTest, VarOtherUse_Load) {
    auto* i = b.FunctionParam("i", ty.i32());
    auto* foo = b.Function("foo", ty.u32());
    foo->SetParams({i});
    b.Append(foo->Block(), [&] {
        auto* v = b.Var<function, array<u32, 3>>("v");
        b.Store(v, b.Composite<array<u32, 3>>(10_u, 20_u, 30_u));
        auto* p = b.Access<ptr<function, u32>>(v, i);
        auto* elem = b.Load(p);
        b.Load(v);
        b.Return(foo, elem);
    });

    auto* src = R"(
%foo = func(%i:i32):u32 {
  $B1: {
    %v:ptr<function, array<u32, 3>, read_write> = var
    store %v, array<u32, 3>(10u, 20u, 30u)
    %4:ptr<function, u32, read_write> = access %v, %i
    %5:u32 = load %4
    %6:array<u32, 3> = load %v
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(HoistConstantArray);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_HoistConstantArrayTest, VarOtherUse_CallParameter) {
    auto* bar = b.Function("bar", ty.void_());
    auto* param = b.FunctionParam("param", ty.ptr<function, array<u32, 3>>());
    bar->SetParams({param});
    b.Append(bar->Block(), [&] { b.Return(bar); });

    auto* i = b.FunctionParam("i", ty.i32());
    auto* foo = b.Function("foo", ty.u32());
    foo->SetParams({i});
    b.Append(foo->Block(), [&] {
        auto* v = b.Var<function, array<u32, 3>>("v");
        b.Store(v, b.Composite<array<u32, 3>>(10_u, 20_u, 30_u));
        auto* p = b.Access<ptr<function, u32>>(v, i);
        auto* elem = b.Load(p);
        b.Call(bar, v);
        b.Return(foo, elem);
    });

    auto* src = R"(
%bar = func(%param:ptr<function, array<u32, 3>, read_write>):void {
  $B1: {
    ret
  }
}
%foo = func(%i:i32):u32 {
  $B2: {
    %v:ptr<function, array<u32, 3>, read_write> = var
    store %v, array<u32, 3>(10u, 20u, 30u)
    %6:ptr<function, u32, read_write> = access %v, %i
    %7:u32 = load %6
    %8:void = call %bar, %v
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(HoistConstantArray);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_HoistConstantArrayTest, NoVarUses) {
    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        b.Var<function, array<u32, 3>>("v");
        b.Return(foo);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %v:ptr<function, array<u32, 3>, read_write> = var
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(HoistConstantArray);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_HoistConstantArrayTest, NoStores) {
    auto* i = b.FunctionParam("i", ty.i32());
    auto* foo = b.Function("foo", ty.u32());
    foo->SetParams({i});
    b.Append(foo->Block(), [&] {
        auto* v = b.Var<function, array<u32, 3>>("v");
        auto* p = b.Access<ptr<function, u32>>(v, i);
        auto* elem = b.Load(p);
        b.Return(foo, elem);
    });

    auto* src = R"(
%foo = func(%i:i32):u32 {
  $B1: {
    %v:ptr<function, array<u32, 3>, read_write> = var
    %4:ptr<function, u32, read_write> = access %v, %i
    %5:u32 = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(HoistConstantArray);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_HoistConstantArrayTest, TwoStores) {
    auto* i = b.FunctionParam("i", ty.i32());
    auto* foo = b.Function("foo", ty.u32());
    foo->SetParams({i});
    b.Append(foo->Block(), [&] {
        auto* v = b.Var<function, array<u32, 3>>("v");
        b.Store(v, b.Composite<array<u32, 3>>(1_u, 2_u, 3_u));
        b.Store(v, b.Composite<array<u32, 3>>(10_u, 20_u, 30_u));
        auto* p = b.Access<ptr<function, u32>>(v, i);
        auto* elem = b.Load(p);
        b.Return(foo, elem);
    });

    auto* src = R"(
%foo = func(%i:i32):u32 {
  $B1: {
    %v:ptr<function, array<u32, 3>, read_write> = var
    store %v, array<u32, 3>(1u, 2u, 3u)
    store %v, array<u32, 3>(10u, 20u, 30u)
    %4:ptr<function, u32, read_write> = access %v, %i
    %5:u32 = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(HoistConstantArray);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_HoistConstantArrayTest, OneStoreNoAccesses) {
    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        auto* v = b.Var<function, array<u32, 3>>("v");
        b.Store(v, b.Composite<array<u32, 3>>(10_u, 20_u, 30_u));
        b.Return(foo);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %v:ptr<function, array<u32, 3>, read_write> = var
    store %v, array<u32, 3>(10u, 20u, 30u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(HoistConstantArray);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_HoistConstantArrayTest, OneStoreTwoAccesses) {
    auto* i = b.FunctionParam("i", ty.i32());
    auto* foo = b.Function("foo", ty.u32());
    foo->SetParams({i});
    b.Append(foo->Block(), [&] {
        auto* v = b.Var<function, array<u32, 3>>("v");
        b.Store(v, b.Composite<array<u32, 3>>(10_u, 20_u, 30_u));
        auto* p = b.Access<ptr<function, u32>>(v, i);
        auto* elem = b.Load(p);
        b.Access<ptr<function, u32>>(v, i);
        b.Return(foo, elem);
    });

    auto* src = R"(
%foo = func(%i:i32):u32 {
  $B1: {
    %v:ptr<function, array<u32, 3>, read_write> = var
    store %v, array<u32, 3>(10u, 20u, 30u)
    %4:ptr<function, u32, read_write> = access %v, %i
    %5:u32 = load %4
    %6:ptr<function, u32, read_write> = access %v, %i
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(HoistConstantArray);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_HoistConstantArrayTest, OneStoreOneAccessOtherUses_Ptr) {
    auto* i = b.FunctionParam("i", ty.i32());
    auto* foo = b.Function("foo", ty.u32());
    foo->SetParams({i});
    b.Append(foo->Block(), [&] {
        auto* v = b.Var<function, array<u32, 3>>("v");
        b.Store(v, b.Composite<array<u32, 3>>(10_u, 20_u, 30_u));
        auto* p = b.Access<ptr<function, u32>>(v, i);
        auto* elem = b.Load(p);
        b.Let("other_ptr", p);
        b.Return(foo, elem);
    });

    auto* src = R"(
%foo = func(%i:i32):u32 {
  $B1: {
    %v:ptr<function, array<u32, 3>, read_write> = var
    store %v, array<u32, 3>(10u, 20u, 30u)
    %4:ptr<function, u32, read_write> = access %v, %i
    %5:u32 = load %4
    %other_ptr:ptr<function, u32, read_write> = let %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(HoistConstantArray);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_HoistConstantArrayTest, OneStoreOneAccessOtherUses_CallParameter) {
    auto* bar = b.Function("bar", ty.void_());
    auto* param = b.FunctionParam("param", ty.ptr<function, u32>());
    bar->SetParams({param});
    b.Append(bar->Block(), [&] { b.Return(bar); });

    auto* i = b.FunctionParam("i", ty.i32());
    auto* foo = b.Function("foo", ty.u32());
    foo->SetParams({i});
    b.Append(foo->Block(), [&] {
        auto* v = b.Var<function, array<u32, 3>>("v");
        b.Store(v, b.Composite<array<u32, 3>>(10_u, 20_u, 30_u));
        auto* p = b.Access<ptr<function, u32>>(v, i);
        auto* elem = b.Load(p);
        b.Call(bar, p);
        b.Return(foo, elem);
    });

    auto* src = R"(
%bar = func(%param:ptr<function, u32, read_write>):void {
  $B1: {
    ret
  }
}
%foo = func(%i:i32):u32 {
  $B2: {
    %v:ptr<function, array<u32, 3>, read_write> = var
    store %v, array<u32, 3>(10u, 20u, 30u)
    %6:ptr<function, u32, read_write> = access %v, %i
    %7:u32 = load %6
    %8:void = call %bar, %6
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(HoistConstantArray);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_HoistConstantArrayTest, OneStoreOneAccessNoLoads) {
    auto* i = b.FunctionParam("i", ty.i32());
    auto* foo = b.Function("foo", ty.void_());
    foo->SetParams({i});
    b.Append(foo->Block(), [&] {
        auto* v = b.Var<function, array<u32, 3>>("v");
        b.Store(v, b.Composite<array<u32, 3>>(10_u, 20_u, 30_u));
        b.Access<ptr<function, u32>>(v, i);
        b.Return(foo);
    });

    auto* src = R"(
%foo = func(%i:i32):void {
  $B1: {
    %v:ptr<function, array<u32, 3>, read_write> = var
    store %v, array<u32, 3>(10u, 20u, 30u)
    %4:ptr<function, u32, read_write> = access %v, %i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(HoistConstantArray);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_HoistConstantArrayTest, OneStoreOneAccessTwoLoads) {
    auto* i = b.FunctionParam("i", ty.i32());
    auto* foo = b.Function("foo", ty.u32());
    foo->SetParams({i});
    b.Append(foo->Block(), [&] {
        auto* v = b.Var<function, array<u32, 3>>("v");
        b.Store(v, b.Composite<array<u32, 3>>(10_u, 20_u, 30_u));
        auto* p = b.Access<ptr<function, u32>>(v, i);
        auto* elem = b.Load(p);
        b.Load(p);
        b.Return(foo, elem);
    });

    auto* src = R"(
%foo = func(%i:i32):u32 {
  $B1: {
    %v:ptr<function, array<u32, 3>, read_write> = var
    store %v, array<u32, 3>(10u, 20u, 30u)
    %4:ptr<function, u32, read_write> = access %v, %i
    %5:u32 = load %4
    %6:u32 = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(HoistConstantArray);

    EXPECT_EQ(expect, str());
}

}  // namespace

}  // namespace tint::spirv::reader::lower
