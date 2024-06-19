// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/msl/writer/helper_test.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::msl::writer {
namespace {

TEST_F(MslWriterTest, Function_Empty) {
    auto* func = b.Function("foo", ty.void_());
    func->Block()->Append(b.Return(func));

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
}
)");
}

TEST_F(MslWriterTest, EntryPointParameterBufferBindingPoint) {
    auto* storage = b.Var("storage_var", ty.ptr(core::AddressSpace::kStorage, ty.i32()));
    auto* uniform = b.Var("uniform_var", ty.ptr(core::AddressSpace::kUniform, ty.i32()));
    storage->SetBindingPoint(0, 1);
    uniform->SetBindingPoint(0, 2);
    mod.root_block->Append(storage);
    mod.root_block->Append(uniform);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Load(storage);
        b.Load(uniform);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_module_vars_struct {
  device int* storage_var;
  const constant int* uniform_var;
};

fragment void foo(device int* storage_var [[buffer(1)]], const constant int* uniform_var [[buffer(2)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.storage_var=storage_var, .uniform_var=uniform_var};
}
)");
}

TEST_F(MslWriterTest, EntryPointParameterHandleBindingPoint) {
    auto* t = ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32());
    auto* texture = b.Var("texture", ty.ptr<handle>(t));
    auto* sampler = b.Var("sampler", ty.ptr<handle>(ty.sampler()));
    texture->SetBindingPoint(0, 1);
    sampler->SetBindingPoint(0, 2);
    mod.root_block->Append(texture);
    mod.root_block->Append(sampler);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Load(texture);
        b.Load(sampler);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

struct tint_module_vars_struct {
  texture2d<float, access::sample> texture;
  sampler sampler;
};

fragment void foo(texture2d<float, access::sample> texture [[texture(1)]], sampler sampler [[sampler(2)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.texture=texture, .sampler=sampler};
}
)");
}

}  // namespace
}  // namespace tint::msl::writer
