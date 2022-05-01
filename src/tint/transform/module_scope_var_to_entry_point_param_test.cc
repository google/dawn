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

#include "src/tint/transform/module_scope_var_to_entry_point_param.h"

#include <utility>

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using ModuleScopeVarToEntryPointParamTest = TransformTest;

TEST_F(ModuleScopeVarToEntryPointParamTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<ModuleScopeVarToEntryPointParam>(src));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, ShouldRunHasGlobal) {
    auto* src = R"(
var<private> v : i32;
)";

    EXPECT_TRUE(ShouldRun<ModuleScopeVarToEntryPointParam>(src));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Basic) {
    auto* src = R"(
var<private> p : f32;
var<workgroup> w : f32;

@stage(compute) @workgroup_size(1)
fn main() {
  w = p;
}
)";

    auto* expect = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  @internal(disable_validation__ignore_storage_class) var<workgroup> tint_symbol : f32;
  @internal(disable_validation__ignore_storage_class) var<private> tint_symbol_1 : f32;
  tint_symbol = tint_symbol_1;
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Basic_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  w = p;
}

var<workgroup> w : f32;
var<private> p : f32;
)";

    auto* expect = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  @internal(disable_validation__ignore_storage_class) var<workgroup> tint_symbol : f32;
  @internal(disable_validation__ignore_storage_class) var<private> tint_symbol_1 : f32;
  tint_symbol = tint_symbol_1;
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, FunctionCalls) {
    auto* src = R"(
var<private> p : f32;
var<workgroup> w : f32;

fn no_uses() {
}

fn bar(a : f32, b : f32) {
  p = a;
  w = b;
}

fn foo(a : f32) {
  let b : f32 = 2.0;
  bar(a, b);
  no_uses();
}

@stage(compute) @workgroup_size(1)
fn main() {
  foo(1.0);
}
)";

    auto* expect = R"(
fn no_uses() {
}

fn bar(a : f32, b : f32, @internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol : ptr<private, f32>, @internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol_1 : ptr<workgroup, f32>) {
  *(tint_symbol) = a;
  *(tint_symbol_1) = b;
}

fn foo(a : f32, @internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol_2 : ptr<private, f32>, @internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol_3 : ptr<workgroup, f32>) {
  let b : f32 = 2.0;
  bar(a, b, tint_symbol_2, tint_symbol_3);
  no_uses();
}

@stage(compute) @workgroup_size(1)
fn main() {
  @internal(disable_validation__ignore_storage_class) var<private> tint_symbol_4 : f32;
  @internal(disable_validation__ignore_storage_class) var<workgroup> tint_symbol_5 : f32;
  foo(1.0, &(tint_symbol_4), &(tint_symbol_5));
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, FunctionCalls_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  foo(1.0);
}

fn foo(a : f32) {
  let b : f32 = 2.0;
  bar(a, b);
  no_uses();
}

fn no_uses() {
}

fn bar(a : f32, b : f32) {
  p = a;
  w = b;
}

var<private> p : f32;
var<workgroup> w : f32;
)";

    auto* expect = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  @internal(disable_validation__ignore_storage_class) var<private> tint_symbol : f32;
  @internal(disable_validation__ignore_storage_class) var<workgroup> tint_symbol_1 : f32;
  foo(1.0, &(tint_symbol), &(tint_symbol_1));
}

fn foo(a : f32, @internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol_2 : ptr<private, f32>, @internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol_3 : ptr<workgroup, f32>) {
  let b : f32 = 2.0;
  bar(a, b, tint_symbol_2, tint_symbol_3);
  no_uses();
}

fn no_uses() {
}

fn bar(a : f32, b : f32, @internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol_4 : ptr<private, f32>, @internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol_5 : ptr<workgroup, f32>) {
  *(tint_symbol_4) = a;
  *(tint_symbol_5) = b;
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Constructors) {
    auto* src = R"(
var<private> a : f32 = 1.0;
var<private> b : f32 = f32();

@stage(compute) @workgroup_size(1)
fn main() {
  let x : f32 = a + b;
}
)";

    auto* expect = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  @internal(disable_validation__ignore_storage_class) var<private> tint_symbol : f32 = 1.0;
  @internal(disable_validation__ignore_storage_class) var<private> tint_symbol_1 : f32 = f32();
  let x : f32 = (tint_symbol + tint_symbol_1);
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Constructors_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  let x : f32 = a + b;
}

var<private> b : f32 = f32();
var<private> a : f32 = 1.0;
)";

    auto* expect = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  @internal(disable_validation__ignore_storage_class) var<private> tint_symbol : f32 = 1.0;
  @internal(disable_validation__ignore_storage_class) var<private> tint_symbol_1 : f32 = f32();
  let x : f32 = (tint_symbol + tint_symbol_1);
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Pointers) {
    auto* src = R"(
var<private> p : f32;
var<workgroup> w : f32;

@stage(compute) @workgroup_size(1)
fn main() {
  let p_ptr : ptr<private, f32> = &p;
  let w_ptr : ptr<workgroup, f32> = &w;
  let x : f32 = *p_ptr + *w_ptr;
  *p_ptr = x;
}
)";

    auto* expect = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  @internal(disable_validation__ignore_storage_class) var<private> tint_symbol : f32;
  @internal(disable_validation__ignore_storage_class) var<workgroup> tint_symbol_1 : f32;
  let p_ptr : ptr<private, f32> = &(tint_symbol);
  let w_ptr : ptr<workgroup, f32> = &(tint_symbol_1);
  let x : f32 = (*(p_ptr) + *(w_ptr));
  *(p_ptr) = x;
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Pointers_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  let p_ptr : ptr<private, f32> = &p;
  let w_ptr : ptr<workgroup, f32> = &w;
  let x : f32 = *p_ptr + *w_ptr;
  *p_ptr = x;
}

var<workgroup> w : f32;
var<private> p : f32;
)";

    auto* expect = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  @internal(disable_validation__ignore_storage_class) var<private> tint_symbol : f32;
  @internal(disable_validation__ignore_storage_class) var<workgroup> tint_symbol_1 : f32;
  let p_ptr : ptr<private, f32> = &(tint_symbol);
  let w_ptr : ptr<workgroup, f32> = &(tint_symbol_1);
  let x : f32 = (*(p_ptr) + *(w_ptr));
  *(p_ptr) = x;
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, FoldAddressOfDeref) {
    auto* src = R"(
var<private> v : f32;

fn bar(p : ptr<private, f32>) {
  (*p) = 0.0;
}

fn foo() {
  bar(&v);
}

@stage(compute) @workgroup_size(1)
fn main() {
  foo();
}
)";

    auto* expect = R"(
fn bar(p : ptr<private, f32>) {
  *(p) = 0.0;
}

fn foo(@internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol : ptr<private, f32>) {
  bar(tint_symbol);
}

@stage(compute) @workgroup_size(1)
fn main() {
  @internal(disable_validation__ignore_storage_class) var<private> tint_symbol_1 : f32;
  foo(&(tint_symbol_1));
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, FoldAddressOfDeref_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  foo();
}

fn foo() {
  bar(&v);
}

fn bar(p : ptr<private, f32>) {
  (*p) = 0.0;
}

var<private> v : f32;
)";

    auto* expect = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  @internal(disable_validation__ignore_storage_class) var<private> tint_symbol : f32;
  foo(&(tint_symbol));
}

fn foo(@internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol_1 : ptr<private, f32>) {
  bar(tint_symbol_1);
}

fn bar(p : ptr<private, f32>) {
  *(p) = 0.0;
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Buffers_Basic) {
    auto* src = R"(
struct S {
  a : f32,
};

@group(0) @binding(0)
var<uniform> u : S;
@group(0) @binding(1)
var<storage> s : S;

@stage(compute) @workgroup_size(1)
fn main() {
  _ = u;
  _ = s;
}
)";

    auto* expect = R"(
struct S {
  a : f32,
}

@stage(compute) @workgroup_size(1)
fn main(@group(0) @binding(0) @internal(disable_validation__entry_point_parameter) @internal(disable_validation__ignore_storage_class) tint_symbol : ptr<uniform, S>, @group(0) @binding(1) @internal(disable_validation__entry_point_parameter) @internal(disable_validation__ignore_storage_class) tint_symbol_1 : ptr<storage, S>) {
  _ = *(tint_symbol);
  _ = *(tint_symbol_1);
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Buffers_Basic_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  _ = u;
  _ = s;
}

@group(0) @binding(0) var<uniform> u : S;
@group(0) @binding(1) var<storage> s : S;

struct S {
  a : f32,
};

)";

    auto* expect = R"(
@stage(compute) @workgroup_size(1)
fn main(@group(0) @binding(0) @internal(disable_validation__entry_point_parameter) @internal(disable_validation__ignore_storage_class) tint_symbol : ptr<uniform, S>, @group(0) @binding(1) @internal(disable_validation__entry_point_parameter) @internal(disable_validation__ignore_storage_class) tint_symbol_1 : ptr<storage, S>) {
  _ = *(tint_symbol);
  _ = *(tint_symbol_1);
}

struct S {
  a : f32,
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Buffer_RuntimeArray) {
    auto* src = R"(
@group(0) @binding(0)
var<storage> buffer : array<f32>;

@stage(compute) @workgroup_size(1)
fn main() {
  _ = buffer[0];
}
)";

    auto* expect = R"(
struct tint_symbol_1 {
  arr : array<f32>,
}

@stage(compute) @workgroup_size(1)
fn main(@group(0) @binding(0) @internal(disable_validation__entry_point_parameter) @internal(disable_validation__ignore_storage_class) tint_symbol : ptr<storage, tint_symbol_1>) {
  _ = (*(tint_symbol)).arr[0];
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Buffer_RuntimeArray_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  _ = buffer[0];
}

@group(0) @binding(0)
var<storage> buffer : array<f32>;
)";

    auto* expect = R"(
struct tint_symbol_1 {
  arr : array<f32>,
}

@stage(compute) @workgroup_size(1)
fn main(@group(0) @binding(0) @internal(disable_validation__entry_point_parameter) @internal(disable_validation__ignore_storage_class) tint_symbol : ptr<storage, tint_symbol_1>) {
  _ = (*(tint_symbol)).arr[0];
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Buffer_RuntimeArrayInsideFunction) {
    auto* src = R"(
@group(0) @binding(0)
var<storage> buffer : array<f32>;

fn foo() {
  _ = buffer[0];
}

@stage(compute) @workgroup_size(1)
fn main() {
  foo();
}
)";

    auto* expect = R"(
struct tint_symbol_2 {
  arr : array<f32>,
}

fn foo(@internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol : ptr<storage, array<f32>>) {
  _ = (*(tint_symbol))[0];
}

@stage(compute) @workgroup_size(1)
fn main(@group(0) @binding(0) @internal(disable_validation__entry_point_parameter) @internal(disable_validation__ignore_storage_class) tint_symbol_1 : ptr<storage, tint_symbol_2>) {
  foo(&((*(tint_symbol_1)).arr));
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Buffer_RuntimeArrayInsideFunction_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  foo();
}

fn foo() {
  _ = buffer[0];
}

@group(0) @binding(0) var<storage> buffer : array<f32>;
)";

    auto* expect = R"(
struct tint_symbol_1 {
  arr : array<f32>,
}

@stage(compute) @workgroup_size(1)
fn main(@group(0) @binding(0) @internal(disable_validation__entry_point_parameter) @internal(disable_validation__ignore_storage_class) tint_symbol : ptr<storage, tint_symbol_1>) {
  foo(&((*(tint_symbol)).arr));
}

fn foo(@internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol_2 : ptr<storage, array<f32>>) {
  _ = (*(tint_symbol_2))[0];
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Buffer_RuntimeArray_Alias) {
    auto* src = R"(
type myarray = array<f32>;

@group(0) @binding(0)
var<storage> buffer : myarray;

@stage(compute) @workgroup_size(1)
fn main() {
  _ = buffer[0];
}
)";

    auto* expect = R"(
struct tint_symbol_1 {
  arr : array<f32>,
}

type myarray = array<f32>;

@stage(compute) @workgroup_size(1)
fn main(@group(0) @binding(0) @internal(disable_validation__entry_point_parameter) @internal(disable_validation__ignore_storage_class) tint_symbol : ptr<storage, tint_symbol_1>) {
  _ = (*(tint_symbol)).arr[0];
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Buffer_RuntimeArray_Alias_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  _ = buffer[0];
}

@group(0) @binding(0) var<storage> buffer : myarray;

type myarray = array<f32>;
)";

    auto* expect = R"(
struct tint_symbol_1 {
  arr : array<f32>,
}

@stage(compute) @workgroup_size(1)
fn main(@group(0) @binding(0) @internal(disable_validation__entry_point_parameter) @internal(disable_validation__ignore_storage_class) tint_symbol : ptr<storage, tint_symbol_1>) {
  _ = (*(tint_symbol)).arr[0];
}

type myarray = array<f32>;
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Buffer_ArrayOfStruct) {
    auto* src = R"(
struct S {
  f : f32,
};

@group(0) @binding(0)
var<storage> buffer : array<S>;

@stage(compute) @workgroup_size(1)
fn main() {
  _ = buffer[0];
}
)";

    auto* expect = R"(
struct S {
  f : f32,
}

struct tint_symbol_1 {
  arr : array<S>,
}

@stage(compute) @workgroup_size(1)
fn main(@group(0) @binding(0) @internal(disable_validation__entry_point_parameter) @internal(disable_validation__ignore_storage_class) tint_symbol : ptr<storage, tint_symbol_1>) {
  _ = (*(tint_symbol)).arr[0];
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Buffer_ArrayOfStruct_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  _ = buffer[0];
}

@group(0) @binding(0) var<storage> buffer : array<S>;

struct S {
  f : f32,
};
)";

    auto* expect = R"(
struct S {
  f : f32,
}

struct tint_symbol_1 {
  arr : array<S>,
}

@stage(compute) @workgroup_size(1)
fn main(@group(0) @binding(0) @internal(disable_validation__entry_point_parameter) @internal(disable_validation__ignore_storage_class) tint_symbol : ptr<storage, tint_symbol_1>) {
  _ = (*(tint_symbol)).arr[0];
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Buffers_FunctionCalls) {
    auto* src = R"(
struct S {
  a : f32,
};

@group(0) @binding(0)
var<uniform> u : S;
@group(0) @binding(1)
var<storage> s : S;

fn no_uses() {
}

fn bar(a : f32, b : f32) {
  _ = u;
  _ = s;
}

fn foo(a : f32) {
  let b : f32 = 2.0;
  _ = u;
  bar(a, b);
  no_uses();
}

@stage(compute) @workgroup_size(1)
fn main() {
  foo(1.0);
}
)";

    auto* expect = R"(
struct S {
  a : f32,
}

fn no_uses() {
}

fn bar(a : f32, b : f32, @internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol : ptr<uniform, S>, @internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol_1 : ptr<storage, S>) {
  _ = *(tint_symbol);
  _ = *(tint_symbol_1);
}

fn foo(a : f32, @internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol_2 : ptr<uniform, S>, @internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol_3 : ptr<storage, S>) {
  let b : f32 = 2.0;
  _ = *(tint_symbol_2);
  bar(a, b, tint_symbol_2, tint_symbol_3);
  no_uses();
}

@stage(compute) @workgroup_size(1)
fn main(@group(0) @binding(0) @internal(disable_validation__entry_point_parameter) @internal(disable_validation__ignore_storage_class) tint_symbol_4 : ptr<uniform, S>, @group(0) @binding(1) @internal(disable_validation__entry_point_parameter) @internal(disable_validation__ignore_storage_class) tint_symbol_5 : ptr<storage, S>) {
  foo(1.0, tint_symbol_4, tint_symbol_5);
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Buffers_FunctionCalls_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  foo(1.0);
}

fn foo(a : f32) {
  let b : f32 = 2.0;
  _ = u;
  bar(a, b);
  no_uses();
}

fn no_uses() {
}

fn bar(a : f32, b : f32) {
  _ = u;
  _ = s;
}

struct S {
  a : f32,
};

@group(0) @binding(0)
var<uniform> u : S;
@group(0) @binding(1)
var<storage> s : S;
)";

    auto* expect = R"(
@stage(compute) @workgroup_size(1)
fn main(@group(0) @binding(0) @internal(disable_validation__entry_point_parameter) @internal(disable_validation__ignore_storage_class) tint_symbol : ptr<uniform, S>, @group(0) @binding(1) @internal(disable_validation__entry_point_parameter) @internal(disable_validation__ignore_storage_class) tint_symbol_1 : ptr<storage, S>) {
  foo(1.0, tint_symbol, tint_symbol_1);
}

fn foo(a : f32, @internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol_2 : ptr<uniform, S>, @internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol_3 : ptr<storage, S>) {
  let b : f32 = 2.0;
  _ = *(tint_symbol_2);
  bar(a, b, tint_symbol_2, tint_symbol_3);
  no_uses();
}

fn no_uses() {
}

fn bar(a : f32, b : f32, @internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol_4 : ptr<uniform, S>, @internal(disable_validation__ignore_storage_class) @internal(disable_validation__ignore_invalid_pointer_argument) tint_symbol_5 : ptr<storage, S>) {
  _ = *(tint_symbol_4);
  _ = *(tint_symbol_5);
}

struct S {
  a : f32,
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, HandleTypes_Basic) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;
@group(0) @binding(1) var s : sampler;

@stage(compute) @workgroup_size(1)
fn main() {
  _ = t;
  _ = s;
}
)";

    auto* expect = R"(
@stage(compute) @workgroup_size(1)
fn main(@group(0) @binding(0) @internal(disable_validation__entry_point_parameter) tint_symbol : texture_2d<f32>, @group(0) @binding(1) @internal(disable_validation__entry_point_parameter) tint_symbol_1 : sampler) {
  _ = tint_symbol;
  _ = tint_symbol_1;
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, HandleTypes_FunctionCalls) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;
@group(0) @binding(1) var s : sampler;

fn no_uses() {
}

fn bar(a : f32, b : f32) {
  _ = t;
  _ = s;
}

fn foo(a : f32) {
  let b : f32 = 2.0;
  _ = t;
  bar(a, b);
  no_uses();
}

@stage(compute) @workgroup_size(1)
fn main() {
  foo(1.0);
}
)";

    auto* expect = R"(
fn no_uses() {
}

fn bar(a : f32, b : f32, tint_symbol : texture_2d<f32>, tint_symbol_1 : sampler) {
  _ = tint_symbol;
  _ = tint_symbol_1;
}

fn foo(a : f32, tint_symbol_2 : texture_2d<f32>, tint_symbol_3 : sampler) {
  let b : f32 = 2.0;
  _ = tint_symbol_2;
  bar(a, b, tint_symbol_2, tint_symbol_3);
  no_uses();
}

@stage(compute) @workgroup_size(1)
fn main(@group(0) @binding(0) @internal(disable_validation__entry_point_parameter) tint_symbol_4 : texture_2d<f32>, @group(0) @binding(1) @internal(disable_validation__entry_point_parameter) tint_symbol_5 : sampler) {
  foo(1.0, tint_symbol_4, tint_symbol_5);
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, HandleTypes_FunctionCalls_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  foo(1.0);
}

fn foo(a : f32) {
  let b : f32 = 2.0;
  _ = t;
  bar(a, b);
  no_uses();
}

fn no_uses() {
}

fn bar(a : f32, b : f32) {
  _ = t;
  _ = s;
}

@group(0) @binding(0) var t : texture_2d<f32>;
@group(0) @binding(1) var s : sampler;
)";

    auto* expect = R"(
@stage(compute) @workgroup_size(1)
fn main(@group(0) @binding(0) @internal(disable_validation__entry_point_parameter) tint_symbol : texture_2d<f32>, @group(0) @binding(1) @internal(disable_validation__entry_point_parameter) tint_symbol_1 : sampler) {
  foo(1.0, tint_symbol, tint_symbol_1);
}

fn foo(a : f32, tint_symbol_2 : texture_2d<f32>, tint_symbol_3 : sampler) {
  let b : f32 = 2.0;
  _ = tint_symbol_2;
  bar(a, b, tint_symbol_2, tint_symbol_3);
  no_uses();
}

fn no_uses() {
}

fn bar(a : f32, b : f32, tint_symbol_4 : texture_2d<f32>, tint_symbol_5 : sampler) {
  _ = tint_symbol_4;
  _ = tint_symbol_5;
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, Matrix) {
    auto* src = R"(
var<workgroup> m : mat2x2<f32>;

@stage(compute) @workgroup_size(1)
fn main() {
  let x = m;
}
)";

    auto* expect = R"(
struct tint_symbol_2 {
  m : mat2x2<f32>,
}

@stage(compute) @workgroup_size(1)
fn main(@internal(disable_validation__entry_point_parameter) tint_symbol_1 : ptr<workgroup, tint_symbol_2>) {
  let tint_symbol : ptr<workgroup, mat2x2<f32>> = &((*(tint_symbol_1)).m);
  let x = *(tint_symbol);
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, NestedMatrix) {
    auto* src = R"(
struct S1 {
  m : mat2x2<f32>,
};
struct S2 {
  s : S1,
};
var<workgroup> m : array<S2, 4>;

@stage(compute) @workgroup_size(1)
fn main() {
  let x = m;
}
)";

    auto* expect = R"(
struct S1 {
  m : mat2x2<f32>,
}

struct S2 {
  s : S1,
}

struct tint_symbol_2 {
  m : array<S2, 4u>,
}

@stage(compute) @workgroup_size(1)
fn main(@internal(disable_validation__entry_point_parameter) tint_symbol_1 : ptr<workgroup, tint_symbol_2>) {
  let tint_symbol : ptr<workgroup, array<S2, 4u>> = &((*(tint_symbol_1)).m);
  let x = *(tint_symbol);
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that we do not duplicate a struct type used by multiple workgroup
// variables that are promoted to threadgroup memory arguments.
TEST_F(ModuleScopeVarToEntryPointParamTest, DuplicateThreadgroupArgumentTypes) {
    auto* src = R"(
struct S {
  m : mat2x2<f32>,
};

var<workgroup> a : S;

var<workgroup> b : S;

@stage(compute) @workgroup_size(1)
fn main() {
  let x = a;
  let y = b;
}
)";

    auto* expect = R"(
struct S {
  m : mat2x2<f32>,
}

struct tint_symbol_3 {
  a : S,
  b : S,
}

@stage(compute) @workgroup_size(1)
fn main(@internal(disable_validation__entry_point_parameter) tint_symbol_1 : ptr<workgroup, tint_symbol_3>) {
  let tint_symbol : ptr<workgroup, S> = &((*(tint_symbol_1)).a);
  let tint_symbol_2 : ptr<workgroup, S> = &((*(tint_symbol_1)).b);
  let x = *(tint_symbol);
  let y = *(tint_symbol_2);
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that we do not duplicate a struct type used by multiple workgroup
// variables that are promoted to threadgroup memory arguments.
TEST_F(ModuleScopeVarToEntryPointParamTest, DuplicateThreadgroupArgumentTypes_OutOfOrder) {
    auto* src = R"(
@stage(compute) @workgroup_size(1)
fn main() {
  let x = a;
  let y = b;
}

var<workgroup> a : S;
var<workgroup> b : S;

struct S {
  m : mat2x2<f32>,
};
)";

    auto* expect = R"(
struct S {
  m : mat2x2<f32>,
}

struct tint_symbol_3 {
  a : S,
  b : S,
}

@stage(compute) @workgroup_size(1)
fn main(@internal(disable_validation__entry_point_parameter) tint_symbol_1 : ptr<workgroup, tint_symbol_3>) {
  let tint_symbol : ptr<workgroup, S> = &((*(tint_symbol_1)).a);
  let tint_symbol_2 : ptr<workgroup, S> = &((*(tint_symbol_1)).b);
  let x = *(tint_symbol);
  let y = *(tint_symbol_2);
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, UnusedVariables) {
    auto* src = R"(
struct S {
  a : f32,
};

var<private> p : f32;
var<workgroup> w : f32;

@group(0) @binding(0)
var<uniform> ub : S;
@group(0) @binding(1)
var<storage> sb : S;

@group(0) @binding(2) var t : texture_2d<f32>;
@group(0) @binding(3) var s : sampler;

@stage(compute) @workgroup_size(1)
fn main() {
}
)";

    auto* expect = R"(
struct S {
  a : f32,
}

@stage(compute) @workgroup_size(1)
fn main() {
}
)";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ModuleScopeVarToEntryPointParamTest, EmtpyModule) {
    auto* src = "";

    auto got = Run<ModuleScopeVarToEntryPointParam>(src);

    EXPECT_EQ(src, str(got));
}

}  // namespace
}  // namespace tint::transform
