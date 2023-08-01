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

#include "src/tint/lang/msl/writer/ast_printer/helper_test.h"
#include "src/tint/utils/text/string_stream.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::msl::writer {
namespace {

using MslASTPrinterTest = TestHelper;

TEST_F(MslASTPrinterTest, EmitExpression_Bitcast) {
    auto* a = Let("a", Expr(1_i));
    auto* bitcast = Bitcast<f32>(Expr("a"));
    WrapInFunction(a, bitcast);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, bitcast)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "as_type<float>(a)");
}

}  // namespace
}  // namespace tint::msl::writer
