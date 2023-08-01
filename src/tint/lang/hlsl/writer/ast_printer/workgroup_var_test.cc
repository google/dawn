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

#include "gmock/gmock.h"
#include "src/tint/lang/hlsl/writer/ast_printer/helper_test.h"
#include "src/tint/lang/wgsl/ast/id_attribute.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"

using ::testing::HasSubstr;

using namespace tint::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

using HlslASTPrinterTest_WorkgroupVar = TestHelper;

TEST_F(HlslASTPrinterTest_WorkgroupVar, Basic) {
    GlobalVar("wg", ty.f32(), builtin::AddressSpace::kWorkgroup);

    Func("main", tint::Empty, ty.void_(), Vector{Assign("wg", 1.2_f)},
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });
    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.Result(), HasSubstr("groupshared float wg;\n"));
}

TEST_F(HlslASTPrinterTest_WorkgroupVar, Aliased) {
    auto* alias = Alias("F32", ty.f32());

    GlobalVar("wg", ty.Of(alias), builtin::AddressSpace::kWorkgroup);

    Func("main", tint::Empty, ty.void_(), Vector{Assign("wg", 1.2_f)},
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });
    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.Result(), HasSubstr("groupshared float wg;\n"));
}

}  // namespace
}  // namespace tint::hlsl::writer
