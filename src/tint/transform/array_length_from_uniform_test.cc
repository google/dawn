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

#include "src/tint/transform/array_length_from_uniform.h"

#include <utility>

#include "src/tint/transform/simplify_pointers.h"
#include "src/tint/transform/test_helper.h"
#include "src/tint/transform/unshadow.h"

namespace tint::transform {
namespace {

using ArrayLengthFromUniformTest = TransformTest;

TEST_F(ArrayLengthFromUniformTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    ArrayLengthFromUniform::Config cfg({0, 30u});
    cfg.bindpoint_to_size_index.emplace(sem::BindingPoint{0, 0}, 0);

    DataMap data;
    data.Add<ArrayLengthFromUniform::Config>(std::move(cfg));

    EXPECT_FALSE(ShouldRun<ArrayLengthFromUniform>(src, data));
}

TEST_F(ArrayLengthFromUniformTest, ShouldRunNoArrayLength) {
    auto* src = R"(
struct SB {
  x : i32,
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
}
)";

    ArrayLengthFromUniform::Config cfg({0, 30u});
    cfg.bindpoint_to_size_index.emplace(sem::BindingPoint{0, 0}, 0);

    DataMap data;
    data.Add<ArrayLengthFromUniform::Config>(std::move(cfg));

    EXPECT_FALSE(ShouldRun<ArrayLengthFromUniform>(src, data));
}

TEST_F(ArrayLengthFromUniformTest, ShouldRunWithArrayLength) {
    auto* src = R"(
struct SB {
  x : i32,
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = arrayLength(&sb.arr);
}
)";

    ArrayLengthFromUniform::Config cfg({0, 30u});
    cfg.bindpoint_to_size_index.emplace(sem::BindingPoint{0, 0}, 0);

    DataMap data;
    data.Add<ArrayLengthFromUniform::Config>(std::move(cfg));

    EXPECT_TRUE(ShouldRun<ArrayLengthFromUniform>(src, data));
}

TEST_F(ArrayLengthFromUniformTest, Error_MissingTransformData) {
    auto* src = R"(
struct SB {
  x : i32,
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = arrayLength(&sb.arr);
}
)";

    auto* expect =
        "error: missing transform data for "
        "tint::transform::ArrayLengthFromUniform";

    auto got = Run<Unshadow, SimplifyPointers, ArrayLengthFromUniform>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ArrayLengthFromUniformTest, Basic) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read> sb : array<i32>;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = arrayLength(&sb);
}
)";

    auto* expect = R"(
struct tint_symbol {
  buffer_size : array<vec4<u32>, 1u>,
}

@group(0) @binding(30) var<uniform> tint_symbol_1 : tint_symbol;

@group(0) @binding(0) var<storage, read> sb : array<i32>;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = (tint_symbol_1.buffer_size[0u][0u] / 4u);
}
)";

    ArrayLengthFromUniform::Config cfg({0, 30u});
    cfg.bindpoint_to_size_index.emplace(sem::BindingPoint{0, 0}, 0);

    DataMap data;
    data.Add<ArrayLengthFromUniform::Config>(std::move(cfg));

    auto got = Run<Unshadow, SimplifyPointers, ArrayLengthFromUniform>(src, data);

    EXPECT_EQ(expect, str(got));
    auto* val = got.data.Get<ArrayLengthFromUniform::Result>();
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(std::unordered_set<uint32_t>({0}), val->used_size_indices);
}

TEST_F(ArrayLengthFromUniformTest, BasicInStruct) {
    auto* src = R"(
struct SB {
  x : i32,
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = arrayLength(&sb.arr);
}
)";

    auto* expect = R"(
struct tint_symbol {
  buffer_size : array<vec4<u32>, 1u>,
}

@group(0) @binding(30) var<uniform> tint_symbol_1 : tint_symbol;

struct SB {
  x : i32,
  arr : array<i32>,
}

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = ((tint_symbol_1.buffer_size[0u][0u] - 4u) / 4u);
}
)";

    ArrayLengthFromUniform::Config cfg({0, 30u});
    cfg.bindpoint_to_size_index.emplace(sem::BindingPoint{0, 0}, 0);

    DataMap data;
    data.Add<ArrayLengthFromUniform::Config>(std::move(cfg));

    auto got = Run<Unshadow, SimplifyPointers, ArrayLengthFromUniform>(src, data);

    EXPECT_EQ(expect, str(got));
    EXPECT_EQ(std::unordered_set<uint32_t>({0}),
              got.data.Get<ArrayLengthFromUniform::Result>()->used_size_indices);
}

TEST_F(ArrayLengthFromUniformTest, MultipleStorageBuffers) {
    auto* src = R"(
struct SB1 {
  x : i32,
  arr1 : array<i32>,
};
struct SB2 {
  x : i32,
  arr2 : array<vec4<f32>>,
};
struct SB4 {
  x : i32,
  arr4 : array<vec4<f32>>,
};

@group(0) @binding(2) var<storage, read> sb1 : SB1;
@group(1) @binding(2) var<storage, read> sb2 : SB2;
@group(2) @binding(2) var<storage, read> sb3 : array<vec4<f32>>;
@group(3) @binding(2) var<storage, read> sb4 : SB4;
@group(4) @binding(2) var<storage, read> sb5 : array<vec4<f32>>;

@compute @workgroup_size(1)
fn main() {
  var len1 : u32 = arrayLength(&(sb1.arr1));
  var len2 : u32 = arrayLength(&(sb2.arr2));
  var len3 : u32 = arrayLength(&sb3);
  var len4 : u32 = arrayLength(&(sb4.arr4));
  var len5 : u32 = arrayLength(&sb5);
  var x : u32 = (len1 + len2 + len3 + len4 + len5);
}
)";

    auto* expect = R"(
struct tint_symbol {
  buffer_size : array<vec4<u32>, 2u>,
}

@group(0) @binding(30) var<uniform> tint_symbol_1 : tint_symbol;

struct SB1 {
  x : i32,
  arr1 : array<i32>,
}

struct SB2 {
  x : i32,
  arr2 : array<vec4<f32>>,
}

struct SB4 {
  x : i32,
  arr4 : array<vec4<f32>>,
}

@group(0) @binding(2) var<storage, read> sb1 : SB1;

@group(1) @binding(2) var<storage, read> sb2 : SB2;

@group(2) @binding(2) var<storage, read> sb3 : array<vec4<f32>>;

@group(3) @binding(2) var<storage, read> sb4 : SB4;

@group(4) @binding(2) var<storage, read> sb5 : array<vec4<f32>>;

@compute @workgroup_size(1)
fn main() {
  var len1 : u32 = ((tint_symbol_1.buffer_size[0u][0u] - 4u) / 4u);
  var len2 : u32 = ((tint_symbol_1.buffer_size[0u][1u] - 16u) / 16u);
  var len3 : u32 = (tint_symbol_1.buffer_size[0u][2u] / 16u);
  var len4 : u32 = ((tint_symbol_1.buffer_size[0u][3u] - 16u) / 16u);
  var len5 : u32 = (tint_symbol_1.buffer_size[1u][0u] / 16u);
  var x : u32 = ((((len1 + len2) + len3) + len4) + len5);
}
)";

    ArrayLengthFromUniform::Config cfg({0, 30u});
    cfg.bindpoint_to_size_index.emplace(sem::BindingPoint{0, 2u}, 0);
    cfg.bindpoint_to_size_index.emplace(sem::BindingPoint{1u, 2u}, 1);
    cfg.bindpoint_to_size_index.emplace(sem::BindingPoint{2u, 2u}, 2);
    cfg.bindpoint_to_size_index.emplace(sem::BindingPoint{3u, 2u}, 3);
    cfg.bindpoint_to_size_index.emplace(sem::BindingPoint{4u, 2u}, 4);

    DataMap data;
    data.Add<ArrayLengthFromUniform::Config>(std::move(cfg));

    auto got = Run<Unshadow, SimplifyPointers, ArrayLengthFromUniform>(src, data);

    EXPECT_EQ(expect, str(got));
    EXPECT_EQ(std::unordered_set<uint32_t>({0, 1, 2, 3, 4}),
              got.data.Get<ArrayLengthFromUniform::Result>()->used_size_indices);
}

TEST_F(ArrayLengthFromUniformTest, MultipleUnusedStorageBuffers) {
    auto* src = R"(
struct SB1 {
  x : i32,
  arr1 : array<i32>,
};
struct SB2 {
  x : i32,
  arr2 : array<vec4<f32>>,
};
struct SB4 {
  x : i32,
  arr4 : array<vec4<f32>>,
};

@group(0) @binding(2) var<storage, read> sb1 : SB1;
@group(1) @binding(2) var<storage, read> sb2 : SB2;
@group(2) @binding(2) var<storage, read> sb3 : array<vec4<f32>>;
@group(3) @binding(2) var<storage, read> sb4 : SB4;
@group(4) @binding(2) var<storage, read> sb5 : array<vec4<f32>>;

@compute @workgroup_size(1)
fn main() {
  var len1 : u32 = arrayLength(&(sb1.arr1));
  var len3 : u32 = arrayLength(&sb3);
  var x : u32 = (len1 + len3);
}
)";

    auto* expect = R"(
struct tint_symbol {
  buffer_size : array<vec4<u32>, 1u>,
}

@group(0) @binding(30) var<uniform> tint_symbol_1 : tint_symbol;

struct SB1 {
  x : i32,
  arr1 : array<i32>,
}

struct SB2 {
  x : i32,
  arr2 : array<vec4<f32>>,
}

struct SB4 {
  x : i32,
  arr4 : array<vec4<f32>>,
}

@group(0) @binding(2) var<storage, read> sb1 : SB1;

@group(1) @binding(2) var<storage, read> sb2 : SB2;

@group(2) @binding(2) var<storage, read> sb3 : array<vec4<f32>>;

@group(3) @binding(2) var<storage, read> sb4 : SB4;

@group(4) @binding(2) var<storage, read> sb5 : array<vec4<f32>>;

@compute @workgroup_size(1)
fn main() {
  var len1 : u32 = ((tint_symbol_1.buffer_size[0u][0u] - 4u) / 4u);
  var len3 : u32 = (tint_symbol_1.buffer_size[0u][2u] / 16u);
  var x : u32 = (len1 + len3);
}
)";

    ArrayLengthFromUniform::Config cfg({0, 30u});
    cfg.bindpoint_to_size_index.emplace(sem::BindingPoint{0, 2u}, 0);
    cfg.bindpoint_to_size_index.emplace(sem::BindingPoint{1u, 2u}, 1);
    cfg.bindpoint_to_size_index.emplace(sem::BindingPoint{2u, 2u}, 2);
    cfg.bindpoint_to_size_index.emplace(sem::BindingPoint{3u, 2u}, 3);
    cfg.bindpoint_to_size_index.emplace(sem::BindingPoint{4u, 2u}, 4);

    DataMap data;
    data.Add<ArrayLengthFromUniform::Config>(std::move(cfg));

    auto got = Run<Unshadow, SimplifyPointers, ArrayLengthFromUniform>(src, data);

    EXPECT_EQ(expect, str(got));
    EXPECT_EQ(std::unordered_set<uint32_t>({0, 2}),
              got.data.Get<ArrayLengthFromUniform::Result>()->used_size_indices);
}

TEST_F(ArrayLengthFromUniformTest, NoArrayLengthCalls) {
    auto* src = R"(
struct SB {
  x : i32,
  arr : array<i32>,
}

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  _ = &(sb.arr);
}
)";

    ArrayLengthFromUniform::Config cfg({0, 30u});
    cfg.bindpoint_to_size_index.emplace(sem::BindingPoint{0, 0}, 0);

    DataMap data;
    data.Add<ArrayLengthFromUniform::Config>(std::move(cfg));

    auto got = Run<Unshadow, SimplifyPointers, ArrayLengthFromUniform>(src, data);

    EXPECT_EQ(src, str(got));
    EXPECT_EQ(got.data.Get<ArrayLengthFromUniform::Result>(), nullptr);
}

TEST_F(ArrayLengthFromUniformTest, MissingBindingPointToIndexMapping) {
    auto* src = R"(
struct SB1 {
  x : i32,
  arr1 : array<i32>,
};

struct SB2 {
  x : i32,
  arr2 : array<vec4<f32>>,
};

@group(0) @binding(2) var<storage, read> sb1 : SB1;

@group(1) @binding(2) var<storage, read> sb2 : SB2;

@compute @workgroup_size(1)
fn main() {
  var len1 : u32 = arrayLength(&(sb1.arr1));
  var len2 : u32 = arrayLength(&(sb2.arr2));
  var x : u32 = (len1 + len2);
}
)";

    auto* expect = R"(
struct tint_symbol {
  buffer_size : array<vec4<u32>, 1u>,
}

@group(0) @binding(30) var<uniform> tint_symbol_1 : tint_symbol;

struct SB1 {
  x : i32,
  arr1 : array<i32>,
}

struct SB2 {
  x : i32,
  arr2 : array<vec4<f32>>,
}

@group(0) @binding(2) var<storage, read> sb1 : SB1;

@group(1) @binding(2) var<storage, read> sb2 : SB2;

@compute @workgroup_size(1)
fn main() {
  var len1 : u32 = ((tint_symbol_1.buffer_size[0u][0u] - 4u) / 4u);
  var len2 : u32 = arrayLength(&(sb2.arr2));
  var x : u32 = (len1 + len2);
}
)";

    ArrayLengthFromUniform::Config cfg({0, 30u});
    cfg.bindpoint_to_size_index.emplace(sem::BindingPoint{0, 2}, 0);

    DataMap data;
    data.Add<ArrayLengthFromUniform::Config>(std::move(cfg));

    auto got = Run<Unshadow, SimplifyPointers, ArrayLengthFromUniform>(src, data);

    EXPECT_EQ(expect, str(got));
    EXPECT_EQ(std::unordered_set<uint32_t>({0}),
              got.data.Get<ArrayLengthFromUniform::Result>()->used_size_indices);
}

TEST_F(ArrayLengthFromUniformTest, OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main() {
  var len : u32 = arrayLength(&sb.arr);
}

@group(0) @binding(0) var<storage, read> sb : SB;

struct SB {
  x : i32,
  arr : array<i32>,
};
)";

    auto* expect = R"(
struct tint_symbol {
  buffer_size : array<vec4<u32>, 1u>,
}

@group(0) @binding(30) var<uniform> tint_symbol_1 : tint_symbol;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = ((tint_symbol_1.buffer_size[0u][0u] - 4u) / 4u);
}

@group(0) @binding(0) var<storage, read> sb : SB;

struct SB {
  x : i32,
  arr : array<i32>,
}
)";

    ArrayLengthFromUniform::Config cfg({0, 30u});
    cfg.bindpoint_to_size_index.emplace(sem::BindingPoint{0, 0}, 0);

    DataMap data;
    data.Add<ArrayLengthFromUniform::Config>(std::move(cfg));

    auto got = Run<Unshadow, SimplifyPointers, ArrayLengthFromUniform>(src, data);

    EXPECT_EQ(expect, str(got));
    EXPECT_EQ(std::unordered_set<uint32_t>({0}),
              got.data.Get<ArrayLengthFromUniform::Result>()->used_size_indices);
}

}  // namespace
}  // namespace tint::transform
