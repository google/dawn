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

#include "gmock/gmock.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/msl/writer/ast_printer/helper_test.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::msl::writer {
namespace {

using ::testing::HasSubstr;
using MslSanitizerTest = TestHelper;

TEST_F(MslSanitizerTest, Call_ArrayLength) {
    auto* s = Structure("my_struct", Vector{Member(0, "a", ty.array<f32>())});
    GlobalVar("b", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead, Binding(1_a),
              Group(2_a));

    Func("a_func", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("len", ty.u32(), Call("arrayLength", AddressOf(MemberAccessor("b", "a"))))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    Options opts = DefaultOptions();
    opts.array_length_from_uniform.ubo_binding = 30;
    opts.array_length_from_uniform.bindpoint_to_size_index.emplace(BindingPoint{2, 1}, 1);
    ASTPrinter& gen = SanitizeAndBuild(opts);

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    auto got = gen.Result();
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

struct TintArrayLengths {
  /* 0x0000 */ tint_array<uint4, 1> array_lengths;
};

struct my_struct {
  tint_array<float, 1> a;
};

fragment void a_func(const constant TintArrayLengths* tint_symbol [[buffer(30)]]) {
  uint len = (((*(tint_symbol)).array_lengths[0u][1u] - 0u) / 4u);
  return;
}

)";
    EXPECT_EQ(expect, got);
}

TEST_F(MslSanitizerTest, Call_ArrayLength_OtherMembersInStruct) {
    auto* s = Structure("my_struct", Vector{
                                         Member(0, "z", ty.f32()),
                                         Member(4, "a", ty.array<f32>()),
                                     });
    GlobalVar("b", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead, Binding(1_a),
              Group(2_a));

    Func("a_func", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("len", ty.u32(), Call("arrayLength", AddressOf(MemberAccessor("b", "a"))))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    Options opts = DefaultOptions();
    opts.array_length_from_uniform.ubo_binding = 30;
    opts.array_length_from_uniform.bindpoint_to_size_index.emplace(BindingPoint{2, 1}, 1);
    ASTPrinter& gen = SanitizeAndBuild(opts);

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    auto got = gen.Result();
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

struct TintArrayLengths {
  /* 0x0000 */ tint_array<uint4, 1> array_lengths;
};

struct my_struct {
  float z;
  tint_array<float, 1> a;
};

fragment void a_func(const constant TintArrayLengths* tint_symbol [[buffer(30)]]) {
  uint len = (((*(tint_symbol)).array_lengths[0u][1u] - 4u) / 4u);
  return;
}

)";

    EXPECT_EQ(expect, got);
}

TEST_F(MslSanitizerTest, Call_ArrayLength_ViaLets) {
    auto* s = Structure("my_struct", Vector{Member(0, "a", ty.array<f32>())});
    GlobalVar("b", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead, Binding(1_a),
              Group(2_a));

    auto* p = Let("p", AddressOf("b"));
    auto* p2 = Let("p2", AddressOf(MemberAccessor(Deref(p), "a")));

    Func("a_func", tint::Empty, ty.void_(),
         Vector{
             Decl(p),
             Decl(p2),
             Decl(Var("len", ty.u32(), Call("arrayLength", p2))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    Options opts = DefaultOptions();
    opts.array_length_from_uniform.ubo_binding = 30;
    opts.array_length_from_uniform.bindpoint_to_size_index.emplace(BindingPoint{2, 1}, 1);
    ASTPrinter& gen = SanitizeAndBuild(opts);

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    auto got = gen.Result();
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

struct TintArrayLengths {
  /* 0x0000 */ tint_array<uint4, 1> array_lengths;
};

struct my_struct {
  tint_array<float, 1> a;
};

fragment void a_func(const constant TintArrayLengths* tint_symbol [[buffer(30)]]) {
  uint len = (((*(tint_symbol)).array_lengths[0u][1u] - 0u) / 4u);
  return;
}

)";

    EXPECT_EQ(expect, got);
}

TEST_F(MslSanitizerTest, Call_ArrayLength_ArrayLengthFromUniform) {
    auto* s = Structure("my_struct", Vector{Member(0, "a", ty.array<f32>())});
    GlobalVar("b", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead, Binding(1_a),
              Group(0_a));
    GlobalVar("c", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead, Binding(2_a),
              Group(0_a));

    Func("a_func", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("len", ty.u32(),
                      Add(Call("arrayLength", AddressOf(MemberAccessor("b", "a"))),
                          Call("arrayLength", AddressOf(MemberAccessor("c", "a")))))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    Options options;
    options.array_length_from_uniform.ubo_binding = 29;
    options.array_length_from_uniform.bindpoint_to_size_index.emplace(BindingPoint{0, 1}, 7u);
    options.array_length_from_uniform.bindpoint_to_size_index.emplace(BindingPoint{0, 2}, 2u);
    ASTPrinter& gen = SanitizeAndBuild(options);

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    auto got = gen.Result();
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

struct TintArrayLengths {
  /* 0x0000 */ tint_array<uint4, 2> array_lengths;
};

struct my_struct {
  tint_array<float, 1> a;
};

fragment void a_func(const constant TintArrayLengths* tint_symbol [[buffer(29)]]) {
  uint len = ((((*(tint_symbol)).array_lengths[1u][3u] - 0u) / 4u) + (((*(tint_symbol)).array_lengths[0u][2u] - 0u) / 4u));
  return;
}

)";
    EXPECT_EQ(expect, got);
}

TEST_F(MslSanitizerTest, Call_ArrayLength_ArrayLengthFromUniformMissingBinding) {
    auto* s = Structure("my_struct", Vector{Member(0, "a", ty.array<f32>())});
    GlobalVar("b", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead, Binding(1_a),
              Group(0_a));
    GlobalVar("c", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead, Binding(2_a),
              Group(0_a));

    Func("a_func", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("len", ty.u32(),
                      Add(Call("arrayLength", AddressOf(MemberAccessor("b", "a"))),
                          Call("arrayLength", AddressOf(MemberAccessor("c", "a")))))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    Options options;
    options.array_length_from_uniform.ubo_binding = 29;
    options.array_length_from_uniform.bindpoint_to_size_index.emplace(BindingPoint{0, 2}, 2u);
    ASTPrinter& gen = SanitizeAndBuild(options);

    ASSERT_FALSE(gen.Generate());
    EXPECT_THAT(gen.Diagnostics().Str(), HasSubstr("Unable to translate builtin: arrayLength"));
}

}  // namespace
}  // namespace tint::msl::writer
