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

#include "src/tint/lang/hlsl/writer/raise/decompose_memory_access.h"

#include <gtest/gtest.h>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

using HlslWriterDecomposeMemoryAccessTest = core::ir::transform::TransformTest;

TEST_F(HlslWriterDecomposeMemoryAccessTest, NoBufferAccess) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] { b.Return(func); });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;
    Run(DecomposeMemoryAccess);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeMemoryAccessTest, VectorLoad) {
    auto* var = b.Var<storage, vec4<f32>, core::Access::kRead>("v");

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.LoadVectorElement(var, 0_u));
        b.Let("b", b.LoadVectorElement(var, 1_u));
        b.Let("c", b.LoadVectorElement(var, 2_u));
        b.Let("d", b.LoadVectorElement(var, 3_u));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, vec4<f32>, read> = var
}

%foo = @fragment func():void {
  $B2: {
    %3:f32 = load_vector_element %v, 0u
    %a:f32 = let %3
    %5:f32 = load_vector_element %v, 1u
    %b:f32 = let %5
    %7:f32 = load_vector_element %v, 2u
    %c:f32 = let %7
    %9:f32 = load_vector_element %v, 3u
    %d:f32 = let %9
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<vec4<f32>, read> = var
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = %v.Load 0u
    %4:f32 = bitcast %3
    %a:f32 = let %4
    %6:u32 = %v.Load 4u
    %7:f32 = bitcast %6
    %b:f32 = let %7
    %9:u32 = %v.Load 8u
    %10:f32 = bitcast %9
    %c:f32 = let %10
    %12:u32 = %v.Load 12u
    %13:f32 = bitcast %12
    %d:f32 = let %13
    ret
  }
}
)";

    Run(DecomposeMemoryAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeMemoryAccessTest, VectorStore) {
    auto* var = b.Var<storage, vec4<f32>, core::Access::kReadWrite>("v");

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(var, 0_u, 2_f);
        b.StoreVectorElement(var, 1_u, 4_f);
        b.StoreVectorElement(var, 2_u, 8_f);
        b.StoreVectorElement(var, 3_u, 16_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, vec4<f32>, read_write> = var
}

%foo = @fragment func():void {
  $B2: {
    store_vector_element %v, 0u, 2.0f
    store_vector_element %v, 1u, 4.0f
    store_vector_element %v, 2u, 8.0f
    store_vector_element %v, 3u, 16.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<vec4<f32>, read_write> = var
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = bitcast 2.0f
    %4:void = %v.Store 0u, %3
    %5:u32 = bitcast 4.0f
    %6:void = %v.Store 4u, %5
    %7:u32 = bitcast 8.0f
    %8:void = %v.Store 8u, %7
    %9:u32 = bitcast 16.0f
    %10:void = %v.Store 12u, %9
    ret
  }
}
)";

    Run(DecomposeMemoryAccess);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
