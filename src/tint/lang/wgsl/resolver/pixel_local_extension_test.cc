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

#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ResolverPixelLocalExtensionTest = ResolverTest;

TEST_F(ResolverPixelLocalExtensionTest, AddressSpaceUsedWithExtension) {
    // enable chromium_experimental_pixel_local;
    // var<pixel_local> v : f16;
    Enable(Source{{12, 34}}, core::Extension::kChromiumExperimentalPixelLocal);

    GlobalVar("v", ty.u32(), core::AddressSpace::kPixelLocal);

#if TINT_ENABLE_LOCAL_STORAGE_EXTENSION
    EXPECT_TRUE(r()->Resolve()) << r()->error();
#else
    // TODO(crbug.com/dawn/1704): Remove when chromium_experimental_pixel_local is production-ready
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: chromium_experimental_pixel_local requires TINT_ENABLE_LOCAL_STORAGE_EXTENSION)");
#endif
}

TEST_F(ResolverPixelLocalExtensionTest, AddressSpaceUsedWithoutExtension) {
    // var<pixel_local> v : u32;
    AST().AddGlobalVariable(create<ast::Var>(
        /* name */ Ident("v"),
        /* type */ ty.u32(),
        /* declared_address_space */ Expr(Source{{12, 34}}, core::AddressSpace::kPixelLocal),
        /* declared_access */ nullptr,
        /* initializer */ nullptr,
        /* attributes */ Empty));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: 'pixel_local' address space requires the 'chromium_experimental_pixel_local' extension enabled)");
}

#if TINT_ENABLE_LOCAL_STORAGE_EXTENSION

namespace type_tests {
struct Case {
    builder::ast_type_func_ptr type;
    std::string name;
    bool pass;
};

static std::ostream& operator<<(std::ostream& o, const Case& c) {
    return o << c.name;
}

template <typename T>
Case Pass() {
    return Case{builder::DataType<T>::AST, builder::DataType<T>::Name(), true};
}

template <typename T>
Case Fail() {
    return Case{builder::DataType<T>::AST, builder::DataType<T>::Name(), false};
}

using ResolverPixelLocalExtensionTest_Types = ResolverTestWithParam<Case>;

TEST_P(ResolverPixelLocalExtensionTest_Types, Direct) {
    // var<pixel_local> v : <type>;

    Enable(core::Extension::kChromiumExperimentalPixelLocal);
    GlobalVar(Source{{12, 34}}, "v", GetParam().type(*this), core::AddressSpace::kPixelLocal);

    if (GetParam().pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(
            r()->error(),
            R"(12:34 error: 'pixel_local' address space variables can only be of type 'i32', 'u32', 'f32' or a struct of those types)");
    }
}

TEST_P(ResolverPixelLocalExtensionTest_Types, Struct) {
    // struct S {
    //   a : i32,
    //   m : <type>,
    // }
    // var<pixel_local> v : S;

    Enable(core::Extension::kChromiumExperimentalPixelLocal);
    Structure("S", Vector{
                       Member("a", ty.i32()),
                       Member(Source{{12, 34}}, "m", GetParam().type(*this)),
                   });
    GlobalVar(Source{{56, 78}}, "v", ty("S"), core::AddressSpace::kPixelLocal);

    if (GetParam().pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(
            r()->error(),
            R"(12:34 error: struct members used in the 'pixel_local' address space can only be of the type 'i32', 'u32' or 'f32'
56:78 note: struct 'S' used in the 'pixel_local' address space here)");
    }
}

INSTANTIATE_TEST_SUITE_P(Valid,
                         ResolverPixelLocalExtensionTest_Types,
                         testing::Values(Pass<i32>(),  //
                                         Pass<u32>(),  //
                                         Pass<f32>()));

INSTANTIATE_TEST_SUITE_P(Invalid,
                         ResolverPixelLocalExtensionTest_Types,
                         testing::Values(Fail<bool>(),
                                         Fail<atomic<i32>>(),
                                         Fail<vec2<f32>>(),
                                         Fail<vec3<i32>>(),
                                         Fail<vec4<u32>>(),
                                         Fail<array<u32, 4>>()));

}  // namespace type_tests

#endif

}  // namespace
}  // namespace tint::resolver
