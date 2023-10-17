// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/replace_identifier.h"

#include <string>

#include "gtest/gtest.h"

#include "src/tint/fuzzers/tint_ast_fuzzer/mutator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/node_id_map.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/writer/writer.h"

namespace tint::fuzzers::ast_fuzzer {
namespace {

TEST(ReplaceIdentifierTest, NotApplicable_Simple) {
    std::string content = R"(
    fn main() {
      let a = 5;
      let c = 6;
      let b = a + 5;

      let d = vec2<i32>(1, 2);
      let e = d.x;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    const auto& main_fn_stmts = program.AST().Functions()[0]->body->statements;

    const auto* a_var = main_fn_stmts[0]->As<ast::VariableDeclStatement>()->variable;
    ASSERT_NE(a_var, nullptr);

    const auto* b_var = main_fn_stmts[2]->As<ast::VariableDeclStatement>()->variable;
    ASSERT_NE(b_var, nullptr);

    const auto* e_var = main_fn_stmts[4]->As<ast::VariableDeclStatement>()->variable;
    ASSERT_NE(e_var, nullptr);

    auto a_var_id = node_id_map.GetId(a_var);
    ASSERT_NE(a_var_id, 0);

    auto b_var_id = node_id_map.GetId(b_var);
    ASSERT_NE(b_var_id, 0);

    const auto* sum_expr = b_var->initializer->As<ast::BinaryExpression>();
    ASSERT_NE(sum_expr, nullptr);

    auto a_ident_id = node_id_map.GetId(sum_expr->lhs);
    ASSERT_NE(a_ident_id, 0);

    auto sum_expr_id = node_id_map.GetId(sum_expr);
    ASSERT_NE(sum_expr_id, 0);

    auto e_var_id = node_id_map.GetId(e_var);
    ASSERT_NE(e_var_id, 0);

    auto vec_member_access_id =
        node_id_map.GetId(e_var->initializer->As<ast::MemberAccessorExpression>()->member);
    ASSERT_NE(vec_member_access_id, 0);

    // use_id is invalid.
    EXPECT_FALSE(MutationReplaceIdentifier(0, a_var_id).IsApplicable(program, node_id_map));

    // use_id is not an identifier expression.
    EXPECT_FALSE(
        MutationReplaceIdentifier(sum_expr_id, a_var_id).IsApplicable(program, node_id_map));

    // use_id is an identifier but not a variable user.
    EXPECT_FALSE(MutationReplaceIdentifier(vec_member_access_id, a_var_id)
                     .IsApplicable(program, node_id_map));

    // replacement_id is invalid.
    EXPECT_FALSE(MutationReplaceIdentifier(a_ident_id, 0).IsApplicable(program, node_id_map));

    // replacement_id is not a variable.
    EXPECT_FALSE(
        MutationReplaceIdentifier(a_ident_id, sum_expr_id).IsApplicable(program, node_id_map));

    // Can't replace a variable with itself.
    EXPECT_FALSE(
        MutationReplaceIdentifier(a_ident_id, a_var_id).IsApplicable(program, node_id_map));

    // Replacement is not in scope.
    EXPECT_FALSE(
        MutationReplaceIdentifier(a_ident_id, b_var_id).IsApplicable(program, node_id_map));
    EXPECT_FALSE(
        MutationReplaceIdentifier(a_ident_id, e_var_id).IsApplicable(program, node_id_map));
}

TEST(ReplaceIdentifierTest, GlobalVarNotInScope) {
    // Can't use the global variable if it's not in scope.
    std::string shader = R"(
var<private> a: i32;

fn f() {
  a = 3;
}

var<private> b: i32;
)";
    Source::File file("test.wgsl", shader);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    auto use_id = node_id_map.GetId(
        program.AST().Functions()[0]->body->statements[0]->As<ast::AssignmentStatement>()->lhs);
    ASSERT_NE(use_id, 0);

    auto replacement_id = node_id_map.GetId(program.AST().GlobalVariables()[1]);
    ASSERT_NE(replacement_id, 0);

    ASSERT_FALSE(
        MutationReplaceIdentifier(use_id, replacement_id).IsApplicable(program, node_id_map));
}

TEST(ReplaceIdentifierTest, NotApplicable1) {
    // Can't replace `a` with `b` since the store type is wrong (the same storage
    // class though).
    std::string shader = R"(
var<private> a: i32;
var<private> b: u32;
fn f() {
  *&a = 4;
}
)";
    Source::File file("test.wgsl", shader);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    auto replacement_id = node_id_map.GetId(program.AST().GlobalVariables()[1]);
    ASSERT_NE(replacement_id, 0);

    auto use_id = node_id_map.GetId(program.AST()
                                        .Functions()[0]
                                        ->body->statements[0]
                                        ->As<ast::AssignmentStatement>()
                                        ->lhs->As<ast::UnaryOpExpression>()
                                        ->expr->As<ast::UnaryOpExpression>()
                                        ->expr);
    ASSERT_NE(use_id, 0);

    ASSERT_FALSE(
        MutationReplaceIdentifier(use_id, replacement_id).IsApplicable(program, node_id_map));
}

TEST(ReplaceIdentifierTest, NotApplicable2) {
    // Can't replace `a` with `b` since the store type is wrong (the storage
    // class is different though).
    std::string shader = R"(
var<private> a: i32;
fn f() {
  var b: u32;
  *&a = 4;
}
)";
    Source::File file("test.wgsl", shader);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    auto replacement_id = node_id_map.GetId(program.AST()
                                                .Functions()[0]
                                                ->body->statements[0]
                                                ->As<ast::VariableDeclStatement>()
                                                ->variable);
    ASSERT_NE(replacement_id, 0);

    auto use_id = node_id_map.GetId(program.AST()
                                        .Functions()[0]
                                        ->body->statements[1]
                                        ->As<ast::AssignmentStatement>()
                                        ->lhs->As<ast::UnaryOpExpression>()
                                        ->expr->As<ast::UnaryOpExpression>()
                                        ->expr);
    ASSERT_NE(use_id, 0);

    ASSERT_FALSE(
        MutationReplaceIdentifier(use_id, replacement_id).IsApplicable(program, node_id_map));
}

TEST(ReplaceIdentifierTest, NotApplicable3) {
    // Can't replace `a` with `b` since the latter is not a reference (the store
    // type is the same, though).
    std::string shader = R"(
var<private> a: i32;
fn f() {
  let b = 45;
  *&a = 4;
}
)";
    Source::File file("test.wgsl", shader);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    auto replacement_id = node_id_map.GetId(program.AST()
                                                .Functions()[0]
                                                ->body->statements[0]
                                                ->As<ast::VariableDeclStatement>()
                                                ->variable);
    ASSERT_NE(replacement_id, 0);

    auto use_id = node_id_map.GetId(program.AST()
                                        .Functions()[0]
                                        ->body->statements[1]
                                        ->As<ast::AssignmentStatement>()
                                        ->lhs->As<ast::UnaryOpExpression>()
                                        ->expr->As<ast::UnaryOpExpression>()
                                        ->expr);
    ASSERT_NE(use_id, 0);

    ASSERT_FALSE(
        MutationReplaceIdentifier(use_id, replacement_id).IsApplicable(program, node_id_map));
}

TEST(ReplaceIdentifierTest, NotApplicable4) {
    // Can't replace `a` with `b` since the latter is not a reference (the store
    // type is the same, though).
    std::string shader = R"(
var<private> a: i32;
fn f(b: i32) {
  *&a = 4;
}
)";
    Source::File file("test.wgsl", shader);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    auto replacement_id = node_id_map.GetId(program.AST().Functions()[0]->params[0]);
    ASSERT_NE(replacement_id, 0);

    auto use_id = node_id_map.GetId(program.AST()
                                        .Functions()[0]
                                        ->body->statements[0]
                                        ->As<ast::AssignmentStatement>()
                                        ->lhs->As<ast::UnaryOpExpression>()
                                        ->expr->As<ast::UnaryOpExpression>()
                                        ->expr);
    ASSERT_NE(use_id, 0);

    ASSERT_FALSE(
        MutationReplaceIdentifier(use_id, replacement_id).IsApplicable(program, node_id_map));
}

TEST(ReplaceIdentifierTest, NotApplicable5) {
    // Can't replace `a` with `b` since the latter has a wrong access mode
    // (`read` for uniform address space).
    std::string shader = R"(
struct S {
  a: i32
}

var<private> a: S;
@group(1) @binding(1) var<uniform> b: S;
fn f() {
  *&a = S(4);
}
)";
    Source::File file("test.wgsl", shader);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    auto replacement_id = node_id_map.GetId(program.AST().GlobalVariables()[1]);
    ASSERT_NE(replacement_id, 0);

    auto use_id = node_id_map.GetId(program.AST()
                                        .Functions()[0]
                                        ->body->statements[0]
                                        ->As<ast::AssignmentStatement>()
                                        ->lhs->As<ast::UnaryOpExpression>()
                                        ->expr->As<ast::UnaryOpExpression>()
                                        ->expr);
    ASSERT_NE(use_id, 0);

    ASSERT_FALSE(
        MutationReplaceIdentifier(use_id, replacement_id).IsApplicable(program, node_id_map));
}

TEST(ReplaceIdentifierTest, NotApplicable6) {
    // Can't replace `ptr_b` with `a` since the latter is not a pointer.
    std::string shader = R"(
struct S {
  a: i32
}

var<private> a: S;
@group(1) @binding(1) var<uniform> b: S;
fn f() {
  let ptr_b = &b;
  *&a = *ptr_b;
}
)";
    Source::File file("test.wgsl", shader);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    auto replacement_id = node_id_map.GetId(program.AST().GlobalVariables()[0]);
    ASSERT_NE(replacement_id, 0);

    auto use_id = node_id_map.GetId(program.AST()
                                        .Functions()[0]
                                        ->body->statements[1]
                                        ->As<ast::AssignmentStatement>()
                                        ->rhs->As<ast::UnaryOpExpression>()
                                        ->expr);
    ASSERT_NE(use_id, 0);

    ASSERT_FALSE(
        MutationReplaceIdentifier(use_id, replacement_id).IsApplicable(program, node_id_map));
}

TEST(ReplaceIdentifierTest, NotApplicable8) {
    // Can't replace `ptr_b` with `c` since the latter has a wrong access mode and
    // address space.
    std::string shader = R"(
struct S {
  a: i32
}

var<private> a: S;
@group(1) @binding(1) var<uniform> b: S;
@group(1) @binding(2) var<storage, read_write> c: S;
fn f() {
  let ptr_b = &b;
  *&a = *ptr_b;
}
)";
    Source::File file("test.wgsl", shader);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    auto replacement_id = node_id_map.GetId(program.AST().GlobalVariables()[2]);
    ASSERT_NE(replacement_id, 0);

    auto use_id = node_id_map.GetId(program.AST()
                                        .Functions()[0]
                                        ->body->statements[1]
                                        ->As<ast::AssignmentStatement>()
                                        ->rhs->As<ast::UnaryOpExpression>()
                                        ->expr);
    ASSERT_NE(use_id, 0);

    ASSERT_FALSE(
        MutationReplaceIdentifier(use_id, replacement_id).IsApplicable(program, node_id_map));
}

TEST(ReplaceIdentifierTest, NotApplicable9) {
    // Can't replace `b` with `e` since the latter is not a reference.
    std::string shader = R"(
struct S {
  a: i32
}

var<private> a: S;
const e = 3;
@group(1) @binding(1) var<uniform> b: S;
fn f() {
  *&a = *&b;
}
)";
    Source::File file("test.wgsl", shader);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    auto replacement_id = node_id_map.GetId(program.AST().GlobalVariables()[1]);
    ASSERT_NE(replacement_id, 0);

    auto use_id = node_id_map.GetId(program.AST()
                                        .Functions()[0]
                                        ->body->statements[0]
                                        ->As<ast::AssignmentStatement>()
                                        ->rhs->As<ast::UnaryOpExpression>()
                                        ->expr->As<ast::UnaryOpExpression>()
                                        ->expr);
    ASSERT_NE(use_id, 0);

    ASSERT_FALSE(
        MutationReplaceIdentifier(use_id, replacement_id).IsApplicable(program, node_id_map));
}

TEST(ReplaceIdentifierTest, NotApplicable10) {
    // Can't replace `b` with `e` since the latter has a wrong access mode.
    std::string shader = R"(
struct S {
  a: i32
}

var<private> a: S;
@group(0) @binding(0) var<storage, read_write> e: S;
@group(1) @binding(1) var<uniform> b: S;
fn f() {
  *&a = *&b;
}
)";
    Source::File file("test.wgsl", shader);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    auto replacement_id = node_id_map.GetId(program.AST().GlobalVariables()[1]);
    ASSERT_NE(replacement_id, 0);

    auto use_id = node_id_map.GetId(program.AST()
                                        .Functions()[0]
                                        ->body->statements[0]
                                        ->As<ast::AssignmentStatement>()
                                        ->rhs->As<ast::UnaryOpExpression>()
                                        ->expr->As<ast::UnaryOpExpression>()
                                        ->expr);
    ASSERT_NE(use_id, 0);

    ASSERT_FALSE(
        MutationReplaceIdentifier(use_id, replacement_id).IsApplicable(program, node_id_map));
}

TEST(ReplaceIdentifierTest, Applicable1) {
    // Can replace `a` with `b` (same address space).
    std::string shader = R"(
fn f() {
  var b : vec2<u32>;
  var a = vec2<u32>(34u, 45u);
  (*&a)[1] = 3u;
}
)";
    Source::File file("test.wgsl", shader);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    auto use_id = node_id_map.GetId(program.AST()
                                        .Functions()[0]
                                        ->body->statements[2]
                                        ->As<ast::AssignmentStatement>()
                                        ->lhs->As<ast::IndexAccessorExpression>()
                                        ->object->As<ast::UnaryOpExpression>()
                                        ->expr->As<ast::UnaryOpExpression>()
                                        ->expr);
    ASSERT_NE(use_id, 0);

    auto replacement_id = node_id_map.GetId(program.AST()
                                                .Functions()[0]
                                                ->body->statements[0]
                                                ->As<ast::VariableDeclStatement>()
                                                ->variable);
    ASSERT_NE(replacement_id, 0);

    ASSERT_TRUE(MaybeApplyMutation(program, MutationReplaceIdentifier(use_id, replacement_id),
                                   node_id_map, program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    wgsl::writer::Options options;
    auto result = wgsl::writer::Generate(program, options);
    ASSERT_TRUE(result) << result.Failure();

    std::string expected_shader = R"(fn f() {
  var b : vec2<u32>;
  var a = vec2<u32>(34u, 45u);
  (*(&(b)))[1] = 3u;
}
)";
    ASSERT_EQ(expected_shader, result->wgsl);
}

TEST(ReplaceIdentifierTest, Applicable2) {
    // Can replace `ptr_a` with `b` - the function parameter.
    std::string shader = R"(
fn f(b: ptr<function, vec2<u32>>) {
  var a = vec2<u32>(34u, 45u);
  let ptr_a = &a;
  (*ptr_a)[1] = 3u;
}
)";
    Source::File file("test.wgsl", shader);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    auto use_id = node_id_map.GetId(program.AST()
                                        .Functions()[0]
                                        ->body->statements[2]
                                        ->As<ast::AssignmentStatement>()
                                        ->lhs->As<ast::IndexAccessorExpression>()
                                        ->object->As<ast::UnaryOpExpression>()
                                        ->expr);
    ASSERT_NE(use_id, 0);

    auto replacement_id = node_id_map.GetId(program.AST().Functions()[0]->params[0]);
    ASSERT_NE(replacement_id, 0);

    ASSERT_TRUE(MaybeApplyMutation(program, MutationReplaceIdentifier(use_id, replacement_id),
                                   node_id_map, program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    wgsl::writer::Options options;
    auto result = wgsl::writer::Generate(program, options);
    ASSERT_TRUE(result) << result.Failure();

    std::string expected_shader = R"(fn f(b : ptr<function, vec2<u32>>) {
  var a = vec2<u32>(34u, 45u);
  let ptr_a = &(a);
  (*(b))[1] = 3u;
}
)";
    ASSERT_EQ(expected_shader, result->wgsl);
}

TEST(ReplaceIdentifierTest, NotApplicable12) {
    // Can't replace `a` with `b` (both are references with different storage
    // class).
    std::string shader = R"(
var<private> b : vec2<u32>;
fn f() {
  var a = vec2<u32>(34u, 45u);
  (*&a)[1] = 3u;
}
)";
    Source::File file("test.wgsl", shader);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    auto use_id = node_id_map.GetId(program.AST()
                                        .Functions()[0]
                                        ->body->statements[1]
                                        ->As<ast::AssignmentStatement>()
                                        ->lhs->As<ast::IndexAccessorExpression>()
                                        ->object->As<ast::UnaryOpExpression>()
                                        ->expr->As<ast::UnaryOpExpression>()
                                        ->expr);
    ASSERT_NE(use_id, 0);

    auto replacement_id = node_id_map.GetId(program.AST().GlobalVariables()[0]);
    ASSERT_NE(replacement_id, 0);

    ASSERT_FALSE(
        MutationReplaceIdentifier(use_id, replacement_id).IsApplicable(program, node_id_map));
}

TEST(ReplaceIdentifierTest, NotApplicable13) {
    // Can't replace `a` with `b` (both are references with different storage
    // class).
    std::string shader = R"(
var<private> b : vec2<u32>;
fn f() {
  var a = vec2<u32>(34u, 45u);
  let c = (*&a)[1];
}
)";
    Source::File file("test.wgsl", shader);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    auto use_id = node_id_map.GetId(program.AST()
                                        .Functions()[0]
                                        ->body->statements[1]
                                        ->As<ast::VariableDeclStatement>()
                                        ->variable->initializer->As<ast::IndexAccessorExpression>()
                                        ->object->As<ast::UnaryOpExpression>()
                                        ->expr->As<ast::UnaryOpExpression>()
                                        ->expr);
    ASSERT_NE(use_id, 0);

    auto replacement_id = node_id_map.GetId(program.AST().GlobalVariables()[0]);
    ASSERT_NE(replacement_id, 0);

    ASSERT_FALSE(
        MutationReplaceIdentifier(use_id, replacement_id).IsApplicable(program, node_id_map));
}

TEST(ReplaceIdentifierTest, NotApplicable14) {
    // Can't replace `ptr_a` with `ptr_b` (both are pointers with different
    // address space).
    std::string shader = R"(
var<private> b: vec2<u32>;
fn f() {
  var a = vec2<u32>(34u, 45u);
  let ptr_a = &a;
  let ptr_b = &b;
  let c = (*ptr_a)[1];
}
)";
    Source::File file("test.wgsl", shader);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    auto use_id = node_id_map.GetId(program.AST()
                                        .Functions()[0]
                                        ->body->statements[3]
                                        ->As<ast::VariableDeclStatement>()
                                        ->variable->initializer->As<ast::IndexAccessorExpression>()
                                        ->object->As<ast::UnaryOpExpression>()
                                        ->expr);
    ASSERT_NE(use_id, 0);

    auto replacement_id = node_id_map.GetId(program.AST()
                                                .Functions()[0]
                                                ->body->statements[2]
                                                ->As<ast::VariableDeclStatement>()
                                                ->variable);
    ASSERT_NE(replacement_id, 0);
    ASSERT_FALSE(
        MutationReplaceIdentifier(use_id, replacement_id).IsApplicable(program, node_id_map));
}

}  // namespace
}  // namespace tint::fuzzers::ast_fuzzer
