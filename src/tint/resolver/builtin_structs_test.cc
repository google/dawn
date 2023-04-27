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

#include "src/tint/resolver/resolver.h"
#include "src/tint/resolver/resolver_test_helper.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

////////////////////////////////////////////////////////////////////////////////
// access
////////////////////////////////////////////////////////////////////////////////
using ResolverBuiltinStructs = ResolverTestWithParam<builtin::Builtin>;

TEST_P(ResolverBuiltinStructs, Resolve) {
    Enable(builtin::Extension::kF16);

    // var<private> p : NAME;
    auto* var = GlobalVar("p", ty(GetParam()), builtin::AddressSpace::kPrivate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* str = As<type::Struct>(TypeOf(var)->UnwrapRef());
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Name().Name(), utils::ToString(GetParam()));
    EXPECT_FALSE(Is<sem::Struct>(str));
}

INSTANTIATE_TEST_SUITE_P(,
                         ResolverBuiltinStructs,
                         testing::Values(builtin::Builtin::kAtomicCompareExchangeResultI32,
                                         builtin::Builtin::kAtomicCompareExchangeResultU32,
                                         builtin::Builtin::kFrexpResultAbstract,
                                         builtin::Builtin::kFrexpResultF16,
                                         builtin::Builtin::kFrexpResultF32,
                                         builtin::Builtin::kFrexpResultVec2Abstract,
                                         builtin::Builtin::kFrexpResultVec2F16,
                                         builtin::Builtin::kFrexpResultVec2F32,
                                         builtin::Builtin::kFrexpResultVec3Abstract,
                                         builtin::Builtin::kFrexpResultVec3F16,
                                         builtin::Builtin::kFrexpResultVec3F32,
                                         builtin::Builtin::kFrexpResultVec4Abstract,
                                         builtin::Builtin::kFrexpResultVec4F16,
                                         builtin::Builtin::kFrexpResultVec4F32,
                                         builtin::Builtin::kModfResultAbstract,
                                         builtin::Builtin::kModfResultF16,
                                         builtin::Builtin::kModfResultF32,
                                         builtin::Builtin::kModfResultVec2Abstract,
                                         builtin::Builtin::kModfResultVec2F16,
                                         builtin::Builtin::kModfResultVec2F32,
                                         builtin::Builtin::kModfResultVec3Abstract,
                                         builtin::Builtin::kModfResultVec3F16,
                                         builtin::Builtin::kModfResultVec3F32,
                                         builtin::Builtin::kModfResultVec4Abstract,
                                         builtin::Builtin::kModfResultVec4F16,
                                         builtin::Builtin::kModfResultVec4F32));

}  // namespace
}  // namespace tint::resolver
