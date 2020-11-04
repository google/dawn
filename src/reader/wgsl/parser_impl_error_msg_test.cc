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

#include "gtest/gtest.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

class ParserImplErrorTest : public ParserImplTest {};

#define EXPECT(SOURCE, EXPECTED)                                     \
  do {                                                               \
    std::string source = SOURCE;                                     \
    std::string expected = EXPECTED;                                 \
    auto* p = parser(source);                                        \
    EXPECT_EQ(false, p->Parse());                                    \
    EXPECT_EQ(true, p->diagnostics().contains_errors());             \
    EXPECT_EQ(expected, diag::Formatter().format(p->diagnostics())); \
  } while (false)

TEST_F(ParserImplErrorTest, AdditiveInvalidExpr) {
  EXPECT("fn f() -> void { return 1.0 + <; }",
         "test.wgsl:1:31 error: unable to parse right side of + expression\n"
         "fn f() -> void { return 1.0 + <; }\n"
         "                              ^\n");
}

TEST_F(ParserImplErrorTest, AndInvalidExpr) {
  EXPECT("fn f() -> void { return 1 & >; }",
         "test.wgsl:1:29 error: unable to parse right side of & expression\n"
         "fn f() -> void { return 1 & >; }\n"
         "                            ^\n");
}

TEST_F(ParserImplErrorTest, ArrayIndexExprInvalidExpr) {
  EXPECT("fn f() -> void { x = y[^]; }",
         "test.wgsl:1:24 error: unable to parse expression inside []\n"
         "fn f() -> void { x = y[^]; }\n"
         "                       ^\n");
}

TEST_F(ParserImplErrorTest, ArrayIndexExprMissingRBracket) {
  EXPECT("fn f() -> void { x = y[1; }",
         "test.wgsl:1:25 error: missing ] for array accessor\n"
         "fn f() -> void { x = y[1; }\n"
         "                        ^\n");
}

TEST_F(ParserImplErrorTest, AssignmentStmtMissingAssignment) {
  EXPECT("fn f() -> void { a; }",
         "test.wgsl:1:19 error: missing = for assignment\n"
         "fn f() -> void { a; }\n"
         "                  ^\n");
}

TEST_F(ParserImplErrorTest, AssignmentStmtMissingAssignment2) {
  EXPECT("fn f() -> void { a : i32; }",
         "test.wgsl:1:20 error: missing = for assignment\n"
         "fn f() -> void { a : i32; }\n"
         "                   ^\n");
}

TEST_F(ParserImplErrorTest, AssignmentStmtMissingSemicolon) {
  EXPECT("fn f() -> void { a = 1 }",
         "test.wgsl:1:24 error: expected ';' for assignment statement\n"
         "fn f() -> void { a = 1 }\n"
         "                       ^\n");
}

TEST_F(ParserImplErrorTest, AssignmentStmtInvalidRHS) {
  EXPECT("fn f() -> void { a = >; }",
         "test.wgsl:1:22 error: unable to parse right side of assignment\n"
         "fn f() -> void { a = >; }\n"
         "                     ^\n");
}

TEST_F(ParserImplErrorTest, BitcastExprMissingLessThan) {
  EXPECT("fn f() -> void { x = bitcast(y); }",
         "test.wgsl:1:29 error: missing < for bitcast expression\n"
         "fn f() -> void { x = bitcast(y); }\n"
         "                            ^\n");
}

TEST_F(ParserImplErrorTest, BitcastExprMissingGreaterThan) {
  EXPECT("fn f() -> void { x = bitcast<u32(y); }",
         "test.wgsl:1:33 error: missing > for bitcast expression\n"
         "fn f() -> void { x = bitcast<u32(y); }\n"
         "                                ^\n");
}

TEST_F(ParserImplErrorTest, BitcastExprMissingType) {
  EXPECT("fn f() -> void { x = bitcast<>(y); }",
         "test.wgsl:1:30 error: missing type for bitcast expression\n"
         "fn f() -> void { x = bitcast<>(y); }\n"
         "                             ^\n");
}

TEST_F(ParserImplErrorTest, BreakStmtMissingSemicolon) {
  EXPECT("fn f() -> void { loop { break } }",
         "test.wgsl:1:31 error: expected ';' for break statement\n"
         "fn f() -> void { loop { break } }\n"
         "                              ^\n");
}

TEST_F(ParserImplErrorTest, CallExprMissingRParen) {
  EXPECT("fn f() -> void { x = f(1.; }",
         "test.wgsl:1:26 error: expected ')' for call expression\n"
         "fn f() -> void { x = f(1.; }\n"
         "                         ^\n");
}

TEST_F(ParserImplErrorTest, CallStmtMissingRParen) {
  EXPECT("fn f() -> void { f(1.; }",
         "test.wgsl:1:22 error: expected ')' for call statement\n"
         "fn f() -> void { f(1.; }\n"
         "                     ^\n");
}

TEST_F(ParserImplErrorTest, CallStmtInvalidArgument0) {
  EXPECT("fn f() -> void { f(<); }",
         "test.wgsl:1:20 error: unable to parse argument expression\n"
         "fn f() -> void { f(<); }\n"
         "                   ^\n");
}

TEST_F(ParserImplErrorTest, CallStmtInvalidArgument1) {
  EXPECT(
      "fn f() -> void { f(1.0, <); }",
      "test.wgsl:1:25 error: unable to parse argument expression after comma\n"
      "fn f() -> void { f(1.0, <); }\n"
      "                        ^\n");
}

TEST_F(ParserImplErrorTest, CallStmtMissingSemicolon) {
  EXPECT("fn f() -> void { f() }",
         "test.wgsl:1:22 error: expected ';' for function call\n"
         "fn f() -> void { f() }\n"
         "                     ^\n");
}

TEST_F(ParserImplErrorTest, ConstructorExprMissingLParen) {
  EXPECT("fn f() -> void { x = vec2<u32>1,2); }",
         "test.wgsl:1:31 error: expected '(' for type constructor\n"
         "fn f() -> void { x = vec2<u32>1,2); }\n"
         "                              ^\n");
}

TEST_F(ParserImplErrorTest, ConstructorExprMissingRParen) {
  EXPECT("fn f() -> void { x = vec2<u32>(1,2; }",
         "test.wgsl:1:35 error: expected ')' for type constructor\n"
         "fn f() -> void { x = vec2<u32>(1,2; }\n"
         "                                  ^\n");
}

TEST_F(ParserImplErrorTest, ConstVarStmtInvalid) {
  EXPECT("fn f() -> void { const >; }",
         "test.wgsl:1:24 error: unable to parse variable declaration\n"
         "fn f() -> void { const >; }\n"
         "                       ^\n");
}

TEST_F(ParserImplErrorTest, ConstVarStmtMissingAssignment) {
  EXPECT("fn f() -> void { const a : i32; }",
         "test.wgsl:1:31 error: missing = for constant declaration\n"
         "fn f() -> void { const a : i32; }\n"
         "                              ^\n");
}

TEST_F(ParserImplErrorTest, ConstVarStmtMissingConstructor) {
  EXPECT("fn f() -> void { const a : i32 = >; }",
         "test.wgsl:1:34 error: missing constructor for const declaration\n"
         "fn f() -> void { const a : i32 = >; }\n"
         "                                 ^\n");
}

TEST_F(ParserImplErrorTest, ContinueStmtMissingSemicolon) {
  EXPECT("fn f() -> void { loop { continue } }",
         "test.wgsl:1:34 error: expected ';' for continue statement\n"
         "fn f() -> void { loop { continue } }\n"
         "                                 ^\n");
}

TEST_F(ParserImplErrorTest, DiscardStmtMissingSemicolon) {
  EXPECT("fn f() -> void { discard }",
         "test.wgsl:1:26 error: expected ';' for discard statement\n"
         "fn f() -> void { discard }\n"
         "                         ^\n");
}

TEST_F(ParserImplErrorTest, EqualityInvalidExpr) {
  EXPECT("fn f() -> void { return 1 == >; }",
         "test.wgsl:1:30 error: unable to parse right side of == expression\n"
         "fn f() -> void { return 1 == >; }\n"
         "                             ^\n");
}

TEST_F(ParserImplErrorTest, ForLoopInitializerMissingSemicolon) {
  EXPECT("fn f() -> void { for (var i : i32 = 0 i < 8; i=i+1) {} }",
         "test.wgsl:1:39 error: expected ';' for initializer in for loop\n"
         "fn f() -> void { for (var i : i32 = 0 i < 8; i=i+1) {} }\n"
         "                                      ^\n");
}

TEST_F(ParserImplErrorTest, ForLoopConditionMissingSemicolon) {
  EXPECT("fn f() -> void { for (var i : i32 = 0; i < 8 i=i+1) {} }",
         "test.wgsl:1:46 error: expected ';' for condition in for loop\n"
         "fn f() -> void { for (var i : i32 = 0; i < 8 i=i+1) {} }\n"
         "                                             ^\n");
}

TEST_F(ParserImplErrorTest, ForLoopMissingLParen) {
  EXPECT("fn f() -> void { for var i : i32 = 0; i < 8; i=i+1) {} }",
         "test.wgsl:1:22 error: expected '(' for for loop\n"
         "fn f() -> void { for var i : i32 = 0; i < 8; i=i+1) {} }\n"
         "                     ^^^\n");
}

TEST_F(ParserImplErrorTest, ForLoopMissingRParen) {
  EXPECT("fn f() -> void { for (var i : i32 = 0; i < 8; i=i+1 {} }",
         "test.wgsl:1:53 error: expected ')' for for loop\n"
         "fn f() -> void { for (var i : i32 = 0; i < 8; i=i+1 {} }\n"
         "                                                    ^\n");
}

TEST_F(ParserImplErrorTest, ForLoopMissingLBrace) {
  EXPECT("fn f() -> void { for (var i : i32 = 0; i < 8; i=i+1) } }",
         "test.wgsl:1:54 error: expected '{' for for loop\n"
         "fn f() -> void { for (var i : i32 = 0; i < 8; i=i+1) } }\n"
         "                                                     ^\n");
}

TEST_F(ParserImplErrorTest, ForLoopMissingRBrace) {
  EXPECT("fn f() -> void { for (var i : i32 = 0; i < 8; i=i+1) {",
         "test.wgsl:1:55 error: expected '}' for for loop\n"
         "fn f() -> void { for (var i : i32 = 0; i < 8; i=i+1) {\n"
         "                                                      ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclInvalid) {
  EXPECT("[[stage(vertex)]] x;",
         "test.wgsl:1:19 error: error parsing function declaration\n"
         "[[stage(vertex)]] x;\n"
         "                  ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoMissingEnd) {
  EXPECT("[[stage(vertex) fn f() -> void {}",
         "test.wgsl:1:17 error: missing ]] for function decorations\n"
         "[[stage(vertex) fn f() -> void {}\n"
         "                ^^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoStageMissingLParen) {
  EXPECT("[[stage vertex]] fn f() -> void {}",
         "test.wgsl:1:9 error: expected '(' for stage decoration\n"
         "[[stage vertex]] fn f() -> void {}\n"
         "        ^^^^^^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoStageMissingRParen) {
  EXPECT("[[stage(vertex]] fn f() -> void {}",
         "test.wgsl:1:15 error: expected ')' for stage decoration\n"
         "[[stage(vertex]] fn f() -> void {}\n"
         "              ^^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoStageInvalid) {
  EXPECT("[[stage(x)]] fn f() -> void {}",
         "test.wgsl:1:9 error: invalid value for stage decoration\n"
         "[[stage(x)]] fn f() -> void {}\n"
         "        ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoStageTypeInvalid) {
  // TODO(bclayton) - BUG(https://crbug.com/tint/291)
  EXPECT("[[shader(vertex)]] fn main() -> void {}",
         "test.wgsl:1:1 error: invalid token\n"
         "[[shader(vertex)]] fn main() -> void {}\n"
         "^^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeMissingLParen) {
  EXPECT("[[workgroup_size 1]] fn f() -> void {}",
         "test.wgsl:1:18 error: expected '(' for workgroup_size decoration\n"
         "[[workgroup_size 1]] fn f() -> void {}\n"
         "                 ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeMissingRParen) {
  EXPECT("[[workgroup_size(1]] fn f() -> void {}",
         "test.wgsl:1:19 error: expected ')' for workgroup_size decoration\n"
         "[[workgroup_size(1]] fn f() -> void {}\n"
         "                  ^^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeXInvalid) {
  EXPECT("[[workgroup_size(x)]] fn f() -> void {}",
         "test.wgsl:1:18 error: expected signed integer literal for "
         "workgroup_size x parameter\n"
         "[[workgroup_size(x)]] fn f() -> void {}\n"
         "                 ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeXNegative) {
  EXPECT("[[workgroup_size(-1)]] fn f() -> void {}",
         "test.wgsl:1:18 error: workgroup_size x parameter must be greater "
         "than 0\n"
         "[[workgroup_size(-1)]] fn f() -> void {}\n"
         "                 ^^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeXZero) {
  EXPECT("[[workgroup_size(0)]] fn f() -> void {}",
         "test.wgsl:1:18 error: workgroup_size x parameter must be greater "
         "than 0\n"
         "[[workgroup_size(0)]] fn f() -> void {}\n"
         "                 ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeYInvalid) {
  EXPECT("[[workgroup_size(1, x)]] fn f() -> void {}",
         "test.wgsl:1:21 error: expected signed integer literal for "
         "workgroup_size y parameter\n"
         "[[workgroup_size(1, x)]] fn f() -> void {}\n"
         "                    ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeYNegative) {
  EXPECT("[[workgroup_size(1, -1)]] fn f() -> void {}",
         "test.wgsl:1:21 error: workgroup_size y parameter must be greater "
         "than 0\n"
         "[[workgroup_size(1, -1)]] fn f() -> void {}\n"
         "                    ^^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeYZero) {
  EXPECT("[[workgroup_size(1, 0)]] fn f() -> void {}",
         "test.wgsl:1:21 error: workgroup_size y parameter must be greater "
         "than 0\n"
         "[[workgroup_size(1, 0)]] fn f() -> void {}\n"
         "                    ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeZInvalid) {
  EXPECT("[[workgroup_size(1, 2, x)]] fn f() -> void {}",
         "test.wgsl:1:24 error: expected signed integer literal for "
         "workgroup_size z parameter\n"
         "[[workgroup_size(1, 2, x)]] fn f() -> void {}\n"
         "                       ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeZNegative) {
  EXPECT("[[workgroup_size(1, 2, -1)]] fn f() -> void {}",
         "test.wgsl:1:24 error: workgroup_size z parameter must be greater "
         "than 0\n"
         "[[workgroup_size(1, 2, -1)]] fn f() -> void {}\n"
         "                       ^^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeZZero) {
  EXPECT("[[workgroup_size(1, 2, 0)]] fn f() -> void {}",
         "test.wgsl:1:24 error: workgroup_size z parameter must be greater "
         "than 0\n"
         "[[workgroup_size(1, 2, 0)]] fn f() -> void {}\n"
         "                       ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingIdentifier) {
  EXPECT("fn () -> void {}",
         "test.wgsl:1:4 error: expected identifier for function declaration\n"
         "fn () -> void {}\n"
         "   ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingLParen) {
  EXPECT("fn f) -> void {}",
         "test.wgsl:1:5 error: expected '(' for function declaration\n"
         "fn f) -> void {}\n"
         "    ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingRParen) {
  EXPECT("fn f( -> void {}",
         "test.wgsl:1:7 error: expected ')' for function declaration\n"
         "fn f( -> void {}\n"
         "      ^^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingArrow) {
  EXPECT("fn f() void {}",
         "test.wgsl:1:8 error: missing -> for function declaration\n"
         "fn f() void {}\n"
         "       ^^^^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclInvalidReturnType) {
  EXPECT("fn f() -> 1 {}",
         "test.wgsl:1:11 error: unable to determine function return type\n"
         "fn f() -> 1 {}\n"
         "          ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclParamMissingColon) {
  EXPECT("fn f(x) -> void {}",
         "test.wgsl:1:7 error: missing : for identifier declaration\n"
         "fn f(x) -> void {}\n"
         "      ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclParamInvalidType) {
  EXPECT("fn f(x : 1) -> void {}",
         "test.wgsl:1:10 error: invalid type for identifier declaration\n"
         "fn f(x : 1) -> void {}\n"
         "         ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclParamMissing) {
  EXPECT("fn f(x : i32, ) -> void {}",
         "test.wgsl:1:13 error: found , but no variable declaration\n"
         "fn f(x : i32, ) -> void {}\n"
         "            ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingLBrace) {
  EXPECT("fn f() -> void }",
         "test.wgsl:1:16 error: expected '{'\n"
         "fn f() -> void }\n"
         "               ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingRBrace) {
  EXPECT("fn f() -> void {",
         "test.wgsl:1:17 error: expected '}'\n"
         "fn f() -> void {\n"
         "                ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstInvalidIdentifier) {
  EXPECT("const ^ : i32 = 1;",
         "test.wgsl:1:7 error: error parsing constant variable identifier\n"
         "const ^ : i32 = 1;\n"
         "      ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstMissingSemicolon) {
  EXPECT("const i : i32 = 1",
         "test.wgsl:1:18 error: expected ';' for constant declaration\n"
         "const i : i32 = 1\n"
         "                 ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstMissingLParen) {
  EXPECT("const i : vec2<i32> = vec2<i32>;",
         "test.wgsl:1:32 error: expected '(' for type constructor\n"
         "const i : vec2<i32> = vec2<i32>;\n"
         "                               ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstMissingRParen) {
  EXPECT("const i : vec2<i32> = vec2<i32>(1., 2.;",
         "test.wgsl:1:39 error: expected ')' for type constructor\n"
         "const i : vec2<i32> = vec2<i32>(1., 2.;\n"
         "                                      ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstMissingAssignment) {
  EXPECT("const i : vec2<i32>;",
         "test.wgsl:1:20 error: missing = for const declaration\n"
         "const i : vec2<i32>;\n"
         "                   ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstBadConstLiteral) {
  EXPECT("const i : vec2<i32> = vec2<i32>(!);",
         "test.wgsl:1:33 error: unable to parse const literal\n"
         "const i : vec2<i32> = vec2<i32>(!);\n"
         "                                ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstExprMaxDepth) {
  uint32_t kMaxConstExprDepth = 128;

  std::stringstream src;
  std::stringstream mkr;
  src << "const i : i32 = ";
  mkr << "                ";
  for (size_t i = 0; i < kMaxConstExprDepth + 8; i++) {
    src << "f32(";
    if (i < kMaxConstExprDepth + 1) {
      mkr << "    ";
    } else if (i == kMaxConstExprDepth + 1) {
      mkr << "^^^";
    }
  }
  src << "1.0";
  for (size_t i = 0; i < 200; i++) {
    src << ")";
  }
  src << ";";
  std::stringstream err;
  err << "test.wgsl:1:533 error: max const_expr depth reached\n"
      << src.str() << "\n"
      << mkr.str() << "\n";
  EXPECT(src.str().c_str(), err.str().c_str());
}

TEST_F(ParserImplErrorTest, GlobalDeclConstExprMissingLParen) {
  EXPECT("const i : vec2<i32> = vec2<i32> 1, 2);",
         "test.wgsl:1:33 error: expected '(' for type constructor\n"
         "const i : vec2<i32> = vec2<i32> 1, 2);\n"
         "                                ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstExprMissingRParen) {
  EXPECT("const i : vec2<i32> = vec2<i32>(1, 2;",
         "test.wgsl:1:37 error: expected ')' for type constructor\n"
         "const i : vec2<i32> = vec2<i32>(1, 2;\n"
         "                                    ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclSampledTextureMissingLessThan_Old) {
  EXPECT("var x : texture_sampled_1d;",
         "test.wgsl:1:28 error: missing '<' for sampled texture type\n"
         "var x : texture_sampled_1d;\n"
         "                           ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclSampledTextureMissingGreaterThan_Old) {
  EXPECT("var x : texture_sampled_1d<f32;",
         "test.wgsl:1:32 error: missing '>' for sampled texture type\n"
         "var x : texture_sampled_1d<f32;\n"
         "                               ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclSampledTextureInvalidSubtype_Old) {
  EXPECT("var x : texture_sampled_1d<1>;",
         "test.wgsl:1:28 error: invalid subtype for sampled texture type\n"
         "var x : texture_sampled_1d<1>;\n"
         "                           ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclSampledTextureMissingLessThan) {
  EXPECT("var x : texture_1d;",
         "test.wgsl:1:20 error: missing '<' for sampled texture type\n"
         "var x : texture_1d;\n"
         "                   ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclSampledTextureMissingGreaterThan) {
  EXPECT("var x : texture_1d<f32;",
         "test.wgsl:1:24 error: missing '>' for sampled texture type\n"
         "var x : texture_1d<f32;\n"
         "                       ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclSampledTextureInvalidSubtype) {
  EXPECT("var x : texture_1d<1>;",
         "test.wgsl:1:20 error: invalid subtype for sampled texture type\n"
         "var x : texture_1d<1>;\n"
         "                   ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclMultisampledTextureMissingLessThan) {
  EXPECT("var x : texture_multisampled_2d;",
         "test.wgsl:1:33 error: missing '<' for multisampled texture type\n"
         "var x : texture_multisampled_2d;\n"
         "                                ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclMultisampledTextureMissingGreaterThan) {
  EXPECT("var x : texture_multisampled_2d<f32;",
         "test.wgsl:1:37 error: missing '>' for multisampled texture type\n"
         "var x : texture_multisampled_2d<f32;\n"
         "                                    ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclMultisampledTextureInvalidSubtype) {
  EXPECT("var x : texture_multisampled_2d<1>;",
         "test.wgsl:1:33 error: invalid subtype for multisampled texture type\n"
         "var x : texture_multisampled_2d<1>;\n"
         "                                ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStorageTextureMissingLessThan) {
  EXPECT("var x : texture_ro_2d;",
         "test.wgsl:1:23 error: missing '<' for storage texture type\n"
         "var x : texture_ro_2d;\n"
         "                      ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStorageTextureMissingGreaterThan) {
  EXPECT("var x : texture_ro_2d<r8uint;",
         "test.wgsl:1:30 error: missing '>' for storage texture type\n"
         "var x : texture_ro_2d<r8uint;\n"
         "                             ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStorageTextureMissingInvalidSubtype) {
  EXPECT("var x : texture_ro_2d<1>;",
         "test.wgsl:1:23 error: invalid format for storage texture type\n"
         "var x : texture_ro_2d<1>;\n"
         "                      ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDecoMissingStruct) {
  EXPECT("[[block]];",
         "test.wgsl:1:10 error: missing struct declaration\n"
         "[[block]];\n"
         "         ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDecoMissingEnd) {
  EXPECT("[[block struct {};",
         "test.wgsl:1:9 error: missing ]] for struct decoration\n"
         "[[block struct {};\n"
         "        ^^^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDeclMissingIdentifier) {
  EXPECT("struct {};",
         "test.wgsl:1:8 error: expected identifier for struct declaration\n"
         "struct {};\n"
         "       ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDeclMissingSemicolon) {
  EXPECT("struct S {}",
         "test.wgsl:1:12 error: expected ';' for struct declaration\n"
         "struct S {}\n"
         "           ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDeclMissingLBrace) {
  EXPECT("struct S };",
         "test.wgsl:1:10 error: expected '{' for struct declaration\n"
         "struct S };\n"
         "         ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDeclMissingRBrace) {
  EXPECT("struct S { i : i32;",
         "test.wgsl:1:20 error: expected '}' for struct declaration\n"
         "struct S { i : i32;\n"
         "                   ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberDecoEmpty) {
  EXPECT("struct S { [[]] i : i32; };",
         "test.wgsl:1:14 error: empty struct member decoration found\n"
         "struct S { [[]] i : i32; };\n"
         "             ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberDecoMissingEnd) {
  EXPECT("struct S { [[ i : i32; };",
         "test.wgsl:1:15 error: missing ]] for struct member decoration\n"
         "struct S { [[ i : i32; };\n"
         "              ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberInvalidIdentifier) {
  EXPECT("struct S { 1 : i32; };",
         "test.wgsl:1:12 error: invalid identifier declaration\n"
         "struct S { 1 : i32; };\n"
         "           ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberMissingSemicolon) {
  EXPECT("struct S { i : i32 };",
         "test.wgsl:1:20 error: expected ';' for struct member\n"
         "struct S { i : i32 };\n"
         "                   ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberOffsetMissingLParen) {
  EXPECT("struct S { [[offset 1)]] i : i32 };",
         "test.wgsl:1:21 error: expected '(' for offset decoration\n"
         "struct S { [[offset 1)]] i : i32 };\n"
         "                    ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberOffsetMissingRParen) {
  EXPECT("struct S { [[offset(1]] i : i32 };",
         "test.wgsl:1:22 error: expected ')' for offset decoration\n"
         "struct S { [[offset(1]] i : i32 };\n"
         "                     ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberOffsetInvaldValue) {
  EXPECT("struct S { [[offset(x)]] i : i32 };",
         "test.wgsl:1:21 error: expected signed integer literal for offset "
         "decoration\n"
         "struct S { [[offset(x)]] i : i32 };\n"
         "                    ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberOffsetNegativeValue) {
  EXPECT("struct S { [[offset(-2)]] i : i32 };",
         "test.wgsl:1:21 error: offset decoration must be positive\n"
         "struct S { [[offset(-2)]] i : i32 };\n"
         "                    ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeAliasMissingIdentifier) {
  EXPECT("type 1 = f32;",
         "test.wgsl:1:6 error: expected identifier for type alias\n"
         "type 1 = f32;\n"
         "     ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeAliasInvalidType) {
  EXPECT("type meow = 1;",
         "test.wgsl:1:13 error: invalid type alias\n"
         "type meow = 1;\n"
         "            ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeAliasMissingAssignment) {
  EXPECT("type meow f32",
         "test.wgsl:1:11 error: expected '=' for type alias\n"
         "type meow f32\n"
         "          ^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeAliasMissingSemicolon) {
  EXPECT("type meow = f32",
         "test.wgsl:1:16 error: expected ';' for type alias\n"
         "type meow = f32\n"
         "               ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeInvalid) {
  EXPECT("var x : fish;",
         "test.wgsl:1:9 error: unknown constructed type 'fish'\n"
         "var x : fish;\n"
         "        ^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeDecoInvalid) {
  EXPECT("var x : [[]] i32;",
         "test.wgsl:1:9 error: invalid type for identifier declaration\n"
         "var x : [[]] i32;\n"
         "        ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayMissingLessThan) {
  EXPECT("var i : array;",
         "test.wgsl:1:14 error: expected '<' for array declaration\n"
         "var i : array;\n"
         "             ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayMissingGreaterThan) {
  EXPECT("var i : array<u32, 3;",
         "test.wgsl:1:21 error: expected '>' for array declaration\n"
         "var i : array<u32, 3;\n"
         "                    ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayDecoNotArray) {
  EXPECT("var i : [[stride(1)]] i32;",
         "test.wgsl:1:23 error: found array decoration but no array\n"
         "var i : [[stride(1)]] i32;\n"
         "                      ^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayDecoMissingEnd) {
  EXPECT("var i : [[stride(1) array<i32>;",
         "test.wgsl:1:21 error: expected ']]' for array decoration\n"
         "var i : [[stride(1) array<i32>;\n"
         "                    ^^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayDecoStrideMissingLParen) {
  EXPECT("var i : [[stride 1)]] array<i32>;",
         "test.wgsl:1:18 error: expected '(' for stride decoration\n"
         "var i : [[stride 1)]] array<i32>;\n"
         "                 ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayDecoStrideMissingRParen) {
  EXPECT("var i : [[stride(1]] array<i32>;",
         "test.wgsl:1:19 error: expected ')' for stride decoration\n"
         "var i : [[stride(1]] array<i32>;\n"
         "                  ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayDecoStrideInvalid) {
  EXPECT("var i : [[stride(x)]] array<i32>;",
         "test.wgsl:1:18 error: expected signed integer literal for stride "
         "decoration\n"
         "var i : [[stride(x)]] array<i32>;\n"
         "                 ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayDecoStrideNegative) {
  EXPECT("var i : [[stride(-1)]] array<i32>;",
         "test.wgsl:1:18 error: stride decoration must be greater than 0\n"
         "var i : [[stride(-1)]] array<i32>;\n"
         "                 ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayMissingType) {
  EXPECT("var i : array<1, 3>;",
         "test.wgsl:1:15 error: invalid type for array declaration\n"
         "var i : array<1, 3>;\n"
         "              ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayInvalidSize) {
  EXPECT(
      "var i : array<u32, x>;",
      "test.wgsl:1:20 error: expected signed integer literal for array size\n"
      "var i : array<u32, x>;\n"
      "                   ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayNegativeSize) {
  EXPECT("var i : array<u32, -3>;",
         "test.wgsl:1:20 error: array size must be greater than 0\n"
         "var i : array<u32, -3>;\n"
         "                   ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoListEmpty) {
  EXPECT("[[]] var i : i32;",
         "test.wgsl:1:3 error: empty variable decoration list\n"
         "[[]] var i : i32;\n"
         "  ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoListInvalid) {
  EXPECT("[[location(1), meow]] var i : i32;",
         "test.wgsl:1:16 error: missing variable decoration after comma\n"
         "[[location(1), meow]] var i : i32;\n"
         "               ^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoListMissingComma) {
  EXPECT("[[location(1) set(2)]] var i : i32;",
         "test.wgsl:1:15 error: missing comma in variable decoration list\n"
         "[[location(1) set(2)]] var i : i32;\n"
         "              ^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoListMissingEnd) {
  EXPECT("[[location(1) meow]] var i : i32;",
         "test.wgsl:1:15 error: missing ]] for variable decoration\n"
         "[[location(1) meow]] var i : i32;\n"
         "              ^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoLocationMissingLParen) {
  EXPECT("[[location 1]] var i : i32;",
         "test.wgsl:1:12 error: expected '(' for location decoration\n"
         "[[location 1]] var i : i32;\n"
         "           ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoLocationMissingRParen) {
  EXPECT("[[location (1]] var i : i32;",
         "test.wgsl:1:14 error: expected ')' for location decoration\n"
         "[[location (1]] var i : i32;\n"
         "             ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoLocationInvalidValue) {
  EXPECT("[[location(x)]] var i : i32;",
         "test.wgsl:1:12 error: expected signed integer literal for location "
         "decoration\n"
         "[[location(x)]] var i : i32;\n"
         "           ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBuiltinMissingLParen) {
  EXPECT("[[builtin position]] var i : i32;",
         "test.wgsl:1:11 error: expected '(' for builtin decoration\n"
         "[[builtin position]] var i : i32;\n"
         "          ^^^^^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBuiltinMissingRParen) {
  EXPECT("[[builtin(position]] var i : i32;",
         "test.wgsl:1:19 error: expected ')' for builtin decoration\n"
         "[[builtin(position]] var i : i32;\n"
         "                  ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBuiltinInvalidIdentifer) {
  EXPECT("[[builtin(1)]] var i : i32;",
         "test.wgsl:1:11 error: expected identifier for builtin\n"
         "[[builtin(1)]] var i : i32;\n"
         "          ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBuiltinInvalidValue) {
  EXPECT("[[builtin(x)]] var i : i32;",
         "test.wgsl:1:11 error: invalid value for builtin decoration\n"
         "[[builtin(x)]] var i : i32;\n"
         "          ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBindingMissingLParen) {
  EXPECT("[[binding 1]] var i : i32;",
         "test.wgsl:1:11 error: expected '(' for binding decoration\n"
         "[[binding 1]] var i : i32;\n"
         "          ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBindingMissingRParen) {
  EXPECT("[[binding(1]] var i : i32;",
         "test.wgsl:1:12 error: expected ')' for binding decoration\n"
         "[[binding(1]] var i : i32;\n"
         "           ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBindingInvalidValue) {
  EXPECT("[[binding(x)]] var i : i32;",
         "test.wgsl:1:11 error: expected signed integer literal for binding "
         "decoration\n"
         "[[binding(x)]] var i : i32;\n"
         "          ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoSetMissingLParen) {
  EXPECT("[[set 1]] var i : i32;",
         "test.wgsl:1:7 error: expected '(' for set decoration\n"
         "[[set 1]] var i : i32;\n"
         "      ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoSetMissingRParen) {
  EXPECT("[[set(1]] var i : i32;",
         "test.wgsl:1:8 error: expected ')' for set decoration\n"
         "[[set(1]] var i : i32;\n"
         "       ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBindingSetValue) {
  EXPECT("[[set(x)]] var i : i32;",
         "test.wgsl:1:7 error: expected signed integer literal for set "
         "decoration\n"
         "[[set(x)]] var i : i32;\n"
         "      ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarInvalidIdentifier) {
  EXPECT("var ^ : mat4x4;",
         "test.wgsl:1:5 error: invalid identifier declaration\n"
         "var ^ : mat4x4;\n"
         "    ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarMatrixMissingLessThan) {
  EXPECT("var i : mat4x4;",
         "test.wgsl:1:15 error: missing < for matrix\n"
         "var i : mat4x4;\n"
         "              ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarMatrixMissingGreaterThan) {
  EXPECT("var i : mat4x4<u32;",
         "test.wgsl:1:19 error: missing > for matrix\n"
         "var i : mat4x4<u32;\n"
         "                  ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarMatrixMissingType) {
  EXPECT("var i : mat4x4<1>;",
         "test.wgsl:1:16 error: unable to determine subtype for matrix\n"
         "var i : mat4x4<1>;\n"
         "               ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarMissingSemicolon) {
  EXPECT("var i : i32",
         "test.wgsl:1:12 error: expected ';' for variable declaration\n"
         "var i : i32\n"
         "           ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarPtrMissingLessThan) {
  EXPECT("var i : ptr;",
         "test.wgsl:1:12 error: missing < for ptr declaration\n"
         "var i : ptr;\n"
         "           ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarPtrMissingGreaterThan) {
  EXPECT("var i : ptr<in, u32;",
         "test.wgsl:1:20 error: missing > for ptr declaration\n"
         "var i : ptr<in, u32;\n"
         "                   ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarPtrMissingComma) {
  EXPECT("var i : ptr<in u32>;",
         "test.wgsl:1:16 error: missing , for ptr declaration\n"
         "var i : ptr<in u32>;\n"
         "               ^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarPtrMissingStorageClass) {
  EXPECT("var i : ptr<meow, u32>;",
         "test.wgsl:1:13 error: missing storage class for ptr declaration\n"
         "var i : ptr<meow, u32>;\n"
         "            ^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarPtrMissingType) {
  EXPECT("var i : ptr<in, 1>;",
         "test.wgsl:1:17 error: missing type for ptr declaration\n"
         "var i : ptr<in, 1>;\n"
         "                ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarStorageDeclInvalidClass) {
  EXPECT("var<fish> i : i32",
         "test.wgsl:1:5 error: invalid storage class for variable decoration\n"
         "var<fish> i : i32\n"
         "    ^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarStorageDeclMissingGThan) {
  EXPECT("var<in i : i32",
         "test.wgsl:1:8 error: missing > for variable decoration\n"
         "var<in i : i32\n"
         "       ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarVectorMissingLessThan) {
  EXPECT("var i : vec3;",
         "test.wgsl:1:13 error: missing < for vector\n"
         "var i : vec3;\n"
         "            ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarVectorMissingGreaterThan) {
  EXPECT("var i : vec3<u32;",
         "test.wgsl:1:17 error: missing > for vector\n"
         "var i : vec3<u32;\n"
         "                ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarVectorMissingType) {
  EXPECT("var i : vec3<1>;",
         "test.wgsl:1:14 error: unable to determine subtype for vector\n"
         "var i : vec3<1>;\n"
         "             ^\n");
}

TEST_F(ParserImplErrorTest, IfStmtMissingLParen) {
  EXPECT("fn f() -> void { if true) {} }",
         "test.wgsl:1:21 error: expected '('\n"
         "fn f() -> void { if true) {} }\n"
         "                    ^^^^\n");
}

TEST_F(ParserImplErrorTest, IfStmtMissingRParen) {
  EXPECT("fn f() -> void { if (true {} }",
         "test.wgsl:1:27 error: expected ')'\n"
         "fn f() -> void { if (true {} }\n"
         "                          ^\n");
}

TEST_F(ParserImplErrorTest, IfStmtInvalidCond) {
  EXPECT("fn f() -> void { if (>) {} }",
         "test.wgsl:1:22 error: unable to parse expression\n"
         "fn f() -> void { if (>) {} }\n"
         "                     ^\n");
}

TEST_F(ParserImplErrorTest, LogicalAndInvalidExpr) {
  EXPECT("fn f() -> void { return 1 && >; }",
         "test.wgsl:1:30 error: unable to parse right side of && expression\n"
         "fn f() -> void { return 1 && >; }\n"
         "                             ^\n");
}

TEST_F(ParserImplErrorTest, LogicalOrInvalidExpr) {
  EXPECT("fn f() -> void { return 1 || >; }",
         "test.wgsl:1:30 error: unable to parse right side of || expression\n"
         "fn f() -> void { return 1 || >; }\n"
         "                             ^\n");
}

TEST_F(ParserImplErrorTest, LoopMissingLBrace) {
  EXPECT("fn f() -> void { loop } }",
         "test.wgsl:1:23 error: expected '{' for loop\n"
         "fn f() -> void { loop } }\n"
         "                      ^\n");
}

TEST_F(ParserImplErrorTest, LoopMissingRBrace) {
  EXPECT("fn f() -> void { loop {",
         "test.wgsl:1:24 error: expected '}' for loop\n"
         "fn f() -> void { loop {\n"
         "                       ^\n");
}

TEST_F(ParserImplErrorTest, MemberExprMissingIdentifier) {
  EXPECT("fn f() -> void { x = a.; }",
         "test.wgsl:1:24 error: expected identifier for member accessor\n"
         "fn f() -> void { x = a.; }\n"
         "                       ^\n");
}

TEST_F(ParserImplErrorTest, MultiplicativeInvalidExpr) {
  EXPECT("fn f() -> void { return 1.0 * <; }",
         "test.wgsl:1:31 error: unable to parse right side of * expression\n"
         "fn f() -> void { return 1.0 * <; }\n"
         "                              ^\n");
}

TEST_F(ParserImplErrorTest, OrInvalidExpr) {
  EXPECT("fn f() -> void { return 1 | >; }",
         "test.wgsl:1:29 error: unable to parse right side of | expression\n"
         "fn f() -> void { return 1 | >; }\n"
         "                            ^\n");
}

TEST_F(ParserImplErrorTest, RelationalInvalidExpr) {
  EXPECT("fn f() -> void { return 1 < >; }",
         "test.wgsl:1:29 error: unable to parse right side of < expression\n"
         "fn f() -> void { return 1 < >; }\n"
         "                            ^\n");
}

TEST_F(ParserImplErrorTest, ReturnStmtMissingSemicolon) {
  EXPECT("fn f() -> void { return }",
         "test.wgsl:1:25 error: expected ';' for return statement\n"
         "fn f() -> void { return }\n"
         "                        ^\n");
}

TEST_F(ParserImplErrorTest, ShiftInvalidExpr) {
  EXPECT("fn f() -> void { return 1 << >; }",
         "test.wgsl:1:30 error: unable to parse right side of << expression\n"
         "fn f() -> void { return 1 << >; }\n"
         "                             ^\n");
}

TEST_F(ParserImplErrorTest, SwitchStmtMissingLBrace) {
  EXPECT("fn f() -> void { switch(1) }",
         "test.wgsl:1:28 error: expected '{' for switch statement\n"
         "fn f() -> void { switch(1) }\n"
         "                           ^\n");
}

TEST_F(ParserImplErrorTest, SwitchStmtMissingRBrace) {
  EXPECT("fn f() -> void { switch(1) {",
         "test.wgsl:1:29 error: expected '}' for switch statement\n"
         "fn f() -> void { switch(1) {\n"
         "                            ^\n");
}

TEST_F(ParserImplErrorTest, SwitchStmtInvalidCase) {
  EXPECT("fn f() -> void { switch(1) { case ^: } }",
         "test.wgsl:1:35 error: unable to parse case selectors\n"
         "fn f() -> void { switch(1) { case ^: } }\n"
         "                                  ^\n");
}

TEST_F(ParserImplErrorTest, SwitchStmtInvalidCase2) {
  EXPECT(
      "fn f() -> void { switch(1) { case false: } }",
      "test.wgsl:1:35 error: invalid case selector must be an integer value\n"
      "fn f() -> void { switch(1) { case false: } }\n"
      "                                  ^^^^^\n");
}

TEST_F(ParserImplErrorTest, SwitchStmtCaseMissingColon) {
  EXPECT("fn f() -> void { switch(1) { case 1 {} } }",
         "test.wgsl:1:37 error: expected ':' for case statement\n"
         "fn f() -> void { switch(1) { case 1 {} } }\n"
         "                                    ^\n");
}

TEST_F(ParserImplErrorTest, SwitchStmtCaseMissingLBrace) {
  EXPECT("fn f() -> void { switch(1) { case 1: } } }",
         "test.wgsl:1:38 error: expected '{' for case statement\n"
         "fn f() -> void { switch(1) { case 1: } } }\n"
         "                                     ^\n");
}

TEST_F(ParserImplErrorTest, SwitchStmtCaseMissingRBrace) {
  EXPECT("fn f() -> void { switch(1) { case 1: {",
         "test.wgsl:1:39 error: expected '}' for case statement\n"
         "fn f() -> void { switch(1) { case 1: {\n"
         "                                      ^\n");
}

TEST_F(ParserImplErrorTest, SwitchStmtCaseFallthroughMissingSemicolon) {
  EXPECT("fn f() -> void { switch(1) { case 1: { fallthrough } case 2: {} } }",
         "test.wgsl:1:52 error: expected ';' for fallthrough statement\n"
         "fn f() -> void { switch(1) { case 1: { fallthrough } case 2: {} } }\n"
         "                                                   ^\n");
}

TEST_F(ParserImplErrorTest, VarStmtMissingSemicolon) {
  EXPECT("fn f() -> void { var a : u32 }",
         "test.wgsl:1:30 error: expected ';' for variable declaration\n"
         "fn f() -> void { var a : u32 }\n"
         "                             ^\n");
}

TEST_F(ParserImplErrorTest, VarStmtInvalidAssignment) {
  EXPECT("fn f() -> void { var a : u32 = >; }",
         "test.wgsl:1:32 error: missing constructor for variable declaration\n"
         "fn f() -> void { var a : u32 = >; }\n"
         "                               ^\n");
}

TEST_F(ParserImplErrorTest, UnaryInvalidExpr) {
  EXPECT("fn f() -> void { return !<; }",
         "test.wgsl:1:26 error: unable to parse right side of ! expression\n"
         "fn f() -> void { return !<; }\n"
         "                         ^\n");
}

TEST_F(ParserImplErrorTest, UnexpectedToken) {
  EXPECT("unexpected",
         "test.wgsl:1:1 error: invalid token\n"
         "unexpected\n"
         "^^^^^^^^^^\n");
}

TEST_F(ParserImplErrorTest, XorInvalidExpr) {
  EXPECT("fn f() -> void { return 1 ^ >; }",
         "test.wgsl:1:29 error: unable to parse right side of ^ expression\n"
         "fn f() -> void { return 1 ^ >; }\n"
         "                            ^\n");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
