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

TEST_F(MslWriter_BuiltinPolyfillTest, AtomicCompareExchangeWeak) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                   core::BuiltinFn::kAtomicCompareExchangeWeak, a, 1_i, 2_i);
        auto* if_ = b.If(b.Access<bool>(result, 1_u));
        b.Append(if_->True(), [&] {  //
            b.Return(func, b.Access<i32>(result, 0_u));
        });
        b.Return(func, 42_i);
    });

    auto* src = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %a, 1i, 2i
    %4:bool = access %3, 1u
    if %4 [t: $B2] {  # if_1
      $B2: {  # true
        %5:i32 = access %3, 0u
        ret %5
      }
    }
    ret 42i
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:__atomic_compare_exchange_result_i32 = call %4, %a, 1i, 2i
    %5:bool = access %3, 1u
    if %5 [t: $B2] {  # if_1
      $B2: {  # true
        %6:i32 = access %3, 0u
        ret %6
      }
    }
    ret 42i
  }
}
%4 = func(%atomic_ptr:ptr<workgroup, atomic<i32>, read_write>, %cmp:i32, %val:i32):__atomic_compare_exchange_result_i32 {
  $B3: {
    %old_value:ptr<function, i32, read_write> = var, %cmp
    %11:bool = msl.atomic_compare_exchange_weak_explicit %atomic_ptr, %old_value, %val, 0u, 0u
    %12:i32 = load %old_value
    %13:__atomic_compare_exchange_result_i32 = construct %12, %11
    ret %13
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest,
       AtomicCompareExchangeWeak_Multiple_SameAddressSpace_SameType) {
    auto* a = b.FunctionParam<ptr<workgroup, atomic<i32>>>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result_a =
            b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                   core::BuiltinFn::kAtomicCompareExchangeWeak, a, 1_i, 2_i);
        auto* result_b =
            b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                   core::BuiltinFn::kAtomicCompareExchangeWeak, a, 3_i, 4_i);
        auto* if_ = b.If(b.Access<bool>(result_a, 1_u));
        b.Append(if_->True(), [&] {  //
            b.Return(func, b.Access<i32>(result_a, 0_u));
        });
        b.Return(func, b.Access<i32>(result_b, 0_u));
    });

    auto* src = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %a, 1i, 2i
    %4:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %a, 3i, 4i
    %5:bool = access %3, 1u
    if %5 [t: $B2] {  # if_1
      $B2: {  # true
        %6:i32 = access %3, 0u
        ret %6
      }
    }
    %7:i32 = access %4, 0u
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

%foo = func(%a:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B1: {
    %3:__atomic_compare_exchange_result_i32 = call %4, %a, 1i, 2i
    %5:__atomic_compare_exchange_result_i32 = call %4, %a, 3i, 4i
    %6:bool = access %3, 1u
    if %6 [t: $B2] {  # if_1
      $B2: {  # true
        %7:i32 = access %3, 0u
        ret %7
      }
    }
    %8:i32 = access %5, 0u
    ret %8
  }
}
%4 = func(%atomic_ptr:ptr<workgroup, atomic<i32>, read_write>, %cmp:i32, %val:i32):__atomic_compare_exchange_result_i32 {
  $B3: {
    %old_value:ptr<function, i32, read_write> = var, %cmp
    %13:bool = msl.atomic_compare_exchange_weak_explicit %atomic_ptr, %old_value, %val, 0u, 0u
    %14:i32 = load %old_value
    %15:__atomic_compare_exchange_result_i32 = construct %14, %13
    ret %15
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest,
       AtomicCompareExchangeWeak_Multiple_SameAddressSpace_DifferentType) {
    auto* ai = b.FunctionParam<ptr<workgroup, atomic<i32>>>("ai");
    auto* au = b.FunctionParam<ptr<workgroup, atomic<u32>>>("au");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({ai, au});
    b.Append(func->Block(), [&] {
        auto* result_a =
            b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                   core::BuiltinFn::kAtomicCompareExchangeWeak, ai, 1_i, 2_i);
        auto* result_b =
            b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.u32()),
                   core::BuiltinFn::kAtomicCompareExchangeWeak, au, 3_u, 4_u);
        auto* if_ = b.If(b.Access<bool>(result_a, 1_u));
        b.Append(if_->True(), [&] {  //
            b.Return(func, b.Access<i32>(result_a, 0_u));
        });
        b.Return(func, b.Convert<i32>(b.Access<u32>(result_b, 0_u)));
    });

    auto* src = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

__atomic_compare_exchange_result_u32 = struct @align(4) {
  old_value:u32 @offset(0)
  exchanged:bool @offset(4)
}

%foo = func(%ai:ptr<workgroup, atomic<i32>, read_write>, %au:ptr<workgroup, atomic<u32>, read_write>):i32 {
  $B1: {
    %4:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %ai, 1i, 2i
    %5:__atomic_compare_exchange_result_u32 = atomicCompareExchangeWeak %au, 3u, 4u
    %6:bool = access %4, 1u
    if %6 [t: $B2] {  # if_1
      $B2: {  # true
        %7:i32 = access %4, 0u
        ret %7
      }
    }
    %8:u32 = access %5, 0u
    %9:i32 = convert %8
    ret %9
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

__atomic_compare_exchange_result_u32 = struct @align(4) {
  old_value:u32 @offset(0)
  exchanged:bool @offset(4)
}

%foo = func(%ai:ptr<workgroup, atomic<i32>, read_write>, %au:ptr<workgroup, atomic<u32>, read_write>):i32 {
  $B1: {
    %4:__atomic_compare_exchange_result_i32 = call %5, %ai, 1i, 2i
    %6:__atomic_compare_exchange_result_u32 = call %7, %au, 3u, 4u
    %8:bool = access %4, 1u
    if %8 [t: $B2] {  # if_1
      $B2: {  # true
        %9:i32 = access %4, 0u
        ret %9
      }
    }
    %10:u32 = access %6, 0u
    %11:i32 = convert %10
    ret %11
  }
}
%5 = func(%atomic_ptr:ptr<workgroup, atomic<i32>, read_write>, %cmp:i32, %val:i32):__atomic_compare_exchange_result_i32 {
  $B3: {
    %old_value:ptr<function, i32, read_write> = var, %cmp
    %16:bool = msl.atomic_compare_exchange_weak_explicit %atomic_ptr, %old_value, %val, 0u, 0u
    %17:i32 = load %old_value
    %18:__atomic_compare_exchange_result_i32 = construct %17, %16
    ret %18
  }
}
%7 = func(%atomic_ptr_1:ptr<workgroup, atomic<u32>, read_write>, %cmp_1:u32, %val_1:u32):__atomic_compare_exchange_result_u32 {  # %atomic_ptr_1: 'atomic_ptr', %cmp_1: 'cmp', %val_1: 'val'
  $B4: {
    %old_value_1:ptr<function, u32, read_write> = var, %cmp_1  # %old_value_1: 'old_value'
    %23:bool = msl.atomic_compare_exchange_weak_explicit %atomic_ptr_1, %old_value_1, %val_1, 0u, 0u
    %24:u32 = load %old_value_1
    %25:__atomic_compare_exchange_result_u32 = construct %24, %23
    ret %25
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_BuiltinPolyfillTest,
       AtomicCompareExchangeWeak_Multiple_DifferentAddressSpace_SameType) {
    auto* aw = b.FunctionParam<ptr<workgroup, atomic<i32>>>("aw");
    auto* as = b.FunctionParam<ptr<storage, atomic<i32>>>("as");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({aw, as});
    b.Append(func->Block(), [&] {
        auto* result_a =
            b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                   core::BuiltinFn::kAtomicCompareExchangeWeak, aw, 1_i, 2_i);
        auto* result_b =
            b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                   core::BuiltinFn::kAtomicCompareExchangeWeak, as, 3_i, 4_i);
        auto* if_ = b.If(b.Access<bool>(result_a, 1_u));
        b.Append(if_->True(), [&] {  //
            b.Return(func, b.Access<i32>(result_a, 0_u));
        });
        b.Return(func, b.Access<i32>(result_b, 0_u));
    });

    auto* src = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

%foo = func(%aw:ptr<workgroup, atomic<i32>, read_write>, %as:ptr<storage, atomic<i32>, read_write>):i32 {
  $B1: {
    %4:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %aw, 1i, 2i
    %5:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %as, 3i, 4i
    %6:bool = access %4, 1u
    if %6 [t: $B2] {  # if_1
      $B2: {  # true
        %7:i32 = access %4, 0u
        ret %7
      }
    }
    %8:i32 = access %5, 0u
    ret %8
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

%foo = func(%aw:ptr<workgroup, atomic<i32>, read_write>, %as:ptr<storage, atomic<i32>, read_write>):i32 {
  $B1: {
    %4:__atomic_compare_exchange_result_i32 = call %5, %aw, 1i, 2i
    %6:__atomic_compare_exchange_result_i32 = call %7, %as, 3i, 4i
    %8:bool = access %4, 1u
    if %8 [t: $B2] {  # if_1
      $B2: {  # true
        %9:i32 = access %4, 0u
        ret %9
      }
    }
    %10:i32 = access %6, 0u
    ret %10
  }
}
%5 = func(%atomic_ptr:ptr<workgroup, atomic<i32>, read_write>, %cmp:i32, %val:i32):__atomic_compare_exchange_result_i32 {
  $B3: {
    %old_value:ptr<function, i32, read_write> = var, %cmp
    %15:bool = msl.atomic_compare_exchange_weak_explicit %atomic_ptr, %old_value, %val, 0u, 0u
    %16:i32 = load %old_value
    %17:__atomic_compare_exchange_result_i32 = construct %16, %15
    ret %17
  }
}
%7 = func(%atomic_ptr_1:ptr<storage, atomic<i32>, read_write>, %cmp_1:i32, %val_1:i32):__atomic_compare_exchange_result_i32 {  # %atomic_ptr_1: 'atomic_ptr', %cmp_1: 'cmp', %val_1: 'val'
  $B4: {
    %old_value_1:ptr<function, i32, read_write> = var, %cmp_1  # %old_value_1: 'old_value'
    %22:bool = msl.atomic_compare_exchange_weak_explicit %atomic_ptr_1, %old_value_1, %val_1, 0u, 0u
    %23:i32 = load %old_value_1
    %24:__atomic_compare_exchange_result_i32 = construct %23, %22
    ret %24
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
