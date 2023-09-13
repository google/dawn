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

#include "src/tint/lang/msl/writer/ast_raise/pixel_local.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::msl::writer {
namespace {

struct Binding {
    uint32_t field_index;
    uint32_t attachment_index;
};

ast::transform::DataMap Bindings(std::initializer_list<Binding> bindings) {
    PixelLocal::Config cfg;
    for (auto& binding : bindings) {
        cfg.attachments.Add(binding.field_index, binding.attachment_index);
    }
    ast::transform::DataMap data;
    data.Add<PixelLocal::Config>(std::move(cfg));
    return data;
}

using PixelLocalTest = ast::transform::TransformTest;

TEST_F(PixelLocalTest, EmptyModule) {
    auto* src = "";

    EXPECT_FALSE(ShouldRun<PixelLocal>(src, Bindings({})));
}

TEST_F(PixelLocalTest, Var) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : i32,
};

var<pixel_local> P : PixelLocal;
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : i32,
}

var<private> P : PixelLocal;
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(PixelLocalTest, UseInEntryPoint) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn F() {
  P.a += 42;
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

struct F_res {
  @location(1)
  output_0 : u32,
}

@fragment
fn F(pixel_local_1 : PixelLocal) -> F_res {
  P = pixel_local_1;
  F_inner();
  return F_res(P.a);
}

struct PixelLocal {
  @internal(attachment(1)) @internal(disable_validation__entry_point_parameter)
  a : u32,
}

var<private> P : PixelLocal;

fn F_inner() {
  P.a += 42;
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(PixelLocalTest, UseInCallee) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

fn X() {
  P.a += 42;
}

@fragment
fn F() {
  X();
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

struct F_res {
  @location(1)
  output_0 : u32,
}

@fragment
fn F(pixel_local_1 : PixelLocal) -> F_res {
  P = pixel_local_1;
  F_inner();
  return F_res(P.a);
}

struct PixelLocal {
  @internal(attachment(1)) @internal(disable_validation__entry_point_parameter)
  a : u32,
}

var<private> P : PixelLocal;

fn X() {
  P.a += 42;
}

fn F_inner() {
  X();
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(PixelLocalTest, MultipleAttachments) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn F() {
  P.a = 42;
  P.b = i32(P.c);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

struct F_res {
  @location(1)
  output_0 : u32,
  @location(0)
  output_1 : i32,
  @location(10)
  output_2 : f32,
}

@fragment
fn F(pixel_local_1 : PixelLocal) -> F_res {
  P = pixel_local_1;
  F_inner();
  return F_res(P.a, P.b, P.c);
}

struct PixelLocal {
  @internal(attachment(1)) @internal(disable_validation__entry_point_parameter)
  a : u32,
  @internal(attachment(0)) @internal(disable_validation__entry_point_parameter)
  b : i32,
  @internal(attachment(10)) @internal(disable_validation__entry_point_parameter)
  c : f32,
}

var<private> P : PixelLocal;

fn F_inner() {
  P.a = 42;
  P.b = i32(P.c);
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1}, {1, 0}, {2, 10}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(PixelLocalTest, WithBuiltinInputParameter) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn F(@builtin(position) pos : vec4f) {
  P.a += u32(pos.x);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

struct F_res {
  @location(1)
  output_0 : u32,
}

@fragment
fn F(@builtin(position) pos : vec4f, pixel_local_1 : PixelLocal) -> F_res {
  P = pixel_local_1;
  F_inner(pos);
  return F_res(P.a);
}

struct PixelLocal {
  @internal(attachment(1)) @internal(disable_validation__entry_point_parameter)
  a : u32,
}

var<private> P : PixelLocal;

fn F_inner(pos : vec4f) {
  P.a += u32(pos.x);
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(PixelLocalTest, WithInvariantBuiltinInputParameter) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn F(@invariant @builtin(position) pos : vec4f) {
  P.a += u32(pos.x);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

struct F_res {
  @location(1)
  output_0 : u32,
}

@fragment
fn F(@invariant @builtin(position) pos : vec4f, pixel_local_1 : PixelLocal) -> F_res {
  P = pixel_local_1;
  F_inner(pos);
  return F_res(P.a);
}

struct PixelLocal {
  @internal(attachment(1)) @internal(disable_validation__entry_point_parameter)
  a : u32,
}

var<private> P : PixelLocal;

fn F_inner(pos : vec4f) {
  P.a += u32(pos.x);
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(PixelLocalTest, WithBuiltinInputStructParameter) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

struct In {
  @builtin(position) pos : vec4f,
}

@fragment
fn F(in : In) {
  P.a += u32(in.pos.x);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

struct F_res {
  @location(1)
  output_0 : u32,
}

@fragment
fn F(in : In, pixel_local_1 : PixelLocal) -> F_res {
  P = pixel_local_1;
  F_inner(in);
  return F_res(P.a);
}

struct PixelLocal {
  @internal(attachment(1)) @internal(disable_validation__entry_point_parameter)
  a : u32,
}

var<private> P : PixelLocal;

struct In {
  @builtin(position)
  pos : vec4f,
}

fn F_inner(in : In) {
  P.a += u32(in.pos.x);
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(PixelLocalTest, WithInvariantBuiltinInputStructParameter) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

struct In {
  @invariant @builtin(position) pos : vec4f,
}

@fragment
fn F(in : In) {
  P.a += u32(in.pos.x);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

struct F_res {
  @location(1)
  output_0 : u32,
}

@fragment
fn F(in : In, pixel_local_1 : PixelLocal) -> F_res {
  P = pixel_local_1;
  F_inner(in);
  return F_res(P.a);
}

struct PixelLocal {
  @internal(attachment(1)) @internal(disable_validation__entry_point_parameter)
  a : u32,
}

var<private> P : PixelLocal;

struct In {
  @invariant @builtin(position)
  pos : vec4f,
}

fn F_inner(in : In) {
  P.a += u32(in.pos.x);
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(PixelLocalTest, WithLocationInputParameter) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn F(@location(0) a : vec4f, @interpolate(flat) @location(1) b : vec4f) {
  P.a += u32(a.x) + u32(b.y);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

struct F_res {
  @location(1)
  output_0 : u32,
}

@fragment
fn F(@location(0) a : vec4f, @interpolate(flat) @location(1) b : vec4f, pixel_local_1 : PixelLocal) -> F_res {
  P = pixel_local_1;
  F_inner(a, b);
  return F_res(P.a);
}

struct PixelLocal {
  @internal(attachment(1)) @internal(disable_validation__entry_point_parameter)
  a : u32,
}

var<private> P : PixelLocal;

fn F_inner(a : vec4f, b : vec4f) {
  P.a += (u32(a.x) + u32(b.y));
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(PixelLocalTest, WithLocationInputStructParameter) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

struct In {
  @location(0) a : vec4f,
  @interpolate(flat) @location(1) b : vec4f,
}

@fragment
fn F(in : In) {
  P.a += u32(in.a.x) + u32(in.b.y);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

struct F_res {
  @location(1)
  output_0 : u32,
}

@fragment
fn F(in : In, pixel_local_1 : PixelLocal) -> F_res {
  P = pixel_local_1;
  F_inner(in);
  return F_res(P.a);
}

struct PixelLocal {
  @internal(attachment(1)) @internal(disable_validation__entry_point_parameter)
  a : u32,
}

var<private> P : PixelLocal;

struct In {
  @location(0)
  a : vec4f,
  @interpolate(flat) @location(1)
  b : vec4f,
}

fn F_inner(in : In) {
  P.a += (u32(in.a.x) + u32(in.b.y));
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(PixelLocalTest, WithBuiltinAndLocationInputParameter) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn F(@builtin(position) pos : vec4f, @location(0) uv : vec4f) {
  P.a += u32(pos.x) + u32(uv.x);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

struct F_res {
  @location(1)
  output_0 : u32,
}

@fragment
fn F(@builtin(position) pos : vec4f, @location(0) uv : vec4f, pixel_local_1 : PixelLocal) -> F_res {
  P = pixel_local_1;
  F_inner(pos, uv);
  return F_res(P.a);
}

struct PixelLocal {
  @internal(attachment(1)) @internal(disable_validation__entry_point_parameter)
  a : u32,
}

var<private> P : PixelLocal;

fn F_inner(pos : vec4f, uv : vec4f) {
  P.a += (u32(pos.x) + u32(uv.x));
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(PixelLocalTest, WithBuiltinAndLocationInputStructParameter) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

struct In {
  @builtin(position) pos : vec4f,
  @location(0) uv : vec4f,
}

@fragment
fn F(in : In) {
  P.a += u32(in.pos.x) + u32(in.uv.x);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

struct F_res {
  @location(1)
  output_0 : u32,
}

@fragment
fn F(in : In, pixel_local_1 : PixelLocal) -> F_res {
  P = pixel_local_1;
  F_inner(in);
  return F_res(P.a);
}

struct PixelLocal {
  @internal(attachment(1)) @internal(disable_validation__entry_point_parameter)
  a : u32,
}

var<private> P : PixelLocal;

struct In {
  @builtin(position)
  pos : vec4f,
  @location(0)
  uv : vec4f,
}

fn F_inner(in : In) {
  P.a += (u32(in.pos.x) + u32(in.uv.x));
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(PixelLocalTest, WithSingleFragmentOutput) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn F() -> @location(0) vec4f {
  P.a += 42;
  return vec4f(1);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

struct F_res {
  @location(1)
  output_0 : u32,
  @location(0)
  output_1 : vec4<f32>,
}

@fragment
fn F(pixel_local_1 : PixelLocal) -> F_res {
  P = pixel_local_1;
  let result = F_inner();
  return F_res(P.a, result);
}

struct PixelLocal {
  @internal(attachment(1)) @internal(disable_validation__entry_point_parameter)
  a : u32,
}

var<private> P : PixelLocal;

fn F_inner() -> vec4f {
  P.a += 42;
  return vec4f(1);
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1}}));

    EXPECT_EQ(expect, str(got));
}

TEST_F(PixelLocalTest, WithMultipleFragmentOutputs) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : u32,
}

var<pixel_local> P : PixelLocal;

struct Output {
  @location(0) x : vec4f,
  @location(2) y : vec4f,
}

@fragment
fn F() -> Output {
  P.a += 42;
  return Output(vec4f(1), vec4f(9));
}
)";

    auto* expect =
        R"(
enable chromium_experimental_pixel_local;

struct F_res {
  @location(1)
  output_0 : u32,
  @location(5)
  output_1 : u32,
  @location(0)
  output_2 : vec4<f32>,
  @location(2)
  output_3 : vec4<f32>,
}

@fragment
fn F(pixel_local_1 : PixelLocal) -> F_res {
  P = pixel_local_1;
  let result = F_inner();
  return F_res(P.a, P.b, result.x, result.y);
}

struct PixelLocal {
  @internal(attachment(1)) @internal(disable_validation__entry_point_parameter)
  a : u32,
  @internal(attachment(5)) @internal(disable_validation__entry_point_parameter)
  b : u32,
}

var<private> P : PixelLocal;

struct Output {
  x : vec4f,
  y : vec4f,
}

fn F_inner() -> Output {
  P.a += 42;
  return Output(vec4f(1), vec4f(9));
}
)";

    auto got = Run<PixelLocal>(src, Bindings({{0, 1}, {1, 5}}));

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::msl::writer
