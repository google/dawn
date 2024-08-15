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

#include "gmock/gmock.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

namespace tint::resolver {

using namespace tint::core::fluent_types;  // NOLINT

namespace {

constexpr size_t kMaxClipDistancesSize = 8;

using ResolverClipDistancesExtensionTest = ResolverTest;

// Using a clip_distances builtin attribute without WGSL extension `clip_distances` enabled should
// fail.
TEST_F(ResolverClipDistancesExtensionTest, UseClipDistancesWithoutExtensionError) {
    Structure("Outputs", Vector{
                             Member("a", ty.array<f32, kMaxClipDistancesSize>(),
                                    Vector{Builtin(core::BuiltinValue::kClipDistances)}),
                         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: use of '@builtin(clip_distances)' requires enabling extension 'clip_distances')");
}

// Using a clip_distances builtin attribute on i32 scalar should fail.
TEST_F(ResolverClipDistancesExtensionTest, UseClipDistancesOnI32Error) {
    Enable(wgsl::Extension::kClipDistances);
    Structure("Outputs",
              Vector{
                  Member("a", ty.i32(), Vector{Builtin(core::BuiltinValue::kClipDistances)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: store type of '@builtin(clip_distances)' must be 'array<f32, N>' (N <= 8))");
}

// Using a clip_distances builtin attribute on a vector of i32 should fail.
TEST_F(ResolverClipDistancesExtensionTest, UseClipDistancesOnI32VecError) {
    Enable(wgsl::Extension::kClipDistances);
    Structure("Outputs",
              Vector{
                  Member("a", ty.vec4<i32>(), Vector{Builtin(core::BuiltinValue::kClipDistances)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: store type of '@builtin(clip_distances)' must be 'array<f32, N>' (N <= 8))");
}

// Using a clip_distances builtin attribute on i32 array should fail.
TEST_F(ResolverClipDistancesExtensionTest, UseClipDistancesOnI32ArrayError) {
    Enable(wgsl::Extension::kClipDistances);
    Structure("Outputs", Vector{
                             Member("a", ty.array<i32, kMaxClipDistancesSize>(),
                                    Vector{Builtin(core::BuiltinValue::kClipDistances)}),
                         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: store type of '@builtin(clip_distances)' must be 'array<f32, N>' (N <= 8))");
}

// Using a clip_distances builtin attribute on u32 scalar should fail.
TEST_F(ResolverClipDistancesExtensionTest, UseClipDistancesOnU32Error) {
    Enable(wgsl::Extension::kClipDistances);
    Structure("Outputs",
              Vector{
                  Member("a", ty.u32(), Vector{Builtin(core::BuiltinValue::kClipDistances)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: store type of '@builtin(clip_distances)' must be 'array<f32, N>' (N <= 8))");
}

// Using a clip_distances builtin attribute on a vector of u32 should fail.
TEST_F(ResolverClipDistancesExtensionTest, UseClipDistancesOnU32VecError) {
    Enable(wgsl::Extension::kClipDistances);
    Structure("Outputs",
              Vector{
                  Member("a", ty.vec4<u32>(), Vector{Builtin(core::BuiltinValue::kClipDistances)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: store type of '@builtin(clip_distances)' must be 'array<f32, N>' (N <= 8))");
}

// Using a clip_distances builtin attribute on u32 array should fail.
TEST_F(ResolverClipDistancesExtensionTest, UseClipDistancesOnU32ArrayError) {
    Enable(wgsl::Extension::kClipDistances);
    Structure("Outputs", Vector{
                             Member("a", ty.array<u32, kMaxClipDistancesSize>(),
                                    Vector{Builtin(core::BuiltinValue::kClipDistances)}),
                         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: store type of '@builtin(clip_distances)' must be 'array<f32, N>' (N <= 8))");
}

// Using a clip_distances builtin attribute on f16 scalar should fail.
TEST_F(ResolverClipDistancesExtensionTest, UseClipDistancesOnF16Error) {
    Enable(wgsl::Extension::kClipDistances);
    Enable(wgsl::Extension::kF16);
    Structure("Outputs",
              Vector{
                  Member("a", ty.f16(), Vector{Builtin(core::BuiltinValue::kClipDistances)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: store type of '@builtin(clip_distances)' must be 'array<f32, N>' (N <= 8))");
}

// Using a clip_distances builtin attribute on a vector of f16 should fail.
TEST_F(ResolverClipDistancesExtensionTest, UseClipDistancesOnF16VecError) {
    Enable(wgsl::Extension::kClipDistances);
    Enable(wgsl::Extension::kF16);
    Structure("Outputs",
              Vector{
                  Member("a", ty.vec4<f16>(), Vector{Builtin(core::BuiltinValue::kClipDistances)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: store type of '@builtin(clip_distances)' must be 'array<f32, N>' (N <= 8))");
}

// Using a clip_distances builtin attribute on f16 array should fail.
TEST_F(ResolverClipDistancesExtensionTest, UseClipDistancesOnF16ArrayError) {
    Enable(wgsl::Extension::kClipDistances);
    Enable(wgsl::Extension::kF16);
    Structure("Outputs", Vector{
                             Member("a", ty.array<f16, kMaxClipDistancesSize>(),
                                    Vector{Builtin(core::BuiltinValue::kClipDistances)}),
                         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: store type of '@builtin(clip_distances)' must be 'array<f32, N>' (N <= 8))");
}

// Using a clip_distances builtin attribute on f32 scalar should fail.
TEST_F(ResolverClipDistancesExtensionTest, UseClipDistancesOnF32Error) {
    Enable(wgsl::Extension::kClipDistances);
    Structure("Outputs",
              Vector{
                  Member("a", ty.f32(), Vector{Builtin(core::BuiltinValue::kClipDistances)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: store type of '@builtin(clip_distances)' must be 'array<f32, N>' (N <= 8))");
}

// Using a clip_distances builtin attribute on a vector of f32 should fail.
TEST_F(ResolverClipDistancesExtensionTest, UseClipDistancesOnF32VecError) {
    Enable(wgsl::Extension::kClipDistances);
    Structure("Outputs",
              Vector{
                  Member("a", ty.vec4<f32>(), Vector{Builtin(core::BuiltinValue::kClipDistances)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: store type of '@builtin(clip_distances)' must be 'array<f32, N>' (N <= 8))");
}

// Using a clip_distances builtin attribute on f32 unsized array should fail.
TEST_F(ResolverClipDistancesExtensionTest, UseClipDistancesOnF32UnsizedArrayError) {
    Enable(wgsl::Extension::kClipDistances);
    Structure("Outputs",
              Vector{
                  Member("a", ty.array<f32>(), Vector{Builtin(core::BuiltinValue::kClipDistances)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: store type of '@builtin(clip_distances)' must be 'array<f32, N>' (N <= 8))");
}

// Using a clip_distances builtin attribute on f32 array with size == 1 should pass.
TEST_F(ResolverClipDistancesExtensionTest, UseClipDistancesOnF32ArraySize1Success) {
    Enable(wgsl::Extension::kClipDistances);
    Structure("Outputs", Vector{
                             Member("a", ty.array<f32, 1>(),
                                    Vector{Builtin(core::BuiltinValue::kClipDistances)}),
                         });

    EXPECT_TRUE(r()->Resolve());
}

// Using a clip_distances builtin attribute on f32 array with size == kMaxClipDistancesSize should
// pass.
TEST_F(ResolverClipDistancesExtensionTest, UseClipDistancesOnF32ArraySize8Success) {
    Enable(wgsl::Extension::kClipDistances);
    Structure("Outputs", Vector{
                             Member("a", ty.array<f32, kMaxClipDistancesSize>(),
                                    Vector{Builtin(core::BuiltinValue::kClipDistances)}),
                         });

    EXPECT_TRUE(r()->Resolve());
}

// Using a clip_distances builtin attribute on f32 array with size > kMaxClipDistancesSize should
// fail.
TEST_F(ResolverClipDistancesExtensionTest, UseClipDistancesOnF32ArrayWithTooLargeSizeFail) {
    Enable(wgsl::Extension::kClipDistances);
    Structure("Outputs", Vector{
                             Member("a", ty.array<f32, kMaxClipDistancesSize + 1>(),
                                    Vector{Builtin(core::BuiltinValue::kClipDistances)}),
                         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: store type of '@builtin(clip_distances)' must be 'array<f32, N>' (N <= 8))");
}

}  // namespace
}  // namespace tint::resolver
