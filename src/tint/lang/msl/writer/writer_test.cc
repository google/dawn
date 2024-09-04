// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/msl/writer/helper_test.h"

#include "gmock/gmock.h"

namespace tint::msl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(MslWriterTest, WorkgroupAllocations) {
    auto* var_a = b.Var("a", ty.ptr<workgroup, i32>());
    auto* var_b = b.Var("b", ty.ptr<workgroup, i32>());
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);

    auto* foo = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute,
                           std::array<uint32_t, 3>{1u, 1u, 1u});
    b.Append(foo->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        b.Store(var_a, b.Add<i32>(load_a, load_b));
        b.Return(foo);
    });

    // No allocations, but still needs an entry in the map.
    auto* bar = b.Function("bar", ty.void_(), core::ir::Function::PipelineStage::kCompute,
                           std::array<uint32_t, 3>{1u, 1u, 1u});
    b.Append(bar->Block(), [&] { b.Return(bar); });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

struct tint_module_vars_struct {
  threadgroup int* a;
  threadgroup int* b;
};

struct tint_symbol_2 {
  int tint_symbol;
  int tint_symbol_1;
};

void foo_inner(uint tint_local_index, tint_module_vars_struct tint_module_vars) {
  if ((tint_local_index == 0u)) {
    (*tint_module_vars.a) = 0;
    (*tint_module_vars.b) = 0;
  }
  threadgroup_barrier(mem_flags::mem_threadgroup);
  (*tint_module_vars.a) = as_type<int>((as_type<uint>((*tint_module_vars.a)) + as_type<uint>((*tint_module_vars.b))));
}

kernel void bar() {
}

kernel void foo(uint tint_local_index [[thread_index_in_threadgroup]], threadgroup tint_symbol_2* v [[threadgroup(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.a=(&(*v).tint_symbol), .b=(&(*v).tint_symbol_1)};
  foo_inner(tint_local_index, tint_module_vars);
}
)");
    ASSERT_EQ(output_.workgroup_allocations.size(), 2u);
    ASSERT_EQ(output_.workgroup_allocations.count("foo"), 1u);
    ASSERT_EQ(output_.workgroup_allocations.count("bar"), 1u);
    EXPECT_THAT(output_.workgroup_allocations.at("foo"), testing::ElementsAre(8u));
    EXPECT_THAT(output_.workgroup_allocations.at("bar"), testing::ElementsAre());
}

TEST_F(MslWriterTest, NeedsStorageBufferSizes_False) {
    auto* var = b.Var("a", ty.ptr<storage, array<u32>>());
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* foo = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute,
                           std::array<uint32_t, 3>{1u, 1u, 1u});
    b.Append(foo->Block(), [&] {
        b.Store(b.Access<ptr<storage, u32>>(var, 0_u), 42_u);
        b.Return(foo);
    });

    Options options;
    options.array_length_from_uniform.bindpoint_to_size_index[{0u, 0u}] = 0u;
    options.array_length_from_uniform.ubo_binding = 30u;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
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

struct tint_module_vars_struct {
  device tint_array<uint, 1>* a;
};

kernel void foo(device tint_array<uint, 1>* a [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.a=a};
  (*tint_module_vars.a)[0u] = 42u;
}
)");
    EXPECT_FALSE(output_.needs_storage_buffer_sizes);
}

TEST_F(MslWriterTest, NeedsStorageBufferSizes_True) {
    auto* var = b.Var("a", ty.ptr<storage, array<u32>>());
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* foo = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute,
                           std::array<uint32_t, 3>{1u, 1u, 1u});
    b.Append(foo->Block(), [&] {
        auto* length = b.Call<u32>(core::BuiltinFn::kArrayLength, var);
        b.Store(b.Access<ptr<storage, u32>>(var, 0_u), length);
        b.Return(foo);
    });

    Options options;
    options.array_length_from_uniform.bindpoint_to_size_index[{0u, 0u}] = 0u;
    options.array_length_from_uniform.ubo_binding = 30u;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
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

struct tint_module_vars_struct {
  device tint_array<uint, 1>* a;
  const constant tint_array<uint4, 1>* tint_storage_buffer_sizes;
};

kernel void foo(device tint_array<uint, 1>* a [[buffer(0)]], const constant tint_array<uint4, 1>* tint_storage_buffer_sizes [[buffer(30)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.a=a, .tint_storage_buffer_sizes=tint_storage_buffer_sizes};
  (*tint_module_vars.a)[0u] = ((*tint_module_vars.tint_storage_buffer_sizes)[0u][0u] / 4u);
}
)");
    EXPECT_TRUE(output_.needs_storage_buffer_sizes);
}

}  // namespace
}  // namespace tint::msl::writer
