// Copyright 2022 The Tint Authors.
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

#include "src/tint/transform/disable_uniformity_analysis.h"

#include <string>
#include <utility>

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using DisableUniformityAnalysisTest = TransformTest;

TEST_F(DisableUniformityAnalysisTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_TRUE(ShouldRun<DisableUniformityAnalysis>(src));
}

TEST_F(DisableUniformityAnalysisTest, ShouldRunExtensionAlreadyPresent) {
    auto* src = R"(
enable chromium_disable_uniformity_analysis;
)";

    EXPECT_FALSE(ShouldRun<DisableUniformityAnalysis>(src));
}

TEST_F(DisableUniformityAnalysisTest, EmptyModule) {
    auto* src = R"()";

    auto* expect = R"(
enable chromium_disable_uniformity_analysis;
)";

    auto got = Run<DisableUniformityAnalysis>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DisableUniformityAnalysisTest, NonEmptyModule) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read> global : i32;

@compute @workgroup_size(64)
fn main() {
  if ((global == 42)) {
    workgroupBarrier();
  }
}
)";

    auto expect = "\nenable chromium_disable_uniformity_analysis;\n" + std::string(src);

    auto got = Run<DisableUniformityAnalysis>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
