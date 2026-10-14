// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/msl/writer/raise/alias_to_let.h"

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/msl/ir/builtin_call.h"

namespace tint::msl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class MslWriter_AliasToLetTest : public core::ir::transform::TransformTest {
  protected:
    void SetUp() override { mod.properties.Add(core::ir::Property::kAllow8BitIntegers); }
};

TEST_F(MslWriter_AliasToLetTest, AliasPointerOffset_AddLet) {
    auto* v = b.Var("v", ty.ptr(workgroup, ty.array(ty.u8(), 16)));
    mod.root_block->Append(v);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* offset = b.CallExplicit<msl::ir::BuiltinCall>(
            ty.ptr(workgroup, ty.vec2u()), msl::BuiltinFn::kAliasPointerOffset,
            Vector<core::ir::TemplateParameter, 1>{ty.vec2u()}, v, 0_u);
        b.LoadVectorElement(offset, 0_u);
        b.LoadVectorElement(offset, 1_u);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, array<u8, 16>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, vec2<u32>, read_write> = msl.alias_pointer_offset<vec2<u32>> %v, 0u
    %4:u32 = load_vector_element %3, 0u
    %5:u32 = load_vector_element %3, 1u
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, array<u8, 16>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, vec2<u32>, read_write> = msl.alias_pointer_offset<vec2<u32>> %v, 0u
    %4:ptr<workgroup, vec2<u32>, read_write> = let %3
    %5:u32 = load_vector_element %4, 0u
    %6:u32 = load_vector_element %4, 1u
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(AliasToLet);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_AliasToLetTest, PointerOffset_NoLet) {
    auto* v = b.Var("v", ty.ptr(workgroup, ty.array(ty.u8(), 16)));
    mod.root_block->Append(v);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* offset = b.CallExplicit<msl::ir::BuiltinCall>(
            ty.ptr(workgroup, ty.vec2u()), msl::BuiltinFn::kPointerOffset,
            Vector<core::ir::TemplateParameter, 1>{ty.vec2u()}, v, 0_u);
        b.LoadVectorElement(offset, 0_u);
        b.LoadVectorElement(offset, 1_u);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, array<u8, 16>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, vec2<u32>, read_write> = msl.pointer_offset<vec2<u32>> %v, 0u
    %4:u32 = load_vector_element %3, 0u
    %5:u32 = load_vector_element %3, 1u
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(AliasToLet);

    EXPECT_EQ(src, str());
}

}  // namespace
}  // namespace tint::msl::writer::raise
