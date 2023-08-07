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

#include "src/tint/lang/msl/writer/common/printer_support.h"
#include "gtest/gtest.h"

namespace tint::msl::writer {
namespace {

struct MslBuiltinData {
    core::BuiltinValue builtin;
    const char* attribute_name;
};
inline std::ostream& operator<<(std::ostream& out, MslBuiltinData data) {
    StringStream str;
    str << data.builtin;
    out << str.str();
    return out;
}

using MslBuiltinConversionTest = testing::TestWithParam<MslBuiltinData>;
TEST_P(MslBuiltinConversionTest, Emit) {
    auto params = GetParam();
    EXPECT_EQ(BuiltinToAttribute(params.builtin), std::string(params.attribute_name));
}

INSTANTIATE_TEST_SUITE_P(
    MslPrinterTest,
    MslBuiltinConversionTest,
    testing::Values(
        MslBuiltinData{core::BuiltinValue::kPosition, "position"},
        MslBuiltinData{core::BuiltinValue::kVertexIndex, "vertex_id"},
        MslBuiltinData{core::BuiltinValue::kInstanceIndex, "instance_id"},
        MslBuiltinData{core::BuiltinValue::kFrontFacing, "front_facing"},
        MslBuiltinData{core::BuiltinValue::kFragDepth, "depth(any)"},
        MslBuiltinData{core::BuiltinValue::kLocalInvocationId, "thread_position_in_threadgroup"},
        MslBuiltinData{core::BuiltinValue::kLocalInvocationIndex, "thread_index_in_threadgroup"},
        MslBuiltinData{core::BuiltinValue::kGlobalInvocationId, "thread_position_in_grid"},
        MslBuiltinData{core::BuiltinValue::kWorkgroupId, "threadgroup_position_in_grid"},
        MslBuiltinData{core::BuiltinValue::kNumWorkgroups, "threadgroups_per_grid"},
        MslBuiltinData{core::BuiltinValue::kSampleIndex, "sample_id"},
        MslBuiltinData{core::BuiltinValue::kSampleMask, "sample_mask"},
        MslBuiltinData{core::BuiltinValue::kPointSize, "point_size"}));

}  // namespace
}  // namespace tint::msl::writer
