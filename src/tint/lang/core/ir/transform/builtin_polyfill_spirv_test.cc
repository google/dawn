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

#include "src/tint/lang/core/ir/transform/builtin_polyfill_spirv.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"

namespace tint::ir::transform {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

using IR_BuiltinPolyfillSpirvTest = TransformTest;

TEST_F(IR_BuiltinPolyfillSpirvTest, ArrayLength) {
    auto* arr = ty.runtime_array(ty.i32());
    auto* str_ty = ty.Struct(mod.symbols.New("Buffer"), {
                                                            {mod.symbols.New("a"), ty.i32()},
                                                            {mod.symbols.New("b"), ty.i32()},
                                                            {mod.symbols.New("arr"), arr},
                                                        });
    auto* var = b.Var("var", ty.ptr(storage, str_ty));
    var->SetBindingPoint(0, 0);
    b.RootBlock()->Append(var);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(storage, arr), var, 2_u);
        auto* result = b.Call(ty.u32(), builtin::Function::kArrayLength, access);
        b.Return(func, result);
    });

    auto* src = R"(
Buffer = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
  arr:array<i32> @offset(8)
}

%b1 = block {  # root
  %var:ptr<storage, Buffer, read_write> = var @binding_point(0, 0)
}

%foo = func():u32 -> %b2 {
  %b2 = block {
    %3:ptr<storage, array<i32>, read_write> = access %var, 2u
    %4:u32 = arrayLength %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Buffer = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
  arr:array<i32> @offset(8)
}

%b1 = block {  # root
  %var:ptr<storage, Buffer, read_write> = var @binding_point(0, 0)
}

%foo = func():u32 -> %b2 {
  %b2 = block {
    %3:ptr<storage, array<i32>, read_write> = access %var, 2u
    %4:u32 = spirv.array_length %var, 2u
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, ArrayLength_ViaLet_BeforeAccess) {
    auto* arr = ty.runtime_array(ty.i32());
    auto* str_ty = ty.Struct(mod.symbols.New("Buffer"), {
                                                            {mod.symbols.New("a"), ty.i32()},
                                                            {mod.symbols.New("b"), ty.i32()},
                                                            {mod.symbols.New("arr"), arr},
                                                        });
    auto* var = b.Var("var", ty.ptr(storage, str_ty));
    var->SetBindingPoint(0, 0);
    b.RootBlock()->Append(var);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* let_a = b.Let("a", var);
        auto* let_b = b.Let("b", let_a);
        auto* access = b.Access(ty.ptr(storage, arr), let_b, 2_u);
        auto* result = b.Call(ty.u32(), builtin::Function::kArrayLength, access);
        b.Return(func, result);
    });

    auto* src = R"(
Buffer = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
  arr:array<i32> @offset(8)
}

%b1 = block {  # root
  %var:ptr<storage, Buffer, read_write> = var @binding_point(0, 0)
}

%foo = func():u32 -> %b2 {
  %b2 = block {
    %a:ptr<storage, Buffer, read_write> = let %var
    %b:ptr<storage, Buffer, read_write> = let %a
    %5:ptr<storage, array<i32>, read_write> = access %b, 2u
    %6:u32 = arrayLength %5
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Buffer = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
  arr:array<i32> @offset(8)
}

%b1 = block {  # root
  %var:ptr<storage, Buffer, read_write> = var @binding_point(0, 0)
}

%foo = func():u32 -> %b2 {
  %b2 = block {
    %a:ptr<storage, Buffer, read_write> = let %var
    %b:ptr<storage, Buffer, read_write> = let %a
    %5:ptr<storage, array<i32>, read_write> = access %b, 2u
    %6:u32 = spirv.array_length %b, 2u
    ret %6
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, ArrayLength_ViaLet_AfterAccess) {
    auto* arr = ty.runtime_array(ty.i32());
    auto* str_ty = ty.Struct(mod.symbols.New("Buffer"), {
                                                            {mod.symbols.New("a"), ty.i32()},
                                                            {mod.symbols.New("b"), ty.i32()},
                                                            {mod.symbols.New("arr"), arr},
                                                        });
    auto* var = b.Var("var", ty.ptr(storage, str_ty));
    var->SetBindingPoint(0, 0);
    b.RootBlock()->Append(var);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(storage, arr), var, 2_u);
        auto* let_a = b.Let("a", access);
        auto* let_b = b.Let("b", let_a);
        auto* result = b.Call(ty.u32(), builtin::Function::kArrayLength, let_b);
        b.Return(func, result);
    });

    auto* src = R"(
Buffer = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
  arr:array<i32> @offset(8)
}

%b1 = block {  # root
  %var:ptr<storage, Buffer, read_write> = var @binding_point(0, 0)
}

%foo = func():u32 -> %b2 {
  %b2 = block {
    %3:ptr<storage, array<i32>, read_write> = access %var, 2u
    %a:ptr<storage, array<i32>, read_write> = let %3
    %b:ptr<storage, array<i32>, read_write> = let %a
    %6:u32 = arrayLength %b
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Buffer = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
  arr:array<i32> @offset(8)
}

%b1 = block {  # root
  %var:ptr<storage, Buffer, read_write> = var @binding_point(0, 0)
}

%foo = func():u32 -> %b2 {
  %b2 = block {
    %3:ptr<storage, array<i32>, read_write> = access %var, 2u
    %a:ptr<storage, array<i32>, read_write> = let %3
    %b:ptr<storage, array<i32>, read_write> = let %a
    %6:u32 = spirv.array_length %var, 2u
    ret %6
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, AtomicAdd_Storage) {
    auto* var = b.Var(ty.ptr(storage, ty.atomic(ty.i32())));
    var->SetBindingPoint(0, 0);
    b.RootBlock()->Append(var);

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kAtomicAdd, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<storage, atomic<i32>, read_write> = var @binding_point(0, 0)
}

%foo = func(%arg1:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = atomicAdd %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<storage, atomic<i32>, read_write> = var @binding_point(0, 0)
}

%foo = func(%arg1:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = spirv.atomic_iadd %1, 1u, 0u, %arg1
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, AtomicAdd_Workgroup) {
    auto* var = b.RootBlock()->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kAtomicAdd, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%arg1:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = atomicAdd %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%arg1:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = spirv.atomic_iadd %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, AtomicAnd) {
    auto* var = b.RootBlock()->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kAtomicAnd, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%arg1:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = atomicAnd %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%arg1:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = spirv.atomic_and %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, AtomicCompareExchangeWeak) {
    auto* var = b.RootBlock()->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* cmp = b.FunctionParam("cmp", ty.i32());
    auto* val = b.FunctionParam("val", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cmp, val});

    b.Append(func->Block(), [&] {
        auto* result_ty = type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32());
        auto* result =
            b.Call(result_ty, builtin::Function::kAtomicCompareExchangeWeak, var, cmp, val);
        b.Return(func, b.Access(ty.i32(), result, 0_u));
    });

    auto* src = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%cmp:i32, %val:i32):i32 -> %b2 {
  %b2 = block {
    %5:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %1, %cmp, %val
    %6:i32 = access %5, 0u
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%cmp:i32, %val:i32):i32 -> %b2 {
  %b2 = block {
    %5:i32 = spirv.atomic_compare_exchange %1, 2u, 0u, 0u, %val, %cmp
    %6:bool = eq %5, %cmp
    %7:__atomic_compare_exchange_result_i32 = construct %5, %6
    %8:i32 = access %7, 0u
    ret %8
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, AtomicExchange) {
    auto* var = b.RootBlock()->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kAtomicExchange, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%arg1:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = atomicExchange %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%arg1:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = spirv.atomic_exchange %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, AtomicLoad) {
    auto* var = b.RootBlock()->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* func = b.Function("foo", ty.i32());

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kAtomicLoad, var);
        b.Return(func, result);
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func():i32 -> %b2 {
  %b2 = block {
    %3:i32 = atomicLoad %1
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func():i32 -> %b2 {
  %b2 = block {
    %3:i32 = spirv.atomic_load %1, 2u, 0u
    ret %3
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, AtomicMax_I32) {
    auto* var = b.RootBlock()->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kAtomicMax, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%arg1:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = atomicMax %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%arg1:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = spirv.atomic_smax %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, AtomicMax_U32) {
    auto* var = b.RootBlock()->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.u32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.u32());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), builtin::Function::kAtomicMax, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<u32>, read_write> = var
}

%foo = func(%arg1:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = atomicMax %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<u32>, read_write> = var
}

%foo = func(%arg1:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = spirv.atomic_umax %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, AtomicMin_I32) {
    auto* var = b.RootBlock()->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kAtomicMin, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%arg1:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = atomicMin %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%arg1:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = spirv.atomic_smin %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, AtomicMin_U32) {
    auto* var = b.RootBlock()->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.u32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.u32());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), builtin::Function::kAtomicMin, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<u32>, read_write> = var
}

%foo = func(%arg1:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = atomicMin %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<u32>, read_write> = var
}

%foo = func(%arg1:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = spirv.atomic_umin %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, AtomicOr) {
    auto* var = b.RootBlock()->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kAtomicOr, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%arg1:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = atomicOr %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%arg1:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = spirv.atomic_or %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, AtomicStore) {
    auto* var = b.RootBlock()->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), builtin::Function::kAtomicStore, var, arg1);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%arg1:i32):void -> %b2 {
  %b2 = block {
    %4:void = atomicStore %1, %arg1
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%arg1:i32):void -> %b2 {
  %b2 = block {
    %4:void = spirv.atomic_store %1, 2u, 0u, %arg1
    ret
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, AtomicSub) {
    auto* var = b.RootBlock()->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kAtomicSub, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%arg1:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = atomicSub %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%arg1:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = spirv.atomic_isub %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, AtomicXor) {
    auto* var = b.RootBlock()->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kAtomicXor, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%arg1:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = atomicXor %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var
}

%foo = func(%arg1:i32):i32 -> %b2 {
  %b2 = block {
    %4:i32 = spirv.atomic_xor %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, Dot_Vec4f) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg1, arg2});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), builtin::Function::kDot, arg1, arg2);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg1:vec4<f32>, %arg2:vec4<f32>):f32 -> %b1 {
  %b1 = block {
    %4:f32 = dot %arg1, %arg2
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg1:vec4<f32>, %arg2:vec4<f32>):f32 -> %b1 {
  %b1 = block {
    %4:f32 = spirv.dot %arg1, %arg2
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, Dot_Vec2i) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec2<i32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec2<i32>());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1, arg2});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kDot, arg1, arg2);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg1:vec2<i32>, %arg2:vec2<i32>):i32 -> %b1 {
  %b1 = block {
    %4:i32 = dot %arg1, %arg2
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg1:vec2<i32>, %arg2:vec2<i32>):i32 -> %b1 {
  %b1 = block {
    %4:i32 = access %arg1, 0u
    %5:i32 = access %arg2, 0u
    %6:i32 = mul %4, %5
    %7:i32 = access %arg1, 1u
    %8:i32 = access %arg2, 1u
    %9:i32 = mul %7, %8
    %10:i32 = add %6, %9
    ret %10
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, Dot_Vec4u) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<u32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<u32>());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg1, arg2});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), builtin::Function::kDot, arg1, arg2);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg1:vec4<u32>, %arg2:vec4<u32>):u32 -> %b1 {
  %b1 = block {
    %4:u32 = dot %arg1, %arg2
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg1:vec4<u32>, %arg2:vec4<u32>):u32 -> %b1 {
  %b1 = block {
    %4:u32 = access %arg1, 0u
    %5:u32 = access %arg2, 0u
    %6:u32 = mul %4, %5
    %7:u32 = access %arg1, 1u
    %8:u32 = access %arg2, 1u
    %9:u32 = mul %7, %8
    %10:u32 = add %6, %9
    %11:u32 = access %arg1, 2u
    %12:u32 = access %arg2, 2u
    %13:u32 = mul %11, %12
    %14:u32 = add %10, %13
    %15:u32 = access %arg1, 3u
    %16:u32 = access %arg2, 3u
    %17:u32 = mul %15, %16
    %18:u32 = add %14, %17
    ret %18
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, Select_ScalarCondition_ScalarOperands) {
    auto* argf = b.FunctionParam("argf", ty.i32());
    auto* argt = b.FunctionParam("argt", ty.i32());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({argf, argt, cond});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kSelect, argf, argt, cond);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%argf:i32, %argt:i32, %cond:bool):i32 -> %b1 {
  %b1 = block {
    %5:i32 = select %argf, %argt, %cond
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%argf:i32, %argt:i32, %cond:bool):i32 -> %b1 {
  %b1 = block {
    %5:i32 = spirv.select %cond, %argt, %argf
    ret %5
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, Select_VectorCondition_VectorOperands) {
    auto* argf = b.FunctionParam("argf", ty.vec4<i32>());
    auto* argt = b.FunctionParam("argt", ty.vec4<i32>());
    auto* cond = b.FunctionParam("cond", ty.vec4<bool>());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({argf, argt, cond});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<i32>(), builtin::Function::kSelect, argf, argt, cond);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%argf:vec4<i32>, %argt:vec4<i32>, %cond:vec4<bool>):vec4<i32> -> %b1 {
  %b1 = block {
    %5:vec4<i32> = select %argf, %argt, %cond
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%argf:vec4<i32>, %argt:vec4<i32>, %cond:vec4<bool>):vec4<i32> -> %b1 {
  %b1 = block {
    %5:vec4<i32> = spirv.select %cond, %argt, %argf
    ret %5
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, Select_ScalarCondition_VectorOperands) {
    auto* argf = b.FunctionParam("argf", ty.vec4<i32>());
    auto* argt = b.FunctionParam("argt", ty.vec4<i32>());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({argf, argt, cond});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<i32>(), builtin::Function::kSelect, argf, argt, cond);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%argf:vec4<i32>, %argt:vec4<i32>, %cond:bool):vec4<i32> -> %b1 {
  %b1 = block {
    %5:vec4<i32> = select %argf, %argt, %cond
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%argf:vec4<i32>, %argt:vec4<i32>, %cond:bool):vec4<i32> -> %b1 {
  %b1 = block {
    %5:vec4<bool> = construct %cond, %cond, %cond, %cond
    %6:vec4<i32> = spirv.select %5, %argt, %argf
    ret %6
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureLoad_2D) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* lod = b.FunctionParam("lod", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, coords, lod});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), builtin::Function::kTextureLoad, t, coords, lod);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %coords:vec2<i32>, %lod:i32):vec4<f32> -> %b1 {
  %b1 = block {
    %5:vec4<f32> = textureLoad %t, %coords, %lod
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %coords:vec2<i32>, %lod:i32):vec4<f32> -> %b1 {
  %b1 = block {
    %5:vec4<f32> = spirv.image_fetch %t, %coords, 2u, %lod
    ret %5
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureLoad_2DArray) {
    auto* t = b.FunctionParam(
        "t", ty.Get<type::SampledTexture>(type::TextureDimension::k2dArray, ty.f32()));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* lod = b.FunctionParam("lod", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, coords, array_idx, lod});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), builtin::Function::kTextureLoad, t, coords, array_idx, lod);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %coords:vec2<i32>, %array_idx:i32, %lod:i32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:vec4<f32> = textureLoad %t, %coords, %array_idx, %lod
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>, %coords:vec2<i32>, %array_idx:i32, %lod:i32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:vec3<i32> = construct %coords, %array_idx
    %7:vec4<f32> = spirv.image_fetch %t, %6, 2u, %lod
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureLoad_2DArray_IndexDifferentType) {
    auto* t = b.FunctionParam(
        "t", ty.Get<type::SampledTexture>(type::TextureDimension::k2dArray, ty.f32()));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.u32());
    auto* lod = b.FunctionParam("lod", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, coords, array_idx, lod});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), builtin::Function::kTextureLoad, t, coords, array_idx, lod);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %coords:vec2<i32>, %array_idx:u32, %lod:i32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:vec4<f32> = textureLoad %t, %coords, %array_idx, %lod
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>, %coords:vec2<i32>, %array_idx:u32, %lod:i32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:i32 = convert %array_idx
    %7:vec3<i32> = construct %coords, %6
    %8:vec4<f32> = spirv.image_fetch %t, %7, 2u, %lod
    ret %8
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureLoad_Multisampled2D) {
    auto* t = b.FunctionParam(
        "t", ty.Get<type::MultisampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* sample_idx = b.FunctionParam("sample_idx", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, coords, sample_idx});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), builtin::Function::kTextureLoad, t, coords, sample_idx);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_multisampled_2d<f32>, %coords:vec2<i32>, %sample_idx:i32):vec4<f32> -> %b1 {
  %b1 = block {
    %5:vec4<f32> = textureLoad %t, %coords, %sample_idx
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_multisampled_2d<f32>, %coords:vec2<i32>, %sample_idx:i32):vec4<f32> -> %b1 {
  %b1 = block {
    %5:vec4<f32> = spirv.image_fetch %t, %coords, 64u, %sample_idx
    ret %5
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureLoad_Depth2D) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::k2d));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* lod = b.FunctionParam("lod", ty.i32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, coords, lod});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), builtin::Function::kTextureLoad, t, coords, lod);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %coords:vec2<i32>, %lod:i32):f32 -> %b1 {
  %b1 = block {
    %5:f32 = textureLoad %t, %coords, %lod
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d, %coords:vec2<i32>, %lod:i32):f32 -> %b1 {
  %b1 = block {
    %5:vec4<f32> = spirv.image_fetch %t, %coords, 2u, %lod
    %6:f32 = access %5, 0u
    ret %6
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSample_1D) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k1d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), builtin::Function::kTextureSample, t, s, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_1d<f32>, %s:sampler, %coords:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %5:vec4<f32> = textureSample %t, %s, %coords
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_1d<f32>, %s:sampler, %coords:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %5:spirv.sampled_image = spirv.sampled_image %t, %s
    %6:vec4<f32> = spirv.image_sample_implicit_lod %5, %coords, 0u
    ret %6
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSample_2D) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), builtin::Function::kTextureSample, t, s, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %5:vec4<f32> = textureSample %t, %s, %coords
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %5:spirv.sampled_image = spirv.sampled_image %t, %s
    %6:vec4<f32> = spirv.image_sample_implicit_lod %5, %coords, 0u
    ret %6
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSample_2D_Offset) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureSample, t, s, coords,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %5:vec4<f32> = textureSample %t, %s, %coords, vec2<i32>(1i)
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %5:spirv.sampled_image = spirv.sampled_image %t, %s
    %6:vec4<f32> = spirv.image_sample_implicit_lod %5, %coords, 8u, vec2<i32>(1i)
    ret %6
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSample_2DArray_Offset) {
    auto* t = b.FunctionParam(
        "t", ty.Get<type::SampledTexture>(type::TextureDimension::k2dArray, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, array_idx});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureSample, t, s, coords, array_idx,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:vec4<f32> = textureSample %t, %s, %coords, %array_idx, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:f32 = convert %array_idx
    %8:vec3<f32> = construct %coords, %7
    %9:vec4<f32> = spirv.image_sample_implicit_lod %6, %8, 8u, vec2<i32>(1i)
    ret %9
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleBias_2D) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* bias = b.FunctionParam("bias", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, bias});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), builtin::Function::kTextureSampleBias, t, s, coords, bias);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %bias:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:vec4<f32> = textureSampleBias %t, %s, %coords, %bias
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %bias:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:vec4<f32> = spirv.image_sample_implicit_lod %6, %coords, 1u, %bias
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleBias_2D_Offset) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* bias = b.FunctionParam("bias", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, bias});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureSampleBias, t, s, coords, bias,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %bias:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:vec4<f32> = textureSampleBias %t, %s, %coords, %bias, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %bias:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:vec4<f32> = spirv.image_sample_implicit_lod %6, %coords, 9u, %bias, vec2<i32>(1i)
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleBias_2DArray_Offset) {
    auto* t = b.FunctionParam(
        "t", ty.Get<type::SampledTexture>(type::TextureDimension::k2dArray, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* bias = b.FunctionParam("bias", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, array_idx, bias});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureSampleBias, t, s, coords, array_idx, bias,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %bias:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %7:vec4<f32> = textureSampleBias %t, %s, %coords, %array_idx, %bias, vec2<i32>(1i)
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %bias:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %7:spirv.sampled_image = spirv.sampled_image %t, %s
    %8:f32 = convert %array_idx
    %9:vec3<f32> = construct %coords, %8
    %10:vec4<f32> = spirv.image_sample_implicit_lod %7, %9, 9u, %bias, vec2<i32>(1i)
    ret %10
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleCompare_2D) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* dref = b.FunctionParam("dref", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, dref});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.f32(), builtin::Function::kTextureSampleCompare, t, s, coords, dref);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %dref:f32):f32 -> %b1 {
  %b1 = block {
    %6:f32 = textureSampleCompare %t, %s, %coords, %dref
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %dref:f32):f32 -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:f32 = spirv.image_sample_dref_implicit_lod %6, %coords, %dref, 0u
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleCompare_2D_Offset) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* dref = b.FunctionParam("dref", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, dref});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(
            ty.f32(), builtin::Function::kTextureSampleCompare, t, s, coords, dref,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %dref:f32):f32 -> %b1 {
  %b1 = block {
    %6:f32 = textureSampleCompare %t, %s, %coords, %dref, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %dref:f32):f32 -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:f32 = spirv.image_sample_dref_implicit_lod %6, %coords, %dref, 8u, vec2<i32>(1i)
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleCompare_2DArray_Offset) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::k2dArray));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* bias = b.FunctionParam("bias", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, array_idx, bias});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(
            ty.f32(), builtin::Function::kTextureSampleCompare, t, s, coords, array_idx, bias,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d_array, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %bias:f32):f32 -> %b1 {
  %b1 = block {
    %7:f32 = textureSampleCompare %t, %s, %coords, %array_idx, %bias, vec2<i32>(1i)
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d_array, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %bias:f32):f32 -> %b1 {
  %b1 = block {
    %7:spirv.sampled_image = spirv.sampled_image %t, %s
    %8:f32 = convert %array_idx
    %9:vec3<f32> = construct %coords, %8
    %10:f32 = spirv.image_sample_dref_implicit_lod %7, %9, %bias, 8u, vec2<i32>(1i)
    ret %10
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleCompareLevel_2D) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* dref = b.FunctionParam("dref", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, dref});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.f32(), builtin::Function::kTextureSampleCompareLevel, t, s, coords, dref);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %dref:f32):f32 -> %b1 {
  %b1 = block {
    %6:f32 = textureSampleCompareLevel %t, %s, %coords, %dref
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %dref:f32):f32 -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:f32 = spirv.image_sample_dref_implicit_lod %6, %coords, %dref, 2u, 0.0f
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleCompareLevel_2D_Offset) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* dref = b.FunctionParam("dref", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, dref});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(
            ty.f32(), builtin::Function::kTextureSampleCompareLevel, t, s, coords, dref,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %dref:f32):f32 -> %b1 {
  %b1 = block {
    %6:f32 = textureSampleCompareLevel %t, %s, %coords, %dref, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %dref:f32):f32 -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:f32 = spirv.image_sample_dref_implicit_lod %6, %coords, %dref, 10u, 0.0f, vec2<i32>(1i)
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleCompareLevel_2DArray_Offset) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::k2dArray));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* bias = b.FunctionParam("bias", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, array_idx, bias});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(
            ty.f32(), builtin::Function::kTextureSampleCompareLevel, t, s, coords, array_idx, bias,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d_array, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %bias:f32):f32 -> %b1 {
  %b1 = block {
    %7:f32 = textureSampleCompareLevel %t, %s, %coords, %array_idx, %bias, vec2<i32>(1i)
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d_array, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %bias:f32):f32 -> %b1 {
  %b1 = block {
    %7:spirv.sampled_image = spirv.sampled_image %t, %s
    %8:f32 = convert %array_idx
    %9:vec3<f32> = construct %coords, %8
    %10:f32 = spirv.image_sample_dref_implicit_lod %7, %9, %bias, 10u, 0.0f, vec2<i32>(1i)
    ret %10
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleGrad_2D) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* ddx = b.FunctionParam("ddx", ty.vec2<f32>());
    auto* ddy = b.FunctionParam("ddy", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, ddx, ddy});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), builtin::Function::kTextureSampleBias, t, s, coords, ddx, ddy);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %7:vec4<f32> = textureSampleBias %t, %s, %coords, %ddx, %ddy
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %7:spirv.sampled_image = spirv.sampled_image %t, %s
    %8:vec4<f32> = spirv.image_sample_implicit_lod %7, %coords, 9u, %ddx, %ddy
    ret %8
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleGrad_2D_Offset) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* ddx = b.FunctionParam("ddx", ty.vec2<f32>());
    auto* ddy = b.FunctionParam("ddy", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, ddx, ddy});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureSampleBias, t, s, coords, ddx, ddy,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %7:vec4<f32> = textureSampleBias %t, %s, %coords, %ddx, %ddy, vec2<i32>(1i)
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %7:spirv.sampled_image = spirv.sampled_image %t, %s
    %8:vec4<f32> = spirv.image_sample_implicit_lod %7, %coords, 9u, %ddx, %ddy
    ret %8
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleGrad_2DArray_Offset) {
    auto* t = b.FunctionParam(
        "t", ty.Get<type::SampledTexture>(type::TextureDimension::k2dArray, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* ddx = b.FunctionParam("ddx", ty.vec2<f32>());
    auto* ddy = b.FunctionParam("ddy", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, array_idx, ddx, ddy});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureSampleBias, t, s, coords, array_idx, ddx,
            ddy,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %8:vec4<f32> = textureSampleBias %t, %s, %coords, %array_idx, %ddx, %ddy, vec2<i32>(1i)
    ret %8
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %8:spirv.sampled_image = spirv.sampled_image %t, %s
    %9:f32 = convert %array_idx
    %10:vec3<f32> = construct %coords, %9
    %11:vec4<f32> = spirv.image_sample_implicit_lod %8, %10, 9u, %ddx, %ddy
    ret %11
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleLevel_2D) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* lod = b.FunctionParam("lod", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, lod});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), builtin::Function::kTextureSampleLevel, t, s, coords, lod);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %lod:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:vec4<f32> = textureSampleLevel %t, %s, %coords, %lod
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %lod:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:vec4<f32> = spirv.image_sample_explicit_lod %6, %coords, 2u, %lod
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleLevel_2D_Offset) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* lod = b.FunctionParam("lod", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, lod});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureSampleLevel, t, s, coords, lod,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %lod:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:vec4<f32> = textureSampleLevel %t, %s, %coords, %lod, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %lod:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:vec4<f32> = spirv.image_sample_explicit_lod %6, %coords, 10u, %lod, vec2<i32>(1i)
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleLevel_2DArray_Offset) {
    auto* t = b.FunctionParam(
        "t", ty.Get<type::SampledTexture>(type::TextureDimension::k2dArray, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* lod = b.FunctionParam("lod", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, array_idx, lod});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureSampleLevel, t, s, coords, array_idx, lod,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %lod:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %7:vec4<f32> = textureSampleLevel %t, %s, %coords, %array_idx, %lod, vec2<i32>(1i)
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %lod:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %7:spirv.sampled_image = spirv.sampled_image %t, %s
    %8:f32 = convert %array_idx
    %9:vec3<f32> = construct %coords, %8
    %10:vec4<f32> = spirv.image_sample_explicit_lod %7, %9, 10u, %lod, vec2<i32>(1i)
    ret %10
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureGather_2D) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* component = b.FunctionParam("component", ty.i32());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({component, t, s, coords});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), builtin::Function::kTextureGather, component, t, s, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%component:i32, %t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %6:vec4<f32> = textureGather %component, %t, %s, %coords
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%component:i32, %t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:vec4<f32> = spirv.image_gather %6, %coords, %component, 0u
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureGather_2D_Offset) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* component = b.FunctionParam("component", ty.i32());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, component, coords});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureGather, component, t, s, coords,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %component:i32, %coords:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %6:vec4<f32> = textureGather %component, %t, %s, %coords, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %component:i32, %coords:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:vec4<f32> = spirv.image_gather %6, %coords, %component, 8u, vec2<i32>(1i)
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureGather_2DArray_Offset) {
    auto* t = b.FunctionParam(
        "t", ty.Get<type::SampledTexture>(type::TextureDimension::k2dArray, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* component = b.FunctionParam("component", ty.i32());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, component, coords, array_idx});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureGather, component, t, s, coords, array_idx,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %component:i32, %coords:vec2<f32>, %array_idx:i32):vec4<f32> -> %b1 {
  %b1 = block {
    %7:vec4<f32> = textureGather %component, %t, %s, %coords, %array_idx, vec2<i32>(1i)
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %component:i32, %coords:vec2<f32>, %array_idx:i32):vec4<f32> -> %b1 {
  %b1 = block {
    %7:spirv.sampled_image = spirv.sampled_image %t, %s
    %8:f32 = convert %array_idx
    %9:vec3<f32> = construct %coords, %8
    %10:vec4<f32> = spirv.image_gather %7, %9, %component, 8u, vec2<i32>(1i)
    ret %10
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureGather_Depth2D) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), builtin::Function::kTextureGather, t, s, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %5:vec4<f32> = textureGather %t, %s, %coords
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %5:spirv.sampled_image = spirv.sampled_image %t, %s
    %6:vec4<f32> = spirv.image_gather %5, %coords, 0u, 0u
    ret %6
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureGatherCompare_Depth2D) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* depth = b.FunctionParam("depth", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, depth});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), builtin::Function::kTextureGatherCompare, t, s, coords, depth);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %depth:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:vec4<f32> = textureGatherCompare %t, %s, %coords, %depth
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %depth:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:vec4<f32> = spirv.image_dref_gather %6, %coords, %depth, 0u
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureGatherCompare_Depth2D_Offset) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* depth = b.FunctionParam("depth", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, depth});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureGatherCompare, t, s, coords, depth,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %depth:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:vec4<f32> = textureGatherCompare %t, %s, %coords, %depth, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %depth:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:vec4<f32> = spirv.image_dref_gather %6, %coords, %depth, 8u, vec2<i32>(1i)
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureGatherCompare_Depth2DArray) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::k2dArray));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.u32());
    auto* depth = b.FunctionParam("depth", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, array_idx, depth});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), builtin::Function::kTextureGatherCompare, t, s,
                              coords, array_idx, depth);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d_array, %s:sampler, %coords:vec2<f32>, %array_idx:u32, %depth:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %7:vec4<f32> = textureGatherCompare %t, %s, %coords, %array_idx, %depth
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d_array, %s:sampler, %coords:vec2<f32>, %array_idx:u32, %depth:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %7:spirv.sampled_image = spirv.sampled_image %t, %s
    %8:f32 = convert %array_idx
    %9:vec3<f32> = construct %coords, %8
    %10:vec4<f32> = spirv.image_dref_gather %7, %9, %depth, 0u
    ret %10
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureStore_2D) {
    auto format = builtin::TexelFormat::kR32Float;
    auto* t = b.FunctionParam("t", ty.Get<type::StorageTexture>(
                                       type::TextureDimension::k2d, format, builtin::Access::kWrite,
                                       type::StorageTexture::SubtypeFor(format, ty)));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* texel = b.FunctionParam("texel", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t, coords, texel});

    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), builtin::Function::kTextureStore, t, coords, texel);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d<r32float, write>, %coords:vec2<i32>, %texel:i32):void -> %b1 {
  %b1 = block {
    %5:void = textureStore %t, %coords, %texel
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_2d<r32float, write>, %coords:vec2<i32>, %texel:i32):void -> %b1 {
  %b1 = block {
    %5:void = spirv.image_write %t, %coords, %texel, 0u
    ret
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureStore_2DArray) {
    auto format = builtin::TexelFormat::kR32Float;
    auto* t =
        b.FunctionParam("t", ty.Get<type::StorageTexture>(
                                 type::TextureDimension::k2dArray, format, builtin::Access::kWrite,
                                 type::StorageTexture::SubtypeFor(format, ty)));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* texel = b.FunctionParam("texel", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t, coords, array_idx, texel});

    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), builtin::Function::kTextureStore, t, coords, array_idx, texel);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d_array<r32float, write>, %coords:vec2<i32>, %array_idx:i32, %texel:i32):void -> %b1 {
  %b1 = block {
    %6:void = textureStore %t, %coords, %array_idx, %texel
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_2d_array<r32float, write>, %coords:vec2<i32>, %array_idx:i32, %texel:i32):void -> %b1 {
  %b1 = block {
    %6:vec3<i32> = construct %coords, %array_idx
    %7:void = spirv.image_write %t, %6, %texel, 0u
    ret
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureStore_2DArray_IndexDifferentType) {
    auto format = builtin::TexelFormat::kR32Float;
    auto* t =
        b.FunctionParam("t", ty.Get<type::StorageTexture>(
                                 type::TextureDimension::k2dArray, format, builtin::Access::kWrite,
                                 type::StorageTexture::SubtypeFor(format, ty)));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.u32());
    auto* texel = b.FunctionParam("texel", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t, coords, array_idx, texel});

    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), builtin::Function::kTextureStore, t, coords, array_idx, texel);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d_array<r32float, write>, %coords:vec2<i32>, %array_idx:u32, %texel:i32):void -> %b1 {
  %b1 = block {
    %6:void = textureStore %t, %coords, %array_idx, %texel
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_2d_array<r32float, write>, %coords:vec2<i32>, %array_idx:u32, %texel:i32):void -> %b1 {
  %b1 = block {
    %6:i32 = convert %array_idx
    %7:vec3<i32> = construct %coords, %6
    %8:void = spirv.image_write %t, %7, %texel, 0u
    ret
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureDimensions_2D_ImplicitLod) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<u32>(), builtin::Function::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>):vec2<u32> -> %b1 {
  %b1 = block {
    %3:vec2<u32> = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>):vec2<u32> -> %b1 {
  %b1 = block {
    %3:vec2<u32> = spirv.image_query_size_lod %t, 0u
    ret %3
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureDimensions_2D_ExplicitLod) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* lod = b.FunctionParam("lod", ty.i32());
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t, lod});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<u32>(), builtin::Function::kTextureDimensions, t, lod);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %lod:i32):vec2<u32> -> %b1 {
  %b1 = block {
    %4:vec2<u32> = textureDimensions %t, %lod
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %lod:i32):vec2<u32> -> %b1 {
  %b1 = block {
    %4:vec2<u32> = spirv.image_query_size_lod %t, %lod
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureDimensions_2DArray) {
    auto* t = b.FunctionParam(
        "t", ty.Get<type::SampledTexture>(type::TextureDimension::k2dArray, ty.f32()));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<u32>(), builtin::Function::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>):vec2<u32> -> %b1 {
  %b1 = block {
    %3:vec2<u32> = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>):vec2<u32> -> %b1 {
  %b1 = block {
    %3:vec3<u32> = spirv.image_query_size_lod %t, 0u
    %4:vec2<u32> = swizzle %3, xy
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureDimensions_Multisampled) {
    auto* t = b.FunctionParam(
        "t", ty.Get<type::MultisampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<u32>(), builtin::Function::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_multisampled_2d<f32>):vec2<u32> -> %b1 {
  %b1 = block {
    %3:vec2<u32> = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_multisampled_2d<f32>):vec2<u32> -> %b1 {
  %b1 = block {
    %3:vec2<u32> = spirv.image_query_size %t
    ret %3
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureNumLayers_2DArray) {
    auto* t = b.FunctionParam(
        "t", ty.Get<type::SampledTexture>(type::TextureDimension::k2dArray, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), builtin::Function::kTextureNumLayers, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>):u32 -> %b1 {
  %b1 = block {
    %3:u32 = textureNumLayers %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>):u32 -> %b1 {
  %b1 = block {
    %3:vec3<u32> = spirv.image_query_size_lod %t, 0u
    %4:u32 = access %3, 2u
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureNumLayers_CubeArray) {
    auto* t = b.FunctionParam(
        "t", ty.Get<type::SampledTexture>(type::TextureDimension::kCubeArray, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), builtin::Function::kTextureNumLayers, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_cube_array<f32>):u32 -> %b1 {
  %b1 = block {
    %3:u32 = textureNumLayers %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_cube_array<f32>):u32 -> %b1 {
  %b1 = block {
    %3:vec3<u32> = spirv.image_query_size_lod %t, 0u
    %4:u32 = access %3, 2u
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureNumLayers_Depth2DArray) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::k2dArray));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), builtin::Function::kTextureNumLayers, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d_array):u32 -> %b1 {
  %b1 = block {
    %3:u32 = textureNumLayers %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d_array):u32 -> %b1 {
  %b1 = block {
    %3:vec3<u32> = spirv.image_query_size_lod %t, 0u
    %4:u32 = access %3, 2u
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureNumLayers_DepthCubeArray) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::kCubeArray));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), builtin::Function::kTextureNumLayers, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_cube_array):u32 -> %b1 {
  %b1 = block {
    %3:u32 = textureNumLayers %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_cube_array):u32 -> %b1 {
  %b1 = block {
    %3:vec3<u32> = spirv.image_query_size_lod %t, 0u
    %4:u32 = access %3, 2u
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureNumLayers_Storage2DArray) {
    auto format = builtin::TexelFormat::kR32Float;
    auto* t = b.FunctionParam("t", ty.Get<type::StorageTexture>(
                                       type::TextureDimension::k2d, format, builtin::Access::kWrite,
                                       type::StorageTexture::SubtypeFor(format, ty)));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), builtin::Function::kTextureNumLayers, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d<r32float, write>):u32 -> %b1 {
  %b1 = block {
    %3:u32 = textureNumLayers %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_2d<r32float, write>):u32 -> %b1 {
  %b1 = block {
    %3:vec3<u32> = spirv.image_query_size %t
    %4:u32 = access %3, 2u
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::ir::transform
