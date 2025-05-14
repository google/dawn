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

#include "src/tint/lang/msl/writer/raise/module_constant.h"

#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/sampled_texture.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer::raise {
namespace {

class MslWriter_ModuleConstantTest : public core::ir::transform::TransformTest {
  public:
    void SetUp() override { capabilities.Add(core::ir::Capability::kAllowModuleScopeLets); }
};

TEST_F(MslWriter_ModuleConstantTest, ConstArray) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* c = b.Composite<array<u32, 3>>(1_u, 2_u, 3_u);
        auto* index = b.Let(1_u);
        auto* access = b.Access(ty.u32(), c, index);
        auto* r = b.Let("q", access);
        b.Return(func, r);
    });

    auto* src = R"(
%foo = func():u32 {
  $B1: {
    %2:u32 = let 1u
    %3:u32 = access array<u32, 3>(1u, 2u, 3u), %2
    %q:u32 = let %3
    ret %q
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:array<u32, 3> = let array<u32, 3>(1u, 2u, 3u)
}

%foo = func():u32 {
  $B2: {
    %3:u32 = let 1u
    %4:u32 = access %1, %3
    %q:u32 = let %4
    ret %q
  }
}
)";

    Run(ModuleConstant);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleConstantTest, ConstVec3) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* c = b.Composite<vec3<u32>>(1_u, 0_u, 1_u);
        auto* index = b.Let(1_u);
        auto* access = b.Access(ty.u32(), c, index);
        auto* r = b.Let("q", access);
        b.Return(func, r);
    });

    auto* src = R"(
%foo = func():u32 {
  $B1: {
    %2:u32 = let 1u
    %3:u32 = access vec3<u32>(1u, 0u, 1u), %2
    %q:u32 = let %3
    ret %q
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():u32 {
  $B1: {
    %2:u32 = let 1u
    %3:u32 = access vec3<u32>(1u, 0u, 1u), %2
    %q:u32 = let %3
    ret %q
  }
}
)";
    Run(ModuleConstant);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleConstantTest, ConstStruct) {
    auto* s = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.Register("a"), ty.u32()},
                                                  {mod.symbols.Register("b"), ty.u32()},
                                              });
    auto* c = b.Splat(s, 1_u);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.u32(), c, 0_u);
        auto* r = b.Let("q", access);
        b.Return(func, r);
    });

    auto* src = R"(
S = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

%foo = func():u32 {
  $B1: {
    %2:u32 = access S(1u), 0u
    %q:u32 = let %2
    ret %q
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %1:S = let S(1u)
}

%foo = func():u32 {
  $B2: {
    %3:u32 = access %1, 0u
    %q:u32 = let %3
    ret %q
  }
}
)";
    Run(ModuleConstant);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleConstantTest, ConstArrayNested) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* c = b.Composite<array<array<u32, 2>, 2>>(b.Composite<array<u32, 2>>(1_u, 3_u),
                                                       b.Composite<array<u32, 2>>(1_u, 3_u));
        auto* index = b.Let(1_u);
        auto* access = b.Access(ty.u32(), c, index, index);
        auto* r = b.Let("q", access);
        b.Return(func, r);
    });

    auto* src = R"(
%foo = func():u32 {
  $B1: {
    %2:u32 = let 1u
    %3:u32 = access array<array<u32, 2>, 2>(array<u32, 2>(1u, 3u)), %2, %2
    %q:u32 = let %3
    ret %q
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:array<array<u32, 2>, 2> = let array<array<u32, 2>, 2>(array<u32, 2>(1u, 3u))
}

%foo = func():u32 {
  $B2: {
    %3:u32 = let 1u
    %4:u32 = access %1, %3, %3
    %q:u32 = let %4
    ret %q
  }
}
)";

    Run(ModuleConstant);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleConstantTest, ConstArrayStruct) {
    auto* func = b.Function("foo", ty.u32());
    auto* s = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.Register("a"), ty.u32()},
                                                  {mod.symbols.Register("b"), ty.u32()},
                                              });

    b.Append(func->Block(), [&] {
        auto array_struct_type = ty.array(s, 2);
        auto* c = b.Composite(array_struct_type, b.Splat(s, 1_u), b.Splat(s, 2_u));
        auto* index = b.Let(1_u);
        auto* access = b.Access(ty.u32(), c, index, 0_u);
        auto* r = b.Let("q", access);
        b.Return(func, r);
    });

    auto* src = R"(
S = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

%foo = func():u32 {
  $B1: {
    %2:u32 = let 1u
    %3:u32 = access array<S, 2>(S(1u), S(2u)), %2, 0u
    %q:u32 = let %3
    ret %q
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %1:array<S, 2> = let array<S, 2>(S(1u), S(2u))
}

%foo = func():u32 {
  $B2: {
    %3:u32 = let 1u
    %4:u32 = access %1, %3, 0u
    %q:u32 = let %4
    ret %q
  }
}
)";

    Run(ModuleConstant);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleConstantTest, ConstStructArray) {
    auto* func = b.Function("foo", ty.u32());
    auto* s =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.Register("a"), ty.array(ty.u32(), 2)},
                                            {mod.symbols.Register("b"), ty.array(ty.u32(), 2)},
                                        });

    b.Append(func->Block(), [&] {
        auto* c = b.Splat(s, 1_u);
        auto* index = b.Let(1_u);
        auto* access = b.Access(ty.u32(), c, 0_u, index);
        auto* r = b.Let("q", access);
        b.Return(func, r);
    });

    auto* src = R"(
S = struct @align(4) {
  a:array<u32, 2> @offset(0)
  b:array<u32, 2> @offset(8)
}

%foo = func():u32 {
  $B1: {
    %2:u32 = let 1u
    %3:u32 = access S(1u), 0u, %2
    %q:u32 = let %3
    ret %q
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:array<u32, 2> @offset(0)
  b:array<u32, 2> @offset(8)
}

$B1: {  # root
  %1:S = let S(1u)
}

%foo = func():u32 {
  $B2: {
    %3:u32 = let 1u
    %4:u32 = access %1, 0u, %3
    %q:u32 = let %4
    ret %q
  }
}
)";

    Run(ModuleConstant);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleConstantTest, ConstArrayDuplicate) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* c = b.Composite<array<u32, 3>>(1_u, 2_u, 3_u);
        auto* index = b.Let(1_u);
        auto* access = b.Access(ty.u32(), c, index);
        auto* r = b.Let("q", access);
        auto* c2 = b.Composite<array<u32, 3>>(1_u, 2_u, 3_u);
        auto* access2 = b.Access(ty.u32(), c2, r);
        b.Return(func, access2);
    });

    auto* src = R"(
%foo = func():u32 {
  $B1: {
    %2:u32 = let 1u
    %3:u32 = access array<u32, 3>(1u, 2u, 3u), %2
    %q:u32 = let %3
    %5:u32 = access array<u32, 3>(1u, 2u, 3u), %q
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:array<u32, 3> = let array<u32, 3>(1u, 2u, 3u)
}

%foo = func():u32 {
  $B2: {
    %3:u32 = let 1u
    %4:u32 = access %1, %3
    %q:u32 = let %4
    %6:u32 = access %1, %q
    ret %6
  }
}
)";

    Run(ModuleConstant);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::msl::writer::raise
