// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/helper_test.h"

#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/resource_binding.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/u32.h"

namespace tint::core::type {
namespace {

using ResourceBindingTest = TestHelper;

TEST_F(ResourceBindingTest, Creation) {
    Manager ty;
    auto* a = ty.resource_binding();
    EXPECT_NE(a, nullptr);
}

TEST_F(ResourceBindingTest, Hash) {
    Manager ty;
    auto* a = ty.resource_binding();
    auto* a2 = ty.resource_binding();
    EXPECT_EQ(a->unique_hash, a2->unique_hash);
}

TEST_F(ResourceBindingTest, Equals) {
    Manager ty;
    auto* rb1 = ty.resource_binding();
    auto* rb2 = ty.resource_binding();
    auto* t = ty.sampled_texture(TextureDimension::k2d, ty.u32());

    EXPECT_EQ(rb1, rb2);
    EXPECT_FALSE(rb1->Equals(*t));
}

TEST_F(ResourceBindingTest, FriendlyName) {
    Manager ty;
    auto* a = ty.resource_binding();
    EXPECT_EQ(a->FriendlyName(), "resource_binding");
}

TEST_F(ResourceBindingTest, Clone) {
    Manager ty;
    auto* t = ty.resource_binding();

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* s = t->Clone(ctx);
    EXPECT_NE(s, nullptr);
}

}  // namespace
}  // namespace tint::core::type
