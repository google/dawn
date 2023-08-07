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
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/utils/text/string_stream.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::msl::writer {
namespace {

using MslASTPrinterTest = TestHelper;

TEST_F(MslASTPrinterTest, EmitExpression_Call_WithoutParams) {
    Func("my_func", tint::Empty, ty.f32(), Vector{Return(1.23_f)});

    auto* call = Call("my_func");
    WrapInFunction(call);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "my_func()");
}

TEST_F(MslASTPrinterTest, EmitExpression_Call_WithParams) {
    Func("my_func",
         Vector{
             Param(Sym(), ty.f32()),
             Param(Sym(), ty.f32()),
         },
         ty.f32(),
         Vector{
             Return(1.23_f),
         });
    GlobalVar("param1", ty.f32(), core::AddressSpace::kPrivate);
    GlobalVar("param2", ty.f32(), core::AddressSpace::kPrivate);

    auto* call = Call("my_func", "param1", "param2");
    WrapInFunction(call);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "my_func(param1, param2)");
}

TEST_F(MslASTPrinterTest, EmitStatement_Call) {
    Func("my_func",
         Vector{
             Param(Sym(), ty.f32()),
             Param(Sym(), ty.f32()),
         },
         ty.void_(), tint::Empty, tint::Empty);
    GlobalVar("param1", ty.f32(), core::AddressSpace::kPrivate);
    GlobalVar("param2", ty.f32(), core::AddressSpace::kPrivate);

    auto* call = Call("my_func", "param1", "param2");
    auto* stmt = CallStmt(call);
    WrapInFunction(stmt);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), "  my_func(param1, param2);\n");
}

}  // namespace
}  // namespace tint::msl::writer
