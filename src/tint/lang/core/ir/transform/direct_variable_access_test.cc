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

#include "src/tint/lang/core/ir/transform/direct_variable_access.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/struct.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace {

static constexpr DirectVariableAccessOptions kTransformPrivate = {
    /* transform_private */ true,
    /* transform_function */ false,
};

static constexpr DirectVariableAccessOptions kTransformFunction = {
    /* transform_private */ false,
    /* transform_function */ true,
};

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// remove uncalled
////////////////////////////////////////////////////////////////////////////////
namespace remove_uncalled {

using IR_DirectVariableAccessTest_RemoveUncalled = TransformTest;

TEST_F(IR_DirectVariableAccessTest_RemoveUncalled, PtrUniform) {
    b.Append(b.ir.root_block, [&] { b.Var<private_>("keep_me", 42_i); });

    auto* u = b.Function("u", ty.i32());
    auto* p = b.FunctionParam("p", ty.ptr<uniform, i32, read>());
    u->SetParams({
        b.FunctionParam("pre", ty.i32()),
        p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(u->Block(), [&] { b.Return(u, b.Load(p)); });

    auto* src = R"(
%b1 = block {  # root
  %keep_me:ptr<private, i32, read_write> = var, 42i
}

%u = func(%pre:i32, %p:ptr<uniform, i32, read>, %post:i32):i32 -> %b2 {
  %b2 = block {
    %6:i32 = load %p
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %keep_me:ptr<private, i32, read_write> = var, 42i
}

)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_RemoveUncalled, PtrStorage) {
    b.Append(b.ir.root_block, [&] { b.Var<private_>("keep_me", 42_i); });

    auto* s = b.Function("s", ty.i32());
    auto* p = b.FunctionParam("p", ty.ptr<storage, i32, read>());
    s->SetParams({
        b.FunctionParam("pre", ty.i32()),
        p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(s->Block(), [&] { b.Return(s, b.Load(p)); });

    auto* src = R"(
%b1 = block {  # root
  %keep_me:ptr<private, i32, read_write> = var, 42i
}

%s = func(%pre:i32, %p:ptr<storage, i32, read>, %post:i32):i32 -> %b2 {
  %b2 = block {
    %6:i32 = load %p
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %keep_me:ptr<private, i32, read_write> = var, 42i
}

)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_RemoveUncalled, PtrWorkgroup) {
    b.Append(b.ir.root_block, [&] { b.Var<private_>("keep_me", 42_i); });

    auto* w = b.Function("w", ty.i32());
    auto* p = b.FunctionParam("p", ty.ptr<workgroup, i32>());
    w->SetParams({
        b.FunctionParam("pre", ty.i32()),
        p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(w->Block(), [&] { b.Return(w, b.Load(p)); });

    auto* src = R"(
%b1 = block {  # root
  %keep_me:ptr<private, i32, read_write> = var, 42i
}

%w = func(%pre:i32, %p:ptr<workgroup, i32, read_write>, %post:i32):i32 -> %b2 {
  %b2 = block {
    %6:i32 = load %p
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %keep_me:ptr<private, i32, read_write> = var, 42i
}

)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_RemoveUncalled, PtrPrivate_Disabled) {
    b.Append(b.ir.root_block, [&] { b.Var<private_>("keep_me", 42_i); });

    auto* f = b.Function("f", ty.i32());
    auto* p = b.FunctionParam("p", ty.ptr<private_, i32>());
    f->SetParams({
        b.FunctionParam("pre", ty.i32()),
        p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(f->Block(), [&] { b.Return(f, b.Load(p)); });

    auto* src = R"(
%b1 = block {  # root
  %keep_me:ptr<private, i32, read_write> = var, 42i
}

%f = func(%pre:i32, %p:ptr<private, i32, read_write>, %post:i32):i32 -> %b2 {
  %b2 = block {
    %6:i32 = load %p
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_RemoveUncalled, PtrPrivate_Enabled) {
    b.Append(b.ir.root_block, [&] { b.Var<private_>("keep_me", 42_i); });

    auto* f = b.Function("f", ty.i32());
    auto* p = b.FunctionParam("p", ty.ptr<private_, i32>());
    f->SetParams({
        b.FunctionParam("pre", ty.i32()),
        p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(f->Block(), [&] { b.Return(f, b.Load(p)); });

    auto* src = R"(
%b1 = block {  # root
  %keep_me:ptr<private, i32, read_write> = var, 42i
}

%f = func(%pre:i32, %p:ptr<private, i32, read_write>, %post:i32):i32 -> %b2 {
  %b2 = block {
    %6:i32 = load %p
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %keep_me:ptr<private, i32, read_write> = var, 42i
}

)";
    Run(DirectVariableAccess, kTransformPrivate);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_RemoveUncalled, PtrFunction_Disabled) {
    b.Append(b.ir.root_block, [&] { b.Var<private_>("keep_me", 42_i); });

    auto* f = b.Function("f", ty.i32());
    auto* p = b.FunctionParam("p", ty.ptr<function, i32>());
    f->SetParams({
        b.FunctionParam("pre", ty.i32()),
        p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(f->Block(), [&] { b.Return(f, b.Load(p)); });

    auto* src = R"(
%b1 = block {  # root
  %keep_me:ptr<private, i32, read_write> = var, 42i
}

%f = func(%pre:i32, %p:ptr<function, i32, read_write>, %post:i32):i32 -> %b2 {
  %b2 = block {
    %6:i32 = load %p
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_RemoveUncalled, PtrFunction_Enabled) {
    b.Append(b.ir.root_block, [&] { b.Var<private_>("keep_me", 42_i); });

    auto* f = b.Function("f", ty.i32());
    auto* p = b.FunctionParam("p", ty.ptr<function, i32>());
    f->SetParams({
        b.FunctionParam("pre", ty.i32()),
        p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(f->Block(), [&] { b.Return(f, b.Load(p)); });

    auto* src = R"(
%b1 = block {  # root
  %keep_me:ptr<private, i32, read_write> = var, 42i
}

%f = func(%pre:i32, %p:ptr<function, i32, read_write>, %post:i32):i32 -> %b2 {
  %b2 = block {
    %6:i32 = load %p
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %keep_me:ptr<private, i32, read_write> = var, 42i
}

)";

    Run(DirectVariableAccess, kTransformFunction);

    EXPECT_EQ(expect, str());
}

}  // namespace remove_uncalled

////////////////////////////////////////////////////////////////////////////////
// pointer chains
////////////////////////////////////////////////////////////////////////////////
namespace pointer_chains_tests {

using IR_DirectVariableAccessTest_PtrChains = TransformTest;

TEST_F(IR_DirectVariableAccessTest_PtrChains, ConstantIndices) {
    Var* U = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 U = b.Var<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read>("U");
                 U->SetBindingPoint(0, 0);
             });

    auto* fn_a = b.Function("a", ty.vec4<i32>());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<uniform, vec4<i32>, read>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* p0 = b.Let("p0", U);
        auto* p1 = b.Access(ty.ptr<uniform, array<array<vec4<i32>, 8>, 8>, read>(), p0, 1_i);
        b.ir.SetName(p1, "p1");
        auto* p2 = b.Access(ty.ptr<uniform, array<vec4<i32>, 8>, read>(), p1, 2_i);
        b.ir.SetName(p2, "p2");
        auto* p3 = b.Access(ty.ptr<uniform, vec4<i32>, read>(), p2, 3_i);
        b.ir.SetName(p3, "p3");
        b.Call(ty.vec4<i32>(), fn_a, 10_i, p3, 20_i);
        b.Return(fn_b);
    });

    auto* fn_c = b.Function("c", ty.void_());
    auto* fn_c_p =
        b.FunctionParam("p", ty.ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read>());
    fn_c->SetParams({fn_c_p});
    b.Append(fn_c->Block(), [&] {
        auto* p0 = b.Let("p0", fn_c_p);
        auto* p1 = b.Access(ty.ptr<uniform, array<array<vec4<i32>, 8>, 8>, read>(), p0, 1_i);
        b.ir.SetName(p1, "p1");
        auto* p2 = b.Access(ty.ptr<uniform, array<vec4<i32>, 8>, read>(), p1, 2_i);
        b.ir.SetName(p2, "p2");
        auto* p3 = b.Access(ty.ptr<uniform, vec4<i32>, read>(), p2, 3_i);
        b.ir.SetName(p3, "p3");
        b.Call(ty.vec4<i32>(), fn_a, 10_i, p3, 20_i);
        b.Return(fn_c);
    });

    auto* fn_d = b.Function("d", ty.void_());
    b.Append(fn_d->Block(), [&] {
        b.Call(ty.void_(), fn_c, U);
        b.Return(fn_d);
    });

    auto* src = R"(
%b1 = block {  # root
  %U:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read> = var @binding_point(0, 0)
}

%a = func(%pre:i32, %p:ptr<uniform, vec4<i32>, read>, %post:i32):vec4<i32> -> %b2 {
  %b2 = block {
    %6:vec4<i32> = load %p
    ret %6
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %p0:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read> = let %U
    %p1:ptr<uniform, array<array<vec4<i32>, 8>, 8>, read> = access %p0, 1i
    %p2:ptr<uniform, array<vec4<i32>, 8>, read> = access %p1, 2i
    %p3:ptr<uniform, vec4<i32>, read> = access %p2, 3i
    %12:vec4<i32> = call %a, 10i, %p3, 20i
    ret
  }
}
%c = func(%p_1:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read>):void -> %b4 {  # %p_1: 'p'
  %b4 = block {
    %p0_1:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read> = let %p_1  # %p0_1: 'p0'
    %p1_1:ptr<uniform, array<array<vec4<i32>, 8>, 8>, read> = access %p0_1, 1i  # %p1_1: 'p1'
    %p2_1:ptr<uniform, array<vec4<i32>, 8>, read> = access %p1_1, 2i  # %p2_1: 'p2'
    %p3_1:ptr<uniform, vec4<i32>, read> = access %p2_1, 3i  # %p3_1: 'p3'
    %19:vec4<i32> = call %a, 10i, %p3_1, 20i
    ret
  }
}
%d = func():void -> %b5 {
  %b5 = block {
    %21:void = call %c, %U
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect =
        R"(
%b1 = block {  # root
  %U:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read> = var @binding_point(0, 0)
}

%a_U_X_X_X = func(%pre:i32, %p_indices:array<u32, 3>, %post:i32):vec4<i32> -> %b2 {
  %b2 = block {
    %6:u32 = access %p_indices, 0u
    %7:u32 = access %p_indices, 1u
    %8:u32 = access %p_indices, 2u
    %9:ptr<uniform, vec4<i32>, read> = access %U, %6, %7, %8
    %10:vec4<i32> = load %9
    ret %10
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %12:u32 = convert 3i
    %13:u32 = convert 2i
    %14:u32 = convert 1i
    %15:array<u32, 3> = construct %14, %13, %12
    %16:vec4<i32> = call %a_U_X_X_X, 10i, %15, 20i
    ret
  }
}
%c_U = func():void -> %b4 {
  %b4 = block {
    %18:u32 = convert 3i
    %19:u32 = convert 2i
    %20:u32 = convert 1i
    %21:array<u32, 3> = construct %20, %19, %18
    %22:vec4<i32> = call %a_U_X_X_X, 10i, %21, 20i
    ret
  }
}
%d = func():void -> %b5 {
  %b5 = block {
    %24:void = call %c_U
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PtrChains, DynamicIndices) {
    Var* U = nullptr;
    Var* i = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 U = b.Var<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read>("U");
                 U->SetBindingPoint(0, 0);
                 i = b.Var<private_, i32>("i");
             });

    auto* fn_first = b.Function("first", ty.i32());
    auto* fn_second = b.Function("second", ty.i32());
    auto* fn_third = b.Function("third", ty.i32());
    for (auto fn : {fn_first, fn_second, fn_third}) {
        b.Append(fn->Block(), [&] {
            b.Store(i, b.Add(ty.i32(), b.Load(i), 1_i));
            b.Return(fn, b.Load(i));
        });
    }

    auto* fn_a = b.Function("a", ty.vec4<i32>());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<uniform, vec4<i32>, read>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* p0 = b.Let("p0", U);
        auto* first = b.Call(fn_first);
        auto* p1 = b.Access(ty.ptr<uniform, array<array<vec4<i32>, 8>, 8>, read>(), p0, first);
        b.ir.SetName(p1, "p1");
        auto* second = b.Call(fn_second);
        auto* third = b.Call(fn_third);
        auto* p2 = b.Access(ty.ptr<uniform, vec4<i32>, read>(), p1, second, third);
        b.ir.SetName(p2, "p2");
        b.Call(ty.vec4<i32>(), fn_a, 10_i, p2, 20_i);
        b.Return(fn_b);
    });

    auto* fn_c = b.Function("c", ty.void_());
    auto* fn_c_p =
        b.FunctionParam("p", ty.ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read>());
    fn_c->SetParams({fn_c_p});
    b.Append(fn_c->Block(), [&] {
        auto* p0 = b.Let("p0", fn_c_p);
        auto* first = b.Call(fn_first);
        auto* p1 = b.Access(ty.ptr<uniform, array<array<vec4<i32>, 8>, 8>, read>(), p0, first);
        b.ir.SetName(p1, "p1");
        auto* second = b.Call(fn_second);
        auto* third = b.Call(fn_third);
        auto* p2 = b.Access(ty.ptr<uniform, vec4<i32>, read>(), p1, second, third);
        b.ir.SetName(p2, "p2");
        b.Call(ty.vec4<i32>(), fn_a, 10_i, p2, 20_i);
        b.Return(fn_c);
    });

    auto* fn_d = b.Function("d", ty.void_());
    b.Append(fn_d->Block(), [&] {
        b.Call(ty.void_(), fn_c, U);
        b.Return(fn_d);
    });

    auto* src = R"(
%b1 = block {  # root
  %U:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read> = var @binding_point(0, 0)
  %i:ptr<private, i32, read_write> = var
}

%first = func():i32 -> %b2 {
  %b2 = block {
    %4:i32 = load %i
    %5:i32 = add %4, 1i
    store %i, %5
    %6:i32 = load %i
    ret %6
  }
}
%second = func():i32 -> %b3 {
  %b3 = block {
    %8:i32 = load %i
    %9:i32 = add %8, 1i
    store %i, %9
    %10:i32 = load %i
    ret %10
  }
}
%third = func():i32 -> %b4 {
  %b4 = block {
    %12:i32 = load %i
    %13:i32 = add %12, 1i
    store %i, %13
    %14:i32 = load %i
    ret %14
  }
}
%a = func(%pre:i32, %p:ptr<uniform, vec4<i32>, read>, %post:i32):vec4<i32> -> %b5 {
  %b5 = block {
    %19:vec4<i32> = load %p
    ret %19
  }
}
%b = func():void -> %b6 {
  %b6 = block {
    %p0:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read> = let %U
    %22:i32 = call %first
    %p1:ptr<uniform, array<array<vec4<i32>, 8>, 8>, read> = access %p0, %22
    %24:i32 = call %second
    %25:i32 = call %third
    %p2:ptr<uniform, vec4<i32>, read> = access %p1, %24, %25
    %27:vec4<i32> = call %a, 10i, %p2, 20i
    ret
  }
}
%c = func(%p_1:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read>):void -> %b7 {  # %p_1: 'p'
  %b7 = block {
    %p0_1:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read> = let %p_1  # %p0_1: 'p0'
    %31:i32 = call %first
    %p1_1:ptr<uniform, array<array<vec4<i32>, 8>, 8>, read> = access %p0_1, %31  # %p1_1: 'p1'
    %33:i32 = call %second
    %34:i32 = call %third
    %p2_1:ptr<uniform, vec4<i32>, read> = access %p1_1, %33, %34  # %p2_1: 'p2'
    %36:vec4<i32> = call %a, 10i, %p2_1, 20i
    ret
  }
}
%d = func():void -> %b8 {
  %b8 = block {
    %38:void = call %c, %U
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %U:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read> = var @binding_point(0, 0)
  %i:ptr<private, i32, read_write> = var
}

%first = func():i32 -> %b2 {
  %b2 = block {
    %4:i32 = load %i
    %5:i32 = add %4, 1i
    store %i, %5
    %6:i32 = load %i
    ret %6
  }
}
%second = func():i32 -> %b3 {
  %b3 = block {
    %8:i32 = load %i
    %9:i32 = add %8, 1i
    store %i, %9
    %10:i32 = load %i
    ret %10
  }
}
%third = func():i32 -> %b4 {
  %b4 = block {
    %12:i32 = load %i
    %13:i32 = add %12, 1i
    store %i, %13
    %14:i32 = load %i
    ret %14
  }
}
%a_U_X_X_X = func(%pre:i32, %p_indices:array<u32, 3>, %post:i32):vec4<i32> -> %b5 {
  %b5 = block {
    %19:u32 = access %p_indices, 0u
    %20:u32 = access %p_indices, 1u
    %21:u32 = access %p_indices, 2u
    %22:ptr<uniform, vec4<i32>, read> = access %U, %19, %20, %21
    %23:vec4<i32> = load %22
    ret %23
  }
}
%b = func():void -> %b6 {
  %b6 = block {
    %25:i32 = call %first
    %26:i32 = call %second
    %27:i32 = call %third
    %28:u32 = convert %26
    %29:u32 = convert %27
    %30:u32 = convert %25
    %31:array<u32, 3> = construct %30, %28, %29
    %32:vec4<i32> = call %a_U_X_X_X, 10i, %31, 20i
    ret
  }
}
%c_U = func():void -> %b7 {
  %b7 = block {
    %34:i32 = call %first
    %35:i32 = call %second
    %36:i32 = call %third
    %37:u32 = convert %35
    %38:u32 = convert %36
    %39:u32 = convert %34
    %40:array<u32, 3> = construct %39, %37, %38
    %41:vec4<i32> = call %a_U_X_X_X, 10i, %40, 20i
    ret
  }
}
%d = func():void -> %b8 {
  %b8 = block {
    %43:void = call %c_U
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

}  // namespace pointer_chains_tests

////////////////////////////////////////////////////////////////////////////////
// 'uniform' address space
////////////////////////////////////////////////////////////////////////////////
namespace uniform_as_tests {

using IR_DirectVariableAccessTest_UniformAS = TransformTest;

TEST_F(IR_DirectVariableAccessTest_UniformAS, Param_ptr_i32_read) {
    Var* U = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 U = b.Var<uniform, i32, read>("U");
                 U->SetBindingPoint(0, 0);
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<uniform, i32, read>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        b.Call(fn_a, 10_i, U, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
%b1 = block {  # root
  %U:ptr<uniform, i32, read> = var @binding_point(0, 0)
}

%a = func(%pre:i32, %p:ptr<uniform, i32, read>, %post:i32):i32 -> %b2 {
  %b2 = block {
    %6:i32 = load %p
    ret %6
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %8:i32 = call %a, 10i, %U, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %U:ptr<uniform, i32, read> = var @binding_point(0, 0)
}

%a_U = func(%pre:i32, %post:i32):i32 -> %b2 {
  %b2 = block {
    %5:ptr<uniform, i32, read> = access %U
    %6:i32 = load %5
    ret %6
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %8:i32 = call %a_U, 10i, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_UniformAS, Param_ptr_vec4i32_Via_array_DynamicRead) {
    Var* U = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 U = b.Var<uniform, array<vec4<i32>, 8>, read>("U");
                 U->SetBindingPoint(0, 0);
             });

    auto* fn_a = b.Function("a", ty.vec4<i32>());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<uniform, vec4<i32>, read>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* I = b.Let("I", 3_i);
        auto* access = b.Access(ty.ptr<uniform, vec4<i32>, read>(), U, I);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
%b1 = block {  # root
  %U:ptr<uniform, array<vec4<i32>, 8>, read> = var @binding_point(0, 0)
}

%a = func(%pre:i32, %p:ptr<uniform, vec4<i32>, read>, %post:i32):vec4<i32> -> %b2 {
  %b2 = block {
    %6:vec4<i32> = load %p
    ret %6
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %I:i32 = let 3i
    %9:ptr<uniform, vec4<i32>, read> = access %U, %I
    %10:vec4<i32> = call %a, 10i, %9, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %U:ptr<uniform, array<vec4<i32>, 8>, read> = var @binding_point(0, 0)
}

%a_U_X = func(%pre:i32, %p_indices:array<u32, 1>, %post:i32):vec4<i32> -> %b2 {
  %b2 = block {
    %6:u32 = access %p_indices, 0u
    %7:ptr<uniform, vec4<i32>, read> = access %U, %6
    %8:vec4<i32> = load %7
    ret %8
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %I:i32 = let 3i
    %11:u32 = convert %I
    %12:array<u32, 1> = construct %11
    %13:vec4<i32> = call %a_U_X, 10i, %12, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_UniformAS, CallChaining) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    auto* Outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.Register("arr"), ty.array(Inner, 4)},
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    Var* U = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 U = b.Var("U", ty.ptr<uniform, read>(Outer));
                 U->SetBindingPoint(0, 0);
             });

    auto* fn_0 = b.Function("f0", ty.f32());
    auto* fn_0_p = b.FunctionParam("p", ty.ptr<uniform, vec4<f32>, read>());
    fn_0->SetParams({fn_0_p});
    b.Append(fn_0->Block(), [&] { b.Return(fn_0, b.LoadVectorElement(fn_0_p, 0_u)); });

    auto* fn_1 = b.Function("f1", ty.f32());
    auto* fn_1_p = b.FunctionParam("p", ty.ptr<uniform, mat3x4<f32>, read>());
    fn_1->SetParams({fn_1_p});
    b.Append(fn_1->Block(), [&] {
        auto* res = b.Var<function, f32>("res");
        {
            // res += f0(&(*p)[1]);
            auto* call_0 = b.Call(fn_0, b.Access(ty.ptr<uniform, vec4<f32>, read>(), fn_1_p, 1_i));
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &(*p)[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<uniform, vec4<f32>, read>(), fn_1_p, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // res += f0(&U.arr[2].mat[1]);
            auto* access = b.Access(ty.ptr<uniform, vec4<f32>, read>(), U, 0_u, 2_i, 0_u, 1_i);
            auto* call_0 = b.Call(fn_0, access);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &U.arr[2].mat[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<uniform, vec4<f32>, read>(), U, 0_u, 2_i, 0_u, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }

        b.Return(fn_1, b.Load(res));
    });

    auto* fn_2 = b.Function("f2", ty.f32());
    auto* fn_2_p = b.FunctionParam("p", ty.ptr<uniform, read>(Inner));
    fn_2->SetParams({fn_2_p});
    b.Append(fn_2->Block(), [&] {
        auto* p_mat = b.Access(ty.ptr<uniform, mat3x4<f32>, read>(), fn_2_p, 0_u);
        b.ir.SetName(p_mat, "p_mat");
        b.Return(fn_2, b.Call(fn_1, p_mat));
    });

    auto* fn_3 = b.Function("f3", ty.f32());
    auto* fn_3_p0 = b.FunctionParam("p0", ty.ptr<uniform, read>(ty.array(Inner, 4)));
    auto* fn_3_p1 = b.FunctionParam("p1", ty.ptr<uniform, mat3x4<f32>, read>());
    fn_3->SetParams({fn_3_p0, fn_3_p1});
    b.Append(fn_3->Block(), [&] {
        auto* p0_inner = b.Access(ty.ptr<uniform, read>(Inner), fn_3_p0, 3_i);
        b.ir.SetName(p0_inner, "p0_inner");
        auto* call_0 = b.Call(ty.f32(), fn_2, p0_inner);
        auto* call_1 = b.Call(ty.f32(), fn_1, fn_3_p1);
        b.Return(fn_3, b.Add(ty.f32(), call_0, call_1));
    });

    auto* fn_4 = b.Function("f4", ty.f32());
    auto* fn_4_p = b.FunctionParam("p", ty.ptr<uniform, read>(Outer));
    fn_4->SetParams({fn_4_p});
    b.Append(fn_4->Block(), [&] {
        auto* access_0 = b.Access(ty.ptr<uniform, read>(ty.array(Inner, 4)), fn_4_p, 0_u);
        auto* access_1 = b.Access(ty.ptr<uniform, mat3x4<f32>, read>(), U, 1_u);
        b.Return(fn_4, b.Call(ty.f32(), fn_3, access_0, access_1));
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        b.Call(ty.f32(), fn_4, U);
        b.Return(fn_b);
    });

    auto* src = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

%b1 = block {  # root
  %U:ptr<uniform, Outer, read> = var @binding_point(0, 0)
}

%f0 = func(%p:ptr<uniform, vec4<f32>, read>):f32 -> %b2 {
  %b2 = block {
    %4:f32 = load_vector_element %p, 0u
    ret %4
  }
}
%f1 = func(%p_1:ptr<uniform, mat3x4<f32>, read>):f32 -> %b3 {  # %p_1: 'p'
  %b3 = block {
    %res:ptr<function, f32, read_write> = var
    %8:ptr<uniform, vec4<f32>, read> = access %p_1, 1i
    %9:f32 = call %f0, %8
    %10:f32 = load %res
    %11:f32 = add %10, %9
    store %res, %11
    %p_vec:ptr<uniform, vec4<f32>, read> = access %p_1, 1i
    %13:f32 = call %f0, %p_vec
    %14:f32 = load %res
    %15:f32 = add %14, %13
    store %res, %15
    %16:ptr<uniform, vec4<f32>, read> = access %U, 0u, 2i, 0u, 1i
    %17:f32 = call %f0, %16
    %18:f32 = load %res
    %19:f32 = add %18, %17
    store %res, %19
    %p_vec_1:ptr<uniform, vec4<f32>, read> = access %U, 0u, 2i, 0u, 1i  # %p_vec_1: 'p_vec'
    %21:f32 = call %f0, %p_vec_1
    %22:f32 = load %res
    %23:f32 = add %22, %21
    store %res, %23
    %24:f32 = load %res
    ret %24
  }
}
%f2 = func(%p_2:ptr<uniform, Inner, read>):f32 -> %b4 {  # %p_2: 'p'
  %b4 = block {
    %p_mat:ptr<uniform, mat3x4<f32>, read> = access %p_2, 0u
    %28:f32 = call %f1, %p_mat
    ret %28
  }
}
%f3 = func(%p0:ptr<uniform, array<Inner, 4>, read>, %p1:ptr<uniform, mat3x4<f32>, read>):f32 -> %b5 {
  %b5 = block {
    %p0_inner:ptr<uniform, Inner, read> = access %p0, 3i
    %33:f32 = call %f2, %p0_inner
    %34:f32 = call %f1, %p1
    %35:f32 = add %33, %34
    ret %35
  }
}
%f4 = func(%p_3:ptr<uniform, Outer, read>):f32 -> %b6 {  # %p_3: 'p'
  %b6 = block {
    %38:ptr<uniform, array<Inner, 4>, read> = access %p_3, 0u
    %39:ptr<uniform, mat3x4<f32>, read> = access %U, 1u
    %40:f32 = call %f3, %38, %39
    ret %40
  }
}
%b = func():void -> %b7 {
  %b7 = block {
    %42:f32 = call %f4, %U
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

%b1 = block {  # root
  %U:ptr<uniform, Outer, read> = var @binding_point(0, 0)
}

%f0_U_mat_X = func(%p_indices:array<u32, 1>):f32 -> %b2 {
  %b2 = block {
    %4:u32 = access %p_indices, 0u
    %5:ptr<uniform, vec4<f32>, read> = access %U, 1u, %4
    %6:f32 = load_vector_element %5, 0u
    ret %6
  }
}
%f0_U_arr_X_mat_X = func(%p_indices_1:array<u32, 2>):f32 -> %b3 {  # %p_indices_1: 'p_indices'
  %b3 = block {
    %9:u32 = access %p_indices_1, 0u
    %10:u32 = access %p_indices_1, 1u
    %11:ptr<uniform, vec4<f32>, read> = access %U, 0u, %9, 0u, %10
    %12:f32 = load_vector_element %11, 0u
    ret %12
  }
}
%f1_U_mat = func():f32 -> %b4 {
  %b4 = block {
    %res:ptr<function, f32, read_write> = var
    %15:u32 = convert 1i
    %16:array<u32, 1> = construct %15
    %17:f32 = call %f0_U_mat_X, %16
    %18:f32 = load %res
    %19:f32 = add %18, %17
    store %res, %19
    %20:u32 = convert 1i
    %21:array<u32, 1> = construct %20
    %22:f32 = call %f0_U_mat_X, %21
    %23:f32 = load %res
    %24:f32 = add %23, %22
    store %res, %24
    %25:u32 = convert 2i
    %26:u32 = convert 1i
    %27:array<u32, 2> = construct %25, %26
    %28:f32 = call %f0_U_arr_X_mat_X, %27
    %29:f32 = load %res
    %30:f32 = add %29, %28
    store %res, %30
    %31:u32 = convert 2i
    %32:u32 = convert 1i
    %33:array<u32, 2> = construct %31, %32
    %34:f32 = call %f0_U_arr_X_mat_X, %33
    %35:f32 = load %res
    %36:f32 = add %35, %34
    store %res, %36
    %37:f32 = load %res
    ret %37
  }
}
%f1_U_arr_X_mat = func(%p_indices_2:array<u32, 1>):f32 -> %b5 {  # %p_indices_2: 'p_indices'
  %b5 = block {
    %40:u32 = access %p_indices_2, 0u
    %res_1:ptr<function, f32, read_write> = var  # %res_1: 'res'
    %42:u32 = convert 1i
    %43:array<u32, 2> = construct %40, %42
    %44:f32 = call %f0_U_arr_X_mat_X, %43
    %45:f32 = load %res_1
    %46:f32 = add %45, %44
    store %res_1, %46
    %47:u32 = convert 1i
    %48:array<u32, 2> = construct %40, %47
    %49:f32 = call %f0_U_arr_X_mat_X, %48
    %50:f32 = load %res_1
    %51:f32 = add %50, %49
    store %res_1, %51
    %52:u32 = convert 2i
    %53:u32 = convert 1i
    %54:array<u32, 2> = construct %52, %53
    %55:f32 = call %f0_U_arr_X_mat_X, %54
    %56:f32 = load %res_1
    %57:f32 = add %56, %55
    store %res_1, %57
    %58:u32 = convert 2i
    %59:u32 = convert 1i
    %60:array<u32, 2> = construct %58, %59
    %61:f32 = call %f0_U_arr_X_mat_X, %60
    %62:f32 = load %res_1
    %63:f32 = add %62, %61
    store %res_1, %63
    %64:f32 = load %res_1
    ret %64
  }
}
%f2_U_arr_X = func(%p_indices_3:array<u32, 1>):f32 -> %b6 {  # %p_indices_3: 'p_indices'
  %b6 = block {
    %67:u32 = access %p_indices_3, 0u
    %68:array<u32, 1> = construct %67
    %69:f32 = call %f1_U_arr_X_mat, %68
    ret %69
  }
}
%f3_U_arr_U_mat = func():f32 -> %b7 {
  %b7 = block {
    %71:u32 = convert 3i
    %72:array<u32, 1> = construct %71
    %73:f32 = call %f2_U_arr_X, %72
    %74:f32 = call %f1_U_mat
    %75:f32 = add %73, %74
    ret %75
  }
}
%f4_U = func():f32 -> %b8 {
  %b8 = block {
    %77:f32 = call %f3_U_arr_U_mat
    ret %77
  }
}
%b = func():void -> %b9 {
  %b9 = block {
    %79:f32 = call %f4_U
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

}  // namespace uniform_as_tests

////////////////////////////////////////////////////////////////////////////////
// 'storage' address space
////////////////////////////////////////////////////////////////////////////////
namespace storage_as_tests {

using IR_DirectVariableAccessTest_StorageAS = TransformTest;

TEST_F(IR_DirectVariableAccessTest_StorageAS, Param_ptr_i32_Via_struct_read) {
    auto* str_ = ty.Struct(mod.symbols.New("str"), {
                                                       {mod.symbols.Register("i"), ty.i32()},
                                                   });

    Var* S = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 S = b.Var("S", ty.ptr<storage, read>(str_));
                 S->SetBindingPoint(0, 0);
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<storage, i32, read>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage, i32, read>(), S, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

%b1 = block {  # root
  %S:ptr<storage, str, read> = var @binding_point(0, 0)
}

%a = func(%pre:i32, %p:ptr<storage, i32, read>, %post:i32):i32 -> %b2 {
  %b2 = block {
    %6:i32 = load %p
    ret %6
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %8:ptr<storage, i32, read> = access %S, 0u
    %9:i32 = call %a, 10i, %8, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

%b1 = block {  # root
  %S:ptr<storage, str, read> = var @binding_point(0, 0)
}

%a_S_i = func(%pre:i32, %post:i32):i32 -> %b2 {
  %b2 = block {
    %5:ptr<storage, i32, read> = access %S, 0u
    %6:i32 = load %5
    ret %6
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %8:i32 = call %a_S_i, 10i, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_StorageAS, Param_ptr_arr_i32_Via_struct_write) {
    auto* str_ =
        ty.Struct(mod.symbols.New("str"), {
                                              {mod.symbols.Register("arr"), ty.array<i32, 4>()},
                                          });

    Var* S = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 S = b.Var("S", ty.ptr<storage>(str_));
                 S->SetBindingPoint(0, 0);
             });

    auto* fn_a = b.Function("a", ty.void_());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<storage, array<i32, 4>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] {
        b.Store(fn_a_p, b.Splat(ty.array<i32, 4>(), 0_i, 4));
        b.Return(fn_a);
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage, array<i32, 4>>(), S, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  arr:array<i32, 4> @offset(0)
}

%b1 = block {  # root
  %S:ptr<storage, str, read_write> = var @binding_point(0, 0)
}

%a = func(%pre:i32, %p:ptr<storage, array<i32, 4>, read_write>, %post:i32):void -> %b2 {
  %b2 = block {
    store %p, array<i32, 4>(0i)
    ret
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %7:ptr<storage, array<i32, 4>, read_write> = access %S, 0u
    %8:void = call %a, 10i, %7, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
str = struct @align(4) {
  arr:array<i32, 4> @offset(0)
}

%b1 = block {  # root
  %S:ptr<storage, str, read_write> = var @binding_point(0, 0)
}

%a_S_arr = func(%pre:i32, %post:i32):void -> %b2 {
  %b2 = block {
    %5:ptr<storage, array<i32, 4>, read_write> = access %S, 0u
    store %5, array<i32, 4>(0i)
    ret
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %7:void = call %a_S_arr, 10i, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_StorageAS, Param_ptr_vec4i32_Via_array_DynamicWrite) {
    Var* S = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 S = b.Var<storage, array<vec4<i32>, 8>>("S");
                 S->SetBindingPoint(0, 0);
             });

    auto* fn_a = b.Function("a", ty.void_());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<storage, vec4<i32>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] {
        b.Store(fn_a_p, b.Splat(ty.vec4<i32>(), 0_i, 4));
        b.Return(fn_a);
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* I = b.Let("I", 3_i);
        auto* access = b.Access(ty.ptr<storage, vec4<i32>>(), S, I);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
%b1 = block {  # root
  %S:ptr<storage, array<vec4<i32>, 8>, read_write> = var @binding_point(0, 0)
}

%a = func(%pre:i32, %p:ptr<storage, vec4<i32>, read_write>, %post:i32):void -> %b2 {
  %b2 = block {
    store %p, vec4<i32>(0i)
    ret
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %I:i32 = let 3i
    %8:ptr<storage, vec4<i32>, read_write> = access %S, %I
    %9:void = call %a, 10i, %8, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %S:ptr<storage, array<vec4<i32>, 8>, read_write> = var @binding_point(0, 0)
}

%a_S_X = func(%pre:i32, %p_indices:array<u32, 1>, %post:i32):void -> %b2 {
  %b2 = block {
    %6:u32 = access %p_indices, 0u
    %7:ptr<storage, vec4<i32>, read_write> = access %S, %6
    store %7, vec4<i32>(0i)
    ret
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %I:i32 = let 3i
    %10:u32 = convert %I
    %11:array<u32, 1> = construct %10
    %12:void = call %a_S_X, 10i, %11, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_StorageAS, CallChaining) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    auto* Outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.Register("arr"), ty.array(Inner, 4)},
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    Var* S = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 S = b.Var("S", ty.ptr<storage, read>(Outer));
                 S->SetBindingPoint(0, 0);
             });

    auto* fn_0 = b.Function("f0", ty.f32());
    auto* fn_0_p = b.FunctionParam("p", ty.ptr<storage, vec4<f32>, read>());
    fn_0->SetParams({fn_0_p});
    b.Append(fn_0->Block(), [&] { b.Return(fn_0, b.LoadVectorElement(fn_0_p, 0_u)); });

    auto* fn_1 = b.Function("f1", ty.f32());
    auto* fn_1_p = b.FunctionParam("p", ty.ptr<storage, mat3x4<f32>, read>());
    fn_1->SetParams({fn_1_p});
    b.Append(fn_1->Block(), [&] {
        auto* res = b.Var<function, f32>("res");
        {
            // res += f0(&(*p)[1]);
            auto* call_0 = b.Call(fn_0, b.Access(ty.ptr<storage, vec4<f32>, read>(), fn_1_p, 1_i));
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &(*p)[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<storage, vec4<f32>, read>(), fn_1_p, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // res += f0(&U.arr[2].mat[1]);
            auto* access = b.Access(ty.ptr<storage, vec4<f32>, read>(), S, 0_u, 2_i, 0_u, 1_i);
            auto* call_0 = b.Call(fn_0, access);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &U.arr[2].mat[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<storage, vec4<f32>, read>(), S, 0_u, 2_i, 0_u, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }

        b.Return(fn_1, b.Load(res));
    });

    auto* fn_2 = b.Function("f2", ty.f32());
    auto* fn_2_p = b.FunctionParam("p", ty.ptr<storage, read>(Inner));
    fn_2->SetParams({fn_2_p});
    b.Append(fn_2->Block(), [&] {
        auto* p_mat = b.Access(ty.ptr<storage, mat3x4<f32>, read>(), fn_2_p, 0_u);
        b.ir.SetName(p_mat, "p_mat");
        b.Return(fn_2, b.Call(fn_1, p_mat));
    });

    auto* fn_3 = b.Function("f3", ty.f32());
    auto* fn_3_p0 = b.FunctionParam("p0", ty.ptr<storage, read>(ty.array(Inner, 4)));
    auto* fn_3_p1 = b.FunctionParam("p1", ty.ptr<storage, mat3x4<f32>, read>());
    fn_3->SetParams({fn_3_p0, fn_3_p1});
    b.Append(fn_3->Block(), [&] {
        auto* p0_inner = b.Access(ty.ptr<storage, read>(Inner), fn_3_p0, 3_i);
        b.ir.SetName(p0_inner, "p0_inner");
        auto* call_0 = b.Call(ty.f32(), fn_2, p0_inner);
        auto* call_1 = b.Call(ty.f32(), fn_1, fn_3_p1);
        b.Return(fn_3, b.Add(ty.f32(), call_0, call_1));
    });

    auto* fn_4 = b.Function("f4", ty.f32());
    auto* fn_4_p = b.FunctionParam("p", ty.ptr<storage, read>(Outer));
    fn_4->SetParams({fn_4_p});
    b.Append(fn_4->Block(), [&] {
        auto* access_0 = b.Access(ty.ptr<storage, read>(ty.array(Inner, 4)), fn_4_p, 0_u);
        auto* access_1 = b.Access(ty.ptr<storage, mat3x4<f32>, read>(), S, 1_u);
        b.Return(fn_4, b.Call(ty.f32(), fn_3, access_0, access_1));
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        b.Call(ty.f32(), fn_4, S);
        b.Return(fn_b);
    });

    auto* src = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

%b1 = block {  # root
  %S:ptr<storage, Outer, read> = var @binding_point(0, 0)
}

%f0 = func(%p:ptr<storage, vec4<f32>, read>):f32 -> %b2 {
  %b2 = block {
    %4:f32 = load_vector_element %p, 0u
    ret %4
  }
}
%f1 = func(%p_1:ptr<storage, mat3x4<f32>, read>):f32 -> %b3 {  # %p_1: 'p'
  %b3 = block {
    %res:ptr<function, f32, read_write> = var
    %8:ptr<storage, vec4<f32>, read> = access %p_1, 1i
    %9:f32 = call %f0, %8
    %10:f32 = load %res
    %11:f32 = add %10, %9
    store %res, %11
    %p_vec:ptr<storage, vec4<f32>, read> = access %p_1, 1i
    %13:f32 = call %f0, %p_vec
    %14:f32 = load %res
    %15:f32 = add %14, %13
    store %res, %15
    %16:ptr<storage, vec4<f32>, read> = access %S, 0u, 2i, 0u, 1i
    %17:f32 = call %f0, %16
    %18:f32 = load %res
    %19:f32 = add %18, %17
    store %res, %19
    %p_vec_1:ptr<storage, vec4<f32>, read> = access %S, 0u, 2i, 0u, 1i  # %p_vec_1: 'p_vec'
    %21:f32 = call %f0, %p_vec_1
    %22:f32 = load %res
    %23:f32 = add %22, %21
    store %res, %23
    %24:f32 = load %res
    ret %24
  }
}
%f2 = func(%p_2:ptr<storage, Inner, read>):f32 -> %b4 {  # %p_2: 'p'
  %b4 = block {
    %p_mat:ptr<storage, mat3x4<f32>, read> = access %p_2, 0u
    %28:f32 = call %f1, %p_mat
    ret %28
  }
}
%f3 = func(%p0:ptr<storage, array<Inner, 4>, read>, %p1:ptr<storage, mat3x4<f32>, read>):f32 -> %b5 {
  %b5 = block {
    %p0_inner:ptr<storage, Inner, read> = access %p0, 3i
    %33:f32 = call %f2, %p0_inner
    %34:f32 = call %f1, %p1
    %35:f32 = add %33, %34
    ret %35
  }
}
%f4 = func(%p_3:ptr<storage, Outer, read>):f32 -> %b6 {  # %p_3: 'p'
  %b6 = block {
    %38:ptr<storage, array<Inner, 4>, read> = access %p_3, 0u
    %39:ptr<storage, mat3x4<f32>, read> = access %S, 1u
    %40:f32 = call %f3, %38, %39
    ret %40
  }
}
%b = func():void -> %b7 {
  %b7 = block {
    %42:f32 = call %f4, %S
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

%b1 = block {  # root
  %S:ptr<storage, Outer, read> = var @binding_point(0, 0)
}

%f0_S_mat_X = func(%p_indices:array<u32, 1>):f32 -> %b2 {
  %b2 = block {
    %4:u32 = access %p_indices, 0u
    %5:ptr<storage, vec4<f32>, read> = access %S, 1u, %4
    %6:f32 = load_vector_element %5, 0u
    ret %6
  }
}
%f0_S_arr_X_mat_X = func(%p_indices_1:array<u32, 2>):f32 -> %b3 {  # %p_indices_1: 'p_indices'
  %b3 = block {
    %9:u32 = access %p_indices_1, 0u
    %10:u32 = access %p_indices_1, 1u
    %11:ptr<storage, vec4<f32>, read> = access %S, 0u, %9, 0u, %10
    %12:f32 = load_vector_element %11, 0u
    ret %12
  }
}
%f1_S_mat = func():f32 -> %b4 {
  %b4 = block {
    %res:ptr<function, f32, read_write> = var
    %15:u32 = convert 1i
    %16:array<u32, 1> = construct %15
    %17:f32 = call %f0_S_mat_X, %16
    %18:f32 = load %res
    %19:f32 = add %18, %17
    store %res, %19
    %20:u32 = convert 1i
    %21:array<u32, 1> = construct %20
    %22:f32 = call %f0_S_mat_X, %21
    %23:f32 = load %res
    %24:f32 = add %23, %22
    store %res, %24
    %25:u32 = convert 2i
    %26:u32 = convert 1i
    %27:array<u32, 2> = construct %25, %26
    %28:f32 = call %f0_S_arr_X_mat_X, %27
    %29:f32 = load %res
    %30:f32 = add %29, %28
    store %res, %30
    %31:u32 = convert 2i
    %32:u32 = convert 1i
    %33:array<u32, 2> = construct %31, %32
    %34:f32 = call %f0_S_arr_X_mat_X, %33
    %35:f32 = load %res
    %36:f32 = add %35, %34
    store %res, %36
    %37:f32 = load %res
    ret %37
  }
}
%f1_S_arr_X_mat = func(%p_indices_2:array<u32, 1>):f32 -> %b5 {  # %p_indices_2: 'p_indices'
  %b5 = block {
    %40:u32 = access %p_indices_2, 0u
    %res_1:ptr<function, f32, read_write> = var  # %res_1: 'res'
    %42:u32 = convert 1i
    %43:array<u32, 2> = construct %40, %42
    %44:f32 = call %f0_S_arr_X_mat_X, %43
    %45:f32 = load %res_1
    %46:f32 = add %45, %44
    store %res_1, %46
    %47:u32 = convert 1i
    %48:array<u32, 2> = construct %40, %47
    %49:f32 = call %f0_S_arr_X_mat_X, %48
    %50:f32 = load %res_1
    %51:f32 = add %50, %49
    store %res_1, %51
    %52:u32 = convert 2i
    %53:u32 = convert 1i
    %54:array<u32, 2> = construct %52, %53
    %55:f32 = call %f0_S_arr_X_mat_X, %54
    %56:f32 = load %res_1
    %57:f32 = add %56, %55
    store %res_1, %57
    %58:u32 = convert 2i
    %59:u32 = convert 1i
    %60:array<u32, 2> = construct %58, %59
    %61:f32 = call %f0_S_arr_X_mat_X, %60
    %62:f32 = load %res_1
    %63:f32 = add %62, %61
    store %res_1, %63
    %64:f32 = load %res_1
    ret %64
  }
}
%f2_S_arr_X = func(%p_indices_3:array<u32, 1>):f32 -> %b6 {  # %p_indices_3: 'p_indices'
  %b6 = block {
    %67:u32 = access %p_indices_3, 0u
    %68:array<u32, 1> = construct %67
    %69:f32 = call %f1_S_arr_X_mat, %68
    ret %69
  }
}
%f3_S_arr_S_mat = func():f32 -> %b7 {
  %b7 = block {
    %71:u32 = convert 3i
    %72:array<u32, 1> = construct %71
    %73:f32 = call %f2_S_arr_X, %72
    %74:f32 = call %f1_S_mat
    %75:f32 = add %73, %74
    ret %75
  }
}
%f4_S = func():f32 -> %b8 {
  %b8 = block {
    %77:f32 = call %f3_S_arr_S_mat
    ret %77
  }
}
%b = func():void -> %b9 {
  %b9 = block {
    %79:f32 = call %f4_S
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

}  // namespace storage_as_tests

////////////////////////////////////////////////////////////////////////////////
// 'workgroup' address space
////////////////////////////////////////////////////////////////////////////////
namespace workgroup_as_tests {

using IR_DirectVariableAccessTest_WorkgroupAS = TransformTest;

TEST_F(IR_DirectVariableAccessTest_WorkgroupAS, Param_ptr_vec4i32_Via_array_StaticRead) {
    Var* W = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 W = b.Var("W", ty.ptr<workgroup, array<vec4<i32>, 8>>());
             });

    auto* fn_a = b.Function("a", ty.vec4<i32>());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<workgroup, vec4<i32>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* access = b.Access(ty.ptr<workgroup, vec4<i32>>(), W, 3_i);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
%b1 = block {  # root
  %W:ptr<workgroup, array<vec4<i32>, 8>, read_write> = var
}

%a = func(%pre:i32, %p:ptr<workgroup, vec4<i32>, read_write>, %post:i32):vec4<i32> -> %b2 {
  %b2 = block {
    %6:vec4<i32> = load %p
    ret %6
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %8:ptr<workgroup, vec4<i32>, read_write> = access %W, 3i
    %9:vec4<i32> = call %a, 10i, %8, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %W:ptr<workgroup, array<vec4<i32>, 8>, read_write> = var
}

%a_W_X = func(%pre:i32, %p_indices:array<u32, 1>, %post:i32):vec4<i32> -> %b2 {
  %b2 = block {
    %6:u32 = access %p_indices, 0u
    %7:ptr<workgroup, vec4<i32>, read_write> = access %W, %6
    %8:vec4<i32> = load %7
    ret %8
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %10:u32 = convert 3i
    %11:array<u32, 1> = construct %10
    %12:vec4<i32> = call %a_W_X, 10i, %11, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_WorkgroupAS, Param_ptr_vec4i32_Via_array_StaticWrite) {
    Var* W = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 W = b.Var<workgroup, array<vec4<i32>, 8>>("W");
             });

    auto* fn_a = b.Function("a", ty.void_());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<workgroup, vec4<i32>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] {
        b.Store(fn_a_p, b.Splat(ty.vec4<i32>(), 0_i, 4));
        b.Return(fn_a);
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* access = b.Access(ty.ptr<workgroup, vec4<i32>>(), W, 3_i);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
%b1 = block {  # root
  %W:ptr<workgroup, array<vec4<i32>, 8>, read_write> = var
}

%a = func(%pre:i32, %p:ptr<workgroup, vec4<i32>, read_write>, %post:i32):void -> %b2 {
  %b2 = block {
    store %p, vec4<i32>(0i)
    ret
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %7:ptr<workgroup, vec4<i32>, read_write> = access %W, 3i
    %8:void = call %a, 10i, %7, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %W:ptr<workgroup, array<vec4<i32>, 8>, read_write> = var
}

%a_W_X = func(%pre:i32, %p_indices:array<u32, 1>, %post:i32):void -> %b2 {
  %b2 = block {
    %6:u32 = access %p_indices, 0u
    %7:ptr<workgroup, vec4<i32>, read_write> = access %W, %6
    store %7, vec4<i32>(0i)
    ret
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %9:u32 = convert 3i
    %10:array<u32, 1> = construct %9
    %11:void = call %a_W_X, 10i, %10, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_WorkgroupAS, CallChaining) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    auto* Outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.Register("arr"), ty.array(Inner, 4)},
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    Var* W = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 W = b.Var("W", ty.ptr<workgroup>(Outer));
             });

    auto* fn_0 = b.Function("f0", ty.f32());
    auto* fn_0_p = b.FunctionParam("p", ty.ptr<workgroup, vec4<f32>>());
    fn_0->SetParams({fn_0_p});
    b.Append(fn_0->Block(), [&] { b.Return(fn_0, b.LoadVectorElement(fn_0_p, 0_u)); });

    auto* fn_1 = b.Function("f1", ty.f32());
    auto* fn_1_p = b.FunctionParam("p", ty.ptr<workgroup, mat3x4<f32>>());
    fn_1->SetParams({fn_1_p});
    b.Append(fn_1->Block(), [&] {
        auto* res = b.Var<function, f32>("res");
        {
            // res += f0(&(*p)[1]);
            auto* call_0 = b.Call(fn_0, b.Access(ty.ptr<workgroup, vec4<f32>>(), fn_1_p, 1_i));
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &(*p)[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<workgroup, vec4<f32>>(), fn_1_p, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // res += f0(&U.arr[2].mat[1]);
            auto* access = b.Access(ty.ptr<workgroup, vec4<f32>>(), W, 0_u, 2_i, 0_u, 1_i);
            auto* call_0 = b.Call(fn_0, access);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &U.arr[2].mat[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<workgroup, vec4<f32>>(), W, 0_u, 2_i, 0_u, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }

        b.Return(fn_1, b.Load(res));
    });

    auto* fn_2 = b.Function("f2", ty.f32());
    auto* fn_2_p = b.FunctionParam("p", ty.ptr<workgroup>(Inner));
    fn_2->SetParams({fn_2_p});
    b.Append(fn_2->Block(), [&] {
        auto* p_mat = b.Access(ty.ptr<workgroup, mat3x4<f32>>(), fn_2_p, 0_u);
        b.ir.SetName(p_mat, "p_mat");
        b.Return(fn_2, b.Call(fn_1, p_mat));
    });

    auto* fn_3 = b.Function("f3", ty.f32());
    auto* fn_3_p0 = b.FunctionParam("p0", ty.ptr<workgroup>(ty.array(Inner, 4)));
    auto* fn_3_p1 = b.FunctionParam("p1", ty.ptr<workgroup, mat3x4<f32>>());
    fn_3->SetParams({fn_3_p0, fn_3_p1});
    b.Append(fn_3->Block(), [&] {
        auto* p0_inner = b.Access(ty.ptr<workgroup>(Inner), fn_3_p0, 3_i);
        b.ir.SetName(p0_inner, "p0_inner");
        auto* call_0 = b.Call(ty.f32(), fn_2, p0_inner);
        auto* call_1 = b.Call(ty.f32(), fn_1, fn_3_p1);
        b.Return(fn_3, b.Add(ty.f32(), call_0, call_1));
    });

    auto* fn_4 = b.Function("f4", ty.f32());
    auto* fn_4_p = b.FunctionParam("p", ty.ptr<workgroup>(Outer));
    fn_4->SetParams({fn_4_p});
    b.Append(fn_4->Block(), [&] {
        auto* access_0 = b.Access(ty.ptr<workgroup>(ty.array(Inner, 4)), fn_4_p, 0_u);
        auto* access_1 = b.Access(ty.ptr<workgroup, mat3x4<f32>>(), W, 1_u);
        b.Return(fn_4, b.Call(ty.f32(), fn_3, access_0, access_1));
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        b.Call(ty.f32(), fn_4, W);
        b.Return(fn_b);
    });

    auto* src = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

%b1 = block {  # root
  %W:ptr<workgroup, Outer, read_write> = var
}

%f0 = func(%p:ptr<workgroup, vec4<f32>, read_write>):f32 -> %b2 {
  %b2 = block {
    %4:f32 = load_vector_element %p, 0u
    ret %4
  }
}
%f1 = func(%p_1:ptr<workgroup, mat3x4<f32>, read_write>):f32 -> %b3 {  # %p_1: 'p'
  %b3 = block {
    %res:ptr<function, f32, read_write> = var
    %8:ptr<workgroup, vec4<f32>, read_write> = access %p_1, 1i
    %9:f32 = call %f0, %8
    %10:f32 = load %res
    %11:f32 = add %10, %9
    store %res, %11
    %p_vec:ptr<workgroup, vec4<f32>, read_write> = access %p_1, 1i
    %13:f32 = call %f0, %p_vec
    %14:f32 = load %res
    %15:f32 = add %14, %13
    store %res, %15
    %16:ptr<workgroup, vec4<f32>, read_write> = access %W, 0u, 2i, 0u, 1i
    %17:f32 = call %f0, %16
    %18:f32 = load %res
    %19:f32 = add %18, %17
    store %res, %19
    %p_vec_1:ptr<workgroup, vec4<f32>, read_write> = access %W, 0u, 2i, 0u, 1i  # %p_vec_1: 'p_vec'
    %21:f32 = call %f0, %p_vec_1
    %22:f32 = load %res
    %23:f32 = add %22, %21
    store %res, %23
    %24:f32 = load %res
    ret %24
  }
}
%f2 = func(%p_2:ptr<workgroup, Inner, read_write>):f32 -> %b4 {  # %p_2: 'p'
  %b4 = block {
    %p_mat:ptr<workgroup, mat3x4<f32>, read_write> = access %p_2, 0u
    %28:f32 = call %f1, %p_mat
    ret %28
  }
}
%f3 = func(%p0:ptr<workgroup, array<Inner, 4>, read_write>, %p1:ptr<workgroup, mat3x4<f32>, read_write>):f32 -> %b5 {
  %b5 = block {
    %p0_inner:ptr<workgroup, Inner, read_write> = access %p0, 3i
    %33:f32 = call %f2, %p0_inner
    %34:f32 = call %f1, %p1
    %35:f32 = add %33, %34
    ret %35
  }
}
%f4 = func(%p_3:ptr<workgroup, Outer, read_write>):f32 -> %b6 {  # %p_3: 'p'
  %b6 = block {
    %38:ptr<workgroup, array<Inner, 4>, read_write> = access %p_3, 0u
    %39:ptr<workgroup, mat3x4<f32>, read_write> = access %W, 1u
    %40:f32 = call %f3, %38, %39
    ret %40
  }
}
%b = func():void -> %b7 {
  %b7 = block {
    %42:f32 = call %f4, %W
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

%b1 = block {  # root
  %W:ptr<workgroup, Outer, read_write> = var
}

%f0_W_mat_X = func(%p_indices:array<u32, 1>):f32 -> %b2 {
  %b2 = block {
    %4:u32 = access %p_indices, 0u
    %5:ptr<workgroup, vec4<f32>, read_write> = access %W, 1u, %4
    %6:f32 = load_vector_element %5, 0u
    ret %6
  }
}
%f0_W_arr_X_mat_X = func(%p_indices_1:array<u32, 2>):f32 -> %b3 {  # %p_indices_1: 'p_indices'
  %b3 = block {
    %9:u32 = access %p_indices_1, 0u
    %10:u32 = access %p_indices_1, 1u
    %11:ptr<workgroup, vec4<f32>, read_write> = access %W, 0u, %9, 0u, %10
    %12:f32 = load_vector_element %11, 0u
    ret %12
  }
}
%f1_W_mat = func():f32 -> %b4 {
  %b4 = block {
    %res:ptr<function, f32, read_write> = var
    %15:u32 = convert 1i
    %16:array<u32, 1> = construct %15
    %17:f32 = call %f0_W_mat_X, %16
    %18:f32 = load %res
    %19:f32 = add %18, %17
    store %res, %19
    %20:u32 = convert 1i
    %21:array<u32, 1> = construct %20
    %22:f32 = call %f0_W_mat_X, %21
    %23:f32 = load %res
    %24:f32 = add %23, %22
    store %res, %24
    %25:u32 = convert 2i
    %26:u32 = convert 1i
    %27:array<u32, 2> = construct %25, %26
    %28:f32 = call %f0_W_arr_X_mat_X, %27
    %29:f32 = load %res
    %30:f32 = add %29, %28
    store %res, %30
    %31:u32 = convert 2i
    %32:u32 = convert 1i
    %33:array<u32, 2> = construct %31, %32
    %34:f32 = call %f0_W_arr_X_mat_X, %33
    %35:f32 = load %res
    %36:f32 = add %35, %34
    store %res, %36
    %37:f32 = load %res
    ret %37
  }
}
%f1_W_arr_X_mat = func(%p_indices_2:array<u32, 1>):f32 -> %b5 {  # %p_indices_2: 'p_indices'
  %b5 = block {
    %40:u32 = access %p_indices_2, 0u
    %res_1:ptr<function, f32, read_write> = var  # %res_1: 'res'
    %42:u32 = convert 1i
    %43:array<u32, 2> = construct %40, %42
    %44:f32 = call %f0_W_arr_X_mat_X, %43
    %45:f32 = load %res_1
    %46:f32 = add %45, %44
    store %res_1, %46
    %47:u32 = convert 1i
    %48:array<u32, 2> = construct %40, %47
    %49:f32 = call %f0_W_arr_X_mat_X, %48
    %50:f32 = load %res_1
    %51:f32 = add %50, %49
    store %res_1, %51
    %52:u32 = convert 2i
    %53:u32 = convert 1i
    %54:array<u32, 2> = construct %52, %53
    %55:f32 = call %f0_W_arr_X_mat_X, %54
    %56:f32 = load %res_1
    %57:f32 = add %56, %55
    store %res_1, %57
    %58:u32 = convert 2i
    %59:u32 = convert 1i
    %60:array<u32, 2> = construct %58, %59
    %61:f32 = call %f0_W_arr_X_mat_X, %60
    %62:f32 = load %res_1
    %63:f32 = add %62, %61
    store %res_1, %63
    %64:f32 = load %res_1
    ret %64
  }
}
%f2_W_arr_X = func(%p_indices_3:array<u32, 1>):f32 -> %b6 {  # %p_indices_3: 'p_indices'
  %b6 = block {
    %67:u32 = access %p_indices_3, 0u
    %68:array<u32, 1> = construct %67
    %69:f32 = call %f1_W_arr_X_mat, %68
    ret %69
  }
}
%f3_W_arr_W_mat = func():f32 -> %b7 {
  %b7 = block {
    %71:u32 = convert 3i
    %72:array<u32, 1> = construct %71
    %73:f32 = call %f2_W_arr_X, %72
    %74:f32 = call %f1_W_mat
    %75:f32 = add %73, %74
    ret %75
  }
}
%f4_W = func():f32 -> %b8 {
  %b8 = block {
    %77:f32 = call %f3_W_arr_W_mat
    ret %77
  }
}
%b = func():void -> %b9 {
  %b9 = block {
    %79:f32 = call %f4_W
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

}  // namespace workgroup_as_tests

////////////////////////////////////////////////////////////////////////////////
// 'private' address space
////////////////////////////////////////////////////////////////////////////////
namespace private_as_tests {

using IR_DirectVariableAccessTest_PrivateAS = TransformTest;

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Enabled_Param_ptr_i32_read) {
    Var* P = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 P = b.Var("P", ty.ptr<private_, i32>());
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<private_, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        b.Call(fn_a, 10_i, P, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
%b1 = block {  # root
  %P:ptr<private, i32, read_write> = var
}

%a = func(%pre:i32, %p:ptr<private, i32, read_write>, %post:i32):i32 -> %b2 {
  %b2 = block {
    %6:i32 = load %p
    ret %6
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %8:i32 = call %a, 10i, %P, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %P:ptr<private, i32, read_write> = var
}

%a_P = func(%pre:i32, %post:i32):i32 -> %b2 {
  %b2 = block {
    %5:ptr<private, i32, read_write> = access %P
    %6:i32 = load %5
    ret %6
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %8:i32 = call %a_P, 10i, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformPrivate);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Enabled_Param_ptr_i32_write) {
    Var* P = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 P = b.Var("P", ty.ptr<private_, i32>());
             });

    auto* fn_a = b.Function("a", ty.void_());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<private_, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] {
        b.Store(fn_a_p, 42_i);
        b.Return(fn_a);
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        b.Call(fn_a, 10_i, P, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
%b1 = block {  # root
  %P:ptr<private, i32, read_write> = var
}

%a = func(%pre:i32, %p:ptr<private, i32, read_write>, %post:i32):void -> %b2 {
  %b2 = block {
    store %p, 42i
    ret
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %7:void = call %a, 10i, %P, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %P:ptr<private, i32, read_write> = var
}

%a_P = func(%pre:i32, %post:i32):void -> %b2 {
  %b2 = block {
    %5:ptr<private, i32, read_write> = access %P
    store %5, 42i
    ret
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %7:void = call %a_P, 10i, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformPrivate);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Enabled_Param_ptr_i32_Via_struct_read) {
    auto* str_ = ty.Struct(mod.symbols.New("str"), {
                                                       {mod.symbols.Register("i"), ty.i32()},
                                                   });

    Var* P = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 P = b.Var("P", ty.ptr<private_>(str_));
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<private_, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* access = b.Access(ty.ptr<private_, i32>(), P, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

%b1 = block {  # root
  %P:ptr<private, str, read_write> = var
}

%a = func(%pre:i32, %p:ptr<private, i32, read_write>, %post:i32):i32 -> %b2 {
  %b2 = block {
    %6:i32 = load %p
    ret %6
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %8:ptr<private, i32, read_write> = access %P, 0u
    %9:i32 = call %a, 10i, %8, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

%b1 = block {  # root
  %P:ptr<private, str, read_write> = var
}

%a_P_i = func(%pre:i32, %post:i32):i32 -> %b2 {
  %b2 = block {
    %5:ptr<private, i32, read_write> = access %P, 0u
    %6:i32 = load %5
    ret %6
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %8:i32 = call %a_P_i, 10i, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformPrivate);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Disabled_Param_ptr_i32_Via_struct_read) {
    auto* str_ = ty.Struct(mod.symbols.New("str"), {
                                                       {mod.symbols.Register("i"), ty.i32()},
                                                   });

    Var* P = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 P = b.Var("P", ty.ptr<private_>(str_));
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<private_, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* access = b.Access(ty.ptr<private_, i32>(), P, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

%b1 = block {  # root
  %P:ptr<private, str, read_write> = var
}

%a = func(%pre:i32, %p:ptr<private, i32, read_write>, %post:i32):i32 -> %b2 {
  %b2 = block {
    %6:i32 = load %p
    ret %6
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %8:ptr<private, i32, read_write> = access %P, 0u
    %9:i32 = call %a, 10i, %8, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Enabled_Param_ptr_arr_i32_Via_struct_write) {
    auto* str_ =
        ty.Struct(mod.symbols.New("str"), {
                                              {mod.symbols.Register("arr"), ty.array<i32, 4>()},
                                          });

    Var* P = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 P = b.Var("P", ty.ptr<private_>(str_));
             });

    auto* fn_a = b.Function("a", ty.void_());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<private_, array<i32, 4>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] {
        b.Store(fn_a_p, b.Splat(ty.array<i32, 4>(), 0_i, 4));
        b.Return(fn_a);
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* access = b.Access(ty.ptr<private_, array<i32, 4>>(), P, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  arr:array<i32, 4> @offset(0)
}

%b1 = block {  # root
  %P:ptr<private, str, read_write> = var
}

%a = func(%pre:i32, %p:ptr<private, array<i32, 4>, read_write>, %post:i32):void -> %b2 {
  %b2 = block {
    store %p, array<i32, 4>(0i)
    ret
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %7:ptr<private, array<i32, 4>, read_write> = access %P, 0u
    %8:void = call %a, 10i, %7, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
str = struct @align(4) {
  arr:array<i32, 4> @offset(0)
}

%b1 = block {  # root
  %P:ptr<private, str, read_write> = var
}

%a_P_arr = func(%pre:i32, %post:i32):void -> %b2 {
  %b2 = block {
    %5:ptr<private, array<i32, 4>, read_write> = access %P, 0u
    store %5, array<i32, 4>(0i)
    ret
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %7:void = call %a_P_arr, 10i, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformPrivate);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Disabled_Param_ptr_arr_i32_Via_struct_write) {
    auto* str_ =
        ty.Struct(mod.symbols.New("str"), {
                                              {mod.symbols.Register("arr"), ty.array<i32, 4>()},
                                          });

    Var* P = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 P = b.Var("P", ty.ptr<private_>(str_));
             });

    auto* fn_a = b.Function("a", ty.void_());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<private_, array<i32, 4>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] {
        b.Store(fn_a_p, b.Splat(ty.array<i32, 4>(), 0_i, 4));
        b.Return(fn_a);
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* access = b.Access(ty.ptr<private_, array<i32, 4>>(), P, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  arr:array<i32, 4> @offset(0)
}

%b1 = block {  # root
  %P:ptr<private, str, read_write> = var
}

%a = func(%pre:i32, %p:ptr<private, array<i32, 4>, read_write>, %post:i32):void -> %b2 {
  %b2 = block {
    store %p, array<i32, 4>(0i)
    ret
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %7:ptr<private, array<i32, 4>, read_write> = access %P, 0u
    %8:void = call %a, 10i, %7, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Enabled_Param_ptr_i32_mixed) {
    auto* str_ = ty.Struct(mod.symbols.New("str"), {
                                                       {mod.symbols.Register("i"), ty.i32()},
                                                   });

    Var* Pi = nullptr;
    Var* Ps = nullptr;
    Var* Pa = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 Pi = b.Var("Pi", ty.ptr<private_, i32>());
                 Ps = b.Var("Ps", ty.ptr<private_>(str_));
                 Pa = b.Var("Pa", ty.ptr<private_, array<i32, 4>>());
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<private_, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        {  // a(10, &Pi, 20);
            b.Call(fn_a, 10_i, Pi, 20_i);
        }
        {  // a(30, &Ps.i, 40);
            auto* access = b.Access(ty.ptr<private_, i32>(), Ps, 0_u);
            b.Call(fn_a, 30_i, access, 40_i);
        }
        {  // a(50, &Pa[2], 60);
            auto* access = b.Access(ty.ptr<private_, i32>(), Pa, 2_i);
            b.Call(fn_a, 50_i, access, 60_i);
        }
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

%b1 = block {  # root
  %Pi:ptr<private, i32, read_write> = var
  %Ps:ptr<private, str, read_write> = var
  %Pa:ptr<private, array<i32, 4>, read_write> = var
}

%a = func(%pre:i32, %p:ptr<private, i32, read_write>, %post:i32):i32 -> %b2 {
  %b2 = block {
    %8:i32 = load %p
    ret %8
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %10:i32 = call %a, 10i, %Pi, 20i
    %11:ptr<private, i32, read_write> = access %Ps, 0u
    %12:i32 = call %a, 30i, %11, 40i
    %13:ptr<private, i32, read_write> = access %Pa, 2i
    %14:i32 = call %a, 50i, %13, 60i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

%b1 = block {  # root
  %Pi:ptr<private, i32, read_write> = var
  %Ps:ptr<private, str, read_write> = var
  %Pa:ptr<private, array<i32, 4>, read_write> = var
}

%a_Pi = func(%pre:i32, %post:i32):i32 -> %b2 {
  %b2 = block {
    %7:ptr<private, i32, read_write> = access %Pi
    %8:i32 = load %7
    ret %8
  }
}
%a_Ps_i = func(%pre_1:i32, %post_1:i32):i32 -> %b3 {  # %pre_1: 'pre', %post_1: 'post'
  %b3 = block {
    %12:ptr<private, i32, read_write> = access %Ps, 0u
    %13:i32 = load %12
    ret %13
  }
}
%a_Pa_X = func(%pre_2:i32, %p_indices:array<u32, 1>, %post_2:i32):i32 -> %b4 {  # %pre_2: 'pre', %post_2: 'post'
  %b4 = block {
    %18:u32 = access %p_indices, 0u
    %19:ptr<private, i32, read_write> = access %Pa, %18
    %20:i32 = load %19
    ret %20
  }
}
%b = func():void -> %b5 {
  %b5 = block {
    %22:i32 = call %a_Pi, 10i, 20i
    %23:i32 = call %a_Ps_i, 30i, 40i
    %24:u32 = convert 2i
    %25:array<u32, 1> = construct %24
    %26:i32 = call %a_Pa_X, 50i, %25, 60i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformPrivate);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Disabled_Param_ptr_i32_mixed) {
    auto* str_ = ty.Struct(mod.symbols.New("str"), {
                                                       {mod.symbols.Register("i"), ty.i32()},
                                                   });

    Var* Pi = nullptr;
    Var* Ps = nullptr;
    Var* Pa = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 Pi = b.Var("Pi", ty.ptr<private_, i32>());
                 Ps = b.Var("Ps", ty.ptr<private_>(str_));
                 Pa = b.Var("Pa", ty.ptr<private_, array<i32, 4>>());
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<private_, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        {  // a(10, &Pi, 20);
            b.Call(fn_a, 10_i, Pi, 20_i);
        }
        {  // a(30, &Ps.i, 40);
            auto* access = b.Access(ty.ptr<private_, i32>(), Ps, 0_u);
            b.Call(fn_a, 30_i, access, 40_i);
        }
        {  // a(50, &Pa[2], 60);
            auto* access = b.Access(ty.ptr<private_, i32>(), Pa, 2_i);
            b.Call(fn_a, 50_i, access, 60_i);
        }
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

%b1 = block {  # root
  %Pi:ptr<private, i32, read_write> = var
  %Ps:ptr<private, str, read_write> = var
  %Pa:ptr<private, array<i32, 4>, read_write> = var
}

%a = func(%pre:i32, %p:ptr<private, i32, read_write>, %post:i32):i32 -> %b2 {
  %b2 = block {
    %8:i32 = load %p
    ret %8
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %10:i32 = call %a, 10i, %Pi, 20i
    %11:ptr<private, i32, read_write> = access %Ps, 0u
    %12:i32 = call %a, 30i, %11, 40i
    %13:ptr<private, i32, read_write> = access %Pa, 2i
    %14:i32 = call %a, 50i, %13, 60i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Enabled_CallChaining) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    auto* Outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.Register("arr"), ty.array(Inner, 4)},
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    Var* P = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 P = b.Var("P", ty.ptr<private_>(Outer));
             });

    auto* fn_0 = b.Function("f0", ty.f32());
    auto* fn_0_p = b.FunctionParam("p", ty.ptr<private_, vec4<f32>>());
    fn_0->SetParams({fn_0_p});
    b.Append(fn_0->Block(), [&] { b.Return(fn_0, b.LoadVectorElement(fn_0_p, 0_u)); });

    auto* fn_1 = b.Function("f1", ty.f32());
    auto* fn_1_p = b.FunctionParam("p", ty.ptr<private_, mat3x4<f32>>());
    fn_1->SetParams({fn_1_p});
    b.Append(fn_1->Block(), [&] {
        auto* res = b.Var<function, f32>("res");
        {
            // res += f0(&(*p)[1]);
            auto* call_0 = b.Call(fn_0, b.Access(ty.ptr<private_, vec4<f32>>(), fn_1_p, 1_i));
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &(*p)[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<private_, vec4<f32>>(), fn_1_p, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // res += f0(&U.arr[2].mat[1]);
            auto* access = b.Access(ty.ptr<private_, vec4<f32>>(), P, 0_u, 2_i, 0_u, 1_i);
            auto* call_0 = b.Call(fn_0, access);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &U.arr[2].mat[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<private_, vec4<f32>>(), P, 0_u, 2_i, 0_u, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }

        b.Return(fn_1, b.Load(res));
    });

    auto* fn_2 = b.Function("f2", ty.f32());
    auto* fn_2_p = b.FunctionParam("p", ty.ptr<private_>(Inner));
    fn_2->SetParams({fn_2_p});
    b.Append(fn_2->Block(), [&] {
        auto* p_mat = b.Access(ty.ptr<private_, mat3x4<f32>>(), fn_2_p, 0_u);
        b.ir.SetName(p_mat, "p_mat");
        b.Return(fn_2, b.Call(fn_1, p_mat));
    });

    auto* fn_3 = b.Function("f3", ty.f32());
    auto* fn_3_p0 = b.FunctionParam("p0", ty.ptr<private_>(ty.array(Inner, 4)));
    auto* fn_3_p1 = b.FunctionParam("p1", ty.ptr<private_, mat3x4<f32>>());
    fn_3->SetParams({fn_3_p0, fn_3_p1});
    b.Append(fn_3->Block(), [&] {
        auto* p0_inner = b.Access(ty.ptr<private_>(Inner), fn_3_p0, 3_i);
        b.ir.SetName(p0_inner, "p0_inner");
        auto* call_0 = b.Call(ty.f32(), fn_2, p0_inner);
        auto* call_1 = b.Call(ty.f32(), fn_1, fn_3_p1);
        b.Return(fn_3, b.Add(ty.f32(), call_0, call_1));
    });

    auto* fn_4 = b.Function("f4", ty.f32());
    auto* fn_4_p = b.FunctionParam("p", ty.ptr<private_>(Outer));
    fn_4->SetParams({fn_4_p});
    b.Append(fn_4->Block(), [&] {
        auto* access_0 = b.Access(ty.ptr<private_>(ty.array(Inner, 4)), fn_4_p, 0_u);
        auto* access_1 = b.Access(ty.ptr<private_, mat3x4<f32>>(), P, 1_u);
        b.Return(fn_4, b.Call(ty.f32(), fn_3, access_0, access_1));
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        b.Call(ty.f32(), fn_4, P);
        b.Return(fn_b);
    });

    auto* src = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

%b1 = block {  # root
  %P:ptr<private, Outer, read_write> = var
}

%f0 = func(%p:ptr<private, vec4<f32>, read_write>):f32 -> %b2 {
  %b2 = block {
    %4:f32 = load_vector_element %p, 0u
    ret %4
  }
}
%f1 = func(%p_1:ptr<private, mat3x4<f32>, read_write>):f32 -> %b3 {  # %p_1: 'p'
  %b3 = block {
    %res:ptr<function, f32, read_write> = var
    %8:ptr<private, vec4<f32>, read_write> = access %p_1, 1i
    %9:f32 = call %f0, %8
    %10:f32 = load %res
    %11:f32 = add %10, %9
    store %res, %11
    %p_vec:ptr<private, vec4<f32>, read_write> = access %p_1, 1i
    %13:f32 = call %f0, %p_vec
    %14:f32 = load %res
    %15:f32 = add %14, %13
    store %res, %15
    %16:ptr<private, vec4<f32>, read_write> = access %P, 0u, 2i, 0u, 1i
    %17:f32 = call %f0, %16
    %18:f32 = load %res
    %19:f32 = add %18, %17
    store %res, %19
    %p_vec_1:ptr<private, vec4<f32>, read_write> = access %P, 0u, 2i, 0u, 1i  # %p_vec_1: 'p_vec'
    %21:f32 = call %f0, %p_vec_1
    %22:f32 = load %res
    %23:f32 = add %22, %21
    store %res, %23
    %24:f32 = load %res
    ret %24
  }
}
%f2 = func(%p_2:ptr<private, Inner, read_write>):f32 -> %b4 {  # %p_2: 'p'
  %b4 = block {
    %p_mat:ptr<private, mat3x4<f32>, read_write> = access %p_2, 0u
    %28:f32 = call %f1, %p_mat
    ret %28
  }
}
%f3 = func(%p0:ptr<private, array<Inner, 4>, read_write>, %p1:ptr<private, mat3x4<f32>, read_write>):f32 -> %b5 {
  %b5 = block {
    %p0_inner:ptr<private, Inner, read_write> = access %p0, 3i
    %33:f32 = call %f2, %p0_inner
    %34:f32 = call %f1, %p1
    %35:f32 = add %33, %34
    ret %35
  }
}
%f4 = func(%p_3:ptr<private, Outer, read_write>):f32 -> %b6 {  # %p_3: 'p'
  %b6 = block {
    %38:ptr<private, array<Inner, 4>, read_write> = access %p_3, 0u
    %39:ptr<private, mat3x4<f32>, read_write> = access %P, 1u
    %40:f32 = call %f3, %38, %39
    ret %40
  }
}
%b = func():void -> %b7 {
  %b7 = block {
    %42:f32 = call %f4, %P
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

%b1 = block {  # root
  %P:ptr<private, Outer, read_write> = var
}

%f0_P_mat_X = func(%p_indices:array<u32, 1>):f32 -> %b2 {
  %b2 = block {
    %4:u32 = access %p_indices, 0u
    %5:ptr<private, vec4<f32>, read_write> = access %P, 1u, %4
    %6:f32 = load_vector_element %5, 0u
    ret %6
  }
}
%f0_P_arr_X_mat_X = func(%p_indices_1:array<u32, 2>):f32 -> %b3 {  # %p_indices_1: 'p_indices'
  %b3 = block {
    %9:u32 = access %p_indices_1, 0u
    %10:u32 = access %p_indices_1, 1u
    %11:ptr<private, vec4<f32>, read_write> = access %P, 0u, %9, 0u, %10
    %12:f32 = load_vector_element %11, 0u
    ret %12
  }
}
%f1_P_mat = func():f32 -> %b4 {
  %b4 = block {
    %res:ptr<function, f32, read_write> = var
    %15:u32 = convert 1i
    %16:array<u32, 1> = construct %15
    %17:f32 = call %f0_P_mat_X, %16
    %18:f32 = load %res
    %19:f32 = add %18, %17
    store %res, %19
    %20:u32 = convert 1i
    %21:array<u32, 1> = construct %20
    %22:f32 = call %f0_P_mat_X, %21
    %23:f32 = load %res
    %24:f32 = add %23, %22
    store %res, %24
    %25:u32 = convert 2i
    %26:u32 = convert 1i
    %27:array<u32, 2> = construct %25, %26
    %28:f32 = call %f0_P_arr_X_mat_X, %27
    %29:f32 = load %res
    %30:f32 = add %29, %28
    store %res, %30
    %31:u32 = convert 2i
    %32:u32 = convert 1i
    %33:array<u32, 2> = construct %31, %32
    %34:f32 = call %f0_P_arr_X_mat_X, %33
    %35:f32 = load %res
    %36:f32 = add %35, %34
    store %res, %36
    %37:f32 = load %res
    ret %37
  }
}
%f1_P_arr_X_mat = func(%p_indices_2:array<u32, 1>):f32 -> %b5 {  # %p_indices_2: 'p_indices'
  %b5 = block {
    %40:u32 = access %p_indices_2, 0u
    %res_1:ptr<function, f32, read_write> = var  # %res_1: 'res'
    %42:u32 = convert 1i
    %43:array<u32, 2> = construct %40, %42
    %44:f32 = call %f0_P_arr_X_mat_X, %43
    %45:f32 = load %res_1
    %46:f32 = add %45, %44
    store %res_1, %46
    %47:u32 = convert 1i
    %48:array<u32, 2> = construct %40, %47
    %49:f32 = call %f0_P_arr_X_mat_X, %48
    %50:f32 = load %res_1
    %51:f32 = add %50, %49
    store %res_1, %51
    %52:u32 = convert 2i
    %53:u32 = convert 1i
    %54:array<u32, 2> = construct %52, %53
    %55:f32 = call %f0_P_arr_X_mat_X, %54
    %56:f32 = load %res_1
    %57:f32 = add %56, %55
    store %res_1, %57
    %58:u32 = convert 2i
    %59:u32 = convert 1i
    %60:array<u32, 2> = construct %58, %59
    %61:f32 = call %f0_P_arr_X_mat_X, %60
    %62:f32 = load %res_1
    %63:f32 = add %62, %61
    store %res_1, %63
    %64:f32 = load %res_1
    ret %64
  }
}
%f2_P_arr_X = func(%p_indices_3:array<u32, 1>):f32 -> %b6 {  # %p_indices_3: 'p_indices'
  %b6 = block {
    %67:u32 = access %p_indices_3, 0u
    %68:array<u32, 1> = construct %67
    %69:f32 = call %f1_P_arr_X_mat, %68
    ret %69
  }
}
%f3_P_arr_P_mat = func():f32 -> %b7 {
  %b7 = block {
    %71:u32 = convert 3i
    %72:array<u32, 1> = construct %71
    %73:f32 = call %f2_P_arr_X, %72
    %74:f32 = call %f1_P_mat
    %75:f32 = add %73, %74
    ret %75
  }
}
%f4_P = func():f32 -> %b8 {
  %b8 = block {
    %77:f32 = call %f3_P_arr_P_mat
    ret %77
  }
}
%b = func():void -> %b9 {
  %b9 = block {
    %79:f32 = call %f4_P
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformPrivate);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Disabled_CallChaining) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    auto* Outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.Register("arr"), ty.array(Inner, 4)},
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    Var* P = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 P = b.Var("P", ty.ptr<private_>(Outer));
             });

    auto* fn_0 = b.Function("f0", ty.f32());
    auto* fn_0_p = b.FunctionParam("p", ty.ptr<private_, vec4<f32>>());
    fn_0->SetParams({fn_0_p});
    b.Append(fn_0->Block(), [&] { b.Return(fn_0, b.LoadVectorElement(fn_0_p, 0_u)); });

    auto* fn_1 = b.Function("f1", ty.f32());
    auto* fn_1_p = b.FunctionParam("p", ty.ptr<private_, mat3x4<f32>>());
    fn_1->SetParams({fn_1_p});
    b.Append(fn_1->Block(), [&] {
        auto* res = b.Var<function, f32>("res");
        {
            // res += f0(&(*p)[1]);
            auto* call_0 = b.Call(fn_0, b.Access(ty.ptr<private_, vec4<f32>>(), fn_1_p, 1_i));
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &(*p)[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<private_, vec4<f32>>(), fn_1_p, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // res += f0(&U.arr[2].mat[1]);
            auto* access = b.Access(ty.ptr<private_, vec4<f32>>(), P, 0_u, 2_i, 0_u, 1_i);
            auto* call_0 = b.Call(fn_0, access);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &U.arr[2].mat[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<private_, vec4<f32>>(), P, 0_u, 2_i, 0_u, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }

        b.Return(fn_1, b.Load(res));
    });

    auto* fn_2 = b.Function("f2", ty.f32());
    auto* fn_2_p = b.FunctionParam("p", ty.ptr<private_>(Inner));
    fn_2->SetParams({fn_2_p});
    b.Append(fn_2->Block(), [&] {
        auto* p_mat = b.Access(ty.ptr<private_, mat3x4<f32>>(), fn_2_p, 0_u);
        b.ir.SetName(p_mat, "p_mat");
        b.Return(fn_2, b.Call(fn_1, p_mat));
    });

    auto* fn_3 = b.Function("f3", ty.f32());
    auto* fn_3_p0 = b.FunctionParam("p0", ty.ptr<private_>(ty.array(Inner, 4)));
    auto* fn_3_p1 = b.FunctionParam("p1", ty.ptr<private_, mat3x4<f32>>());
    fn_3->SetParams({fn_3_p0, fn_3_p1});
    b.Append(fn_3->Block(), [&] {
        auto* p0_inner = b.Access(ty.ptr<private_>(Inner), fn_3_p0, 3_i);
        b.ir.SetName(p0_inner, "p0_inner");
        auto* call_0 = b.Call(ty.f32(), fn_2, p0_inner);
        auto* call_1 = b.Call(ty.f32(), fn_1, fn_3_p1);
        b.Return(fn_3, b.Add(ty.f32(), call_0, call_1));
    });

    auto* fn_4 = b.Function("f4", ty.f32());
    auto* fn_4_p = b.FunctionParam("p", ty.ptr<private_>(Outer));
    fn_4->SetParams({fn_4_p});
    b.Append(fn_4->Block(), [&] {
        auto* access_0 = b.Access(ty.ptr<private_>(ty.array(Inner, 4)), fn_4_p, 0_u);
        auto* access_1 = b.Access(ty.ptr<private_, mat3x4<f32>>(), P, 1_u);
        b.Return(fn_4, b.Call(ty.f32(), fn_3, access_0, access_1));
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        b.Call(ty.f32(), fn_4, P);
        b.Return(fn_b);
    });

    auto* src = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

%b1 = block {  # root
  %P:ptr<private, Outer, read_write> = var
}

%f0 = func(%p:ptr<private, vec4<f32>, read_write>):f32 -> %b2 {
  %b2 = block {
    %4:f32 = load_vector_element %p, 0u
    ret %4
  }
}
%f1 = func(%p_1:ptr<private, mat3x4<f32>, read_write>):f32 -> %b3 {  # %p_1: 'p'
  %b3 = block {
    %res:ptr<function, f32, read_write> = var
    %8:ptr<private, vec4<f32>, read_write> = access %p_1, 1i
    %9:f32 = call %f0, %8
    %10:f32 = load %res
    %11:f32 = add %10, %9
    store %res, %11
    %p_vec:ptr<private, vec4<f32>, read_write> = access %p_1, 1i
    %13:f32 = call %f0, %p_vec
    %14:f32 = load %res
    %15:f32 = add %14, %13
    store %res, %15
    %16:ptr<private, vec4<f32>, read_write> = access %P, 0u, 2i, 0u, 1i
    %17:f32 = call %f0, %16
    %18:f32 = load %res
    %19:f32 = add %18, %17
    store %res, %19
    %p_vec_1:ptr<private, vec4<f32>, read_write> = access %P, 0u, 2i, 0u, 1i  # %p_vec_1: 'p_vec'
    %21:f32 = call %f0, %p_vec_1
    %22:f32 = load %res
    %23:f32 = add %22, %21
    store %res, %23
    %24:f32 = load %res
    ret %24
  }
}
%f2 = func(%p_2:ptr<private, Inner, read_write>):f32 -> %b4 {  # %p_2: 'p'
  %b4 = block {
    %p_mat:ptr<private, mat3x4<f32>, read_write> = access %p_2, 0u
    %28:f32 = call %f1, %p_mat
    ret %28
  }
}
%f3 = func(%p0:ptr<private, array<Inner, 4>, read_write>, %p1:ptr<private, mat3x4<f32>, read_write>):f32 -> %b5 {
  %b5 = block {
    %p0_inner:ptr<private, Inner, read_write> = access %p0, 3i
    %33:f32 = call %f2, %p0_inner
    %34:f32 = call %f1, %p1
    %35:f32 = add %33, %34
    ret %35
  }
}
%f4 = func(%p_3:ptr<private, Outer, read_write>):f32 -> %b6 {  # %p_3: 'p'
  %b6 = block {
    %38:ptr<private, array<Inner, 4>, read_write> = access %p_3, 0u
    %39:ptr<private, mat3x4<f32>, read_write> = access %P, 1u
    %40:f32 = call %f3, %38, %39
    ret %40
  }
}
%b = func():void -> %b7 {
  %b7 = block {
    %42:f32 = call %f4, %P
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

}  // namespace private_as_tests

////////////////////////////////////////////////////////////////////////////////
// 'function' address space
////////////////////////////////////////////////////////////////////////////////
namespace function_as_tests {

using IR_DirectVariableAccessTest_FunctionAS = TransformTest;

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Enabled_LocalPtr) {
    auto* fn = b.Function("f", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* v = b.Var<function, i32>("v");
        auto* p = b.Let("p", v);
        b.Var<function>("x", b.Load(p));
        b.Return(fn);
    });

    auto* src = R"(
%f = func():void -> %b1 {
  %b1 = block {
    %v:ptr<function, i32, read_write> = var
    %p:ptr<function, i32, read_write> = let %v
    %4:i32 = load %p
    %x:ptr<function, i32, read_write> = var, %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;  // Nothing changes

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Enabled_Param_ptr_i32_read) {
    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<function, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* F = b.Var<function, i32>("F");
        b.Call(fn_a, 10_i, F, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
%a = func(%pre:i32, %p:ptr<function, i32, read_write>, %post:i32):i32 -> %b1 {
  %b1 = block {
    %5:i32 = load %p
    ret %5
  }
}
%b = func():void -> %b2 {
  %b2 = block {
    %F:ptr<function, i32, read_write> = var
    %8:i32 = call %a, 10i, %F, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a_P = func(%pre:i32, %p_root:ptr<function, i32, read_write>, %post:i32):i32 -> %b1 {
  %b1 = block {
    %5:ptr<function, i32, read_write> = access %p_root
    %6:i32 = load %5
    ret %6
  }
}
%b = func():void -> %b2 {
  %b2 = block {
    %F:ptr<function, i32, read_write> = var
    %9:i32 = call %a_P, 10i, %F, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformFunction);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Enabled_Param_ptr_i32_write) {
    auto* fn_a = b.Function("a", ty.void_());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<function, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] {
        b.Store(fn_a_p, 42_i);
        b.Return(fn_a);
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* F = b.Var<function, i32>("F");
        b.Call(fn_a, 10_i, F, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
%a = func(%pre:i32, %p:ptr<function, i32, read_write>, %post:i32):void -> %b1 {
  %b1 = block {
    store %p, 42i
    ret
  }
}
%b = func():void -> %b2 {
  %b2 = block {
    %F:ptr<function, i32, read_write> = var
    %7:void = call %a, 10i, %F, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a_P = func(%pre:i32, %p_root:ptr<function, i32, read_write>, %post:i32):void -> %b1 {
  %b1 = block {
    %5:ptr<function, i32, read_write> = access %p_root
    store %5, 42i
    ret
  }
}
%b = func():void -> %b2 {
  %b2 = block {
    %F:ptr<function, i32, read_write> = var
    %8:void = call %a_P, 10i, %F, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformFunction);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Enabled_Param_ptr_i32_Via_struct_read) {
    auto* str_ = ty.Struct(mod.symbols.New("str"), {
                                                       {mod.symbols.Register("i"), ty.i32()},
                                                   });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<function, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* F = b.Var("F", ty.ptr<function>(str_));
        auto* access = b.Access(ty.ptr<function, i32>(), F, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

%a = func(%pre:i32, %p:ptr<function, i32, read_write>, %post:i32):i32 -> %b1 {
  %b1 = block {
    %5:i32 = load %p
    ret %5
  }
}
%b = func():void -> %b2 {
  %b2 = block {
    %F:ptr<function, str, read_write> = var
    %8:ptr<function, i32, read_write> = access %F, 0u
    %9:i32 = call %a, 10i, %8, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

%a_P_i = func(%pre:i32, %p_root:ptr<function, str, read_write>, %post:i32):i32 -> %b1 {
  %b1 = block {
    %5:ptr<function, i32, read_write> = access %p_root, 0u
    %6:i32 = load %5
    ret %6
  }
}
%b = func():void -> %b2 {
  %b2 = block {
    %F:ptr<function, str, read_write> = var
    %9:i32 = call %a_P_i, 10i, %F, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformFunction);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Enabled_Param_ptr_arr_i32_Via_struct_write) {
    auto* str_ =
        ty.Struct(mod.symbols.New("str"), {
                                              {mod.symbols.Register("arr"), ty.array<i32, 4>()},
                                          });

    auto* fn_a = b.Function("a", ty.void_());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<function, array<i32, 4>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] {
        b.Store(fn_a_p, b.Splat(ty.array<i32, 4>(), 0_i, 4));
        b.Return(fn_a);
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* F = b.Var("F", ty.ptr<function>(str_));
        auto* access = b.Access(ty.ptr<function, array<i32, 4>>(), F, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  arr:array<i32, 4> @offset(0)
}

%a = func(%pre:i32, %p:ptr<function, array<i32, 4>, read_write>, %post:i32):void -> %b1 {
  %b1 = block {
    store %p, array<i32, 4>(0i)
    ret
  }
}
%b = func():void -> %b2 {
  %b2 = block {
    %F:ptr<function, str, read_write> = var
    %7:ptr<function, array<i32, 4>, read_write> = access %F, 0u
    %8:void = call %a, 10i, %7, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
str = struct @align(4) {
  arr:array<i32, 4> @offset(0)
}

%a_P_arr = func(%pre:i32, %p_root:ptr<function, str, read_write>, %post:i32):void -> %b1 {
  %b1 = block {
    %5:ptr<function, array<i32, 4>, read_write> = access %p_root, 0u
    store %5, array<i32, 4>(0i)
    ret
  }
}
%b = func():void -> %b2 {
  %b2 = block {
    %F:ptr<function, str, read_write> = var
    %8:void = call %a_P_arr, 10i, %F, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformFunction);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Enabled_Param_ptr_i32_mixed) {
    auto* str_ = ty.Struct(mod.symbols.New("str"), {
                                                       {mod.symbols.Register("i"), ty.i32()},
                                                   });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<function, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* Fi = b.Var("Fi", ty.ptr<function, i32>());
        auto* Fs = b.Var("Fs", ty.ptr<function>(str_));
        auto* Fa = b.Var("Fa", ty.ptr<function, array<i32, 4>>());
        {  // a(10, &Fi, 20);
            b.Call(fn_a, 10_i, Fi, 20_i);
        }
        {  // a(30, &Fs.i, 40);
            auto* access = b.Access(ty.ptr<function, i32>(), Fs, 0_u);
            b.Call(fn_a, 30_i, access, 40_i);
        }
        {  // a(50, &Fa[2], 60);
            auto* access = b.Access(ty.ptr<function, i32>(), Fa, 2_i);
            b.Call(fn_a, 50_i, access, 60_i);
        }
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

%a = func(%pre:i32, %p:ptr<function, i32, read_write>, %post:i32):i32 -> %b1 {
  %b1 = block {
    %5:i32 = load %p
    ret %5
  }
}
%b = func():void -> %b2 {
  %b2 = block {
    %Fi:ptr<function, i32, read_write> = var
    %Fs:ptr<function, str, read_write> = var
    %Fa:ptr<function, array<i32, 4>, read_write> = var
    %10:i32 = call %a, 10i, %Fi, 20i
    %11:ptr<function, i32, read_write> = access %Fs, 0u
    %12:i32 = call %a, 30i, %11, 40i
    %13:ptr<function, i32, read_write> = access %Fa, 2i
    %14:i32 = call %a, 50i, %13, 60i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

%a_P = func(%pre:i32, %p_root:ptr<function, i32, read_write>, %post:i32):i32 -> %b1 {
  %b1 = block {
    %5:ptr<function, i32, read_write> = access %p_root
    %6:i32 = load %5
    ret %6
  }
}
%a_P_i = func(%pre_1:i32, %p_root_1:ptr<function, str, read_write>, %post_1:i32):i32 -> %b2 {  # %pre_1: 'pre', %p_root_1: 'p_root', %post_1: 'post'
  %b2 = block {
    %11:ptr<function, i32, read_write> = access %p_root_1, 0u
    %12:i32 = load %11
    ret %12
  }
}
%a_P_X = func(%pre_2:i32, %p_root_2:ptr<function, array<i32, 4>, read_write>, %p_indices:array<u32, 1>, %post_2:i32):i32 -> %b3 {  # %pre_2: 'pre', %p_root_2: 'p_root', %post_2: 'post'
  %b3 = block {
    %18:u32 = access %p_indices, 0u
    %19:ptr<function, i32, read_write> = access %p_root_2, %18
    %20:i32 = load %19
    ret %20
  }
}
%b = func():void -> %b4 {
  %b4 = block {
    %Fi:ptr<function, i32, read_write> = var
    %Fs:ptr<function, str, read_write> = var
    %Fa:ptr<function, array<i32, 4>, read_write> = var
    %25:i32 = call %a_P, 10i, %Fi, 20i
    %26:i32 = call %a_P_i, 30i, %Fs, 40i
    %27:u32 = convert 2i
    %28:array<u32, 1> = construct %27
    %29:i32 = call %a_P_X, 50i, %Fa, %28, 60i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformFunction);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Disabled_Param_ptr_i32_Via_struct_read) {
    auto* str_ = ty.Struct(mod.symbols.New("str"), {
                                                       {mod.symbols.Register("i"), ty.i32()},
                                                   });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<function, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* F = b.Var("F", ty.ptr<function>(str_));
        auto* access = b.Access(ty.ptr<function, i32>(), F, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

%a = func(%pre:i32, %p:ptr<function, i32, read_write>, %post:i32):i32 -> %b1 {
  %b1 = block {
    %5:i32 = load %p
    ret %5
  }
}
%b = func():void -> %b2 {
  %b2 = block {
    %F:ptr<function, str, read_write> = var
    %8:ptr<function, i32, read_write> = access %F, 0u
    %9:i32 = call %a, 10i, %8, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Disabled_Param_ptr_arr_i32_Via_struct_write) {
    auto* str_ =
        ty.Struct(mod.symbols.New("str"), {
                                              {mod.symbols.Register("arr"), ty.array<i32, 4>()},
                                          });

    auto* fn_a = b.Function("a", ty.void_());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<function, array<i32, 4>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] {
        b.Store(fn_a_p, b.Splat(ty.array<i32, 4>(), 0_i, 4));
        b.Return(fn_a);
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* F = b.Var("F", ty.ptr<function>(str_));
        auto* access = b.Access(ty.ptr<function, array<i32, 4>>(), F, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  arr:array<i32, 4> @offset(0)
}

%a = func(%pre:i32, %p:ptr<function, array<i32, 4>, read_write>, %post:i32):void -> %b1 {
  %b1 = block {
    store %p, array<i32, 4>(0i)
    ret
  }
}
%b = func():void -> %b2 {
  %b2 = block {
    %F:ptr<function, str, read_write> = var
    %7:ptr<function, array<i32, 4>, read_write> = access %F, 0u
    %8:void = call %a, 10i, %7, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

}  // namespace function_as_tests

////////////////////////////////////////////////////////////////////////////////
// builtin function calls
////////////////////////////////////////////////////////////////////////////////
namespace builtin_fn_calls {

using IR_DirectVariableAccessTest_BuiltinFn = TransformTest;

TEST_F(IR_DirectVariableAccessTest_BuiltinFn, ArrayLength) {
    Var* S = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 S = b.Var<storage, array<f32>>("S");
                 S->SetBindingPoint(0, 0);
             });

    auto* fn_len = b.Function("len", ty.u32());
    auto* fn_len_p = b.FunctionParam("p", ty.ptr<storage, array<f32>>());
    fn_len->SetParams({fn_len_p});
    b.Append(fn_len->Block(),
             [&] {  //
                 b.Return(fn_len, b.Call(ty.u32(), core::BuiltinFn::kArrayLength, fn_len_p));
             });

    auto* fn_f = b.Function("b", ty.void_());
    b.Append(fn_f->Block(), [&] {
        b.Call(fn_len, S);
        b.Return(fn_f);
    });

    auto* src = R"(
%b1 = block {  # root
  %S:ptr<storage, array<f32>, read_write> = var @binding_point(0, 0)
}

%len = func(%p:ptr<storage, array<f32>, read_write>):u32 -> %b2 {
  %b2 = block {
    %4:u32 = arrayLength %p
    ret %4
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %6:u32 = call %len, %S
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %S:ptr<storage, array<f32>, read_write> = var @binding_point(0, 0)
}

%len_S = func():u32 -> %b2 {
  %b2 = block {
    %3:ptr<storage, array<f32>, read_write> = access %S
    %4:u32 = arrayLength %3
    ret %4
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %6:u32 = call %len_S
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_BuiltinFn, AtomicLoad) {
    Var* W = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 W = b.Var("W", ty.ptr<workgroup>(ty.atomic<i32>()));
             });

    auto* fn_load = b.Function("load", ty.i32());
    auto* fn_load_p = b.FunctionParam("p", ty.ptr<workgroup>(ty.atomic<i32>()));
    fn_load->SetParams({fn_load_p});
    b.Append(fn_load->Block(),
             [&] {  //
                 b.Return(fn_load, b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad, fn_load_p));
             });

    auto* fn_f = b.Function("b", ty.void_());
    b.Append(fn_f->Block(), [&] {
        b.Call(fn_load, W);
        b.Return(fn_f);
    });

    auto* src = R"(
%b1 = block {  # root
  %W:ptr<workgroup, atomic<i32>, read_write> = var
}

%load = func(%p:ptr<workgroup, atomic<i32>, read_write>):i32 -> %b2 {
  %b2 = block {
    %4:i32 = atomicLoad %p
    ret %4
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %6:i32 = call %load, %W
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %W:ptr<workgroup, atomic<i32>, read_write> = var
}

%load_W = func():i32 -> %b2 {
  %b2 = block {
    %3:ptr<workgroup, atomic<i32>, read_write> = access %W
    %4:i32 = atomicLoad %3
    ret %4
  }
}
%b = func():void -> %b3 {
  %b3 = block {
    %6:i32 = call %load_W
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

}  // namespace builtin_fn_calls

////////////////////////////////////////////////////////////////////////////////
// complex tests
////////////////////////////////////////////////////////////////////////////////
namespace complex_tests {

using IR_DirectVariableAccessTest_Complex = TransformTest;

TEST_F(IR_DirectVariableAccessTest_Complex, Param_ptr_mixed_vec4i32_ViaMultiple) {
    auto* str_ = ty.Struct(mod.symbols.New("str"), {
                                                       {mod.symbols.Register("i"), ty.vec4<i32>()},
                                                   });

    Var* U = nullptr;
    Var* U_str = nullptr;
    Var* U_arr = nullptr;
    Var* U_arr_arr = nullptr;
    Var* S = nullptr;
    Var* S_str = nullptr;
    Var* S_arr = nullptr;
    Var* S_arr_arr = nullptr;
    Var* W = nullptr;
    Var* W_str = nullptr;
    Var* W_arr = nullptr;
    Var* W_arr_arr = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 U = b.Var<uniform, vec4<i32>, read>("U");
                 U->SetBindingPoint(0, 0);
                 U_str = b.Var("U_str", ty.ptr<uniform, read>(str_));
                 U_str->SetBindingPoint(0, 1);
                 U_arr = b.Var<uniform, array<vec4<i32>, 8>, read>("U_arr");
                 U_arr->SetBindingPoint(0, 2);
                 U_arr_arr = b.Var<uniform, array<array<vec4<i32>, 8>, 4>, read>("U_arr_arr");
                 U_arr_arr->SetBindingPoint(0, 3);

                 S = b.Var<storage, vec4<i32>, read>("S");
                 S->SetBindingPoint(1, 0);
                 S_str = b.Var("S_str", ty.ptr<storage, read>(str_));
                 S_str->SetBindingPoint(1, 1);
                 S_arr = b.Var<storage, array<vec4<i32>, 8>, read>("S_arr");
                 S_arr->SetBindingPoint(1, 2);
                 S_arr_arr = b.Var<storage, array<array<vec4<i32>, 8>, 4>, read>("S_arr_arr");
                 S_arr_arr->SetBindingPoint(1, 3);

                 W = b.Var<workgroup, vec4<i32>>("W");
                 W_str = b.Var("W_str", ty.ptr<workgroup>(str_));
                 W_arr = b.Var<workgroup, array<vec4<i32>, 8>>("W_arr");
                 W_arr_arr = b.Var<workgroup, array<array<vec4<i32>, 8>, 4>>("W_arr_arr");
             });

    auto* fn_u = b.Function("fn_u", ty.vec4<i32>());
    auto* fn_u_p = b.FunctionParam("p", ty.ptr<uniform, vec4<i32>, read>());
    fn_u->SetParams({fn_u_p});
    b.Append(fn_u->Block(), [&] { b.Return(fn_u, b.Load(fn_u_p)); });

    auto* fn_s = b.Function("fn_s", ty.vec4<i32>());
    auto* fn_s_p = b.FunctionParam("p", ty.ptr<storage, vec4<i32>, read>());
    fn_s->SetParams({fn_s_p});
    b.Append(fn_s->Block(), [&] { b.Return(fn_s, b.Load(fn_s_p)); });

    auto* fn_w = b.Function("fn_w", ty.vec4<i32>());
    auto* fn_w_p = b.FunctionParam("p", ty.ptr<workgroup, vec4<i32>>());
    fn_w->SetParams({fn_w_p});
    b.Append(fn_w->Block(), [&] { b.Return(fn_w, b.Load(fn_w_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* I = b.Let("I", 3_i);
        auto* J = b.Let("J", 4_i);

        auto* u = b.Call(fn_u, U);
        b.ir.SetName(u, "u");
        auto* u_str = b.Call(fn_u, b.Access(ty.ptr<uniform, vec4<i32>, read>(), U_str, 0_u));
        b.ir.SetName(u_str, "u_str");
        auto* u_arr0 = b.Call(fn_u, b.Access(ty.ptr<uniform, vec4<i32>, read>(), U_arr, 0_i));
        b.ir.SetName(u_arr0, "u_arr0");
        auto* u_arr1 = b.Call(fn_u, b.Access(ty.ptr<uniform, vec4<i32>, read>(), U_arr, 1_i));
        b.ir.SetName(u_arr1, "u_arr1");
        auto* u_arrI = b.Call(fn_u, b.Access(ty.ptr<uniform, vec4<i32>, read>(), U_arr, I));
        b.ir.SetName(u_arrI, "u_arrI");
        auto* u_arr1_arr0 =
            b.Call(fn_u, b.Access(ty.ptr<uniform, vec4<i32>, read>(), U_arr_arr, 1_i, 0_i));
        b.ir.SetName(u_arr1_arr0, "u_arr1_arr0");
        auto* u_arr2_arrI =
            b.Call(fn_u, b.Access(ty.ptr<uniform, vec4<i32>, read>(), U_arr_arr, 2_i, I));
        b.ir.SetName(u_arr2_arrI, "u_arr2_arrI");
        auto* u_arrI_arr2 =
            b.Call(fn_u, b.Access(ty.ptr<uniform, vec4<i32>, read>(), U_arr_arr, I, 2_i));
        b.ir.SetName(u_arrI_arr2, "u_arrI_arr2");
        auto* u_arrI_arrJ =
            b.Call(fn_u, b.Access(ty.ptr<uniform, vec4<i32>, read>(), U_arr_arr, I, J));
        b.ir.SetName(u_arrI_arrJ, "u_arrI_arrJ");

        auto* s = b.Call(fn_s, S);
        b.ir.SetName(s, "s");
        auto* s_str = b.Call(fn_s, b.Access(ty.ptr<storage, vec4<i32>, read>(), S_str, 0_u));
        b.ir.SetName(s_str, "s_str");
        auto* s_arr0 = b.Call(fn_s, b.Access(ty.ptr<storage, vec4<i32>, read>(), S_arr, 0_i));
        b.ir.SetName(s_arr0, "s_arr0");
        auto* s_arr1 = b.Call(fn_s, b.Access(ty.ptr<storage, vec4<i32>, read>(), S_arr, 1_i));
        b.ir.SetName(s_arr1, "s_arr1");
        auto* s_arrI = b.Call(fn_s, b.Access(ty.ptr<storage, vec4<i32>, read>(), S_arr, I));
        b.ir.SetName(s_arrI, "s_arrI");
        auto* s_arr1_arr0 =
            b.Call(fn_s, b.Access(ty.ptr<storage, vec4<i32>, read>(), S_arr_arr, 1_i, 0_i));
        b.ir.SetName(s_arr1_arr0, "s_arr1_arr0");
        auto* s_arr2_arrI =
            b.Call(fn_s, b.Access(ty.ptr<storage, vec4<i32>, read>(), S_arr_arr, 2_i, I));
        b.ir.SetName(s_arr2_arrI, "s_arr2_arrI");
        auto* s_arrI_arr2 =
            b.Call(fn_s, b.Access(ty.ptr<storage, vec4<i32>, read>(), S_arr_arr, I, 2_i));
        b.ir.SetName(s_arrI_arr2, "s_arrI_arr2");
        auto* s_arrI_arrJ =
            b.Call(fn_s, b.Access(ty.ptr<storage, vec4<i32>, read>(), S_arr_arr, I, J));
        b.ir.SetName(s_arrI_arrJ, "s_arrI_arrJ");

        auto* w = b.Call(fn_w, W);
        b.ir.SetName(w, "w");
        auto* w_str = b.Call(fn_w, b.Access(ty.ptr<workgroup, vec4<i32>>(), W_str, 0_u));
        b.ir.SetName(w_str, "w_str");
        auto* w_arr0 = b.Call(fn_w, b.Access(ty.ptr<workgroup, vec4<i32>>(), W_arr, 0_i));
        b.ir.SetName(w_arr0, "w_arr0");
        auto* w_arr1 = b.Call(fn_w, b.Access(ty.ptr<workgroup, vec4<i32>>(), W_arr, 1_i));
        b.ir.SetName(w_arr1, "w_arr1");
        auto* w_arrI = b.Call(fn_w, b.Access(ty.ptr<workgroup, vec4<i32>>(), W_arr, I));
        b.ir.SetName(w_arrI, "w_arrI");
        auto* w_arr1_arr0 =
            b.Call(fn_w, b.Access(ty.ptr<workgroup, vec4<i32>>(), W_arr_arr, 1_i, 0_i));
        b.ir.SetName(w_arr1_arr0, "w_arr1_arr0");
        auto* w_arr2_arrI =
            b.Call(fn_w, b.Access(ty.ptr<workgroup, vec4<i32>>(), W_arr_arr, 2_i, I));
        b.ir.SetName(w_arr2_arrI, "w_arr2_arrI");
        auto* w_arrI_arr2 =
            b.Call(fn_w, b.Access(ty.ptr<workgroup, vec4<i32>>(), W_arr_arr, I, 2_i));
        b.ir.SetName(w_arrI_arr2, "w_arrI_arr2");
        auto* w_arrI_arrJ = b.Call(fn_w, b.Access(ty.ptr<workgroup, vec4<i32>>(), W_arr_arr, I, J));
        b.ir.SetName(w_arrI_arrJ, "w_arrI_arrJ");

        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(16) {
  i:vec4<i32> @offset(0)
}

%b1 = block {  # root
  %U:ptr<uniform, vec4<i32>, read> = var @binding_point(0, 0)
  %U_str:ptr<uniform, str, read> = var @binding_point(0, 1)
  %U_arr:ptr<uniform, array<vec4<i32>, 8>, read> = var @binding_point(0, 2)
  %U_arr_arr:ptr<uniform, array<array<vec4<i32>, 8>, 4>, read> = var @binding_point(0, 3)
  %S:ptr<storage, vec4<i32>, read> = var @binding_point(1, 0)
  %S_str:ptr<storage, str, read> = var @binding_point(1, 1)
  %S_arr:ptr<storage, array<vec4<i32>, 8>, read> = var @binding_point(1, 2)
  %S_arr_arr:ptr<storage, array<array<vec4<i32>, 8>, 4>, read> = var @binding_point(1, 3)
  %W:ptr<workgroup, vec4<i32>, read_write> = var
  %W_str:ptr<workgroup, str, read_write> = var
  %W_arr:ptr<workgroup, array<vec4<i32>, 8>, read_write> = var
  %W_arr_arr:ptr<workgroup, array<array<vec4<i32>, 8>, 4>, read_write> = var
}

%fn_u = func(%p:ptr<uniform, vec4<i32>, read>):vec4<i32> -> %b2 {
  %b2 = block {
    %15:vec4<i32> = load %p
    ret %15
  }
}
%fn_s = func(%p_1:ptr<storage, vec4<i32>, read>):vec4<i32> -> %b3 {  # %p_1: 'p'
  %b3 = block {
    %18:vec4<i32> = load %p_1
    ret %18
  }
}
%fn_w = func(%p_2:ptr<workgroup, vec4<i32>, read_write>):vec4<i32> -> %b4 {  # %p_2: 'p'
  %b4 = block {
    %21:vec4<i32> = load %p_2
    ret %21
  }
}
%b = func():void -> %b5 {
  %b5 = block {
    %I:i32 = let 3i
    %J:i32 = let 4i
    %u:vec4<i32> = call %fn_u, %U
    %26:ptr<uniform, vec4<i32>, read> = access %U_str, 0u
    %u_str:vec4<i32> = call %fn_u, %26
    %28:ptr<uniform, vec4<i32>, read> = access %U_arr, 0i
    %u_arr0:vec4<i32> = call %fn_u, %28
    %30:ptr<uniform, vec4<i32>, read> = access %U_arr, 1i
    %u_arr1:vec4<i32> = call %fn_u, %30
    %32:ptr<uniform, vec4<i32>, read> = access %U_arr, %I
    %u_arrI:vec4<i32> = call %fn_u, %32
    %34:ptr<uniform, vec4<i32>, read> = access %U_arr_arr, 1i, 0i
    %u_arr1_arr0:vec4<i32> = call %fn_u, %34
    %36:ptr<uniform, vec4<i32>, read> = access %U_arr_arr, 2i, %I
    %u_arr2_arrI:vec4<i32> = call %fn_u, %36
    %38:ptr<uniform, vec4<i32>, read> = access %U_arr_arr, %I, 2i
    %u_arrI_arr2:vec4<i32> = call %fn_u, %38
    %40:ptr<uniform, vec4<i32>, read> = access %U_arr_arr, %I, %J
    %u_arrI_arrJ:vec4<i32> = call %fn_u, %40
    %s:vec4<i32> = call %fn_s, %S
    %43:ptr<storage, vec4<i32>, read> = access %S_str, 0u
    %s_str:vec4<i32> = call %fn_s, %43
    %45:ptr<storage, vec4<i32>, read> = access %S_arr, 0i
    %s_arr0:vec4<i32> = call %fn_s, %45
    %47:ptr<storage, vec4<i32>, read> = access %S_arr, 1i
    %s_arr1:vec4<i32> = call %fn_s, %47
    %49:ptr<storage, vec4<i32>, read> = access %S_arr, %I
    %s_arrI:vec4<i32> = call %fn_s, %49
    %51:ptr<storage, vec4<i32>, read> = access %S_arr_arr, 1i, 0i
    %s_arr1_arr0:vec4<i32> = call %fn_s, %51
    %53:ptr<storage, vec4<i32>, read> = access %S_arr_arr, 2i, %I
    %s_arr2_arrI:vec4<i32> = call %fn_s, %53
    %55:ptr<storage, vec4<i32>, read> = access %S_arr_arr, %I, 2i
    %s_arrI_arr2:vec4<i32> = call %fn_s, %55
    %57:ptr<storage, vec4<i32>, read> = access %S_arr_arr, %I, %J
    %s_arrI_arrJ:vec4<i32> = call %fn_s, %57
    %w:vec4<i32> = call %fn_w, %W
    %60:ptr<workgroup, vec4<i32>, read_write> = access %W_str, 0u
    %w_str:vec4<i32> = call %fn_w, %60
    %62:ptr<workgroup, vec4<i32>, read_write> = access %W_arr, 0i
    %w_arr0:vec4<i32> = call %fn_w, %62
    %64:ptr<workgroup, vec4<i32>, read_write> = access %W_arr, 1i
    %w_arr1:vec4<i32> = call %fn_w, %64
    %66:ptr<workgroup, vec4<i32>, read_write> = access %W_arr, %I
    %w_arrI:vec4<i32> = call %fn_w, %66
    %68:ptr<workgroup, vec4<i32>, read_write> = access %W_arr_arr, 1i, 0i
    %w_arr1_arr0:vec4<i32> = call %fn_w, %68
    %70:ptr<workgroup, vec4<i32>, read_write> = access %W_arr_arr, 2i, %I
    %w_arr2_arrI:vec4<i32> = call %fn_w, %70
    %72:ptr<workgroup, vec4<i32>, read_write> = access %W_arr_arr, %I, 2i
    %w_arrI_arr2:vec4<i32> = call %fn_w, %72
    %74:ptr<workgroup, vec4<i32>, read_write> = access %W_arr_arr, %I, %J
    %w_arrI_arrJ:vec4<i32> = call %fn_w, %74
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
str = struct @align(16) {
  i:vec4<i32> @offset(0)
}

%b1 = block {  # root
  %U:ptr<uniform, vec4<i32>, read> = var @binding_point(0, 0)
  %U_str:ptr<uniform, str, read> = var @binding_point(0, 1)
  %U_arr:ptr<uniform, array<vec4<i32>, 8>, read> = var @binding_point(0, 2)
  %U_arr_arr:ptr<uniform, array<array<vec4<i32>, 8>, 4>, read> = var @binding_point(0, 3)
  %S:ptr<storage, vec4<i32>, read> = var @binding_point(1, 0)
  %S_str:ptr<storage, str, read> = var @binding_point(1, 1)
  %S_arr:ptr<storage, array<vec4<i32>, 8>, read> = var @binding_point(1, 2)
  %S_arr_arr:ptr<storage, array<array<vec4<i32>, 8>, 4>, read> = var @binding_point(1, 3)
  %W:ptr<workgroup, vec4<i32>, read_write> = var
  %W_str:ptr<workgroup, str, read_write> = var
  %W_arr:ptr<workgroup, array<vec4<i32>, 8>, read_write> = var
  %W_arr_arr:ptr<workgroup, array<array<vec4<i32>, 8>, 4>, read_write> = var
}

%fn_u_U = func():vec4<i32> -> %b2 {
  %b2 = block {
    %14:ptr<uniform, vec4<i32>, read> = access %U
    %15:vec4<i32> = load %14
    ret %15
  }
}
%fn_u_U_str_i = func():vec4<i32> -> %b3 {
  %b3 = block {
    %17:ptr<uniform, vec4<i32>, read> = access %U_str, 0u
    %18:vec4<i32> = load %17
    ret %18
  }
}
%fn_u_U_arr_X = func(%p_indices:array<u32, 1>):vec4<i32> -> %b4 {
  %b4 = block {
    %21:u32 = access %p_indices, 0u
    %22:ptr<uniform, vec4<i32>, read> = access %U_arr, %21
    %23:vec4<i32> = load %22
    ret %23
  }
}
%fn_u_U_arr_arr_X_X = func(%p_indices_1:array<u32, 2>):vec4<i32> -> %b5 {  # %p_indices_1: 'p_indices'
  %b5 = block {
    %26:u32 = access %p_indices_1, 0u
    %27:u32 = access %p_indices_1, 1u
    %28:ptr<uniform, vec4<i32>, read> = access %U_arr_arr, %26, %27
    %29:vec4<i32> = load %28
    ret %29
  }
}
%fn_s_S = func():vec4<i32> -> %b6 {
  %b6 = block {
    %31:ptr<storage, vec4<i32>, read> = access %S
    %32:vec4<i32> = load %31
    ret %32
  }
}
%fn_s_S_str_i = func():vec4<i32> -> %b7 {
  %b7 = block {
    %34:ptr<storage, vec4<i32>, read> = access %S_str, 0u
    %35:vec4<i32> = load %34
    ret %35
  }
}
%fn_s_S_arr_X = func(%p_indices_2:array<u32, 1>):vec4<i32> -> %b8 {  # %p_indices_2: 'p_indices'
  %b8 = block {
    %38:u32 = access %p_indices_2, 0u
    %39:ptr<storage, vec4<i32>, read> = access %S_arr, %38
    %40:vec4<i32> = load %39
    ret %40
  }
}
%fn_s_S_arr_arr_X_X = func(%p_indices_3:array<u32, 2>):vec4<i32> -> %b9 {  # %p_indices_3: 'p_indices'
  %b9 = block {
    %43:u32 = access %p_indices_3, 0u
    %44:u32 = access %p_indices_3, 1u
    %45:ptr<storage, vec4<i32>, read> = access %S_arr_arr, %43, %44
    %46:vec4<i32> = load %45
    ret %46
  }
}
%fn_w_W = func():vec4<i32> -> %b10 {
  %b10 = block {
    %48:ptr<workgroup, vec4<i32>, read_write> = access %W
    %49:vec4<i32> = load %48
    ret %49
  }
}
%fn_w_W_str_i = func():vec4<i32> -> %b11 {
  %b11 = block {
    %51:ptr<workgroup, vec4<i32>, read_write> = access %W_str, 0u
    %52:vec4<i32> = load %51
    ret %52
  }
}
%fn_w_W_arr_X = func(%p_indices_4:array<u32, 1>):vec4<i32> -> %b12 {  # %p_indices_4: 'p_indices'
  %b12 = block {
    %55:u32 = access %p_indices_4, 0u
    %56:ptr<workgroup, vec4<i32>, read_write> = access %W_arr, %55
    %57:vec4<i32> = load %56
    ret %57
  }
}
%fn_w_W_arr_arr_X_X = func(%p_indices_5:array<u32, 2>):vec4<i32> -> %b13 {  # %p_indices_5: 'p_indices'
  %b13 = block {
    %60:u32 = access %p_indices_5, 0u
    %61:u32 = access %p_indices_5, 1u
    %62:ptr<workgroup, vec4<i32>, read_write> = access %W_arr_arr, %60, %61
    %63:vec4<i32> = load %62
    ret %63
  }
}
%b = func():void -> %b14 {
  %b14 = block {
    %I:i32 = let 3i
    %J:i32 = let 4i
    %u:vec4<i32> = call %fn_u_U
    %u_str:vec4<i32> = call %fn_u_U_str_i
    %69:u32 = convert 0i
    %70:array<u32, 1> = construct %69
    %u_arr0:vec4<i32> = call %fn_u_U_arr_X, %70
    %72:u32 = convert 1i
    %73:array<u32, 1> = construct %72
    %u_arr1:vec4<i32> = call %fn_u_U_arr_X, %73
    %75:u32 = convert %I
    %76:array<u32, 1> = construct %75
    %u_arrI:vec4<i32> = call %fn_u_U_arr_X, %76
    %78:u32 = convert 1i
    %79:u32 = convert 0i
    %80:array<u32, 2> = construct %78, %79
    %u_arr1_arr0:vec4<i32> = call %fn_u_U_arr_arr_X_X, %80
    %82:u32 = convert 2i
    %83:u32 = convert %I
    %84:array<u32, 2> = construct %82, %83
    %u_arr2_arrI:vec4<i32> = call %fn_u_U_arr_arr_X_X, %84
    %86:u32 = convert %I
    %87:u32 = convert 2i
    %88:array<u32, 2> = construct %86, %87
    %u_arrI_arr2:vec4<i32> = call %fn_u_U_arr_arr_X_X, %88
    %90:u32 = convert %I
    %91:u32 = convert %J
    %92:array<u32, 2> = construct %90, %91
    %u_arrI_arrJ:vec4<i32> = call %fn_u_U_arr_arr_X_X, %92
    %s:vec4<i32> = call %fn_s_S
    %s_str:vec4<i32> = call %fn_s_S_str_i
    %96:u32 = convert 0i
    %97:array<u32, 1> = construct %96
    %s_arr0:vec4<i32> = call %fn_s_S_arr_X, %97
    %99:u32 = convert 1i
    %100:array<u32, 1> = construct %99
    %s_arr1:vec4<i32> = call %fn_s_S_arr_X, %100
    %102:u32 = convert %I
    %103:array<u32, 1> = construct %102
    %s_arrI:vec4<i32> = call %fn_s_S_arr_X, %103
    %105:u32 = convert 1i
    %106:u32 = convert 0i
    %107:array<u32, 2> = construct %105, %106
    %s_arr1_arr0:vec4<i32> = call %fn_s_S_arr_arr_X_X, %107
    %109:u32 = convert 2i
    %110:u32 = convert %I
    %111:array<u32, 2> = construct %109, %110
    %s_arr2_arrI:vec4<i32> = call %fn_s_S_arr_arr_X_X, %111
    %113:u32 = convert %I
    %114:u32 = convert 2i
    %115:array<u32, 2> = construct %113, %114
    %s_arrI_arr2:vec4<i32> = call %fn_s_S_arr_arr_X_X, %115
    %117:u32 = convert %I
    %118:u32 = convert %J
    %119:array<u32, 2> = construct %117, %118
    %s_arrI_arrJ:vec4<i32> = call %fn_s_S_arr_arr_X_X, %119
    %w:vec4<i32> = call %fn_w_W
    %w_str:vec4<i32> = call %fn_w_W_str_i
    %123:u32 = convert 0i
    %124:array<u32, 1> = construct %123
    %w_arr0:vec4<i32> = call %fn_w_W_arr_X, %124
    %126:u32 = convert 1i
    %127:array<u32, 1> = construct %126
    %w_arr1:vec4<i32> = call %fn_w_W_arr_X, %127
    %129:u32 = convert %I
    %130:array<u32, 1> = construct %129
    %w_arrI:vec4<i32> = call %fn_w_W_arr_X, %130
    %132:u32 = convert 1i
    %133:u32 = convert 0i
    %134:array<u32, 2> = construct %132, %133
    %w_arr1_arr0:vec4<i32> = call %fn_w_W_arr_arr_X_X, %134
    %136:u32 = convert 2i
    %137:u32 = convert %I
    %138:array<u32, 2> = construct %136, %137
    %w_arr2_arrI:vec4<i32> = call %fn_w_W_arr_arr_X_X, %138
    %140:u32 = convert %I
    %141:u32 = convert 2i
    %142:array<u32, 2> = construct %140, %141
    %w_arrI_arr2:vec4<i32> = call %fn_w_W_arr_arr_X_X, %142
    %144:u32 = convert %I
    %145:u32 = convert %J
    %146:array<u32, 2> = construct %144, %145
    %w_arrI_arrJ:vec4<i32> = call %fn_w_W_arr_arr_X_X, %146
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_Complex, Indexing) {
    Var* S = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 S = b.Var<storage, array<array<array<array<i32, 9>, 9>, 9>, 50>, read>("S");
                 S->SetBindingPoint(0, 0);
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_i = b.FunctionParam("i", ty.i32());
    fn_a->SetParams({fn_a_i});
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, fn_a_i); });

    auto* fn_b = b.Function("b", ty.i32());
    auto* fn_b_p = b.FunctionParam("p", ty.ptr<storage, array<array<array<i32, 9>, 9>, 9>, read>());
    fn_b->SetParams({fn_b_p});
    b.Append(fn_b->Block(), [&] {
        auto load_0 = b.Load(b.Access(ty.ptr<storage, i32, read>(), fn_b_p, 0_i, 1_i, 2_i));
        auto call_0 = b.Call(fn_a, load_0);
        auto call_1 = b.Call(fn_a, 3_i);
        auto load_1 = b.Load(b.Access(ty.ptr<storage, i32, read>(), fn_b_p, call_1, 4_i, 5_i));
        auto call_2 = b.Call(fn_a, load_1);
        auto call_3 = b.Call(fn_a, 7_i);
        auto load_2 = b.Load(b.Access(ty.ptr<storage, i32, read>(), fn_b_p, 6_i, call_3, 8_i));
        auto call_4 = b.Call(fn_a, load_2);
        auto load_3 =
            b.Load(b.Access(ty.ptr<storage, i32, read>(), fn_b_p, call_0, call_2, call_4));

        b.Return(fn_b, load_3);
    });

    auto* fn_c = b.Function("c", ty.void_());
    b.Append(fn_c->Block(), [&] {
        auto* access =
            b.Access(ty.ptr<storage, array<array<array<i32, 9>, 9>, 9>, read>(), S, 42_i);
        auto* v = b.Call(fn_b, access);
        b.ir.SetName(v, "v");
        b.Return(fn_c);
    });

    auto* src = R"(
%b1 = block {  # root
  %S:ptr<storage, array<array<array<array<i32, 9>, 9>, 9>, 50>, read> = var @binding_point(0, 0)
}

%a = func(%i:i32):i32 -> %b2 {
  %b2 = block {
    ret %i
  }
}
%b = func(%p:ptr<storage, array<array<array<i32, 9>, 9>, 9>, read>):i32 -> %b3 {
  %b3 = block {
    %6:ptr<storage, i32, read> = access %p, 0i, 1i, 2i
    %7:i32 = load %6
    %8:i32 = call %a, %7
    %9:i32 = call %a, 3i
    %10:ptr<storage, i32, read> = access %p, %9, 4i, 5i
    %11:i32 = load %10
    %12:i32 = call %a, %11
    %13:i32 = call %a, 7i
    %14:ptr<storage, i32, read> = access %p, 6i, %13, 8i
    %15:i32 = load %14
    %16:i32 = call %a, %15
    %17:ptr<storage, i32, read> = access %p, %8, %12, %16
    %18:i32 = load %17
    ret %18
  }
}
%c = func():void -> %b4 {
  %b4 = block {
    %20:ptr<storage, array<array<array<i32, 9>, 9>, 9>, read> = access %S, 42i
    %v:i32 = call %b, %20
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %S:ptr<storage, array<array<array<array<i32, 9>, 9>, 9>, 50>, read> = var @binding_point(0, 0)
}

%a = func(%i:i32):i32 -> %b2 {
  %b2 = block {
    ret %i
  }
}
%b_S_X = func(%p_indices:array<u32, 1>):i32 -> %b3 {
  %b3 = block {
    %6:u32 = access %p_indices, 0u
    %7:ptr<storage, array<array<array<i32, 9>, 9>, 9>, read> = access %S, %6
    %8:ptr<storage, i32, read> = access %7, 0i, 1i, 2i
    %9:i32 = load %8
    %10:i32 = call %a, %9
    %11:i32 = call %a, 3i
    %12:ptr<storage, i32, read> = access %7, %11, 4i, 5i
    %13:i32 = load %12
    %14:i32 = call %a, %13
    %15:i32 = call %a, 7i
    %16:ptr<storage, i32, read> = access %7, 6i, %15, 8i
    %17:i32 = load %16
    %18:i32 = call %a, %17
    %19:ptr<storage, i32, read> = access %7, %10, %14, %18
    %20:i32 = load %19
    ret %20
  }
}
%c = func():void -> %b4 {
  %b4 = block {
    %22:u32 = convert 42i
    %23:array<u32, 1> = construct %22
    %v:i32 = call %b_S_X, %23
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_Complex, IndexingInPtrCall) {
    Var* S = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 S = b.Var<storage, array<array<array<array<i32, 9>, 9>, 9>, 50>, read>("S");
                 S->SetBindingPoint(0, 0);
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_i = b.FunctionParam("i", ty.ptr<storage, i32, read>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_i,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_i)); });

    auto* fn_b = b.Function("b", ty.i32());
    auto* fn_b_p = b.FunctionParam("p", ty.ptr<storage, array<array<array<i32, 9>, 9>, 9>, read>());
    fn_b->SetParams({fn_b_p});
    b.Append(fn_b->Block(), [&] {
        auto access_0 = b.Access(ty.ptr<storage, i32, read>(), fn_b_p, 0_i, 1_i, 2_i);
        auto call_0 = b.Call(fn_a, 20_i, access_0, 30_i);

        auto access_1 = b.Access(ty.ptr<storage, i32, read>(), fn_b_p, 3_i, 4_i, 5_i);
        auto call_1 = b.Call(fn_a, 40_i, access_1, 50_i);

        auto access_2 = b.Access(ty.ptr<storage, i32, read>(), fn_b_p, 6_i, 7_i, 8_i);
        auto call_2 = b.Call(fn_a, 60_i, access_2, 70_i);

        auto access_3 = b.Access(ty.ptr<storage, i32, read>(), fn_b_p, call_0, call_1, call_2);
        auto call_3 = b.Call(fn_a, 10_i, access_3, 80_i);

        b.Return(fn_b, call_3);
    });

    auto* fn_c = b.Function("c", ty.void_());
    b.Append(fn_c->Block(), [&] {
        auto* access =
            b.Access(ty.ptr<storage, array<array<array<i32, 9>, 9>, 9>, read>(), S, 42_i);
        auto* v = b.Call(fn_b, access);
        b.ir.SetName(v, "v");
        b.Return(fn_c);
    });

    auto* src = R"(
%b1 = block {  # root
  %S:ptr<storage, array<array<array<array<i32, 9>, 9>, 9>, 50>, read> = var @binding_point(0, 0)
}

%a = func(%pre:i32, %i:ptr<storage, i32, read>, %post:i32):i32 -> %b2 {
  %b2 = block {
    %6:i32 = load %i
    ret %6
  }
}
%b = func(%p:ptr<storage, array<array<array<i32, 9>, 9>, 9>, read>):i32 -> %b3 {
  %b3 = block {
    %9:ptr<storage, i32, read> = access %p, 0i, 1i, 2i
    %10:i32 = call %a, 20i, %9, 30i
    %11:ptr<storage, i32, read> = access %p, 3i, 4i, 5i
    %12:i32 = call %a, 40i, %11, 50i
    %13:ptr<storage, i32, read> = access %p, 6i, 7i, 8i
    %14:i32 = call %a, 60i, %13, 70i
    %15:ptr<storage, i32, read> = access %p, %10, %12, %14
    %16:i32 = call %a, 10i, %15, 80i
    ret %16
  }
}
%c = func():void -> %b4 {
  %b4 = block {
    %18:ptr<storage, array<array<array<i32, 9>, 9>, 9>, read> = access %S, 42i
    %v:i32 = call %b, %18
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %S:ptr<storage, array<array<array<array<i32, 9>, 9>, 9>, 50>, read> = var @binding_point(0, 0)
}

%a_S_X_X_X_X = func(%pre:i32, %i_indices:array<u32, 4>, %post:i32):i32 -> %b2 {
  %b2 = block {
    %6:u32 = access %i_indices, 0u
    %7:u32 = access %i_indices, 1u
    %8:u32 = access %i_indices, 2u
    %9:u32 = access %i_indices, 3u
    %10:ptr<storage, i32, read> = access %S, %6, %7, %8, %9
    %11:i32 = load %10
    ret %11
  }
}
%b_S_X = func(%p_indices:array<u32, 1>):i32 -> %b3 {
  %b3 = block {
    %14:u32 = access %p_indices, 0u
    %15:u32 = convert 0i
    %16:u32 = convert 1i
    %17:u32 = convert 2i
    %18:array<u32, 4> = construct %14, %15, %16, %17
    %19:i32 = call %a_S_X_X_X_X, 20i, %18, 30i
    %20:u32 = convert 3i
    %21:u32 = convert 4i
    %22:u32 = convert 5i
    %23:array<u32, 4> = construct %14, %20, %21, %22
    %24:i32 = call %a_S_X_X_X_X, 40i, %23, 50i
    %25:u32 = convert 6i
    %26:u32 = convert 7i
    %27:u32 = convert 8i
    %28:array<u32, 4> = construct %14, %25, %26, %27
    %29:i32 = call %a_S_X_X_X_X, 60i, %28, 70i
    %30:u32 = convert %19
    %31:u32 = convert %24
    %32:u32 = convert %29
    %33:array<u32, 4> = construct %14, %30, %31, %32
    %34:i32 = call %a_S_X_X_X_X, 10i, %33, 80i
    ret %34
  }
}
%c = func():void -> %b4 {
  %b4 = block {
    %36:u32 = convert 42i
    %37:array<u32, 1> = construct %36
    %v:i32 = call %b_S_X, %37
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_Complex, IndexingDualPointers) {
    Var* S = nullptr;
    Var* U = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 S = b.Var<storage, array<array<array<i32, 9>, 9>, 50>, read>("S");
                 S->SetBindingPoint(0, 0);
                 U = b.Var<uniform, array<array<array<vec4<i32>, 9>, 9>, 50>, read>("U");
                 U->SetBindingPoint(0, 0);
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_i = b.FunctionParam("i", ty.i32());
    fn_a->SetParams({fn_a_i});
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, fn_a_i); });

    auto* fn_b = b.Function("b", ty.i32());
    auto* fn_b_s = b.FunctionParam("s", ty.ptr<storage, array<array<i32, 9>, 9>, read>());
    auto* fn_b_u = b.FunctionParam("u", ty.ptr<uniform, array<array<vec4<i32>, 9>, 9>, read>());
    fn_b->SetParams({fn_b_s, fn_b_u});
    b.Append(fn_b->Block(), [&] {
        auto access_0 = b.Access(ty.ptr<uniform, vec4<i32>, read>(), fn_b_u, 0_i, 1_i);
        auto call_0 = b.Call(fn_a, b.LoadVectorElement(access_0, 0_u));
        auto call_1 = b.Call(fn_a, 3_i);

        auto access_1 = b.Access(ty.ptr<uniform, vec4<i32>, read>(), fn_b_u, call_1, 4_i);
        auto call_2 = b.Call(fn_a, b.LoadVectorElement(access_1, 1_u));

        auto access_2 = b.Access(ty.ptr<storage, i32, read>(), fn_b_s, call_0, call_2);

        b.Return(fn_b, b.Load(access_2));
    });

    auto* fn_c = b.Function("c", ty.void_());
    b.Append(fn_c->Block(), [&] {
        auto* access_0 = b.Access(ty.ptr<storage, array<array<i32, 9>, 9>, read>(), S, 42_i);
        auto* access_1 = b.Access(ty.ptr<uniform, array<array<vec4<i32>, 9>, 9>, read>(), U, 24_i);
        auto* v = b.Call(fn_b, access_0, access_1);
        b.ir.SetName(v, "v");
        b.Return(fn_c);
    });

    auto* src = R"(
%b1 = block {  # root
  %S:ptr<storage, array<array<array<i32, 9>, 9>, 50>, read> = var @binding_point(0, 0)
  %U:ptr<uniform, array<array<array<vec4<i32>, 9>, 9>, 50>, read> = var @binding_point(0, 0)
}

%a = func(%i:i32):i32 -> %b2 {
  %b2 = block {
    ret %i
  }
}
%b = func(%s:ptr<storage, array<array<i32, 9>, 9>, read>, %u:ptr<uniform, array<array<vec4<i32>, 9>, 9>, read>):i32 -> %b3 {
  %b3 = block {
    %8:ptr<uniform, vec4<i32>, read> = access %u, 0i, 1i
    %9:i32 = load_vector_element %8, 0u
    %10:i32 = call %a, %9
    %11:i32 = call %a, 3i
    %12:ptr<uniform, vec4<i32>, read> = access %u, %11, 4i
    %13:i32 = load_vector_element %12, 1u
    %14:i32 = call %a, %13
    %15:ptr<storage, i32, read> = access %s, %10, %14
    %16:i32 = load %15
    ret %16
  }
}
%c = func():void -> %b4 {
  %b4 = block {
    %18:ptr<storage, array<array<i32, 9>, 9>, read> = access %S, 42i
    %19:ptr<uniform, array<array<vec4<i32>, 9>, 9>, read> = access %U, 24i
    %v:i32 = call %b, %18, %19
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %S:ptr<storage, array<array<array<i32, 9>, 9>, 50>, read> = var @binding_point(0, 0)
  %U:ptr<uniform, array<array<array<vec4<i32>, 9>, 9>, 50>, read> = var @binding_point(0, 0)
}

%a = func(%i:i32):i32 -> %b2 {
  %b2 = block {
    ret %i
  }
}
%b_S_X_U_X = func(%s_indices:array<u32, 1>, %u_indices:array<u32, 1>):i32 -> %b3 {
  %b3 = block {
    %8:u32 = access %s_indices, 0u
    %9:ptr<storage, array<array<i32, 9>, 9>, read> = access %S, %8
    %10:u32 = access %u_indices, 0u
    %11:ptr<uniform, array<array<vec4<i32>, 9>, 9>, read> = access %U, %10
    %12:ptr<uniform, vec4<i32>, read> = access %11, 0i, 1i
    %13:i32 = load_vector_element %12, 0u
    %14:i32 = call %a, %13
    %15:i32 = call %a, 3i
    %16:ptr<uniform, vec4<i32>, read> = access %11, %15, 4i
    %17:i32 = load_vector_element %16, 1u
    %18:i32 = call %a, %17
    %19:ptr<storage, i32, read> = access %9, %14, %18
    %20:i32 = load %19
    ret %20
  }
}
%c = func():void -> %b4 {
  %b4 = block {
    %22:u32 = convert 42i
    %23:array<u32, 1> = construct %22
    %24:u32 = convert 24i
    %25:array<u32, 1> = construct %24
    %v:i32 = call %b_S_X_U_X, %23, %25
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

}  // namespace complex_tests

}  // namespace
}  // namespace tint::core::ir::transform
