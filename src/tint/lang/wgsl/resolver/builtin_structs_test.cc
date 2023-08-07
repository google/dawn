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

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

////////////////////////////////////////////////////////////////////////////////
// access
////////////////////////////////////////////////////////////////////////////////
using ResolverBuiltinStructs = ResolverTestWithParam<core::Builtin>;

TEST_P(ResolverBuiltinStructs, Resolve) {
    Enable(core::Extension::kF16);

    // var<private> p : NAME;
    auto* var = GlobalVar("p", ty(GetParam()), core::AddressSpace::kPrivate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* str = As<type::Struct>(TypeOf(var)->UnwrapRef());
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Name().Name(), tint::ToString(GetParam()));
    EXPECT_FALSE(Is<sem::Struct>(str));
}

INSTANTIATE_TEST_SUITE_P(,
                         ResolverBuiltinStructs,
                         testing::Values(core::Builtin::kAtomicCompareExchangeResultI32,
                                         core::Builtin::kAtomicCompareExchangeResultU32,
                                         core::Builtin::kFrexpResultAbstract,
                                         core::Builtin::kFrexpResultF16,
                                         core::Builtin::kFrexpResultF32,
                                         core::Builtin::kFrexpResultVec2Abstract,
                                         core::Builtin::kFrexpResultVec2F16,
                                         core::Builtin::kFrexpResultVec2F32,
                                         core::Builtin::kFrexpResultVec3Abstract,
                                         core::Builtin::kFrexpResultVec3F16,
                                         core::Builtin::kFrexpResultVec3F32,
                                         core::Builtin::kFrexpResultVec4Abstract,
                                         core::Builtin::kFrexpResultVec4F16,
                                         core::Builtin::kFrexpResultVec4F32,
                                         core::Builtin::kModfResultAbstract,
                                         core::Builtin::kModfResultF16,
                                         core::Builtin::kModfResultF32,
                                         core::Builtin::kModfResultVec2Abstract,
                                         core::Builtin::kModfResultVec2F16,
                                         core::Builtin::kModfResultVec2F32,
                                         core::Builtin::kModfResultVec3Abstract,
                                         core::Builtin::kModfResultVec3F16,
                                         core::Builtin::kModfResultVec3F32,
                                         core::Builtin::kModfResultVec4Abstract,
                                         core::Builtin::kModfResultVec4F16,
                                         core::Builtin::kModfResultVec4F32));

}  // namespace
}  // namespace tint::resolver
