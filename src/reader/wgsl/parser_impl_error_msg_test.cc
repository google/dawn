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

class ParserImplErrorTest : public ParserImplTest {
 public:
  void expect(const char* source, const char* expected) {
    auto* p = parser(source);
    EXPECT_EQ(p->Parse(), false);
    EXPECT_EQ(p->has_error(), true);
    EXPECT_EQ(p->error(), expected);
  }
};

TEST_F(ParserImplErrorTest, AdditiveInvalidExpr) {
  expect("fn f() -> void { return 1.0 + <; }",
         "1:31: unable to parse right side of + expression");
}

TEST_F(ParserImplErrorTest, AndInvalidExpr) {
  expect("fn f() -> void { return 1 & >; }",
         "1:29: unable to parse right side of & expression");
}

TEST_F(ParserImplErrorTest, ArrayIndexExprInvalidExpr) {
  expect("fn f() -> void { x = y[^]; }",
         "1:24: unable to parse expression inside []");
}

TEST_F(ParserImplErrorTest, ArrayIndexExprMissingRBracket) {
  expect("fn f() -> void { x = y[1; }", "1:25: missing ] for array accessor");
}

TEST_F(ParserImplErrorTest, AssignmentStmtMissingAssignment) {
  expect("fn f() -> void { a; }", "1:19: missing = for assignment");
}

TEST_F(ParserImplErrorTest, AssignmentStmtMissingAssignment2) {
  expect("fn f() -> void { a : i32; }", "1:20: missing = for assignment");
}

TEST_F(ParserImplErrorTest, AssignmentStmtMissingSemicolon) {
  expect("fn f() -> void { a = 1 }", "1:24: missing ;");
}

TEST_F(ParserImplErrorTest, AssignmentStmtInvalidRHS) {
  expect("fn f() -> void { a = >; }",
         "1:22: unable to parse right side of assignment");
}

TEST_F(ParserImplErrorTest, BitcastExprMissingLessThan) {
  expect("fn f() -> void { x = bitcast(y); }",
         "1:29: missing < for bitcast expression");
}

TEST_F(ParserImplErrorTest, BitcastExprMissingGreaterThan) {
  expect("fn f() -> void { x = bitcast<u32(y); }",
         "1:33: missing > for bitcast expression");
}

TEST_F(ParserImplErrorTest, BitcastExprMissingType) {
  expect("fn f() -> void { x = bitcast<>(y); }",
         "1:30: missing type for bitcast expression");
}

TEST_F(ParserImplErrorTest, BreakStmtMissingSemicolon) {
  expect("fn f() -> void { loop { break } }", "1:31: missing ;");
}

TEST_F(ParserImplErrorTest, CallExprMissingRParen) {
  expect("fn f() -> void { x = f(1.; }", "1:26: missing ) for call expression");
}

TEST_F(ParserImplErrorTest, CallStmtMissingRParen) {
  expect("fn f() -> void { f(1.; }", "1:22: missing ) for call statement");
}

TEST_F(ParserImplErrorTest, CallStmtInvalidArgument0) {
  expect("fn f() -> void { f(<); }",
         "1:20: unable to parse argument expression");
}

TEST_F(ParserImplErrorTest, CallStmtInvalidArgument1) {
  expect("fn f() -> void { f(1.0, <); }",
         "1:25: unable to parse argument expression after comma");
}

TEST_F(ParserImplErrorTest, CallStmtMissingSemicolon) {
  expect("fn f() -> void { f() }", "1:22: missing ;");
}

TEST_F(ParserImplErrorTest, ConstructorExprMissingLParen) {
  expect("fn f() -> void { x = vec2<u32>1,2); }",
         "1:31: missing ( for type constructor");
}

TEST_F(ParserImplErrorTest, ConstructorExprMissingRParen) {
  expect("fn f() -> void { x = vec2<u32>(1,2; }",
         "1:35: missing ) for type constructor");
}

TEST_F(ParserImplErrorTest, ConstVarStmtInvalid) {
  expect("fn f() -> void { const >; }",
         "1:24: unable to parse variable declaration");
}

TEST_F(ParserImplErrorTest, ConstVarStmtMissingAssignment) {
  expect("fn f() -> void { const a : i32; }",
         "1:31: missing = for constant declaration");
}

TEST_F(ParserImplErrorTest, ConstVarStmtMissingConstructor) {
  expect("fn f() -> void { const a : i32 = >; }",
         "1:34: missing constructor for const declaration");
}

TEST_F(ParserImplErrorTest, ContinueStmtMissingSemicolon) {
  expect("fn f() -> void { loop { continue } }", "1:34: missing ;");
}

TEST_F(ParserImplErrorTest, DiscardStmtMissingSemicolon) {
  expect("fn f() -> void { discard }", "1:26: missing ;");
}

TEST_F(ParserImplErrorTest, EqualityInvalidExpr) {
  expect("fn f() -> void { return 1 == >; }",
         "1:30: unable to parse right side of == expression");
}

TEST_F(ParserImplErrorTest, ForLoopInitializerMissingSemicolon) {
  expect("fn f() -> void { for (var i : i32 = 0 i < 8; i=i+1) {} }",
         "1:39: missing ';' after initializer in for loop");
}

TEST_F(ParserImplErrorTest, ForLoopConditionMissingSemicolon) {
  expect("fn f() -> void { for (var i : i32 = 0; i < 8 i=i+1) {} }",
         "1:46: missing ';' after condition in for loop");
}

TEST_F(ParserImplErrorTest, ForLoopMissingLParen) {
  expect("fn f() -> void { for var i : i32 = 0; i < 8; i=i+1) {} }",
         "1:22: missing for loop (");
}

TEST_F(ParserImplErrorTest, ForLoopMissingRParen) {
  expect("fn f() -> void { for (var i : i32 = 0; i < 8; i=i+1 {} }",
         "1:53: missing for loop )");
}

TEST_F(ParserImplErrorTest, ForLoopMissingLBrace) {
  expect("fn f() -> void { for (var i : i32 = 0; i < 8; i=i+1) } }",
         "1:54: missing for loop {");
}

TEST_F(ParserImplErrorTest, ForLoopMissingRBrace) {
  expect("fn f() -> void { for (var i : i32 = 0; i < 8; i=i+1) {",
         "1:55: missing for loop }");
}

TEST_F(ParserImplErrorTest, FunctionDeclInvalid) {
  expect("[[stage(vertex)]] x;", "1:19: error parsing function declaration");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoMissingEnd) {
  expect("[[stage(vertex) fn f() -> void {}",
         "1:17: missing ]] for function decorations");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoStageMissingLParen) {
  expect("[[stage vertex]] fn f() -> void {}",
         "1:9: missing ( for stage decoration");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoStageMissingRParen) {
  expect("[[stage(vertex]] fn f() -> void {}",
         "1:15: missing ) for stage decoration");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoStageInvalid) {
  expect("[[stage(x)]] fn f() -> void {}",
         "1:9: invalid value for stage decoration");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeMissingLParen) {
  expect("[[workgroup_size 1]] fn f() -> void {}",
         "1:18: missing ( for workgroup_size");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeMissingRParen) {
  expect("[[workgroup_size(1]] fn f() -> void {}",
         "1:19: missing ) for workgroup_size");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeXInvalid) {
  expect("[[workgroup_size(x)]] fn f() -> void {}",
         "1:18: missing x value for workgroup_size");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeXNegative) {
  expect("[[workgroup_size(-1)]] fn f() -> void {}",
         "1:18: invalid value for workgroup_size x parameter");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeYInvalid) {
  expect("[[workgroup_size(1, x)]] fn f() -> void {}",
         "1:21: missing y value for workgroup_size");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeYNegative) {
  expect("[[workgroup_size(1, -1)]] fn f() -> void {}",
         "1:21: invalid value for workgroup_size y parameter");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeZInvalid) {
  expect("[[workgroup_size(1, 2, x)]] fn f() -> void {}",
         "1:24: missing z value for workgroup_size");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeZNegative) {
  expect("[[workgroup_size(1, 2, -1)]] fn f() -> void {}",
         "1:24: invalid value for workgroup_size z parameter");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingIdentifier) {
  expect("fn () -> void {}", "1:4: missing identifier for function");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingLParen) {
  expect("fn f) -> void {}", "1:5: missing ( for function declaration");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingRParen) {
  expect("fn f( -> void {}", "1:7: missing ) for function declaration");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingArrow) {
  expect("fn f() void {}", "1:8: missing -> for function declaration");
}

TEST_F(ParserImplErrorTest, FunctionDeclInvalidReturnType) {
  expect("fn f() -> 1 {}", "1:11: unable to determine function return type");
}

TEST_F(ParserImplErrorTest, FunctionDeclParamMissingColon) {
  expect("fn f(x) -> void {}", "1:7: missing : for identifier declaration");
}

TEST_F(ParserImplErrorTest, FunctionDeclParamInvalidType) {
  expect("fn f(x : 1) -> void {}",
         "1:10: invalid type for identifier declaration");
}

TEST_F(ParserImplErrorTest, FunctionDeclParamMissing) {
  expect("fn f(x : i32, ) -> void {}",
         "1:13: found , but no variable declaration");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingLBrace) {
  expect("fn f() -> void }", "1:16: missing {");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingRBrace) {
  expect("fn f() -> void {", "1:17: missing }");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstInvalidIdentifier) {
  expect("const ^ : i32 = 1;",
         "1:7: error parsing constant variable identifier");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstMissingSemicolon) {
  expect("const i : i32 = 1", "1:18: missing ';' for constant declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstMissingLParen) {
  expect("const i : vec2<i32> = vec2<i32>;",
         "1:32: missing ( for type constructor");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstMissingRParen) {
  expect("const i : vec2<i32> = vec2<i32>(1., 2.;",
         "1:39: missing ) for type constructor");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstMissingAssignment) {
  expect("const i : vec2<i32>;", "1:20: missing = for const declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstBadConstLiteral) {
  expect("const i : vec2<i32> = vec2<i32>(!);",
         "1:33: unable to parse const literal");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstExprMaxDepth) {
  std::stringstream out;
  out << "const i : i32 = ";
  for (size_t i = 0; i < 200; i++) {
    out << "f32(";
  }
  out << "1.0";
  for (size_t i = 0; i < 200; i++) {
    out << ")";
  }
  out << ";";
  expect(out.str().c_str(), "1:533: max const_expr depth reached");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstExprMissingLParen) {
  expect("const i : vec2<i32> = vec2<i32> 1, 2);",
         "1:33: missing ( for type constructor");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstExprMissingRParen) {
  expect("const i : vec2<i32> = vec2<i32>(1, 2;",
         "1:37: missing ) for type constructor");
}

TEST_F(ParserImplErrorTest, GlobalDeclSampledTextureMissingLessThan) {
  expect("var x : texture_sampled_1d;",
         "1:28: missing '<' for sampled texture type");
}

TEST_F(ParserImplErrorTest, GlobalDeclSampledTextureMissingGreaterThan) {
  expect("var x : texture_sampled_1d<f32;",
         "1:32: missing '>' for sampled texture type");
}

TEST_F(ParserImplErrorTest, GlobalDeclSampledTextureInvalidSubtype) {
  expect("var x : texture_sampled_1d<1>;",
         "1:28: invalid subtype for sampled texture type");
}

TEST_F(ParserImplErrorTest, GlobalDeclMultisampledTextureMissingLessThan) {
  expect("var x : texture_multisampled_2d;",
         "1:33: missing '<' for multisampled texture type");
}

TEST_F(ParserImplErrorTest, GlobalDeclMultisampledTextureMissingGreaterThan) {
  expect("var x : texture_multisampled_2d<f32;",
         "1:37: missing '>' for multisampled texture type");
}

TEST_F(ParserImplErrorTest, GlobalDeclMultisampledTextureInvalidSubtype) {
  expect("var x : texture_multisampled_2d<1>;",
         "1:33: invalid subtype for multisampled texture type");
}

TEST_F(ParserImplErrorTest, GlobalDeclStorageTextureMissingLessThan) {
  expect("var x : texture_ro_2d;",
         "1:23: missing '<' for storage texture type");
}

TEST_F(ParserImplErrorTest, GlobalDeclStorageTextureMissingGreaterThan) {
  expect("var x : texture_ro_2d<r8uint;",
         "1:30: missing '>' for storage texture type");
}

TEST_F(ParserImplErrorTest, GlobalDeclStorageTextureMissingInvalidSubtype) {
  expect("var x : texture_ro_2d<1>;",
         "1:23: invalid format for storage texture type");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDecoMissingStruct) {
  expect("[[block]];", "1:10: missing struct declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDecoMissingEnd) {
  expect("[[block struct {};", "1:9: missing ]] for struct decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDeclMissingIdentifier) {
  expect("struct {};", "1:8: missing identifier for struct declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDeclMissingSemicolon) {
  expect("struct S {}", "1:12: missing ';' for struct declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDeclMissingLBrace) {
  expect("struct S };", "1:10: missing { for struct declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDeclMissingRBrace) {
  expect("struct S { i : i32;", "1:20: missing } for struct declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberDecoEmpty) {
  expect("struct S { [[]] i : i32; };",
         "1:14: empty struct member decoration found");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberDecoMissingEnd) {
  expect("struct S { [[ i : i32; };",
         "1:15: missing ]] for struct member decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberInvalidIdentifier) {
  expect("struct S { 1 : i32; };", "1:12: invalid identifier declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberMissingSemicolon) {
  expect("struct S { i : i32 };", "1:20: missing ; for struct member");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberOffsetMissingLParen) {
  expect("struct S { [[offset 1)]] i : i32 };", "1:21: missing ( for offset");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberOffsetMissingRParen) {
  expect("struct S { [[offset(1]] i : i32 };", "1:22: missing ) for offset");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberOffsetInvaldValue) {
  expect("struct S { [[offset(x)]] i : i32 };",
         "1:21: invalid value for offset decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberOffsetNegativeValue) {
  expect("struct S { [[offset(-2)]] i : i32 };",
         "1:21: offset value must be >= 0");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeAliasMissingIdentifier) {
  expect("type 1 = f32;", "1:6: missing identifier for type alias");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeAliasInvalidType) {
  expect("type meow = 1;", "1:13: invalid type alias");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeAliasMissingAssignment) {
  expect("type meow f32", "1:11: missing = for type alias");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeAliasMissingSemicolon) {
  expect("type meow = f32", "1:16: missing ';' for type alias");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeInvalid) {
  expect("var x : fish;", "1:9: unknown constructed type 'fish'");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeDecoInvalid) {
  expect("var x : [[]] i32;", "1:9: invalid type for identifier declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayMissingLessThan) {
  expect("var i : array;", "1:14: missing < for array declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayMissingGreaterThan) {
  expect("var i : array<u32, 3;", "1:21: missing > for array declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayDecoNotArray) {
  expect("var i : [[stride(1)]] i32;",
         "1:23: found array decoration but no array");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayDecoMissingEnd) {
  expect("var i : [[stride(1) array<i32>;",
         "1:21: missing ]] for array decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayDecoStrideMissingLParen) {
  expect("var i : [[stride 1)]] array<i32>;",
         "1:18: missing ( for stride attribute");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayDecoStrideMissingRParen) {
  expect("var i : [[stride(1]] array<i32>;",
         "1:19: missing ) for stride attribute");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayDecoStrideInvalid) {
  expect("var i : [[stride(x)]] array<i32>;",
         "1:18: missing value for stride decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayDecoStrideNegative) {
  expect("var i : [[stride(-1)]] array<i32>;",
         "1:18: invalid stride value: -1");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayMissingType) {
  expect("var i : array<1, 3>;", "1:15: invalid type for array declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayInvalidSize) {
  expect("var i : array<u32, x>;", "1:20: missing size of array declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayNegativeSize) {
  expect("var i : array<u32, -3>;", "1:20: invalid size for array declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoListEmpty) {
  expect("[[]] var i : i32;", "1:3: empty variable decoration list");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoListInvalid) {
  expect("[[location(1), meow]] var i : i32;",
         "1:16: missing variable decoration after comma");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoListMissingComma) {
  expect("[[location(1) set(2)]] var i : i32;",
         "1:15: missing comma in variable decoration list");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoListMissingEnd) {
  expect("[[location(1) meow]] var i : i32;",
         "1:15: missing ]] for variable decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoLocationMissingLParen) {
  expect("[[location 1]] var i : i32;",
         "1:12: missing ( for location decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoLocationMissingRParen) {
  expect("[[location (1]] var i : i32;",
         "1:14: missing ) for location decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoLocationInvalidValue) {
  expect("[[location(x)]] var i : i32;",
         "1:12: invalid value for location decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBuiltinMissingLParen) {
  expect("[[builtin position]] var i : i32;",
         "1:11: missing ( for builtin decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBuiltinMissingRParen) {
  expect("[[builtin(position]] var i : i32;",
         "1:19: missing ) for builtin decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBuiltinInvalidIdentifer) {
  expect("[[builtin(1)]] var i : i32;",
         "1:11: expected identifier for builtin");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBuiltinInvalidValue) {
  expect("[[builtin(x)]] var i : i32;",
         "1:11: invalid value for builtin decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBindingMissingLParen) {
  expect("[[binding 1]] var i : i32;",
         "1:11: missing ( for binding decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBindingMissingRParen) {
  expect("[[binding(1]] var i : i32;",
         "1:12: missing ) for binding decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBindingInvalidValue) {
  expect("[[binding(x)]] var i : i32;",
         "1:11: invalid value for binding decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoSetMissingLParen) {
  expect("[[set 1]] var i : i32;", "1:7: missing ( for set decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoSetMissingRParen) {
  expect("[[set(1]] var i : i32;", "1:8: missing ) for set decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBindingSetValue) {
  expect("[[set(x)]] var i : i32;", "1:7: invalid value for set decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarInvalidIdentifier) {
  expect("var ^ : mat4x4;", "1:5: invalid identifier declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarMatrixMissingLessThan) {
  expect("var i : mat4x4;", "1:15: missing < for matrix");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarMatrixMissingGreaterThan) {
  expect("var i : mat4x4<u32;", "1:19: missing > for matrix");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarMatrixMissingType) {
  expect("var i : mat4x4<1>;", "1:16: unable to determine subtype for matrix");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarMissingSemicolon) {
  expect("var i : i32", "1:12: missing ';' for variable declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarPtrMissingLessThan) {
  expect("var i : ptr;", "1:12: missing < for ptr declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarPtrMissingGreaterThan) {
  expect("var i : ptr<in, u32;", "1:20: missing > for ptr declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarPtrMissingComma) {
  expect("var i : ptr<in u32>;", "1:16: missing , for ptr declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarPtrMissingStorageClass) {
  expect("var i : ptr<meow, u32>;",
         "1:13: missing storage class for ptr declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarPtrMissingType) {
  expect("var i : ptr<in, 1>;", "1:17: missing type for ptr declaration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarStorageDeclInvalidClass) {
  expect("var<fish> i : i32",
         "1:5: invalid storage class for variable decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarStorageDeclMissingGThan) {
  expect("var<in i : i32", "1:8: missing > for variable decoration");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarVectorMissingLessThan) {
  expect("var i : vec3;", "1:13: missing < for vector");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarVectorMissingGreaterThan) {
  expect("var i : vec3<u32;", "1:17: missing > for vector");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarVectorMissingType) {
  expect("var i : vec3<1>;", "1:14: unable to determine subtype for vector");
}

TEST_F(ParserImplErrorTest, IfStmtMissingLParen) {
  expect("fn f() -> void { if true) {} }", "1:21: expected (");
}

TEST_F(ParserImplErrorTest, IfStmtMissingRParen) {
  expect("fn f() -> void { if (true {} }", "1:27: expected )");
}

TEST_F(ParserImplErrorTest, IfStmtInvalidCond) {
  expect("fn f() -> void { if (>) {} }", "1:22: unable to parse expression");
}

TEST_F(ParserImplErrorTest, LogicalAndInvalidExpr) {
  expect("fn f() -> void { return 1 && >; }",
         "1:30: unable to parse right side of && expression");
}

TEST_F(ParserImplErrorTest, LogicalOrInvalidExpr) {
  expect("fn f() -> void { return 1 || >; }",
         "1:30: unable to parse right side of || expression");
}

TEST_F(ParserImplErrorTest, LoopMissingLBrace) {
  expect("fn f() -> void { loop } }", "1:23: missing { for loop");
}

TEST_F(ParserImplErrorTest, LoopMissingRBrace) {
  expect("fn f() -> void { loop {", "1:24: missing } for loop");
}

TEST_F(ParserImplErrorTest, MemberExprMissingIdentifier) {
  expect("fn f() -> void { x = a.; }",
         "1:24: missing identifier for member accessor");
}

TEST_F(ParserImplErrorTest, MultiplicativeInvalidExpr) {
  expect("fn f() -> void { return 1.0 * <; }",
         "1:31: unable to parse right side of * expression");
}

TEST_F(ParserImplErrorTest, OrInvalidExpr) {
  expect("fn f() -> void { return 1 | >; }",
         "1:29: unable to parse right side of | expression");
}

TEST_F(ParserImplErrorTest, RelationalInvalidExpr) {
  expect("fn f() -> void { return 1 < >; }",
         "1:29: unable to parse right side of < expression");
}

TEST_F(ParserImplErrorTest, ReturnStmtMissingSemicolon) {
  expect("fn f() -> void { return }", "1:25: missing ;");
}

TEST_F(ParserImplErrorTest, ShiftInvalidExpr) {
  expect("fn f() -> void { return 1 << >; }",
         "1:30: unable to parse right side of << expression");
}

TEST_F(ParserImplErrorTest, SwitchStmtMissingLBrace) {
  expect("fn f() -> void { switch(1) }",
         "1:28: missing { for switch statement");
}

TEST_F(ParserImplErrorTest, SwitchStmtMissingRBrace) {
  expect("fn f() -> void { switch(1) {",
         "1:29: missing } for switch statement");
}

TEST_F(ParserImplErrorTest, SwitchStmtInvalidCase) {
  expect("fn f() -> void { switch(1) { case ^: } }",
         "1:35: unable to parse case selectors");
}

TEST_F(ParserImplErrorTest, SwitchStmtInvalidCase2) {
  expect("fn f() -> void { switch(1) { case false: } }",
         "1:35: invalid case selector must be an integer value");
}

TEST_F(ParserImplErrorTest, SwitchStmtCaseMissingColon) {
  expect("fn f() -> void { switch(1) { case 1 {} } }",
         "1:37: missing : for case statement");
}

TEST_F(ParserImplErrorTest, SwitchStmtCaseMissingLBrace) {
  expect("fn f() -> void { switch(1) { case 1: } } }",
         "1:38: missing { for case statement");
}

TEST_F(ParserImplErrorTest, SwitchStmtCaseMissingRBrace) {
  expect("fn f() -> void { switch(1) { case 1: {",
         "1:39: missing } for case statement");
}

TEST_F(ParserImplErrorTest, SwitchStmtCaseFallthroughMissingSemicolon) {
  expect("fn f() -> void { switch(1) { case 1: { fallthrough } case 2: {} } }",
         "1:52: missing ;");
}

TEST_F(ParserImplErrorTest, VarStmtMissingSemicolon) {
  expect("fn f() -> void { var a : u32 }", "1:30: missing ;");
}

TEST_F(ParserImplErrorTest, VarStmtInvalidAssignment) {
  expect("fn f() -> void { var a : u32 = >; }",
         "1:32: missing constructor for variable declaration");
}

TEST_F(ParserImplErrorTest, UnaryInvalidExpr) {
  expect("fn f() -> void { return !<; }",
         "1:26: unable to parse right side of ! expression");
}

TEST_F(ParserImplErrorTest, UnexpectedToken) {
  expect("unexpected", "1:1: invalid token (kIdentifier) encountered");
}

TEST_F(ParserImplErrorTest, XorInvalidExpr) {
  expect("fn f() -> void { return 1 ^ >; }",
         "1:29: unable to parse right side of ^ expression");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
