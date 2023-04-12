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
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/writer/msl/test_helper.h"

namespace tint::writer::msl {
namespace {

using ::testing::HasSubstr;
using namespace tint::number_suffixes;  // NOLINT

using MslSanitizerTest = TestHelper;

TEST_F(MslSanitizerTest, Call_ArrayLength) {
    auto* s = Structure("my_struct", utils::Vector{Member(0, "a", ty.array<f32>())});
    GlobalVar("b", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kRead, Binding(1_a),
              Group(2_a));

    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("len", ty.u32(), Call("arrayLength", AddressOf(MemberAccessor("b", "a"))))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    Options opts = DefaultOptions();
    opts.array_length_from_uniform.ubo_binding = sem::BindingPoint{0, 30};
    opts.array_length_from_uniform.bindpoint_to_size_index.emplace(sem::BindingPoint{2, 1}, 1);
    GeneratorImpl& gen = SanitizeAndBuild(opts);

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    auto got = gen.result();
    auto* expect = R"(#include <metal_stdlib>

using namespace metal;

template<typename T, size_t N>
struct tint_array {
    const constant T& operator[](size_t i) const constant { return elements[i]; }
    device T& operator[](size_t i) device { return elements[i]; }
    const device T& operator[](size_t i) const device { return elements[i]; }
    thread T& operator[](size_t i) thread { return elements[i]; }
    const thread T& operator[](size_t i) const thread { return elements[i]; }
    threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
    const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
    T elements[N];
};

struct tint_symbol {
  /* 0x0000 */ tint_array<uint4, 1> buffer_size;
};

struct my_struct {
  tint_array<float, 1> a;
};

fragment void a_func(const constant tint_symbol* tint_symbol_2 [[buffer(30)]]) {
  uint len = (((*(tint_symbol_2)).buffer_size[0u][1u] - 0u) / 4u);
  return;
}

)";
    EXPECT_EQ(expect, got);
}

TEST_F(MslSanitizerTest, Call_ArrayLength_OtherMembersInStruct) {
    auto* s = Structure("my_struct", utils::Vector{
                                         Member(0, "z", ty.f32()),
                                         Member(4, "a", ty.array<f32>()),
                                     });
    GlobalVar("b", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kRead, Binding(1_a),
              Group(2_a));

    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("len", ty.u32(), Call("arrayLength", AddressOf(MemberAccessor("b", "a"))))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    Options opts = DefaultOptions();
    opts.array_length_from_uniform.ubo_binding = sem::BindingPoint{0, 30};
    opts.array_length_from_uniform.bindpoint_to_size_index.emplace(sem::BindingPoint{2, 1}, 1);
    GeneratorImpl& gen = SanitizeAndBuild(opts);

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    auto got = gen.result();
    auto* expect = R"(#include <metal_stdlib>

using namespace metal;

template<typename T, size_t N>
struct tint_array {
    const constant T& operator[](size_t i) const constant { return elements[i]; }
    device T& operator[](size_t i) device { return elements[i]; }
    const device T& operator[](size_t i) const device { return elements[i]; }
    thread T& operator[](size_t i) thread { return elements[i]; }
    const thread T& operator[](size_t i) const thread { return elements[i]; }
    threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
    const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
    T elements[N];
};

struct tint_symbol {
  /* 0x0000 */ tint_array<uint4, 1> buffer_size;
};

struct my_struct {
  float z;
  tint_array<float, 1> a;
};

fragment void a_func(const constant tint_symbol* tint_symbol_2 [[buffer(30)]]) {
  uint len = (((*(tint_symbol_2)).buffer_size[0u][1u] - 4u) / 4u);
  return;
}

)";

    EXPECT_EQ(expect, got);
}

TEST_F(MslSanitizerTest, Call_ArrayLength_ViaLets) {
    auto* s = Structure("my_struct", utils::Vector{Member(0, "a", ty.array<f32>())});
    GlobalVar("b", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kRead, Binding(1_a),
              Group(2_a));

    auto* p = Let("p", AddressOf("b"));
    auto* p2 = Let("p2", AddressOf(MemberAccessor(Deref(p), "a")));

    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(p),
             Decl(p2),
             Decl(Var("len", ty.u32(), Call("arrayLength", p2))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    Options opts = DefaultOptions();
    opts.array_length_from_uniform.ubo_binding = sem::BindingPoint{0, 30};
    opts.array_length_from_uniform.bindpoint_to_size_index.emplace(sem::BindingPoint{2, 1}, 1);
    GeneratorImpl& gen = SanitizeAndBuild(opts);

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    auto got = gen.result();
    auto* expect = R"(#include <metal_stdlib>

using namespace metal;

template<typename T, size_t N>
struct tint_array {
    const constant T& operator[](size_t i) const constant { return elements[i]; }
    device T& operator[](size_t i) device { return elements[i]; }
    const device T& operator[](size_t i) const device { return elements[i]; }
    thread T& operator[](size_t i) thread { return elements[i]; }
    const thread T& operator[](size_t i) const thread { return elements[i]; }
    threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
    const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
    T elements[N];
};

struct tint_symbol {
  /* 0x0000 */ tint_array<uint4, 1> buffer_size;
};

struct my_struct {
  tint_array<float, 1> a;
};

fragment void a_func(const constant tint_symbol* tint_symbol_2 [[buffer(30)]]) {
  uint len = (((*(tint_symbol_2)).buffer_size[0u][1u] - 0u) / 4u);
  return;
}

)";

    EXPECT_EQ(expect, got);
}

TEST_F(MslSanitizerTest, Call_ArrayLength_ArrayLengthFromUniform) {
    auto* s = Structure("my_struct", utils::Vector{Member(0, "a", ty.array<f32>())});
    GlobalVar("b", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kRead, Binding(1_a),
              Group(0_a));
    GlobalVar("c", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kRead, Binding(2_a),
              Group(0_a));

    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("len", ty.u32(),
                      Add(Call("arrayLength", AddressOf(MemberAccessor("b", "a"))),
                          Call("arrayLength", AddressOf(MemberAccessor("c", "a")))))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    Options options;
    options.array_length_from_uniform.ubo_binding = {0, 29};
    options.array_length_from_uniform.bindpoint_to_size_index.emplace(sem::BindingPoint{0, 1}, 7u);
    options.array_length_from_uniform.bindpoint_to_size_index.emplace(sem::BindingPoint{0, 2}, 2u);
    GeneratorImpl& gen = SanitizeAndBuild(options);

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    auto got = gen.result();
    auto* expect = R"(#include <metal_stdlib>

using namespace metal;

template<typename T, size_t N>
struct tint_array {
    const constant T& operator[](size_t i) const constant { return elements[i]; }
    device T& operator[](size_t i) device { return elements[i]; }
    const device T& operator[](size_t i) const device { return elements[i]; }
    thread T& operator[](size_t i) thread { return elements[i]; }
    const thread T& operator[](size_t i) const thread { return elements[i]; }
    threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
    const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
    T elements[N];
};

struct tint_symbol {
  /* 0x0000 */ tint_array<uint4, 2> buffer_size;
};

struct my_struct {
  tint_array<float, 1> a;
};

fragment void a_func(const constant tint_symbol* tint_symbol_2 [[buffer(29)]]) {
  uint len = ((((*(tint_symbol_2)).buffer_size[1u][3u] - 0u) / 4u) + (((*(tint_symbol_2)).buffer_size[0u][2u] - 0u) / 4u));
  return;
}

)";
    EXPECT_EQ(expect, got);
}

TEST_F(MslSanitizerTest, Call_ArrayLength_ArrayLengthFromUniformMissingBinding) {
    auto* s = Structure("my_struct", utils::Vector{Member(0, "a", ty.array<f32>())});
    GlobalVar("b", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kRead, Binding(1_a),
              Group(0_a));
    GlobalVar("c", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kRead, Binding(2_a),
              Group(0_a));

    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("len", ty.u32(),
                      Add(Call("arrayLength", AddressOf(MemberAccessor("b", "a"))),
                          Call("arrayLength", AddressOf(MemberAccessor("c", "a")))))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    Options options;
    options.array_length_from_uniform.ubo_binding = {0, 29};
    options.array_length_from_uniform.bindpoint_to_size_index.emplace(sem::BindingPoint{0, 2}, 2u);
    GeneratorImpl& gen = SanitizeAndBuild(options);

    ASSERT_FALSE(gen.Generate());
    EXPECT_THAT(gen.Diagnostics().str(), HasSubstr("Unable to translate builtin: arrayLength"));
}

}  // namespace
}  // namespace tint::writer::msl
