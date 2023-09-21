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

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

////////////////////////////////////////////////////////////////////////////////
// access
////////////////////////////////////////////////////////////////////////////////
using ResolverBuiltinStructs = ResolverTestWithParam<core::BuiltinType>;

TEST_P(ResolverBuiltinStructs, Resolve) {
    Enable(wgsl::Extension::kF16);

    // var<private> p : NAME;
    auto* var = GlobalVar("p", ty(GetParam()), core::AddressSpace::kPrivate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* str = As<core::type::Struct>(TypeOf(var)->UnwrapRef());
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Name().Name(), tint::ToString(GetParam()));
    EXPECT_FALSE(Is<sem::Struct>(str));
}

INSTANTIATE_TEST_SUITE_P(,
                         ResolverBuiltinStructs,
                         testing::Values(core::BuiltinType::kAtomicCompareExchangeResultI32,
                                         core::BuiltinType::kAtomicCompareExchangeResultU32,
                                         core::BuiltinType::kFrexpResultAbstract,
                                         core::BuiltinType::kFrexpResultF16,
                                         core::BuiltinType::kFrexpResultF32,
                                         core::BuiltinType::kFrexpResultVec2Abstract,
                                         core::BuiltinType::kFrexpResultVec2F16,
                                         core::BuiltinType::kFrexpResultVec2F32,
                                         core::BuiltinType::kFrexpResultVec3Abstract,
                                         core::BuiltinType::kFrexpResultVec3F16,
                                         core::BuiltinType::kFrexpResultVec3F32,
                                         core::BuiltinType::kFrexpResultVec4Abstract,
                                         core::BuiltinType::kFrexpResultVec4F16,
                                         core::BuiltinType::kFrexpResultVec4F32,
                                         core::BuiltinType::kModfResultAbstract,
                                         core::BuiltinType::kModfResultF16,
                                         core::BuiltinType::kModfResultF32,
                                         core::BuiltinType::kModfResultVec2Abstract,
                                         core::BuiltinType::kModfResultVec2F16,
                                         core::BuiltinType::kModfResultVec2F32,
                                         core::BuiltinType::kModfResultVec3Abstract,
                                         core::BuiltinType::kModfResultVec3F16,
                                         core::BuiltinType::kModfResultVec3F32,
                                         core::BuiltinType::kModfResultVec4Abstract,
                                         core::BuiltinType::kModfResultVec4F16,
                                         core::BuiltinType::kModfResultVec4F32));

}  // namespace
}  // namespace tint::resolver
