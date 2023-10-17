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

#include "src/tint/lang/core/ir/transform/zero_init_workgroup_memory.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_ZeroInitWorkgroupMemoryTest : public TransformTest {
  protected:
    Function* MakeEntryPoint(const char* name,
                             uint32_t wgsize_x,
                             uint32_t wgsize_y,
                             uint32_t wgsize_z) {
        auto* func = b.Function(name, ty.void_(), Function::PipelineStage::kCompute);
        func->SetWorkgroupSize(wgsize_x, wgsize_y, wgsize_z);
        return func;
    }

    Var* MakeVar(const char* name, const type::Type* store_type) {
        auto* var = b.Var(name, ty.ptr(workgroup, store_type));
        mod.root_block->Append(var);
        return var;
    }
};

TEST_F(IR_ZeroInitWorkgroupMemoryTest, NoRootBlock) {
    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    auto* expect = R"(
%main = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, WorkgroupVarUnused) {
    MakeVar("wgvar", ty.i32());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, i32, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ScalarBool) {
    auto* var = MakeVar("wgvar", ty.bool_());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:bool = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    %4:bool = eq %tint_local_index, 0u
    if %4 [t: %b3] {  # if_1
      %b3 = block {  # true
        store %wgvar, false
        exit_if  # if_1
      }
    }
    %5:void = workgroupBarrier
    %6:bool = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ScalarI32) {
    auto* var = MakeVar("wgvar", ty.i32());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, i32, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:i32 = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, i32, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    %4:bool = eq %tint_local_index, 0u
    if %4 [t: %b3] {  # if_1
      %b3 = block {  # true
        store %wgvar, 0i
        exit_if  # if_1
      }
    }
    %5:void = workgroupBarrier
    %6:i32 = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ScalarU32) {
    auto* var = MakeVar("wgvar", ty.u32());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, u32, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:u32 = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, u32, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    %4:bool = eq %tint_local_index, 0u
    if %4 [t: %b3] {  # if_1
      %b3 = block {  # true
        store %wgvar, 0u
        exit_if  # if_1
      }
    }
    %5:void = workgroupBarrier
    %6:u32 = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ScalarF32) {
    auto* var = MakeVar("wgvar", ty.f32());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, f32, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:f32 = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, f32, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    %4:bool = eq %tint_local_index, 0u
    if %4 [t: %b3] {  # if_1
      %b3 = block {  # true
        store %wgvar, 0.0f
        exit_if  # if_1
      }
    }
    %5:void = workgroupBarrier
    %6:f32 = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ScalarF16) {
    auto* var = MakeVar("wgvar", ty.f16());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, f16, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:f16 = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, f16, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    %4:bool = eq %tint_local_index, 0u
    if %4 [t: %b3] {  # if_1
      %b3 = block {  # true
        store %wgvar, 0.0h
        exit_if  # if_1
      }
    }
    %5:void = workgroupBarrier
    %6:f16 = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, AtomicI32) {
    auto* var = MakeVar("wgvar", ty.atomic<i32>());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad, var);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, atomic<i32>, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:i32 = atomicLoad %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, atomic<i32>, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    %4:bool = eq %tint_local_index, 0u
    if %4 [t: %b3] {  # if_1
      %b3 = block {  # true
        %5:void = atomicStore %wgvar, 0i
        exit_if  # if_1
      }
    }
    %6:void = workgroupBarrier
    %7:i32 = atomicLoad %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, AtomicU32) {
    auto* var = MakeVar("wgvar", ty.atomic<u32>());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Call(ty.u32(), core::BuiltinFn::kAtomicLoad, var);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, atomic<u32>, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:u32 = atomicLoad %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, atomic<u32>, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    %4:bool = eq %tint_local_index, 0u
    if %4 [t: %b3] {  # if_1
      %b3 = block {  # true
        %5:void = atomicStore %wgvar, 0u
        exit_if  # if_1
      }
    }
    %6:void = workgroupBarrier
    %7:u32 = atomicLoad %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ArrayOfI32) {
    auto* var = MakeVar("wgvar", ty.array<i32, 4>());

    auto* func = MakeEntryPoint("main", 11, 2, 3);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, array<i32, 4>, read_write> = var
}

%main = @compute @workgroup_size(11, 2, 3) func():void -> %b2 {
  %b2 = block {
    %3:array<i32, 4> = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, array<i32, 4>, read_write> = var
}

%main = @compute @workgroup_size(11, 2, 3) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    loop [i: %b3, b: %b4, c: %b5] {  # loop_1
      %b3 = block {  # initializer
        next_iteration %b4 %tint_local_index
      }
      %b4 = block (%idx:u32) {  # body
        %5:bool = gte %idx:u32, 4u
        if %5 [t: %b6] {  # if_1
          %b6 = block {  # true
            exit_loop  # loop_1
          }
        }
        %6:ptr<workgroup, i32, read_write> = access %wgvar, %idx:u32
        store %6, 0i
        continue %b5
      }
      %b5 = block {  # continuing
        %7:u32 = add %idx:u32, 66u
        next_iteration %b4 %7
      }
    }
    %8:void = workgroupBarrier
    %9:array<i32, 4> = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ArrayOfArrayOfU32) {
    auto* var = MakeVar("wgvar", ty.array(ty.array<u32, 5>(), 7));

    auto* func = MakeEntryPoint("main", 11, 2, 3);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, array<array<u32, 5>, 7>, read_write> = var
}

%main = @compute @workgroup_size(11, 2, 3) func():void -> %b2 {
  %b2 = block {
    %3:array<array<u32, 5>, 7> = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, array<array<u32, 5>, 7>, read_write> = var
}

%main = @compute @workgroup_size(11, 2, 3) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    loop [i: %b3, b: %b4, c: %b5] {  # loop_1
      %b3 = block {  # initializer
        next_iteration %b4 %tint_local_index
      }
      %b4 = block (%idx:u32) {  # body
        %5:bool = gte %idx:u32, 35u
        if %5 [t: %b6] {  # if_1
          %b6 = block {  # true
            exit_loop  # loop_1
          }
        }
        %6:u32 = mod %idx:u32, 5u
        %7:u32 = div %idx:u32, 5u
        %8:ptr<workgroup, u32, read_write> = access %wgvar, %7, %6
        store %8, 0u
        continue %b5
      }
      %b5 = block {  # continuing
        %9:u32 = add %idx:u32, 66u
        next_iteration %b4 %9
      }
    }
    %10:void = workgroupBarrier
    %11:array<array<u32, 5>, 7> = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ArrayOfArrayOfArray) {
    auto* var = MakeVar("wgvar", ty.array(ty.array(ty.array<i32, 7>(), 5), 3));

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, array<array<array<i32, 7>, 5>, 3>, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:array<array<array<i32, 7>, 5>, 3> = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, array<array<array<i32, 7>, 5>, 3>, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    loop [i: %b3, b: %b4, c: %b5] {  # loop_1
      %b3 = block {  # initializer
        next_iteration %b4 %tint_local_index
      }
      %b4 = block (%idx:u32) {  # body
        %5:bool = gte %idx:u32, 105u
        if %5 [t: %b6] {  # if_1
          %b6 = block {  # true
            exit_loop  # loop_1
          }
        }
        %6:u32 = mod %idx:u32, 7u
        %7:u32 = div %idx:u32, 7u
        %8:u32 = mod %7, 5u
        %9:u32 = div %idx:u32, 35u
        %10:ptr<workgroup, i32, read_write> = access %wgvar, %9, %8, %6
        store %10, 0i
        continue %b5
      }
      %b5 = block {  # continuing
        %11:u32 = add %idx:u32, 1u
        next_iteration %b4 %11
      }
    }
    %12:void = workgroupBarrier
    %13:array<array<array<i32, 7>, 5>, 3> = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, NestedArrayInnerSizeOne) {
    auto* var = MakeVar("wgvar", ty.array(ty.array(ty.array<i32, 1>(), 5), 3));

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, array<array<array<i32, 1>, 5>, 3>, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:array<array<array<i32, 1>, 5>, 3> = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, array<array<array<i32, 1>, 5>, 3>, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    loop [i: %b3, b: %b4, c: %b5] {  # loop_1
      %b3 = block {  # initializer
        next_iteration %b4 %tint_local_index
      }
      %b4 = block (%idx:u32) {  # body
        %5:bool = gte %idx:u32, 15u
        if %5 [t: %b6] {  # if_1
          %b6 = block {  # true
            exit_loop  # loop_1
          }
        }
        %6:u32 = mod %idx:u32, 5u
        %7:u32 = div %idx:u32, 5u
        %8:ptr<workgroup, i32, read_write> = access %wgvar, %7, %6, 0u
        store %8, 0i
        continue %b5
      }
      %b5 = block {  # continuing
        %9:u32 = add %idx:u32, 1u
        next_iteration %b4 %9
      }
    }
    %10:void = workgroupBarrier
    %11:array<array<array<i32, 1>, 5>, 3> = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, NestedArrayMiddleSizeOne) {
    auto* var = MakeVar("wgvar", ty.array(ty.array(ty.array<i32, 3>(), 1), 5));

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, array<array<array<i32, 3>, 1>, 5>, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:array<array<array<i32, 3>, 1>, 5> = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, array<array<array<i32, 3>, 1>, 5>, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    loop [i: %b3, b: %b4, c: %b5] {  # loop_1
      %b3 = block {  # initializer
        next_iteration %b4 %tint_local_index
      }
      %b4 = block (%idx:u32) {  # body
        %5:bool = gte %idx:u32, 15u
        if %5 [t: %b6] {  # if_1
          %b6 = block {  # true
            exit_loop  # loop_1
          }
        }
        %6:u32 = mod %idx:u32, 3u
        %7:u32 = div %idx:u32, 3u
        %8:ptr<workgroup, i32, read_write> = access %wgvar, %7, 0u, %6
        store %8, 0i
        continue %b5
      }
      %b5 = block {  # continuing
        %9:u32 = add %idx:u32, 1u
        next_iteration %b4 %9
      }
    }
    %10:void = workgroupBarrier
    %11:array<array<array<i32, 3>, 1>, 5> = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, NestedArrayOuterSizeOne) {
    auto* var = MakeVar("wgvar", ty.array(ty.array(ty.array<i32, 3>(), 5), 1));

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, array<array<array<i32, 3>, 5>, 1>, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:array<array<array<i32, 3>, 5>, 1> = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, array<array<array<i32, 3>, 5>, 1>, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    loop [i: %b3, b: %b4, c: %b5] {  # loop_1
      %b3 = block {  # initializer
        next_iteration %b4 %tint_local_index
      }
      %b4 = block (%idx:u32) {  # body
        %5:bool = gte %idx:u32, 15u
        if %5 [t: %b6] {  # if_1
          %b6 = block {  # true
            exit_loop  # loop_1
          }
        }
        %6:u32 = mod %idx:u32, 3u
        %7:u32 = div %idx:u32, 3u
        %8:ptr<workgroup, i32, read_write> = access %wgvar, 0u, %7, %6
        store %8, 0i
        continue %b5
      }
      %b5 = block {  # continuing
        %9:u32 = add %idx:u32, 1u
        next_iteration %b4 %9
      }
    }
    %10:void = workgroupBarrier
    %11:array<array<array<i32, 3>, 5>, 1> = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, NestedArrayTotalSizeOne) {
    auto* var = MakeVar("wgvar", ty.array(ty.array<i32, 1>(), 1));

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, array<array<i32, 1>, 1>, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:array<array<i32, 1>, 1> = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, array<array<i32, 1>, 1>, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    %4:bool = eq %tint_local_index, 0u
    if %4 [t: %b3] {  # if_1
      %b3 = block {  # true
        %5:ptr<workgroup, i32, read_write> = access %wgvar, 0u, 0u
        store %5, 0i
        exit_if  # if_1
      }
    }
    %6:void = workgroupBarrier
    %7:array<array<i32, 1>, 1> = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, StructOfScalars) {
    auto* s = ty.Struct(mod.symbols.New("MyStruct"), {
                                                         {mod.symbols.New("a"), ty.i32()},
                                                         {mod.symbols.New("b"), ty.u32()},
                                                         {mod.symbols.New("c"), ty.f32()},
                                                     });
    auto* var = MakeVar("wgvar", s);

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
  c:f32 @offset(8)
}

%b1 = block {  # root
  %wgvar:ptr<workgroup, MyStruct, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:MyStruct = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
  c:f32 @offset(8)
}

%b1 = block {  # root
  %wgvar:ptr<workgroup, MyStruct, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    %4:bool = eq %tint_local_index, 0u
    if %4 [t: %b3] {  # if_1
      %b3 = block {  # true
        store %wgvar, MyStruct(0i, 0u, 0.0f)
        exit_if  # if_1
      }
    }
    %5:void = workgroupBarrier
    %6:MyStruct = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, NestedStructOfScalars) {
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), ty.i32()},
                                                          {mod.symbols.New("b"), ty.u32()},
                                                      });
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("c"), ty.f32()},
                                                          {mod.symbols.New("inner"), inner},
                                                          {mod.symbols.New("d"), ty.bool_()},
                                                      });
    auto* var = MakeVar("wgvar", outer);

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

Outer = struct @align(4) {
  c:f32 @offset(0)
  inner:Inner @offset(4)
  d:bool @offset(12)
}

%b1 = block {  # root
  %wgvar:ptr<workgroup, Outer, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:Outer = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

Outer = struct @align(4) {
  c:f32 @offset(0)
  inner:Inner @offset(4)
  d:bool @offset(12)
}

%b1 = block {  # root
  %wgvar:ptr<workgroup, Outer, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    %4:bool = eq %tint_local_index, 0u
    if %4 [t: %b3] {  # if_1
      %b3 = block {  # true
        store %wgvar, Outer(0.0f, Inner(0i, 0u), false)
        exit_if  # if_1
      }
    }
    %5:void = workgroupBarrier
    %6:Outer = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, NestedStructOfScalarsWithAtomic) {
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), ty.i32()},
                                                          {mod.symbols.New("b"), ty.atomic<u32>()},
                                                      });
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("c"), ty.f32()},
                                                          {mod.symbols.New("inner"), inner},
                                                          {mod.symbols.New("d"), ty.bool_()},
                                                      });
    auto* var = MakeVar("wgvar", outer);

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  a:i32 @offset(0)
  b:atomic<u32> @offset(4)
}

Outer = struct @align(4) {
  c:f32 @offset(0)
  inner:Inner @offset(4)
  d:bool @offset(12)
}

%b1 = block {  # root
  %wgvar:ptr<workgroup, Outer, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:Outer = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  a:i32 @offset(0)
  b:atomic<u32> @offset(4)
}

Outer = struct @align(4) {
  c:f32 @offset(0)
  inner:Inner @offset(4)
  d:bool @offset(12)
}

%b1 = block {  # root
  %wgvar:ptr<workgroup, Outer, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    %4:bool = eq %tint_local_index, 0u
    if %4 [t: %b3] {  # if_1
      %b3 = block {  # true
        %5:ptr<workgroup, f32, read_write> = access %wgvar, 0u
        store %5, 0.0f
        %6:ptr<workgroup, i32, read_write> = access %wgvar, 1u, 0u
        store %6, 0i
        %7:ptr<workgroup, atomic<u32>, read_write> = access %wgvar, 1u, 1u
        %8:void = atomicStore %7, 0u
        %9:ptr<workgroup, bool, read_write> = access %wgvar, 2u
        store %9, false
        exit_if  # if_1
      }
    }
    %10:void = workgroupBarrier
    %11:Outer = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ArrayOfStructOfArrayOfStructWithAtomic) {
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), ty.i32()},
                                                          {mod.symbols.New("b"), ty.atomic<u32>()},
                                                      });
    auto* outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.New("c"), ty.f32()},
                                                {mod.symbols.New("inner"), ty.array(inner, 13)},
                                                {mod.symbols.New("d"), ty.bool_()},
                                            });
    auto* var = MakeVar("wgvar", ty.array(outer, 7));

    auto* func = MakeEntryPoint("main", 7, 3, 2);
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  a:i32 @offset(0)
  b:atomic<u32> @offset(4)
}

Outer = struct @align(4) {
  c:f32 @offset(0)
  inner:array<Inner, 13> @offset(4)
  d:bool @offset(108)
}

%b1 = block {  # root
  %wgvar:ptr<workgroup, array<Outer, 7>, read_write> = var
}

%main = @compute @workgroup_size(7, 3, 2) func():void -> %b2 {
  %b2 = block {
    %3:array<Outer, 7> = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  a:i32 @offset(0)
  b:atomic<u32> @offset(4)
}

Outer = struct @align(4) {
  c:f32 @offset(0)
  inner:array<Inner, 13> @offset(4)
  d:bool @offset(108)
}

%b1 = block {  # root
  %wgvar:ptr<workgroup, array<Outer, 7>, read_write> = var
}

%main = @compute @workgroup_size(7, 3, 2) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    loop [i: %b3, b: %b4, c: %b5] {  # loop_1
      %b3 = block {  # initializer
        next_iteration %b4 %tint_local_index
      }
      %b4 = block (%idx:u32) {  # body
        %5:bool = gte %idx:u32, 7u
        if %5 [t: %b6] {  # if_1
          %b6 = block {  # true
            exit_loop  # loop_1
          }
        }
        %6:ptr<workgroup, f32, read_write> = access %wgvar, %idx:u32, 0u
        store %6, 0.0f
        %7:ptr<workgroup, bool, read_write> = access %wgvar, %idx:u32, 2u
        store %7, false
        continue %b5
      }
      %b5 = block {  # continuing
        %8:u32 = add %idx:u32, 42u
        next_iteration %b4 %8
      }
    }
    loop [i: %b7, b: %b8, c: %b9] {  # loop_2
      %b7 = block {  # initializer
        next_iteration %b8 %tint_local_index
      }
      %b8 = block (%idx_1:u32) {  # body
        %10:bool = gte %idx_1:u32, 91u
        if %10 [t: %b10] {  # if_2
          %b10 = block {  # true
            exit_loop  # loop_2
          }
        }
        %11:u32 = mod %idx_1:u32, 13u
        %12:u32 = div %idx_1:u32, 13u
        %13:ptr<workgroup, i32, read_write> = access %wgvar, %12, 1u, %11, 0u
        store %13, 0i
        %14:u32 = mod %idx_1:u32, 13u
        %15:u32 = div %idx_1:u32, 13u
        %16:ptr<workgroup, atomic<u32>, read_write> = access %wgvar, %15, 1u, %14, 1u
        %17:void = atomicStore %16, 0u
        continue %b9
      }
      %b9 = block {  # continuing
        %18:u32 = add %idx_1:u32, 42u
        next_iteration %b8 %18
      }
    }
    %19:void = workgroupBarrier
    %20:array<Outer, 7> = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, MultipleVariables_DifferentIterationCounts) {
    auto* var_a = MakeVar("var_a", ty.bool_());
    auto* var_b = MakeVar("var_b", ty.array<i32, 4>());
    auto* var_c = MakeVar("var_c", ty.array(ty.array<u32, 5>(), 7));

    auto* func = MakeEntryPoint("main", 11, 2, 3);
    b.Append(func->Block(), [&] {  //
        b.Load(var_a);
        b.Load(var_b);
        b.Load(var_c);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %var_a:ptr<workgroup, bool, read_write> = var
  %var_b:ptr<workgroup, array<i32, 4>, read_write> = var
  %var_c:ptr<workgroup, array<array<u32, 5>, 7>, read_write> = var
}

%main = @compute @workgroup_size(11, 2, 3) func():void -> %b2 {
  %b2 = block {
    %5:bool = load %var_a
    %6:array<i32, 4> = load %var_b
    %7:array<array<u32, 5>, 7> = load %var_c
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %var_a:ptr<workgroup, bool, read_write> = var
  %var_b:ptr<workgroup, array<i32, 4>, read_write> = var
  %var_c:ptr<workgroup, array<array<u32, 5>, 7>, read_write> = var
}

%main = @compute @workgroup_size(11, 2, 3) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    %6:bool = eq %tint_local_index, 0u
    if %6 [t: %b3] {  # if_1
      %b3 = block {  # true
        store %var_a, false
        exit_if  # if_1
      }
    }
    loop [i: %b4, b: %b5, c: %b6] {  # loop_1
      %b4 = block {  # initializer
        next_iteration %b5 %tint_local_index
      }
      %b5 = block (%idx:u32) {  # body
        %8:bool = gte %idx:u32, 4u
        if %8 [t: %b7] {  # if_2
          %b7 = block {  # true
            exit_loop  # loop_1
          }
        }
        %9:ptr<workgroup, i32, read_write> = access %var_b, %idx:u32
        store %9, 0i
        continue %b6
      }
      %b6 = block {  # continuing
        %10:u32 = add %idx:u32, 66u
        next_iteration %b5 %10
      }
    }
    loop [i: %b8, b: %b9, c: %b10] {  # loop_2
      %b8 = block {  # initializer
        next_iteration %b9 %tint_local_index
      }
      %b9 = block (%idx_1:u32) {  # body
        %12:bool = gte %idx_1:u32, 35u
        if %12 [t: %b11] {  # if_3
          %b11 = block {  # true
            exit_loop  # loop_2
          }
        }
        %13:u32 = mod %idx_1:u32, 5u
        %14:u32 = div %idx_1:u32, 5u
        %15:ptr<workgroup, u32, read_write> = access %var_c, %14, %13
        store %15, 0u
        continue %b10
      }
      %b10 = block {  # continuing
        %16:u32 = add %idx_1:u32, 66u
        next_iteration %b9 %16
      }
    }
    %17:void = workgroupBarrier
    %18:bool = load %var_a
    %19:array<i32, 4> = load %var_b
    %20:array<array<u32, 5>, 7> = load %var_c
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, MultipleVariables_SharedIterationCounts) {
    auto* var_a = MakeVar("var_a", ty.bool_());
    auto* var_b = MakeVar("var_b", ty.i32());
    auto* var_c = MakeVar("var_c", ty.array<i32, 42>());
    auto* var_d = MakeVar("var_d", ty.array(ty.array<u32, 6>(), 7));

    auto* func = MakeEntryPoint("main", 11, 2, 3);
    b.Append(func->Block(), [&] {  //
        b.Load(var_a);
        b.Load(var_b);
        b.Load(var_c);
        b.Load(var_d);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %var_a:ptr<workgroup, bool, read_write> = var
  %var_b:ptr<workgroup, i32, read_write> = var
  %var_c:ptr<workgroup, array<i32, 42>, read_write> = var
  %var_d:ptr<workgroup, array<array<u32, 6>, 7>, read_write> = var
}

%main = @compute @workgroup_size(11, 2, 3) func():void -> %b2 {
  %b2 = block {
    %6:bool = load %var_a
    %7:i32 = load %var_b
    %8:array<i32, 42> = load %var_c
    %9:array<array<u32, 6>, 7> = load %var_d
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %var_a:ptr<workgroup, bool, read_write> = var
  %var_b:ptr<workgroup, i32, read_write> = var
  %var_c:ptr<workgroup, array<i32, 42>, read_write> = var
  %var_d:ptr<workgroup, array<array<u32, 6>, 7>, read_write> = var
}

%main = @compute @workgroup_size(11, 2, 3) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    %7:bool = eq %tint_local_index, 0u
    if %7 [t: %b3] {  # if_1
      %b3 = block {  # true
        store %var_a, false
        store %var_b, 0i
        exit_if  # if_1
      }
    }
    loop [i: %b4, b: %b5, c: %b6] {  # loop_1
      %b4 = block {  # initializer
        next_iteration %b5 %tint_local_index
      }
      %b5 = block (%idx:u32) {  # body
        %9:bool = gte %idx:u32, 42u
        if %9 [t: %b7] {  # if_2
          %b7 = block {  # true
            exit_loop  # loop_1
          }
        }
        %10:ptr<workgroup, i32, read_write> = access %var_c, %idx:u32
        store %10, 0i
        %11:u32 = mod %idx:u32, 6u
        %12:u32 = div %idx:u32, 6u
        %13:ptr<workgroup, u32, read_write> = access %var_d, %12, %11
        store %13, 0u
        continue %b6
      }
      %b6 = block {  # continuing
        %14:u32 = add %idx:u32, 66u
        next_iteration %b5 %14
      }
    }
    %15:void = workgroupBarrier
    %16:bool = load %var_a
    %17:i32 = load %var_b
    %18:array<i32, 42> = load %var_c
    %19:array<array<u32, 6>, 7> = load %var_d
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ExistingLocalInvocationIndex) {
    auto* var = MakeVar("wgvar", ty.bool_());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    auto* global_id = b.FunctionParam("global_id", ty.vec4<u32>());
    global_id->SetBuiltin(FunctionParam::Builtin::kGlobalInvocationId);
    auto* index = b.FunctionParam("index", ty.u32());
    index->SetBuiltin(FunctionParam::Builtin::kLocalInvocationIndex);
    func->SetParams({global_id, index});
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%global_id:vec4<u32> [@global_invocation_id], %index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    %5:bool = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%global_id:vec4<u32> [@global_invocation_id], %index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    %5:bool = eq %index, 0u
    if %5 [t: %b3] {  # if_1
      %b3 = block {  # true
        store %wgvar, false
        exit_if  # if_1
      }
    }
    %6:void = workgroupBarrier
    %7:bool = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, ExistingLocalInvocationIndexInStruct) {
    auto* var = MakeVar("wgvar", ty.bool_());

    auto* structure =
        ty.Struct(mod.symbols.New("MyStruct"),
                  {
                      {
                          mod.symbols.New("global_id"),
                          ty.vec3<u32>(),
                          {{}, {}, core::BuiltinValue::kGlobalInvocationId, {}, false},
                      },
                      {
                          mod.symbols.New("index"),
                          ty.u32(),
                          {{}, {}, core::BuiltinValue::kLocalInvocationIndex, {}, false},
                      },
                  });
    auto* func = MakeEntryPoint("main", 1, 1, 1);
    func->SetParams({b.FunctionParam("params", structure)});
    b.Append(func->Block(), [&] {  //
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  global_id:vec3<u32> @offset(0), @builtin(global_invocation_id)
  index:u32 @offset(12), @builtin(local_invocation_index)
}

%b1 = block {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%params:MyStruct):void -> %b2 {
  %b2 = block {
    %4:bool = load %wgvar
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(16) {
  global_id:vec3<u32> @offset(0), @builtin(global_invocation_id)
  index:u32 @offset(12), @builtin(local_invocation_index)
}

%b1 = block {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%params:MyStruct):void -> %b2 {
  %b2 = block {
    %4:u32 = access %params, 1u
    %5:bool = eq %4, 0u
    if %5 [t: %b3] {  # if_1
      %b3 = block {  # true
        store %wgvar, false
        exit_if  # if_1
      }
    }
    %6:void = workgroupBarrier
    %7:bool = load %wgvar
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, UseInsideNestedBlock) {
    auto* var = MakeVar("wgvar", ty.bool_());

    auto* func = MakeEntryPoint("main", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        auto* ifelse = b.If(true);
        b.Append(ifelse->True(), [&] {  //
            auto* sw = b.Switch(42_i);
            auto* def_case = b.Case(sw, Vector{core::ir::Switch::CaseSelector()});
            b.Append(def_case, [&] {  //
                auto* loop = b.Loop();
                b.Append(loop->Body(), [&] {  //
                    b.Continue(loop);
                    b.Append(loop->Continuing(), [&] {  //
                        auto* load = b.Load(var);
                        b.BreakIf(loop, load);
                    });
                });
                b.ExitSwitch(sw);
            });
            b.ExitIf(ifelse);
        });
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    if true [t: %b3] {  # if_1
      %b3 = block {  # true
        switch 42i [c: (default, %b4)] {  # switch_1
          %b4 = block {  # case
            loop [b: %b5, c: %b6] {  # loop_1
              %b5 = block {  # body
                continue %b6
              }
              %b6 = block {  # continuing
                %3:bool = load %wgvar
                break_if %3 %b5
              }
            }
            exit_switch  # switch_1
          }
        }
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%main = @compute @workgroup_size(1, 1, 1) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b2 {
  %b2 = block {
    %4:bool = eq %tint_local_index, 0u
    if %4 [t: %b3] {  # if_1
      %b3 = block {  # true
        store %wgvar, false
        exit_if  # if_1
      }
    }
    %5:void = workgroupBarrier
    if true [t: %b4] {  # if_2
      %b4 = block {  # true
        switch 42i [c: (default, %b5)] {  # switch_1
          %b5 = block {  # case
            loop [b: %b6, c: %b7] {  # loop_1
              %b6 = block {  # body
                continue %b7
              }
              %b7 = block {  # continuing
                %6:bool = load %wgvar
                break_if %6 %b6
              }
            }
            exit_switch  # switch_1
          }
        }
        exit_if  # if_2
      }
    }
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, UseInsideIndirectFunctionCall) {
    auto* var = MakeVar("wgvar", ty.bool_());

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {  //
            b.Continue(loop);
            b.Append(loop->Continuing(), [&] {  //
                auto* load = b.Load(var);
                b.BreakIf(loop, load);
            });
        });
        b.Return(foo);
    });

    auto* bar = b.Function("foo", ty.void_());
    b.Append(bar->Block(), [&] {  //
        auto* ifelse = b.If(true);
        b.Append(ifelse->True(), [&] {  //
            b.Call(ty.void_(), foo);
            b.ExitIf(ifelse);
        });
        b.Return(bar);
    });

    auto* func = MakeEntryPoint("func", 1, 1, 1);
    b.Append(func->Block(), [&] {  //
        auto* ifelse = b.If(true);
        b.Append(ifelse->True(), [&] {  //
            auto* sw = b.Switch(42_i);
            auto* def_case = b.Case(sw, Vector{core::ir::Switch::CaseSelector()});
            b.Append(def_case, [&] {  //
                auto* loop = b.Loop();
                b.Append(loop->Body(), [&] {  //
                    b.Continue(loop);
                    b.Append(loop->Continuing(), [&] {  //
                        b.Call(ty.void_(), bar);
                        b.BreakIf(loop, true);
                    });
                });
                b.ExitSwitch(sw);
            });
            b.ExitIf(ifelse);
        });
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%foo = func():void -> %b2 {
  %b2 = block {
    loop [b: %b3, c: %b4] {  # loop_1
      %b3 = block {  # body
        continue %b4
      }
      %b4 = block {  # continuing
        %3:bool = load %wgvar
        break_if %3 %b3
      }
    }
    ret
  }
}
%foo_1 = func():void -> %b5 {  # %foo_1: 'foo'
  %b5 = block {
    if true [t: %b6] {  # if_1
      %b6 = block {  # true
        %5:void = call %foo
        exit_if  # if_1
      }
    }
    ret
  }
}
%func = @compute @workgroup_size(1, 1, 1) func():void -> %b7 {
  %b7 = block {
    if true [t: %b8] {  # if_2
      %b8 = block {  # true
        switch 42i [c: (default, %b9)] {  # switch_1
          %b9 = block {  # case
            loop [b: %b10, c: %b11] {  # loop_2
              %b10 = block {  # body
                continue %b11
              }
              %b11 = block {  # continuing
                %7:void = call %foo_1
                break_if true %b10
              }
            }
            exit_switch  # switch_1
          }
        }
        exit_if  # if_2
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%foo = func():void -> %b2 {
  %b2 = block {
    loop [b: %b3, c: %b4] {  # loop_1
      %b3 = block {  # body
        continue %b4
      }
      %b4 = block {  # continuing
        %3:bool = load %wgvar
        break_if %3 %b3
      }
    }
    ret
  }
}
%foo_1 = func():void -> %b5 {  # %foo_1: 'foo'
  %b5 = block {
    if true [t: %b6] {  # if_1
      %b6 = block {  # true
        %5:void = call %foo
        exit_if  # if_1
      }
    }
    ret
  }
}
%func = @compute @workgroup_size(1, 1, 1) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b7 {
  %b7 = block {
    %8:bool = eq %tint_local_index, 0u
    if %8 [t: %b8] {  # if_2
      %b8 = block {  # true
        store %wgvar, false
        exit_if  # if_2
      }
    }
    %9:void = workgroupBarrier
    if true [t: %b9] {  # if_3
      %b9 = block {  # true
        switch 42i [c: (default, %b10)] {  # switch_1
          %b10 = block {  # case
            loop [b: %b11, c: %b12] {  # loop_2
              %b11 = block {  # body
                continue %b12
              }
              %b12 = block {  # continuing
                %10:void = call %foo_1
                break_if true %b11
              }
            }
            exit_switch  # switch_1
          }
        }
        exit_if  # if_3
      }
    }
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ZeroInitWorkgroupMemoryTest, MultipleEntryPoints_SameVarViaHelper) {
    auto* var = MakeVar("wgvar", ty.bool_());

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {  //
            b.Continue(loop);
            b.Append(loop->Continuing(), [&] {  //
                auto* load = b.Load(var);
                b.BreakIf(loop, load);
            });
        });
        b.Return(foo);
    });

    auto* ep1 = MakeEntryPoint("ep1", 1, 1, 1);
    b.Append(ep1->Block(), [&] {  //
        b.Call(ty.void_(), foo);
        b.Return(ep1);
    });

    auto* ep2 = MakeEntryPoint("ep2", 1, 1, 1);
    b.Append(ep2->Block(), [&] {  //
        b.Call(ty.void_(), foo);
        b.Return(ep2);
    });

    auto* src = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%foo = func():void -> %b2 {
  %b2 = block {
    loop [b: %b3, c: %b4] {  # loop_1
      %b3 = block {  # body
        continue %b4
      }
      %b4 = block {  # continuing
        %3:bool = load %wgvar
        break_if %3 %b3
      }
    }
    ret
  }
}
%ep1 = @compute @workgroup_size(1, 1, 1) func():void -> %b5 {
  %b5 = block {
    %5:void = call %foo
    ret
  }
}
%ep2 = @compute @workgroup_size(1, 1, 1) func():void -> %b6 {
  %b6 = block {
    %7:void = call %foo
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %wgvar:ptr<workgroup, bool, read_write> = var
}

%foo = func():void -> %b2 {
  %b2 = block {
    loop [b: %b3, c: %b4] {  # loop_1
      %b3 = block {  # body
        continue %b4
      }
      %b4 = block {  # continuing
        %3:bool = load %wgvar
        break_if %3 %b3
      }
    }
    ret
  }
}
%ep1 = @compute @workgroup_size(1, 1, 1) func(%tint_local_index:u32 [@local_invocation_index]):void -> %b5 {
  %b5 = block {
    %6:bool = eq %tint_local_index, 0u
    if %6 [t: %b6] {  # if_1
      %b6 = block {  # true
        store %wgvar, false
        exit_if  # if_1
      }
    }
    %7:void = workgroupBarrier
    %8:void = call %foo
    ret
  }
}
%ep2 = @compute @workgroup_size(1, 1, 1) func(%tint_local_index_1:u32 [@local_invocation_index]):void -> %b7 {  # %tint_local_index_1: 'tint_local_index'
  %b7 = block {
    %11:bool = eq %tint_local_index_1, 0u
    if %11 [t: %b8] {  # if_2
      %b8 = block {  # true
        store %wgvar, false
        exit_if  # if_2
      }
    }
    %12:void = workgroupBarrier
    %13:void = call %foo
    ret
  }
}
)";

    Run(ZeroInitWorkgroupMemory);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
