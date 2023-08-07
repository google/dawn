// Copyright 2021 The Tint Authors.
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

#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/load.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

using namespace tint::number_suffixes;  // NOLINT

struct ResolverPtrRefTest : public resolver::TestHelper, public testing::Test {};

TEST_F(ResolverPtrRefTest, AddressOf) {
    // var v : i32;
    // &v

    auto* v = Var("v", ty.i32());
    auto* expr = AddressOf(v);

    WrapInFunction(v, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(expr)->Is<type::Pointer>());
    EXPECT_TRUE(TypeOf(expr)->As<type::Pointer>()->StoreType()->Is<type::I32>());
    EXPECT_EQ(TypeOf(expr)->As<type::Pointer>()->AddressSpace(), core::AddressSpace::kFunction);
}

TEST_F(ResolverPtrRefTest, AddressOfThenDeref) {
    // var v : i32;
    // *(&v)

    auto* v = Var("v", ty.i32());
    auto* expr = Deref(AddressOf(v));

    WrapInFunction(v, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* load = Sem().Get<sem::Load>(expr);
    ASSERT_NE(load, nullptr);

    auto* ref = load->Reference();
    ASSERT_NE(ref, nullptr);

    ASSERT_TRUE(ref->Type()->Is<type::Reference>());
    EXPECT_TRUE(ref->Type()->As<type::Reference>()->StoreType()->Is<type::I32>());
}

TEST_F(ResolverPtrRefTest, DefaultPtrAddressSpace) {
    // https://gpuweb.github.io/gpuweb/wgsl/#storage-class

    auto* buf = Structure("S", Vector{Member("m", ty.i32())});
    auto* function = Var("f", ty.i32());
    auto* private_ = GlobalVar("p", ty.i32(), core::AddressSpace::kPrivate);
    auto* workgroup = GlobalVar("w", ty.i32(), core::AddressSpace::kWorkgroup);
    auto* uniform =
        GlobalVar("ub", ty.Of(buf), core::AddressSpace::kUniform, Binding(0_a), Group(0_a));
    auto* storage =
        GlobalVar("sb", ty.Of(buf), core::AddressSpace::kStorage, Binding(1_a), Group(0_a));

    auto* function_ptr =
        Let("f_ptr", ty.ptr(core::AddressSpace::kFunction, ty.i32()), AddressOf(function));
    auto* private_ptr =
        Let("p_ptr", ty.ptr(core::AddressSpace::kPrivate, ty.i32()), AddressOf(private_));
    auto* workgroup_ptr =
        Let("w_ptr", ty.ptr(core::AddressSpace::kWorkgroup, ty.i32()), AddressOf(workgroup));
    auto* uniform_ptr =
        Let("ub_ptr", ty.ptr(core::AddressSpace::kUniform, ty.Of(buf)), AddressOf(uniform));
    auto* storage_ptr =
        Let("sb_ptr", ty.ptr(core::AddressSpace::kStorage, ty.Of(buf)), AddressOf(storage));

    WrapInFunction(function, function_ptr, private_ptr, workgroup_ptr, uniform_ptr, storage_ptr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(function_ptr)->Is<type::Pointer>())
        << "function_ptr is " << TypeOf(function_ptr)->TypeInfo().name;
    ASSERT_TRUE(TypeOf(private_ptr)->Is<type::Pointer>())
        << "private_ptr is " << TypeOf(private_ptr)->TypeInfo().name;
    ASSERT_TRUE(TypeOf(workgroup_ptr)->Is<type::Pointer>())
        << "workgroup_ptr is " << TypeOf(workgroup_ptr)->TypeInfo().name;
    ASSERT_TRUE(TypeOf(uniform_ptr)->Is<type::Pointer>())
        << "uniform_ptr is " << TypeOf(uniform_ptr)->TypeInfo().name;
    ASSERT_TRUE(TypeOf(storage_ptr)->Is<type::Pointer>())
        << "storage_ptr is " << TypeOf(storage_ptr)->TypeInfo().name;

    EXPECT_EQ(TypeOf(function_ptr)->As<type::Pointer>()->Access(), core::Access::kReadWrite);
    EXPECT_EQ(TypeOf(private_ptr)->As<type::Pointer>()->Access(), core::Access::kReadWrite);
    EXPECT_EQ(TypeOf(workgroup_ptr)->As<type::Pointer>()->Access(), core::Access::kReadWrite);
    EXPECT_EQ(TypeOf(uniform_ptr)->As<type::Pointer>()->Access(), core::Access::kRead);
    EXPECT_EQ(TypeOf(storage_ptr)->As<type::Pointer>()->Access(), core::Access::kRead);
}

}  // namespace
}  // namespace tint::resolver
