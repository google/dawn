// Copyright 2020 The Tint Authors.
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

#include "gmock/gmock.h"
#include "src/tint/reader/spirv/parser_impl_test_helper.h"
#include "src/tint/reader/spirv/spirv_tools_helpers_test.h"

namespace tint::reader::spirv {
namespace {

using ::testing::Eq;
using ::testing::HasSubstr;

std::string Preamble() {
    return R"(
    OpCapability Shader
    OpCapability Sampled1D
    OpCapability Image1D
    OpCapability StorageImageExtendedFormats
    OpCapability ImageQuery
    OpMemoryModel Logical Simple
  )";
}

std::string FragMain() {
    return R"(
    OpEntryPoint Fragment %main "main" ; assume no IO
    OpExecutionMode %main OriginUpperLeft
  )";
}

std::string MainBody() {
    return R"(
    %main = OpFunction %void None %voidfn
    %main_entry = OpLabel
    OpReturn
    OpFunctionEnd
  )";
}

std::string CommonTypes() {
    return R"(
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %bool = OpTypeBool
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1

    %v2int = OpTypeVector %int 2
    %v3int = OpTypeVector %int 3
    %v4int = OpTypeVector %int 4
    %v2uint = OpTypeVector %uint 2
    %v3uint = OpTypeVector %uint 3
    %v4uint = OpTypeVector %uint 4
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
    %v4float = OpTypeVector %float 4

    %true = OpConstantTrue %bool
    %false = OpConstantFalse %bool

    %int_1 = OpConstant %int 1
    %int_m5 = OpConstant %int -5
    %int_min = OpConstant %int 0x80000000
    %int_max = OpConstant %int 0x7fffffff
    %uint_0 = OpConstant %uint 0
    %uint_max = OpConstant %uint 0xffffffff

    %float_minus_5 = OpConstant %float -5
    %float_half = OpConstant %float 0.5
    %float_ten = OpConstant %float 10
  )";
}

struct ConstantCase {
    std::string spirv_type;
    std::string spirv_value;
    std::string wgsl_value;
};
inline std::ostream& operator<<(std::ostream& out, const ConstantCase& c) {
    out << "ConstantCase(" << c.spirv_type << ", " << c.spirv_value << ", " << c.wgsl_value << ")";
    return out;
}

using SpvParserConstantTest = SpvParserTestBase<::testing::TestWithParam<ConstantCase>>;

TEST_P(SpvParserConstantTest, ReturnValue) {
    const auto spirv_type = GetParam().spirv_type;
    const auto spirv_value = GetParam().spirv_value;
    const auto wgsl_value = GetParam().wgsl_value;
    const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %fty = OpTypeFunction )" +
                          spirv_type + R"(

     %200 = OpFunction )" +
                          spirv_type + R"( None %fty
     %fentry = OpLabel
     OpReturnValue )" + spirv_value +
                          R"(
     OpFunctionEnd
     )" + MainBody();
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->Parse());
    auto fe = p->function_emitter(200);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program(), fe.ast_body());
    const auto expect = std::string("return ") + wgsl_value + std::string(";\n");

    EXPECT_EQ(got, expect);
}

INSTANTIATE_TEST_SUITE_P(Scalars,
                         SpvParserConstantTest,
                         ::testing::ValuesIn(std::vector<ConstantCase>{
                             {"%bool", "%true", "true"},
                             {"%bool", "%false", "false"},
                             {"%int", "%int_1", "1i"},
                             {"%int", "%int_m5", "-5i"},
                             {"%int", "%int_min", "i32(-2147483648)"},
                             {"%int", "%int_max", "2147483647i"},
                             {"%uint", "%uint_0", "0u"},
                             {"%uint", "%uint_max", "4294967295u"},
                             {"%float", "%float_minus_5", "-5.0f"},
                             {"%float", "%float_half", "0.5f"},
                             {"%float", "%float_ten", "10.0f"}}));

}  // namespace
}  // namespace tint::reader::spirv
