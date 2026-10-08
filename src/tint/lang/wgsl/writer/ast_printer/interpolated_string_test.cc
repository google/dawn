// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "gmock/gmock.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/interpolated_string_expression.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/ast/string_literal_expression.h"
#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"
#include "src/tint/lang/wgsl/writer/ast_printer/helper_test.h"
#include "src/tint/utils/text/string_stream.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::wgsl::writer {
namespace {

using WgslASTPrinterTest = TestHelper;

TEST_F(WgslASTPrinterTest, Emit_StringLiteral_Empty) {
    auto* lit = create<ast::StringLiteralExpression>("");

    ASTPrinter& gen = Build(/* resolve */ false);

    StringStream out;
    gen.EmitExpression(out, lit);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "``");
}

TEST_F(WgslASTPrinterTest, Emit_StringLiteral_Simple) {
    auto* lit = create<ast::StringLiteralExpression>("hello world");

    ASTPrinter& gen = Build(/* resolve */ false);

    StringStream out;
    gen.EmitExpression(out, lit);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "`hello world`");
}

TEST_F(WgslASTPrinterTest, Emit_StringLiteral_Escapes) {
    auto* lit = create<ast::StringLiteralExpression>("a\\b`c\nd\re\tf");

    ASTPrinter& gen = Build(/* resolve */ false);

    StringStream out;
    gen.EmitExpression(out, lit);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "`a\\\\b\\`c\\nd\\re\\tf`");
}

TEST_F(WgslASTPrinterTest, Emit_StringLiteral_Dollar) {
    auto* lit = create<ast::StringLiteralExpression>("$foo ${bar} $");

    ASTPrinter& gen = Build(/* resolve */ false);

    StringStream out;
    gen.EmitExpression(out, lit);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "`\\$foo \\${bar} \\$`");
}

TEST_F(WgslASTPrinterTest, Emit_InterpolatedString_SingleSubstitution) {
    auto* tmpl = create<ast::InterpolatedStringExpression>(Vector{
        create<ast::StringLiteralExpression>("val: "),
        Expr("x"),
    });

    ASTPrinter& gen = Build(/* resolve */ false);

    StringStream out;
    gen.EmitExpression(out, tmpl);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "`val: ${x}`");
}

TEST_F(WgslASTPrinterTest, Emit_InterpolatedString_MultipleSubstitutions) {
    auto* tmpl = create<ast::InterpolatedStringExpression>(Vector{
        create<ast::StringLiteralExpression>("x: "),
        Expr("x"),
        create<ast::StringLiteralExpression>(", y: "),
        Expr("y"),
        create<ast::StringLiteralExpression>("!"),
    });

    ASTPrinter& gen = Build(/* resolve */ false);

    StringStream out;
    gen.EmitExpression(out, tmpl);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "`x: ${x}, y: ${y}!`");
}

TEST_F(WgslASTPrinterTest, Emit_InterpolatedString_HeadSubstitution) {
    auto* tmpl = create<ast::InterpolatedStringExpression>(Vector{
        Expr("x"),
        create<ast::StringLiteralExpression>(" is the answer"),
    });

    ASTPrinter& gen = Build(/* resolve */ false);

    StringStream out;
    gen.EmitExpression(out, tmpl);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "`${x} is the answer`");
}

TEST_F(WgslASTPrinterTest, Emit_InterpolatedString_TailSubstitution) {
    auto* tmpl = create<ast::InterpolatedStringExpression>(Vector{
        create<ast::StringLiteralExpression>("the answer is "),
        Expr("x"),
    });

    ASTPrinter& gen = Build(/* resolve */ false);

    StringStream out;
    gen.EmitExpression(out, tmpl);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "`the answer is ${x}`");
}

TEST_F(WgslASTPrinterTest, Emit_InterpolatedString_AdjacentSubstitutions) {
    auto* tmpl = create<ast::InterpolatedStringExpression>(Vector{
        Expr("x"),
        Expr("y"),
    });

    ASTPrinter& gen = Build(/* resolve */ false);

    StringStream out;
    gen.EmitExpression(out, tmpl);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "`${x}${y}`");
}

TEST_F(WgslASTPrinterTest, Emit_InterpolatedString_ExpressionsWithOperators) {
    auto* tmpl = create<ast::InterpolatedStringExpression>(Vector{
        create<ast::StringLiteralExpression>("result: "),
        Add("x", 1_i),
    });

    ASTPrinter& gen = Build(/* resolve */ false);

    StringStream out;
    gen.EmitExpression(out, tmpl);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "`result: ${(x + 1i)}`");
}

TEST_F(WgslASTPrinterTest, Emit_InterpolatedString_Nested) {
    auto* inner = create<ast::InterpolatedStringExpression>(Vector{
        create<ast::StringLiteralExpression>("inner "),
        Expr("x"),
    });
    auto* outer = create<ast::InterpolatedStringExpression>(Vector{
        create<ast::StringLiteralExpression>("outer "),
        inner,
    });

    ASTPrinter& gen = Build(/* resolve */ false);

    StringStream out;
    gen.EmitExpression(out, outer);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "`outer ${`inner ${x}`}`");
}

TEST_F(WgslASTPrinterTest, Emit_CallPrintString) {
    auto* call = Call("print", create<ast::StringLiteralExpression>("hello world"));
    WrapInFunction(CallStmt(call));

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, call);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "print(`hello world`)");
}

TEST_F(WgslASTPrinterTest, Emit_CallPrintInterpolatedString) {
    auto* tmpl = create<ast::InterpolatedStringExpression>(Vector{
        create<ast::StringLiteralExpression>("value: "),
        Expr(42_i),
    });
    auto* call = Call("print", tmpl);
    WrapInFunction(CallStmt(call));

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, call);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "print(`value: ${42i}`)");
}

TEST_F(WgslASTPrinterTest, Generate_Print) {
    Func("main", tint::Empty, ty.void_(),
         Vector{
             CallStmt(Call("print", create<ast::InterpolatedStringExpression>(Vector{
                                        create<ast::StringLiteralExpression>("x = "),
                                        Expr(1_i),
                                    }))),
         },
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    ASTPrinter& gen = Build();
    EXPECT_TRUE(gen.Generate());
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(@compute @workgroup_size(1i)
fn main() {
  print(`x = ${1i}`);
}
)");
}

}  // namespace
}  // namespace tint::wgsl::writer
