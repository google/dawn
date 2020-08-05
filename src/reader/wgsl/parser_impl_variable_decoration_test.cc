// Copyright 2020 The Tint Authors.
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

#include "gtest/gtest.h"
#include "src/ast/binding_decoration.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/location_decoration.h"
#include "src/ast/set_decoration.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, VariableDecoration_Location) {
  auto* p = parser("location 4");
  auto deco = p->variable_decoration();
  ASSERT_NE(deco, nullptr);
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(deco->IsLocation());

  auto* loc = deco->AsLocation();
  EXPECT_EQ(loc->value(), 4u);
}

TEST_F(ParserImplTest, VariableDecoration_Location_MissingValue) {
  auto* p = parser("location");
  auto deco = p->variable_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:9: invalid value for location decoration");
}

TEST_F(ParserImplTest, VariableDecoration_Location_MissingInvalid) {
  auto* p = parser("location nan");
  auto deco = p->variable_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:10: invalid value for location decoration");
}

struct BuiltinData {
  const char* input;
  ast::Builtin result;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
  out << std::string(data.input);
  return out;
}
class BuiltinTest : public testing::TestWithParam<BuiltinData> {
 public:
  BuiltinTest() = default;
  ~BuiltinTest() override = default;

  void SetUp() override { ctx_.Reset(); }

  void TearDown() override { impl_ = nullptr; }

  ParserImpl* parser(const std::string& str) {
    impl_ = std::make_unique<ParserImpl>(&ctx_, str);
    return impl_.get();
  }

 private:
  std::unique_ptr<ParserImpl> impl_;
  Context ctx_;
};

TEST_P(BuiltinTest, VariableDecoration_Builtin) {
  auto params = GetParam();
  auto* p = parser(std::string("builtin ") + params.input);

  auto deco = p->variable_decoration();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(deco, nullptr);
  ASSERT_TRUE(deco->IsBuiltin());

  auto* builtin = deco->AsBuiltin();
  EXPECT_EQ(builtin->value(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    ParserImplTest,
    BuiltinTest,
    testing::Values(
        BuiltinData{"position", ast::Builtin::kPosition},
        BuiltinData{"vertex_idx", ast::Builtin::kVertexIdx},
        BuiltinData{"instance_idx", ast::Builtin::kInstanceIdx},
        BuiltinData{"front_facing", ast::Builtin::kFrontFacing},
        BuiltinData{"frag_coord", ast::Builtin::kFragCoord},
        BuiltinData{"frag_depth", ast::Builtin::kFragDepth},
        BuiltinData{"workgroup_size", ast::Builtin::kWorkgroupSize},
        BuiltinData{"local_invocation_id", ast::Builtin::kLocalInvocationId},
        BuiltinData{"local_invocation_idx", ast::Builtin::kLocalInvocationIdx},
        BuiltinData{"global_invocation_id",
                    ast::Builtin::kGlobalInvocationId}));

TEST_F(ParserImplTest, VariableDecoration_Builtin_MissingValue) {
  auto* p = parser("builtin");
  auto deco = p->variable_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:8: expected identifier for builtin");
}

TEST_F(ParserImplTest, VariableDecoration_Builtin_InvalidValue) {
  auto* p = parser("builtin other_thingy");
  auto deco = p->variable_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:9: invalid value for builtin decoration");
}

TEST_F(ParserImplTest, VariableDecoration_Builtin_MissingInvalid) {
  auto* p = parser("builtin 3");
  auto deco = p->variable_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:9: expected identifier for builtin");
}

TEST_F(ParserImplTest, VariableDecoration_Binding) {
  auto* p = parser("binding 4");
  auto deco = p->variable_decoration();
  ASSERT_NE(deco, nullptr);
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(deco->IsBinding());

  auto* binding = deco->AsBinding();
  EXPECT_EQ(binding->value(), 4u);
}

TEST_F(ParserImplTest, VariableDecoration_Binding_MissingValue) {
  auto* p = parser("binding");
  auto deco = p->variable_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:8: invalid value for binding decoration");
}

TEST_F(ParserImplTest, VariableDecoration_Binding_MissingInvalid) {
  auto* p = parser("binding nan");
  auto deco = p->variable_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:9: invalid value for binding decoration");
}

TEST_F(ParserImplTest, VariableDecoration_set) {
  auto* p = parser("set 4");
  auto deco = p->variable_decoration();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(deco.get(), nullptr);
  ASSERT_TRUE(deco->IsSet());

  auto* set = deco->AsSet();
  EXPECT_EQ(set->value(), 4u);
}

TEST_F(ParserImplTest, VariableDecoration_Set_MissingValue) {
  auto* p = parser("set");
  auto deco = p->variable_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:4: invalid value for set decoration");
}

TEST_F(ParserImplTest, VariableDecoration_Set_MissingInvalid) {
  auto* p = parser("set nan");
  auto deco = p->variable_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:5: invalid value for set decoration");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
