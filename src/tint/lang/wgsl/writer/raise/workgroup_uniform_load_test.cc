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

#include "src/tint/lang/wgsl/writer/raise/workgroup_uniform_load.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/storage_texture.h"

namespace tint::wgsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class WgslWriter_WorkgroupUniformLoadTest : public core::ir::transform::TransformTest {
  protected:
    void Barrier() { b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier); }
};

TEST_F(WgslWriter_WorkgroupUniformLoadTest, BarrierLoadBarrier) {
    auto* w = b.Var<workgroup, i32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* load = b.Load(w);
        Barrier();
        b.Return(f, load);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, i32, read_write> = var
}

%f = func():i32 {
  $B2: {
    %3:void = workgroupBarrier
    %4:i32 = load %w
    %5:void = workgroupBarrier
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, i32, read_write> = var
}

%f = func():i32 {
  $B2: {
    %3:i32 = wgsl.workgroupUniformLoad %w
    ret %3
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_WorkgroupUniformLoadTest, BarrierAtomicLoadBarrier) {
    auto* w = b.Var<workgroup, atomic<u32>, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.u32());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* load = b.Call(ty.u32(), core::BuiltinFn::kAtomicLoad, w);
        Barrier();
        b.Return(f, load);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, atomic<u32>, read_write> = var
}

%f = func():u32 {
  $B2: {
    %3:void = workgroupBarrier
    %4:u32 = atomicLoad %w
    %5:void = workgroupBarrier
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, atomic<u32>, read_write> = var
}

%f = func():u32 {
  $B2: {
    %3:u32 = wgsl.workgroupUniformLoad %w
    ret %3
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// The value computed from the load sits between the load and the second barrier.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, ValuesBeforeSecondBarrier) {
    auto* w = b.Var<workgroup, i32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.bool_());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* load = b.Load(w);
        auto* cond = b.NotEqual(load, 0_i);
        Barrier();
        b.Return(f, cond);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, i32, read_write> = var
}

%f = func():bool {
  $B2: {
    %3:void = workgroupBarrier
    %4:i32 = load %w
    %5:bool = neq %4, 0i
    %6:void = workgroupBarrier
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, i32, read_write> = var
}

%f = func():bool {
  $B2: {
    %3:i32 = wgsl.workgroupUniformLoad %w
    %4:bool = neq %3, 0i
    ret %4
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// The pointer to the array element is made between the first barrier and the load.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, ConstantIndex) {
    auto* w = b.Var<workgroup, array<u32, 4>, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.u32());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* load = b.Load(b.Access<ptr<workgroup, u32, read_write>>(w, 2_u));
        Barrier();
        b.Return(f, load);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, array<u32, 4>, read_write> = var
}

%f = func():u32 {
  $B2: {
    %3:void = workgroupBarrier
    %4:ptr<workgroup, u32, read_write> = access %w, 2u
    %5:u32 = load %4
    %6:void = workgroupBarrier
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, array<u32, 4>, read_write> = var
}

%f = func():u32 {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %w, 2u
    %4:u32 = wgsl.workgroupUniformLoad %3
    ret %4
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// workgroupUniformLoad requires a uniform pointer, which a parameter may not be.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, NonConstantIndex_NoChange) {
    auto* w = b.Var<workgroup, array<u32, 4>, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.u32());
    auto* idx = b.FunctionParam<u32>("idx");
    f->SetParams({idx});
    b.Append(f->Block(), [&] {
        Barrier();
        auto* load = b.Load(b.Access<ptr<workgroup, u32, read_write>>(w, idx));
        Barrier();
        b.Return(f, load);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, array<u32, 4>, read_write> = var
}

%f = func(%idx:u32):u32 {
  $B2: {
    %4:void = workgroupBarrier
    %5:ptr<workgroup, u32, read_write> = access %w, %idx
    %6:u32 = load %5
    %7:void = workgroupBarrier
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// A store to workgroup memory cannot be moved across a barrier.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, WorkgroupStoreBeforeLoad_NoChange) {
    auto* w = b.Var<workgroup, i32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] {
        Barrier();
        b.Store(w, 42_i);
        auto* load = b.Load(w);
        Barrier();
        b.Return(f, load);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, i32, read_write> = var
}

%f = func():i32 {
  $B2: {
    %3:void = workgroupBarrier
    store %w, 42i
    %4:i32 = load %w
    %5:void = workgroupBarrier
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_WorkgroupUniformLoadTest, StorageLoad_NoChange) {
    auto* s = b.Var<storage, i32, read_write>("s");
    s->SetBindingPoint(0, 0);
    b.ir.root_block->Append(s);
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* load = b.Load(s);
        Barrier();
        b.Return(f, load);
    });

    auto* src = R"(
$B1: {  # root
  %s:ptr<storage, i32, read_write> = var @binding_point(0, 0)
}

%f = func():i32 {
  $B2: {
    %3:void = workgroupBarrier
    %4:i32 = load %s
    %5:void = workgroupBarrier
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// Stores to a function variable are not seen by other invocations, so can be moved across the
// second barrier.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, FunctionVarStoreBeforeSecondBarrier) {
    auto* w = b.Var<workgroup, i32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, i32>("v");
        Barrier();
        auto* load = b.Load(w);
        b.Store(v, load);
        Barrier();
        b.Return(f, b.Load(v));
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, i32, read_write> = var
}

%f = func():i32 {
  $B2: {
    %v:ptr<function, i32, read_write> = var
    %4:void = workgroupBarrier
    %5:i32 = load %w
    store %v, %5
    %6:void = workgroupBarrier
    %7:i32 = load %v
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, i32, read_write> = var
}

%f = func():i32 {
  $B2: {
    %v:ptr<function, i32, read_write> = var
    %4:i32 = wgsl.workgroupUniformLoad %w
    store %v, %4
    %5:i32 = load %v
    ret %5
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// Without a second barrier, the load is replaced as its value decides a branch.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, NoSecondBarrier_DecidesIf) {
    auto* w = b.Var<workgroup, u32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* load = b.Load(w);
        auto* if_ = b.If(b.GreaterThan(load, 0_u));
        b.Append(if_->True(), [&] {
            Barrier();
            b.ExitIf(if_);
        });
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func():void {
  $B2: {
    %3:void = workgroupBarrier
    %4:u32 = load %w
    %5:bool = gt %4, 0u
    if %5 [t: $B3] {  # if_1
      $B3: {  # true
        %6:void = workgroupBarrier
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func():void {
  $B2: {
    %3:u32 = wgsl.workgroupUniformLoad %w
    %4:bool = gt %3, 0u
    if %4 [t: $B3] {  # if_1
      $B3: {  # true
        %5:void = workgroupBarrier
        exit_if  # if_1
      }
    }
    ret
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// Without a second barrier, the value decides a branch after passing through a function variable.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, NoSecondBarrier_DecidesIfThroughVar) {
    auto* w = b.Var<workgroup, u32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, u32>("v");
        Barrier();
        auto* load = b.Load(w);
        b.Store(v, load);
        auto* if_ = b.If(b.GreaterThan(b.Load(v), 0_u));
        b.Append(if_->True(), [&] {
            Barrier();
            b.ExitIf(if_);
        });
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func():void {
  $B2: {
    %v:ptr<function, u32, read_write> = var
    %4:void = workgroupBarrier
    %5:u32 = load %w
    store %v, %5
    %6:u32 = load %v
    %7:bool = gt %6, 0u
    if %7 [t: $B3] {  # if_1
      $B3: {  # true
        %8:void = workgroupBarrier
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func():void {
  $B2: {
    %v:ptr<function, u32, read_write> = var
    %4:u32 = wgsl.workgroupUniformLoad %w
    store %v, %4
    %5:u32 = load %v
    %6:bool = gt %5, 0u
    if %6 [t: $B3] {  # if_1
      $B3: {  # true
        %7:void = workgroupBarrier
        exit_if  # if_1
      }
    }
    ret
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// Without a second barrier, the replacement would add a barrier. The value does not decide control
// flow, so it is left as a load.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, NoSecondBarrier_NoControlFlow_NoChange) {
    auto* w = b.Var<workgroup, u32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* s = b.Var<storage, u32, read_write>("s");
    s->SetBindingPoint(0, 0);
    b.ir.root_block->Append(s);
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* load = b.Load(w);
        b.Store(s, b.Add(load, 1_u));
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
  %s:ptr<storage, u32, read_write> = var @binding_point(0, 0)
}

%f = func():void {
  $B2: {
    %4:void = workgroupBarrier
    %5:u32 = load %w
    %6:u32 = add %5, 1u
    store %s, %6
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// Two loads between barriers. Neither decides control flow, so replacing them would only add
// barriers.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, TwoLoads_NoControlFlow_NoChange) {
    auto* w1 = b.Var<workgroup, u32, read_write>("w1");
    auto* w2 = b.Var<workgroup, u32, read_write>("w2");
    b.ir.root_block->Append(w1);
    b.ir.root_block->Append(w2);
    auto* s = b.Var<storage, u32, read_write>("s");
    s->SetBindingPoint(0, 0);
    b.ir.root_block->Append(s);
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* l1 = b.Load(w1);
        auto* l2 = b.Load(w2);
        auto* sum = b.Add(l1, l2);
        Barrier();
        b.Store(s, sum);
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %w1:ptr<workgroup, u32, read_write> = var
  %w2:ptr<workgroup, u32, read_write> = var
  %s:ptr<storage, u32, read_write> = var @binding_point(0, 0)
}

%f = func():void {
  $B2: {
    %5:void = workgroupBarrier
    %6:u32 = load %w1
    %7:u32 = load %w2
    %8:u32 = add %6, %7
    %9:void = workgroupBarrier
    store %s, %8
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// Two loads between barriers, and their sum decides a branch. Both must be replaced.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, TwoLoads_DecidesIf) {
    auto* w1 = b.Var<workgroup, u32, read_write>("w1");
    auto* w2 = b.Var<workgroup, u32, read_write>("w2");
    b.ir.root_block->Append(w1);
    b.ir.root_block->Append(w2);
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* l1 = b.Load(w1);
        auto* l2 = b.Load(w2);
        auto* sum = b.Add(l1, l2);
        Barrier();
        auto* if_ = b.If(b.GreaterThan(sum, 0_u));
        b.Append(if_->True(), [&] {
            Barrier();
            b.ExitIf(if_);
        });
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %w1:ptr<workgroup, u32, read_write> = var
  %w2:ptr<workgroup, u32, read_write> = var
}

%f = func():void {
  $B2: {
    %4:void = workgroupBarrier
    %5:u32 = load %w1
    %6:u32 = load %w2
    %7:u32 = add %5, %6
    %8:void = workgroupBarrier
    %9:bool = gt %7, 0u
    if %9 [t: $B3] {  # if_1
      $B3: {  # true
        %10:void = workgroupBarrier
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w1:ptr<workgroup, u32, read_write> = var
  %w2:ptr<workgroup, u32, read_write> = var
}

%f = func():void {
  $B2: {
    %4:u32 = wgsl.workgroupUniformLoad %w1
    %5:u32 = wgsl.workgroupUniformLoad %w2
    %6:u32 = add %4, %5
    %7:bool = gt %6, 0u
    if %7 [t: $B3] {  # if_1
      $B3: {  # true
        %8:void = workgroupBarrier
        exit_if  # if_1
      }
    }
    ret
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// The second barrier of one load is the first barrier of the next.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, SharedBarrier) {
    auto* w1 = b.Var<workgroup, i32, read_write>("w1");
    auto* w2 = b.Var<workgroup, i32, read_write>("w2");
    b.ir.root_block->Append(w1);
    b.ir.root_block->Append(w2);
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* l1 = b.Load(w1);
        Barrier();
        auto* l2 = b.Load(w2);
        Barrier();
        b.Return(f, b.Add(l1, l2));
    });

    auto* src = R"(
$B1: {  # root
  %w1:ptr<workgroup, i32, read_write> = var
  %w2:ptr<workgroup, i32, read_write> = var
}

%f = func():i32 {
  $B2: {
    %4:void = workgroupBarrier
    %5:i32 = load %w1
    %6:void = workgroupBarrier
    %7:i32 = load %w2
    %8:void = workgroupBarrier
    %9:i32 = add %5, %7
    ret %9
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w1:ptr<workgroup, i32, read_write> = var
  %w2:ptr<workgroup, i32, read_write> = var
}

%f = func():i32 {
  $B2: {
    %4:i32 = wgsl.workgroupUniformLoad %w1
    %5:i32 = wgsl.workgroupUniformLoad %w2
    %6:i32 = add %4, %5
    ret %6
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// The second barrier of one load is the first barrier of the next, and only the first load is
// replaced.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, SharedBarrier_SecondNotReplaced) {
    auto* w1 = b.Var<workgroup, i32, read_write>("w1");
    auto* w2 = b.Var<workgroup, i32, read_write>("w2");
    auto* s = b.Var<storage, i32, read_write>("s");
    s->SetBindingPoint(0, 0);
    b.ir.root_block->Append(w1);
    b.ir.root_block->Append(w2);
    b.ir.root_block->Append(s);
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* l1 = b.Load(w1);
        Barrier();
        b.Store(s, b.Load(w2));
        b.Return(f, l1);
    });

    auto* src = R"(
$B1: {  # root
  %w1:ptr<workgroup, i32, read_write> = var
  %w2:ptr<workgroup, i32, read_write> = var
  %s:ptr<storage, i32, read_write> = var @binding_point(0, 0)
}

%f = func():i32 {
  $B2: {
    %5:void = workgroupBarrier
    %6:i32 = load %w1
    %7:void = workgroupBarrier
    %8:i32 = load %w2
    store %s, %8
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w1:ptr<workgroup, i32, read_write> = var
  %w2:ptr<workgroup, i32, read_write> = var
  %s:ptr<storage, i32, read_write> = var @binding_point(0, 0)
}

%f = func():i32 {
  $B2: {
    %5:i32 = wgsl.workgroupUniformLoad %w1
    %6:i32 = load %w2
    store %s, %6
    ret %5
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// The second barrier of one run is the first barrier of the next, and only the second run is
// replaced.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, SharedBarrier_FirstNotReplaced) {
    auto* w1 = b.Var<workgroup, i32, read_write>("w1");
    auto* w2 = b.Var<workgroup, i32, read_write>("w2");
    auto* s = b.Var<storage, i32, read_write>("s");
    s->SetBindingPoint(0, 0);
    b.ir.root_block->Append(w1);
    b.ir.root_block->Append(w2);
    b.ir.root_block->Append(s);
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* l1 = b.Load(w1);
        auto* l2 = b.Load(w2);
        Barrier();
        auto* l3 = b.Load(w1);
        Barrier();
        b.Store(s, b.Add(b.Add(l1, l2), l3));
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %w1:ptr<workgroup, i32, read_write> = var
  %w2:ptr<workgroup, i32, read_write> = var
  %s:ptr<storage, i32, read_write> = var @binding_point(0, 0)
}

%f = func():void {
  $B2: {
    %5:void = workgroupBarrier
    %6:i32 = load %w1
    %7:i32 = load %w2
    %8:void = workgroupBarrier
    %9:i32 = load %w1
    %10:void = workgroupBarrier
    %11:i32 = add %6, %7
    %12:i32 = add %11, %9
    store %s, %12
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w1:ptr<workgroup, i32, read_write> = var
  %w2:ptr<workgroup, i32, read_write> = var
  %s:ptr<storage, i32, read_write> = var @binding_point(0, 0)
}

%f = func():void {
  $B2: {
    %5:void = workgroupBarrier
    %6:i32 = load %w1
    %7:i32 = load %w2
    %8:i32 = wgsl.workgroupUniformLoad %w1
    %9:i32 = add %6, %7
    %10:i32 = add %9, %8
    store %s, %10
    ret
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// Vector element accesses to function memory, and stores to private memory, can be moved across a
// barrier.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, InvocationLocalAccessesBeforeSecondBarrier) {
    auto* w = b.Var<workgroup, i32, read_write>("w");
    auto* p = b.Var<private_, i32, read_write>("p");
    b.ir.root_block->Append(w);
    b.ir.root_block->Append(p);
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, vec4<i32>, read_write>("v");
        Barrier();
        auto* load = b.Load(w);
        b.StoreVectorElement(v, 0_u, load);
        auto* e = b.LoadVectorElement(v, 1_u);
        b.Store(p, e);
        Barrier();
        b.Return(f, load);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, i32, read_write> = var
  %p:ptr<private, i32, read_write> = var
}

%f = func():i32 {
  $B2: {
    %v:ptr<function, vec4<i32>, read_write> = var
    %5:void = workgroupBarrier
    %6:i32 = load %w
    store_vector_element %v, 0u, %6
    %7:i32 = load_vector_element %v, 1u
    store %p, %7
    %8:void = workgroupBarrier
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, i32, read_write> = var
  %p:ptr<private, i32, read_write> = var
}

%f = func():i32 {
  $B2: {
    %v:ptr<function, vec4<i32>, read_write> = var
    %5:i32 = wgsl.workgroupUniformLoad %w
    store_vector_element %v, 0u, %5
    %6:i32 = load_vector_element %v, 1u
    store %p, %6
    ret %5
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// A vector element store to workgroup memory cannot be moved across a barrier.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, WorkgroupStoreVectorElement_NoChange) {
    auto* w = b.Var<workgroup, i32, read_write>("w");
    auto* v = b.Var<workgroup, vec4<i32>, read_write>("v");
    b.ir.root_block->Append(w);
    b.ir.root_block->Append(v);
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] {
        Barrier();
        b.StoreVectorElement(v, 0_u, 1_i);
        auto* load = b.Load(w);
        Barrier();
        b.Return(f, load);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, i32, read_write> = var
  %v:ptr<workgroup, vec4<i32>, read_write> = var
}

%f = func():i32 {
  $B2: {
    %4:void = workgroupBarrier
    store_vector_element %v, 0u, 1i
    %5:i32 = load %w
    %6:void = workgroupBarrier
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// An instruction with side effects cannot be moved across a barrier.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, SideEffects_NoChange) {
    auto* w = b.Var<workgroup, i32, read_write>("w");
    auto* a = b.Var<workgroup, atomic<i32>, read_write>("a");
    b.ir.root_block->Append(w);
    b.ir.root_block->Append(a);
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] {
        Barrier();
        b.Call(ty.i32(), core::BuiltinFn::kAtomicAdd, a, 1_i);
        auto* load = b.Load(w);
        Barrier();
        b.Return(f, load);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, i32, read_write> = var
  %a:ptr<workgroup, atomic<i32>, read_write> = var
}

%f = func():i32 {
  $B2: {
    %4:void = workgroupBarrier
    %5:i32 = atomicAdd %a, 1i
    %6:i32 = load %w
    %7:void = workgroupBarrier
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// workgroupBarrier does not order accesses to storage memory, so they can be moved across it.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, StorageAccessesInSpan) {
    auto* w = b.Var<workgroup, u32, read_write>("w");
    auto* s = b.Var<storage, u32, read_write>("s");
    s->SetBindingPoint(0, 0);
    b.ir.root_block->Append(w);
    b.ir.root_block->Append(s);
    auto* f = b.Function("f", ty.u32());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* before = b.Load(s);
        auto* load = b.Load(w);
        b.Store(s, load);
        Barrier();
        b.Return(f, b.Add(before, load));
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
  %s:ptr<storage, u32, read_write> = var @binding_point(0, 0)
}

%f = func():u32 {
  $B2: {
    %4:void = workgroupBarrier
    %5:u32 = load %s
    %6:u32 = load %w
    store %s, %6
    %7:void = workgroupBarrier
    %8:u32 = add %5, %6
    ret %8
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
  %s:ptr<storage, u32, read_write> = var @binding_point(0, 0)
}

%f = func():u32 {
  $B2: {
    %4:u32 = load %s
    %5:u32 = wgsl.workgroupUniformLoad %w
    store %s, %5
    %6:u32 = add %4, %5
    ret %6
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_WorkgroupUniformLoadTest, StorageAtomicInSpan) {
    auto* w = b.Var<workgroup, u32, read_write>("w");
    auto* s = b.Var<storage, atomic<u32>, read_write>("s");
    s->SetBindingPoint(0, 0);
    b.ir.root_block->Append(w);
    b.ir.root_block->Append(s);
    auto* f = b.Function("f", ty.u32());
    b.Append(f->Block(), [&] {
        Barrier();
        b.Call(ty.u32(), core::BuiltinFn::kAtomicAdd, s, 1_u);
        auto* load = b.Load(w);
        Barrier();
        b.Return(f, load);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
  %s:ptr<storage, atomic<u32>, read_write> = var @binding_point(0, 0)
}

%f = func():u32 {
  $B2: {
    %4:void = workgroupBarrier
    %5:u32 = atomicAdd %s, 1u
    %6:u32 = load %w
    %7:void = workgroupBarrier
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
  %s:ptr<storage, atomic<u32>, read_write> = var @binding_point(0, 0)
}

%f = func():u32 {
  $B2: {
    %4:u32 = atomicAdd %s, 1u
    %5:u32 = wgsl.workgroupUniformLoad %w
    ret %5
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_WorkgroupUniformLoadTest, TextureStoreInSpan) {
    auto* w = b.Var<workgroup, u32, read_write>("w");
    auto* texture_ty =
        ty.storage_texture(core::type::TextureDimension::k2d, core::TexelFormat::kR32Uint, write);
    auto* t = b.Var("t", ty.ptr(handle, texture_ty));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(w);
    b.ir.root_block->Append(t);
    auto* f = b.Function("f", ty.u32());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* texture = b.Load(t);
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, texture, b.Splat<vec2<u32>>(0_u),
               b.Splat<vec4<u32>>(1_u));
        auto* load = b.Load(w);
        Barrier();
        b.Return(f, load);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
  %t:ptr<handle, texture_storage_2d<r32uint, write>, read> = var @binding_point(0, 0)
}

%f = func():u32 {
  $B2: {
    %4:void = workgroupBarrier
    %5:texture_storage_2d<r32uint, write> = load %t
    %6:void = textureStore %5, vec2<u32>(0u), vec4<u32>(1u)
    %7:u32 = load %w
    %8:void = workgroupBarrier
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
  %t:ptr<handle, texture_storage_2d<r32uint, write>, read> = var @binding_point(0, 0)
}

%f = func():u32 {
  $B2: {
    %4:texture_storage_2d<r32uint, write> = load %t
    %5:void = textureStore %4, vec2<u32>(0u), vec4<u32>(1u)
    %6:u32 = wgsl.workgroupUniformLoad %w
    ret %6
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// storageBarrier is a control barrier, so cannot be moved across a workgroupBarrier.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, StorageBarrierInSpan_NoChange) {
    auto* w = b.Var<workgroup, u32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.u32());
    b.Append(f->Block(), [&] {
        Barrier();
        b.Call(ty.void_(), core::BuiltinFn::kStorageBarrier);
        auto* load = b.Load(w);
        Barrier();
        b.Return(f, load);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func():u32 {
  $B2: {
    %3:void = workgroupBarrier
    %4:void = storageBarrier
    %5:u32 = load %w
    %6:void = workgroupBarrier
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// textureBarrier is a control barrier, so cannot be moved across a workgroupBarrier.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, TextureBarrierInSpan_NoChange) {
    auto* w = b.Var<workgroup, u32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.u32());
    b.Append(f->Block(), [&] {
        Barrier();
        b.Call(ty.void_(), core::BuiltinFn::kTextureBarrier);
        auto* load = b.Load(w);
        Barrier();
        b.Return(f, load);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func():u32 {
  $B2: {
    %3:void = workgroupBarrier
    %4:void = textureBarrier
    %5:u32 = load %w
    %6:void = workgroupBarrier
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// A call to a user function may access workgroup memory, so cannot be moved across a barrier.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, UserCallInSpan_NoChange) {
    auto* w = b.Var<workgroup, u32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* g = b.Function("g", ty.void_());
    b.Append(g->Block(), [&] { b.Return(g); });
    auto* f = b.Function("f", ty.u32());
    b.Append(f->Block(), [&] {
        Barrier();
        b.Call(g);
        auto* load = b.Load(w);
        Barrier();
        b.Return(f, load);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%g = func():void {
  $B2: {
    ret
  }
}
%f = func():u32 {
  $B3: {
    %4:void = workgroupBarrier
    %5:void = call %g
    %6:u32 = load %w
    %7:void = workgroupBarrier
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// A load through a let of a pointer to workgroup memory.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, LoadThroughLet) {
    auto* w = b.Var<workgroup, i32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] {
        auto* p = b.Let("p", w);
        Barrier();
        auto* load = b.Load(p);
        Barrier();
        b.Return(f, load);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, i32, read_write> = var
}

%f = func():i32 {
  $B2: {
    %p:ptr<workgroup, i32, read_write> = let %w
    %4:void = workgroupBarrier
    %5:i32 = load %p
    %6:void = workgroupBarrier
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, i32, read_write> = var
}

%f = func():i32 {
  $B2: {
    %p:ptr<workgroup, i32, read_write> = let %w
    %4:i32 = wgsl.workgroupUniformLoad %p
    ret %4
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// Without a second barrier, the value leaves the function through a pointer parameter, so the
// caller may use it to decide a branch.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, NoSecondBarrier_StoredThroughPointerParam) {
    auto* w = b.Var<workgroup, u32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.ptr<function, u32>());
    f->SetParams({p});
    b.Append(f->Block(), [&] {
        Barrier();
        b.Store(p, b.Load(w));
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func(%p:ptr<function, u32, read_write>):void {
  $B2: {
    %4:void = workgroupBarrier
    %5:u32 = load %w
    store %p, %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func(%p:ptr<function, u32, read_write>):void {
  $B2: {
    %4:u32 = wgsl.workgroupUniformLoad %w
    store %p, %4
    ret
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// Without a second barrier, the load is replaced as its value decides a switch.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, NoSecondBarrier_DecidesSwitch) {
    auto* w = b.Var<workgroup, u32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* sw = b.Switch(b.Load(w));
        b.Append(b.DefaultCase(sw), [&] {
            Barrier();
            b.ExitSwitch(sw);
        });
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func():void {
  $B2: {
    %3:void = workgroupBarrier
    %4:u32 = load %w
    switch %4 [c: (default, $B3)] {  # switch_1
      $B3: {  # case
        %5:void = workgroupBarrier
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func():void {
  $B2: {
    %3:u32 = wgsl.workgroupUniformLoad %w
    switch %3 [c: (default, $B3)] {  # switch_1
      $B3: {  # case
        %4:void = workgroupBarrier
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// Without a second barrier, the load is replaced as its value decides a break_if.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, NoSecondBarrier_DecidesBreakIf) {
    auto* w = b.Var<workgroup, u32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* load = b.Load(w);
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            Barrier();
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, b.GreaterThan(load, 0_u)); });
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func():void {
  $B2: {
    %3:void = workgroupBarrier
    %4:u32 = load %w
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        %5:void = workgroupBarrier
        continue  # -> $B4
      }
      $B4: {  # continuing
        %6:bool = gt %4, 0u
        break_if %6  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func():void {
  $B2: {
    %3:u32 = wgsl.workgroupUniformLoad %w
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        %4:void = workgroupBarrier
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:bool = gt %3, 0u
        break_if %5  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// Without a second barrier, the value is passed to a function, which may use it to decide a branch.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, NoSecondBarrier_PassedToFunction) {
    auto* w = b.Var<workgroup, u32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* g = b.Function("g", ty.void_());
    g->SetParams({b.FunctionParam<u32>("x")});
    b.Append(g->Block(), [&] { b.Return(g); });
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        Barrier();
        b.Call(g, b.Load(w));
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%g = func(%x:u32):void {
  $B2: {
    ret
  }
}
%f = func():void {
  $B3: {
    %5:void = workgroupBarrier
    %6:u32 = load %w
    %7:void = call %g, %6
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%g = func(%x:u32):void {
  $B2: {
    ret
  }
}
%f = func():void {
  $B3: {
    %5:u32 = wgsl.workgroupUniformLoad %w
    %6:void = call %g, %5
    ret
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// Without a second barrier, the value is returned, and the caller may use it to decide a branch.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, NoSecondBarrier_Returned) {
    auto* w = b.Var<workgroup, u32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.u32());
    b.Append(f->Block(), [&] {
        Barrier();
        b.Return(f, b.Load(w));
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func():u32 {
  $B2: {
    %3:void = workgroupBarrier
    %4:u32 = load %w
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func():u32 {
  $B2: {
    %3:u32 = wgsl.workgroupUniformLoad %w
    ret %3
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// Without a second barrier, the value decides a branch after passing through an element of a
// function variable.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, NoSecondBarrier_DecidesIfThroughArrayElement) {
    auto* w = b.Var<workgroup, u32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, array<u32, 4>>("v");
        Barrier();
        auto* el = b.Access<ptr<function, u32>>(v, 1_u);
        b.Store(el, b.Load(w));
        auto* load = b.Load(b.Access<ptr<function, u32>>(v, 1_u));
        auto* if_ = b.If(b.GreaterThan(load, 0_u));
        b.Append(if_->True(), [&] {
            Barrier();
            b.ExitIf(if_);
        });
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func():void {
  $B2: {
    %v:ptr<function, array<u32, 4>, read_write> = var
    %4:void = workgroupBarrier
    %5:ptr<function, u32, read_write> = access %v, 1u
    %6:u32 = load %w
    store %5, %6
    %7:ptr<function, u32, read_write> = access %v, 1u
    %8:u32 = load %7
    %9:bool = gt %8, 0u
    if %9 [t: $B3] {  # if_1
      $B3: {  # true
        %10:void = workgroupBarrier
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func():void {
  $B2: {
    %v:ptr<function, array<u32, 4>, read_write> = var
    %4:ptr<function, u32, read_write> = access %v, 1u
    %5:u32 = wgsl.workgroupUniformLoad %w
    store %4, %5
    %6:ptr<function, u32, read_write> = access %v, 1u
    %7:u32 = load %6
    %8:bool = gt %7, 0u
    if %8 [t: $B3] {  # if_1
      $B3: {  # true
        %9:void = workgroupBarrier
        exit_if  # if_1
      }
    }
    ret
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// Without a second barrier, the value reaches the same instruction along two paths, neither of
// which decides control flow.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, NoSecondBarrier_ValueUsedTwice_NoChange) {
    auto* w = b.Var<workgroup, u32, read_write>("w");
    auto* s = b.Var<storage, u32, read_write>("s");
    s->SetBindingPoint(0, 0);
    b.ir.root_block->Append(w);
    b.ir.root_block->Append(s);
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* load = b.Load(w);
        auto* x = b.Add(load, 1_u);
        auto* y = b.Add(load, 2_u);
        b.Store(s, b.Add(x, y));
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
  %s:ptr<storage, u32, read_write> = var @binding_point(0, 0)
}

%f = func():void {
  $B2: {
    %4:void = workgroupBarrier
    %5:u32 = load %w
    %6:u32 = add %5, 1u
    %7:u32 = add %5, 2u
    %8:u32 = add %6, %7
    store %s, %8
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// workgroupUniformLoad requires a uniform pointer, which a pointer parameter may not be.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, LoadThroughPointerParam_NoChange) {
    auto* f = b.Function("f", ty.u32());
    auto* p = b.FunctionParam("p", ty.ptr<workgroup, u32>());
    f->SetParams({p});
    b.Append(f->Block(), [&] {
        Barrier();
        auto* load = b.Load(p);
        Barrier();
        b.Return(f, load);
    });

    auto* src = R"(
%f = func(%p:ptr<workgroup, u32, read_write>):u32 {
  $B1: {
    %3:void = workgroupBarrier
    %4:u32 = load %p
    %5:void = workgroupBarrier
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// A control instruction between the first barrier and the load stops the run.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, ControlInstructionBeforeLoad_NoChange) {
    auto* w = b.Var<workgroup, u32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.u32());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.ExitIf(if_); });
        auto* load = b.Load(w);
        Barrier();
        b.Return(f, load);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func():u32 {
  $B2: {
    %3:void = workgroupBarrier
    if true [t: $B3] {  # if_1
      $B3: {  # true
        exit_if  # if_1
      }
    }
    %4:u32 = load %w
    %5:void = workgroupBarrier
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

// Loads are replaced in nested blocks, and in every function.
TEST_F(WgslWriter_WorkgroupUniformLoadTest, NestedBlockAndMultipleFunctions) {
    auto* w = b.Var<workgroup, u32, read_write>("w");
    b.ir.root_block->Append(w);
    auto* f = b.Function("f", ty.u32());
    b.Append(f->Block(), [&] {
        Barrier();
        auto* load = b.Load(w);
        Barrier();
        b.Return(f, load);
    });
    auto* g = b.Function("g", ty.u32());
    b.Append(g->Block(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            Barrier();
            auto* load = b.Load(w);
            Barrier();
            b.Return(g, load);
        });
        b.Return(g, 0_u);
    });

    auto* src = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func():u32 {
  $B2: {
    %3:void = workgroupBarrier
    %4:u32 = load %w
    %5:void = workgroupBarrier
    ret %4
  }
}
%g = func():u32 {
  $B3: {
    if true [t: $B4] {  # if_1
      $B4: {  # true
        %7:void = workgroupBarrier
        %8:u32 = load %w
        %9:void = workgroupBarrier
        ret %8
      }
    }
    ret 0u
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %w:ptr<workgroup, u32, read_write> = var
}

%f = func():u32 {
  $B2: {
    %3:u32 = wgsl.workgroupUniformLoad %w
    ret %3
  }
}
%g = func():u32 {
  $B3: {
    if true [t: $B4] {  # if_1
      $B4: {  # true
        %5:u32 = wgsl.workgroupUniformLoad %w
        ret %5
      }
    }
    ret 0u
  }
}
)";

    Run(WorkgroupUniformLoad);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::wgsl::writer::raise
