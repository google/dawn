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

#include "src/tint/lang/wgsl/ast/transform/push_constant_helper.h"

#include <memory>

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

namespace tint::ast::transform {
namespace {

class PushConstantHelperTest : public testing::Test {
  public:
    Program Parse(const char* s) {
        wgsl::reader::Options options;
        options.allowed_features = wgsl::AllowedFeatures::Everything();
        auto file = std::make_unique<Source::File>("test", s);
        return wgsl::reader::Parse(file.get(), options);
    }
};

TEST_F(PushConstantHelperTest, InsertMembersInOrder) {
    auto* s = R"()";
    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  /* @offset(0) */
  foo : f32,
  /* @offset(4) */
  bar : f32,
}

var<push_constant> push_constants : PushConstants;
)";

    auto program = Parse(s);
    ASSERT_TRUE(program.IsValid());
    ProgramBuilder b;
    program::CloneContext ctx{&b, &program, /* auto_clone_symbols */ true};
    PushConstantHelper helper(ctx);

    helper.InsertMember("foo", b.ty.f32(), 0);
    helper.InsertMember("bar", b.ty.f32(), 4);

    Symbol buffer_name = helper.Run();

    EXPECT_EQ(buffer_name.Name(), "push_constants");
    ctx.Clone();
    Program output = resolver::Resolve(b);
    ASSERT_TRUE(output.IsValid());
    EXPECT_EQ(expect, str(output));
}

TEST_F(PushConstantHelperTest, InsertMembersOutOfOrder) {
    auto* s = R"()";
    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  /* @offset(0) */
  bar : f32,
  /* @offset(4) */
  foo : f32,
}

var<push_constant> push_constants : PushConstants;
)";

    auto program = Parse(s);
    ASSERT_TRUE(program.IsValid());
    ProgramBuilder b;
    program::CloneContext ctx{&b, &program, /* auto_clone_symbols */ true};
    PushConstantHelper helper(ctx);

    helper.InsertMember("foo", b.ty.f32(), 4);
    helper.InsertMember("bar", b.ty.f32(), 0);

    Symbol buffer_name = helper.Run();

    EXPECT_EQ(buffer_name.Name(), "push_constants");
    ctx.Clone();
    Program output = resolver::Resolve(b);
    ASSERT_TRUE(output.IsValid());
    EXPECT_EQ(expect, str(output));
}

TEST_F(PushConstantHelperTest, InsertMembersBeforeAndAfter) {
    auto* s = R"()";
    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  /* @offset(0) */
  foo : f32,
  /* @offset(4) */
  bar : f32,
  /* @offset(8) */
  baz : f32,
}

var<push_constant> push_constants : PushConstants;
)";

    auto program = Parse(s);
    ASSERT_TRUE(program.IsValid());
    ProgramBuilder b;
    program::CloneContext ctx{&b, &program, /* auto_clone_symbols */ true};
    PushConstantHelper helper(ctx);

    helper.InsertMember("bar", b.ty.f32(), 4);
    helper.InsertMember("foo", b.ty.f32(), 0);
    helper.InsertMember("baz", b.ty.f32(), 8);

    Symbol buffer_name = helper.Run();

    EXPECT_EQ(buffer_name.Name(), "push_constants");
    ctx.Clone();
    Program output = resolver::Resolve(b);
    ASSERT_TRUE(output.IsValid());
    EXPECT_EQ(expect, str(output));
}

TEST_F(PushConstantHelperTest, InsertMembersThreeInReverse) {
    auto* s = R"()";
    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  /* @offset(0) */
  foo : f32,
  /* @offset(4) */
  bar : f32,
  /* @offset(8) */
  baz : f32,
}

var<push_constant> push_constants : PushConstants;
)";

    auto program = Parse(s);
    ASSERT_TRUE(program.IsValid());
    ProgramBuilder b;
    program::CloneContext ctx{&b, &program, /* auto_clone_symbols */ true};
    PushConstantHelper helper(ctx);

    helper.InsertMember("baz", b.ty.f32(), 8);
    helper.InsertMember("bar", b.ty.f32(), 4);
    helper.InsertMember("foo", b.ty.f32(), 0);

    Symbol buffer_name = helper.Run();

    EXPECT_EQ(buffer_name.Name(), "push_constants");
    ctx.Clone();
    Program output = resolver::Resolve(b);
    ASSERT_TRUE(output.IsValid());
    EXPECT_EQ(expect, str(output));
}

TEST_F(PushConstantHelperTest, MemberOffsetCollision) {
    auto* s = R"()";
    auto program = Parse(s);
    ASSERT_TRUE(program.IsValid());
    ProgramBuilder b;
    program::CloneContext ctx{&b, &program, /* auto_clone_symbols */ true};
    PushConstantHelper helper(ctx);

    helper.InsertMember("foo", b.ty.f32(), 0);
    helper.InsertMember("bar", b.ty.f32(), 0);

    Symbol buffer_name = helper.Run();

    ctx.Clone();
    Program output = resolver::Resolve(b);
    ASSERT_FALSE(output.IsValid());
}

TEST_F(PushConstantHelperTest, ExistingPushConstantBlockOneNew) {
    auto* src = R"(
enable chromium_experimental_push_constant;

struct S {
  a : f32,
}

var<push_constant> constants : S;

@fragment fn main() -> @location(0) f32 {
  return constants.a;
}
     )";
    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  a : f32,
  /* @offset(4) */
  foo : f32,
}

struct S {
  a : f32,
}

var<push_constant> constants : PushConstants;

@fragment
fn main() -> @location(0) f32 {
  return constants.a;
}
)";

    auto program = Parse(src);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();
    ProgramBuilder b;
    program::CloneContext ctx{&b, &program, /* auto_clone_symbols */ true};
    PushConstantHelper helper(ctx);

    helper.InsertMember("foo", b.ty.f32(), 4);

    Symbol buffer_name = helper.Run();

    EXPECT_EQ(buffer_name.Name(), "constants");
    ctx.Clone();
    Program output = resolver::Resolve(b);
    EXPECT_EQ(expect, str(output));
}

TEST_F(PushConstantHelperTest, ExistingPushConstantBlockTwoNewOutOfOrder) {
    auto* src = R"(
enable chromium_experimental_push_constant;

struct S {
  a : f32,
}

var<push_constant> constants : S;

@fragment fn main() -> @location(0) f32 {
  return constants.a;
}
     )";
    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  a : f32,
  /* @offset(4) */
  bar : f32,
  /* @offset(8) */
  foo : f32,
}

struct S {
  a : f32,
}

var<push_constant> constants : PushConstants;

@fragment
fn main() -> @location(0) f32 {
  return constants.a;
}
)";

    auto program = Parse(src);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();
    ProgramBuilder b;
    program::CloneContext ctx{&b, &program, /* auto_clone_symbols */ true};
    PushConstantHelper helper(ctx);

    helper.InsertMember("foo", b.ty.f32(), 8);
    helper.InsertMember("bar", b.ty.f32(), 4);

    Symbol buffer_name = helper.Run();

    EXPECT_EQ(buffer_name.Name(), "constants");
    ctx.Clone();
    Program output = resolver::Resolve(b);
    EXPECT_EQ(expect, str(output));
}

TEST_F(PushConstantHelperTest, ExistingPushConstantBlockCollisionWithExisting) {
    auto* src = R"(
enable chromium_experimental_push_constant;

struct S {
  a : f32,
}

var<push_constant> constants : S;

@fragment fn main() -> @location(0) f32 {
  return constants.a;
}
     )";

    auto program = Parse(src);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();
    ProgramBuilder b;
    program::CloneContext ctx{&b, &program, /* auto_clone_symbols */ true};
    PushConstantHelper helper(ctx);

    helper.InsertMember("foo", b.ty.f32(), 0);

    Symbol buffer_name = helper.Run();

    EXPECT_EQ(buffer_name.Name(), "constants");
    ctx.Clone();
    Program output = resolver::Resolve(b);
    EXPECT_FALSE(output.IsValid());
}

}  // namespace
}  // namespace tint::ast::transform
