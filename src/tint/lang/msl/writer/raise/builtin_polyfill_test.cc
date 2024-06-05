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

#include "src/tint/lang/msl/writer/raise/builtin_polyfill.h"

#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/builtin_structs.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer::raise {
namespace {

using MslWriter_BuiltinPolyfillTest = core::ir::transform::TransformTest;

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicAdd_Workgroup_I32) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kAtomicAdd, a, 1_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = atomicAdd %a, 1i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = msl.atomic_fetch_add_explicit %a, 1i, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicAdd_Storage_U32) {
    auto* a = b.FunctionParam<ptr<storage, atomic<u32>>>("a");
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kAtomicAdd, a, 1_u);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<storage, atomic<u32>, read_write>):u32 {
  $B1: {
    %3:u32 = atomicAdd %a, 1u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<storage, atomic<u32>, read_write>):u32 {
  $B1: {
    %3:u32 = msl.atomic_fetch_add_explicit %a, 1u, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicAnd) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kAtomicAnd, a, 1_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = atomicAnd %a, 1i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = msl.atomic_fetch_and_explicit %a, 1i, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicExchange) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kAtomicExchange, a, 1_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = atomicExchange %a, 1i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = msl.atomic_exchange_explicit %a, 1i, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicLoad) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kAtomicLoad, a);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = atomicLoad %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = msl.atomic_load_explicit %a, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicMax) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kAtomicMax, a, 1_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = atomicMax %a, 1i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = msl.atomic_fetch_max_explicit %a, 1i, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicMin) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kAtomicMin, a, 1_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = atomicMin %a, 1i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = msl.atomic_fetch_min_explicit %a, 1i, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicOr) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kAtomicOr, a, 1_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = atomicOr %a, 1i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = msl.atomic_fetch_or_explicit %a, 1i, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicStore) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kAtomicStore, a, 1_i);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):void {
  $B1: {
    %3:void = atomicStore %a, 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):void {
  $B1: {
    %3:void = msl.atomic_store_explicit %a, 1i, 0u
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicSub) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kAtomicSub, a, 1_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = atomicSub %a, 1i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = msl.atomic_fetch_sub_explicit %a, 1i, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicXor) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<i32>(core::BuiltinFn::kAtomicXor, a, 1_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = atomicXor %a, 1i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:i32 = msl.atomic_fetch_xor_explicit %a, 1i, 0u
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, WorkgroupBarrier) {
    auto* func = b.Function("foo", ty.void_());
    func->SetStage(core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:void = workgroupBarrier
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:void = msl.threadgroup_barrier 4u
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, StorageBarrier) {
    auto* func = b.Function("foo", ty.void_());
    func->SetStage(core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kStorageBarrier);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:void = storageBarrier
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:void = msl.threadgroup_barrier 1u
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest, TextureBarrier) {
    auto* func = b.Function("foo", ty.void_());
    func->SetStage(core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureBarrier);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:void = textureBarrier
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:void = msl.threadgroup_barrier 2u
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::msl::writer::raise
